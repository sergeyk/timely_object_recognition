from timely.common_imports import *

from timely.dataset import Dataset
from timely.dataset_policy import DatasetPolicy
from timely.evaluation import Evaluation
from timely.sliding_windows import SlidingWindows
import timely.config as config
import math

class TestEvaluationSynthetic:
  def __init__(self):
    self.d = Dataset('test_data2',force=True)
    self.classes = ["A","B","C"]
    self.det_gt = self.d.get_det_gt()

  def test(self):
    scores = np.ones(self.det_gt.shape[0])
    dets = self.det_gt.append_column('score',scores)

    scores = np.ones(self.d.get_det_gt_for_class('A').shape[0])
    dets_just_A = self.d.get_det_gt_for_class('A')
    dets_just_A = dets_just_A.append_column('score',scores)

    self.d.set_values('uniform')
    assert(np.all(self.d.values == 1./3 * np.ones(len(self.classes))))
    dp = DatasetPolicy(self.d,self.d,detector='perfect')
    ev = Evaluation(dp)
    ap = ev.compute_det_map(dets,self.det_gt)
    assert(ap==1)
    ap = ev.compute_det_map(dets_just_A,self.det_gt)
    print(ap)
    assert(ut.fequal(ap, 0.33333333333333))

    self.d.set_values('inverse_prior')
    assert(np.all(self.d.values == np.array([0.25,0.25,0.5])))
    dp = DatasetPolicy(self.d,self.d,detector='perfect')
    ev = Evaluation(dp)
    ap = ev.compute_det_map(dets,self.det_gt)
    assert(ap==1)
    ap = ev.compute_det_map(dets_just_A,self.det_gt)
    print(ap)
    assert(ut.fequal(ap, 0.25))

