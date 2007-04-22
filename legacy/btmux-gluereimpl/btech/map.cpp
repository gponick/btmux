#include <string>

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "externs.h"
#include "timeutil.h"
#include "svdhash.h"

#include "btech.h"
#include "map.h"

MAP::MAP(dbref object, int x, int y) {
	m_dbref = object;
	setattr("width", tprintf("%d",x));
	setattr("height", tprintf("%d",y));
	setattr("temperature", "30");
	setattr("gravity", "100");
	std::string row("");

	for(int j = 0; j<y; j++) {
		for(int i = 0; i<x; i++) 
			row += '.';	
		setattr(tprintf("map_%d", j), row);
		row = "";
	}

}

MAP::MAP(void) {

}

MAP::~MAP(void) {

}

void MAP::load() {

}

void MAP::save() {

}

void MAP::update() {

}

int MAP::size() {
    return sizeof(this);
}

void MAP::View(dbref executor, int x, int y) {
	for(int j=0; j<getattrn("height"); j++) 
		notify(executor, getattr(tprintf("map_%d", j)).c_str());
}

int MAP::HandledCommand(dbref executor, char *pCommand) {
	if(!string_match(pCommand,"view")) {
		View(executor, 0, 0);
		return 1;
	} else
		return 0;
}

inline void MAP::TACSketchRow(char *pos, int left_offset, char const *src, int len)
{
    memset(pos, ' ', left_offset);
    memcpy(pos + left_offset, src, len);
    pos[left_offset + len] = '\0';
}

void MAP::TACSketchMap(char *buf, MECH * mech, int sx, int sy, int wx, int wy, 
	int dispcols, int top_offset, int left_offset, int docolour, int dohexlos)
{
    static char const hexrow[2][311] = {    
        "\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/]["
        "\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/]["
        "\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/]["
        "\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/]["
        "\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/",
        "/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\]["
        "/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\]["
        "/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\]["
        "/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\]["
        "/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\"
    };
    int x, y;
    int oddcol1 = Is_OddCol(sx);	/* One iff first hex col is odd */
    char *pos;
    int mapcols = TACDispCols(wx);
    hexlosmap_info * losmap = NULL;

		// First create a blank hex map.
    pos = buf;
    for (y = 0; y < top_offset; y++) {
		memset(pos, ' ', dispcols - 1);
		pos[dispcols - 1] = '\0';
		pos += dispcols;
    }
    for (y = 0; y < wy; y++) {
		TACSketchRow(pos, left_offset, hexrow[oddcol1], mapcols);
		pos += dispcols;
		TACSketchRow(pos, left_offset, hexrow[!oddcol1], mapcols);
		pos += dispcols;
    }
    TACSketchRow(pos, left_offset, hexrow[oddcol1], mapcols);

		// Now draw the terrain and elevation. 
    pos = buf + top_offset * dispcols + left_offset;
	wx = (wx < (getattrn("width") - sx) ? wx : (getattrn("width") - sx));
	wy = (wy < (getattrn("height") - sy) ? wy : (getattrn("height") -sy));

//    if (dohexlos)
//		losmap = CalculateLOSMap(mech, (0 > sx ? 0 : sx), (0 > sy ? 0 : sy), wx, wy);

	for (y = (0 > -sy ? 0 : -sy); y < wy; y++) {
		for (x = (0 > -sx ? 0 : -sx); x < wx; x++) {
			int terr, elev, losflag = MAPLOSHEX_SEE | MAPLOSHEX_SEEN;
			char *base;
			char topchar, botchar;
	    
//			if (losmap)
//				losflag = ((losmap)->map[LOSMap_Hex2Index(losmap, sx+x, sy+y)]);

			if (!(losflag & MAPLOSHEX_SEEN)) {
				terr = 'X';
				elev = 40; /* 'X' */
			} else {
				if (losflag & MAPLOSHEX_SEETERRAIN)
				    terr = GetTerrain(sx + x, sy + y);
				else
					terr = UNKNOWN_TERRAIN;
		
				if (losflag & MAPLOSHEX_SEEELEV)
					elev = GetElevation(sx + x, sy + y);
				else
					elev = 15; /* Ugly hack: '0' + 15 == '?' */
			}
			base = pos + TACHexOffset(x, y, dispcols, oddcol1);

			switch (terr) {
				case WATER:
 					 // Colour hack:  Draw deep water with '\242' if using colour so 
					 // colourize_TACmap() knows to use dark blue rather than light blue
					if (docolour && elev >= 2) {
						topchar = '\242';
						botchar = '\242';
					} else {
						topchar = '~';
						botchar = '~';
					}
					break;
				case HIGHWATER:
					topchar = '~';
					botchar = '+';
					break;
				case BRIDGE:
					topchar = '#';
					botchar = '+';
					break;
				case ' ':		/* GRASSLAND */
					topchar = ' ';
					botchar = '_';
					break;
				case UNKNOWN_TERRAIN:
					topchar = '?';
					botchar = '?';
					break;
				default:
					topchar = terr;
					botchar = terr;
					break;
			}

			base[0] = topchar;
			base[1] = topchar;
			base[dispcols + 0] = botchar;
			if (elev > 0) {
				botchar = '0' + elev;
			}
			base[dispcols + 1] = botchar;
		}
    }
}

	// Draw one of the seven hexes that a Dropship takes up on a tac map.
