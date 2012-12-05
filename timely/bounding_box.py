from timely.common_imports import *
import timely.config as config

from scipy.weave import blitz

class BoundingBox:
  """
  Methods for constructing location in an image and converting between
  different formats of location (e.g. from image space to feature space).
  The native storage format is np.array([x,y,w,h]).
  """

  columns = ['x','y','w','h']

  def __init__(self, seq=None, format='width'):
    """
    Instantiate from a sequence containing either 
    x,y,w,h (format=='width', default) or
    x1,x2,y1,y2 (format!='width').
    """
    if seq:
      x = float(seq[0])
      y = float(seq[1])
      if format == 'width':
        w = float(seq[2])
        h = float(seq[3])
      else:
        w = float(seq[2]-x+1)
        h = float(seq[3]-y+1)
      self.arr = np.array([x,y,w,h])

  def __repr__(self):
    return "BoundingBox: %s" % self.get_arr()

  def __eq__(self,other):
    return np.all(self.arr == other.arr)
  def __ne__(self,other):
    return not __eq__(self,other)

  def area(self):
    "Return area."
    return self.arr[2]*self.arr[3]

  @classmethod
  def clipboxes_arr(cls, arr, bounds):
    """
    Take an arr in (x,y,w,h) format and clip boxes to fit into bounds,
    provided as (min_x,min_y,max_x,max_y). Remove detections that are
    entirely outside the bounds.
    """
    arr = cls.convert_arr_to_corners(arr)
    arr[:,0] = np.maximum(arr[:,0],bounds[0])
    arr[:,1] = np.maximum(arr[:,1],bounds[1])
    arr[:,2] = np.minimum(arr[:,2],bounds[2])
    arr[:,3] = np.minimum(arr[:,3],bounds[3])
    arr = cls.convert_arr_from_corners(arr)

    pick = np.flatnonzero((arr[:,2]>0) & (arr[:,3]>0))
    return arr[pick,:]

  # NOTE (sergeyk): I've experimented with all kinds of ways of doing this,
  # and this is the fastest I've found.
  @classmethod
  def convert_arr_from_corners(cls, arr):
    """Take an arr in x1,y1,x2,y2 format, and return arr in x,y,w,h format."""
    ret_arr = np.copy(arr)
    if arr.ndim>1:
      expr = "ret_arr[:,2] = arr[:,2]-arr[:,0]+1;"+\
             "ret_arr[:,3] = arr[:,3]-arr[:,1]+1"
      blitz(expr)
    else:
      ret_arr[2] = -arr[0]+arr[2]+1
      ret_arr[3] = -arr[1]+arr[3]+1
    return ret_arr

  @classmethod
  def convert_arr_to_corners(cls, arr):
    """Take an arr in x1,y1,w,h format, and return arr in x1,y1,x2,y2 format."""
    ret_arr = np.copy(arr)
    if arr.ndim>1:
      expr = "ret_arr[:,2] = arr[:,2]+arr[:,0]-1;"+\
             "ret_arr[:,3] = arr[:,3]+arr[:,1]-1"
      blitz(expr)
    else:
      ret_arr[2] = arr[0]+arr[2]-1
      ret_arr[3] = arr[1]+arr[3]-1
    return ret_arr

  def get_arr(self,format='width'):
    """
    Returns ndarray representation of 
    [x,y,w,h] (format=='width', default) or
    [x1,y1,x2,y2] (format!='width').
    """
    if format == 'width':
      return self.arr
    else:
      return BoundingBox.convert_arr_from_corners(self.arr)

  @classmethod
  def get_overlap(cls,bb,bbgt,format='width'):
    """
    Return the PASCAL overlap, defined as the area of intersection
    over the area of union.
    bb can be an (n,4) ndarray.
    bbgt must be an (4,) ndarray or a list.
    """
    if not format=='width':
      return cls.get_overlap_corners_format(bb,bbgt)
    return cls.get_overlap_width_format(bb,bbgt)

  @classmethod
  def get_overlap_corners_format(cls,bb,bbgt):
    # once we transpose this from Nx4 to 4xN, we can share the code between the
    # 1-D and 2-D arrays
    bb = bb.T
    bi = np.array([
      np.maximum(bb[0,],bbgt[0]),
      np.maximum(bb[1,],bbgt[1]),
      np.minimum(bb[2,],bbgt[2]),
      np.minimum(bb[3,],bbgt[3])])
    iw = bi[2,]-bi[0,]+1
    ih = bi[3,]-bi[1,]+1
    ua =  (bb[2,]-bb[0,]+1)*(bb[3,]-bb[1,]+1) + \
          (bbgt[2]-bbgt[0]+1)*(bbgt[3]-bbgt[1]+1) - \
          iw*ih
    if bb.ndim > 1:
      ov = np.zeros(bb.shape[1])
      mask = np.all((iw>0, ih>0),0)
      ov[mask] = iw[mask]*ih[mask]/ua[mask]
    else:
      ov = 0
      if iw>0 and ih>0:
        ov = iw*ih/ua
    return ov
    
  @classmethod
  def get_overlap_width_format(cls,bb,bbgt):
    # once we transpose this from Nx4 to 4xN, we can share the code between the
    # 1-D and 2-D arrays
    bb = bb.T
    bi = np.array([
      np.maximum(bb[0,],bbgt[0]),
      np.maximum(bb[1,],bbgt[1]),
      np.minimum(bb[2,]+bb[0,]-1,bbgt[2]+bbgt[0]-1),
      np.minimum(bb[3,]+bb[1,]-1,bbgt[3]+bbgt[1]-1)])
    iw = bi[2,]-bi[0,]+1
    ih = bi[3,]-bi[1,]+1
    ua =  bb[2,]*bb[3,] + \
          bbgt[2]*bbgt[3] - \
          iw*ih
    if bb.ndim > 1:
      ov = np.zeros(bb.shape[1])
      mask = np.all((iw>0, ih>0),0)
      ov[mask] = iw[mask]*ih[mask]/ua[mask]
    else:
      ov = 0
      if iw>0 and ih>0:
        ov = iw*ih/ua
    return ov
