## Timely Object Recognition

A system for multi-class object detection that uses reinforcement learning.
Configured to use pre-output DPM [1] detections and pre-output GIST features
on the PASCAL VOC dataset.
Relies on the FastInf library [2] for one of the inference modes (code included).

Ongoing work on making this increasingly simple to run, check here for updates.


### References

[1] P. F. Felzenszwalb, R. B. Girshick, and D. McAllester, “Cascade object detection with deformable part models,” in CVPR, 2010, pp. 2241–2248.

[2] A. Jaimovich and I. Mcgraw, “FastInf : An Efficient Approximate Inference Library,” Journal of Machine Learning Research, vol. 11, pp. 1733–1736, 2010.