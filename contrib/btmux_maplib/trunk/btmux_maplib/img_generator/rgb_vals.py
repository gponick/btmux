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
Holds all of the RGB values for terrain-elevation combinations.
"""
# Dictionary of terrain type characters and their elevation's RGB values.
cmap = {}

# Water is backwards, since it's negative Z.
cmap["~"] = [(8,47,170), # 0z
             (8,44,160),
             (8,41,150),
             (8,38,140),
             (8,35,130),
             (8,32,120),
             (8,29,110),
             (8,26,100),
             (8,23,90),                                                                                              
             (8,20,80)] # -9z


cmap["."] = [(200,200,200), # 0z
             (205,205,205),
             (210,210,210),
             (215,215,215),
             (220,220,220),
             (225,225,225),
             (230,230,230),
             (235,235,235),
             (240,240,240),
             (245,245,245)] # 9z

cmap["^"] = [(68,38,8), # 0z
             (71,41,11),
             (74,44,14),
             (77,47,17),
             (80,50,20),
             (83,53,23),
             (86,56,26),
             (89,59,29),
             (92,62,32),
             (95,65,35)] # 9z

cmap["%"] = [(114,114,64), # 0z
             (117,117,64),
             (120,120,64),
             (123,123,64),
             (126,126,64),
             (129,129,64),
             (132,132,64),
             (135,135,64),
             (138,138,64),
             (142,142,64)] # 9z

cmap["#"] = [(53,53,53), # 0z
             (60,60,60),
             (67,67,67),
             (74,74,74),
             (81,81,81),
             (88,88,88),
             (95,95,95),
             (102,102,102),
             (109,109,109),
             (116,116,116)] # 9z

cmap["`"] = [(93,165,66), # 0z
             (93,168,66),
             (93,171,66),
             (93,174,66),
             (93,177,66),
             (93,180,66),
             (93,183,66),
             (93,186,66),
             (93,189,66),
             (93,192,66)] # 9z

# Some maps/map editors use a single quote for light forest (which is bad).
# Alias it here.
cmap["'"] = cmap["`"]

cmap["\""] =[(76,114,78), # 0z
             (78,118,76),
             (80,122,74),
             (82,126,72),
             (84,130,70),
             (86,134,68),
             (88,138,66),
             (90,142,64),
             (92,146,62),
             (94,150,60)] # 9z

cmap["/"] = [(0,240,240), # 0z
             (0,235,235),
             (0,230,230),
             (0,225,225),
             (0,220,220),
             (0,215,215),
             (0,210,210),
             (0,205,205),
             (0,200,200),
             (0,195,195)] # 9z

cmap["@"] = [(150,0,150), # 0z
             (160,0,160),
             (170,0,170),
             (180,0,180),
             (190,0,190),
             (200,0,200),
             (210,0,210),
             (220,0,220),
             (230,0,230),
             (240,0,240)] # 9z

cmap["}"] = [(200,200,0), # 0z
             (205,205,0),
             (210,210,0),
             (215,215,0),
             (220,220,0),
             (225,225,0),
             (230,230,0),
             (235,235,0),
             (240,240,0),
             (245,245,0)] # 9z

cmap["="] = [(75,150,150), # 0z
             (80,160,160),
             (85,170,170),
             (90,180,180),
             (95,190,190),
             (100,200,200),
             (105,210,210),
             (110,220,220),
             (115,230,230),
             (120,240,240)] # 9z

cmap["&"] = [(150,30,0), # 0z
             (165,33,0),
             (180,36,0),
             (190,39,0),
             (200,42,0),
             (210,45,0),
             (220,48,0),
             (230,51,0),
             (240,54,0),
             (250,57,0)] # 9z

# Alias ice to water.
cmap["-"] = cmap["~"]
# Snow to clear.
cmap["+"] = cmap["."]
