"""
Methods to train a FastInf MRF.
"""

from synthetic.common_imports import *
from synthetic.common_mpi import *

import synthetic.config as config
import subprocess as subp

from synthetic.dataset import Dataset
from synthetic.gist_classifier import gist_classify_dataset
from matplotlib.pylab import *
import argparse
from matplotlib.pyplot import hist
from synthetic.ext_detector_regions import RegionModel

class FastinfDiscretizer(object):
  def __init__(self,d,suffix):
    # For a given setting return bounds as num_bins x num_cols
    self.bounds = np.loadtxt(config.get_mrf_bound_filename(d, suffix),ndmin=2)
    
  def discretize_value(self, val, clf_idx):
    """
    For d, suffix discretize val for all 20 classes. 
    Returns discretized value for clf_idx
    """
    discr_val = ut.determine_bin(np.tile(val, (1,self.bounds.shape[1]))[0,:], self.bounds, asInt=True)
    return discr_val.astype(int)[clf_idx]
  
def discretize_table(table, num_bins, asInt=True, linsp=False):
  """
  discretize the given table and also return the bounds for the column 
  discretization. as num_bins x num_cols
  """
  all_bounds = np.zeros((num_bins+1, table.shape[1]))
  new_table = np.zeros(table.shape)
  
  for coldex in range(table.shape[1]):
    col = table[:, coldex]
     
    if np.where(col==col[0])[0].shape[0] == col.shape[0]:
      bounds = (np.arange(num_bins+1)/num_bins)
    else:
      if not linsp:
        bounds = ut.importance_sample(col, num_bins+1)
      else:
        bounds = np.linspace(np.min(col), np.max(col), num_bins+1)
      
    all_bounds[:, coldex] = bounds
      
    new_table[:, coldex] = ut.determine_bin(col, bounds, asInt)
  if asInt:    
    return (all_bounds, new_table.astype(int))
  else:
    return (all_bounds, new_table)

def write_out_mrf(table, num_bins, filename, data_filename, second_table=None, pairwise=True, force=True):
  """
  Again we assume the table to be of the form displayed below.
  """
   
  num_vars = table.shape[1]/2
  wm = open(filename, 'w')
  modelfile = config.get_mrf_model(num_vars)
  print modelfile, os.path.exists(modelfile)
  
  if force or not os.path.exists(modelfile):
    #===========
    #= Model
    #===========  
    # ===========Variables==========
    wm.write('@Variables\n')
    for i in range(num_vars):
      wm.write('var%d\t2\n'%i)
    for i in range(num_vars):
      wm.write('var%d\t%d\n'%(i+num_vars, num_bins))
      
    if not second_table == None:
      for i in range(num_vars):
        wm.write('var%d\t%d\n'%(i+2*num_vars, num_bins))    
    wm.write('@End\n')
    wm.write('\n')
    
    # ===========Cliques============
    wm.write('@Cliques\n')
    if not pairwise:
      # top clique:
      wm.write('cl0\t%d'%num_vars)
      wm.write('\t')
      for i in range(num_vars):
        wm.write(' %d'%i)
      wm.write('\t%d\t'%num_vars)
      for i in range(num_vars):
        wm.write(' %d'%(i+1))
      wm.write('\n')
    else:
      combs = list(itertools.combinations(range(num_vars), 2))
      num_combs = len(combs)    
      for idx, comb in enumerate(combs):
        # neighboring cliques:
        neighs = []
        for i, c in enumerate(combs):
          if not c==comb:
            if c[0] in comb or c[1] in comb:
              neighs.append(i)
  
        wm.write('cl%d\t2\t%d %d\t%d\t'%(idx, comb[0], comb[1], len(neighs)))
        for n in neighs:
          wm.write('%d '%n)
        wm.write('\n')
    #pairwise cliques
    for l in range(num_vars):
      neighs = []
      for i, c in enumerate(combs):
        #if not c==[i, i+num_vars]:
        if l in c:
          neighs.append(i)
      wm.write('cl%d\t2\t%d %d\t%d\t'%(l+1+idx, l, l+num_vars, len(neighs)))
      for n in neighs:
        wm.write('%d '%n)
      wm.write('\n')
      
    if not second_table == None:
      for l in range(num_vars):
        neighs = []
        for i, c in enumerate(combs):
          #if not c==[i, i+num_vars]:
          if l in c:
            neighs.append(i)
        wm.write('cl%d\t2\t%d %d\t%d\t'%(l+1+idx+num_vars, l, l+2*num_vars, len(neighs)))
        for n in neighs:
          wm.write('%d '%n)
        wm.write('\n')
    wm.write('@End\n')
    wm.write('\n')
    num_cliques = l+2+idx
    if not second_table == None:
      num_cliques += num_vars
    print num_cliques
      
    # ===========Measures==========
    # Well, there is a segfault if these are empty :/
    wm.write('@Measures\n')
    if not pairwise:
      wm.write('mes0\t%d\t'%(num_vars))
      for _ in range(num_vars):
        wm.write('2 ')
      wm.write('\t')
      for _ in range(2**num_vars):
        wm.write('.1 ')
      wm.write('\n')
    else:
      for j in range(num_combs):
        wm.write('mes%d\t2\t2 2\t.1 .1 .1 .1\n'%j)
      
    for i in range(num_vars):
      wm.write('mes%d\t2\t2 %d'%(i+j+1, num_bins))
      wm.write('\t')
      for _ in range(num_bins*2):
        wm.write('.1 ')
      wm.write('\n')
    if not second_table == None:
      for i in range(num_vars):
        wm.write('mes%d\t2\t2 %d'%(i+j+1+num_vars, num_bins))
        wm.write('\t')
        for _ in range(num_bins*2):
          wm.write('.1 ')
        wm.write('\n')  
    wm.write('@End\n')
    wm.write('\n')
    
    # ===========CliqueToMeasure==========
    wm.write('@CliqueToMeasure\n')
    for i in range(num_cliques):
      wm.write('%(i)d\t%(i)d\n'%dict(i=i))  
    wm.write('@End\n')
    
    wm.close()
    
    # copy to modelfile
    os.system('cp %s %s'%(filename,modelfile))
  else:
    print 'load model...'
    # copy from modelfile
    os.system('cp %s %s'%(modelfile,filename))
  
  print 'reformat data'
  #===========
  #= Data
  #===========
  wd = open(data_filename, 'w')
  if not second_table == None:
    table = np.hstack((table, second_table))
  for rowdex in range(table.shape[0]):
    wd.write('( ')
    for i in range(table.shape[1]):
      wd.write('%.2f '%table[rowdex, i])    
    wd.write(')\n')
  wd.close()
  
