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
from btmux_maplib.exceptions import *

class MuxMap(object):
    """
    The MuxMap class is intended to be a very minimalistic container for map
    data. It is advisable to either to push any code that does not fit into
    the 'raw map data' category into another class or sub-class.
    """
    # Dimensions tuple
    dimensions = None
    # List of hex terrain [y][x]
    terrain_list = []
    # List of hex elevations [y][x]
    elevation_list = []
    
    def __init__(self, dimensions=None):
        """
        Args:
        * size: (tuple) A tuple in the form of (width, height). If specified,
                initializes a map of this size that is all level 0 clear hexes.
                If none specified, you'll need to populate the map's data
                structures by another means (a map parser).
        """
        self.clear_map()
        if dimensions:
            self.dimensions = dimensions
            
            for y in range(0, self.get_map_height()):
                self.terrain_list.append([])
                self.elevation_list.append([])
                for x in range(0, self.get_map_width()):
                    self.terrain_list[y].append('.')
                    self.elevation_list[y].append('0')

    def clear_map(self):
        """
        Re-sets the map to its original, empty state.
        """
        self.dimensions = None
        self.terrain_list = []
        self.elevation_list = []
                  
    def get_map_dimensions(self):
        """
        Returns an (X,Y) tuple of the map's dimensions.
        """
        if not self.dimensions:
            raise MapDimsNotSet
        
        return self.dimensions
    
    def get_map_width(self):
        try:
            return self.dimensions[0]
        except TypeError:
            raise MapDimsNotSet
    
    def get_map_height(self):
        try:
            return self.dimensions[1]
        except TypeError:
            raise MapDimsNotSet
    
    def get_hex_terrain(self, x, y):
        """
        Returns a hex's terrain character given an X and Y value.
        """
        if len(self.terrain_list) == 0:
            raise TerrainListNotSet
        
        try:
            return self.terrain_list[y][x]
        except IndexError:
            raise InvalidHex(x, y)
        
    def get_hex_elevation(self, x, y):
        """
        Returns a hex's elevation given an X and Y value.
        """
        if len(self.elevation_list) == 0:
            raise ElevationListNotSet
        
        try:
            return int(self.elevation_list[y][x])
        except IndexError:
            raise InvalidHex(x, y)