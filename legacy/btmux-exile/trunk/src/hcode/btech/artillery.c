/* 
   Artillery code for
   - standard rounds (damage to target hex, damage/2 to neighbor hexes)
   - smoke rounds (to be implemented)
   - fascam rounds (to be implemented) 
 */

#include <math.h>

#include "mech.h"
#include "artillery.h"
#include "mech.events.h"
#include "create.h"
#include "p.artillery.h"
#include "p.mech.combat.h"
#include "p.mech.utils.h"
#include "p.map.obj.h"
#include "p.mech.hitloc.h"
#include "p.mine.h"
#include "spath.h"
#include "btechstats.h"

struct artillery_shot_type *free_shot_list = NULL;
static void artillery_hit(artillery_shot * s);

static const char *artillery_type(artillery_shot * s)
{
    if (s->type == CL_ARROW || s->type == IS_ARROW)
	return "a missile";
    return "a round";
}

static struct {
    int dir;
    char *desc;
} arty_dirs[] = {
    {
    0, "north"}, {
    60, "northeast"}, {
    90, "east"}, {
    120, "southeast"}, {
    180, "south"}, {
    240, "southwest"}, {
    270, "west"}, {
    300, "northwest"}, {
    0, NULL}
};

static const char *artillery_direction(artillery_shot * s)
{
    float fx, fy, tx, ty;
    int b, d, i, best = -1, bestd = 0;

    MapCoordToRealCoord(s->from_x, s->from_y, &fx, &fy);
    MapCoordToRealCoord(s->to_x, s->to_y, &tx, &ty);
    b = FindBearing(fx, fy, tx, ty);
    for (i = 0; arty_dirs[i].desc; i++) {
	d = abs(b - arty_dirs[i].dir);
	if (best < 0 || d < bestd) {
	    best = i;
	    bestd = d;
	}
    }
    if (best < 0)
	return "Invalid";
    return arty_dirs[best].desc;
}

int artillery_round_flight_time(float fx, float fy, float tx, float ty)
{
    int delaymod = 2;
    int delay = MAX(ARTILLERY_MINIMUM_FLIGHT,
/*	(FindHexRange(fx, fy, tx, ty) / ARTY_SPEED)); */
	(FindHexRange(fx, fy, tx, ty) / delaymod));

    /* XXX Different weapons, diff. speed? */
    return delay;
}


static void artillery_hit_event(EVENT * e)
{
    artillery_shot *s = (artillery_shot *) e->data;

    artillery_hit(s);
    ADD_TO_LIST_HEAD(free_shot_list, next, s);
}

void artillery_shoot(MECH * mech, int targx, int targy, int windex,
    int wmode, int ishit)
{
    struct artillery_shot_type *s;
    float fx, fy, tx, ty;
    MAP *map;

    if (free_shot_list) {
	s = free_shot_list;
	free_shot_list = free_shot_list->next;
    } else
	Create(s, artillery_shot, 1);
    s->from_x = MechX(mech);
    s->from_y = MechY(mech);
/*    s->to_x = targx; */
/*    s->to_y = targy; */
    s->type = windex;
    s->mode = wmode;
    s->ishit = ishit;
    s->shooter = mech->mynum;
    s->map = mech->mapindex;
    map = getMap(mech->mapindex);
    s->to_x = (targx < 2 ? 2 : targx > (map->map_width - 2) ? (map->map_width - 2) : targx);
    s->to_y = (targy < 2 ? 2 : targy > (map->map_height) - 2 ? (map->map_height - 2) : targy);
    MechLOSBroadcast(mech, tprintf("shoots %s towards %s!",
	    artillery_type(s), artillery_direction(s)));
    MapCoordToRealCoord(s->from_x, s->from_y, &fx, &fy);
    MapCoordToRealCoord(s->to_x, s->to_y, &tx, &ty);
    event_add(artillery_round_flight_time(fx, fy, tx, ty), 0, EVENT_DHIT,
	artillery_hit_event, (void *) s, NULL);
}

