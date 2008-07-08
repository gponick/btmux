from exceps import *

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