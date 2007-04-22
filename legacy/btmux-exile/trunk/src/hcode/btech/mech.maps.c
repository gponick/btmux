#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "create.h"
#include "mech.events.h"
#include "p.mech.utils.h"
#include "p.mech.los.h"
#include "p.eject.h"
#include "p.mech.restrict.h"
#include "p.mech.maps.h"
#include "p.ds.bay.h"
#include "p.bsuit.h"
#include "p.autopilot.h"
#include "mech.sensor.h" 
#include "mech.ecm.h"

void mech_findcenter(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    float fx, fy;
    int x, y;

    cch(MECH_USUAL);
    x = MechX(mech);
    y = MechY(mech);
    MapCoordToRealCoord(x, y, &fx, &fy);
    notify(player, tprintf("Current hex: (%d,%d,%d)\tRange to center: %.2f\tBearing to center: %d",
	    x, y, MechZ(mech), FindHexRange(fx, fy, MechFX(mech), MechFY(mech)),
	    FindBearing(MechFX(mech), MechFY(mech), fx, fy)));
}

#define NAVIGATE_LINES 13
#define NAVIGATE_CENTERX 14
#define NAVIGATE_CENTERY 6

#define NAVIGATE_MAX_X_DIFF 9
#define NAVIGATE_MAX_Y_DIFF 4

const char *GetTerrainName_base(int t)
{
    switch (t) {
    case GRASSLAND:
    case '_':
	return "Grassland";
    case HEAVY_FOREST:
	return "Heavy Forest";
    case LIGHT_FOREST:
	return "Light Forest";
    case ICE:
	return "Ice";
    case BRIDGE:
	return "Bridge";
    case DBRIDGE:
	return "Damaged Bridge";
    case HIGHWATER:
    case WATER:
	return "Water";
    case ROUGH:
	return "Rough";
    case MOUNTAINS:
	return "Mountains";
    case ROAD:
	return "Road";
    case BUILDING:
	return "Building";
    case FIRE:
	return "Fire";
    case SMOKE:
	return "Smoke";
    case HSMOKE:
	return "Heavy Smoke";
    case WALL:
	return "Wall";
    case SNOW:
	return "Snow";
    case DESERT:
	return "Desert";
    }
    return "Unknown";
}

const char *GetTerrainName(MAP * map, int x, int y)
{
    return GetTerrainName_base(GetTerrain(map, x, y));
}

#define ShowMech(mech,letter) \
  bearing = FindBearing(fx, fy, MechFX(mech), MechFY(mech)); \
  range = FindHexRange(fx, fy, MechFX(mech), MechFY(mech)); \
  if (range > 0.5) \
    range = 0.5; \
  bearing = AcceptableDegree(bearing + 180); \
  bear = (bearing + 90) * TWOPIOVER360; \
  tx = NAVIGATE_CENTERX + range * NAVIGATE_MAX_X_DIFF * 2 * (f1=cos(bear)); \
  ty = NAVIGATE_CENTERY + range * NAVIGATE_MAX_Y_DIFF * 2 * (f2=sin(bear)); \
  tx = BOUNDED(NAVIGATE_CENTERX - NAVIGATE_MAX_X_DIFF, tx,  \
	       NAVIGATE_CENTERX + NAVIGATE_MAX_X_DIFF); \
  ty = BOUNDED(NAVIGATE_CENTERY - NAVIGATE_MAX_Y_DIFF, ty,  \
	       NAVIGATE_CENTERY + NAVIGATE_MAX_Y_DIFF); \
  mybuff[ty][tx] = letter;

void mech_navigate(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *t;
    char mybuff[NAVIGATE_LINES][MBUF_SIZE];
    int i = 0;
    MAP *mech_map;
    char **maptext;
    float fx, fy;
    int bearing;
    float bear;
    float range;
    int tx, ty;
    float f1, f2;

    cch(MECH_USUAL);

    MapCoordToRealCoord(MechX(mech), MechY(mech), &fx, &fy);
    mech_map = getMap(mech->mapindex);
    if (mech_map->map_width > 0 && mech_map->map_height > 0) {
	maptext =
	    MakeMapText(player, mech, mech_map, MechX(mech), MechY(mech),
	    5, 5, 4);
    } else {
	static char dummy[] = "";
	static char *dummy2[12] =
	    { dummy, dummy, dummy, dummy, dummy, dummy,
	    dummy, dummy, dummy, dummy, dummy, NULL
	};

	maptext = dummy2;
    }

    sprintf(mybuff[0],
	"              0                                          %.150s",
	maptext[0]);
    sprintf(mybuff[1],
	"         ___________                                     %.150s",
	maptext[1]);
    sprintf(mybuff[2],
	"        /           \\          Location:%4d,%4d, %3d   %.150s",
	MechX(mech), MechY(mech), MechZ(mech), maptext[2]);
    sprintf(mybuff[3],
	"  300  /             \\  60     Terrain: %14s   %.150s",
	GetTerrainName(mech_map, MechX(mech), MechY(mech)), maptext[3]);
    sprintf(mybuff[4],
	"      /               \\                                  %.150s",
	maptext[4]);
    sprintf(mybuff[5],
	"     /                 \\                                 %.150s",
	maptext[5]);
    sprintf(mybuff[6],
	"270 (                   )  90  Speed:           %6.1f   %.150s",
	MechSpeed(mech), maptext[6]);
    sprintf(mybuff[7],
	"     \\                 /       Vertical Speed:  %6.1f   %.150s",
	MechVerticalSpeed(mech), maptext[7]);
    sprintf(mybuff[8],
	"      \\               /        Heading:           %4d   %.150s",
	MechFacing(mech), maptext[8]);
    sprintf(mybuff[9],
	"  240  \\             /  120                              %.150s",
	maptext[9]);
    sprintf(mybuff[10],
	"        \\___________/                                    %.150s",
	maptext[10]);
    sprintf(mybuff[11], "                      ");
    sprintf(mybuff[12], "             180");

#if 0
    for (i = 0; i < mech_map->first_free; i++) {
	if (mech_map->mechsOnMap[i] < 0)
	    continue;
	if (!(t = FindObjectsData(mech_map->mechsOnMap[i])))
	    continue;
	if (t == mech)
	    continue;
	if (MechX(t) != MechX(mech) || MechY(t) != MechY(mech))
	    continue;
	if (!InLineOfSight(mech, t, MechX(mech), MechY(mech), 0.0))
	    continue;
	ShowMech(t, MechTeam(mech) == MechTeam(t) ? 'x' : 'X');
    }
    ShowMech(mech, '*');
    for (i = 0; i < NAVIGATE_LINES; i++)
	notify(player, mybuff[i]);
#endif
    navigate_sketch_mechs(mech, mech_map, MechX(mech), MechY(mech), mybuff);
    for (i = 0; i < NAVIGATE_LINES; i++)
        notify(player, mybuff[i]); 
}


/* INDENT OFF */

