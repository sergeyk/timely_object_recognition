import numpy as np

class Sample:
  """
  Class to allow equality testing to remove duplicate samples during training.
  """

  def __init__(self):
    "Initialize all the fields with None."
    fields = [
      'img_ind','step_ind','state','action_ind','dt','t',
      'det_naive_ap', 'det_actual_ap', 'entropy',
      'auc_ap_raw', 'auc_ap', 'auc_entropy']
    for field in fields:
      self.__dict__[field] = None

  def __repr__(self):
    return """
Sample at img_ind: %(img_ind)s:
  step_ind: %(step_ind)d
  state:
  %(state)s
  action_ind: %(action_ind)d
  dt: %(dt).3f \t|\t t: %(t).3f
  auc_ap_raw/auc_ap/det_naive_ap/det_actual_ap: %(auc_ap_raw).3f/%(auc_ap).3f/%(det_naive_ap).3f/%(det_actual_ap).3f
  auc_entropy/entropy: %(auc_entropy).3f/%(entropy).3f
"""%self.__dict__

  def __ne__(self,other):
    return not self.__eq__(other)
    
  def __eq__(self,other):
    return \
      self.img_ind == other.img_ind and \
      self.action_ind == other.action_ind and \
      self.dt == other.dt and \
      self.det_naive_ap == other.det_naive_ap and \
      self.det_actual_ap == other.det_actual_ap and \
      self.entropy == other.entropy and \
      self.auc_ap_raw == other.auc_ap_raw and \
      self.auc_ap == other.auc_ap and \
      self.auc_entropy == other.auc_entropy and \
      np.all(self.state == other.state)

  def __hash__(self):
    return self.__repr__()