static int blast_arcf(float fx, float fy, MECH * mech)
{
    int b, dir;

    b = FindBearing(MechFX(mech), MechFY(mech), fx, fy);
    dir = AcceptableDegree(b - MechFacing(mech));
    if (dir > 120 && dir < 240)
	return BACK;
    if (dir > 300 || dir < 60)
	return FRONT;
    if (dir > 180)
	return LEFTSIDE;
    return RIGHTSIDE;
}

static void inferno_end_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    MechStatus(mech) &= ~JELLIED;
    mech_notify(mech, MECHALL,
	"You feel suddenly far cooler as the fires finally die.");
}

void inferno_burn(MECH * mech, int time)
{
    int l;

    if (!(MechStatus(mech) & JELLIED)) {
	MechStatus(mech) |= JELLIED;
	MECHEVENT(mech, EVENT_BURN, inferno_end_event, time, 0);
	return;
    }
    l = event_last_type_data(EVENT_BURN, (void *) mech) + time;
    event_remove_type_data(EVENT_BURN, (void *) mech);
    MECHEVENT(mech, EVENT_BURN, inferno_end_event, l, 0);
}

#if 0
static int blast_arc(int targetx, int targety, MECH * mech)
{
    float fx, fy;

    MapCoordToRealCoord(targetx, targety, &fx, &fy);
    return blast_arcf(fx, fy, mech);
}
#endif

#define TABLE_GEN   0
#define TABLE_PUNCH 1
#define TABLE_KICK  2


void blast_hit_hexf(MAP * map, int dam, int singlehitsize, int heatdam,
    float fx, float fy, float tfx, float tfy, char *tomsg, char *otmsg,
    int table, int safeup, int safedown, int isunderwater, MECH * source, artillery_shot *s)
{
    MECH *tempMech;
    int loop;
    int isrear = 0, iscritical = 0, hitloc;
    int damleft, arc, ndam;
    int ground_zero;
    int wtype;
    short tx, ty;

    RealCoordToMapCoord(&tx, &ty, fx, fy);
    if (tx < 1 || ty < 1 || tx >= (map->map_width - 1) || ty >= (map->map_height - 1))
	return;
    if (!tomsg || !otmsg)
	return;
    if (isunderwater)
	ground_zero = Elevation(map, tx, ty);
    else
	ground_zero = MAX(0, Elevation(map, tx, ty));
    for (loop = 0; loop < map->first_free; loop++)
	if (map->mechsOnMap[loop] >= 0) {
	    tempMech = (MECH *)
		FindObjectsData(map->mechsOnMap[loop]);
	    if (!tempMech)
		continue;
	    if (MechX(tempMech) != tx || MechY(tempMech) != ty)
		continue;
	    /* Far too high.. */
	    if (MechZ(tempMech) >= (safeup + ground_zero))
		continue;
	    /* Far too below (underwater, mostly) */
	    if (		/* MechTerrain(tempMech) == WATER &&  */
		MechZ(tempMech) <= (ground_zero - safedown))
		continue;
#if 0				/* Twinky towertag rule - shoo! */
	    if (MechMove(tempMech) == MOVE_NONE)
		continue;
#endif
	    MechLOSBroadcast(tempMech, otmsg);
	    mech_notify(tempMech, MECHALL, tomsg);
	    arc = blast_arcf(tfx, tfy, tempMech);
	    if (arc == BACK)
		isrear = 1;
	    damleft = dam;
	    while (damleft > 0) {
		if (singlehitsize <= damleft)
		    ndam = singlehitsize;
		else
		    ndam = damleft;
		damleft -= ndam;
		switch (table) {
		case TABLE_PUNCH:
		    FindPunchLoc(tempMech, hitloc, arc, iscritical,
			isrear);
		    break;
		case TABLE_KICK:
		    FindKickLoc(tempMech, hitloc, arc, iscritical, isrear);
		    break;
		default:
		    hitloc =
			FindHitLocation(tempMech, arc, &iscritical,
			&isrear, NULL);
		}
		DamageMech(tempMech, (source == NULL ? tempMech : source), 0, -1, hitloc, isrear,
		    iscritical, ndam, 0, (s ? s->type : -1), 0, 0);
		if (source)
		    AccumulateArtyXP(MechPilot(source), source, tempMech);
	    }
	    if (heatdam)
 	        heat_effect(tempMech, tempMech, heatdam);
	}
}

