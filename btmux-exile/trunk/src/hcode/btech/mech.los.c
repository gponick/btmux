#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "p.map.obj.h"
#include "p.mech.sensor.h"
#include "p.mech.los.h"
#include "p.mech.utils.h"
#include "p.mech.ecm.h"
#include "mech.events.h"

/* 'nice' sensor stuff's in the mech.sensor.c ; nasty brute code
   lies here */

/* from here onwards.. black magic happens. Enter if you're sure of
   your peace of mind :-P */

/* -------------------------------------------------------------------- */

float ActualElevation(MAP * map, int x, int y, MECH * mech)
{

    if (!map)
	return 0.0;
    if (!mech)
	return (float) (Elevation(map, x, y) + 0.1);
    if (MechType(mech) == CLASS_MECH && !Fallen(mech))
	return (float) MechZ(mech) + 1.5;
    else if (IsDS(mech))
	return (float) MechZ(mech) + 2.5;
    if (MechCritStatus(mech) & DUG_IN && !(MechMove(mech) == MOVE_QUAD))
	return (float) MechZ(mech) + 0.1;
    return (float) MechZ(mech) + 0.5;
}

extern int *TraceLOS(MAP * map, int ax, int ay, int bx, int by,

    int *result);

/* from/mech: mech _mech_ seeing _target_ on map _map_, _ff_
   is the previous flag (or seeing _x_,_y_ if _target_ is NULL),
   hexRange is range in hexes */
