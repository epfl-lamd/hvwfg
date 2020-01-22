from ._version import get_versions
from .hv4dr import hv4dr
from .hv5dr import hv5dr
from .wfg import wfg


__version__ = get_versions()['version']
del get_versions


def hv(front, ref, force_wfg=False):
    """Computes the hypervolume of `front` with `ref` as reference

    Minimization is assumed.

    Args:
        front (2D numpy.ndarray): Pareto front to evaluate
        ref (1D numpy.ndarray): Reference point
        force_wfg (bool, default False): Force the use of the simpler WFG code
    Returns:
        Hypervolume value
    """
    if not force_wfg and front.shape[1] == 4:
        return hv4dr(front, ref)
    elif not force_wfg and front.shape[1] == 5:
        return hv5dr(front, ref)
    else:
        return wfg(front, ref)
