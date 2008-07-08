#!/usr/bin/env python

from distutils.core import setup

setup(name='btmux_maplib',
      version='1.0',
      description='BattletechMUX map utility library.',
      author='Gregory Taylor',
      author_email='gtaylor@gc-taylor.com',
      url='http://battletechmux.com/',
      packages=['btmux_maplib', 'btmux_maplib.img_generator',
                'btmux_maplib.map_parsers'],
      platforms = 'Platform Independent',
     )
