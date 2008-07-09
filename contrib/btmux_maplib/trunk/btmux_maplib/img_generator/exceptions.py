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
class BasicImagingException(Exception):
    """
    Basic exception for others to inherit or use.
    """
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)
    
class InvalidImageMode(BasicImagingException):
    """
    Raised when someone tries to set an invalid map imaging mode.
    """
    def __str__(self):
        return repr("Invalid imaging mode: %s" % self.value)