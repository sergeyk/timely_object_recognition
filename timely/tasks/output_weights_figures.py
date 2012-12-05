from pylab import *

w_g = loadtxt('/Users/sergeyk/work/timely/data/results/evals/full_pascal_val/fastinf_gist-csc_default_0-20_greedy_auc_ap_raw_may29/weights.txt').reshape((21,47))
w_rl = loadtxt('/Users/sergeyk/work/timely/data/results/evals/full_pascal_val/fastinf_gist-csc_default_0-20_rl_regression_auc_ap_raw_may29/weights.txt').reshape((21,47))

matshow(-w_rl,cmap='RdBu',vmin=-4,vmax=4)
savefig('gist_rl_weights.png')

matshow(-w_g,cmap='RdBu',vmin=-4,vmax=4)
savefig('gist_greedy_weights.png')