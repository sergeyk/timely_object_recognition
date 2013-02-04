import timely.config as config
from timely.classifier import Classifier
from timely.training import load_svm
from skvisutils import Dataset

class DPMClassifier(Classifier):
  def __init__(self,suffix=''):
    self.name = 'dpm'
    self.suffix = suffix
    
  def create_vector(self, feats, cls, img, intervals, lower, upper):
    if feats.arr.size == 0:
      return np.zeros((1,intervals+1))
    dpm = feats.subset(['score', 'cls_ind', 'img_ind'])
    img_dpm = dpm.filter_on_column('img_ind', img, omit=True)
    if img_dpm.arr.size == 0:
      print 'empty vector'
      return np.zeros((1,intervals+1))
    cls_dpm = img_dpm.filter_on_column('cls_ind', cls, omit=True)
    hist = self.compute_histogram(cls_dpm.arr, intervals, lower, upper)
    vector = np.zeros((1, intervals+1))
    vector[0,0:-1] = hist
    vector[0,-1] = img_dpm.shape[0]
    return vector
  
if __name__=='__main__':
  train_set = 'full_pascal_train'
  train_dataset = Dataset(train_set)  
  dpm_dir = os.path.join(config.res_dir, 'dpm_dets')
  filename = os.path.join(dpm_dir, train_set + '_dets_all_may25_DP.npy')
  dpm_train = np.load(filename)
  dpm_train = dpm_train[()]  
  dpm_train = dpm_train.subset(['score', 'cls_ind', 'img_ind'])
  dpm_classif = DPMClassifier()
  dpm_train.arr = dpm_classif.normalize_dpm_scores(dpm_train.arr)
  
  val_set = 'full_pascal_val'
  test_dataset = Dataset(val_set)  
  dpm_test_dir = os.path.join(config.res_dir, 'dpm_dets')
  filename = os.path.join(dpm_dir, val_set + '_dets_all_may25_DP.npy')
  dpm_test = np.load(filename)
  dpm_test = dpm_test[()]  
  dpm_test = dpm_test.subset(['score', 'cls_ind', 'img_ind'])
  dpm_test.arr = dpm_classif.normalize_dpm_scores(dpm_test.arr) 
  
  lowers = [0.,0.2,0.4]
  uppers = [1.,0.8,0.6]
  kernels = ['linear', 'rbf']
  intervalss = [10, 20, 50]
  clss = range(20)
  # TODO: why only these Cs?
  Cs = [2.5,3.]  
  list_of_parameters = [lowers, uppers, kernels, intervalss, clss, Cs]
  product_of_parameters = list(itertools.product(*list_of_parameters))
  
  for params_idx in range(mpi.comm_rank, len(product_of_parameters), mpi.comm_size):
    params = product_of_parameters[params_idx] 
    lower = params[0]
    upper = params[1]
    kernel = params[2]
    intervals = params[3]
    cls_idx = params[4]
    C = params[5]
    dpm_classif.train_for_cls(train_dataset, dpm_train,intervals,kernel, lower, upper, cls_idx, C)
    dpm_classif.test_svm(test_dataset, dpm_test, intervals,kernel, lower, upper, cls_idx, C)
