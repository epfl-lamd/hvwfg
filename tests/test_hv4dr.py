import os

import numpy as np

from deap.tools._hypervolume import hv
from hvwfg import hv4dr, wfg


def test_4d():
    front = np.loadtxt(os.path.join(os.path.dirname(__file__),
                                    'fixtures', 'ctse3.dat'))
    ref = np.array([1.1, 1.1, 1.1, 1.1])
    hv1 = hv4dr(front, ref)
    hv2 = hv.hypervolume(front, ref)
    hv3 = wfg(front, ref)
    assert abs(hv1 - hv2) <= 1e-12
    assert abs(hv1 - hv3) <= 1e-12
