from timely.common_imports import *
import timely.config as config
from timely.image import Image

class Dataset(object):
  """
  Representation of a dataset, with methods to construct from different sources
  of data, get ground truth, and construct sets of train/test data.
  """

  def __init__(self, name=None, force=False):
    self.classes = []
    self.images = []
    self.name = name
    if re.search('pascal', name):
      self.load_from_pascal(name,force)
    elif name == 'test_data1':
      self.load_from_json(config.test_data1)
    elif name == 'test_data2':
      self.load_from_json(config.test_data2)
    elif name == 'synthetic':
      self.generate_synthetic()
    else:
      print("WARNING: Unknown dataset initialization string, not loading images.")
    self.image_names = [image.name for image in self.images]
    assert(len(self.image_names)==len(np.unique(self.image_names)))
    self.cached_det_ground_truth = {}
    self.set_values('uniform')

  def num_classes(self):
    return len(self.classes)

  def get_ind(self,cls):
    return self.classes.index(cls)

  def __repr__(self):
    return self.get_name()

  def get_name(self):
    return "%s_%s"%(self.name,self.num_images())

  def num_images(self):
    return len(self.images)

  def get_img_ind(self,image):
    return self.images.index(image)

  ###
  # Class Values
  ###
  def set_values(self,mode='uniform'):
    "Set all class values to be uniform or inversely proportional to priors."
    if mode=='uniform':
      self.values = 1.*np.ones(len(self.classes))/len(self.classes)
    elif mode=='inverse_prior':
      gt = self.get_cls_ground_truth(with_diff=False,with_trun=True)
      prior = 1.*gt.sum(0)/gt.shape[0]
      self.values = 1./prior
      self.values /= np.sum(self.values)
    else:
      raise RuntimeError("Unknown mode")

  ###
  # Loaders / Generators
  ###
  def generate_synthetic(self):
    """
    Generate a synthetic dataset of 4 classes that follows some simple
    but strong cooccurence rules.
    """
    num_images = 1000
    choice_probs = {
      (1,0,0,0):2,
      (0,1,0,0):1,
      (0,0,1,0):1,
      (0,0,0,1):1,
      (1,1,0,0):2,
      (1,0,1,0):0,
      (1,0,0,1):0,
      (0,1,1,0):0,
      (0,1,0,1):1,
      (0,0,1,1):2,
      (1,1,1,0):0,
      (1,1,0,1):1,
      (1,0,1,1):1,
      (0,1,1,1):2}
    probs = np.array(choice_probs.values())
    cum_probs = np.cumsum(1.*probs/np.sum(probs))    
    self.classes = ['A','B','C','D']
    for i in range(0,num_images):
      image = Image(100,100,self.classes,str(i))
      choice = np.where(cum_probs>np.random.rand())[0][0]
      objects = []
      for cls_ind,clas in enumerate(choice_probs.keys()[choice]):
        if clas == 1:
          objects.append(np.array([0,0,0,0,cls_ind,0,0]))
      image.objects_table = Table(np.array(objects), Image.columns)
      self.images.append(image)

  def load_from_json(self, filename):
    "Load all parameters of the dataset from a JSON file."
    with open(filename) as f:
      config = json.load(f)
    self.classes = config['classes']
    for data in config['images']:
      self.images.append(Image.load_from_json_data(self.classes,data))

  def load_from_pascal(self, name, force=False):
    """
    Look up the filename associated with the given name.
    Read image names from provided filename, and construct a dataset from the
    corresponding .xml files.
    Save self to disk when loaded for caching purposes.
    If force is True, does not look for cached data when loading.
    """
    tt = ut.TicToc().tic()
    print("Dataset: %s"%name),
    filename = config.get_cached_dataset_filename(name)
    if opexists(filename) and not force:
      with open(filename) as f:
        cached = cPickle.load(f)
        self.classes = cached.classes
        self.images = cached.images
        print("...loaded from cache in %.2f s"%tt.qtoc())
        return
    print("...loading from scratch")
    filename = config.pascal_paths[name]
    self.classes = config.pascal_classes 
    with open(filename) as f:
      imgset = [line.strip() for line in f.readlines()]
    for i,img in enumerate(imgset):
      tt.tic('2')
      if tt.qtoc('2') > 2:
        print("  on image %d/%d"%(i,len(imgset)))
        tt.tic('2')
      if len(img)>0:
        xml_filename = opjoin(config.VOC_dir,'Annotations',img+'.xml')
        self.images.append(Image.load_from_pascal_xml_filename(self.classes,xml_filename))
    filename = config.get_cached_dataset_filename(name)
    print("  ...saving to cache file")
    with open(filename, 'w') as f:
      cPickle.dump(self,f)
    print("  ...done in %.2f s\n"%tt.qtoc())

  ###
  # Ground truth
  ###
  def get_cls_counts(self, with_diff=True, with_trun=True):
    """
    Return (N,K) array of class presence counts, where
    - n corresponds to index into self.images,
    - k corresponds to index into self.classes.
    """
    kwargs = {'with_diff':with_diff, 'with_trun':with_trun}
    return ut.collect(self.images, Image.get_cls_counts, kwargs)

  def get_cls_ground_truth(self,with_diff=True,with_trun=True):
    "Return Table of classification (0/1) ground truth."
    arr = self.get_cls_counts(with_diff,with_trun)>0
    return Table(arr,self.classes)

  def get_det_gt(self, with_diff=True, with_trun=True):
    """
    Return Table of detection ground truth.
    Cache the results for the given parameter settings.
    """
    name = '%s%s'%(with_diff,with_trun)
    if name not in self.cached_det_ground_truth:
      kwargs = {'with_diff':with_diff, 'with_trun':with_trun}
      table = ut.collect_with_index(
        self.images, Image.get_det_gt, kwargs, 'img_ind')
      self.cached_det_ground_truth[name] = table
    return self.cached_det_ground_truth[name]

  def get_det_gt_for_class(self, class_name, with_diff=True, with_trun=True):
    """
    Return Table of detection ground truth, filtered for the given class.
    """
    gt = self.get_det_gt(with_diff,with_trun)
    return gt.filter_on_column('cls_ind',self.classes.index(class_name))

  ###
  # Statistics
  ###
  def plot_distribution(self):
    "Plot histogram of # classes in an image. Return the figure."
    table = self.get_cls_ground_truth(with_diff=False,with_trun=True)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    bins = np.arange(1,max(table.sum(1))+2)
    ax.hist(table.sum(1),bins,align='left',normed=True)
    ax.set_xticks(bins)
    ax.set_xlabel('Number of classes present in the image')
    ax.grid(False)

    dirname = config.get_dataset_stats_dir(self)
    filename = opjoin(dirname,'num_classes.png')
    fig.savefig(filename)
    return fig

  def plot_coocurrence(self, cmap=plt.cm.Reds, color_anchor=[0,1],
    x_tick_rot=90, size=None, title=None, plot_vals=True,
    second_order=False):
    """
    Plot a heat map of conditional occurence, where cell (i,j) means
    P(C_j|C_i). The last column in the K x (K+2) heat map corresponds
    to the prior P(C_i).

    If second_order, plots (K choose 2) x (K+2) heat map corresponding
    to P(C_i|C_j,C_k): second-order correlations.

    Return the figure.
    """
    table = self.get_cls_ground_truth(with_diff=False,with_trun=True)

    # This takes care of most of the difference between normal and second_order
    # In the former case, a "combination" is just one class to condition on.
    combinations = combination_strs = table.cols
    if second_order: 
      combinations = [x for x in itertools.combinations(table.cols,2)]
      combination_strs = ['%s, %s'%(x[0],x[1]) for x in combinations]

    total = table.shape[0]
    N = len(table.cols)
    K = len(combinations)
    data = np.zeros((K,N+2)) # extra columns are for P("nothing"|C) and P(C)
    prior = np.zeros(K)
    for i,combination in enumerate(combinations):
      if second_order:
        cls1 = combination[0]
        cls2 = combination[1]
        conditioned = table.filter_on_column(cls1).filter_on_column(cls2)
      else:
        conditioned = table.filter_on_column(combination)

      # count all the classes
      data[i,:-2] = conditioned.sum()

      # count the number of times that cls was the only one present to get
      # P("nothing"|C)
      if second_order:
        data[i,-2] = ((conditioned.sum(1)-2)==0).sum()
      else:
        data[i,-2] = ((conditioned.sum(1)-1)==0).sum()

      # normalize
      max_val = np.max(data[i,:])
      data[i,:] /= max_val
      data[i,:][data[i,:]==1]=np.nan

      # use the max count to compute the prior
      data[i,-1] = max_val / total

      m = Table(data,table.cols+['nothing','prior'],index=combination_strs)

    # If second_order, sort by prior and remove rows with 0 prior
    if second_order:
      m = m.filter_on_column('prior',0.001,operator.gt).\
            sort_by_column('prior',descending=True)
      # TODO: just take the top K actually, for a side-by-side figure
      m.arr = m.arr[:len(self.classes),:]

    if size:
      fig = plt.figure(figsize=size)
    else:
      w=max(12,m.shape[1])
      h=max(12,m.shape[0])
      fig = plt.figure(figsize=(w,h))
    ax_im = fig.add_subplot(111)

    # make axes for colorbar
    divider = make_axes_locatable(ax_im)
    ax_cb = divider.new_vertical(size="5%", pad=0.1, pack_start=True)
    fig.add_axes(ax_cb)

    #The call to imshow produces the matrix plot:
    im = ax_im.imshow(m.arr, origin='upper', interpolation='nearest',
            vmin=color_anchor[0], vmax=color_anchor[1], cmap=cmap)

    #Formatting:
    ax = ax_im
    ax.set_xticks(np.arange(m.shape[1]))
    ax.set_xticklabels(m.cols)
    for tick in ax.xaxis.iter_ticks():
      tick[0].label2On = True
      tick[0].label1On = False
      tick[0].label2.set_rotation(x_tick_rot)
      tick[0].label2.set_fontsize('x-large')

    ax.set_yticks(np.arange(m.shape[0]))
    ax.set_yticklabels(m.index,size='x-large')

    ax.yaxis.set_minor_locator(
      matplotlib.ticker.FixedLocator(np.arange(-.5,m.shape[0]+0.5)))
    ax.xaxis.set_minor_locator(
      matplotlib.ticker.FixedLocator(np.arange(-.5,m.shape[1]-0.5)))
    ax.grid(False,which='major')
    ax.grid(True,which='minor',ls='-',lw=7,c='w')

    # Make the major and minor tick marks invisible
    for line in ax.xaxis.get_ticklines() + ax.yaxis.get_ticklines():
        line.set_markeredgewidth(0)
    for line in ax.xaxis.get_minorticklines() + ax.yaxis.get_minorticklines():
        line.set_markeredgewidth(0)

    # Limit the area of the plot
    ax.set_ybound([-0.5, m.shape[0] - 0.5])
    ax.set_xbound([-0.5, m.shape[1] - 0.5])

    #The following produces the colorbar and sets the ticks
    #Set the ticks - if 0 is in the interval of values, set that, as well
    #as the maximal and minimal values:
    #Extract the minimum and maximum values for scaling
    max_val = np.nanmax(m.arr)
    min_val = np.nanmin(m.arr)
    if min_val < 0:
      ticks = [color_anchor[0], min_val, 0, max_val, color_anchor[1]]
    #Otherwise - only set the maximal value:
    else:
      ticks = [color_anchor[0], max_val, color_anchor[1]]

    # Plot line separating 'nothing' and 'prior' from rest of plot
    l = ax.add_line(mpl.lines.Line2D(
      [m.shape[1]-2.5,m.shape[1]-2.5],[-.5,m.shape[0]-0.5],
      ls='--',c='gray',lw=2))
    l.set_zorder(3)

    # Display the actual values in the cells
    if plot_vals:
      for i in xrange(0, m.shape[0]):
        for j in xrange(0,m.shape[1]):
          val = m.arr[i,j]
          if np.isnan(val):
            continue
          if val > 0.5:
            ax.text(j-0.2,i+0.1,'%.2f'%val,color='w')
          else:
            ax.text(j-0.2,i+0.1,'%.2f'%val,color='k')

    # Hide the black frame around the plot
    # Doing ax.set_frame_on(False) results in weird thin lines
    # from imshow() at the edges. Instead, we set the frame to white.
    for spine in ax.spines.values():
      spine.set_edgecolor('w')

    # Set title
    if title is not None:
      ax.set_title(title)

    # Plot the colorbar and remove its frame as well.
    cb = fig.colorbar(im, cax=ax_cb, orientation='horizontal',
            cmap=cmap, ticks=ticks, format='%.2f')
    cb.ax.artists.remove(cb.outline)

    # Save figure
    dirname = config.get_dataset_stats_dir(self)
    suffix = '_second_order' if second_order else ''
    filename = opjoin(dirname,'cooccur%s.png'%suffix)
    fig.savefig(filename)

    return fig
