from timely.common_imports import *
import timely.config as config

from timely.dataset import Dataset
from timely.fastinf_model import FastinfModel

def test():
  dataset = Dataset('full_pascal_trainval')
  fm = FastinfModel(dataset, 'perfect', 20)
  # NOTE: just took values from a run of the thing
  
  prior_correct = [float(x) for x in "0.050543  0.053053  0.073697  0.038331  0.050954  0.041879  0.16149\
    0.068721  0.10296   0.026837  0.043779  0.087683  0.063447  0.052205\
    0.41049   0.051664  0.014211  0.068361  0.056969  0.05046".split()]
  np.testing.assert_almost_equal(fm.p_c, prior_correct, 4)
   
  observations = np.zeros(20)
  taken = np.zeros(20)
  fm.update_with_observations(taken,observations)
  np.testing.assert_almost_equal(fm.p_c, prior_correct, 4)
  observations[5] = 1
  taken[5] = 1
  fm.update_with_observations(taken,observations)
  print fm.p_c
  correct = [float(x) for x in  "0.027355   0.11855    0.027593   0.026851   0.012569   0.98999    0.52232\
    0.017783   0.010806   0.015199   0.0044641  0.02389    0.033602   0.089089\
    0.50297    0.0083272  0.0088274  0.0098522  0.034259   0.0086298".split()]
  np.testing.assert_almost_equal(fm.p_c, correct, 4)
  observations[15] = 0
  taken[15] = 1
  fm.update_with_observations(taken,observations)
  correct = [float(x) for x in "2.73590000e-02   1.19030000e-01   2.75500000e-02   2.68760000e-02 \
   1.23920000e-02   9.90200000e-01   5.25320000e-01   1.76120000e-02 \
   1.05030000e-02   1.52130000e-02   4.26410000e-03   2.38250000e-02 \
   3.36870000e-02   8.96450000e-02   5.04300000e-01   8.71880000e-05 \
   8.82630000e-03   9.55290000e-03   3.43240000e-02   8.44510000e-03".split()]
  np.testing.assert_almost_equal(fm.p_c, correct)

  # reinit_marginals
  fm.reset()
  np.testing.assert_equal(fm.p_c, prior_correct)

  print(fm.cache)
  
if __name__=='__main__':
  test()