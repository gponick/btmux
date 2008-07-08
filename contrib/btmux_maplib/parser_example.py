"""
This is an example of how to open a map file, return a MuxMap object, and
get a few values.
"""
from class_map import MuxMap
from map_parsers.parser_stream import MapStreamParser

parser = MapStreamParser(open('sample_data/sample.map', 'r'))
# This is our new MuxMap object.
newmap = parser.get_muxmap()
print "Terrain at 158,54: %s" %(newmap.get_hex_terrain(158, 54))
print "Elevation at 158,54: %d" %(newmap.get_hex_elevation(158, 54))