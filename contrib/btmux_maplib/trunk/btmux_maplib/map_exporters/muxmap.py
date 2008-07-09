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
class MuxMapExporter(object):
    """
    Saves as map as a MUX .map file.
    """
    # Reference to the map object to save.
    map = None
    # Reference to the file-like object to save to.
    fobj = None
    # When True, print debug stuff to stdio.
    debug = False
    
    def __init__(self, map, fileobj):
        self.map = map
        self.fobj = fileobj
        
    def write(self):
        """
        Writes the map to the specified file.
        """
        if self.debug and getattr(self.fobj, "name", None):
            print "Writing MUX map to: %s" % self.fobj.name
            
        self.fobj.writelines(self.map_lines())
        
        if self.debug:
            print "Finished writing MUX map."
        
    def close(self):
        """
        Closes the file descriptor.
        """
        self.fobj.close()
        
    def map_lines(self):
        """
        Returns a string containing what would be written to a .map file.
        """
        if self.debug:
            print "* Creating row list."
        hex_rows = []
        for y in range(0, self.map.get_map_height()):
            hex_rows.append([])
            hex_rows[y] = ""
            for x in range(self.map.get_map_width()):
                hex_rows[y] += "%s%s" % (self.map.get_hex_terrain(x, y),
                                             self.map.get_hex_elevation(x, y))
            hex_rows[y] += "\n"
            
        if self.debug:
            print "* %d rows dumped." % (len(hex_rows))
            
        hex_rows.insert(0, "%d %d\n" % (
                    self.map.get_map_width(),
                    self.map.get_map_height()))
        return hex_rows