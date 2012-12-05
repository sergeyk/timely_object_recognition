import timely.common_imports
import matplotlib.pyplot as plt

class TestFastInf:
  def __init__(self):
    self.d = Dataset('test_pascal_train_tobi')
    
  def test_discretization_sane(self):
    num_bins = 5
    table = np.random.randn(5000,1)    
    _, discr_table = discretize_table(table, num_bins)           
    x = np.hstack(discr_table.T.tolist())
    a,_,_ = plt.hist(x, num_bins)    
    assert(np.where(a > 850)[0].shape[0] == 5)
    
  
  def test_importance_sample(self):
    # Not a very good test... wrote it to have a look at it.
    values = np.array([0,4.5,5.5,6.125,6.375,6.625,6.875,7.5,8.5,13]).astype(float)
    bounds = ut.importance_sample(values, 6)
    bounds_gt = np.array([0,4,6,7,9,13])
    comp = np.absolute(bounds-bounds_gt).astype(int)
    np.testing.assert_equal(comp, np.zeros(comp.shape))  
          
  def test_discretize_value_perfect(self):
    val = 0.3
    d = Dataset('full_pascal_trainval')
    suffix = 'perfect'
    fastdiscr = FastinfDiscretizer(d, suffix)
    discr = fastdiscr.discretize_value(val, clf_idx=0)
    assert(discr == 0)
    
    val = 1
    discr = fastdiscr.discretize_value(val, clf_idx=0)
    assert(discr == 1)
    
  def test_discretize_value_gist_csc(self):
    None

    
                          
if __name__=='__main__':
  tester = TestFastInf()  
  tester.test_discretize_value_gist_csc()
                          
  