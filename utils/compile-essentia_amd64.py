#!/usr/bin/env python3

import os
script_dir = os.path.dirname(os.path.realpath(__file__))

root_dir = os.path.abspath(script_dir + "/..")

# Compile Essentia
os.chdir(os.path.join(root_dir,'libs','essentia'))

configure_command = 'python3 waf configure --build-static --with-cpptests --with-examples'

""" 
Conditional flags:

--with-python to build with Python bindings,

--with-examples to build command line extractors based on the library,

--with-vamp to build Vamp plugin wrapper,

--with-gaia to build with Gaia support,

--with-tensorflow to build with TensorFlow support,

--mode=debug to build in debug mode,

--with-cpptests to build cpptests
"""
os.system(configure_command)

compile_command = 'python3 waf'
os.system(compile_command)