void MAP::TACSketchDS(char *base, int dispcols, char terr)
{
    // Be careful not to overlay a 'mech id or terrain elevation.
    if (!mux_isalpha((unsigned char) base[0])) {
		base[0] = terr;
		base[1] = terr;
    }
    base[dispcols + 0] = terr;
    if (!mux_isdigit((unsigned char) base[dispcols + 1])) 
		base[dispcols + 1] = terr;
}

void MAP::TACSketchOwnMech(char *buf, MECH * mech, int sx,
    int sy, int wx, int wy, int dispcols, int top_offset, int left_offset)
{
    int oddcol1 = Is_OddCol(sx);
    char *pos = buf + top_offset * dispcols + left_offset;
    char *base;
    int x = MechX(mech) - sx;
    int y = MechY(mech) - sy;

    if (x < 0 || x >= wx || y < 0 || y >= wy) {
		return;
    }
    base = pos + TACHexOffset(x, y, dispcols, oddcol1);
    base[0] = '*';
}

void MAP::TACSketchMechs(char *buf, MECH * player_mech, int sx, int sy, int wx, int wy, 
	int dispcols, int top_offset, int left_offset, int docolor, int labels)
{
    int i;
    char *pos = buf + top_offset * dispcols + left_offset;
    int oddcol1 = Is_OddCol(sx);

		// Draw all the 'mechs on the map.
    for (i = 0; i < first_free; i++) {
		int x, y;
		char *base;
		MECH *mech;

		if (mechsOnMap[i] == -1) 
		    continue;
		
		mech = getMech(mechsOnMap[i]);
		if (mech == NULL) 
			continue;
		
		// Check to see if the 'mech is on the tac map and that its in LOS of the player's 'mech.
		x = MechX(mech) - sx;
		y = MechY(mech) - sy;
		if (x < 0 || x >= wx || y < 0 || y >= wy) 
			continue;
		
		if (mech != player_mech && !InLineOfSight(player_mech, mech, MechX(mech), MechY(mech), FlMechRange(player_mech, mech))) 
		    continue;

		base = pos + TACHexOffset(x, y, dispcols, oddcol1);
		if (!(MechSpecials2(mech) & CARRIER_TECH) && IsDS(mech) && ((MechZ(mech) >= ORBIT_Z && mech != player_mech) || Landed(mech) || !Started(mech))) {
		    int ts = DSBearMod(mech);
		    int dir;

			// Dropships are a special case.  They take up seven hexes on a tac map.  First draw the center hex and then the six surronding hexes.
		    if (docolor)	// Colour hack: 'X' would be confused with any enemy con by colourize_TACmap()
				TACSketchDS(base, dispcols, '$');
		    else 
				TACSketchDS(base, dispcols, 'X');

		    for (dir = 0; dir < 6; dir++) {
				int tx = x + dirs[dir][0];
				int ty = y + dirs[dir][1];

				if ((tx + oddcol1) % 2 == 0 && dirs[dir][0] != 0) 
				    ty--;
				if (tx < 0 || tx >= wx || ty < 0 || ty >= wy) 
					continue;

				base = pos + TACHexOffset(tx, ty, dispcols, oddcol1);
				if (Find_DS_Bay_Number(mech, (dir - ts + 6) % 6) >= 0) 
				    TACSketchDS(base, dispcols, '@');
				else 
					TACSketchDS(base, dispcols, '=');
			}
		} else 
			if (mech == player_mech) {
				base[0] = '*';
				base[1] = '*';
			} else {
				char *id = MechIDS(mech, MechSeemsFriend(player_mech, mech));
				base[0] = id[0];
				base[1] = id[1];
			}
	}
}

