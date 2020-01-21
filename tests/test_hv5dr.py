import os

import numpy as np

from deap.tools._hypervolume import hv
from hvwfg import hv5dr, wfg


def test_5d():
    front = np.loadtxt(os.path.join(os.path.dirname(__file__),
                                    'fixtures', 'ctsei3.dat'))
    ref = np.array([1.1, 1.1, 1.1, 1.1, 1.1])
    hv1 = hv5dr(front, ref)
    hv2 = hv.hypervolume(front, ref)
    hv3 = wfg(front, ref)
    assert abs(hv1 - hv3) <= 1e-3
    assert abs(hv1 - hv2) <= 1e-3
    assert abs(hv2 - hv3) <= 1e-12