/*
                0
            ___________                                     /``\][/""\][/""\
           /           \          HEX Location: 254, 122    \`1/``\""/``\""/
     300  /             \  60     Terrain: Light Forest     /``\``/""\`3/""\
         /               \        Elevation:  0             \`2/``\"1/``\""/
        /                 \                                 /""\``/**\`3/""\
   270 (                   )  90  Speed: 0.0                \"4/``\"4/``\""/
        \                 /       Vertical Speed: 0.0       /""\`3/""\`3/""\
         \               /        Heading: 0                \"4/``\"4/``\""/
     240  \             /  120                              /""\`3/""\`3/""\
           \____*______/                                    \"4/][\"4/][\"4/
                180
 */

/* INDENT ON */

char GetLRSMechChar(MECH * mech, MECH * tempMech)
{
    char move_type_up, move_type_low;

    if (mech == tempMech)
	return '*';
    switch (MechMove(tempMech)) {
    case MOVE_FLY:
	if (IsDS(tempMech)) {
	    move_type_up = 'D';
	    move_type_low = 'd';
	} else {
	    move_type_up = 'A';
	    move_type_low = 'a';
	}
	break;
    case MOVE_BIPED:
	move_type_up = 'B';
	move_type_low = 'b';
	break;
    case MOVE_QUAD:
	move_type_up = 'Q';
	move_type_low = 'q';
	break;
    case MOVE_TRACK:
	move_type_up = 'T';
	move_type_low = 't';
	break;
    case MOVE_WHEEL:
	move_type_up = 'W';
	move_type_low = 'w';
	break;
    case MOVE_HOVER:
	move_type_up = 'H';
	move_type_low = 'h';
	break;
    case MOVE_VTOL:
	move_type_up = 'V';
	move_type_low = 'v';
	break;
    case MOVE_HULL:
	move_type_up = 'N';
	move_type_low = 'n';
	break;
    case MOVE_SUB:
	move_type_up = 'S';
	move_type_low = 's';
	break;
    case MOVE_FOIL:
	move_type_up = 'F';
	move_type_low = 'f';
	break;
    default:
	move_type_up = 'U';
	move_type_low = 'u';
	break;
    }
    return (MechTeam(mech) ==
	MechTeam(tempMech) ? move_type_low : move_type_up);
}

static inline char TerrainColorChar(char terrain, int elev)
{
    char tmp = 0;

    switch (terrain) {
    case FIRE:
	tmp = 'R';
	break;
    case HSMOKE:
	tmp = 'W';
	break;
    case SMOKE:
	tmp = 'w';
	break;
    case HEAVY_FOREST:
	tmp = 'g';
	break;
    case LIGHT_FOREST:
    case '\'':
	tmp = 'G';
	break;
    case HIGHWATER:
	return 'B';
    case WATER:
	if (elev < 2 || elev == '0' || elev == '1' || elev == '~')
	    tmp = 'B';
	else
	    tmp = 'b';
	break;
    case ICE:
        tmp = 'c';
	break;
    case SNOW:
	tmp = 'W';
	break;
    case ROUGH:
	tmp = 'Y';
	break;
    case DESERT:
	tmp = 'y';
	break;
    case MOUNTAINS:
	tmp = 'y';
	break;
    case WALL:
    case BUILDING:
	tmp = 'W';
	break;
   case ROAD:
        tmp = 'X';
	break;
    }
    return tmp;
}

#define UpperCl(a) (isupper((a)) || (a) == 'h')
#define LowerCl(a) !(UpperCl(a))

static char *GetLRSMech(MECH * mech, MECH * tempMech, int docolor,
    char *prevc)
{
    static char buf[20];
    char abuf[20];
    char c = GetLRSMechChar(mech, tempMech);
    char d;

    if (!docolor) {
	sprintf(buf, "%c", c);
	return buf;
    }
    if (mech == tempMech)
	d = 'h';
    else if (MechTeam(mech) != MechTeam(tempMech))
	d = 'R';
    else
	d = 'g';
    abuf[0] = 0;
    if (UpperCl(*prevc) && !UpperCl(d))
	strcpy(abuf, "%cn");
    else if (!UpperCl(*prevc) && UpperCl(d))
	strcpy(abuf, "%ch");
    if (c != *prevc && c)
	sprintf(abuf + strlen(abuf), "%%c%c", tolower(d));
    if (*abuf)
	sprintf(buf, "%s%c", abuf, c);
    else
	sprintf(buf, "%c", c);
    *prevc = d;
    return buf;
}

static char *LRSTerrain(MAP * map, int x, int y, int docolor, char *prevc)
{
    static char buf[20];
    char abuf[20];
    char c = GetTerrain(map, x, y);
    int e = GetElev(map, x, y);
    char d = TerrainColorChar(c, e);

    if (!c) {
	buf[0] = 0;
	return buf;
    }
    if (!docolor || c == ' ') {
	sprintf(buf, "%c", c);
	return buf;
    }
    abuf[0] = 0;
    if ((!d && *prevc) || (UpperCl(*prevc) && (!UpperCl(d))))
	strcpy(abuf, "%cn");
    else if (!UpperCl(*prevc) && UpperCl(d))
	strcpy(abuf, "%ch");
    if (c != *prevc && c) {
	if (isupper(d))
	    sprintf(abuf + strlen(abuf), "%%ch%%c%c", tolower(d));
	else
	    sprintf(abuf + strlen(abuf), "%%c%c", d);
    }
    if (*abuf)
	sprintf(buf, "%s%c", abuf, c);
    else
	sprintf(buf, "%c", c);
    *prevc = d;
    return buf;
}

static char *ugly_kludge(char c)
{
    static char buf[2];

    buf[0] = c;
    buf[1] = 0;
    return buf;
}

#define MaxVis(mech,sensor) ((sensors[sensor].maxvis*(((sensor)>=2 && MechMove(mech)==MOVE_NONE)?140:100)) / 100)
#define MaxVar(mech,sensor) sensors[sensor].maxvvar
#define MAXVR(mech,sensor) MaxVis(mech,sensor) - MaxVar(mech, sensor)

int MySeeHex(MECH * mech, MAP * map, int mx, int my, float range)
{
float foox;
float fooy;
int calc;
int prim, sec, ebl = 0;

if (!mech)
    return 1;

/* Sensor logic for vis ranges, ECM, and shiz */
prim = MechSensor(mech)[0];
sec = MechSensor(mech)[1];
/* do our range math */
if (range < 0) {
    MapCoordToRealCoord(mx, my, &foox, &fooy);
    range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), foox, fooy, ZSCALE * GetElev(map, mx, my));
    }
    /* Compare range with sensors */
if ((((MAXVR(mech, prim)) + (MechVisMod(mech) * (MaxVar(mech, prim) + 1)) / 100) < range) &&
     (((MAXVR(mech, sec)) + (MechVisMod(mech) * (MaxVar(mech, prim) + 1)) / 100) < range))
    return 1;

/* Special Case */
if (range < 5)
   return 0;

/* Double check for exception cases to above (Should write way to pass this flag from InLineOfSight() but annoying job */
calc = CalculateLOSFlag(mech, NULL, map, mx, my, 0, range, 1); 
if ((calc & MECHLOSFLAG_BLOCK) && (calc & MECHLOSFLAG_SPREVHEX || calc & MECHLOSFLAG_WBLOCK))
    return 0;

ebl = ((prim == SENSOR_RA || prim == SENSOR_BP || prim == SENSOR_EM || prim == SENSOR_SE) &&
      (sec == SENSOR_RA || sec == SENSOR_BP || sec == SENSOR_EM || sec == SENSOR_SE));

