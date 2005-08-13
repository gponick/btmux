#include "mech.h"
#include "mech.events.h"
#include "p.mech.update.h"
#include "p.bsuit.h"
#include "p.mech.utils.h"
#include "p.mech.hitloc.h"
#include "p.mech.combat.h"
#include <math.h>

#define TMP_TERR '1'

static void swim_except(MAP * map, MECH * mech, int x, int y, char *msg,
    int isbridge)
{
    int i, j;
    MECH *t;

    if (!(Elevation(map, x, y)))
	return;
    for (i = 0; i < map->first_free; i++)
	if ((j = map->mechsOnMap[i]) > 0)
	    if ((t = getMech(j)))
		if (t != mech) {
		    if (MechX(t) != x || MechY(t) != y)
			continue;
		    MechTerrain(t) = WATER;
		    if ((!isbridge && (MechZ(t) == 0) &&
			    (MechMove(t) != MOVE_HOVER)) || (isbridge &&
			    MechZ(t) == MechElev(t))) {
			MechLOSBroadcast(t, msg);
			MechFalls(t, MechElev(t) + isbridge, 0);
			if (MechType(t) == CLASS_VEH_GROUND)
			    if (!Destroyed(t)) {
				mech_notify(t, MECHALL,
				    "Water renders your vehicle inoperational.");
				Destroy(t);
			    }
		    }
		}
}

static void break_sub(MAP * map, MECH * mech, int x, int y, char *msg)
{
    int isbridge = GetRTerrain(map, x, y) == BRIDGE;
    int iswall = GetRTerrain(map, x, y) == WALL;
    int hexelev = GetElevation(map, x, y);
    int loop = 0;
    int lowelev = 9;
    int highelev = 0;
    float fx, fy;
    float xx, yy, rng = 1.0;
    int bearing;
    int t;
    short tx, ty;
    int i, j;
    MECH *tt;

    if (isbridge && !mech)
       {
       SetTerrain(map, x, y, DBRIDGE);
/*       SetElevation(map, x, y, 1); */
/*       swim_except(map, mech, x, y, msg, isbridge); */
       }

    if (iswall && !(mech)) {
       MapCoordToRealCoord(x, y, &xx, &yy);
       for (loop = 0; loop < 6; loop++) {
		bearing = (loop == 0 ? 0 : loop == 1 ? 60 : loop == 2 ? 120 : loop == 3 ? 180 :
			 loop == 4 ? 240 : 300);
		FindXY(xx, yy, bearing, rng, &fx, &fy);
		RealCoordToMapCoord(&tx, &ty, fx, fy);
		if (tx <= 0 || ty <= 0 || tx >= map->map_width - 1 || ty >= map->map_height - 1)
		    continue;
	 	t = GetElevation(map, tx, ty);
		if (GetRTerrain(map, tx, ty) != BUILDING) {
		 	lowelev = (lowelev < t ? lowelev : t);
			highelev = (highelev > t ? highelev : t);
		    }
	  }
/*       if (!(Elevation(map, x, y)))
   	     return; */
        SetElevation(map, x, y, BOUNDED(0,
                       hexelev <= 0 ? 0 : (hexelev - 1) < (highelev - 4) ? (highelev - 4) : (hexelev - 1) < lowelev ? lowelev : hexelev - 1,
                     9));
        if (hexelev != GetElevation(map, x, y)) {
	  for (i = 0; i < map->first_free; i++)
	    if ((j = map->mechsOnMap[i]) > 0)
		if ((tt = getMech(j))) {
		    if (MechX(tt) != x || MechY(tt) != y)
			continue;

		/* Always update the cached MechTerrain, so OD's and jumps will land at right Z */
	            MechTerrain(tt) = GetTerrain(map, x, y);
		    MechElev(tt) = GetElevation(map, x, y);

	            if (MechZ(tt) == hexelev && !Jumping(tt) && !OODing(tt)) {
			if (FlyingT(tt) && !Landed(tt))
			    continue;
			mech_notify(tt, MECHALL, "You plummet to the ground as the ground beneath you leaves!");
			MechLOSBroadcast(tt, "plummets to the ground!");
			MechFalls(tt, abs(MechElev(tt) - hexelev), 0);
			/* those two shouldn't be necessary at all, but don't hurt. */
			MechZ(tt) = MechElev(tt); 
			MechFZ(tt) = MechZ(tt) * ZSCALE;
		    }
               	 }
             }
	}
	if ((GetRTerrain(map, x, y) == ICE) && !(mech)) {
		SetTerrain(map, x, y, WATER);
/*		MechTerrain(mech) = WATER; */
/* TODO: loop over mechs in this hex. don't set on shooting mech */
		return;
	}
}

