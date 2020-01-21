#cython: language_level=3

cdef extern from "c/wfg.h":
    ctypedef double OBJECTIVE

    ctypedef struct POINT:
        OBJECTIVE *objectives

    ctypedef struct FRONT:
        int nPoints
        int n
        POINT *points

    ctypedef struct FILECONTENTS:
        int nFronts
        FRONT *fronts

    double hv(FRONT ps)
    double wrapped_hv(FRONT ps)
    #void allocate_memory(int maxm_, int maxn_)
    #void free_memory()