void blast_hit_hex(MAP * map, int dam, int singlehitsize, int heatdam,
    int fx, int fy, int tx, int ty, char *tomsg, char *otmsg, int table,
    int safeup, int safedown, int isunderwater, MECH * source, artillery_shot *s)
{
    float ftx, fty;
    float ffx, ffy;

    MapCoordToRealCoord(tx, ty, &ftx, &fty);
    MapCoordToRealCoord(fx, fy, &ffx, &ffy);
    blast_hit_hexf(map, dam, singlehitsize, heatdam, ffx, ffy, ftx, fty,
	tomsg, otmsg, table, safeup, safedown, isunderwater, source, s);
}

void blast_hit_hexesf(MAP * map, int dam, int singlehitsize, int heatdam,
    float fx, float fy, float ftx, float fty, char *tomsg, char *otmsg,
    char *tomsg1, char *otmsg1, int table, int safeup, int safedown,
    int isunderwater, int doneighbors, MECH * source, artillery_shot *s)
{
    int x1, y1, x2, y2;
    int dm;
    short tx, ty;
    float hx, hy;
    float t = FindXYRange(fx, fy, ftx, fty);

    dm = MAX(1, (int) t + 1);
#define NUM_NEIGHBORS 6
    blast_hit_hexf(map, dam / dm, singlehitsize, heatdam / dm, fx, fy, ftx,
	fty, tomsg, otmsg, table, safeup, safedown, isunderwater, source, s);
    if (!doneighbors)
	return;
    RealCoordToMapCoord(&tx, &ty, fx, fy);
    for (x1 = (tx - doneighbors); x1 <= (tx + doneighbors); x1++)
	for (y1 = (ty - doneighbors); y1 <= (ty + doneighbors); y1++) {
	    int spot;

	    if ((dm = MyHexDist(tx, ty, x1, y1, 0)) > doneighbors)
		continue;
	    if ((tx == x1) && (ty == y1))
		continue;
	    x2 = BOUNDED(0, x1, map->map_width - 2);
	    y2 = BOUNDED(0, y1, map->map_height - 2);
	    if (x1 != x2 || y1 != y2)
		continue;
	    spot = (x1 == tx && y1 == ty);
	    MapCoordToRealCoord(x1, y1, &hx, &hy);
	    dm++;
	    if (!(dam / dm))
		continue;
	    blast_hit_hexf(map, dam / dm, singlehitsize, heatdam / dm, hx,
		hy, ftx, fty, spot ? tomsg : tomsg1, spot ? otmsg : otmsg1,
		table, safeup, safedown, isunderwater, source, s);

	    /*
	     * Added in burning woods when a mech's engine goes nova
	     *
	     * -Kipsta
	     * 8/4/99
	     */

	    switch (GetRTerrain(map, x1, y1)) {
	    case LIGHT_FOREST:
	    case HEAVY_FOREST:
		if (!find_decorations(map, x1, y1)) {
		    add_decoration(map, x1, y1, TYPE_FIRE, FIRE,
			FIRE_DURATION);
		}

		break;
	    }
	}
}

void blast_hit_hexes(MAP * map, int dam, int singlehitsize, int heatdam,
    int tx, int ty, char *tomsg, char *otmsg, char *tomsg1, char *otmsg1,
    int table, int safeup, int safedown, int isunderwater, int doneighbors, MECH * source, artillery_shot *s)
{
    float fx, fy;

    MapCoordToRealCoord(tx, ty, &fx, &fy);
    blast_hit_hexesf(map, dam, singlehitsize, heatdam, fx, fy, fx, fy,
	tomsg, otmsg, tomsg1, otmsg1, table, safeup, safedown,
	isunderwater, doneighbors, source, s);
}

