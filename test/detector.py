import os

from timely.dataset import Dataset
from timely.image import Image
from timely.sliding_windows import SlidingWindows
from timely.detector import *

class TestDetector:
  def setup(self):
    self.dataset = Dataset('test_pascal_val')
    self.train_dataset = Dataset('test_pascal_train')

  def test_nms(self):
    """
    The test is actually in TestPolicy::load_dpm_detections().
    """
    None

  def test_subclass(self):
    d = Detector(self.train_dataset,self.train_dataset,'dog')
    assert(isinstance(d,Detector))

class TestPerfectDetector(TestDetector):
  def test_expected_time(self):
    d = PerfectDetector(self.train_dataset,self.train_dataset,'dog')
    img = Image(500,375,self.train_dataset.classes,'test')
    print d.expected_time(img)
    assert(d.expected_time(img) == 10)
    img = Image(250,375,self.train_dataset.classes,'test')
    assert(d.expected_time(img) == 5)

  def test_subclass(self):
    d = PerfectDetector(self.train_dataset,self.train_dataset,'dog')
    assert(isinstance(d,Detector))

