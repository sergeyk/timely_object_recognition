"""
Methods to predict class presence in an image using the GIST feature.
"""
import timely.config as config
from skvisutils import Image
from timely.classifier import Classifier
from timely.evaluation import Evaluation

class GistClassifier(Classifier):
  """
  Compute a likelihood-vector for the classes given a (precomputed) gist detection
  """
  def __init__(self, cls, train_d, gist_table=None, val_d=None):
    """ 
    Load all gist features right away
    """
    self.train_d = train_d
    self.val_d = val_d
      
    Classifier.__init__(self)
    
    self.tt.tic()
    if gist_table == None:
      print("Started loading GIST")
      self.gist_table = np.load(config.get_gist_dict_filename(train_d))
      print("Time spent loading gist: %.3f"%self.tt.qtoc())
    else:
      self.gist_table = gist_table    
    self.cls = cls
    self.svm = self.load_svm()
  
  def get_scores_for_image_set(self, image_idc):
    """
    Get a list of image indices (of own dataset) and return all scores as column-vect
    """
    scores = np.zeros((len(image_idc), 1))    
    for idx, img_idx in enumerate(image_idc):
      img = self.dataset.images[img_idx]
      scores[idx] = self.get_score(img)      
    return scores
    
  def get_score(self, img):
    index = self.val_d.get_img_ind(img)
    gist = np.array(self.gist_table[index])
    score = self.svm.predict_proba(gist)[0,1] 
    return score
        
  def load_svm(self):
    filename = config.get_gist_svm_filename(self.cls,self.train_d)
    if os.path.exists(filename):
      print 'load svm %s'%filename
      try:
        svm = cPickle.loads(open(filename).read())
        self.svm = svm
      except:
        print 'could not load svm. Probably we are in crossvalidation and there is much going on'
        svm = None 
    else:
      print 'gist svm for',self.cls,'does not exist at %s'%filename
      svm = None
    return svm
         
  def get_gists_for_imgs(self, imgs, dataset):
    images = dataset.images
    num = imgs.size
    print num
    gist = np.zeros((num, 960))
    ind = 0    
    for img in imgs:
      image = dataset.get_image_by_filename(images[img].name)
      index = dataset.get_img_ind(image)
      gist[ind] = self.gist_table[index]
      ind += 1
    return gist
  
  def train_svm(self, dataset, kernel='linear', C=1.0, gamma=0.0):
    """
    Train classifiers for class  
    """  
    print '%d trains class %s'%(mpi.comm_rank, self.cls)
    t = time.time()
    pos = dataset.get_pos_samples_for_class(self.cls)
    neg = dataset.get_neg_samples_for_class(self.cls)
               
    pos_gist = self.gist_table[pos, :]
    neg_gist = self.gist_table[neg, :]      
    
    x = np.concatenate((pos_gist, neg_gist))
    y = [1]*pos.shape[0] + [-1]*neg.shape[0]
    print '%d compute svm for %s'%(mpi.comm_rank, self.cls)
    svm_filename = config.get_gist_svm_filename(self.cls, dataset)
    print svm_filename
    self.svm = train_svm(x, y, kernel, C, gamma)
    print '\ttook', time.time()-t,'sec'
    print 'the score on train-data is %f'%self.svm.score(x,y)
    table_t = self.svm.predict_proba(x)
    y2 = np.array(y)
    y2 = (y2+1)/2 # switch to 0/1
    ap,_,_ = Evaluation.compute_cls_pr(table_t[:,1], y2)
    print 'ap on train: %f'%ap      
    cPickle.dump(self.svm, open(svm_filename, 'w'))
    return ap 
        
  def classify_dataset(self):     
    return self.svm.predict_proba(self.gist_table)[:,1]
    