class TestEvaluationPerfect:
  def __init__(self):
    self.csc_trainval = cPickle.load(open(os.path.join(config.get_ext_test_support_dir(), 'csc_trainval'), 'r'))
    self.csc_test = cPickle.load(open(os.path.join(config.get_ext_test_support_dir(), 'csc_test'), 'r'))
    self.ext_csc_test = cPickle.load(open(os.path.join(config.get_ext_test_support_dir(), 'ext_csc_test'), 'r'))
    self.ext_csc_trainval = cPickle.load(open(os.path.join(config.get_ext_test_support_dir(), 'ext_csc_trainval'), 'r'))
    self.d_train = Dataset('full_pascal_trainval')
    self.trainval_gt = self.d_train.get_cls_ground_truth()
    self.d_test = Dataset('full_pascal_test')
    self.test_gt = self.d_test.get_cls_ground_truth()
  
  def setup(self):
    train_dataset = Dataset('test_pascal_train',force=True)
    dataset = Dataset('test_pascal_val',force=True)
    self.dp = DatasetPolicy(dataset,train_dataset,detector='perfect')
    self.evaluation = Evaluation(self.dp)    

  def test_compute_pr_multiclass(self):
    cols = ['x','y','w','h','cls_ind','img_ind','diff'] 
    dets_cols = ['x', 'y', 'w', 'h', 'score', 'time', 'cls_ind', 'img_ind']
    
    # two objects of different classes in the image, perfect detection
    arr = np.array(
        [ [0,0,10,10,0,0,0],
          [10,10,10,10,1,0,0] ])
    gt = Table(arr,cols)

    dets_arr = np.array(
        [ [0,0,10,10,-1,-1,0,0],
          [10,10,10,10,-1,-1,1,0] ]) 
    dets = Table(dets_arr,dets_cols)
    
    # make sure gt and gt_cols aren't modified
    gt_arr_copy = gt.arr.copy()
    gt_cols_copy = list(gt.cols)
    ap,rec,prec = self.evaluation.compute_det_pr(dets, gt)
    assert(np.all(gt.arr == gt_arr_copy))
    assert(gt_cols_copy == gt.cols)

    correct_ap = 1
    correct_rec = np.array([0.5,1])
    correct_prec = np.array([1,1])
    print((ap, rec, prec))
    assert(correct_ap == ap)
    assert(np.all(correct_rec==rec))
    assert(np.all(correct_prec==prec))

    # some extra detections to generate false positives
    dets_arr = np.array(
        [ [0,0,10,10,-1,-1,0,0],
          [0,0,10,10,0,-1,0,0],
          [10,10,10,10,0,-1,1,0],
          [10,10,10,10,-1,-1,1,0] ]) 
    dets = Table(dets_arr,dets_cols)

    ap,rec,prec = self.evaluation.compute_det_pr(dets, gt)
    correct_rec = np.array([0.5,1,1,1])
    correct_prec = np.array([1,1,2./3,0.5])
    print((ap, rec, prec))
    assert(np.all(correct_rec==rec))
    assert(np.all(correct_prec==prec))

    # confirm that running on the same dets gives the same answer
    ap,rec,prec = self.evaluation.compute_det_pr(dets, gt)
    correct_rec = np.array([0.5,1,1,1])
    correct_prec = np.array([1,1,2./3,0.5])
    print((ap, rec, prec))
    assert(np.all(correct_rec==rec))
    assert(np.all(correct_prec==prec))

    # now let's add two objects of a different class to gt to lower recall
    arr = np.array(
        [ [0,0,10,10,0,0,0],
          [10,10,10,10,1,0,0],
          [20,20,10,10,2,0,0],
          [30,30,10,10,2,0,0] ])
    gt = Table(arr,cols)
    ap,rec,prec = self.evaluation.compute_det_pr(dets, gt)
    correct_rec = np.array([0.25,0.5,0.5,0.5])
    correct_prec = np.array([1,1,2./3,0.5])
    print((ap, rec, prec))
    assert(np.all(correct_rec==rec))
    assert(np.all(correct_prec==prec))

    # now call it with empty detections
    dets_arr = np.array([])
    dets = Table(dets_arr,dets_cols)
    ap,rec,prec = self.evaluation.compute_det_pr(dets, gt)
    correct_ap = 0
    correct_rec = np.array([0])
    correct_prec = np.array([0])
    print((ap, rec, prec))
    assert(np.all(correct_ap==ap))
    assert(np.all(correct_rec==rec))
    assert(np.all(correct_prec==prec))
  
  def test_compute_cls_map(self):
    res = Evaluation.compute_cls_map(self.csc_trainval, self.trainval_gt)
    assert(round(res,11) == 0.47206391958)
    
  def test_compute_cls_map_half(self):
    table_csc_half = Table()
    table_csc_half.cols = list(self.csc_trainval.cols)
    for _ in range(10):
      rand_inds = np.random.permutation(range(5011))[:2500]
      table_csc_half.arr = self.csc_trainval.arr[rand_inds,:]      
      res = Evaluation.compute_cls_map(table_csc_half, self.trainval_gt)
      assert(round(res,6) > .45)
  
  def test_compute_cls_map_gt(self):
    res = Evaluation.compute_cls_map(self.trainval_gt, self.trainval_gt)
    assert(round(res,6) == 1)
    
  def test_compute_cls_map_gt_half(self):
    rand_inds = np.random.permutation(range(5011))[:2500]
    table_gt_half = Table()
    table_gt_half.arr = np.hstack((self.trainval_gt.arr,np.array(np.arange(5011), ndmin=2).T))
    table_gt_half.arr = table_gt_half.arr[rand_inds,:]
    table_gt_half.cols = list(self.trainval_gt.cols) + ['img_ind']
    res = Evaluation.compute_cls_map(table_gt_half, self.trainval_gt)
    assert(round(res,6) == 1)
  
  def test_compute_cls_map_random_clf(self):
    clf_table = Table()
    num_test = 10
    ress = np.zeros((num_test,))
    for idx in range(num_test):
      clf_table.arr = np.hstack((np.random.rand(5011, 20),np.array(np.arange(5011), ndmin=2).T))
      clf_table.cols = list(self.trainval_gt.cols) + ['img_ind']
      res = Evaluation.compute_cls_map(clf_table, self.trainval_gt)
      ress[idx] = res
    assert(np.mean(ress) < 0.09)
  
  def test_other_scores(self):
    print 'csc_test', Evaluation.compute_cls_map(self.csc_test, self.test_gt)
    print 'csc_trainval', Evaluation.compute_cls_map(self.csc_trainval, self.trainval_gt)
    
    print 'ext_test', Evaluation.compute_cls_map(self.ext_csc_test, self.test_gt)
    print 'ext_trainval', Evaluation.compute_cls_map(self.ext_csc_trainval, self.trainval_gt)
    
    filename = os.path.join(config.get_ext_dets_foldname(self.d_test), 'dp', 'table_chi2_20')
    ext_table_chi2_20 = cPickle.load(open(filename, 'r'))
    print 'ext_chi2_20_test', Evaluation.compute_cls_map(ext_table_chi2_20, self.test_gt)    
    
    filename = os.path.join(config.get_ext_dets_foldname(self.d_train), 'dp', 'table_chi2_20')
    ext_table_chi2_20_tv = cPickle.load(open(filename, 'r'))
    print 'ext_chi2_20_trainval', Evaluation.compute_cls_map(ext_table_chi2_20_tv, self.trainval_gt)
    
    filename = os.path.join(config.get_ext_dets_foldname(self.d_test), 'dp', 'table_rbf_20')
    ext_table_rbf_20 = cPickle.load(open(filename, 'r'))
    print 'ext_rbf_20_test', Evaluation.compute_cls_map(ext_table_rbf_20, self.test_gt)    
    
    filename = os.path.join(config.get_ext_dets_foldname(self.d_train), 'dp', 'table_rbf_20')
    ext_table_rbf_20_tv = cPickle.load(open(filename, 'r'))
    print 'ext_rbf_20_trainval', Evaluation.compute_cls_map(ext_table_rbf_20_tv, self.trainval_gt)
       
    filename = os.path.join(config.get_ext_dets_foldname(self.d_test), 'dp', 'table_linear_20')
    ext_linear_20_test = cPickle.load(open(filename, 'r'))
    print 'ext_linear_test', Evaluation.compute_cls_map(ext_linear_20_test, self.test_gt)
    
    filename = os.path.join(config.get_ext_dets_foldname(self.d_train), 'dp', 'table_linear_20')
    ext_table_linear_20 = cPickle.load(open(filename, 'r'))
    print 'ext_linear_20_trainval', Evaluation.compute_cls_map(ext_table_linear_20, self.trainval_gt)    
        
    filename = 'tab_linear_5'
    ext_tab_lin_5 = cPickle.load(open(filename, 'r'))
    print 'ext_tab_lin_5_trainval', Evaluation.compute_cls_map(ext_tab_lin_5, self.trainval_gt)    
        
if __name__=='__main__':
  tester = TestEvaluationPerfect()
  tester.test_other_scores()
