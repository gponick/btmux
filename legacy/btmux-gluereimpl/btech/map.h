#ifndef MAP_H
#define MAP_H

//temporary crap until everything is in and available
#define MechX(a) 0
#define MechY(a) 0
#define getMech(a) NULL
#define FlMechRange(a, b) 0
#define InLineOfSight(a,b,c,d,e) 1
#define CARRIER_TECH 1
#define ORBIT_Z 100
#define IsDS(a) 0
#define Landed(a) 0
#define MechSpecials2(a) 0
#define MechZ(a) 0
#define Started(a) 1
#define DSBearMod(a) 0
#define Find_DS_Bay_Number(a,b) 0
#define MechIDS(a,b) "AB"
#define MechSeemsFriend(a,b) 1
#define ImproperLZ(a,b,c) 0
#define Ansimap(a) 1
#define A_TACHEIGHT 259


#define Create(var,typ,count) \
if (!(var = (typ *) calloc(sizeof (typ), count))) \
{ fprintf(stderr, "Error mallocating!\n"); exit(1); }
#define MyReCreate(a,b,c) \
if (!((a) = ( b * ) realloc((void *) a, sizeof( b ) * (c) ) )) \
{ printf ("Unable to realloc!\n"); exit(1); }
#define ReCreate(a,b,c) \
if (a) { MyReCreate(a,b,c); } else { Create(a,b,c); }
#define Free(a) if (a) {free(a);a=0;}


const int MAPX               = 1000;	/* max map width */
const int MAPY               = 1000;	/* max map height */
const int MAP_NAME_SIZE      = 30;
const int NUM_MAP_LINKS      = 4;
const int DEFAULT_MAP_WIDTH  = 21;
const int DEFAULT_MAP_HEIGHT = 11;
const int MAP_DISPLAY_WIDTH  = 21;
const int MAP_DISPLAY_HEIGHT = 14; 
const int MAX_ELEV           = 9;

const char GRASSLAND         = ' ';
const char HEAVY_FOREST      = '"';
const char LIGHT_FOREST      = '`';
const char WATER             = '~';
const char HIGHWATER         = '?';
const char ROUGH             = '%';
const char MOUNTAINS         = '^';
const char ROAD              = '#';
const char BUILDING          = '@';
const char FIRE              = '&';
const char TFIRE             = '>';
const char SMOKE             = ':';
const char WALL              = '=';

const char BRIDGE            = '/';
const char SNOW              = '+';
const char ICE               = '-';

const char UNKNOWN_TERRAIN   = '$';

enum { SWATER_IDX, DWATER_IDX, BUILDING_IDX, ROAD_IDX, ROUGH_IDX, MOUNTAIN_IDX,
       FIRE_IDX, ICE_IDX, WALL_IDX, SNOW_IDX, SMOKE_IDX, LWOOD_IDX, HWOOD_IDX,
       UNKNOWN_IDX, CLIFF_IDX, SELF_IDX, FRIEND_IDX, ENEMY_IDX, DS_IDX,
       GOODLZ_IDX, BADLZ_IDX, NUM_COLOR_IDX
};

std::string DEFAULT_COLOR_SCHEME="BbW\0YyRWWWXGgbRHYR\0GR";
std::string DEFAULT_COLOR_STRING="BbWnYyRWWWXGgbRhYRnGR";
std::string custom_color_str = DEFAULT_COLOR_SCHEME;

const int MAPLOS_MAXX = 70;
const int MAPLOS_MAXY = 45;

const int MAPLOS_FLAG_SLITE	= 1;

const int MAPLOSHEX_NOLOS = 0;
const int MAPLOSHEX_SEEN = 1;
const int MAPLOSHEX_SEETERRAIN = 2;
const int MAPLOSHEX_SEEELEV	= 4;
const int MAPLOSHEX_LIT	=	8;
const int MAPLOSHEX_SEE	=	(MAPLOSHEX_SEETERRAIN | MAPLOSHEX_SEEELEV);

typedef struct hexlosmap_info {
    int startx;
    int starty;
    int xsize;
    int ysize;
    int flags;
    unsigned char map[MAPLOS_MAXX * MAPLOS_MAXY];
} hexlosmap_info;

// map creation stuff
const int LASTCHAR = 128;
const int ELEVATIONS = 10;
const int ENTRIES = 256;

static int first_free = 0;
static unsigned char data_to_id[LASTCHAR][ELEVATIONS];
typedef struct hex_struct {
	char terrain;
	char elev;
} HS;
static HS id_to_data[ENTRIES];

int dirs[6][2];

#endif

