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
class MapException(Exception):
    """
    Generic MuxMap exception class.
    """
    def __init__(self, map, value):
        self.value = value
        self.map = map
    def __str__(self):
        return repr(self.value)
    
class InvalidHex(Exception):
    def __init__(self, x, y):
        self.hex_x = x
        self.hex_y = y
    def __str__(self):
        return repr("Hex out of bounds: %d,%d" % (self.hex_x, self.hex_y))
    
class MapDimsNotSet(Exception):
    def __str__(self):
        return repr("Map dimensions have not been set yet.")
    
class TerrainListNotSet(Exception):
    def __str__(self):
        return repr("The terrain list has not been populated yet.")
    
class ElevationListNotSet(Exception):
    def __str__(self):
        return repr("The elevation list has not been populated yet.")