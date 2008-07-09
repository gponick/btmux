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

"""
This is an example of how to open a map file, return a MuxMap object, and
get a few values.
"""
from btmux_maplib.map import MuxMap
from btmux_maplib.map_parsers.fileobj import MapFileObjParser

parser = MapFileObjParser(open('../sample_data/sample.map', 'r'))
# This is our new MuxMap object.
the_map = parser.get_muxmap()
print "Terrain at 158,54: %s" %(the_map.get_hex_terrain(158, 54))
print "Elevation at 158,54: %d" %(the_map.get_hex_elevation(158, 54))