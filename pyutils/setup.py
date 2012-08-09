#!/usr/bin/env python
"""
The setup script for PyModeller. Creates the module, installs
the scripts. 
Good idea to use 'install --prefix=/opt/xxxxx' so not installed
with Python.
"""
import glob
from distutils.core import setup

setup(name='lcrimageutils',
      scripts=glob.glob('bin/*.py'),
        packages=['lcrimageutils'] )
                                                     