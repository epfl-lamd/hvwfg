#cython: language_level=3
#distutils: sources = hvwfg/c/hv4dr.c

import numpy as np

cimport numpy as np
from hvwfg.wfg_common cimport POINT, FRONT, wrapped_hv
from libc.stdlib cimport malloc, free

def hv4dr(np.ndarray[np.double_t, ndim=2, mode="c"] points,
           np.ndarray[np.double_t, ndim=1, mode="c"] reference):
    cdef FRONT f
    cdef int i, j
    cdef np.ndarray[np.int64_t, ndim=1, mode="c"] smaller
    smaller, = (points <= reference).all(axis=1).nonzero()
    if len(smaller) == 0:
        return 0.
    points = np.abs(points - reference)

    f.nPoints = smaller.shape[0]
    f.n = points.shape[1]
    f.points = <POINT *> malloc(f.nPoints * sizeof(POINT))
    for i, j in enumerate(smaller):
        f.points[i].objectives = &points[j, 0]

    calculated = wrapped_hv(f)
    free(f.points)
    return calculated