void MAP::TACSketchCliffs(char *buf, int sx, int sy, int wx, int wy, 
	int dispcols, int top_offset, int left_offset, int cliff_size)
{
    char *pos = buf + top_offset * dispcols + left_offset;
    int y, x;
    int oddcol1 = Is_OddCol(sx);

	wx = (wx < getattrn("width") - sx ? wx : getattrn("width") - sx);
	wy = (wy < getattrn("height") - sy ? wy : getattrn("height") - sy);
	for (y = (0 > -sy ? 0 : -sy); y < wy; y++) {
		int ty = sy + y;
		for (x = (0 > -sx ? 0 : -sx); x < wx; x++) {
		    int tx = sx + x;
		    int oddcolx = Is_OddCol(tx);
			int elev = GetElevation(tx, ty);
		    char *base = pos + TACHexOffset(x, y, dispcols, oddcol1);
			char c;

			// Copy the elevation up to the top of the hex so we can draw a bottom hex edge on every hex.
			c = base[dispcols + 1];
			if (base[0] == '*') {
				base[0] = '*';
				base[1] = '*';
			} else 
				if (mux_isdigit((unsigned char) c)) 
					base[1] = c;

				 // For each hex on the map check to see if each of it's 240, 180, and 120 hex sides is a cliff. 
				 // Don't check for cliffs between hexes that are on the tac map and those that are off of it.

		    if (x != 0 && (y < wy - 1 || oddcolx) && abs(GetElevation(tx - 1, ty + 1 - oddcolx) - elev) >= cliff_size) 
				base[dispcols - 1] = '|';

			if (y < wy - 1 && abs(GetElevation(tx, ty + 1) - elev) >= cliff_size) {
				base[dispcols] = ',';
				base[dispcols + 1] = ',';
		    } else {
				base[dispcols] = '_';
				base[dispcols + 1] = '_';
			}
			if (x < wx - 1 && (y < wy - 1 || oddcolx) && abs(GetElevation(tx + 1, ty + 1 - oddcolx) - elev) >= cliff_size)
				base[dispcols + 2] = '!';
		}
    }
}

void MAP::TACSketchDSLZ(char *buf, MECH * mech, int sx, int sy, int wx, int wy, 
		int dispcols, int top_offset, int left_offset, int cliff_size, int docolor)
{
    char *pos = buf + top_offset * dispcols + left_offset;
    int y, x;
    int oddcol1 = Is_OddCol(sx);

	wx = (wx < getattrn("width") - sx ? wx : getattrn("width") - sx);
    wy = (wy < getattrn("height") - sy ? wy : getattrn("height") - sy);
	for (y = (0 > -sy ? 0 : -sy); y < wy; y++) {
		int ty = sy + y;

		for (x = (0 > -sx ? 0 : -sx); x < wx; x++) {
			int tx = sx + x;
			char *base = pos + TACHexOffset(x, y, dispcols, oddcol1);

			if (ImproperLZ(mech, tx, ty))
				base[dispcols] = docolor ? '\241' : 'X';
			else
				base[dispcols] = docolor ? '\240' : 'O';
		}
    }
}