/* Up -> down */
void drop_thru_ice(MECH * mech)
{
    MAP *map = FindObjectsData(mech->mapindex);

    if (!map || !mech)
	return;
    mech_notify(mech, MECHALL, "You break the ice!");
    MechLOSBroadcast(mech, "breaks the ice!");
    if (MechMove(mech) != MOVE_FOIL) {
	if (MechElev(mech) > 0)
	    MechLOSBroadcast(mech, "vanishes into the waters!");
    }
    break_sub(map, mech, MechX(mech), MechY(mech), "goes swimming!");
/*  MechTerrain(mech) = WATER;  */
    SetTerrain(map, MechX(mech), MechY(mech), WATER);
    if (MechMove(mech) != MOVE_FOIL) {
	if (MechElev(mech) > 0)
	    MechFalls(mech, MechElev(mech), 0);
    }
}

/* Down -> up */
void break_thru_ice(MECH * mech)
{
    MAP *map = FindObjectsData(mech->mapindex);

    MarkForLOSUpdate(mech);
    mech_notify(mech, MECHALL, "You break thru the ice on your way up!");
    MechLOSBroadcast(mech, "breaks through the ice!");
    break_sub(map, mech, MechX(mech), MechY(mech), "goes swimming!");
    MechTerrain(mech) = WATER;
}

/* !FASA 3029 house rules - Weight doesnt' affect ice break, but speed does. Twist on FASA */
/* FASA speaks of pressure per square meter, making weight a mute factor. But said pressure */
/* would be increased by rapider movement. Ie, Running. DJ */

int possibly_drop_thru_ice(MECH * mech)
{
    int bth;

    if (MechMove(mech) == MOVE_HOVER || MechMove(mech) == MOVE_SUB || MechType(mech) == CLASS_MW)
	return 0;
    bth = (IsRunning(MechSpeed(mech), MMaxSpeed(mech)) ? 8 : 10);
    if (MechType(mech) == CLASS_BSUIT)
	bth += 2;
    if (Number(1, 12) < bth)
	return 0;
    drop_thru_ice(mech);
    return 1;
}

#define NUM_NEIGHBORS 6
extern int dirs[6][2];

int growable(MAP * map, int x, int y)
{
    int i, x1, y1, x2, y2;
    int wc = 0;
    int t;

    for (i = 0; i < NUM_NEIGHBORS; i++) {
	x1 = x + dirs[i][0];
	y1 = y + dirs[i][1];
	if (x % 2 && !(x1 % 2))
	    y1--;
	x2 = BOUNDED(0, x1, map->map_width - 1);
	y2 = BOUNDED(0, y1, map->map_height - 1);
	if (x1 != x2 || y1 != y2)
	    continue;
	t = GetRTerrain(map, x1, y1);
	if ((IsWater(t) && t != ICE) ||
	    GetRTerrain(map, x1, y1) == TMP_TERR) wc++;
    }
    if (wc <= 4 && (wc < 2 || (Number(1, 6) > wc)))
	return 1;
    return 0;
}