def create_meassurement_table(num_clss, func):
  """
  Create table containing all measurements in format
     __________
    /          \ 
   A --- B --- C
   |     |     |
   1     2     3
   
  => [A, B, C, 1, 2, 3]
  """
  table = np.zeros((2**num_clss, num_clss*2))
  
  # do a binary counter to fill up this table (as a ripple counter)
  assignments = np.array(np.zeros((num_clss,)))
  for i in range(2**num_clss):
    classif = func(assignments)
    table[i,:] = np.hstack((assignments, classif))
    
    # done?
    go_on = not np.sum(assignments) == num_clss
    # count up
    assignments[-1] += 1    
    # propagate bit up    
    pos = 1
    while go_on:
      if assignments[-pos] == 2:
        assignments[-pos] = 0
        pos += 1
        assignments[-pos] += 1
      else:
        go_on = False      
   
  return table

def execute_lbp(filename_mrf, filename_data, filename_out, add_settings=[]):
  cmd = ['../fastInf/build/bin/learning', '-i', filename_mrf, 
                         '-e', filename_data, '-o', filename_out] + add_settings

  timefile = filename_out+'_time'
  tt = ut.TicToc()
  tt.tic()        
  cmd = ' '.join(cmd)
  ut.run_command(cmd)
  w = open(timefile, 'w')
  w.write('%s\ntook %f sec'%(cmd, tt.toc(quiet=True)))
  w.close()
  print 'everything done'
  return 

def c_corr_to_a(num_lines, func):
  assignment = np.zeros((3,))
  table = np.zeros((num_lines, 6))
  for i in range(num_lines):
    rand = np.random.random((4,))
    assignment[0] = rand[0] > .7
    assignment[1] = rand[1] > .5
    if rand[2] > 0.2:
      assignment[2] = assignment[0]
    else:
      assignment[2] = rand[3] > .5
    
    classif = func(assignment)
    table[i,:] = np.hstack((assignment, classif))
  return table

def store_bound(d, suffix, bounds):
  bound_file = config.get_mrf_bound_filename(d, suffix)
  np.savetxt(bound_file, bounds)  
  print bound_file

def create_regioned_table(rm, gt, images, num_classes):
  num_regions = rm.get_number_regions()
  num_images = len(images)
  region_table = np.zeros((num_images, num_regions*num_classes))
  img_inds = np.unique(gt.subset_arr('img_ind'))
  for ind in img_inds:
    gt_img = gt.filter_on_column('img_ind', ind)
    img = images[int(ind)]
    for row in gt_img.arr:
      region_id = rm.which_region(img, row[gt.cols.index('x')],row[gt.cols.index('y')],  
                                  row[gt.cols.index('w')],row[gt.cols.index('h')])
      cls = row[gt.cols.index('cls_ind')]
      region_table[ind, num_regions*cls + region_id] = row[gt.cols.index('score')]

  return region_table

