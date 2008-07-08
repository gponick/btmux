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