int CalculateLOSFlag(MECH * mech, MECH * target, MAP * map, int x, int y,
    int ff, float hexRange, int forceecm)
{
    MECH *t;
    int IsAngel = ((ff & MECHLOSFLAG_ANGELECM) ? 1 : 0);
    int new_flag = (ff & (MECHLOSFLAG_SEEN)) + MECHLOSFLAG_SEEC2;
    int smoke_count = 0;
    int woods_count = 0;
    int water_count = 0;
    int build_count = 0;
    int height, i;
    int pos_x, pos_y;
    float pos_z, z_inc, end_z;
    int terrain;
    int dopartials = 0;
    int underwater, bothworlds, t_underwater, t_bothworlds, uwatercount = 0;
    int checkecm = 0, ecm = 0, mastercheckecm = (forceecm ? 1 : ((event_tick % TURN) % 3) ? 0 : 1);
    int *coords, coordcount;

#ifndef BT_PARTIAL
    float partial_z, p_z_inc;
#endif

    /* A Hex target off the map? Don't bother */
    if (!target && ((x < 0 || y < 0 || x >= map->map_width || y >= map->map_height)))
	return new_flag + MECHLOSFLAG_BLOCK;

    /* Outside max sensor range in the worst case? Don't bother. */
    if (hexRange > (((MechSpecials(mech) & AA_TECH) || (target &&
		    (MechSpecials(target) & AA_TECH))) ? 100 :
		    ((MechSpecials2(mech) & ADV_AA_TECH) || (target &&
		    (MechSpecials2(target) & ADV_AA_TECH))) ? 5000 : map->maxvis))
	return new_flag + MECHLOSFLAG_BLOCK;

    /* We start at (MechX(mech), MechY(mech)) and wind up at (x,y) */
    pos_x = MechX(mech);
    pos_y = MechY(mech);
    pos_z = ActualElevation(map, pos_x, pos_y, mech);
    end_z = ActualElevation(map, x, y, target);

/* Definition of 'both worlds': According to FASA, if a mech is half
   submerged, or a sub is surfaced, or any naval or hover is on top
   of the water, it can see in both the underwater and overwater 'worlds'.
   In other words, it'll never get a block from the water/air interface.
   Neither will anything get such a block against it. That's what the
   'both worlds' variables test for. */

    if (end_z > 10 && pos_z > 10)
	return new_flag;
    bothworlds = IsWater(MechRTerrain(mech)) &&	/* Can we be in both worlds? */
	(((MechType(mech) == CLASS_MECH) && (MechZ(mech) == -1) &&
	(!Fallen(mech))) || ((WaterBeast(mech)) && (MechZ(mech) == 0)) ||
	((MechMove(mech) == MOVE_HOVER) && (MechZ(mech) == 0)));
    underwater = InWater(mech) && (pos_z < 0.0);
    /* Ice hex targeting special case */ /* Moved up from immediately after bothworlds time-waste check */
    /* to properly parse ice hex special case Z to prevent default MECHLOSFLAG_BLOCK on endz. DJ 9/10/00 */
    if (!target && !underwater && GetRTerrain(map, x, y) == ICE)
        end_z = 0.1;
    if (target) {
	 /* What about him? Both worlds? Or flat out underwater? */
	t_bothworlds = IsWater(MechRTerrain(target)) &&
	    (((MechType(target) == CLASS_MECH) && (MechZ(target) == -1) &&
	     (!Fallen(mech))) || ((WaterBeast(target)) && (MechZ(target) == 0)) ||
	    ((MechMove(target) == MOVE_HOVER) && (MechZ(target) == 0)));

	t_underwater = InWater(target) && (end_z < 0.0);
    } else {
	t_bothworlds = 0;
	t_underwater = (end_z < 0.0);	/* Is the hex underwater? */
    }

    /* Worth our time to mess with figuring partial cover? */
    if (target && mech)
	dopartials = (MechType(target) == CLASS_MECH) && (!Fallen(target));

    /*Same hex is always LoS */
    if ((x == pos_x) && (y == pos_y))
	return new_flag;

    /* Special cases are out of the way, looks like we have to do actual work. */
    coords = TraceLOS(map, pos_x, pos_y, x, y, &coordcount);
    if (coordcount > 0) {
	z_inc = (float) (end_z - pos_z) / ((coordcount / 2));
    } else {
	z_inc = 0;		/* In theory, this should never happen. */
    }

#ifndef BT_PARTIAL
    partial_z = 0;
    p_z_inc = (float) 1 / ((coordcount / 2));
#endif

    if (coordcount > 0) {	/* not in same hex ; in same hex, you see always */
	if (mastercheckecm)
	  for (i = 0; i < map->first_free; i++)
            {
            if (!(t = FindObjectsData(map->mechsOnMap[i])) || !map)
                continue;
//            if (In_Character(mech->mynum) != In_Character(t->mynum))
//               continue;
            if ((MechStatus(t) & COMBAT_SAFE) || !(MechStatus(t) & STARTED))
               continue;
            if (MechMove(t) == MOVE_NONE)
               continue;
            if (MechType(t) == CLASS_MW)
               continue;
            if (MechStatus(t) & ECM_ACTIVE)
              if (FaMechRange(t, mech) <= (hexRange + ECMRange(t))) {
		checkecm = 1;
		break;
                }
	    checkecm = 0;
            }
	    
	for (i = 0; i < coordcount; i += 2) {
	    pos_z += z_inc;
#ifndef BT_PARTIAL
	    partial_z += p_z_inc;
#endif
	    if (coords[i] < 0 || coords[i] >= map->map_width ||
		coords[i + 1] < 0 || coords[i + 1] >= map->map_height)
		continue;
	    /* Should be possible to see into water.. perhaps. But not
	       on vislight */
	    terrain = GetRTerrain(map, coords[i], coords[i + 1]);
	    /* Find if LoS is ECM Affected - DJ */
	    if (checkecm && mastercheckecm) {
		if ((ecm = (ecm_affect_hex(mech, coords[i], coords[i + 1], pos_z, &IsAngel))) > 0) {
			new_flag |= MECHLOSFLAG_ECM;
			if (IsAngel)
				new_flag |= MECHLOSFLAG_ANGELECM;
			checkecm = 0;
			}
		if (ecm != 0)
		    checkecm = 0;
		} else if (IsAngel && !mastercheckecm) {
		    new_flag |= MECHLOSFLAG_ANGELECM;
		}
	    /* get the current height */
	    height = Elevation(map, coords[i], coords[i + 1]);

/* If you, persoanlly, are underwater, the only way you can see someone
   if if they are underwater or in both worlds AND your LoS passes thru
   nothing but water hexes AND your LoS doesn't go thru the sea floor */
	    if (underwater) {

/* LoS hits sea floor */
		if (!(IsWater(terrain)) || ((terrain != BRIDGE && terrain != DBRIDGE) &&
			height >= pos_z)) {
		    new_flag |= (MECHLOSFLAG_BLOCK);
		    return new_flag;
		}

/* LoS pops out of water, AND we're not tracing to half-submerged mech's head */
		if (!t_bothworlds && pos_z >= 0.0) {
		    new_flag |= (MECHLOSFLAG_BLOCK);
		    return new_flag;
		}

/* uwatercount = # hexes LoS travel UNDERWATER */
		if (pos_z < 0.0)
		    uwatercount++;
		water_count++;
	    } else {		/* Viewer is not underwater */
		/* keep track of how many wooded hexes we cross */
		if (pos_z < (height + 2)) {
		    switch (GetTerrain(map, coords[i], coords[i + 1])) {
		    case SMOKE:
		    case HSMOKE:
			if (i < (coordcount - 2))
			    smoke_count +=
				(terrain == SMOKE) ? 1 : 2;
			break;
		    case FIRE:
			new_flag |= MECHLOSFLAG_FIRE;
			break;
		    }
		    switch (terrain) {
		    case LIGHT_FOREST:
		    case HEAVY_FOREST:
			if (i < (coordcount - 2))
			    woods_count +=
				(terrain == LIGHT_FOREST) ? 1 : 2;
			break;
		    case BUILDING:
		    case WALL:
			if (i < (coordcount - 2))
			    build_count++;
			break;
		    case SNOW:
		    case HIGHWATER:
			water_count++;
			break;
		    case ICE:
			if (pos_z < 0.0) {
			    new_flag |= MECHLOSFLAG_BLOCK;
			    return new_flag;
			}
			water_count++;
			break;
		    case WATER: 
/* LoS goes INTO water and we're not tracing to a target in both worlds */
			if (!bothworlds && (pos_z < 0.0)) {
			    new_flag |= MECHLOSFLAG_BLOCK|MECHLOSFLAG_WBLOCK;
			    return new_flag;
			}

/* Hexes in LoS that are phsyically underwater */
			if (pos_z < 0.0)
			    uwatercount++;
			water_count++;
			break;
		    case MOUNTAINS:
			new_flag |= MECHLOSFLAG_MNTN;
			break;
		    }
		}
		/* make this the new 'current hex' */
		if (height >= pos_z && (terrain != BRIDGE && terrain != DBRIDGE)) {
		    new_flag |= MECHLOSFLAG_BLOCK;
		    return new_flag;
		}
#ifndef BT_PARTIAL
		else if (dopartials && height >= (pos_z - partial_z))
		    new_flag |= MECHLOSFLAG_PARTIAL;
#endif
	    }

	if (i == (coordcount - 8))
	    new_flag |= MECHLOSFLAG_SPREVHEX;
	}
    }
    /* Then, we check the hex before target hex */
#ifdef BT_PARTIAL
    if (coordcount >= 4)
	if (dopartials) {
	    if (MechZ(target) >= MechZ(mech) &&
		(Elevation(map, coords[coordcount - 4],
			coords[coordcount - 3]) == (MechZ(target) + 1)))
		new_flag |= MECHLOSFLAG_PARTIAL;
	    if (MechZ(target) == -1 && MechRTerrain(target) == WATER && !(MechZ(mech) <= -2 ||
	    (MechZ(mech) == -1 && Fallen(mech))))
		new_flag |= MECHLOSFLAG_PARTIAL;
	}
#endif

/*
if (!underwater && end_z < 0.0 && coordcount > 2)
   new_flag |= (MECHLOSFLAG_BLOCK|MECHLOSFLAG_WBLOCK);
if (underwater && end_z >= 0.0 && coordcount > 2)
   new_flag |= (MECHLOSFLAG_BLOCK|MECHLOSFLAG_WBLOCK);
*/
/* And now we look once more to make sure we aren't wasting our time */
/* Moved from above to assist WBLOCK code. Water should have blocked it by now. */
if (((underwater) && !(t_underwater || t_bothworlds)) ||
    ((t_underwater) && !(underwater || bothworlds))) {
    return new_flag + (MECHLOSFLAG_BLOCK);
    }

    /* Target lying low in the woods -> 1 set of extra woods between the
       fellow and foes ; this is _unofficial_ rule */
    /* And I don't like it. -Fitz */

/*  if (target)
   if (Fallen(target))
   if (MechType(target) == CLASS_MECH)
   if (MechElev(target) == MechZ(target))
   switch (MechRTerrain(target))
   {
   case LIGHT_FOREST:
   woods_count++;
   break;
   case HEAVY_FOREST:
   woods_count += 2;
   break;
   } */

    water_count = BOUNDED(0, water_count, MECHLOSMAX_WATER - 1);
    woods_count = BOUNDED(0, woods_count, MECHLOSMAX_WOOD - 1);
    new_flag += MECHLOSFLAG_WOOD * woods_count;
    new_flag += MECHLOSFLAG_WATER * water_count;
    new_flag += MECHLOSFLAG_BUILD * build_count;
    new_flag += MECHLOSFLAG_SMOKE * smoke_count;
/* Block EM after 2, Vis/IR after 6 */
    if (uwatercount > 2)
	new_flag |= MECHLOSFLAG_MNTN;
    if (uwatercount > 6)
	new_flag |= MECHLOSFLAG_FIRE;

    return new_flag;
}