def run_fastinf_different_settings(d, ms, rs, suffixs, num_bins = 5):
  
  settings = list(itertools.product(suffixs, ms, rs))
  table_gt = d.get_cls_ground_truth().arr.astype(int)
  print 'run with a total of %d settings'%len(settings)
  
  for setindx in range(comm_rank, len(settings), comm_size):
    second_table = None
    setin = settings[setindx]
    suffix = setin[0]
    m = str(setin[1])
    r1 = str(setin[2])
    
    print 'node %d runs %s, m=%s, r1=%s'%(comm_rank, suffix, m, r1)

    filename = config.get_fastinf_mrf_file(d, suffix)
    data_filename = config.get_fastinf_data_file(d, suffix)
    
    if suffix == 'perfect':      
      table = np.hstack((table_gt, table_gt))
      bounds = np.tile(np.linspace(0, 1, num_bins+1),(table_gt.shape[1],1))
      print bounds
      
    elif suffix == 'GIST':
      table = gist_classify_dataset(d)   
      bounds, discr_table = discretize_table(table, num_bins)  
      table = np.hstack((table_gt, discr_table))
      
    elif suffix == 'CSC':
      filename_csc = os.path.join(config.get_ext_dets_foldname(d),'table')
      print filename_csc
      if not os.path.exists(filename_csc):
        raise RuntimeWarning('The csc classification could not be loaded from %s'%filename_csc)
      orig_table = cPickle.load(open(filename_csc,'r'))
      if isinstance(orig_table, Table):
        orig_table = orig_table.arr[:,:-1]
      bounds, discr_table = discretize_table(orig_table, num_bins)
      table = np.hstack((table_gt, discr_table))
      
    elif suffix == 'CSC_regions':
      rm = RegionModel("1big_2small", 0.5)
      detector = 'csc_default'
      from synthetic.dataset_policy import DatasetPolicy
      orig_table = DatasetPolicy.load_ext_detections(d, detector)            
      gt = d.get_det_gt().copy()
      # we need to spice up the gt by a score of 1 for each class (results in less code)
      gt.cols.append('score')
      gt.arr = np.hstack((gt.arr, np.ones((gt.shape[0], 1))))  
      table_gt_region = create_regioned_table(rm, gt, d.images, len(d.classes))
      # At this point we need to split them for the different regions
      orig_table_region = create_regioned_table(rm, orig_table, d.images, len(d.classes))
      
      bounds, discr_table_region = discretize_table(orig_table_region, num_bins)
      table = np.hstack((table_gt_region, discr_table_region))
      
    elif suffix == 'GIST_CSC':
      filename_csc = os.path.join(config.get_ext_dets_foldname(d),'table')
      if not os.path.exists(filename_csc):
        raise RuntimeWarning('The csc classification could not be loaded from %s'%filename_csc)
      orig_table = cPickle.load(open(filename_csc,'r'))
      if isinstance(orig_table, Table):
        orig_table = orig_table.arr[:,:-1]
      bounds, discr_table = discretize_table(orig_table, num_bins)      
      table = np.hstack((table_gt, discr_table))
      store_bound(d, 'CSC', bounds)
      
      second_table = gist_classify_dataset(d)      
      sec_bounds, second_table = discretize_table(second_table, num_bins)      
      store_bound(d, 'GIST', sec_bounds)
      
      full_bound = np.hstack((sec_bounds, bounds))
      store_bound(d, 'GIST_CSC', full_bound)
    
    if not suffix == 'GIST_CSC':
      store_bound(d, suffix, bounds)
    
    print 'set up table on %d, write out mrf for %s, m=%s, r1=%s'%(comm_rank, suffix, m, r1)   
      
    write_out_mrf(table, num_bins, filename, data_filename, second_table=second_table)
    
    add_sets = ['-m',m]
    if not r1 == '':
      add_sets += ['-r1', r1]
          
    if not second_table == None:
      sec_bound_file = '%s_secbounds'%filename
      for s in add_sets:
        sec_bound_file += '_'+s
      np.savetxt(sec_bound_file, sec_bounds)
      
    print '%d start running lbp for %s, m=%s, r1=%s'%(comm_rank, suffix, m, r1)
    
    filename_out = config.get_fastinf_res_file(d, suffix, m, r1)
    execute_lbp(filename, data_filename, filename_out, add_settings=add_sets)
  
def simply_run_it(dataset):
  parser = argparse.ArgumentParser(
    description="Run fastInf experiments.")

  parser.add_argument('-m',type=int,
    default=0,
    choices=[0,1,2,3,4,5],
    help="""optimization method 0-FR, 1-PR, 2-BFGS, 3-STEEP, 4-NEWTON, 5-GRADIENT (0).""")

  parser.add_argument('-r',type=int,
    default=1,
    help="""parameter of L1 regularization.""")
  
  args = parser.parse_args()
  
  m = args.m
  r = args.r
  d = Dataset(dataset)
  suffixs = ['CSC_regions']
  run_fastinf_different_settings(d, [m], [r], suffixs)

if __name__=='__main__':
  dataset = 'full_pascal_trainval'
  simply_run_it(dataset)