static void artillery_hit_hex(MAP * map, artillery_shot * s, int type,
    int mode, int dam, int tx, int ty, int isdirect)
{
    char buf[LBUF_SIZE];
    char buf1[LBUF_SIZE];
    char buf2[LBUF_SIZE];
    float fx, fy, xx, yy, rng = 1.0;
    int bearing, loop = 0;
    short x2, y2;
    int i;
    MECH * tmech;
    MECH * source;

    if (tx < 1 || ty  < 1 || tx >= (map->map_width - 1) || ty >= (map->map_height - 1))
	return;
    if (!(source = getMech(s->shooter)))
	source = NULL;

    if (mode & SMOKE_MODE) 
		{
	/* Add smoke */
	if (!find_decorations(map, tx, ty)) { 
		if (!(tx < 1 || ty < 1 || tx >= (map->map_width - 1) || ty >= (map->map_height - 1)))
		    add_decoration(map, tx, ty, TYPE_SMOKE, HSMOKE, ((GunStat(type, mode, GUN_DAMAGE) / 5) * 30));
		return;
		}
    return;
    }
    if (mode & INCEND_MODE) {
	/* Add Napalm! Burn babe burn! MUHAHHAHAAH! */
	if (!find_decorations(map, tx, ty))
		{
		if (!(tx < 1 || ty < 1 || tx >= (map->map_width - 1) || ty >= (map->map_height - 1))) {
		    add_decoration(map, tx, ty, TYPE_FIRE, FIRE, ((GunStat(type, mode, GUN_DAMAGE) / 5) * 30));
		    if (GetRTerrain(map, tx, ty) == SNOW)
    		        clear_hex(source, tx, ty, 0, s->type, 0);
		}
  		for (i = 0; i < map->first_free; i++) 
		    {
                    if ((tmech = (FindObjectsData(map->mechsOnMap[i]))) && map)
			if (MechX(tmech) == tx && MechY(tmech) == ty && abs(MechZ(tmech) - Elevation(map, tx, ty)) < 4)
			     heat_effect(tmech, tmech, (GunStat(type, mode, GUN_DAMAGE)));
		    }
		return;
		}
    return;
    }
    if (mode & MINE_MODE) {
	if (!(tx < 1 || ty < 1 || tx >= (map->map_width - 1) || ty >= (map->map_height - 1)))
	    add_mine(map, tx, ty, dam);
	return;
    }
    if (!(mode & CLUSTER_MODE)) {
	if (isdirect)
	    sprintf(buf1, "receives a direct hit!");
	else
	    sprintf(buf1, "is hit by fragments!");
	if (isdirect)
	    sprintf(buf2, "You receive a direct hit!");
	else
	    sprintf(buf2, "You are hit by fragments!");
    } else {
	if (dam > 2)
	    strcpy(buf, "bomblets");
	else
	    strcpy(buf, "a bomblet");
	sprintf(buf1, "is hit by %s!", buf);
	sprintf(buf2, "You are hit by %s!", buf);
    }

/*    blast_hit_hex(map, dam, (mode & CLUSTER_MODE) ? 1 : 5, 0, tx, ty, tx,
	ty, buf2, buf1, (mode & CLUSTER_MODE) ? TABLE_PUNCH : TABLE_GEN,
	10, 4, 0, source); */
    blast_hit_hex(map, dam, (mode & CLUSTER_MODE) ? 1 : 5, 0, tx, ty, tx,
	ty, buf2, buf1, TABLE_GEN, 5, 2, 0, source, s);
/* Added and removed and tweaked again. Localizing fire-from-arty to INCEND_MODE rounds. - DJ */
if (mode & CLUSTER_MODE) 
 {  
 if ((!find_decorations(map, tx, ty)) && (Number(1,12) > 10)) 
                {      
		if (IsForest(GetRTerrain(map, tx, ty))) 
		    if (!(tx < 1 || ty < 1 || tx >= (map->map_width - 1) || ty >= (map->map_height - 1)))
	                    add_decoration(map, tx, ty, TYPE_FIRE, FIRE, FIRE_DURATION); 
                } 
 } else { 
     MapCoordToRealCoord(tx, ty, &xx, &yy); 
     for (loop = 0; loop < 6; loop++) 
         { 
          bearing = (loop == 0 ? 0 : loop == 1 ? 60 : loop == 2 ? 120 : loop == 3 ? 180 : 
                      loop == 4 ? 240 : 300); 
          FindXY(xx, yy, bearing, rng, &fx, &fy); 
          RealCoordToMapCoord(&x2, &y2, fx, fy); 
 	  if (Number(1,12) > 10) 
 	  {  
 	  if (IsForest(GetRTerrain(map, x2, y2)) && (!find_decorations(map, x2, y2)))
	    {  
	    if (!(x2 < 1 || y2 < 1 || x2 >= (map->map_width - 1) || y2 >= (map->map_height - 1)))
                add_decoration(map, x2, y2, TYPE_FIRE, FIRE, FIRE_DURATION); 
            }
          } 
         }  
  } 
}