int AddTerrainMod(MECH * mech, MECH * target, MAP * map, float hexRange)
{
    /* Possibly do a quickie check only */
    if (mech && target) {
	if (map->LOSinfo[mech->
		mapnumber][target->mapnumber] & MECHLOSFLAG_PARTIAL)
		MechStatus(target) |= PARTIAL_COVER;
	else
	    MechStatus(target) &= ~PARTIAL_COVER;
	return Sensor_ToHitBonus(mech, target,
	    map->LOSinfo[mech->mapnumber][target->mapnumber],
	    map->maplight, hexRange);
    }
    return 0;
}

int InLineOfSight_NB(MECH * mech, MECH * target, int x, int y,
    float hexRange)
{
    int i;

    i = InLineOfSight(mech, target, x, y, hexRange);
    if (i & MECHLOSFLAG_BLOCK)
	return 0;
    return i;
}

#define from mech->mapnumber
#define to   target->mapnumber

int InLineOfSight(MECH * mech, MECH * target, int x, int y, float hexRange)
{
    MAP *map;
    float x1, y1;
    int arc;
    int i;

    map = getMap(mech->mapindex);
    if (!map) {
	mech_notify(mech, MECHPILOT,
	    "You are on an invalid map! Map index reset!");
	SendError(tprintf("InLineOfSight:invalid map:Mech %d  Index %d",
		mech->mynum, mech->mapindex));
	mech->mapindex = -1;
	return 0;
    }
    if (x < 0 || y < 0 || y >= map->map_height || x >= map->map_width) {
	SendError(tprintf("x:%d y:%d out of bounds for #%d (LOS check)", x,
		y, mech ? mech->mynum : -1));
	return 0;
    }
    if (target) {
        /* stupid check added by ivan. Might break other muxes, but seems ok for frontiers. */
	if (mech->mapindex != target->mapindex) {
		SendError(tprintf("InLineOfSight:invalid maps:Mech %d  Index %d - Target %d  Index %d",
			mech->mynum, mech->mapindex, target->mynum, target->mapindex));
		return 0;
	}
	x1 = MechFX(target);
	y1 = MechFY(target);
    } else
	MapCoordToRealCoord(x, y, &x1, &y1);
    arc = InWeaponArc(mech, x1, y1);
    /* Possibly do a quickie check only */
    if (MechCritStatus(mech) & CLAIRVOYANT)
	return 1;
    if (mech && target) {
#ifndef ADVANCED_LOS
	i = map->LOSinfo[from][to];
	if (Sensor_CanSee(mech, target, &i, arc, hexRange, map->mapvis,
		map->maplight, map->cloudbase)) {
	    map->LOSinfo[from][to] |=
		(MECHLOSFLAG_SEEN | MECHLOSFLAG_SEESP | MECHLOSFLAG_SEESS);
	    return 1;
	} else {
	    map->LOSinfo[from][to] &=
		~(MECHLOSFLAG_SEEN | MECHLOSFLAG_SEESP |
		MECHLOSFLAG_SEESS);
	    return 0;
	}
#else
	if (map->LOSinfo[from][to] & (MECHLOSFLAG_SEESP |
		MECHLOSFLAG_SEESS)) return (map->LOSinfo[from][to] &
		(MECHLOSFLAG_SEESP | MECHLOSFLAG_SEESS |
		    MECHLOSFLAG_BLOCK));
#endif
	return 0;
    }
    i = CalculateLOSFlag(mech, NULL, map, x, y, 0, hexRange, 0);
    return Sensor_CanSee(mech, NULL, &i, arc, hexRange, map->mapvis,
	map->maplight, map->cloudbase);
}


void mech_losemit(dbref player, MECH * mech, char *buffer)
{
    cch(MECH_USUALSP);
    MechLOSBroadcast(mech, buffer);
    notify(player, "Broadcast done.");
}
