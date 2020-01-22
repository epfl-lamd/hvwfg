import os
import sys

import numpy
import versioneer
from Cython.Build import cythonize
from setuptools import find_packages, setup
from setuptools.extension import Extension

if sys.platform in ['darwin', 'linux', 'bsd']:
    extra_compile_args = ['-O3']
elif sys.platform in ['win32']:
    extra_compile_args = ['/Ox']

ext_modules = [
    Extension("hvwfg.*",
              sources=["hvwfg/*.pyx"],
              include_dirs=[numpy.get_include()],
              extra_compile_args=extra_compile_args)
]

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
    ext_modules=cythonize(ext_modules),
    project_urls={  # Optional
        'Bug Reports': 'https://github.com/epfl-lamd/hvwfg/issues',
        'Source': 'https://github.com/epfl-lamd/hvwfg',
    },
    setup_requires=['pytest-runner', 'cython', 'numpy'],
    install_requires=['numpy'],
    tests_require=['pytest', 'deap']
)