return (ebl && (calc & (MECHLOSFLAG_ECM|MECHLOSFLAG_ANGELECM)) ? 1 : 
	(calc & MECHLOSFLAG_BLOCK) ? 1 :
	0); 
}

#define MYLRSNOSEE(x, y) \
	(((int) (hexrange = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), hfx, hfy, ZSCALE * GetElev(mech_map, x, y))) > (MapIsUnderground(mech_map) ? MechTacRange(mech) : MechLRSRange(mech)) && MySeeHex(mech, mech_map, x, y, -1)) || (MapIsUnderground(mech_map) ? MySeeHex(mech, mech_map, x, y, -1) : 0))

#define MYSHOW(buf,prevc,mask,x,y) \
          if (mechmode) \
          for (j = k; mechs[j] && MechY(mechs[j]) <= (y) && (MechY(mechs[j]) != (y) || MechX(mechs[j]) != (x)); j++) ; \
	  if (mechmode && mechs[j] && MechX(mechs[j]) == (x) && MechY(mechs[j]) == (y)) \
	    sprintf (buf, (mask), GetLRSMech (mech, mechs[j], docolor, &(prevc))); \
	  else \
	    sprintf (buf, (mask), elevmode ? ugly_kludge(GetElev(mech_map, (x), (y)) ? '0' + GetElev(mech_map, (x), (y)) : ' ') : LRSTerrain(mech_map, (x), (y), docolor, &(prevc))) \

void mech_lrsmap(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int loop, b_width, e_width, b_height, e_height;
    int argc, i, j, k = 0;
    short x, y;
    int mapx, mapy;
    MECH *tempMech;
    char topbuff[14 * LRS_DISPLAY_WIDTH + 60];
    char midbuff[14 * LRS_DISPLAY_WIDTH + 60];
    char botbuff[14 * LRS_DISPLAY_WIDTH + 60];
    char trash1[300];
    char trash2[300];
    char *args[5];
    char hextype;
    MAP *mech_map;
    float fx, fy, hfx, hfy;
    int bearing;
    float range;
    float hexrange;
    int target;
    int displayHeight;
    char *str;
    MECH *mechs[MAX_MECHS_PER_MAP];
    int last_mech = 0, mechmode = 0, elevmode = 0;
    int docolor;
    char prevct = 0, prevcb = 0;

    cch(MECH_USUAL);
    docolor = Ansimap(player);
    mech_map = getMap(mech->mapindex);
    mapx = MechX(mech);
    mapy = MechY(mech);
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(!MechLRSRange(mech), "Your system seems to be inoperational.");
    switch (argc) {
    case 3:
	bearing = atoi(args[1]);
	range = atof(args[2]);
	DOCHECK(!(MechCritStatus(mech) & OBSERVATORIC) &&
	    abs((int) range) > (MapIsUnderground(mech_map) ? MechTacRange(mech) : MechLRSRange(mech)),
	    "Those coordinates are out of sensor range!");
	FindXY(MechFX(mech), MechFY(mech), bearing, range, &fx, &fy);
	RealCoordToMapCoord(&x, &y, fx, fy);
	break;
    case 2:
	target = FindTargetDBREFFromMapNumber(mech, args[1]);
	tempMech = getMech(target);
	DOCHECK(!tempMech, "No such target.");
	range = FlMechRange(mech_map, mech, tempMech);
	DOCHECK(!InLineOfSight(mech, tempMech, MechX(tempMech),
		MechY(tempMech), range), "No such target.");
	DOCHECK(abs((int) range) > (MapIsUnderground(mech_map) ? MechTacRange(mech) : MechLRSRange(mech)),
	    "Target is out of scanner range.");
	x = MechX(tempMech);
	y = MechY(tempMech);
	break;
    case 1:
	x = MechX(mech);
	y = MechY(mech);
	break;
    default:
	notify(player, "Invalid number of parameters!");
	return;
    }
    args[0][0] = toupper(args[0][0]);
    mechmode = (args[0][0] == 'M');
    elevmode = (args[0][0] == 'E');
    DOCHECK(args[0][0] != 'M' && args[0][0] != 'E' &&
	args[0][0] != 'T', "Unknown sensor type!");
    str = silly_atr_get(player, A_LRSHEIGHT);
    if (!*str)
	displayHeight = LRS_DISPLAY_HEIGHT;
    else if (sscanf(str, "%d", &displayHeight) != 1 || displayHeight > 40
	|| displayHeight < 10) {
	notify(player,
	    "Illegal LRSHeight attribute.  Must be between 10 and 40");
	displayHeight = LRS_DISPLAY_HEIGHT;
    }
    displayHeight = (displayHeight <= 2 * (MapIsUnderground(mech_map) ? MechTacRange(mech) : MechLRSRange(mech))
	? displayHeight : 2 * (MapIsUnderground(mech_map) ? MechTacRange(mech) : MechLRSRange(mech)));

    displayHeight = (displayHeight <= mech_map->map_height)
	? displayHeight : mech_map->map_height;

    if (!(displayHeight % 2))
	displayHeight++;

    /* x and y hold the viewing center of the map */
    b_width = x - LRS_DISPLAY_WIDTH2;
    b_width = (b_width < 0) ? 0 : b_width;
    e_width = b_width + LRS_DISPLAY_WIDTH;
    if (e_width >= mech_map->map_width) {
	e_width = mech_map->map_width - 1;
	b_width = e_width - LRS_DISPLAY_WIDTH;
	b_width = (b_width < 0) ? 0 : b_width;
    }
    hextype = ((b_width % 2) == 0) ? 'o' : 'e';

    b_height = y - displayHeight / 2;
    b_height = (b_height < 0) ? 0 : b_height;
    e_height = b_height + displayHeight;
    if (e_height > mech_map->map_height) {
	e_height = mech_map->map_height;
	b_height = e_height - displayHeight;
	b_height = (b_height < 0) ? 0 : b_height;
    }
    strcpy(topbuff, "    ");
    strcpy(midbuff, "    ");
    strcpy(botbuff, "    ");
    for (i = b_width; i <= e_width; i++) {
	sprintf(trash1, "%3d", i);
	sprintf(trash2, "%c", trash1[0]);
	strcat(topbuff, trash2);
	sprintf(trash2, "%c", trash1[1]);
	strcat(midbuff, trash2);
	sprintf(trash2, "%c", trash1[2]);
	strcat(botbuff, trash2);
    }
    notify(player, topbuff);
    notify(player, midbuff);
    notify(player, botbuff);


    if (mechmode) {
	for (loop = 0; loop < mech_map->first_free; loop++) {
	    if (mech_map->mechsOnMap[loop] != -1) {
		if ((tempMech = getMech(mech_map->mechsOnMap[loop]))) {
		    if ((mech == tempMech) ||
			InLineOfSight(mech, tempMech, MechX(tempMech),
			    MechY(tempMech), FlMechRange(mech_map, mech,
				tempMech))) mechs[last_mech++] = tempMech;
		}
	    }
	}
	for (i = 0; i < (last_mech - 1); i++)	/* Bubble-sort the list to y/x order */
	    for (j = (i + 1); j < last_mech; j++) {
		if (MechY(mechs[i]) > MechY(mechs[j])) {
		    tempMech = mechs[i];
		    mechs[i] = mechs[j];
		    mechs[j] = tempMech;
		} else if (MechY(mechs[i]) == MechY(mechs[j]) &&
		    MechX(mechs[i]) > MechX(mechs[j])) {
		    tempMech = mechs[i];
		    mechs[i] = mechs[j];
		    mechs[j] = tempMech;
		}
	    }
	k = 0;
	mechs[last_mech] = NULL;
    }
    for (loop = b_height; loop < e_height; loop++) {
	sprintf(topbuff, "%3d ", loop);
	strcpy(botbuff, "    ");
	if (mechmode) {
	    while (mechs[k] && MechY(mechs[k]) < loop)
		k++;
	}
	for (i = b_width; i < e_width; i = i + 2) {
	    if (mechmode)
		for (j = k;
		    mechs[j] && MechY(mechs[j]) == loop &&
		    MechX(mechs[j]) != (i + 1); j++);
	    MapCoordToRealCoord(i + (hextype == 'e'), loop, &hfx, &hfy);
	    if (MYLRSNOSEE(i + (hextype != 'e'), loop))
		if (hextype == 'e')
	            strcat(topbuff, " %ch%cb?%cn");
		else
		    strcat(topbuff, "%ch%cb? %cn");
	    else {
	        MYSHOW(trash1, prevct, hextype == 'e' ? " %s" : "%s ", i + (hextype == 'e'), loop);
	        strcat(topbuff, trash1);
		}
	    if (loop < (mech_map->map_height - 1)) {
		MapCoordToRealCoord(i + (hextype != 'e'), loop + 1, &hfx, &hfy);
		if (MYLRSNOSEE(i + (hextype != 'e'), loop + 1))
		    if (hextype == 'e')
			strcat(botbuff, "%ch%cb? %cn");
		    else
			strcat(botbuff, " %ch%cb?%cn");
		else {
		    MYSHOW(trash1, prevcb, hextype == 'e' ? "%s " : " %s", i + (hextype != 'e'), loop + 1); 
	  	    strcat(botbuff, trash1);
		    }
	    }
	}
	if (i == e_width && (hextype != 'e' ||
		loop < (mech_map->map_height - 2))) {
	    if (hextype == 'e') {
		MapCoordToRealCoord(i, loop + 1, &hfx, &hfy);
		if (MYLRSNOSEE(i, loop + 1))
		    strcat(botbuff, "%ch%cb? %cn");
		else { 
		    MYSHOW(trash1, prevcb, "%s", i, loop + 1);
		    strcat(botbuff, trash1);
		    }
	    } else {
		MapCoordToRealCoord(i, loop, &hfx, &hfy);
		if (MYLRSNOSEE(i, loop))
		    strcat(botbuff, "%ch%cb? %cn");
		else {
		    MYSHOW(trash1, prevct, "%s", i, loop);
		    strcat(topbuff, trash1);
		    strcat(botbuff, " ");
		    }
	    }
	}
	if (docolor) {
	    if (prevct) {
		strcat(topbuff, "%cn");
		prevct = 0;
	    }
	    if (loop < (mech_map->map_height - 1))
		if (prevcb) {
		    strcat(botbuff, "%cn");
		    prevcb = 0;
		}
	}
	sprintf(trash1, " %-3d", loop + 1);
	strcat(botbuff, trash1);
	notify(player, topbuff);
	if (loop < (mech_map->map_height - 1)) {
	    notify(player, botbuff);
	}
    }
return;
}