def gist_evaluate_svms(d_train, d_val):
  gist_scores = np.zeros((len(d_val.images), len(d_val.classes)))
  gist_table = np.load(config.get_gist_dict_filename(d_train.name))
  
  kernels = ['rbf', 'linear', 'poly']
  Cs = [1,10,100]
  gammas = [0,0.3,1]
  setts = list(itertools.product(kernels, Cs, gammas))
  val_gt = d_val.get_cls_ground_truth()
  
  for cls_idx in range(len(d_val.classes)):
    cls = d_val.classes[cls_idx]  
    gist = GistClassifier(cls, d_train, gist_table=gist_table, d_val=d_val)
    filename = config.get_gist_crossval_filename(d_train, cls) 
    # doing some crossval right here!!!
    for set_idx in range(mpi.comm_rank, len(setts), mpi.comm_size):
      sett = setts[len(setts)-1-set_idx]
      kernel = sett[0]
      C = sett[1]
      gamma = sett[2]
      train_ap = gist.train_svm(d_train, kernel, C, gamma)
          
      val_gist_table = np.load(config.get_gist_dict_filename(d_val.name))     
      gist_scores = gist.svm.predict_proba(val_gist_table)[:,1]
      
      val_ap,_,_ = Evaluation.compute_cls_pr(gist_scores, val_gt.subset_arr(cls))
      w = open(filename, 'a')
      w.write('%s C=%d gamma=%f - train: %f, val: %f\n'%(kernel, C, gamma, train_ap, val_ap))
      w.close()
      print 'ap on val: %f'%val_ap
      
  print '%d at safebarrier'%mpi.comm_rank
  safebarrier(comm)
  gist_scores = comm.reduce(gist_scores)
  if mpi.comm_rank == 0:
    print gist_scores
    filename = config.get_gist_classifications_filename(d_val)    
    cPickle.dump(gist_scores, open(filename,'w'))
    res = Evaluation.compute_cls_pr(gist_scores, val_gt.arr)
    print res

def gist_train_good_svms(all_settings, d_train):
  gist_table = np.load(config.get_gist_dict_filename(d_train.name))
  
  for sett_idx in range(mpi.comm_rank, len(all_settings), mpi.comm_size):
    sett = all_settings[sett_idx]    
    cls = sett[0]
    C = sett[1]
    kernel = sett[2]
    gamma = sett[3]    
    gist = GistClassifier(cls, d_train, gist_table)
    filename = config.get_gist_crossval_filename(d_train, cls) 
    gist.train_svm(d_train, kernel, C, gamma)

def gist_classify_dataset(d):
  classes = d.classes
  table = np.zeros((len(d.images), len(classes)))
  savefile = config.get_gist_fastinf_table_name(d, None)
   
  print savefile
  if os.path.exists(savefile):
    return cPickle.load(open(savefile, 'r'))
    None
    
  for cls_idx in range(mpi.comm_rank, len(classes), mpi.comm_size):
    cls = classes[cls_idx]
    
    savefile = config.get_gist_fastinf_table_name(d, cls)
    if os.path.exists(savefile):
      table = cPickle.load(open(savefile,'r'))
      continue
    gist = GistClassifier(cls, d)
    table[:,cls_idx] = gist.classify_dataset()        
    cPickle.dump(table, open(savefile,'w'))
    
  safebarrier(comm)
  table = comm.allreduce(table)  
  if mpi.comm_rank == 0:
    savefile = config.get_gist_fastinf_table_name(d, None)
    cPickle.dump(table, open(savefile,'w'))
  return table

def read_best_svms_from_file(d_train):
  all_settings = []
  for cls in config.pascal_classes:
    filename = config.get_gist_crossval_filename(d_train, cls)
    lines = open(filename,'r').readlines()
    best_ap = 0
    best_line_idx = -1
    for line_idx, line in enumerate(lines):
      ap = float(line.split()[-1])
      if ap > best_ap:
        best_ap = ap
        best_line_idx = line_idx
    
    best_line = lines[best_line_idx].split()
    kernel = best_line[0]
    C = int(best_line[1].split('=')[1])
    gamma = float(best_line[2].split('=')[1])
    all_settings.append([cls, C, kernel, gamma, best_ap])
       
  return all_settings

def training(d, evaluate=False):
  d_train = Dataset('full_pascal_train')
  d_val = Dataset('full_pascal_val')
  
  if evaluate:
    gist_evaluate_svms(d_train, d_val)
  all_settings = read_best_svms_from_file(d_train)  
  gist_train_good_svms(all_settings, d_train)
  
  # Now use the best trained values to also train full_pascal_trainval:
  gist_train_good_svms(all_settings, d)
  
  # compute mean AP:
  sum_ap = 0
  print '====APs===='
  for sett in all_settings:
    sum_ap += sett[-1]
    print '%s \t- %f'%(sett[0], sett[-1]) 
  mean_ap = sum_ap/20.
  print 'mean AP %f'%mean_ap
  
if __name__=='__main__':
  d = Dataset('full_pascal_trainval')
  #training(d)
  gist_classify_dataset(d)
  