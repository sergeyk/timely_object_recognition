import timely.config as config
from timely.detector import Detector
from timely.csc_classifier import CSCClassifier
from timely.dpm_classifier import DPMClassifier

class ExternalDetector(Detector):
  """
  A mock interface to the Felzenszwalb DPM, CSC, or the Pendersoli CtF detector.
  Actually works by pre-loading all the detections and then returning them as
  requested.
  """

  def __init__(self, dataset, train_dataset, cls, dets, detname):
    """
    Expects cached detections in Table format to be passed in.
    The dets should not have the 'cls_ind' column, as they should all be of the
    same class.
    """
    Detector.__init__(self,dataset,train_dataset,cls,detname)
    self.dets = dets
    # TODO: hack for csc_X
    suffix = detname[4:]

    if self.detname=='dpm':
      self.classif = DPMClassifier()
    else:
      self.classif = CSCClassifier(suffix, cls, train_dataset, dataset)

  def detect(self, image, astable=False, dets=None):
    """
    Return the detections that match that image index in cached dets.
    Must return in the same format as the Detector superclass, so we have to
    delete a column.
    """
    if not dets:
      img_ind = self.dataset.get_img_ind(image)
      dets = self.dets.filter_on_column('img_ind',img_ind,omit=True)
    time_passed = 0
    if not dets.arr.shape[0]<1:
      time_passed = np.max(dets.subset_arr('time'))
    # Halve the time passed if my may25 DPM detector, to have reasonable times
    # Also halve the time passed by csc_half detector, because we halved its AP
    if self.detname=='dpm_may25' or self.detname=='csc_half':
      time_passed /= 2

    # TODO: hack that returns time around 1s always
    #hist(np.maximum(0.8,1+0.1*np.random.randn(1000)),20)
    time_passed = np.maximum(0.8,1+0.1*np.random.randn())

    dets = dets.with_column_omitted('time')
    if astable:
      return (dets, time_passed)
    else:
      return (dets.arr, time_passed) 

  def compute_score(self, image, oracle=False, dets=None):
    """
    Return the 0/1 decision of whether the cls of this detector is present in
    the image, given the detections table.
    If oracle=True, returns the correct answer (look up the ground truth).
    """
    if oracle:
      return Detector.compute_score(self, image, oracle)
    if not dets:
      img_ind = self.dataset.get_img_ind(image)
      dets = self.dets.filter_on_column('img_ind',img_ind)
    scores = dets.subset_arr('score')
    score = self.classif.classify_image(scores)
    dt = 0
    # TODO: figure out the dt situation above
    return (score,dt)