int meltable(MAP * map, int x, int y)
{
    int wc = 0;
    int i, x1, y1, x2, y2;

    for (i = 0; i < NUM_NEIGHBORS; i++) {
	x1 = x + dirs[i][0];
	y1 = y + dirs[i][1];
	if (x % 2 && !(x1 % 2))
	    y1--;
	x2 = BOUNDED(0, x1, map->map_width - 1);
	y2 = BOUNDED(0, y1, map->map_height - 1);
	if (x1 != x2 || y1 != y2)
	    continue;
	if (GetRTerrain(map, x1, y1) == ICE)
	    wc++;
    }
    if (wc > 4 && Number(1, 3) > 1)
	return 0;
    return 1;
}

int snow_growable(MAP * map, int x, int y)

{
    int i, x1, y1, x2, y2;
    int wc = 0;
    int t;

    for (i = 0; i < NUM_NEIGHBORS; i++) {
        x1 = x + dirs[i][0];
        y1 = y + dirs[i][1];
        if (x % 2 && !(x1 % 2))
            y1--;
        x2 = BOUNDED(0, x1, map->map_width - 1);
        y2 = BOUNDED(0, y1, map->map_height - 1);
        if (x1 != x2 || y1 != y2)
            continue;
        t = GetRTerrain(map, x1, y1);
        if ((t == TMP_TERR || t == MOUNTAINS || t == HEAVY_FOREST || t == LIGHT_FOREST) && t != WATER)
             wc++;
    }
    if (wc >= 1)
        return 1;
    return 0;
}

int snow_meltable(MAP * map, int x, int y)
{
    int wc = 0;
    int i, x1, y1, x2, y2;

    for (i = 0; i < NUM_NEIGHBORS; i++) {
        x1 = x + dirs[i][0];
        y1 = y + dirs[i][1];
        if (x % 2 && !(x1 % 2))
            y1--;
        x2 = BOUNDED(0, x1, map->map_width - 1);
        y2 = BOUNDED(0, y1, map->map_height - 1);
        if (x1 != x2 || y1 != y2)
            continue;
        if (GetRTerrain(map, x1, y1) == SNOW)
            wc++;
    }
    if (wc > 4 && Number(1, 3) > 1)
        return 0;
    return 1;

}
void ice_growth(dbref player, MAP * map, int num)
{
    int x, y;
    int count = 0;

    for (x = 0; x < map->map_width; x++)
	for (y = 0; y < map->map_height; y++)
	    if (GetRTerrain(map, x, y) == WATER)
		if (Number(1, 100) <= num && growable(map, x, y)) {
		    SetTerrain(map, x, y, TMP_TERR);
		    count++;
		}
    for (x = 0; x < map->map_width; x++)
	for (y = 0; y < map->map_height; y++)
	    if (GetRTerrain(map, x, y) == TMP_TERR)
		SetTerrain(map, x, y, ICE);
    if (count)
	notify(player, tprintf("%d hexes 'iced'.", count));
    else
	notify(player, "No hexes 'iced'.");
}

void ice_melt(dbref player, MAP * map, int num)
{
    int x, y;
    int count = 0;

    for (x = 0; x < map->map_width; x++)
	for (y = 0; y < map->map_height; y++)
	    if (GetRTerrain(map, x, y) == ICE)
		if (Number(1, 100) <= num && meltable(map, x, y)) {
		    break_sub(map, NULL, x, y,
			"goes swimming as ice breaks!");
		    SetTerrain(map, x, y, TMP_TERR);
		    count++;
		}
    for (x = 0; x < map->map_width; x++)
	for (y = 0; y < map->map_height; y++)
	    if (GetRTerrain(map, x, y) == TMP_TERR)
		SetTerrain(map, x, y, WATER);
    if (count)
	notify(player, tprintf("%d hexes melted.", count));
    else
	notify(player, "No hexes melted.");
}