int dirs[6][2] = {
    {0, -1},
    {1, 0},
    {1, 1},
    {0, 1},
    {-1, 1},
    {-1, 0}
};

static void artillery_hit_neighbors(MAP * map, artillery_shot * s,
    int type, int mode, int dam, int tx, int ty, int r)
{
    int x1, y1, x2, y2;
    int i;

    if (mode & (INCEND_MODE|SMOKE_MODE))
	{
        int ii, diam = ((r * 2) + 1);
        int xbase = (tx - r);
        int ybase = (ty - r);
        float fx1, fy1, fx2, fy2, ran;
	ran = ((float) r + .25);
        x2 = tx;
	y2 = tx;
        MapCoordToRealCoord(tx, ty, &fx1, &fy1);
        for (i = 0; i < diam; i++)
           {
           x2 = (xbase + i);
           for (ii = 0; ii < diam; ii++)
               {
               y2 = (ybase + ii);
               if (x2 < 1 || x2 >= (map->map_width - 1) || y2 < 1 || y2 >= (map->map_height - 1))
                   continue;
               MapCoordToRealCoord(x2, y2, &fx2, &fy2);
/*	       SendDebug(
		tprintf("RadiusArty : TargX: %d %3.3f TarggY: %d %3.3f ScanX: %d %3.3f ScanY: %d %3.3f Range : %2.2f XBase: %d YBase : %d Radius : %d",
		tx, fx1, ty, fy1, x2, fx2, y2, fy2, FindHexRange(fx1, fy1, fx2, fy2), xbase, ybase, r)); */
               if (FindHexRange(fx1, fy1, fx2, fy2) <= ran)
                   {
                   artillery_hit_hex(map, s, type, mode, dam, x2, y2, 0);
                   }
               }
           } 
    } else { 
        for (i = 0; i < NUM_NEIGHBORS; i++) 
	    {
	x1 = tx + dirs[i][0];
	y1 = ty + dirs[i][1];
	if (tx % 2 && !(x1 % 2))
	    y1--;
	x2 = BOUNDED(1, x1, map->map_width - 1);
	y2 = BOUNDED(1, y1, map->map_height - 1);
	if (x1 != x2 || y1 != y2)
	    continue; 
        if (x1 < 1 || x1 >= (map->map_width - 1) || y1 < 1 || y1 >= (map->map_height - 1))
            continue;
/*        SendDebug(tprintf("Map: %d Type : %d Mode : %d Dam : %d x1: %d y1 %d",map->mynum, type, mode, dam, x1, y1)); */
	artillery_hit_hex(map, s, type, mode, dam, x1, y1, 0); 
            }
        }

}

