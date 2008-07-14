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
from btmux_maplib.img_generator.mapimage import PixelHexMapImage, HexMapImage
from btmux_maplib.map_parsers.fileobj import MapFileObjParser

# Grab the example map from sample_data.
parser = MapFileObjParser(open('../sample_data/large.map', 'r'))
# This is our new MuxMap object.
the_map = parser.get_muxmap()

# Set up an image generator pointing to the map object.
img_gen = HexMapImage(the_map)
# Pixel map generator. Faster but less pretty.
#img_gen = PixelHexMapImage(the_map)

# Set the mode to elevation map.
#img_gen.set_mode("elevmap")

# Generate the PIL Image.
img_gen.generate_map(max_dimension=400)

# Open with image viewer.
img_gen.show()

# Save the image to a file.
#img.save_image("currmap.png", "PNG")