static inline int is_oddcol(int col)
{
    /*
     * The only real trick here is to handle negative
     * numbers correctly.
     */
    return (unsigned) col & 1;
}

static inline int tac_dispcols(int hexcols)
{
    return hexcols * 3 + 1;
}

static inline int tac_hex_offset(int x, int y, int dispcols, int oddcol1)
{
    int oddcolx = is_oddcol(x + oddcol1);

    return (y * 2 + 1 - oddcolx) * dispcols + x * 3 + 1;
}

static inline void sketch_tac_row(char *pos, int left_offset,
    char const *src, int len)
{
    memset(pos, ' ', left_offset);
    memcpy(pos + left_offset, src, len);
    pos[left_offset + len] = '\0';
}

static void sketch_tac_map(MECH * mech, char *buf, MAP * map, int sx, int sy, int wx,
    int wy, int dispcols, int top_offset, int left_offset, int docolour, int blz, int los, int dens)
{
#if 0
    static char const hexrow[2][76] = {
	"\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/",
	"/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\"
    };
#else
    static char const hexrow[2][310] = {
	"\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/",
	"/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\][/][\\"
    };
#endif
    int x, y;
    int oddcol1 = is_oddcol(sx);	/* One iff first hex col is odd */
    char *pos;
    int mapcols = tac_dispcols(wx);
    float range, fx, fy, fz;
    int notsee = 0, foo, prim, sec;
    /*
     * First create a blank hex map.
     */
    pos = buf;
    for (y = 0; y < top_offset; y++) {
	memset(pos, ' ', dispcols - 1);
	pos[dispcols - 1] = '\0';
	pos += dispcols;
    }
    for (y = 0; y < wy; y++) {
	sketch_tac_row(pos, left_offset, hexrow[oddcol1], mapcols);
	pos += dispcols;
	sketch_tac_row(pos, left_offset, hexrow[!oddcol1], mapcols);
	pos += dispcols;
    }
    sketch_tac_row(pos, left_offset, hexrow[oddcol1], mapcols);

    /*
     * Now draw the terrain and elevation.
     */
    pos = buf + top_offset * dispcols + left_offset;
    wx = MIN(wx, map->map_width - sx);
    wy = MIN(wy, map->map_height - sy);
    for (y = MAX(0, -sy); y < wy; y++) {
	for (x = MAX(0, -sx); x < wx; x++) {
	    int terr, elev;
	    char *base;
	    char topchar, botchar;

	    terr = GetTerrain(map, sx + x, sy + y);
	    elev = GetElev(map, sx + x, sy + y);
	    base = pos + tac_hex_offset(x, y, dispcols, oddcol1);

	    if (mech) {
	        MapCoordToRealCoord(sx + x, sy + y, &fx, &fy);
	        range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), fx, fy, ZSCALE * elev);
		/* Sensor logic for vis ranges, ECM, and shiz */
		prim = MechSensor(mech)[0];
		sec = MechSensor(mech)[1];
		foo = ((prim == SENSOR_RA || prim == SENSOR_BP || prim == SENSOR_EM || prim == SENSOR_SE) &&
		      (sec == SENSOR_RA || sec == SENSOR_BP || sec == SENSOR_EM || sec == SENSOR_SE)); 
		if (MechZ(mech) >= ORBIT_Z + 50)
		    notsee = 1;
		else if (foo && ecm_affect_hex(mech, sx + x, sy + y, elev, &foo) > 0)
		    notsee = 1;
	        else if (los)
		    notsee = !InLineOfSight(mech, NULL, sx + x, sy + y, range);
	        else
	            notsee = ((MapIsUnderground(map) && MySeeHex(mech, map, sx + x, sy + y, range)) || (((int) range > MechTacRange(mech)) && !InLineOfSight(mech, NULL, sx + x, sy + y, range)));
		}

	    if (notsee) { 
		topchar = '?';
		botchar = '?';	
		} else {

	    switch (terr) {
	    case WATER:
		/*
		 * Colour hack:  Draw deep water with '\242'
		 * if using colour so colourize_tac_map()
		 * knows to use dark blue rather than light
		 * blue
		 */
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
		botchar = '~';
		break;

	    case DBRIDGE:
		topchar = '%';
		botchar = '~';
		break;

	    case ' ':		/* GRASSLAND */
		topchar = ' ';
		botchar = '_';
		break;

	    case SMOKE:
		topchar = terr;
		botchar = (GetRTerrain(map, sx + x, sy + y));
		break;

	    case HSMOKE:
		topchar = terr;
		botchar = (GetRTerrain(map, sx + x, sy + y));
		break;

	    case TFIRE:
	    case FIRE:
		topchar = terr;
		if (GetRTerrain(map, sx + x, sy + y) == BRIDGE || GetRTerrain(map, sx + x, sy + y) == DBRIDGE)
		    botchar = '~';
		else
		    botchar = (GetRTerrain(map, sx + x, sy + y));
	   	break;

	    default:
		topchar = terr;
		botchar = terr;
		break;
	    }

	}

	    base[0] = topchar;
	    base[1] = topchar;
	    if (mech && blz && !notsee) {
		base[dispcols + 0] = (ImproperLZ_MapHex(mech, map, sx + x, sy + y) ? 'X' : 'O');
	    } else {
		base[dispcols + 0] = botchar;
	    } 
	    if (dens) {
		botchar = '0' + BOUNDED(0, hex_mineral_density(map, sx + x, sy + y) / 10, 9);
	    } else if (elev > 0 && !notsee) {
		botchar = '0' + elev;
	    }
	    base[dispcols + 1] = botchar;
	}
    }
}

