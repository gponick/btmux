import math
import Image
from cgkit.cgtypes import *
from cgkit.noise import *
from btmux_maplib.map import MuxMap
from btmux_maplib.img_generator.mapimage import PixelHexMapImage

class fBmMapGenerator(object):
    map = None
    oct = 4
    lac = 2.0
    gain = 0.5
    z = 1
    x_add = 900
    y_add = 400
    elev_mul = 1.0
    
    def __init__(self, map):
        self.map = map
        
    def val_to_elev(self, value):
        value *= self.elev_mul
        if value > 1.0:
            return 9  
        elif value == 0.5:
            return 0
        elif value > 0.5:
            return math.ceil(((value - 0.5) / 0.49) * 9)
        else:
            #print "waterr: %f" % value
            return -math.ceil(((0.5 - value) / 0.49) * 9)
        
    def generate_map(self):
        import time     
        for y in range(0, self.map.get_map_height()):
            for x in range(0, self.map.get_map_width()):
                p = 0.01*vec3(x + self.x_add, y + self.y_add, self.z)
                h = fBm(p, octaves=self.oct, lacunarity=self.lac, gain=self.gain)
                elev = self.val_to_elev(h)
                #print "%d,%d -> %d" % (x,y,elev)
                self.map.set_hex_elevation(x, y, elev)
                #print "N%d" %self.map.get_hex_elevation(x, y, )
                #time.sleep(0.01)
        
map = MuxMap(dimensions=(300,300))

gen = fBmMapGenerator(map)
gen.generate_map()

#print "50,50 -> %d" % map.get_hex_elevation(50,10)

img = PixelHexMapImage(map)
#img.debug = True
img.generate_map()
img.show()