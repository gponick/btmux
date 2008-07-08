"""
This is an example of how to open a map file, return a MuxMap object, and
get a few values.
"""
from class_map import MuxMap
from img_generator.mapimage import PixelHexMapImage 
from map_parsers.parser_stream import MapStreamParser

parser = MapStreamParser(open('sample_data/large.map', 'r'))
# This is our new MuxMap object.
newmap = parser.get_muxmap()
# Set up an image generator pointing to the map object.
img = PixelHexMapImage(newmap)
# Generate the PIL Image.
img.generate_map(max_dimension=400)
# Open with image viewer.
img.show()
# Save the image to a file.
#img.save_image("currmap.png", "PNG")