// Colorize a sketch tac map.  Uses dynmaically allocated buffers which are overwritten on each call.
char **MAP::TACColorizeMap(char const *sketch, int dispcols, int disprows)
{
    static char *buf = NULL;
    static int buf_len = 5000;
    static char **lines = NULL;
    static int lines_len = 100;
    int pos = 0;
    int line = 0;
    unsigned char cur_colour = '\0';
    const char *line_start;
    char const *src = sketch;

    if (buf == NULL) 
		Create(buf, char, buf_len);
    
    if (lines == NULL) 
		Create(lines, char *, lines_len);
    
    line_start = (char *) src;
    lines[0] = buf;
    while (lines > 0) {
		unsigned char new_colour;
		unsigned char c = *src++;

		if (c == '\0') {	// End of line.
		    if (cur_colour != '\0') {
				buf[pos++] = '%';
				buf[pos++] = 'c';
				buf[pos++] = 'n';
			}
			buf[pos++] = '\0';
			line++;
			if (line >= disprows)
				break;		/* Done */
			if (line + 1 >= lines_len) {
				lines_len *= 2;
				ReCreate(lines, char *, lines_len);
			}
			line_start += dispcols;
			src = line_start;
			lines[line] = buf + pos;
			continue;
		}

		switch (c) {
			case (unsigned char) '\242':	/* Colour Hack: Deep Water */
			    c = '~';
			    new_colour = custom_color_str[DWATER_IDX];
			    break;
			case (unsigned char) '\241':	/* Colour Hack: improper LZ */
			    c = 'X';
				new_colour = custom_color_str[BADLZ_IDX];
				break;
			case (unsigned char) '\240':	/* Colour Hack: proper LZ */
				c = 'O';
				new_colour = custom_color_str[GOODLZ_IDX];
				break;
			case '?':
				c = '?';
				new_colour = custom_color_str[UNKNOWN_IDX];
				break;
			case '$':		/* Colour Hack: Drop Ship */
			    c = 'X';
				new_colour = custom_color_str[DS_IDX];
				break;
			case '!':		/* Cliff hex edge */
				c = '/';
				new_colour = custom_color_str[CLIFF_IDX];
				break;
			case '|':		/* Cliff hex edge */
				c = '\\';
				new_colour = custom_color_str[CLIFF_IDX];
				break;
			case ',':		/* Cliff hex edge */
				c = '_';
				new_colour = custom_color_str[CLIFF_IDX];
				break;
			case '*':		/* mech itself. */
				new_colour = custom_color_str[SELF_IDX];
				break;
			default:
				if (mux_islower(c)) 	/* Friendly con */
					new_colour = custom_color_str[FRIEND_IDX];
				else 
					if (mux_isupper(c)) 	/* Enemy con */
						new_colour = custom_color_str[ENEMY_IDX];
					else 
						if (mux_isdigit(c)) 	/* Elevation */
							new_colour = cur_colour;
						else
							new_colour = TerrainColorChar(c, 0);
				break;
		}

		if (mux_isupper(new_colour) != mux_isupper(cur_colour))
		    if (mux_isupper(new_colour)) {
				buf[pos++] = '%';
				buf[pos++] = 'c';
				buf[pos++] = 'h';
			} else {
				buf[pos++] = '%';
				buf[pos++] = 'c';
				buf[pos++] = 'n';
				cur_colour = '\0';
			}

		if (mux_tolower(new_colour) != mux_tolower(cur_colour)) {
		    buf[pos++] = '%';
		    buf[pos++] = 'c';
		    if (new_colour == '\0')
				buf[pos++] = 'n';
			else 
				if (new_colour == 'H') {
					buf[pos++] = 'n';
					buf[pos++] = '%';
					buf[pos++] = 'c';
					buf[pos++] = mux_tolower(new_colour);
				} else 
					buf[pos++] = mux_tolower(new_colour);		
		    cur_colour = new_colour;
		}

		buf[pos++] = c;
		if (pos + 11 > buf_len) {
			// If we somehow run out of room then we don't bother to reallocate 'buf' and potentially have
			// a bunch of invalid pointers in 'lines' to fix up. We just restart from scratch with a bigger 'buf'.
			buf_len *= 2;
			free(buf);
			buf = NULL;
			return TACColorizeMap(sketch, dispcols, disprows);
		}
    }
    lines[line] = NULL;
    return lines;
}

