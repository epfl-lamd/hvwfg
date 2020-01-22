import os
import sys

import numpy
import versioneer
from setuptools import find_packages, setup
from setuptools.extension import Extension


try:
    import Cython
    USE_CYTHON = True
except ImportError:
    USE_CYTHON = False

if USE_CYTHON:
    if not os.path.exists('hvwfg/wfg.pyx'):
        ext = '.c'
    else:
        ext = '.pyx'
elif os.path.exists('hvwfg/wfg.c'):
    ext = '.c'
else:
    raise RuntimeError("Cython must be installed")

if sys.platform in ['darwin', 'linux', 'bsd']:
    extra_compile_args = ['-O3']
elif sys.platform in ['win32']:
    extra_compile_args = ['/Ox']

ext_modules = [
    Extension("hvwfg.{}".format(name),
              sources=["hvwfg/{}{}".format(name, ext)],
              include_dirs=[numpy.get_include()],
              extra_compile_args=extra_compile_args)
    for name in ['wfg', 'hv4dr', 'hv5dr']
]

if USE_CYTHON:
    from Cython.Build import cythonize
    ext_modules = cythonize(ext_modules)

here = os.path.abspath(os.path.dirname(__file__))
# Get the long description from the README file
with open(os.path.join(here, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name="hvwfg",
    version=versioneer.get_version(),
    cmdclass=versioneer.get_cmdclass(),
    description="Python wrapper of the WFG hypervolume calculation functions",
    long_description=long_description,
    long_description_content_type='text/markdown',
    author="Cyril Picard",
    author_email='cyril.picard@epfl.ch',
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
    ],
    packages=find_packages(),
    ext_modules=ext_modules,
    project_urls={  # Optional
        'Bug Reports': 'https://github.com/epfl-lamd/hvwfg/issues',
        'Source': 'https://github.com/epfl-lamd/hvwfg',
    },
    url='https://github.com/epfl-lamd/hvwfg',
    license='GPL3',
    setup_requires=['pytest-runner', 'cython', 'numpy'],
    install_requires=['numpy'],
    tests_require=['pytest', 'deap']
)
