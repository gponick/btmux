#!/usr/bin/env python
"""
 BattletechMUX Map Library (btmux_maplib) 
 Copyright (C) 2008  Gregory Taylor

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""
from distutils.core import setup

LONG_DESCRIPTION = \
"""btmux_maplib is a set of utility classes and functions for parsing, 
manipulating, and visualizing BattletechMUX .map files."""

CLASSIFIERS = [
                'Development Status :: 3 - Alpha',
                'Intended Audience :: Developers',
                'License :: OSI Approved :: GNU General Public License (GPL)',
                'Natural Language :: English',
                'Operating System :: OS Independent',
                'Programming Language :: Python',
                'Topic :: Games/Entertainment :: Multi-User Dungeons (MUD)',
                'Topic :: Software Development :: Libraries :: Python Modules' 
              ]

KEYWORDS = 'btmux battletech mux battletechmux btech mud mush map mapping'

setup(name='btmux_maplib',
      version='1.0.0',
      description='BattletechMUX map utility library.',
      long_description = LONG_DESCRIPTION,
      author='Gregory Taylor',
      author_email='gtaylor@gc-taylor.com',
      url='http://docs.btmux.com/index.php/Btmux_maplib',
      download_url='http://docs.btmux.com/index.php/Btmux_maplib',
      packages=['btmux_maplib', 'btmux_maplib.img_generator',
                'btmux_maplib.map_parsers', 'btmux_maplib.map_exporters'],
      platforms = 'Platform Independent',
      license = 'GPLv3',
      classifiers = CLASSIFIERS,
      keywords = KEYWORDS,
     )
