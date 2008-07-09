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
Map image generation classes.
"""
import Image
from btmux_maplib.img_generator import rgb_vals
from btmux_maplib.img_generator.exceptions import InvalidImageMode

class MuxMapImage(object):
    """
    This class serves as a base class for map image types. You generally only
    need to over-ride render_hexes() to have something that works. See
    PixelHexMapImage for an example.
    
    DO NOT USE THIS CLASS DIRECTLY!
    """
    # Reference to the MuxMap object to image.
    map = None
    # The PIL Image object.
    map_img = None
    # Imaging mode, see set_mode().
    mode = "color"
    # Show some debug information.
    debug = True
    
    """
    A lowercase list of valid imaging modes.
     color: Standard color terrain/elevation map.
     elevmap: Elevation only, looks grayscaled aside from water.
    """
    VALID_MODES = ["color", "elevmap"]
    
    def __init__(self, map):
        """
        Default init routine.
        
        Args:
        * map: (MuxMap) The map object to create an image of.
        """
        self.map = map
        
    def set_mode(self, mode):
        mode_lower = mode.lower()
        if mode_lower not in self.VALID_MODES:
            raise InvalidImageMode(mode_lower)
        else:
            self.mode = mode_lower
            if self.debug:
                print "Imaging mode set to: %s" % self.mode
   
    def handle_resizing(self, min_dimension, max_dimension):
        """
        Given a min and/or max dimension, calculate the overall re-size ratio
        for the image and re-size if necessary. Return the scaling multiplier
        used.
        """
        map_width = float(self.map_img.size[0])
        map_height = float(self.map_img.size[1])
        resize_mul = 1.0
        
        if min_dimension != None and (map_width < min_dimension or map_height < min_dimension):
            # Determine the smallest side to bring up to our limit.
            smallest_dim = min(map_width, map_height)
            # Bicubic gives the best look when scaling up.
            resize_filter = Image.BICUBIC
            resize_mul = float(min_dimension) / smallest_dim
            if self.debug:
                print 'Under-sized, re-size needed: (%d/%d) = %f' % (min_dimension,
                                                                     smallest_dim,
                                                                     resize_mul)
            self.resize_img(resize_mul, resize_filter)
        elif max_dimension != None and (map_width > max_dimension or map_height > max_dimension):
            # Determine the largest side to bring down to our limit.
            largest_dim = max(map_width, map_height)
            # Anti-aliasing looks best when scaling down.
            resize_filter = Image.ANTIALIAS
            resize_mul = float(max_dimension) / largest_dim
            print resize_mul
            if self.debug:
                print 'Over-sized, re-size needed: (%d/%d) = %f' % (max_dimension,
                                                                    largest_dim,
                                                                    resize_mul)
            self.resize_img(resize_mul, resize_filter)
        else:
            if self.debug:
                print 'No re-sizing necessary.'
        return resize_mul

    def resize_img(self, resize_mul, resize_filter):
        """
        Re-size the map image by a float value. 1.0 = 100%.
        """
        map_width = self.map_img.size[0]
        map_height = self.map_img.size[1]
        
        if self.debug:
            print 'Re-Size Mul: %f' % resize_mul
            print 'Before  Width: %d  Height: %d' % (map_width, map_height)
            print 'After   Width: %d  Height: %d' % (map_width * resize_mul, 
                                                map_height * resize_mul)

        # Re-size the image with the appropriate size multiplier.     
        self.map_img = self.map_img.resize((int(map_width * resize_mul),
                                            int(map_height * resize_mul)), 
                                            resize_filter)

    def render_hexes(self):
        """
        Stub to alert people trying to use this class. MuxMapImage is not
        meant to be used directly, and future sub-classes need a fall-through.
        """
        print "Implement a render_hexes() method for this class."

    def generate_map(self, min_dimension=None, max_dimension=None):
        """
        Generates a image from a map file, populates the object's map_img 
        attribute with a PIL Image.
        
        min and max dimensions will scale the image if either the height or the
        width goes above the max or under the min size in pixels. You may
        specify one or both.
        """
        self.map_img = Image.new("RGB", self.map.get_map_dimensions())
        self.render_hexes()
        
        # Do any re-sizing needed.
        if min_dimension or max_dimension:
            self.handle_resizing(min_dimension, max_dimension)
            
        if self.debug:
            print 'Image generation complete.'

    def get_terrain_rgb(self, terrain, elev):
        """
        Looks up the correct RGB value tuple for a given terrain and elevation.
        """
        if self.mode == "elevmap" and terrain != "~":
            return rgb_vals.cmap["."][elev]
        
        return rgb_vals.cmap[terrain][elev]
                
            
    def show(self):
        """
        Following PIL convention, show() opens your OS's image viewer for
        the generated file. On Linux/Unix, this is typically xv, on Windows,
        Windows Preview thing.
        """
        self.map_img.show()

    def save(self, filename, format="PNG"):
        """
        Saves the current map file in the specified PIL-supported format.
        """
        self.map_img.save(filename, format)
        
class PixelHexMapImage(MuxMapImage):
    """
    Renders the map's hexes at a one hex per pixel ratio. This does not look
    very good if zoomed or scaled in very far, but is fast and more natural
    on larger maps at 100% or less scaling.
    """
    def render_hexes(self):
        """
        Over-rides the MuxMapImage stub routine to do our one pixel per hex
        rendering.
        """
        # Shortcuts for readability.
        map_width = self.map.get_map_width()
        map_height = self.map.get_map_height()
        
        for y in range(0, map_height):
            for x in range(0, map_width):
                terrain = self.map.get_hex_terrain(x, y)
                elev = self.map.get_hex_elevation(x, y)
                self.map_img.putpixel((x,y), 
                        self.get_terrain_rgb(terrain, elev))