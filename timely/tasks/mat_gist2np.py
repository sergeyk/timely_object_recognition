'''
Created on Nov 17, 2011

@author: Tobias Baumgartner
'''

from common_imports import *

import scipy.io
import synthetic.util as ut

if __name__ == '__main__':
  folder = config.VOC_dir + 'JPEGImages/gist/'
  gist_save = os.path.join(config.res_dir,'gist_features/')
  ut.makedirs(gist_save)
  files = os.listdir(folder)

  mat = {}
  for file_idx in range(len(files)):
    infile = files[file_idx]
    mat[infile[:-4]] = np.transpose(scipy.io.loadmat(os.path.join(folder,infile))['g'])

  cPickle.dump(mat, open(os.path.join(gist_save, 'features'),'w'))