/*
 * Draw one of the seven hexes that a Dropship takes up on a tac map.
 */
static void sketch_tac_ds(char *base, int dispcols, char terr)
{
    /*
     * Becareful not to overlay a 'mech id or terrain elevation.
     */
    if (!isalpha((unsigned char) base[0])) {
	base[0] = terr;
	base[1] = terr;
    }
    base[dispcols + 0] = terr;
    if (!isdigit((unsigned char) base[dispcols + 1])) {
	base[dispcols + 1] = terr;
    }
}

extern int dirs[6][2];

static void sketch_tac_ownmech(char *buf, MAP * map, MECH * mech, int sx,
    int sy, int wx, int wy, int dispcols, int top_offset, int left_offset)
{

    int oddcol1 = is_oddcol(sx);
    char *pos = buf + top_offset * dispcols + left_offset;
    char *base;
    int x = MechX(mech) - sx;
    int y = MechY(mech) - sy;

    if (x < 0 || x >= wx || y < 0 || y >= wy) {
	return;
    }
    base = pos + tac_hex_offset(x, y, dispcols, oddcol1);
    base[0] = '*';
    base[0] = '*';
}

static void sketch_tac_mechs(char *buf, MAP * map, MECH * player_mech,
    int sx, int sy, int wx, int wy, int dispcols, int top_offset,
    int left_offset, int docolour, int labels)
{
    int i;
    char *pos = buf + top_offset * dispcols + left_offset;
    int oddcol1 = is_oddcol(sx);

    /*
     * Draw all the 'mechs on the map.
     */
    for (i = 0; i < map->first_free; i++) {
	int x, y;
	char *base;
	MECH *mech;

	if (map->mechsOnMap[i] == -1) {
	    continue;
	}

	mech = getMech(map->mechsOnMap[i]);
	if (mech == NULL) {
	    continue;
	}

	/*
	 * Check to see if the 'mech is on the tac map and
	 * that its in LOS of the player's 'mech.
	 */
	x = MechX(mech) - sx;
	y = MechY(mech) - sy;
	if (x < 0 || x >= wx || y < 0 || y >= wy) {
	    continue;
	}

	if (mech != player_mech &&
	    !InLineOfSight(player_mech, mech, MechX(mech), MechY(mech),
		FlMechRange(map, player_mech, mech))) {
	    continue;
	}

	base = pos + tac_hex_offset(x, y, dispcols, oddcol1);

	if (!(MechSpecials2(mech) & CARRIER_TECH) && IsDS(mech) && ((MechZ(mech) >= ORBIT_Z && mech != player_mech) || Landed(mech) || !Started(mech))) {
	    int ts = DSBearMod(mech);
	    int dir;

	    /*
	     * Dropships are a special case.  They take up
	     * seven hexes on a tac map.  First draw the
	     * center hex and then the six surronding hexes.
	     */

	    if (docolour) {
		/*
		 * Colour hack: 'X' would be confused with
		 * any enemy con by colourize_tac_map()
		 */
		sketch_tac_ds(base, dispcols, '$');
	    } else {
		sketch_tac_ds(base, dispcols, 'X');
	    }
	    for (dir = 0; dir < 6; dir++) {
		int tx = x + dirs[dir][0];
		int ty = y + dirs[dir][1];

		if ((tx + oddcol1) % 2 == 0 && dirs[dir][0] != 0) {
		    ty--;
		}
		if (tx < 0 || tx >= wx || ty < 0 || ty >= wy) {
		    continue;
		}
		base = pos + tac_hex_offset(tx, ty, dispcols, oddcol1);
		if (Find_DS_Bay_Number(mech, (dir - ts + 6) % 6)
		    >= 0) {
		    sketch_tac_ds(base, dispcols, '@');
		} else {
		    sketch_tac_ds(base, dispcols, '=');
		}
	    }
	} else if (mech == player_mech) {
	    base[0] = '*';
	    base[1] = '*';
	} else {
	    char *id = MechIDS(mech, MechTeam(player_mech)
		== MechTeam(mech));

	    base[0] = id[0];
	    base[1] = id[1];
	}
    }
}
    
