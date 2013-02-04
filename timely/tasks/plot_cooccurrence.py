"""
Runner script to output cooccurrence statistics for the synthetic
and PASCAL datasets.
"""

from skvisutils import Dataset

datasets = [
  'synthetic',
  'full_pascal_train','full_pascal_trainval',
  'full_pascal_val','full_pascal_test']

for dataset in datasets:
  d = Dataset(dataset) 
  f = d.plot_coocurrence()
  f = d.plot_coocurrence(second_order=True)
  f = d.plot_distribution()
