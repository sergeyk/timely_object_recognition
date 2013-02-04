import timely.config as config
from timely.config import get_ext_dets_filename
from timely.classifier import Classifier
from skvisutils import Dataset
from timely.training import svm_predict, svm_proba
from skvisutils import Image
from timely.evaluation import Evaluation

class CSCClassifier(Classifier):
  def __init__(self, suffix, cls, train_dataset, val_dataset, num_bins=5):
    self.name = 'csc'
    self.suffix = suffix
    self.cls = cls
    self.train_dataset = train_dataset
    self.val_dataset = val_dataset
    self.svm = self.load_svm()
    self.num_bins = num_bins
        
  def classify_image(self, scores):
    """
    Return score as a probability [0,1] for this class.
    Scores should be a vector of scores of the detections for this image.
    """
    # TODO: rename to classify_scores(), does not use image at all!
    vector = self.create_vector_from_scores(scores)
    return svm_proba(vector, self.svm)[0][1]
  
  def create_vector_from_scores(self, scores):
    "scores should be filtered for the index of the image"
    scores = self.normalize_dpm_scores(scores)
    vect = np.ones((1,3))
    if scores.size == 0:
      vect[0,:2] = 0
    elif scores.shape[0] == 1:
      vect[0,:2] = [np.max(scores), 0]
    else:
      vect[0,:2] = (-np.sort(-scores))[:2]
    return vect
          
  def normalize_dpm_scores(self, arr):     
    return np.power(np.exp(-2.*arr)+1,-1)
    
  def train_for_cls(self, ext_detector, kernel, C):
    dataset = ext_detector.dataset
    assert(dataset.name in ['full_pascal_train','full_pascal_trainval'])
    print dataset.name

    print '%d trains %s'%(comm_rank, self.cls)
    # Positive samples
    pos_imgs = dataset.get_pos_samples_for_class(self.cls)
    pos = []
    for idx, img_idx in enumerate(pos_imgs):
      image = dataset.images[img_idx]
      img_dets, _ = ext_detector.detect(image, astable=True)
      img_scores = img_dets.subset_arr('score')
      vector = self.create_vector_from_scores(img_scores)
      print 'load image %d/%d on %d'%(idx, len(pos_imgs), comm_rank)
      pos.append(vector)
    pos = np.concatenate(pos)

    # Negative samples
    neg_imgs = dataset.get_neg_samples_for_class(self.cls)
    neg = []
    for idx, img_idx in enumerate(neg_imgs):
      image = dataset.images[img_idx]
      img_dets, _ = ext_detector.detect(image, astable=True)
      img_scores = img_dets.subset_arr('score')
      vector = self.create_vector_from_scores(img_scores)
      print 'load image %d/%d on %d'%(idx, len(neg_imgs), comm_rank)
      neg.append(vector)
    neg = np.concatenate(neg)
    
    print '%d trains the model for'%comm_rank, self.cls
    self.train(pos, neg, kernel, C)
    
  def eval_cls(self, ext_detector):
    print 'evaluate svm for %s'%self.cls
    dataset = ext_detector.dataset
    assert(dataset.name in ['full_pascal_val','full_pascal_test'])
    print dataset.name

    table_cls = np.zeros(len(dataset.images))
    for img_idx, image in enumerate(dataset.images):
      print '%d eval on img %d/%d'%(comm_rank, img_idx, len(dataset.images))
      img_dets, _ = ext_detector.detect(image, astable=True)
      img_scores = img_dets.subset_arr('score')
      score = self.classify_image(img_scores)
      table_cls[img_idx] = score
      
    ap, _,_ = Evaluation.compute_cls_pr(table_cls, dataset.get_cls_ground_truth().subset_arr(self.cls))
    print 'ap on val for %s: %f'%(self.cls, ap)

    return table_cls