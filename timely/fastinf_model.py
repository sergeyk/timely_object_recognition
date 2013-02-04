import pexpect
import timely.config as config
from timely.ngram_model import InferenceModel
from timely.fastInf import FastinfDiscretizer

class FastinfModel(InferenceModel):
  def __init__(self,dataset,model_name,num_obs_vars,m='0',r1='1', lbp_iters=3000):
    # TODO: experiment with different values of fastinf

    self.dataset = dataset
    self.fd = FastinfDiscretizer(self.dataset, model_name)
    self.res_fname = config.get_fastinf_res_file(dataset,model_name,m,r1)

    # TODO: experiment with different amounts of smoothing
    # amount of smoothing is correlated with fastinf slowness, values [0,1)
    self.smoothing = 0.5
    self.cache_fname = config.get_fastinf_cache_file(dataset,model_name,m,r1,self.smoothing)

    if os.path.exists(self.cache_fname):
      with open(self.cache_fname) as f:
        print("Loading fastinf cache from file")
        self.cache = cPickle.load(f)
    else:
      self.cache = {}
    self.cmd = config.fastinf_bin+" -i %s -m 0 -Is %f -Imm %d"%(self.res_fname, self.smoothing, lbp_iters)
    self.num_obs_vars = num_obs_vars
    self.tt = ut.TicToc().tic()
    self.process = pexpect.spawn(self.cmd)
    self.blacklist = []
  
    marginals = self.get_marginals()
    print("FastinfModel: Computed initial marginals in %.3f sec"%self.tt.qtoc())

  def save_cache(self):
    "Write cache out to file with cPickle."
    print("Writing cache out to file with cPickle")
    with open(self.cache_fname,'w') as f:
      cPickle.dump(self.cache,f)

  def update_with_observations(self, observed, observations):
    self.tt.tic()
    evidence = self.num_obs_vars*['?']
    for i in np.flatnonzero(observed):
      evidence[i] = str(self.fd.discretize_value(observations[i],i))
    evidence = "(%s %s )"%(self.dataset.num_classes()*' ?', ' '.join(evidence))
    if evidence in self.blacklist:
      print("comm_rank %d: Skipping blacklisted evidence!"%comm_rank)
      # don't do anything, we don't want to get stuck with another timeout
      return
    try:
      marginals = self.get_marginals(evidence)
    except Exception as e:
      print("comm_rank %d: something went wrong in fastinf:get_marginals!!!"%
        comm_rank)
      print evidence
      print e
      # blacklist this evidence and restart process
      self.blacklist.append(evidence)
      try:
        self.process.close(force=True)
      except Exception as e2:
        print("comm_rank %d: can't close process!"%comm_rank)
      self.process = pexpect.spawn(self.cmd)
      self.get_marginals()
    
    # TODO: hack to set the actual classifier score here!
    # GIST is hard-coded as second half of observations!
    if self.num_obs_vars == len(self.dataset.classes):
      inds = np.flatnonzero(observed)
    else:
      inds = np.flatnonzero(observed[:len(self.dataset.classes)])
    self.p_c[inds] = observations[inds]
    #print("FastinfModel: Computed marginals given evidence in %.3f sec"%self.tt.qtoc())

  def reset(self):
    """
    Get back to the initial state, erasing the effects of any evidence
    that has been applied.
    Sends totally uninformative evidence to get back to the priors.
    Is actually instantaneous due to caching.
    """
    observations = observed = np.zeros(self.num_obs_vars)
    self.update_with_observations(observed,observations)

  def get_marginals(self,evidence=None):
    """
    Parse marginals out of printed output of infer_timely.
    If evidence is given, first sends it to stdin of the process.
    Also update self.p_c with the marginals.
    """
    if evidence:
      if evidence in self.cache:
        print "Fetching cached marginals"
        marginals = self.cache[evidence]
        self.p_c = np.array([m[1] for m in marginals[:self.dataset.num_classes()]])
        return marginals
      else:
        self.process.sendline(evidence)
    self.process.expect('Enter your evidence')
    output = self.process.before
    marginals = FastinfModel.extract_marginals(output)
    # Don't cache anything!
    #self.cache[evidence] = marginals
    self.p_c = np.array([m[1] for m in marginals[:self.dataset.num_classes()]])
    return marginals

  @classmethod
  def extract_marginals(cls, output):
    """
    Parse the output of the infer_timely binary for the variable marginals.
    Return a list of lists, where the index in the outer list is the index
    of the variable, and the index of an inner list is the index of that
    variable's value.
    """
    lines = output.split('\r\n')
    try:
      ind = lines.index('# belief marginals / exact marginals / KL Divergence') 
    except ValueError:
      print("ERROR: cannot find marginals in output")
    marginals = []
    for line in lines[ind+1:]:
      if re.search('Partition',line) or re.search('^\w*$',line):
        break
      vals = line.split()[1:]
      vals = [float(v) for v in vals]
      marginals.append(vals)
    return marginals
