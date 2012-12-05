from pandas import DataFrame
from pylab import *

import timely.config as config

def plot_order(samples,name,filename):
  df = DataFrame({
    'img': [s.img_ind for s in samples],
    'action': [s.action_ind for s in samples],
    't': [s.t+s.dt for s in samples],
    'ap': [s.det_naive_ap for s in samples]
    })
  print(name)
  print("Num samples: %d"%df.shape[0])
  fixed_order = [14,  6,  8, 11, 17,  7,  2, 12, 19, 15, 10, 18,  4,  1, 13,  0,  5,  3,  9, 16]
  fig = plt.figure()
  ax = fig.add_subplot(111)
  ax.set_yticks(arange(-1,21))
  ax.set_yticklabels(['']+[str(d) for d in fixed_order]+[''])
  ax.set_yticklabels(['']+[config.pascal_classes[d] for d in fixed_order]+[''])
  ax.set_ylabel('Action Index')
  ax.set_xticks(arange(-1,22))
  ax.set_xlabel('Time (s)')
  title(name)
  for img in df.img.unique():
    a = (df[df.img==img].action)
    # change to linear for the fixed order
    a = [fixed_order.index(a) for a in a]
    t = df[df.img==img].t
    ap = df[df.img==img].ap
    if name =='Oracle':
      # we don't actually update the samples in oracle mode: only the dets
      # so these have to be sorted, as they are for oracle in dataset_policy
      ap = np.array(sorted(ap,reverse=True))
    size = 0+200*ap
    ax.plot(t,a,'.-',alpha=0.05,color='orange')
    ax.scatter(t,a,size,alpha=0.2)
  plt.show(fig)
  fig.savefig(filename,dpi=300)

dirname = '/Users/sergeyk/work/timely/data/results/evals/full_pascal_test_120/'
samples = np.load(dirname+'fixed_order_csc_default_0-20_manual_1_det_actual_ap_may29/cached_dets.npy_samples.npy')
plot_order(samples,'Fixed Order','./tasks/trajectories_fixed_order.png')

samples = np.load(dirname+'fastinf_csc_default_0-20_rl_half_auc_ap_raw_may29/cached_dets.npy_samples.npy')
plot_order(samples,'RL w/ MRF','./tasks/trajectories_rl_mrf.png')

samples = np.load(dirname+'oracle_csc_default_0-20_manual_1_det_actual_ap_may29/cached_dets.npy_samples.npy')
plot_order(samples,'Oracle','./tasks/trajectories_oracle.png')

samples = np.load(dirname+'random_csc_default_0-20_manual_1_det_actual_ap_may29/cached_dets.npy_samples.npy')
plot_order(samples,'Random','./tasks/trajectories_random.png')