static void sketch_tac_cliffs(char *buf, MECH * mech, MAP * map, int sx, int sy, int wx,
    int wy, int dispcols, int top_offset, int left_offset, int cliff_size)
{
    char *pos = buf + top_offset * dispcols + left_offset;
    int y, x;
    int oddcol1 = is_oddcol(sx);
    float range, fx, fy;

    wx = MIN(wx, map->map_width - sx);
    wy = MIN(wy, map->map_height - sy);
    for (y = MAX(0, -sy); y < wy; y++) {
	int ty = sy + y;

	for (x = MAX(0, -sx); x < wx; x++) {
	    int tx = sx + x;
	    int oddcolx = is_oddcol(tx);
	    int elev = Elevation(map, tx, ty);
	    char *base = pos + tac_hex_offset(x, y, dispcols,
		oddcol1);
	    char c;


	    /*
	     * Copy the elevation up to the top of the hex
	     * so we can draw a bottom hex edge on every hex.
	     */
	    c = base[dispcols + 1];
	    if (base[0] == '*') {
		base[0] = '*';
		base[1] = '*';
	    } else if (isdigit((unsigned char) c)) {
		base[1] = c;
	    }


	    /*
	     * For each hex on the map check to see if each
	     * of it's 240, 180, and 120 hex sides is a cliff.
	     * Don't check for cliffs between hexes that are on
	     * the tac map and those that are off of it.
	     */
	    if (mech) {
	        MapCoordToRealCoord(tx, ty, &fx, &fy);
	        range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), fx, fy, ZSCALE * GetElev(map, x, y));
	        if (((int) range > MechTacRange(mech) && MySeeHex(mech, map, x, y, range)) || ((MapIsUnderground(map)) && MySeeHex(mech, map, x, y, range)))  {
		    base[dispcols] = '_';
		    base[dispcols + 1] = '_'; 
		    continue;
	        }
	    }

	    if (x != 0 && (y < wy - 1 || oddcolx)
		&& abs(Elevation(map, tx - 1, ty + 1 - oddcolx)
		    - elev) >= cliff_size) {

		base[dispcols - 1] = '|';
	    }
	    if (y < wy - 1 && abs(Elevation(map, tx, ty + 1) - elev)
		>= cliff_size) {
		base[dispcols] = ',';
		base[dispcols + 1] = ',';
	    } else {
		base[dispcols] = '_';
		base[dispcols + 1] = '_';
	    }
	    if (x < wx - 1 && (y < wy - 1 || oddcolx)
		&& abs(Elevation(map, tx + 1, ty + 1 - oddcolx)
		    - elev) >= cliff_size) {
		base[dispcols + 2] = '!';
	    }
	}
    }
}

/*
 * Colourize a sketch tac map.  Uses dynmaically allocated buffers
 * which are overwritten on each call.
 */
