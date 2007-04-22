#ifndef _MAP_H
#define _MAP_H

#ifndef ECMD
#define ECMD(a) extern void a (dbref player, void *data, char *buffer)
#endif

/*
   map.h
   Structure definitions and what not for the maps for the mechs.

 */

#define MAX_MECHS_PER_MAP 250

/* map links */
#define MAP_UP 0
#define MAP_DOWN 1
#define MAP_RIGHT 2
#define MAP_LEFT 3

/* Map size */
#define MAPX 1000
#define MAPY 1000
#define MAP_NAME_SIZE 30
#define NUM_MAP_LINKS 4
#define DEFAULT_MAP_WIDTH 21
#define DEFAULT_MAP_HEIGHT 11
#define MAP_DISPLAY_WIDTH 21
#define MAP_DISPLAY_HEIGHT 9
#define MAX_ELEV 9
#define GRASSLAND ' '
#define HEAVY_FOREST '"'
#define LIGHT_FOREST '`'
#define WATER '~'
#define HIGHWATER '?'
#define ROUGH '%'
#define MOUNTAINS '^'
#define ROAD '#'
#define BUILDING '@'
#define FIRE '&'
#define TFIRE '>'
#define SMOKE ':'
#define HSMOKE ';'
#define WALL '='
#define DESERT '}'

#define DBRIDGE '$'
#define BRIDGE '/'
#define SNOW   '+'
#define ICE    '-'

#define MAPFLAG_MAPO     1	/* We got mapobjs */
#define MAPFLAG_SPEC     2	/* We're using special rules - gravity/temp */
#define MAPFLAG_VACUUM   4	/* We're in vacuum */
#define MAPFLAG_FIRES    8	/* We have eternal fires */
#define MAPFLAG_UNDERGROUND 16  /* We're underground. Watch your head */

#define TYPE_FIRE  0		/* Fire - datas = counter until next spread, datac = stuff to burn */
#define TYPE_SMOKE 1		/* Smoke - datas = time until it gets lost */
#define TYPE_DEC   2		/* Decoration, like those 2 previous ones. obj = obj# of DS it is related to, datac = char it replaced */
#define TYPE_LAST_DEC 2
#define TYPE_MINE  3		/* datac = type, datas = damage it causes, datai = extra */
#define TYPE_BUILD 4		/* Building obj=# of the internal map */
#define TYPE_LEAVE 5		/* Reference to what happens when U leave ; obj=# of new map */
#define TYPE_ENTRANCE 6		/* datac = dir of entry (0=dontcare), x/y */

#define TYPE_LINKED  7		/* If this exists, we got a maplink propably */
#define TYPE_BITS    8		/* hangar / mine bit array, if any (in datai) */
#define TYPE_B_LZ    9		/* Land-block */
#define TYPE_MINING  10		/* Mining epicenter */

#define NUM_MAPOBJTYPES 10

#define BUILDFLAG_CS  1		/* Externally CS */
#define BUILDFLAG_CSI 2		/* Internally CS */
#define BUILDFLAG_DSS 4		/* DontShowStep when someone steps on the base */
#define BUILDFLAG_NOB 8		/* No way to break in */
#define BUILDFLAG_HID 16	/* Really hidden */

#define MapIsCS(map)       (map->buildflag & BUILDFLAG_CSI)
#define BuildIsCS(map)     (map->buildflag & BUILDFLAG_CS)
#define BuildIsHidden(map) (map->buildflag & (BUILDFLAG_DSS|BUILDFLAG_HID))
#define BuildIsSafe(map)   (map->buildflag & BUILDFLAG_NOB)
#define BuildIsInvis(map)  (map->buildflag & BUILDFLAG_HID)

#define MapUnderSpecialRules(map) (map->flags & MAPFLAG_SPEC)
#define MapGravityMod(map)        map->grav
#define MapGravity                MapGravityMod
#define MapIsVacuum(map)          (map->flags & MAPFLAG_VACUUM)
#define MapTemperature(map)       map->temp
#define MapCloudbase(map)	  map->cloudbase
#define MapIsUnderground(map)	  (map->flags & MAPFLAG_UNDERGROUND)

typedef struct mapobj_struct {
    short x, y;
    dbref obj;
    char type;
    char datac;
    short datas;
    int datai;
    struct mapobj_struct *next;
} mapobj;

#define MECHMAPFLAG_MOVED  1	/* mech has moved since last LOS update */

