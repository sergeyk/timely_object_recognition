from timely.common_imports import *

import timely.config as config
from timely.fastinf_model import FastinfModel
from timely.ngram_model import RandomModel, NGramModel, FixedOrderModel

class BeliefState(object):
  """
  Encapsulates stuff that we keep track of during policy execution.
  Methods to initialize the model, update with an observed posterior,
  condition on observed values, and compute expected information gain.
  """

  ngram_modes = ['no_smooth','backoff']
  accepted_modes = ngram_modes+['random','fixed_order','fastinf']

  def __init__(self,dataset,actions,mode='fixed_order',bounds=None,model=None,fastinf_model_name='perfect'):
    assert(mode in self.accepted_modes)
    self.mode = mode
    self.dataset = dataset
    self.actions = actions
    self.bounds = bounds
    self.fastinf_model_name = fastinf_model_name

    # Is GIST in the actions? Need to behave differently if so.
    self.gist_mode = ('gist' in [action.name for action in self.actions])
    self.num_obs_vars = len(self.actions)
    if self.gist_mode:
      assert(self.actions[0].name == 'gist')
      self.num_obs_vars = len(self.actions)-1+len(self.dataset.classes)

    if mode == 'random':
      if model:
        assert(isinstance(model,RandomModel))
        self.model = model
      else:
        self.model = RandomModel(len(self.dataset.classes))
    elif mode=='no_smooth' or mode=='backoff':
      if model:
        assert(isinstance(model,NGramModel))
        self.model = model
      else:
        self.model = NGramModel(dataset,mode)
    elif mode=='fixed_order':
      if model:
        assert(isinstance(model,FixedOrderModel))
        self.model = model
      else:
        self.model = FixedOrderModel(dataset)
    elif mode=='fastinf':
      if model:
        assert(isinstance(model,FastinfModel))
        self.model = model
      else:
        self.model = FastinfModel(dataset, self.fastinf_model_name, self.num_obs_vars)
    else:
      raise RuntimeError("Unknown mode")
    self.reset()
    self.orig_p_c = self.get_p_c()

  def __repr__(self):
    return "BeliefState: \n%s\n%s"%(
      self.get_p_c(), zip(self.observed,self.observations))

  def get_p_c(self):
    return self.model.p_c

  def get_entropies(self):
    p_c = self.model.p_c
    p_not_c = 1 - p_c
    return -p_c*ut.log2(p_c) + -p_not_c*ut.log2(p_not_c)

  def reset(self):
    "Zero everything and reset the model."
    self.t = 0
    self.taken = np.zeros(len(self.actions))
    self.observed = np.zeros(self.num_obs_vars)
    self.observations = np.zeros(self.num_obs_vars)
    self.model.reset()

  def update_with_score(self,action_ind,score):
    "Update the taken and observations lists, the model, and get the new marginals."
    self.taken[action_ind] = 1
    self.observed[action_ind] = 1
    self.observations[action_ind] = score
    
    if self.mode in ['random','fixed_order']:
      self.model.update_with_observations(
        self.observed[:len(self.dataset.classes)],
        self.observations[:len(self.dataset.classes)])
    else:
      self.model.update_with_observations(self.observed,self.observations)
    #self.full_feature = self.compute_full_feature()

  def update_with_gist(self,action_ind,scores):
    """
    GIST returns multiple scores (for all the C_i), but is only one action.
    We deal with this by converting to multiple "actions" here, to maintain
    the interface to the FastInf model.
    """
    self.taken[action_ind] = 1
    # FastInf models expect GIST to be the second half of observations
    self.observed[len(self.dataset.classes):] = 1
    self.observations[len(self.dataset.classes):] = scores
    
    if self.mode=='random':
      None
    elif self.mode=='fixed_order':
      self.model.update_with_observations(
        self.observed[len(self.dataset.classes):],
        self.observations[len(self.dataset.classes):])
    else:
      self.model.update_with_observations(self.observed,self.observations)
    #self.full_feature = self.compute_full_feature()

  num_time_blocks = 1
  num_features = num_time_blocks * 47
  # [P(C) [P(C_i|O) for all i] [H(C_i|O) for all i] mean_entropy max_entropy t/S (1-t/S) t/T (1-t/T)]
  def compute_full_feature(self):
    """
    Return featurized representation of the current belief state.
    The features are in action blocks, meaning that this method returns
    an array of size (len(self.actions), self.num_features).
    To get a usable vector, simply flatten() this array.
    This is useful for zeroing-out all actions but the relevant one.
    NOTE: Keep the class variable num_features synced with the behavior here.
    """
    orig_p_c = self.orig_p_c
    p_c = self.get_p_c()
    h_c = self.get_entropies()
    mean_entropy = np.mean(h_c)
    max_entropy = np.max(h_c)
    h_c[h_c==-0]=0
    time_to_start_ratio = 0 if self.t >= self.bounds[0] else self.t/(self.bounds[0])
    time_ratio = 0 if self.t <= self.bounds[0] else self.t/self.bounds[1]

    # If GIST is an action, it's our first action, and doesn't care about any
    # class feature, only the mean_entropy and time_ratio ones.
    if self.gist_mode:
      orig_p_c = np.concatenate(([0],orig_p_c))

    # Tile the dynamic probability features
    num_classes = len(self.dataset.classes)
    p_c = np.tile(np.atleast_2d(p_c),(len(self.actions),1))
    h_c = np.tile(np.atleast_2d(h_c),(len(self.actions),1))

    rest = np.vstack((
        mean_entropy*np.ones(len(self.actions)),
        max_entropy*np.ones(len(self.actions)),
        time_to_start_ratio*np.ones(len(self.actions)),
        1.-time_to_start_ratio*np.ones(len(self.actions)),
        time_ratio*np.ones(len(self.actions)),
        1.-time_ratio*np.ones(len(self.actions))
      )).T

    feat = np.hstack((
        np.atleast_2d(orig_p_c).T, p_c, h_c, rest))

    # zero out those actions that have been taken
    # NOTE: this makes sense because it allows the policy to simply do argmax
    # all the time, without worrying about actions that have been taken:
    # the best it will be able to do for those is 0
    feat[np.flatnonzero(self.taken),:] = 0
    return feat

  def block_out_action(self, full_feature, action_ind=-1):
    """
    Take a full_feature matrix and zero out all the values except those
    in the relevant action block.
    If action_ind < 0, returns the flat feature with nothing zeroed out.
    """
    if action_ind < 0:
      return full_feature.flatten()
    assert(action_ind<len(self.actions))
    feature = np.zeros(np.prod(full_feature.shape))
    start_ind = action_ind*self.num_features
    feature[start_ind:start_ind+self.num_features] = full_feature[action_ind,:]
    return feature
