from mako.template import Template
from skvisutils import Dataset
from skvisutils import BoundingBox

class Evaluation(object):
  """
  Class to output evaluations of detections.
  Has to be initialized with a DatasetPolicy to correctly set the paths.
  """
  MIN_OVERLAP = 0.5
  TIME_INTERVALS = 12

  def __init__(self,dataset_policy=None,dataset=None,name='default'):
    """
    Must have either dataset_policy or dataset and name.
    If dataset_policy is given, dataset and name are ignored.
    """
    assert(dataset_policy or (dataset and name))

    if dataset_policy:
      self.dp = dataset_policy
      self.dataset = dataset_policy.dataset
      self.name = dataset_policy.get_config_name()
    else:
      self.dataset = dataset
      self.name = name

    self.time_intervals = Evaluation.TIME_INTERVALS
    self.min_overlap = Evaluation.MIN_OVERLAP

    # Determine filenames and create directories
    self.results_path = config.get_evals_dp_dir(self.dp)
    self.det_apvst_data_fname = os.path.join(self.results_path, 'det_apvst_table.npy')
    self.det_apvst_data_whole_fname = os.path.join(self.results_path, 'det_apvst_table_whole.npy')
    self.cls_apvst_data_whole_fname = os.path.join(self.results_path, 'cls_apvst_table_whole.npy')

    self.det_apvst_png_fname = os.path.join(self.results_path, 'det_apvst.png')
    self.det_apvst_png_whole_fname = os.path.join(self.results_path, 'det_apvst_whole.png')
    self.cls_apvst_png_whole_fname = os.path.join(self.results_path, 'cls_apvst_whole.png')

    self.dashboard_filename = os.path.join(self.results_path, 'dashboard_%s.html')
    self.wholeset_aps_filename = os.path.join(self.results_path, 'aps_whole.txt')
    wholeset_dirname = ut.makedirs(os.path.join(self.results_path, 'wholeset_detailed'))
    self.pr_whole_png_filename = os.path.join(wholeset_dirname, 'pr_whole_%s.png')
    self.pr_whole_txt_filename = os.path.join(wholeset_dirname, 'pr_whole_%s.txt')
    
  ##############
  # AP vs. Time
  ##############
  def evaluate_vs_t(self,dets=None,clses=None,plot=True,force=False):
    """
    Evaluate detections and classifications in the AP vs Time regime,
    and write out plots to canonical places.
    This one is averaging per-image.
    Return two tables: one for detection and one for classification evals.
    """
    bounds = self.dp.bounds if self.dp and self.dp.bounds else None

    dets_table = None
    clses_table = None
    print self.det_apvst_data_fname
    if os.path.exists(self.det_apvst_data_fname) and not force:
      if comm_rank==0:
        dets_table = np.load(self.det_apvst_data_fname)[()]
    else:
      if not dets:
        dets,clses,samples = self.dp.run_on_dataset(force=True)

      # do this now to save time in the inner loop later
      gt_for_image_list = []
      img_dets_list = []
      gt = self.dataset.get_det_gt(with_diff=True)
      
      for img_ind,image in enumerate(self.dataset.images):
        gt_for_image_list.append(gt.filter_on_column('img_ind',img_ind))
        if dets.arr == None:
          detections = Table() 
        else:
          detections = dets.filter_on_column('img_ind',img_ind)
        img_dets_list.append(detections)
      
      if dets.arr == None:
        points = np.zeros(self.time_intervals)
      else:
        all_times = dets.subset_arr('time')
        points = self.determine_time_points(all_times,bounds)
      num_points = points.shape[0]
      det_arr = np.zeros((num_points,3))
      cls_arr = np.zeros((num_points,3))
      for i in range(comm_rank,num_points,comm_size):
        tt = ut.TicToc().tic()
        point = points[i]
        det_aps = []
        cls_aps = []
        num_dets = 0
        for img_ind,image in enumerate(self.dataset.images):
          gt_for_image = gt_for_image_list[img_ind]
          img_dets = img_dets_list[img_ind] 
          if img_dets.cols == None:
            dets_to_this_point= img_dets
            det_ap = 0
          else:
            dets_to_this_point = img_dets.filter_on_column('time',point,operator.le)          
            num_dets += dets_to_this_point.shape[0]
            #det_ap,_,_ = self.compute_det_pr(dets_to_this_point, gt_for_image)
            det_ap = self.compute_det_map(dets_to_this_point, gt_for_image, det_perspective=False)
          det_aps.append(det_ap)
        det_arr[i,:] = [point,np.mean(det_aps),np.std(det_aps)]
        print("Calculating AP (%.3f) of the %d detections up to %.3fs took %.3fs"%(
          np.mean(det_aps),num_dets,point,tt.qtoc()))
      det_arr_all = None
      if comm_rank == 0:
        det_arr_all = np.zeros((num_points,3))
      safebarrier(comm)
      comm.Reduce(det_arr,det_arr_all)
      if comm_rank==0:
        dets_table = Table(det_arr_all, ['time','ap_mean','ap_std'], self.name)
        np.save(self.det_apvst_data_fname,dets_table)
    # Plot the table
    if plot and comm_rank==0:
      try:
        Evaluation.plot_ap_vs_t([dets_table],self.det_apvst_png_fname, bounds, True, force)
      except:
        print("Could not plot")
    safebarrier(comm)
    return dets_table

  def determine_time_points(self,all_times,bounds):
    """"
    Helper function shared by evaluate_vs_t and evaluate_vs_t_whole.
    all_times is a ndarray of times.
    """
    # determine time sampling points
    points = ut.importance_sample(all_times,self.time_intervals)
    # make sure bounds are included in the sampling points if given
    if bounds:
      points = np.sort(np.array(points.tolist() + bounds))
    else:
      # or at least add the 0 point if no bounds given
      points = np.hstack((0,points))
    return points

  def evaluate_vs_t_whole(self,dets=None,clses=None,plot=True,force=False):
    """
    Evaluate detections in the AP vs Time regime and write out plots to
    canonical places.
    - dets must be on self.dataset
    This version evaluates on the whole dataset instead of averaging per-image
    performances, and so gets rid of error bars.
    """
    bounds = self.dp.bounds if self.dp and self.dp.bounds else None
    
    dets_table = None
    clses_table = None
    if os.path.exists(self.det_apvst_data_whole_fname) and \
       os.path.exists(self.cls_apvst_data_whole_fname) and not force:
      if comm_rank==0:
        dets_table = np.load(self.det_apvst_data_whole_fname)[()]
        clses_table = np.load(self.cls_apvst_data_whole_fname)[()]
    else:
      if not dets:
        dets,clses,samples = self.dp.run_on_dataset()
      if None == dets.arr:
        all_times = clses.subset_arr('time')
      else:
        all_times = dets.subset_arr('time')
      points = self.determine_time_points(all_times,bounds)
      num_points = points.shape[0]
      cls_gt = self.dataset.get_cls_ground_truth(with_diff=False)

      det_arr = np.zeros((num_points,2))
      cls_arr = np.zeros((num_points,2))
      for i in range(comm_rank,num_points,comm_size):
        tt = ut.TicToc().tic()
        point = points[i]
        if dets.shape[0] < 1:
          dets_to_this_point = dets
          ap = 0
        else:
          dets_to_this_point = dets.filter_on_column('time',point,operator.le)
          if True:
            img_inds = np.unique(dets_to_this_point.subset_arr('img_ind').astype(int))
            gt = self.dataset.get_det_gt(with_diff=True).copy()
            gt.arr = gt.arr[img_inds,:]
            #ap,_,_ = self.compute_det_pr(dets_to_this_point,gt)
            ap = self.compute_det_map(dets_to_this_point,gt,det_perspective=False)
          else:
            # TODO: fuck it! takes too long
            ap = 0
        det_arr[i,:] = [point,ap]

        # go through the per-image classifications and only keep the latest time
        # for each cut-off time, accumulating the per-image results
        clses_to_this_point_all_imgs = []
        for img_ind in range(0,self.dataset.num_images()):
          img_clses = clses.filter_on_column('img_ind',img_ind)
          clses_to_this_point = img_clses.filter_on_column('time',
            point,operator.le)
          if clses_to_this_point.shape[0]>0:
            clses_to_this_point = clses_to_this_point.sort_by_column('time')
            clses_to_this_point_all_imgs.append(clses_to_this_point.arr[-1,:])
        
        # turn into a Table, compute the mAP, and store it
        clses_to_this_point_all_imgs = Table(
          arr=np.array(clses_to_this_point_all_imgs), cols=clses.cols)
        cls_arr[i,:] = \
          [point, self.compute_cls_map(clses_to_this_point_all_imgs, cls_gt)]
        
        if not dets_to_this_point.arr == None and not det_arr==None:
          print("Calculating AP (%.3f) of the %d detections up to %.3fs took %.3fs"%(
            det_arr[i,1],dets_to_this_point.shape[0],point,tt.qtoc()))
        else:
          print("We are in gist, calculating AP of detection makes no sense.")
      det_arr_all = None
      cls_arr_all = None
      if comm_rank == 0:
        det_arr_all = np.zeros((num_points,2))
        cls_arr_all = np.zeros((num_points,2))
      safebarrier(comm)
      comm.Reduce(det_arr,det_arr_all)
      comm.Reduce(cls_arr,cls_arr_all)
      if comm_rank==0:
        dets_table = Table(det_arr_all, ['time','ap'], self.name)
        np.save(self.det_apvst_data_whole_fname,dets_table)
        clses_table = Table(cls_arr_all, ['time','ap'], self.name)
        np.save(self.cls_apvst_data_whole_fname,clses_table)
    # Plot the table
    if plot and comm_rank==0:
      try:
        Evaluation.plot_ap_vs_t([dets_table],self.det_apvst_png_whole_fname, bounds, True, force)
        Evaluation.plot_ap_vs_t([clses_table],self.cls_apvst_png_whole_fname, bounds, True, force)
      except:
        print("Could not plot")
    return (dets_table,clses_table)

  @classmethod
  def compute_auc(cls,times,vals,bounds=None):
    """
    Return the area under the curve of vals vs. times, within given bounds.
    """
    if bounds:
      valid_ind = np.flatnonzero((times>=bounds[0]) & (times<=bounds[1]))
      times = times[valid_ind]
      vals = vals[valid_ind]
    auc = np.trapz(vals,times)
    return auc

  @classmethod
  def plot_ap_vs_t(cls, tables, filename, all_bounds=None, with_legend=True, force=False, plot_infos=None):
    """
    Take list of Tables containing AP vs. Time information.
    Bounds are given as a list of the same length as tables, or not at all.
    If table.cols contains 'ap_mean' and 'ap_std' and only one table is given,
    plots error area around the curve.
    If bounds are not given, uses the min and max values for each curve.
    If only one bounds is given, uses that for all tables.
    The legend is assembled from the .name fields in the Tables.
    Save plot to given filename.
    Does not return anything.
    """
    if os.path.exists(filename) and not force:
      print("Plot already exists, not doing anything")
      return
    plt.clf()
    colors = ['black','orange','#4084ff','purple']
    styles = ['-','--','-.','-..']
    prod = [x for x in itertools.product(colors,styles)]
    none_bounds = [None for table in tables]
    
    # TODO: oooh that's messy
    if np.all(all_bounds==none_bounds):
      None
    elif not all_bounds:
      all_bounds = none_bounds
    elif not isinstance(all_bounds[0], types.ListType):
      all_bounds = [all_bounds for table in tables]
    else:
      assert(len(all_bounds)==len(tables))
    
    labels = []
    for i,table in enumerate(tables):
      print("Plotting %s"%table.name)
      bounds = all_bounds[i]
      
      if not plot_infos == None and "line" in plot_infos[i]:
        style = str(plot_infos[i]["line"])
      else:
        style = prod[i][1]
      if not plot_infos == None and "color" in plot_infos[i]:
        color = str(plot_infos[i]["color"])
      else:
        color = prod[i][0]
      times = table.subset_arr('time')
      if 'ap_mean' in table.cols and 'ap_std' in table.cols:
        vals = table.subset_arr('ap_mean')
        stdevs = table.subset_arr('ap_std')
        if len(tables)==1:
          plt.fill_between(times,vals-stdevs,vals+stdevs,color='#4084ff',alpha=0.3)
      else:
        vals = table.subset_arr('ap')
      auc = Evaluation.compute_auc(times,vals,bounds)/float(bounds[1]-bounds[0])

      high_bound_val = vals[-1]
      