/*
 * Draw a tac map for the TACTICAL and NAVIGATE commands.
 *
 * This used to be "one MOFO of a function" but has been simplified
 * in a number of ways.  One is that it used to statically allocated
 * buffers which limit the map drawn to MAP_DISPLAY_WIDTH hexes across
 * and 24 hexes down in size.  The return value should no longer be
 * freed with KillText().
 *
 * player   = dbref of player wanting map (mostly irrelevant)
 * mech     = mech player's in (or NULL, if on map) 
 * map      = map obj itself
 * cx       = middle of the map (x)
 * cy       = middle of the map (y)
 * wx       = width in x
 * wy       = width in y
 * labels   = bit array
 *    1 = the 'top numbers'
 *    2 = the 'side numbers'
 *    4 = navigate mode
 *    8 = show mech cliffs
 *   16 = show tank cliffs
 *   32 = show DS LZ's
 *
 * If navigate mode, wx and wy should be equal and odd.  Navigate maps
 * cannot have top or side labels.
 *
 */

char **MAP::MakeMapText(dbref player, MECH * mech, int cx, int cy, int labels, int dohexlos)
{
    int docolor = Ansimap(player);
    int dispcols, disprows, mapcols;
    int left_offset = 0;
    int top_offset = 0;
    int navigate = 0;
    int sx, sy, wx, wy, i;
    char *base, *str;
    int oddcol1;
    enum {
		MAX_WIDTH = 40,
		MAX_HEIGHT = 24,
		TOP_LABEL = 3,
		LEFT_LABEL = 4,
		RIGHT_LABEL = 3
    };
    static char sketch_buf[((LEFT_LABEL + 1 + MAX_WIDTH * 3 + RIGHT_LABEL + 1) * (TOP_LABEL + 1 + MAX_HEIGHT * 2) + 2) * 5];
    static char *lines[(TOP_LABEL + 1 + MAX_HEIGHT * 2 + 1) * 5];

		// Get the Tacsize attribute 'even tho it says tacheight' from the player, if doesn't exist set the height and width to
		// default params. If it does exist, check the values and make sure they are legit. 
//    str = silly_atr_get(player, A_TACHEIGHT);
	str = "24 40";
    if (!*str) {
		wx = MAP_DISPLAY_WIDTH;
	    wy = MAP_DISPLAY_HEIGHT;
    } else 
		if (sscanf(str, "%d %d", &wy, &wx) != 2 || wy > 24 || wy < 5 || wx < 5 || wx > 40) {
			    notify(player, "Illegal Tacsize attribute. Must be in format 'Height Width' . Height : 5-24 Width : 5-40");
		        wx = MAP_DISPLAY_WIDTH;
			    wy = MAP_DISPLAY_HEIGHT;
		}

    /* Everything worked but lets check the map size */
    wy = (wy <= getattrn("height")) ? wy : getattrn("height");
    wx = (wx <= getattrn("width")) ? wx : getattrn("width");

    if (labels & 4) {
		navigate = 1;
		labels = 0;
    }

		// Figure out the extent of the tac map to draw.  
	wx = (MAX_WIDTH < wx ? MAX_WIDTH : wx);
	wy = (MAX_HEIGHT < wy ? MAX_HEIGHT : wy);

    sx = cx - wx / 2;
    sy = cy - wy / 2;
    if (!navigate) {
		// Only allow navigate maps to include off map hexes.
		sx = (0 > (sx < (getattrn("width") - wx) ? sx : (getattrn("width") - wx)) ? 0 : (sx < (getattrn("width") - wx) ? sx : (getattrn("width") - wx)));
		sy = (0 > (sy < (getattrn("height") - wy) ? sy : (getattrn("height") - wy)) ? 0 : (sy < (getattrn("height") - wy) ? sy : (getattrn("height") - wy)));
		wx = (wx < getattrn("width") ? wx : getattrn("width"));
		wy = (wy < getattrn("height") ? wy : getattrn("height"));
    }

    mapcols = TACDispCols(wx);
    dispcols = mapcols + 1;
    disprows = wy * 2 + 1;
    oddcol1 = Is_OddCol(sx);

    if (navigate)
	    if (oddcol1) {
	         // Insert blank line at the top where we can put a "__" to make the navigate map look pretty.
	        top_offset = 1;
	        disprows++;
	    }
    else {
	    /*
	     * Allow room for the labels.
	     */
	    if (labels & 1) {
    	    left_offset = LEFT_LABEL;
	        dispcols += LEFT_LABEL + RIGHT_LABEL;
	    }
	    if (labels & 2) {
	        top_offset = TOP_LABEL;
	        disprows += TOP_LABEL;
	    }
    }

		// Create a sketch tac map including terrain and elevation.
    TACSketchMap(sketch_buf, mech, sx, sy, wx, wy, dispcols, top_offset, left_offset, docolor, dohexlos);

		// Draw the top and side labels.
    if (labels & 1) 
	    for (int x = 0; x < wx; x++) {
	        char scratch[4];
	        int label = sx + x;

	        if (label < 0 || label > 999) 
		        continue;

	        sprintf(scratch, "%3d", label);
	        base = sketch_buf + left_offset + 1 + x * 3;
	        base[0] = scratch[0];
	        base[1 * dispcols] = scratch[1];
	        base[2 * dispcols] = scratch[2];
	    }

    if (labels & 2) 
	    for (int y = 0; y < wy; y++) {
	        int label = sy + y;

	        base = sketch_buf + (top_offset + 1 + y * 2) * dispcols;
	        if (label < 0 || label > 999) 
		        continue;

	        sprintf(base, "%3d", label);
	        base[3] = ' ';
	        sprintf(base + (dispcols - RIGHT_LABEL - 1), "%3d", label);
	    }

    if (labels & 8) {
    	if (mech != NULL) 
	        TACSketchOwnMech(sketch_buf, mech, sx, sy, wx, wy, dispcols, top_offset, left_offset);
	    TACSketchCliffs(sketch_buf, sx, sy, wx, wy, dispcols, top_offset, left_offset, 3);
    } else 
		if (labels & 16) {
	        if (mech != NULL) 
	            TACSketchOwnMech(sketch_buf, mech, sx, sy, wx, wy, dispcols, top_offset, left_offset);	        
	        TACSketchCliffs(sketch_buf, sx, sy, wx, wy, dispcols, top_offset, left_offset, 2);
        } else 
    		if (labels & 32) {
	            if (mech != NULL) 
	                TACSketchOwnMech(sketch_buf, mech, sx, sy, wx, wy, dispcols, top_offset, left_offset);       
	            TACSketchDSLZ(sketch_buf, mech, sx, sy, wx, wy, dispcols, top_offset, left_offset, 2, docolor);
            } else 
     			if (mech != NULL) 
	                TACSketchMechs(sketch_buf, mech, sx, sy, wx, wy, dispcols, top_offset, left_offset, docolor, labels);
         
    if (navigate) {
	    int n = wx / 2;		/* Hexagon radius */
		if (oddcol1) 		// Navigate hack: erase characters from the sketch map to turn it into a pretty hexagonal shaped map.
			disprows--;		// Don't need the last line in this case. 

		for (i = 0; i < n; i++) {
		    int len;
		    base = sketch_buf + (i + 1) * dispcols + left_offset;
		    len = (n - i - 1) * 3 + 1;
		    memset(base, ' ', len);
		    base[len] = '_';
			base[len + 1] = '_';
		    base[mapcols - len - 2] = '_';
		    base[mapcols - len - 1] = '_';
			base[mapcols - len] = '\0';
			base = sketch_buf + (disprows - i - 1) * dispcols + left_offset;
		    len = (n - i) * 3;
		    memset(base, ' ', len);
			base[mapcols - len] = '\0';
		}

		memset(sketch_buf + left_offset, ' ', n * 3 + 1);
		sketch_buf[left_offset + n * 3 + 1] = '_';
		sketch_buf[left_offset + n * 3 + 2] = '_';
		sketch_buf[left_offset + n * 3 + 3] = '\0';
    }

		// If using color then colorize the sketch map and return the result.
    if (docolor) 
		return TACColorizeMap(sketch_buf, dispcols, disprows);
    
		// If not using color, the sketch map can be used as is.
    for (i = 0; i < disprows; i++) 
		lines[i] = sketch_buf + dispcols * i;

    lines[i] = NULL;
    return lines;
}