static char **colourize_tac_map(char const *sketch, int dispcols,
    int disprows)
{
    static char *buf = NULL;
    static int buf_len = 5000;
    static char **lines = NULL;
    static int lines_len = 100;
    int pos = 0;
    int line = 0;
    unsigned char cur_colour = '\0';
    char *line_start;
    char const *src = sketch;

    if (buf == NULL) {
	Create(buf, char, buf_len);
    }
    if (lines == NULL) {
	Create(lines, char *, lines_len);
    }

    line_start = (char *) src;
    lines[0] = buf;
    while (lines > 0) {
	unsigned char new_colour;
	unsigned char c = *src++;

	if (c == '\0') {
	    /*
	     * End of line.
	     */
	    if (cur_colour != '\0') {
		buf[pos++] = '%';
		buf[pos++] = 'c';
		buf[pos++] = 'n';
	    }
	    buf[pos++] = '\0';
	    line++;
	    if (line >= disprows) {
		break;		/* Done */
	    }
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
	    new_colour = 'b';
	    break;

	case '$':		/* Colour Hack: Drop Ship */
	    c = 'X';
	    new_colour = '\0';
	    break;

	case '!':		/* Cliff hex edge */
	    c = '/';
	    new_colour = 'R';
	    break;

	case '|':		/* Cliff hex edge */
	    c = '\\';
	    new_colour = 'R';
	    break;

	case ',':		/* Cliff hex edge */
	    c = '_';
	    new_colour = 'R';
	    break;
	case '*':		/* mech itself. */
	    new_colour = 'H';
	    break;
	case 'O':
	    new_colour = 'G';
	    break;
//	case '?':
//	    new_colour = 'b';
//	    break; 
	default:
	    if (islower(c)) {	/* Friendly con */
		new_colour = 'Y';
	    } else if (isupper(c)) {	/* Enemy con */
		new_colour = 'R';
	    } else if (isdigit(c)) {	/* Elevation */
		new_colour = cur_colour;
	    } else {
		new_colour = TerrainColorChar(c, 0);
	    }
	    break;
	}

	if (isupper(new_colour) != isupper(cur_colour)) {
	    if (isupper(new_colour)) {
		buf[pos++] = '%';
		buf[pos++] = 'c';
		buf[pos++] = 'h';
	    } else {
		buf[pos++] = '%';
		buf[pos++] = 'c';
		buf[pos++] = 'n';
		cur_colour = '\0';
	    }
	}
	if (tolower(new_colour) != tolower(cur_colour)) {
	    buf[pos++] = '%';
	    buf[pos++] = 'c';
	    if (new_colour == '\0') {
		buf[pos++] = 'n';
	    } else if (new_colour == 'H') {
		buf[pos++] = 'n';
		buf[pos++] = '%';
		buf[pos++] = 'c';
		buf[pos++] = tolower(new_colour);
	    } else {
		buf[pos++] = tolower(new_colour);
	    }
	    cur_colour = new_colour;
	}
	buf[pos++] = c;
	if (pos + 11 > buf_len) {
	    /*
	     * If we somehow run out of room then we don't
	     * bother to reallocate 'buf' and potentially have
	     * a bunch of invalid pointers in 'lines' to fix up.
	     * We just restart from scratch with a bigger 'buf'.
	     */
	    buf_len *= 2;
	    free(buf);
	    buf = NULL;
	    return colourize_tac_map(sketch, dispcols, disprows);
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
 *
 * If navigate mode, wx and wy should be equal and odd.  Navigate maps
 * cannot have top or side labels.
 *
 */

char **MakeMapText(dbref player, MECH * mech, MAP * map, int cx, int cy,
    int wx, int wy, int labels)
{
    int docolour = Ansimap(player);
    int dispcols;
    int disprows;
    int mapcols;
    int left_offset = 0;
    int top_offset = 0;
    int navigate = 0;
    int sx, sy;
    int i;
    int blz;
    char *base;
    int oddcol1;
    enum {
	MAX_WIDTH = 40,
	MAX_HEIGHT = 24,
	TOP_LABEL = 3,
	LEFT_LABEL = 4,
	RIGHT_LABEL = 3
    };
    static char sketch_buf[((LEFT_LABEL + 1 + MAX_WIDTH * 3 + RIGHT_LABEL +
	    1) * (TOP_LABEL + 1 + MAX_HEIGHT * 2) + 2) * 5];
    static char *lines[(TOP_LABEL + 1 + MAX_HEIGHT * 2 + 1) * 5];

    if (labels & 4) {
	navigate = 1;
	labels = 0;
    }

    blz = (labels & 32);

    /*
     * Figure out the extent of the tac map to draw.
     */
    wx = MIN(MAX_WIDTH, wx);
    wy = MIN(MAX_HEIGHT, wy);

    sx = cx - wx / 2;
    sy = cy - wy / 2;
    if (!navigate) {
	/*
	 * Only allow navigate maps to include off map hexes.
	 */
	sx = MAX(0, MIN(sx, map->map_width - wx));
	sy = MAX(0, MIN(sy, map->map_height - wy));
	wx = MIN(wx, map->map_width);
	wy = MIN(wy, map->map_height);
    }

    mapcols = tac_dispcols(wx);
    dispcols = mapcols + 1;
    disprows = wy * 2 + 1;
    oddcol1 = is_oddcol(sx);

    if (navigate) {
	if (oddcol1) {
	    /*
	     * Insert blank line at the top where we can put
	     * a "__" to make the navigate map look pretty.
	     */
	    top_offset = 1;
	    disprows++;
	}
    } else {
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

    /*
     * Create a sketch tac map including terrain and elevation.
     */
    sketch_tac_map(mech, sketch_buf, map, sx, sy, wx, wy, dispcols, top_offset,
	left_offset, docolour, blz, (labels & 64 ? 1 : 0), (labels & 128 ? 1 : 0));

    /*
     * Draw the top and side labels.
     */
    if (labels & 1) {
	int x;

	for (x = 0; x < wx; x++) {
	    char scratch[4];
	    int label = sx + x;

	    if (label < 0 || label > 999) {
		continue;
	    }
	    sprintf(scratch, "%3d", label);
	    base = sketch_buf + left_offset + 1 + x * 3;
	    base[0] = scratch[0];
	    base[1 * dispcols] = scratch[1];
	    base[2 * dispcols] = scratch[2];
	}
    }

    if (labels & 2) {
	int y;

	for (y = 0; y < wy; y++) {
	    int label = sy + y;

	    base = sketch_buf + (top_offset + 1 + y * 2)
		* dispcols;
	    if (label < 0 || label > 999) {
		continue;
	    }

	    sprintf(base, "%3d", label);
	    base[3] = ' ';
	    sprintf(base + (dispcols - RIGHT_LABEL - 1), "%3d", label);
	}
    }

    if (labels & 8) {
	if (mech != NULL) {
	    sketch_tac_ownmech(sketch_buf, map, mech, sx, sy, wx, wy,
		dispcols, top_offset, left_offset);
	}
	sketch_tac_cliffs(sketch_buf, mech, map, sx, sy, wx, wy, dispcols,
	    top_offset, left_offset, 3);
    } else if (labels & 16) {
	if (mech != NULL) {
	    sketch_tac_ownmech(sketch_buf, map, mech, sx, sy, wx, wy,
		dispcols, top_offset, left_offset);
	}
	sketch_tac_cliffs(sketch_buf, mech, map, sx, sy, wx, wy, dispcols,
	    top_offset, left_offset, 2);
    } else if (mech != NULL) {
	sketch_tac_mechs(sketch_buf, map, mech, sx, sy, wx, wy, dispcols,
	    top_offset, left_offset, docolour, labels);
    }


    if (navigate) {
	int n = wx / 2;		/* Hexagon radius */

	/*
	 * Navigate hack: erase characters from the sketch map
	 * to turn it into a pretty hexagonal shaped map.
	 */
	if (oddcol1) {
	    /*
	     * Don't need the last line in this case.
	     */
	    disprows--;
	}

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

	    base =
		sketch_buf + (disprows - i - 1) * dispcols + left_offset;
	    len = (n - i) * 3;
	    memset(base, ' ', len);
	    base[mapcols - len] = '\0';
	}

	memset(sketch_buf + left_offset, ' ', n * 3 + 1);
	sketch_buf[left_offset + n * 3 + 1] = '_';
	sketch_buf[left_offset + n * 3 + 2] = '_';
	sketch_buf[left_offset + n * 3 + 3] = '\0';
    }

    if (docolour) {
	/*
	 * If using colour then colourize the sketch map and
	 * return the result.
	 */
	return colourize_tac_map(sketch_buf, dispcols, disprows);
    }

    /*
     * If not using colour, the sketch map can be used as is.
     */
    for (i = 0; i < disprows; i++) {
	lines[i] = sketch_buf + dispcols * i;
    }
    lines[i] = NULL;
    return lines;
}

void mech_tacmap(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int argc, i;
    short x, y;
    int mapx, mapy;
    MECH *tempMech;
    char *args_vec[4];
    char **args = args_vec;
    MAP *mech_map;
    float fx, fy;
    float range;
    int bearing;
    int target;
    int displayHeight = MAP_DISPLAY_HEIGHT, displayWidth = MAP_DISPLAY_WIDTH;
    char *str;
    char **maptext;
    int flags = 3;

    cch(MECH_USUAL);
    mech_map = getMap(mech->mapindex);
    mapx = MechX(mech);
    mapy = MechY(mech);
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(!MechTacRange(mech), "Your system seems to be inoperational.");

    if (argc > 0 && isalpha((unsigned char) args[0][0])
	&& args[0][1] == '\0') {
	switch (tolower((unsigned char) args[0][0])) {
	case 'c':
	    flags |= 8;		/* Show cliffs */
	    break;

	case 't':
	    flags |= 16;	/* Show tank cliffs */
	    break;

	case 'b':
	    flags |= 32;
	    break;

	case 'l':
	    flags |= 64;
	    break;

	case 'd':
	    flags |= 128;
	    break;

	default:
	    notify(player, "Invalid tactical map flag.");
	    return;
	}
	args++;
	argc--;
    }

    switch (argc) {
    case 2:
	bearing = atoi(args[0]);
	range = atof(args[1]);
	DOCHECK(!(MechCritStatus(mech) & OBSERVATORIC) &&
	    abs((int) range) > MechTacRange(mech),
	    "Those coordinates are out of sensor range!");
	FindXY(MechFX(mech), MechFY(mech), bearing, range, &fx, &fy);
	RealCoordToMapCoord(&x, &y, fx, fy);
	break;
    case 1:
	target = FindTargetDBREFFromMapNumber(mech, args[0]);
	tempMech = getMech(target);
	DOCHECK(!tempMech, "No such target.");
	range = FlMechRange(mech_map, mech, tempMech);
	DOCHECK(!InLineOfSight(mech, tempMech, MechX(tempMech),
		MechY(tempMech), range), "No such target.");
	DOCHECK(abs((int) range) > MechTacRange(mech),
	    "Target is out of scanner range.");
	x = MechX(tempMech);
	y = MechY(tempMech);
	break;
    case 0:
	x = MechX(mech);
	y = MechY(mech);
	break;
    default:
	notify(player, "Invalid number of parameters!");
	return;
    }
    str = silly_atr_get(player, A_TACHEIGHT);
    if (!*str) {
	displayHeight = MAP_DISPLAY_HEIGHT;
	displayWidth = MAP_DISPLAY_WIDTH;
    } else if (sscanf(str, "%d %d", &displayHeight, &displayWidth) != 2 || displayHeight > 24 || displayHeight < 5 || displayWidth > 40 || displayWidth < 5) {
	notify(player, "Illegal Tacsize attribute. Must be in format 'Height Width'. Height : 1-24 Width 1-40");
	displayHeight = MAP_DISPLAY_HEIGHT;
	displayWidth = MAP_DISPLAY_WIDTH;
    }
    displayHeight = (displayHeight <= 2 * MechTacRange(mech)
	? displayHeight : 2 * MechTacRange(mech));
    displayWidth = (displayWidth <= 2 * MechTacRange(mech)
	? displayWidth : 2 * MechTacRange(mech));

    displayHeight = (displayHeight <= mech_map->map_height)
	? displayHeight : mech_map->map_height;
    displayWidth = (displayWidth <= mech_map->map_width)
	? displayWidth : mech_map->map_width;
    maptext =
	MakeMapText(player, mech, mech_map, x, y, displayWidth,
	displayHeight, flags);
    for (i = 0; maptext[i]; i++)
	notify(player, maptext[i]);
}


/* XXX Fix 'enterbase <dir>' */
static void mech_enter_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data, *tmpm = NULL, *swarm = NULL;
    mapobj *mapo;
    MAP *map = getMap(mech->mapindex), *newmap;
    int target = (int) e->data2;
    int x, y, i = 0;

    if (!(mapo = find_entrance_by_xy(map, MechX(mech), MechY(mech))))
	return;
    if (!Started(mech) || Uncon(mech) || Jumping(mech) ||
	(MechType(mech) == CLASS_MECH && (Fallen(mech) || Standing(mech)))
	|| OODing(mech) || (fabs(MechSpeed(mech)) * 5 >= MMaxSpeed(mech) &&
	    fabs(MMaxSpeed(mech)) >= MP1) || (MechType(mech) == CLASS_VTOL
	    && AeroFuel(mech) <= 0))
	return;
    if (!(newmap = getMap(mapo->obj)))
	return;
    if (!find_entrance(newmap, target, &x, &y))
	return;
    if ((!can_pass_lock(mech->mynum, newmap->mynum, A_LENTER) &&
	    newmap->cf >= (newmap->cfmax / 2)) || BuildIsSafe(newmap)) {
	mech_notify(mech, MECHALL, "The hangar is locked.");
	return;
    }
/*
    for (i = 0; i < NUM_SECTIONS; i++) {
        if (!SectIsDestroyed(mech, i))
            DOCHECKMA(SectHasBusyWeap(mech, i), "You have weapons recycling!");
            DOCHECKMA(MechSections(mech)[i].recycle, "You are still recovering from your last attack.");
        } 
*/
    DOCHECKMA(!newmap->cf,"The entrance is blocked by rubble.");
    mech_notify(mech, MECHALL, tprintf("You enter %s.",
	    structure_name(mapo)));
    MechLOSBroadcast(mech, tprintf("has entered %s at %d,%d.",
	    structure_name(mapo), MechX(mech), MechY(mech)));
    MarkForLOSUpdate(mech);
    if (MechType(mech) == CLASS_MW && !In_Character(mapo->obj)) {
	enter_mw_bay(mech, mapo->obj);
	return;
    }
    if (MechCarrying(mech) > 0)
	tmpm = getMech(MechCarrying(mech));
    i = (MechIsSwarmed(mech));
    mech_Rsetmapindex(GOD, (void *) mech, tprintf("%d", (int) mapo->obj));
    mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d", x, y));
    MechLOSBroadcast(mech, tprintf("has entered %s at %d,%d.",
	    structure_name(mapo), MechX(mech), MechY(mech)));
    loud_teleport(mech->mynum, mapo->obj);
    if (tmpm) {
	mech_Rsetmapindex(GOD, (void *) tmpm, tprintf("%d",
		(int) mapo->obj));
	mech_Rsetxy(GOD, (void *) tmpm, tprintf("%d %d", x, y));
	loud_teleport(tmpm->mynum, mapo->obj);
    }
    if (i)
	{
	if (!map || !mech)
            return;
        for (i = 0; i < map->first_free; i++)
            {
            if (!(swarm = FindObjectsData(map->mechsOnMap[i])))
                continue;
            if (MechType(swarm) == CLASS_BSUIT)
                if (MechSwarmTarget(swarm) == mech->mynum)
			{
			if (MechTeam(mech) == MechTeam(swarm))
				{
				mech_Rsetmapindex(GOD, (void *) swarm, tprintf("%d", (int) mapo->obj));
				mech_Rsetxy(GOD, (void *) swarm, tprintf("%d %d", x, y));
				loud_teleport(swarm->mynum, mapo->obj);
				CalAutoMapindex(swarm);
				} else {
				bsuit_stopswarmer(mech, swarm, 1);
				}
			}
            }
	}
    CalAutoMapindex(mech);
}


void mech_enterbase(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *map, *newmap;
    int x, y;
    mapobj *mapo;
    char target, *tmpc;
    char *args[2];
    int argc, i;

    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc > 1, "Invalid arguments to command!");
    tmpc = args[0];
    if (argc > 0 && *tmpc && !(*(tmpc + 1)))
	target = tolower(*tmpc);
    else
	target = 0;
    cch(MECH_USUAL);
    map = getMap(mech->mapindex);
    /* For now, no dir checks */
    DOCHECK(MechMove(mech) == MOVE_NONE, "Drag yourself in with yer bloody stumps?");
    DOCHECK((MechStatus2(mech) & BOGDOWN), "While stuck in bog? No way.");
    DOCHECK(Jumping(mech), "While in mid-jump? No way.");
    DOCHECK(Stabilizing(mech), "Wait for your sense of balance to stabilize.");
    DOCHECK(MechType(mech) == CLASS_MECH && (Fallen(mech) ||
	    Standing(mech)), "Crawl inside? I think not. Stand first.");
    DOCHECK(OODing(mech), "While in mid-flight? No way.");
    DOCHECK(MechType(mech) == CLASS_VTOL &&
	AeroFuel(mech) <= 0, "You lack fuel to maneuver in!");
    DOCHECK(FlyingT(mech) &&
	!Landed(mech),
	"You need to land before you can enter the hangar.");
    DOCHECK(IsDS(mech),
	"Heh, you're trying to be funny, right, a DropShip entering hangar?");
    DOCHECK(fabs(MechSpeed(mech)) * 5 >= MMaxSpeed(mech) &&
	fabs(MMaxSpeed(mech)) >= MP1,
	"You are moving too fast to enter the hangar!");
/*
    for (i = 0; i < NUM_SECTIONS; i++) {
        if (!SectIsDestroyed(mech, i))
            DOCHECK(SectHasBusyWeap(mech, i), "You have weapons recycling!");
            DOCHECK(MechSections(mech)[i].recycle, "You are still recovering from your last attack.");
        }
*/
    DOCHECK(!(mapo =
	    find_entrance_by_xy(map, MechX(mech), MechY(mech))),
	"You see nothing to enter here!");
    /* Wow, *gasp*, we got something to enter */
    if (!(newmap = FindObjectsData(mapo->obj))) {
	mech_notify(mech, MECHALL,
	    "You sense wrongness in fabric of space..");
	SendError(tprintf
	    ("Error: No map existing for mapindex #%d (@ %d,%d of #%d)",
		(int) mapo->obj, mapo->x, mapo->y, mech->mapindex));
	return;
    }
/*    DOCHECKMA(!newmap->cf,"The entrance is blocked by rubble."); */
    if (!find_entrance(newmap, target, &x, &y)) {
	mech_notify(mech, MECHALL,
	    "You sense wrongness in fabric of space..");
	SendError(tprintf
	    ("Error: No entrance existing for mapindex #%d (@ %d,%d of #%d)",
		(int) mapo->obj, mapo->x, mapo->y, mech->mapindex));
	return;
    }
    if ((!can_pass_lock(mech->mynum, newmap->mynum, A_LENTER) &&
	    newmap->cf >= (newmap->cfmax / 2)) || BuildIsSafe(newmap)) {
	mech_notify(mech, MECHALL, "The hangar is locked.");
	return;
    }
    DOCHECK(event_count_type_data(EVENT_MISC, (void *) mech),
	"Shut up already!");
    /* XXX Check for other mechs in the hex possibly doing this as well (ick) */
    HexLOSBroadcast(map, MechX(mech), MechY(mech),
	"The doors at $h start to open..");
    MECHEVENT(mech, EVENT_MISC, mech_enter_event, 45, (int) target);
}