#define MECHLOSFLAG_SEEN   0x0001	/* x <mech> has seen <target> (before) */
#define MECHLOSFLAG_SEESP  0x0002	/* x <mech> sees <target> now w/ primary */
#define MECHLOSFLAG_SEESS  0x0004	/* x <mech> sees <target> now w/ secondary */
#define MECHLOSFLAG_SEEC2  0x0008	/* The terrain flag has been calculated */
#define MECHLOSFLAG_MNTN   0x0010	/* Intervening mountain hexes? */

#define MECHLOSFLAG_WOOD   0x0020	/* The number of intervening woods hexes */
#define MECHLOSFLAG_WOOD2  0x0040	/* Heavywood = 2, Light = 1 */
#define MECHLOSFLAG_WOOD3  0x0080
#define MECHLOSFLAG_WOOD4  0x0100
#define MECHLOSBYTES_WOOD  4
#define MECHLOSMAX_WOOD    16	/* No sensors penetrate this far */

#define MECHLOSFLAG_WATER  0x0200	/* The number of intervening water hexes */
#define MECHLOSFLAG_WATER2 0x0400
#define MECHLOSFLAG_WATER3 0x0800
#define MECHLOSBYTES_WATER 3
#define MECHLOSMAX_WATER   8	/* No sensors penetrate this far */

#define MECHLOSFLAG_PARTIAL 0x1000	/* The target's in partial cover for da shooter */
#define MECHLOSFLAG_FIRE    0x2000	/* Fire between mech and target */
#define MECHLOSFLAG_ECM     0x4000  /* Finay, LoS passthrough ECM blocking - DJ */
#define MECHLOSFLAG_BLOCK   0x8000	/* Something blocks vision (elevation propably) */
/* Short to Integer Expansion - DJ */

#define MECHLOSFLAG_BUILD   0x10000	/* Buildings blockign LoS */
#define MECHLOSFLAG_BUILD2  0x20000     /* Number of build (or wall) hexes intervening */
#define MECHLOSFLAG_BUILD3  0x40000
#define MECHLOSFLAG_BUILD4  0x80000
#define MECHLOSBYTES_BUILD  4
#define MECHLOSMAX_BUILD    16

#define MECHLOSFLAG_SMOKE     0x100000  /* New Smoke counting like woods as per Maxtech - DJ */
#define MECHLOSFLAG_SMOKE2    0x200000
#define MECHLOSFLAG_SMOKE3    0x400000
#define MECHLOSFLAG_SMOKE4    0x800000
#define MECHLOSBYTES_SMOKE    4
#define MECHLOSMAX_SMOKE      16
#define MECHLOSFLAG_ANGELECM  0x1000000
#define MECHLOSFLAG_SPREVHEX  0x2000000
#define MECHLOSFLAG_WBLOCK    0x3000000

typedef struct {
    dbref mynum;		/* My dbref */
    unsigned char **map;	/* The map */
    char mapname[MAP_NAME_SIZE + 1];

    short map_width;		/* Width of map <MAPX  */
    short map_height;		/* Height of map */

    char temp;			/* Temperature, in celsius degrees */
    unsigned char grav;		/* Gravity, if any in 1/100 G's */
    short cloudbase;
    char unused_char;
    char mapvis;		/* Visibility on the map, used as base for most sensor types */
    short maxvis;		/* maximum visibility (usually mapvis * n) */
    char maplight;
    short winddir, windspeed;
    /* Now, da wicked stuff */
    byte flags;

    mapobj *mapobj[NUM_MAPOBJTYPES];
    short cf, cfmax;
    dbref onmap;
    char buildflag;

    unsigned char first_free;	/* First free on da map */
    dbref *mechsOnMap;		/* Mechs on the map */
    unsigned int **LOSinfo;	/* Line of sight info */

    /* 1 = mech has moved recently
       2 = mech has possible-LOS event ongoing */
    char *mechflags;
    short moves;		/* Cheat to prevent idle CPU hoggage */
    short movemod;
} MAP;

/* Used by navigate_sketch_map */
#define NAVIGATE_LINES 13

#include "p.map.bits.h"
#include "p.map.h"
#include "p.map.obj.h"
#include "p.map.dynamic.h"

extern void newfreemap(dbref key, void **data, int selector);
extern void map_update(dbref obj, void *data);

#define set_buildflag(a,b) silly_atr_set((a), A_BUILDFLAG, tprintf("%d", (b)))
#define get_buildflag(a) atoi(silly_atr_get((a), A_BUILDFLAG))
#define set_buildcf(a,b) silly_atr_set((a), A_BUILDCF, tprintf("%d %d", (b), (b)))
#endif
