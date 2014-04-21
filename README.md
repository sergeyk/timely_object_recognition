# Timely Object Recognition

A system for deploying object detectors in sequence, guided by a policy that takes into account the current "belief state" of the system.
The policy is learned using reinforcement learning techniques.

Configured to use pre-output DPM [1] detections and pre-output GIST features
on the PASCAL VOC dataset.
Relies on the FastInf library [2] for one of the inference modes (code included).

This code accompanies my NIPS 2012 publication on Timely Object Recognition.

For more recent code, see the [project page](http://sergeykarayev.com/recognition-on-a-budget/).

#### References

- [1] P. F. Felzenszwalb, R. B. Girshick, and D. McAllester, "Cascade object detection with deformable part models," in CVPR, 2010.
- [2] A. Jaimovich and I. Mcgraw, "FastInf : An Efficient Approximate Inference Library," Journal of Machine Learning Research, vol. 11, pp. 1733–1736, 2010.
- [3] S. Karayev, T. Baumgartner, M. Fritz, and T. Darrell, "Timely Object Recognition," in NIPS, 2012.
