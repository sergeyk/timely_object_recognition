import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

import unittest
from numpy.testing import *

import numpy as np

import skvisutils
from timely import TimelyConfig

### Paths
test_dir = os.path.abspath(os.path.dirname(__file__))
test_support_dir = os.path.abspath(
    os.path.join(os.path.dirname(__file__), 'support'))
test_data1 = os.path.join(test_support_dir, 'data1.json')
test_data2 = os.path.join(test_support_dir, 'data2.json')

# create config for small tests
res_dir = os.path.join(test_dir, 'temp', 'res')
temp_dir = os.path.join(test_dir, 'temp', 'temp')
pascal_dir = os.path.join(test_support_dir, 'VOC2007')
test_config = TimelyConfig(res_dir, temp_dir, pascal_dir)

# create config for full tests
PATH_TO_VOC2007 = '/Users/sergeyk/work/vision_data/VOC2007'
full_config = TimelyConfig(res_dir, temp_dir, PATH_TO_VOC2007)
