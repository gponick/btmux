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
from btmux_maplib.class_map import MuxMap

class MapStreamParser(object):
    """
    This class parses a map from any file-like Python object that supports
    readlines(). The most common usage is with open(). From here you may
    return a MuxMap with get_muxmap().
    """
    # Stores the map data as a string.
    map_string = None
    # A tuple of the map's dimensions (X,Y).
    dimensions = None
    
    def __init__(self, stream):
        """
        Pass a file-like Python object that supports readlines() in, and
        set the parser up for parsing.
        """
        self.map_string = stream.readlines()
        # Dimensions are on the first line of the map file.
        dimensions_str = self.map_string[0].split()
        # Store the dimensions in a tuple to prevent tampering.
        self.dimensions = (int(dimensions_str[0]),
                                int(dimensions_str[1]))
               
    def get_map_dimensions(self):
        return self.dimensions
    
    def get_map_width(self):
        return self.get_map_dimensions()[0]
    
    def get_map_height(self):
        return self.get_map_dimensions()[1]
        
    def get_hex_terrain(self, x, y):
        return self.map_string[y+1][x*2]
    
    def get_hex_elevation(self, x, y):
        return self.map_string[y+1][(x*2)+1]
    
    def get_muxmap(self):
        """
        Returns a MuxMap object with the terrain/hexes populated.
        """
        map = MuxMap()
        map.dimensions = self.get_map_dimensions()
    
        # Iterate through our string map data and set up the Lists on the
        # new map object.
        for y in range(0, self.get_map_height()):
            map.terrain_list.append([])
            map.elevation_list.append([])
            for x in range(self.get_map_width()):
                map.terrain_list[y].append(self.get_hex_terrain(x,y))
                map.elevation_list[y].append(self.get_hex_elevation(x,y))
        # Done parsing and storing, bombs away.
        return map
