import timely.config as config
import skpyutils.util as ut

from sklearn.svm import SVC, LinearSVC
from scipy import io

def save_to_mat(filename, X, Y, testX):
  Y = Y.astype('float64')
  X = X.astype('float64')
  print X
  print Y
  testX = testX.astype('float64')
  io.savemat(config.repo_dir + 'fast-additive-svms/tmp/' + filename,{'X':X,'Y':Y,'testX':testX})
  
def load_from_mat(filename, value):
  mdict = {}
  io.loadmat(config.repo_dir +'fast-additive-svms/tmp/'+ filename, mdict)
  return mdict[value]

def mat_train_test_svm(filename):
  ut.run_matlab_script(config.repo_dir + 'fast-additive-svms/', \
         'train_test_svm(\'' +config.repo_dir +'fast-additive-svms/tmp/'+ filename + '\')')  

def chi_square_kernel(x, y):
  """
  Create a custom chi-square kernel  
  """
  chi_sum = 0  
  for i in range(x.size):
    if not (x[0, i] + y[0, i]) == 0 and not x[0, i] == y[0, i]: 
      chi_sum += 2*(x[0, i] - y[0, i])**2/(x[0, i] + y[0, i])
  
  return chi_sum

def train_svm(x, y, kernel='linear', C=100.0, gamma=0.0):
  """
  Train a svm.
  x - n x features data
  y - n x 1 labels
  kernel - kernel to be used in ['linear', 'rbf', 'chi2']
  """
  probab=True # Never change this value!
  if kernel == 'chi2':
    clf = SVC(kernel='precomputed',C=C, probability=probab)
    gram = np.zeros((x.shape[0],x.shape[0]))
    t_gram = time.time()
    inner_total = x.shape[0]**2/2
    inner_act = 0
    for i in range(x.shape[0]):
      for j in range(x.shape[0]-i-1):
        j += i + 1
        kern = chi_square_kernel(x[i,:], x[j,:])
        gram[i,j] = kern
        gram[j,i] = kern
        inner_act += 1
        if inner_act%5000 == 0:
          print '%d is in gram on: %d / %d'%(mpi.comm_rank, inner_act, inner_total)
          print '\t%f seconds passed'%(time.time() - t_gram)
    t_gram = time.time() - t_gram
    print 'computed gram-matrix in',t_gram,'seconds'
    clf.fit(gram, y)
  elif kernel == 'rbf':
    clf = SVC(kernel=kernel, C=C, probability=probab, gamma=gamma)
    clf.fit(x, y)
  elif kernel == 'poly':
    clf = SVC(kernel=kernel, C=C, probability=probab, gamma=0, degree=2)
    clf.fit(x, y)
  elif kernel == 'linear':
    #clf = LinearSVC(C=C, class_weight='auto')
    clf = SVC(C=C,probability=probab)
    clf.fit(x, y, class_weight='auto')
  else:
    raise RuntimeError("Unknown kernel passed to train_svm: %s"%kernel)  
  
  return clf

def svm_predict(x, clf):
  x = np.array(x, dtype='float')
  result = clf.predict(x)
  return (result+1)/2

def svm_proba(x, clf):
  # TODO: why is this method needed? Just call directly!
  return clf.predict_proba(x)

def save_svm(model, filename):
  cPickle.dump(model, open(filename, 'w'))

def load_svm(filename, probability=True):
  dump = open(filename).read()
  model = cPickle.loads(dump)
  return model

def get_hist(assignments, M):
  counts = Counter(assignments.reshape(1,assignments.size).astype('float64')[0])
  histogram = [counts.get(x+1,0) for x in range(M)]
  histogram = np.matrix(histogram, dtype = 'float64')
  histogram = histogram/np.sum(histogram)
  return histogram