#      if bounds != None:
#        high_bound_val = vals[times.tolist().index(bounds[1])]
      
      if not plot_infos == None and "label" in plot_infos[i]:
        label = '%s: (%.4f, %.4f)'%(str(plot_infos[i]["label"]), auc, high_bound_val)
      else:
        label = "(%.4f, %.4f) %s"%(auc,high_bound_val,table.name)
      
      labels.append(label)
      plt.plot(times, vals, style,
          linewidth=3,color=color,label=label)

      # draw vertical lines at bounds, if given
      if bounds:
        low_bound_val = vals[times.tolist().index(bounds[0])]
        plt.vlines(bounds[0],0,low_bound_val,alpha=0.8)
        plt.vlines(bounds[1],0,high_bound_val,alpha=0.8)
    if with_legend:
      plt.legend(loc='upper left')
      leg = plt.gca().get_legend()
      ltext = leg.get_texts()
      plt.setp(ltext, fontsize='small')
    plt.xlabel('Time',size=14)
    plt.ylabel('AP',size=14)
    plt.ylim(0,0.8)
    plt.xlim(0,20)
    plt.grid(True,'major')
    plt.grid(True,'minor')

    # save text file with the label information and save the figure
    txt_filename = os.path.splitext(filename)[0]+'.txt'
    with open(txt_filename,'w') as f:
      f.write(str(labels))
    plt.savefig(filename)

  def evaluate_detections_whole(self,dets=None,force=False):
    """
    Output detection evaluations over the whole dataset in all formats:
    - multi-class (one PR plot)
    - per-class PR plots (only detections of that class are considered)
    """
    if not dets:
      assert(self.dp != None)
      dets,clses,samples = self.dp.run_on_dataset()

    # Per-Class
    num_classes = len(self.dataset.classes)
    dist_aps = np.zeros(num_classes)
    for cls_ind in range(comm_rank, num_classes, comm_size):
      cls = self.dataset.classes[cls_ind] 
      cls_dets = dets.filter_on_column('cls_ind',cls_ind)
      cls_gt = self.dataset.get_get_gt_for_class(cls,with_diff=True)
      dist_aps[cls_ind] = self.compute_and_plot_pr(cls_dets, cls_gt, cls, force)
    aps = None
    if comm_rank==0:
      aps = np.zeros(num_classes)
    safebarrier(comm)
    comm.Reduce(dist_aps,aps)

    # the rest is done by rank==0
    if comm_rank == 0:
      # Multi-class
      gt = self.dataset.get_det_gt(with_diff=True)
      filename = os.path.join(self.results_path, 'pr_whole_multiclass')
      if force or not os.path.exists(filename):
        print("Evaluating %d dets in the multiclass setting..."%dets.shape[0])
        ap_mc = self.compute_and_plot_pr(dets, gt, 'multiclass')

      # Write out the information to a single overview file
      with open(self.wholeset_aps_filename, 'w') as f:
        f.write("Multiclass: %.3f\n"%ap_mc)
        f.write(','.join(self.dataset.classes)+'\n')
        f.write(','.join(['%.3f'%ap for ap in aps])+'\n')

      # Assemble everything in one HTML file, the Dashboard
      filename = self.pr_whole_png_filename%'all'
      names = list(self.dataset.classes)
      names.append('multiclass')
      aps = aps.tolist()
      aps.append(ap_mc)
      template = Template(filename=config.eval_template_filename)
      filename = self.dashboard_filename%'whole'
      names = list(self.dataset.classes)
      names.append('avg')
      aps.append(np.mean(aps))
      with open(filename, 'w') as f:
        f.write(template.render(
          names=names, aps=aps))
    safebarrier(comm)

  def compute_and_plot_pr(self, dets, gt, name, force=False):
    """
    Helper function. Compute the precision-recall curves from the detections
    and ground truth and output them to files.
    Return ap.
    """
    filename = self.pr_whole_txt_filename%name
    if force or not os.path.exists(filename):
      [ap,rec,prec] = self.compute_det_pr(dets, gt)
      try:
        self.plot_pr(ap,rec,prec,name,self.pr_whole_png_filename%name)
      except:
        None
      with open(filename, 'w') as f:
        f.write("%f\n"%ap)
        for i in range(np.shape(rec)[0]):
          f.write("%f %f\n"%(rec[i], prec[i]))
    else:
      with open(filename) as f:
        ap = float(f.readline())
    return ap

  def plot_pr(self, ap, rec, prec, name, filename, force=False):
    """Plot the Precision-Recall curve, saving png to filename."""
    if os.path.exists(filename) and not force:
      print("plot_pr: not doing anything as file exists")
      return
    label = "%s: %.3f"%(name,ap)
    plt.clf()
    plt.plot(rec,prec,label=label,color='black',linewidth=5)
    plt.xlim(0,1)
    plt.ylim(0,1)
    plt.legend(loc='lower left')
    plt.xlabel('Recall',size=16)
    plt.ylabel('Precision',size=16)
    plt.grid(True)
    plt.savefig(filename)
  
  ##############################
  # Computation of Precision-Recall and Average Precision
  ##############################
  def compute_det_map(self, dets, gt, det_perspective=False):
    """
    Compute average of per-class APs for the given detections.
    If det_perspective == True, only classes present in dets are evaluated.
    Otherwise, all classes present in gt are evaluated.
    """
    aps = []
    if det_perspective:
      unique_cls_inds = np.unique(dets.subset_arr('cls_ind'))
    else:
      unique_cls_inds = np.unique(gt.subset_arr('cls_ind'))

    # NOTE: the values are from the training dataset
    values = self.dp.train_dataset.values

    for cls_ind in unique_cls_inds:
      d = dets.filter_on_column('cls_ind',cls_ind)
      g = gt.filter_on_column('cls_ind',cls_ind)
      ap,_,_ = self.compute_det_pr(d,g)
      aps.append(ap*values[cls_ind])
    # Since not all classes could have been used, we should normalize
    # the weighted sum by the total value
    aps /= np.sum([values[cls_ind] for cls_ind in unique_cls_inds])
    return np.sum(aps)

  def compute_det_pr(self, dets, gt):
    pr_and_hard_neg = self.compute_det_pr_and_hard_neg(dets, gt)
    return pr_and_hard_neg[:3]
  
  def compute_det_hard_neg(self, dets, gt):
    pr_and_hard_neg = self.compute_det_pr_and_hard_neg(dets, gt)
    # BUG TODO: shouldn't this be 3?
    return pr_and_hard_neg[4]
    
  @classmethod
  def compute_det_pr_and_hard_neg(cls, dets, gt):
    """
    Take Table of detections and Table of ground truth.
    Ground truth can be for a single image or a whole dataset
    and can contain either all classes or just one class (but the cls_ind col
    must be present in either case).
    Depending on these decisions, the meaning of the PR evaluation is
    different.
    In particular, if gt is for a single class but dets are for multiple
    classes, there will be a lot of false positives!
    NOTE: modifies dets in-place (sorts by score)
    Return ap, recall, and precision vectors as tuple.
    """
    gt = gt.copy()

    # if dets or gt are empty, return 0's
    nd = dets.arr.shape[0]
    if nd < 1 or gt.shape[0] < 1:
      ap = 0
      rec = np.array([0])
      prec = np.array([0])
      return (ap,rec,prec)
    tt = ut.TicToc().tic()

    # augment gt with a column keeping track of matches
    cols = list(gt.cols) + ['matched']
    arr = np.zeros((gt.arr.shape[0],gt.arr.shape[1]+1))
    arr[:,:-1] = gt.arr
    gt = Table(arr,cols)

    # sort detections by confidence
    dets.sort_by_column('score',descending=True)

    # match detections to ground truth objects
    npos = gt.filter_on_column('diff',0).shape[0]
    tp = np.zeros(nd)
    fp = np.zeros(nd)
    hard_neg = np.zeros(nd)
    for d in range(nd):
      if tt.qtoc() > 15:
        print("... on %d/%d dets"%(d,nd))
        tt.tic()

      det = dets.arr[d,:]

      # find ground truth for this image
      if 'img_ind' in gt.cols:
        img_ind = det[dets.ind('img_ind')]
        inds = gt.arr[:,gt.ind('img_ind')] == img_ind
        gt_for_image = gt.arr[inds,:]
      else:
        gt_for_image = gt.arr
      
      if gt_for_image.shape[0] < 1:
        # this can happen if we're passing ground truth for a class
        # false positive due to detection in image that does not contain the class
        fp[d] = 1 
        hard_neg[d] = 1
        continue

      # find the maximally overlapping ground truth element for this detection
      overlaps = BoundingBox.get_overlap(gt_for_image[:,:4],det[:4])
      jmax = overlaps.argmax()
      ovmax = overlaps[jmax]

      # assign detection as true positive/don't care/false positive
      if ovmax >= cls.MIN_OVERLAP:
        if not gt_for_image[jmax,gt.ind('diff')]:
          is_matched = gt_for_image[jmax,gt.ind('matched')]
          if is_matched == 0:
            if gt_for_image[jmax,gt.ind('cls_ind')] == det[dets.ind('cls_ind')]:
              # true positive
              tp[d] = 1
              gt_for_image[jmax,gt.ind('matched')] = 1
            else:
              # false positive due to wrong class
              fp[d] = 1
              hard_neg[d] = 1
          else:
            # false positive due to multiple detection
            # this is still a correct answer, so not a hard negative
            fp[d] = 1
        else:
          None
          # NOT a false positive because object is difficult!
      else:
        # false positive due to not matching any ground truth object
        fp[d] = 1
        hard_neg[d] = 1
      # NOTE: this is very important: otherwise, gt.arr does not get the
      # changes we make to gt_for_image
      if 'img_ind' in gt.cols:
        gt.arr[inds,:] = gt_for_image

    ap,rec,prec = cls.compute_rec_prec_ap(tp,fp,npos)
    return (ap,rec,prec,hard_neg)

  @classmethod
  def compute_cls_map(cls,clses,gt):
    """
    Compute classification mean AP.
    Take Table of classifications and Table of ground truth.
    The results should be for multiple classes, and we average per-class APs.
    There should only be one instance of each img_ind in clses.
    Ground truth should be of the format in Dataset.get_cls_ground_truth().
    NOTE: ground truth must be for the whole dataset!
    """
    if not clses.shape[0]>0:
      return 0
    if 'img_ind' in clses.cols:
      img_inds = clses.subset_arr('img_ind')
      assert(len(img_inds)==len(np.unique(np.array(img_inds))))
      gt = gt.row_subset(img_inds)
    aps = []
    for col in gt.cols:
      ap,rec,prec=cls.compute_cls_pr(clses.subset_arr(col),gt.subset_arr(col))
      aps.append(ap)
    return np.mean(aps)

  @classmethod
  def compute_cls_pr(cls,confidences,gt):
    """
    Take vector of classification confidences and vector of ground truth.
    Return ap,recall,and precision vectors as tuple.
    """
    ind = np.argsort(-confidences, axis=0)
    tp = gt[ind]==True
    fp = gt[ind]==False
    npos = np.sum(gt==True)
    return cls.compute_rec_prec_ap(tp,fp,npos)

  @classmethod
  def compute_rec_prec_ap(cls,tp,fp,npos):
    fp=np.cumsum(fp)
    tp=np.cumsum(tp)
    # NOTE: this will give a warning if dividing by 0, but it's only a warning
    # and the behavior is correct (dividing by 0 gives 0)
    if npos==0:
      rec = np.zeros(tp.shape)
    else:
      rec=1.*tp/npos
    prec=1.*tp/(fp+tp)
    prec = np.nan_to_num(prec)
    ap = cls.compute_ap(rec,prec)
    return (ap,rec,prec)

  @classmethod
  def compute_ap(cls,rec,prec):
    """
    Takes recall and precision vectors and computes piecewise area under the
    curve.
    """
    assert(np.all(rec>=0) and np.all(rec<=1))
    assert(np.all(prec>=0) and np.all(prec<=1))
    mprec = np.hstack((0,prec,0))
    mrec = np.hstack((0,rec,1))
    # make sure prec is monotonically decreasing
    for i in range(len(mprec)-1,0,-1):
      mprec[i-1]=max(mprec[i-1],mprec[i])
    # find piecewise area under the curve
    i = np.add(np.nonzero(mrec[1:] != mrec[0:-1]),1)
    ap = np.sum((mrec[i]-mrec[np.subtract(i,1)])*mprec[i])
    return ap  