void snow_growth(dbref player, MAP * map, int num)
{
    int x, y;
    int count = 0;

    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == GRASSLAND)
                if (Number(1, 100) <= num && snow_growable(map, x, y)) {
                    SetTerrain(map, x, y, TMP_TERR);
                    count++;
                }
    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == TMP_TERR)
                SetTerrain(map, x, y, SNOW);
    if (count)
        notify(player, tprintf("%d hexes 'snowed'.", count));
    else
        notify(player, "No hexes 'snowed'.");
}

void snow_melt(dbref player, MAP * map, int num)
{
    int x, y;
    int count = 0;

    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == SNOW)
                if (Number(1, 100) <= num && snow_meltable(map, x, y)) {
			SetTerrain(map, x, y, TMP_TERR);
                    count++;
                }
    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == TMP_TERR)
                SetTerrain(map, x, y, GRASSLAND);
    if (count)
        notify(player, tprintf("%d snow hexes melted.", count));
    else
        notify(player, "No snow hexes melted.");
}

void map_addice(dbref player, MAP * map, char *buffer)
{
    char *args[2];
    int num;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
	"Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");
    ice_growth(player, map, num);
}

void map_delice(dbref player, MAP * map, char *buffer)
{
    char *args[2];
    int num;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
	"Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");
    ice_melt(player, map, num);
}

void map_addsnow(dbref player, MAP * map, char *buffer)
{
    char *args[2];
    int num;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
        "Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");
    snow_growth(player, map, num);
}

void map_delsnow(dbref player, MAP * map, char *buffer)
{
    char *args[2];
    int num;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
        "Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");
    snow_melt(player, map, num);
}

void possibly_blow_ice(MECH * mech, int weapindx, int x, int y)
{
    MAP *map = FindObjectsData(mech->mapindex);

    if (GetRTerrain(map, x, y) != ICE)
	return;
    if (Number(1, 15) > GunStat(weapindx, 0, GUN_DAMAGE))
	return;
    HexLOSBroadcast(map, x, y, "The ice breaks from the blast!");
    break_sub(map, NULL, x, y, "goes swimming as ice breaks!");
}

void possibly_blow_bridge(MECH * mech, int weapindx, int x, int y)
{
    MAP *map = FindObjectsData(mech->mapindex);

    if ((GetRTerrain(map, x, y) != BRIDGE) && (GetRTerrain(map, x, y) != WALL))
	return;
    if (Number(1, 8 * (1 + (GetElev(map, x,
		    y) / 2))) > GunStat(weapindx, 0, GUN_DAMAGE)) {
	HexLOSBroadcast(map, x, y,
	    "The structure at $H shudders from direct hit!");
	return;
    }
    HexLOSBroadcast(map, x, y, "The structure at $H is blown apart!");
    break_sub(map, NULL, x, y,
	"goes swimming as the bridge is blown apart!");
}

int forestable(MAP * map, int x, int y)
{
    int i, xt, yt;

    for (i = 0; i < NUM_NEIGHBORS; i++) {
        xt = x + dirs[i][0];
        yt = y + dirs[i][1];
        if (x % 2 && !(xt % 2))
            yt--;
	if (xt < 1 || yt < 1 || xt >= map->map_width || yt >= map->map_height)
	    continue;
        if (GetRTerrain(map, abs(xt), abs(yt)) == LIGHT_FOREST || GetRTerrain(map, abs(xt), abs(yt)) == HEAVY_FOREST)
            return 1;
    }
    return 0;
}

void map_growforest(dbref player, MAP * map, char *buffer)
{

    char *args[2];
    int num, x, y, count = 0;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
        "Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");

    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == ROUGH)
                if (Number(1, 100) <= num) {
    		    if (forestable(map, x, y)) {
                        SetTerrain(map, x, y, (Number(1, 10) > 5 ? LIGHT_FOREST : HEAVY_FOREST));
                        count++;
		    }
                }
    if (count)
        notify(player, tprintf("%d hexes 'forested'.", count));
    else
        notify(player, "No hexes 'forested'.");
}