/*
static void artillery_hit_radius(MAP * map, artillery_shot * s,
	int type, int mode, int dam, int tx, int ty, int r)
{
     int i, ii, diam = ((r * 2) + 1), x = tx, y = ty;
     int xbase = (x - r);
     int ybase = (y - r);
     float fx1, fy1, fx2, fy2;

     MapCoordToRealCoord(tx, ty, &fx1, &fy1); 
     for (i = 0; i < diam; i++)
	{
	x = (xbase + i);
	for (ii = 0; ii < diam; ii++)
	    {
	    y = (ybase + i);
            if (x2 < 1 || x2 >= (map->map_width - 1) || y2 < 1 || y2 >= (map->map_height - 1))
                continue;
            MapCoordToRealCoord(x, y, &fx2, &fy2);
	    if (FindHexRange(fx1, fy1, fx2, fy2) <= r)
		{
                artillery_hit_hex(map, s, type, mode, dam, x, y, 0);
		}
	    }
	}
}
*/

static void artillery_cluster_hit(MAP * map, artillery_shot * s, int type,
    int mode, int dam, int tx, int ty)
{
    /* Main idea: Pick <dam/2> bombs of 2pts each, and scatter 'em
       over 5x5 area with weighted numbers */
    int xd, yd, x, y;
    int i;

/*   int bombs = (dam / 2); */
    int targets[5][5];
    int d;

    bzero(targets, sizeof(targets));
    for (i = 0; i < dam; i++) {
	do {
	    xd = Number(-2, 0) + Number(0, 2);
	    yd = Number(-2, 0) + Number(0, 2);
	    x = tx + xd;
	    y = ty + yd;
	}
	while (!(x == BOUNDED(0, x, map->map_width - 1) &&
		y == BOUNDED(0, y, map->map_height - 1)));
	/* Whee.. it's time to drop a bomb to the hex */
	targets[xd + 2][yd + 2]++;
    }
    for (xd = 0; xd < 5; xd++)
	for (yd = 0; yd < 5; yd++)
	    if ((d = targets[xd][yd]))
		if (!((xd + tx - 2) < 1 || (yd + ty - 2) < 1 || (xd + tx - 2) >= (map->map_width - 1) || (yd + ty - 2) >= (map->map_height - 1))) 
		    artillery_hit_hex(map, s, type, mode, d * 2, xd + tx - 2,
		    yd + ty - 2, 1);
}

void artillery_FriendlyAdjustment(dbref mechnum, MAP * map, int x, int y)
{
    MECH *mech;
    MECH *spotter;
    MECH *tempMech = NULL;

    if (!(mech = getMech(mechnum)))
	return;
    /* Ok.. we've a valid guy */
    spotter = getMech(MechSpotter(mech));
    if (!((MechTargX(mech) == x && MechTargY(mech) == y)
	    || (spotter && (MechTargX(spotter) == x &&
		    MechTargY(spotter) == y))))
	return;
    /* Ok.. we've a valid target to adjust fire on */
    /* Now, see if we've any friendlies in LOS.. NOTE: FRIENDLIES ;-) */
    if (spotter) {
	if (MechSeesHex(spotter, map, x, y))
	    tempMech = spotter;
    } else
	tempMech = find_mech_in_hex(mech, map, x, y, 2);
    if (!tempMech)
	return;
    if (!Started(tempMech) || !Started(mech))
	return;
    if (spotter) {
	mech_notify(mech, MECHSTARTED,
	    tprintf("%s sent you some trajectory-correction data.",
		GetMechToMechID(mech, tempMech)));
	mech_notify(tempMech, MECHSTARTED,
	    tprintf("You provide %s with information about the miss.",
		GetMechToMechID(tempMech, mech)));
    }
    MechFireAdjustment(mech)++;
}

