import os

import numpy as np

from deap.tools._hypervolume import hv
from hvwfg import wfg


def test_2d():
    front = np.loadtxt(os.path.join(os.path.dirname(__file__),
                                    'fixtures', 'CF7.dat'))
    ref = np.array([1.1, 1.1])
    hv1 = wfg(front, ref)
    hv2 = hv.hypervolume(front, ref)
    assert abs(hv1 - hv2) <= 1e-12