void map_fixbridge(dbref player, MAP * map, char *buffer)
{

    char *args[2];
    int num, x, y, count = 0;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
        "Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");

    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == DBRIDGE)
                if (Number(1, 100) <= num) {
                        SetTerrain(map, x, y, BRIDGE);
                        count++;
                    }
    if (count)
        notify(player, tprintf("%d hexes 'bridged'.", count));
    else
        notify(player, "No hexes 'bridged'.");
}

void map_fixwall(dbref player, MAP * map, char *buffer)
{

    char *args[2];
    int num, x, y, count = 0, i, j, k, xt, yt, elev = -1;
    MECH *tt;

    DOCHECK(mech_parseattributes(buffer, args, 2) != 1,
        "Invalid arguments!");
    DOCHECK(Readnum(num, args[0]), "Invalid number!");

    for (x = 0; x < map->map_width; x++)
        for (y = 0; y < map->map_height; y++)
            if (GetRTerrain(map, x, y) == WALL)
                if (Number(1, 100) <= num) {
		    elev = 0;
    		    for (i = 0; i < NUM_NEIGHBORS; i++) {
        		xt = x + dirs[i][0];
        		yt = y + dirs[i][1];
        		if (x % 2 && !(xt % 2))
            		    yt--;
			if (abs(xt) < 0 || abs(xt) >= map->map_width || abs(yt) < 0 || abs(yt) >= map->map_height)
			    continue;
        		if (GetRTerrain(map, abs(xt), abs(yt)) == WALL)
			    if (Elevation(map, abs(xt), abs(yt)) > elev)
				elev = Elevation(map, abs(xt), abs(yt));
		        }
		    if (Elevation(map, x, y) < elev) {
	    	        SetElevation(map, x, y, Elevation(map, x, y) + 1);

			/* Fix units in that map hex */
		        for (j = 0; j < map->first_free; j++)
            		   if ((k = map->mechsOnMap[j]) > 0)
		                if ((tt = getMech(k))) {
                		    if (MechX(tt) != x || MechY(tt) != y)
                        		continue;
				    MechTerrain(tt) = GetTerrain(map, x, y);
		                    MechElev(tt) = GetElevation(map, x, y);
				    if (MechZ(tt) < Elevation(map, x, y)) {
		                        MechZ(tt) = Elevation(map, x, y);
		                        MechFZ(tt) = MechZ(tt) * ZSCALE;
					mech_notify(tt, MECHALL, "You step higher as the ground beneath you is rebuilt!");
				    }
			}
                        count++;
		        elev = -1;
			}
                    }
    if (count)
        notify(player, tprintf("%d hexes 'walled'.", count));
    else
        notify(player, "No hexes 'walled'.");
}

void mech_BreakIce(MECH * mech, MAP * mech_map) {
    int  hitloc, hitGroup = 0;
    int isrear = 0, damage, iscritical = 0;
    if (MechTons(mech) <= 100) {
    mech_notify(mech, MECHALL, "You attempt to break the ice!");
    if (MechPilot(mech) == -1 ||
	MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3)) {
    } else {
      mech_notify(mech, MECHALL, "You plow into the ice and it fights back!!");
      MechLOSBroadcast(mech, "Plows into the ice!");
      if (MechSpeed(mech) < 0) hitGroup = BACK; else hitGroup = FRONT;
      hitloc = FindHitLocation(mech, hitGroup, &iscritical, &isrear, NULL);
      DamageMech(mech, mech, 0, -1, hitloc, 0, 0, (((100 - MechTons(mech))/10) * 2), -1, -1, 0, 0);
    }
  }
  mech_notify(mech, MECHALL, "You plow into the ice, turning it into slush");
  SetTerrain(mech_map, MechX(mech), MechY(mech), WATER);
}

