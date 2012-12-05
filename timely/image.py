from timely.common_imports import *
import timely.config as config

from skimage.io import imread,imsave,imshow
import xml.dom.minidom as minidom

from timely.bounding_box import BoundingBox

class Image(object):
  "An image has a name, size and a Table of objects."

  columns = BoundingBox.columns + ['cls_ind', 'diff', 'trun']
  
  def __init__(self,width,height,classes=None,name=None):
    self.width = width
    self.height = height
    self.classes = classes
    self.name = name
    self.filename = None
    self.objects_table = None
    # the above Table is constructed by the loader class

  @property
  def size(self):
    return (self.width,self.height)

  def __repr__(self):
    return "Image (%(name)s)\n  W x H: %(width)d x %(height)d\n  Objects:\n %(objects_table)s" % self.__dict__

  def display_image(self):
    "Display the image using skimage."
    imshow(self.load_image())

  def load_image(self):
    "Return the image loaded using skimage."
    if not self.filename:
      print("Image: Cannot load, no associated filename")
      return None
    return imread(self.filename)

  def get_objects(self,with_diff=False,with_trun=True):
    "Return Table of objects filtered with the parameters."
    df = self.objects_table
    df = df if with_diff else df.filter_on_column('diff',0) 
    df = df if with_trun else df.filter_on_column('trun',0) 
    return df

  def get_det_gt(self, with_diff=False, with_trun=True, cls_name=None):
    """
    Return Table of detection ground truth.
    If class_name is given, only include objects of that class.
    Filter according to with_diff and with_trun.
    """
    objects = self.get_objects(with_diff,with_trun)
    if cls_name and not cls_name=='all':
      cls_ind = self.classes.index(cls_name)
      objects = objects.filter_on_column('cls_ind',cls_ind)
    return objects

  def get_cls_counts(self, with_diff=False, with_trun=True):
    "Return an array of the counts of each class in the image."
    counts = np.zeros(len(self.classes))
    objects = self.get_objects(with_diff,with_trun)
    if objects.shape[0]>0:
      cls_inds = objects.subset_arr('cls_ind').astype('int')
      bincount = np.bincount(cls_inds)
      # need to pad this with zeros for total length of num_classes
      counts[:bincount.size] = bincount
    return counts

  def get_cls_gt(self,with_diff=False,with_trun=False):
    "Return an array of class presence (True/False) ground truth."
    return self.get_cls_counts(with_diff,with_trun)>0

  def contains_class(self, cls_name, with_diff=False, with_trun=True):
    "Return whether the image contains an object of class cls_name."
    gt = self.get_cls_gt(with_diff,with_trun)
    return gt[self.classes.index(cls_name)]

  def contains_cls_ind(self,cls_ind):
    gt = self.get_cls_gt()
    return gt[cls_ind]

  ### 
  # Windows
  ###
  def get_whole_image_bbox(self):
    "Return a BoundingBox with (0,0,width,height) of the image."
    return BoundingBox((0,0,self.width,self.height))

  def get_windows(self,window_params,with_time=False):
    "Return all windows that can be generated with given params."
    return SlidingWindows.get_windows(self,None,window_params,with_time)

  def get_random_windows(self,window_params,num_windows):
    "Return at most num_windows random windows generated according to params."
    windows = self.get_windows(window_params)
    return windows[ut.random_subset_up_to_N(windows.shape[0],num_windows),:]

  ###
  # Loaders
  ###
  @classmethod
  def load_from_json_data(cls, classes, data):
    "Return an Image instantiated from a JSON representation."
    name = data['name']
    width = data['size'][0]
    height = data['size'][1]
    img = Image(width,height,classes,name)
    objects = []
    for obj in data['objects']:
      bbox = BoundingBox(obj['bbox'])
      cls_name = obj['class']
      cls_ind = classes.index(cls_name)
      diff = obj['diff']
      trun = obj['trun']
      objects.append(np.hstack((bbox.get_arr(), cls_ind, diff, trun)))
    if len(objects)>0:
      img.objects_table = Table(np.array(objects), cls.columns)
    else:
      img.objects_table = Table(None, cls.columns)
    return img
  
  @classmethod
  def load_from_pascal_xml_filename(cls, classes, filename):
    "Load image info from a file in the PASCAL VOC XML format."

    def get_data_from_tag(node, tag):
      if tag is "bndbox":
        x1 = int(node.getElementsByTagName(tag)[0].childNodes[1].childNodes[0].data)
        y1 = int(node.getElementsByTagName(tag)[0].childNodes[3].childNodes[0].data)
        x2 = int(node.getElementsByTagName(tag)[0].childNodes[5].childNodes[0].data)
        y2 = int(node.getElementsByTagName(tag)[0].childNodes[7].childNodes[0].data)
        return (x1, y1, x2, y2)
      else:
        return node.getElementsByTagName(tag)[0].childNodes[0].data

    with open(filename) as f:
      data = minidom.parseString(f.read())

    # image info
    name = get_data_from_tag(data, "filename")
    filename = opjoin(config.VOC_dir, 'JPEGImages', name)
    size = data.getElementsByTagName("size")[0]
    im_width = int(get_data_from_tag(size, "width"))
    im_height = int(get_data_from_tag(size, "height"))
    im_depth = int(get_data_from_tag(size, "depth"))
    width = im_width
    height = im_height
    img = Image(width,height,classes,name)

    # per-object info
    objects = []
    for obj in data.getElementsByTagName("object"):
      clas = str(get_data_from_tag(obj, "name")).lower().strip()
      diff = int(get_data_from_tag(obj, "difficult"))
      trun = int(get_data_from_tag(obj, "truncated"))
      rect = get_data_from_tag(obj, "bndbox")
      bbox = BoundingBox(rect, format='corners')
      cls_ind = classes.index(clas)
      objects.append(np.hstack((bbox.get_arr(), cls_ind, diff, trun)))
    if len(objects)>0:
      img.objects_table = Table(np.array(objects), cls.columns)
    else:
      img.objects_table = Table(None, cls.columns)
    return img
