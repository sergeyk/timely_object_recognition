from timely.csc_classifier import CSCClassifier
from skvisutils import Dataset
import timely.config as config

class TestCscClassifier:
  def __init__(self):
    self.d = Dataset('full_pascal_trainval')
    self.d_val = Dataset('full_pascal_test')
    self.cls = 'dog'
    suffix = 'default'
    self.csc = CSCClassifier(suffix, self.cls, self.d, self.d_val)
    csc_test = np.load(config.get_ext_dets_filename(self.d, 'csc_default'))
    self.dets = csc_test[()]
  
  def get_scores_for_img(self, img_ind, cls_ind):
    dets = self.dets.filter_on_column('cls_ind', self.d.classes.index(self.cls), omit=True)
    dets = dets.subset(['score', 'img_ind'])
    dets = dets.filter_on_column('img_ind', 0, omit=True)
    dets.arr = self.csc.normalize_dpm_scores(dets.arr)
    return dets.subset_arr('score')
    
  def test_classify_image(self):
    scores = self.get_scores_for_img(0, self.cls)
    res = self.csc.classify_image(scores)
    assert(round(res,12) == 1.)
    
  def test_compute_histogram(self):
    for img in range(50):
      scores = self.get_scores_for_img(0, self.cls)      
      vector = self.csc.create_vector_from_scores(scores)

if __name__=='__main__':
  tester = TestCscClassifier()
  tester.test_classify_image()
