"""
Take half of the detections, randomly, and write out to disk.
"""

from common_imports import *
import synthetic.config as config

def half_csc(dets_table):
  img_inds = np.unique(dets_table.subset_arr('img_ind'))
  all_dets = []
  for img_ind in img_inds:
    dets_for_img = dets_table.filter_on_column('img_ind',img_ind).arr
    if dets_for_img.shape[0]>0:
      # randomly select some of the dets
      p = 0.5
      take_ind = np.random.rand(dets_for_img.shape[0],1)>=p
      if np.any(take_ind):
        all_dets.append(dets_for_img[take_ind[:,0],:])
  return Table(np.concatenate(all_dets,0), dets_table.cols, name='csc_half')

if __name__ == '__main__':
  for sett in ['train', 'val', 'trainval']:
    data = np.load('/Users/sergeyk/research/object_detection/synthetic/results/ext_dets/full_pascal_%s_csc_default.npy'%sett)[()]
    d = half_csc(data)
    np.save('/Users/sergeyk/research/object_detection/synthetic/results/ext_dets/full_pascal_%s_csc_half.npy'%sett, d)