static void artillery_hit(artillery_shot * s)
{
    /* First, we figure where it exactly hits. Our first-hand information
       is only whether it hits or not, not _where_ it hits */
    double dir;
    int di;
    int dist;
    int weight;
    MAP *map = getMap(s->map);
    int original_x = 0, original_y = 0;
    int dam = ((s->mode & CLUSTER_MODE) ? ((GunStat(s->type, s->mode, GUN_DAMAGE) * 3) / 2) : GunStat(s->type, s->mode, GUN_DAMAGE));

    if (!map)
	return;
    if (!s->ishit) {
	/* Shit! We missed target ;-) */
	/* Time to calculate a new target hex */
	di = Number(0, 359);
	dir = di * TWOPIOVER360;
	dist = Number(2, 10);
	weight = 100 * (dist * 6) / ((dist * 6 + map->windspeed));
	di = (di * weight + map->winddir * (100 - weight)) / 100;
	dist =
	    (dist * weight + (map->windspeed / 6) * (100 - weight)) / 100;
	original_x = s->to_x;
	original_y = s->to_y;
	s->to_x = s->to_x + dist * cos(dir);
	s->to_y = s->to_y + dist * sin(dir);
	s->to_x = BOUNDED(0, s->to_x, map->map_width - 1);
	s->to_y = BOUNDED(0, s->to_y, map->map_height - 1);
        s->to_x = (s->to_x < 2 ? 2 : s->to_x > map->map_width - 2 ? map->map_width - 2 : s->to_x);
	s->to_y = (s->to_y < 2 ? 2 : s->to_y > map->map_height - 2 ? map->map_height -2 : s->to_y); 
	/* Time to calculate if any friendlies have LOS to hex,
	   and if so, adjust fire adjustment unless you lack information /
	   have changed target */
    }
    /* It's time to run for your lives, lil' ones ;-) */
    if (!(s->mode & ARTILLERY_MODES))
	HexLOSBroadcast(map, s->to_x, s->to_y, tprintf("%s fire hits $H from a bearing of %i!",
		&MechWeapons[s->type].name[3], FindBearing(s->to_x, s->to_y, s->from_x, s->from_y)));
    else if (s->mode & CLUSTER_MODE)
	HexLOSBroadcast(map, s->to_x, s->to_y, tprintf("A rain of small bomblets hits $H's surroundings from a bearing of %i!",
	FindBearing(s->to_x, s->to_y, s->from_x, s->from_y)));
    else if (s->mode & MINE_MODE)
	HexLOSBroadcast(map, s->to_x, s->to_y, tprintf("A rain of small bomblets hits $H from a bearing of %i!", 
	FindBearing(s->to_x, s->to_y, s->from_x, s->from_y)));
    else if (s->mode & SMOKE_MODE)
	HexLOSBroadcast(map, s->to_x, s->to_y, tprintf("A %s %s hits $h from a bearing of %i, and smoke starts to billow!",
		&MechWeapons[s->type].name[3], &(artillery_type(s)[2]), FindBearing(s->to_x, s->to_y, s->from_x, s->from_y)));
    else if (s->mode & INCEND_MODE)
	HexLOSBroadcast(map, s->to_x, s->to_y, tprintf("A %s %s hits $h from a bearing of %i, covering the hex in napalm!",
		&MechWeapons[s->type].name[3], &(artillery_type(s)[2]), FindBearing(s->to_x, s->to_y, s->from_x, s->from_y)));
    /* Basic theory:
       - smoke / ordinary rounds are spread with the ordinary functions 
       - mines are otherwise ordinary except no hitting of neighbor hexes
       - cluster bombs are special 
     */
    if (!(s->mode & CLUSTER_MODE)) {
	/* Enjoy ourselves in all neighbor hexes, too */
	artillery_hit_hex(map, s, s->type, s->mode, dam, s->to_x, s->to_y,
	    1);
	if (!(s->mode & MINE_MODE))
	    artillery_hit_neighbors(map, s, s->type, s->mode, dam / 2,
		s->to_x, s->to_y, ((s->mode & (SMOKE_MODE|INCEND_MODE)) ? ((dam / 5) - 1 < 0 ? 0 : (dam / 5) - 1) : 0));

    } else
	artillery_cluster_hit(map, s, s->type, s->mode, dam, s->to_x,
	    s->to_y);
    if (!s->ishit)
	artillery_FriendlyAdjustment(s->shooter, map, original_x,
	    original_y);
}