// map creation private methods
int MAP::GetIndex(char terrain, char elevation)
{
    int i;

    if ((i = data_to_id[(short) terrain][(short) elevation]))
	return i - 1;
    id_to_data[first_free].terrain = terrain;
    id_to_data[first_free].elev = elevation;
    first_free++;
    data_to_id[(short) terrain][(short) elevation] = first_free;
    return first_free - 1;
}

char MAP::GetElevation(int index)
{
    return id_to_data[index].elev;
}

char MAP::GetElevation(int x, int y)
{
    std::string row = getattr(tprintf("map_%d", y));

    return row[x*2 - 1];
}

char MAP::GetTerrain(int index)
{
    return id_to_data[index].terrain;
}

char MAP::GetTerrain(int x, int y)
{
	std::string row = getattr(tprintf("map_%d", y));
    
	return row[x*2];
}

inline char MAP::TerrainColorChar(char terrain, int elev)
{
    switch (terrain) {
    case HIGHWATER:
        return custom_color_str[DWATER_IDX];
    case WATER:
        if (elev < 2 || elev == '0' || elev == '1' || elev == '~')
            return custom_color_str[SWATER_IDX];
        return custom_color_str[DWATER_IDX];
    case BUILDING:
        return custom_color_str[BUILDING_IDX];
    case ROAD:
        return custom_color_str[ROAD_IDX];
    case ROUGH:
        return custom_color_str[ROUGH_IDX];
    case MOUNTAINS:
        return custom_color_str[MOUNTAIN_IDX];
    case FIRE:
        return custom_color_str[FIRE_IDX];
    case ICE:
        return custom_color_str[ICE_IDX];
    case WALL:
        return custom_color_str[WALL_IDX];
    case SNOW:
        return custom_color_str[SNOW_IDX];
    case SMOKE:
        return custom_color_str[SMOKE_IDX];
    case LIGHT_FOREST:
        return custom_color_str[LWOOD_IDX];
    case HEAVY_FOREST:
        return custom_color_str[HWOOD_IDX];
    case UNKNOWN_TERRAIN:
        return custom_color_str[UNKNOWN_IDX];
    }
    return '\0';
}

char *MAP::add_color(char newc, char * prevc, char c)
{
    static char buf[10]; /* won't be filled with more than 7 characters */
    buf[0] = '\0';

    if (newc == *prevc) {
        buf[0] = c;
        buf[1] = '\0';
        return buf;
    }

    if (!newc || ((mux_isupper(*prevc)) && !mux_isupper(newc)) ||
        (newc == 'H' && *prevc))
        strcpy(buf, "%cn");
    else if (mux_isupper(newc) && !mux_isupper(*prevc))
        strcpy(buf, "%ch");

    if (!newc)
        sprintf(buf + strlen(buf), "%c", c);
    else
        sprintf(buf + strlen(buf), "%%c%c%c", mux_tolower(newc), c);
    *prevc = newc;
    return buf;
}
