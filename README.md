# hvwfg

Python wrapper for the Hypervolume calculation code of the
[Walking Fish Group](http://www.wfg.csse.uwa.edu.au/hypervolume/)

This package needs [Cython](https://cython.readthedocs.io/en/latest/) and
[numpy](https://numpy.org) to be installed.

```bash
python setup.py install
```

## Usage

```python
import numpy as np
import hvwfg

# Fitness vector assuming minimization
obj = np.array([[0.3, 0.6],
                [0.4, 0.4],
                [0.6, 0.2]])

ref = np.array([1.1, 1.1])

hvwfg.wfg(obj, ref)
```

Depending on the input size, the fastest code is selected.
