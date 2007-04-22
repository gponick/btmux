#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "map.h"
#include "event.h"
#include "btmacros.h"
#include "mech.events.h"
#include "p.mech.restrict.h"
#include "p.mech.utils.h"
#include "p.mech.startup.h"
#include "p.ds.bay.h"
#include "p.btechstats.h"
#include "p.mechrep.h"
#include "p.crit.h"
#include "p.mech.combat.h"
#include "p.template.h"
#include "p.econ.h"
#include "p.econ_cmds.h"
#include "p.mech.partnames.h"
#include "p.aero.bomb.h"
#include "p.autopilot.h"
#include "p.mech.los.h"
#include "p.eject.h"
#include "p.mech.consistency.h"
#include "mech.tech.h"
#include "p.mech.tech.do.h"
#include "p.mech.custom.h"
#include "p.mech.status.h"

extern dbref pilot_override;

static char *my2string(const char * old)
{
    static char new[64];
    strncpy(new, old, 63);
    new[63] = '\0';
    return new;
}

const char *mechtypenames[CLASS_LAST + 1] = {
    "mech", "tank", "VTOL", "vessel", "aerofighter", "DropShip"
};

const char *mechtypename(MECH * foo)
{
    return mechtypenames[(int) MechType(foo)];
}

int MNumber(MECH * mech, int low, int high)
{
    if ((event_tick / RANDOM_TICK) != MechLastRndU(mech)) {
	MechRnd(mech) = random();
	MechLastRndU(mech) = event_tick / RANDOM_TICK;
    }
    return (low + MechRnd(mech) % (high - low + 1));
}

char *MechIDS(MECH * mech, int islower)
{
    static char buf[3];

    if (mech) {
	buf[0] = MechID(mech)[0];
	buf[1] = MechID(mech)[1];
    } else {
	buf[0] = '*';
	buf[1] = '*';
    }
    buf[2] = 0;

    if (islower) {
	buf[0] = tolower(buf[0]);
	buf[1] = tolower(buf[1]);
    }
    return buf;
}


char *MyToUpper(char *string)
{
    if (*string)
	*string = toupper(*string);
    return string;
}

int CritsInLoc(MECH * mech, int index)
{
    if (MechType(mech) == CLASS_MECH)
	switch (index) {
	case HEAD:
	case RLEG:
	case LLEG:
	    return 6;
	case RARM:
	case LARM:
	    if (MechMove(mech) == MOVE_QUAD)
		return 6;
    } else if (MechType(mech) == CLASS_MW)
	return 2;
    return NUM_CRITICALS;
}

int SectHasBusyWeap(MECH * mech, int sect)
{
    int i = 0, count, critical[MAX_WEAPS_SECTION];
    unsigned char weaptype[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];

    count = FindWeapons(mech, sect, weaptype, weapdata, critical);
    for (i = 0; i < count; i++)
	if (WpnIsRecycling(mech, sect, critical[i]))
	    return 1;
    return 0;
}


MAP *ValidMap(dbref player, dbref map)
{
    char *str;
    MAP *maps;

    DOCHECKN(!Good_obj(map), "Index out of range!");
    str = silly_atr_get(map, A_XTYPE);
    DOCHECKN(!str || !*str, "That is not a valid map! (no XTYPE!)");
    DOCHECKN(strcmp("MAP", str), "That is not a valid map!");
    DOCHECKN(!(maps = getMap(map)), "The map has not been allocated!!");
    return maps;
}

dbref FindTargetDBREFFromMapNumber(MECH * mech, char *mapnum)
{
    int loop;
    MECH *tempMech;
    MAP *map;

    if (mech->mapindex == -1)
	return -1;
    map = getMap(mech->mapindex);
    if (!map) {
	SendError(tprintf("FTDBREFFMN:invalid map:Mech: %d  Index: %d",
		mech->mynum, mech->mapindex));
	mech->mapindex = -1;
	return -1;
    }
    for (loop = 0; loop < map->first_free; loop++)
	if (map->mechsOnMap[loop] != mech->mynum &&
	    map->mechsOnMap[loop] != -1) {
	    tempMech = (MECH *)
		FindObjectsData(map->mechsOnMap[loop]);
	    if (tempMech && !strncasecmp(MechID(tempMech), mapnum, 2))
		return tempMech->mynum;
	}
    return -1;
}

void FindComponents(float magnitude, int degrees, float *x, float *y)
{
    *x = magnitude * fcos((float) (TWOPIOVER360 * (degrees + 90)));
    *y = magnitude * fsin((float) (TWOPIOVER360 * (degrees + 90)));
    *x = -(*x);			/* because 90 is to the right */
    *y = -(*y);			/* because y increases downwards */
}


static int Leave_Hangar(MAP * map, MECH * mech)
{
    MECH *car = NULL;
    MECH *swarm = NULL;
    MAP *newmap;
    int mapob;
    mapobj *mapo;
    int i = (MechIsSwarmed(mech) > 0), land;

    /* For now, leaving leads to finding yourself on the new map
       at a predetermined position */
    mapob = mech->mapindex;
    if (MechCarrying(mech) > 0)
	car = getMech(MechCarrying(mech));
    DOCHECKMA0(!map, "Invalid Map pointer error! Contact a Wizard.");
    DOCHECKMA0(!map->cf, "The entrance is still filled with rubble!");
    DOCHECKMA0(!can_pass_lock(mech->mynum, map->mynum, A_LLEAVE), "The hangar is locked.");

    MechLOSBroadcast(mech, "has left the hangar.");
    mech_Rsetmapindex(GOD, (void *) mech, tprintf("%d", (int) map->mapobj[TYPE_LEAVE]->obj));
    newmap = getMap(map->mapobj[TYPE_LEAVE]->obj);
    if (car)
	mech_Rsetmapindex(GOD, (void *) car, tprintf("%d",
		(int) map->mapobj[TYPE_LEAVE]->obj));
    if (i)
	{
        if (!map || !mech)
            return 0;
        for (i = 0; i < map->first_free; i++)
            {
            if (!(swarm = FindObjectsData(map->mechsOnMap[i])))
                continue;
            if (MechType(swarm) == CLASS_BSUIT)
         	if (MechSwarmTarget(swarm) == mech->mynum)
			mech_Rsetmapindex(GOD, (void *) swarm, tprintf("%d",
			    (int) map->mapobj[TYPE_LEAVE]->obj));
	    }
	}
    map = getMap(mech->mapindex);
    if (mech->mapindex == mapob) {
	SendError(tprintf("#%d %s attempted to leave, but no target map?",
		mech->mynum, GetMechID(mech)));
	mech_notify(mech, MECHALL,
	    "Exit of this map is.. fubared. Please contact a wizard");
	return 0;
    }
    if (!(mapo = find_entrance_by_target(map, mapob))) {
	SendError(tprintf
	    ("#%d %s attempted to leave, but no target place was found? setting the mech at 0,0 at %d.",
		mech->mynum, GetMechID(mech), mech->mapindex));
	mech_notify(mech, MECHALL,
	    "Weird bug happened during leave. Please contact a wizard. ");
	return 1;
    }
    mech_notify(mech, MECHALL, tprintf("You have left %s.",
	    structure_name(mapo)));
    fix_pilotdamage(mech, MechPilot(mech));
    if (FlyingT(mech) && !Landed(mech))
	mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d %d", mapo->x, mapo->y, MIN(5, MechZ(mech)) + Elevation(newmap, mapo->x, mapo->y)));
    else
	mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d", mapo->x, mapo->y));
    land = (Landed(mech) ? 1 : 0);
    ContinueFlying(mech);
    if (land)
	MechStatus(mech) |= LANDED;
    if (car)
	MirrorPosition(mech, car);
    if (MechIsSwarmed(mech))
	{
        if (!map || !mech)
            return 0;
        for (i = 0; i < map->first_free; i++)
            {
            if (!(swarm = FindObjectsData(map->mechsOnMap[i])))
                continue;
            if (MechType(swarm) == CLASS_BSUIT)
                if (MechSwarmTarget(swarm) == mech->mynum)
			{
                        MirrorPosition(mech, swarm);
			loud_teleport(swarm->mynum, mech->mapindex);
			CalAutoMapindex(swarm);
			}
            }
	}
    MechLOSBroadcast(mech, tprintf("has left %s at %d,%d.",
	    structure_name(mapo), MechX(mech), MechY(mech)));
    loud_teleport(mech->mynum, mech->mapindex);
    if (car)
	loud_teleport(car->mynum, mech->mapindex);
    if (In_Character(mech->mynum) && Location(MechPilot(mech)) != mech->mynum) {
	mech_notify(mech, MECHALL, "%ch%cr%cf%ciINTRUDER ALERT! INTRUDER ALERT!%c");
	mech_notify(mech, MECHALL, "%ch%cr%cfAutomatic self-destruct sequence initiated...%c");
	mech_shutdown(GOD, (void *) mech, "");
    }
    CalAutoMapindex(mech);
    if (MechSpeed(mech) > MMaxSpeed(mech))
        MechSpeed(mech) = MMaxSpeed(mech);
    return 1;
}


void CheckEdgeOfMap(MECH * mech)
{
    int pinned = 0;
    int linked;
    MAP *map;

    map = getMap(mech->mapindex);

    if (!map) {
	mech_notify(mech, MECHPILOT,
	    "You are on an invalid map! Map index reset!");
	mech_shutdown(MechPilot(mech), (void *) mech, "");
	SendError(tprintf("CheckEdgeofMap:invalid map:Mech: %d  Index: %d",
		mech->mynum, mech->mapindex));
	mech->mapindex = -1;
	return;
    }
    linked = map_linked(mech->mapindex);
    /* Prevents you from going off the map */
    /* Eventually this could wrap and all that.. */
    if (MechX(mech) < 0) {
	if (linked) {
	    MechX(mech) += map->map_width;
	    pinned = -1;
	} else {
	    MechX(mech) = 0;
	    pinned = 4;
	}
    } else if (MechX(mech) >= map->map_width) {
	if (linked) {
	    MechX(mech) -= map->map_width;
	    pinned = -1;
	} else {
	    MechX(mech) = map->map_width - 1;
	    pinned = 2;
	}
    }
    if (MechY(mech) < 0) {
	if (linked) {
	    pinned = -1;
	    MechY(mech) += map->map_height;
	} else {
	    MechY(mech) = 0;
	    pinned = 1;
	}
    } else if (MechY(mech) >= map->map_height) {
	if (linked) {
	    pinned = -1;
	    MechY(mech) -= map->map_height;
	} else {
	    MechY(mech) = map->map_height - 1;
	    pinned = 3;
	}
    }
    if (pinned > 0) {
	/* This is a DS bay. First, we need to check if the bay's doors are
	   blocked, one way or another.
	 */
	if (map->onmap && IsMech(map->onmap)) {
	    if (Leave_DS(map, mech))
		return;
	} else if (map->flags & MAPFLAG_MAPO && map->mapobj[TYPE_LEAVE])
	    if (Leave_Hangar(map, mech))
		return;
    }
    if (pinned) {
	MapCoordToRealCoord(MechX(mech), MechY(mech), &MechFX(mech),
	    &MechFY(mech));
	if (pinned > 0) {
	    mech_notify(mech, MECHALL, "You cannot move off this map!");
	    if (Jumping(mech) && !is_aero(mech))
		LandMech(mech, 1);
	    MechCocoon(mech) = 0;
	    MechSpeed(mech) = 0.0;
	    if (!is_aero(mech))
	        MechDesiredSpeed(mech) = 0.0;
	    if (is_aero(mech)) {
		SetFacing(mech, pinned == 4 ? 90 : pinned == 2 ? 270 : pinned == 1 ? 180 : 0);
		MechDesiredFacing(mech) = MechFacing(mech);
		MechStartFX(mech) = 0.0;
		MechStartFY(mech) = 0.0;
		MechStartFZ(mech) = 0.0;
		if (!Landed(mech))
		    MaybeMove(mech);
	    }
	}
    }
}

int FindZBearing(float x0, float y0, float z0, float x1, float y1, float z1)
{
float hyp, opp, deg;

hyp = FindRange(x0, y0, z0, x1, y1, z1);
if (hyp <= 0.0)
    return 0;
opp = FindRange(0, 0, 0, 0, 0, fabsf(z1 - z0));
deg = asin(opp / hyp) * (180 / PI);
return ceilf(deg);
}

int FindBearing(float x0, float y0, float x1, float y1)
{
    float deltax, deltay;
    float temp, rads;
    int degrees;

    deltax = (x0 - x1);
    deltay = (y0 - y1);
    if (deltax == 0.0) {
	/* Quick handling inserted here */
	if (deltay > 0.0)
	    return 0;
	else
	    return 180;
    }
    temp = deltay / deltax;
    if (temp < 0.0)
	temp = -temp;
    rads = fatan(temp);
    degrees = (int) (rads * 10.0 / TWOPIOVER360);
    /* Round off degrees */
    degrees = (degrees + 5) / 10;
    if (deltax < 0.0 && deltay < 0.0)
	degrees += 180;
    else if (deltax < 0.0)
	degrees = 180 - degrees;
    else if (deltay < 0.0)
	degrees = 360 - degrees;
    return AcceptableDegree(degrees - 90);
}

int InWeaponArc(MECH * mech, float x, float y)
{
/* Old Code. All fucked up and uglee */
#if 0 
    int temp, at;
    int bearingToTarget;
    int res = NOARC;
    int ds = 0;

    bearingToTarget = FindBearing(MechFX(mech), MechFY(mech), x, y);
    temp = MechFacing(mech) - bearingToTarget;
    if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW ||
	MechType(mech) == CLASS_BSUIT || (ds = IsDS(mech))) {
	if (!ds) {
	    if (MechStatus(mech) & TORSO_RIGHT)
		temp += 60;
	    else if (MechStatus(mech) & TORSO_LEFT)
		temp -= 60;
	}
	at = abs(temp);
	if (at >= 300 || at <= 60)
	    return FORWARDARC;
	if (at >= 120 && at <= 240)
	    return REARARC;
	if (temp <= -240 || (temp >= 60 && temp <= 120))
	    return LSIDEARC;
	if (temp >= 240 || (temp <= -60 && temp >= -120))
	    return RSIDEARC;
	SendError(tprintf
	    ("NoArc: #%d: BearingToTarget:%d Facing:%d At:%d/%d",
		mech->mynum, bearingToTarget, MechFacing(mech), at, temp));
	return NOARC;
    }
    at = abs(temp);
    if (mudconf.reb_icetech) {
	if (at >= 300 || at <= 60)
		res += FORWARDARC;
	else if (at >= 120 && at <= 240)
		res += REARARC;
	else if (temp <= -240 || (temp >= 60 && temp <= 120))
		res += LSIDEARC;
	else if (temp >= 240 || (temp <= -60 && temp >= -120))
		res += RSIDEARC;
	if (!is_aero(mech) && GetSectInt(mech, TURRET)) {
		temp = (MechFacing(mech) + MechTurretFacing(mech)) - bearingToTarget;
		if (temp >= 360)
			temp -= 360;
		if (abs(temp) >= 330 || abs(temp) <= 30)
			res += TURRETARC;
		}
	} else {
    if (at >= 315 || at <= 45)
	res += FORWARDARC;
    else if (at >= 135 && at <= 225)
	res += REARARC;
    else if (temp <= -225 || (temp >= 45 && temp <= 135))
	res += LSIDEARC;
    else if ((temp <= -45 && temp >= -135) || temp >= 225)
	res += RSIDEARC;
    if (!is_aero(mech) && GetSectInt(mech, TURRET)) {
	temp =
	    (MechFacing(mech) + MechTurretFacing(mech)) - bearingToTarget;
	if (temp >= 360)
	    temp -= 360;
	if (abs(temp) >= 300 || abs(temp) <= 60)
	    res += TURRETARC;
	}
    }
    if (res == NOARC)
	SendError(tprintf
	    ("NoArc(veh): #%d: BearingToTarget:%d Facing:%d At:%d/%d",
		mech->mynum, bearingToTarget, MechFacing(mech), at, temp));

    return res;
#endif
    int relat;
    int bearingToTarget;
    int res = NOARC;

    bearingToTarget = FindBearing(MechFX(mech), MechFY(mech), x, y);
    relat = MechFacing(mech) - bearingToTarget;
    if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT) {
        if (MechStatus(mech) & TORSO_RIGHT)
            relat += 59;
        else if (MechStatus(mech) & TORSO_LEFT)
            relat -= 59;
    }
    relat = AcceptableDegree(relat);
    if (relat >= 300 || relat <= 60)
        res |= FORWARDARC;
    if (relat > 120 && relat < 240)
        res |= REARARC;
    if (relat >= 240 && relat < 300)
        res |= RSIDEARC;
    if (relat > 60 && relat <= 120)
        res |= LSIDEARC;
    if ((relat >= 0 && relat < 180) || relat >= 350)
	res |= LHALFARC;
    if ((relat > 180 && relat <= 360) || relat <= 10)
	res |= RHALFARC;

    if (MechHasTurret(mech)) {
        relat = AcceptableDegree((MechFacing(mech) + MechTurretFacing(mech)) - bearingToTarget);
        if (relat >= 330 || relat <= 30)
            res |= TURRETARC;
	}
    if (res == NOARC)
        SendError(tprintf("NoArc: #%d: BearingToTarget:%d Facing:%d", mech->mynum, bearingToTarget, MechFacing(mech)));
    return res; 
}

int FindBaseToHitByRange(int weapindx, float frange, int mode, int nomin, char targ)
{
    int range;

    if (MechWeapons[weapindx].special & PCOMBAT)
	range = (int) (frange * 10 + 0.95);
    else
	range = (int) (frange + 0.95);
    /* Beyond range */
    if (range > EGunRange(weapindx, mode))
	return 1000;

    /* V. Long range */
    if (range > GunStat(weapindx, mode, GUN_LONGRANGE)) {
	return 8;
	}
    /* Long range... */
    if (range > GunStat(weapindx, mode, GUN_MEDRANGE)) {
	if (targ == TARGCOMP_SHORT)
	    return 5;
	else if (targ == TARGCOMP_LONG)
	    return 3;
	else
	    return 4;
	}
    /* Medium range */
    if (range > GunStat(weapindx, mode, GUN_SHORTRANGE)) {
	return 2;
	}
    /* Short range */
    if ((range > GunStat(weapindx, mode, GUN_MINRANGE)) || nomin) {
	if (targ == TARGCOMP_SHORT)
	    return -1;
	else if (targ == TARGCOMP_LONG)
	    return 1;
	else
	    return 0;
	}
    /* If we are at range 0.0

     * Added 8/3/99 by Kipsta (to fix a 0.0 bug)
     */

    if (range == 0) { 
	if ((GunStat(weapindx, mode, GUN_MINRANGE)  == 0) || nomin)
	    return 0;
	else
	    return GunStat(weapindx, mode, GUN_MINRANGE) - range;
    }
    /* Less than or equal to minimum range */
    return GunStat(weapindx, mode, GUN_MINRANGE) - range + 1;
}

int FindBaseToHitByC3Range(int weapindx, float frange, int mode, char targ)
{
    int range;

    if (MechWeapons[weapindx].special & PCOMBAT)
	range = (int) (frange * 10 + 0.95);
    else
	range = (int) (frange + 0.95);
    /* Beyond range */
    if (range > GunRange(weapindx, mode))
	return 1000;
    /* No V. Long range in a C3 network */
    /* Long range... */
    if (range > GunStat(weapindx, mode, GUN_MEDRANGE))
	{
	if (targ == TARGCOMP_SHORT)
	    return 5;
	else if (targ == TARGCOMP_LONG)
	    return 3;
	else
	    return 4;
	}
    /* Medium range */
    if (range > GunStat(weapindx, mode, GUN_SHORTRANGE))
	return 2;

    /* Short range */
    if (targ == TARGCOMP_SHORT)
        return -1;
    else if (targ == TARGCOMP_LONG)
        return 1;
    else
        return 0;
    /* No Minimum Range calculation on C3 network here */
}

char *FindGunnerySkillName(MECH * mech, int weapindx)
{

#if NEW_STATS
    if (weapindx < 0)
	return NULL;
    if (MechType(mech) == CLASS_MW) {
	if (weapindx >= 0) {
	    if (!strcmp(MechWeapons[weapindx].name, "PC.Blade"))
		return "Blade";
	    if (!strcmp(MechWeapons[weapindx].name, "PC.Vibroblade"))
		return "Blade";
	return "Small_Arms";
	}
    }
    else if (IsArtillery(weapindx))
	return "Gunnery-Artillery";
    else if (IsSpooge(weapindx))
	return "Gunnery-Flamer";
    else if (IsMissile(weapindx))
	return "Gunnery-Missile";
    else if (IsAutocannon(weapindx))
	return "Gunnery-Autocannon";
    else if (IsEnergy(weapindx))
	return "Gunnery-Laser";
#else
    if (IsArtillery(weapindx))
	return "Gunnery-Artillery";
    switch (MechType(mech)) {
    case CLASS_BSUIT:
	return "Gunnery-BSuit";
    case CLASS_MECH:
	return "Gunnery-Battlemech";
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
	return "Gunnery-Conventional";
    case CLASS_VEH_VTOL:
    case CLASS_AERO:
	return "Gunnery-Aerospace";
    case CLASS_SPHEROID_DS:
    case CLASS_DS:
	return "Gunnery-Spacecraft";
    case CLASS_MW:
	if (weapindx >= 0) {
	    if (!strcmp(MechWeapons[weapindx].name, "PC.Blade"))
		return "Blade";
	    if (!strcmp(MechWeapons[weapindx].name, "PC.Vibroblade"))
		return "Blade";
	}
	return "Small_Arms";
    }
#endif
    return NULL;
}

char *FindPilotingSkillName(MECH * mech)
{
if (MechType(mech) == CLASS_MW && MechRTerrain(mech) == WATER)
    return "Swimming";
#if NEW_STATS
    switch (MechType(mech)) {
	case CLASS_MW:
	    return "Running";
	case CLASS_BSUIT:
	    return "Piloting-Bsuit";
        case CLASS_VEH_NAVAL:
	    return "Piloting-Naval";
	case CLASS_DS:
	case CLASS_SPHEROID_DS:
	case CLASS_AERO:
	    return "Piloting-Spacecraft";
	case CLASS_VEH_VTOL:
	    return "Piloting-Aerospace";
    }
    switch (MechMove(mech)) {
	case MOVE_BIPED:
	    return "Piloting-Biped";
	case MOVE_QUAD:
	    return "Piloting-Quad";
	case MOVE_TRACK:
	    return "Piloting-Tracked";
	case MOVE_HOVER:
	    return "Piloting-Hover";
	case MOVE_WHEEL:
	    return "Piloting-Wheeled";
    }
#else
    switch (MechType(mech)) {
    case CLASS_MW:
	return "Running";
    case CLASS_BSUIT:
	return "Piloting-BSuit";
    case CLASS_MECH:
	return "Piloting-Battlemech";
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
	return "Drive";
    case CLASS_VEH_VTOL:
	return "Piloting-Aerospace";
    case CLASS_SPHEROID_DS:
    case CLASS_AERO:
    case CLASS_DS:
	return "Piloting-Spacecraft";
    }
#endif
    return NULL;
}

#define MECHSKILL_PILOTING  0
#define MECHSKILL_GUNNERY   1
#define MECHSKILL_SPOTTING  2
#define MECHSKILL_ARTILLERY 3
#define NUM_MECHSKILLS      4
#define GENERIC_FIND_MECHSKILL(num,n) \
if (db[mech->mynum].flags & QUIET) \
{ str = silly_atr_get(mech->mynum, A_MECHSKILLS); \
if (*str) if (sscanf (str, "%d %d %d %d", &i[0], &i[1], &i[2], &i[3]) > num) \
return i[num] - n; } \
else if (MechAuto(mech) > 0) \
{ str = silly_atr_get(MechAuto(mech), A_MECHSKILLS); \
if (*str) if (sscanf (str, "%d %d %d %d", &i[0], &i[1], &i[2], &i[3]) > num) \
return i[num] - n; }

int FindPilotPiloting(MECH * mech)
{
    char *str;
    int i[NUM_MECHSKILLS];

    GENERIC_FIND_MECHSKILL(MECHSKILL_PILOTING, 0);
    if (RGotPilot(mech))
	if ((str = FindPilotingSkillName(mech)))
	    return char_getskilltarget(MechPilot(mech), str, 0);
    return DEFAULT_PILOTING;
}

int FindSPilotPiloting(MECH * mech)
{
    return FindPilotPiloting(mech) + ((MechMove(mech) ==
	MOVE_QUAD && (!(MechCritStatus(mech) & LEG_DESTROYED))) ? -2 : 0) +
	(MechSpecials2(mech) & (TORSOCOCKPIT_TECH|SMALLCOCKPIT_TECH) ? 1 : 0) +
	(MechSpecials(mech) & HARDA_TECH ? 1 : 0) + (MechStatus2(mech) & SPRINTING ? 2 : 0) +
	(MechRTerrain(mech) == DESERT ? 1 : 0);
}

int FindPilotSpotting(MECH * mech)
{
    char *str;
    int i[NUM_MECHSKILLS];

    GENERIC_FIND_MECHSKILL(MECHSKILL_SPOTTING, 0);
    if (RGotPilot(mech))
	return (char_getskilltarget(MechPilot(mech), "Gunnery-Spotting",
		0));
    return DEFAULT_SPOTTING;
}

int FindPilotArtyGun(MECH * mech)
{
    char *str;
    int i[NUM_MECHSKILLS];

    GENERIC_FIND_MECHSKILL(MECHSKILL_ARTILLERY, 0);
    if (RGotGPilot(mech))
	return (char_getskilltarget(GunPilot(mech), "Gunnery-Artillery",
		0));
    return DEFAULT_ARTILLERY;
}

int FindAverageGunnery(MECH * mech)
{
int runtot = 0;
int i;

if (!mech)
    return 12;

for (i = 0; i < 5; i++) {
   runtot += FindPilotGunnery(mech, (i == 0 ? 0 : i == 1 ? 4 : i == 2 ? 5 : i == 3 ? 6 : i == 4 ? 103 : 0));
   }
return (runtot / 5);
}

int FindPilotGunnery(MECH * mech, int weapindx)
{
    char *str;
    int i[NUM_MECHSKILLS];

    GENERIC_FIND_MECHSKILL(MECHSKILL_GUNNERY, 0);
    if (RGotGPilot(mech))
	if ((str = FindGunnerySkillName(mech, weapindx)))
	    return char_getskilltarget(GunPilot(mech), str, 0);
    return DEFAULT_GUNNERY;
}

char *FindTechSkillName(MECH * mech)
{
    switch (MechType(mech)) {
    case CLASS_MECH:
	return "Technician-Battlemech";
    case CLASS_BSUIT:
    	return "Technician-BSuit";
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
	return "Technician-Mechanic";
    case CLASS_AERO:
    case CLASS_VEH_VTOL:
    case CLASS_SPHEROID_DS:
    case CLASS_DS:
	return "Technician-Aerospace";
#if 0				/* Used to be DS tech */
	return (char_getskilltarget(player, "Technician-Spacecraft", 0));
#endif
    }
    return NULL;
}

int FindTechSkill(dbref player, MECH * mech)
{
    char *skname;

    if ((skname = FindTechSkillName(mech)))
	return (char_getskilltarget(player, skname, 0));
    return 18;
}

int MadePilotSkillRoll(MECH * mech, int mods)
{
    int roll, roll_needed, noxp = 0;

    if (Fallen(mech))
	return 1;
    if (Uncon(mech) || !Started(mech) || Blinded(mech) || MechAutofall(mech))
	return 0;

    if (mods <= -1000) {
	noxp = 1;
	mods += 1000;
	}
    roll = Roll();
    roll_needed =
	FindSPilotPiloting(mech) + mods + MechPilotSkillBase(mech);
    if (In_Character(mech->mynum) && Location(MechPilot(mech)) != mech->mynum)
	roll_needed += 5;
    mech_notify(mech, MECHPILOT, "You make a piloting skill roll!");
    mech_notify(mech, MECHPILOT,
	tprintf("Modified Pilot Skill: BTH %d\tRoll: %d", roll_needed,
	    roll));
    if (roll >= roll_needed) {
	if (!noxp && roll_needed > 2)
	    AccumulatePilXP(MechPilot(mech), mech, BOUNDED(1,
		    roll_needed - 7, MAX(2, 1 + mods)));
	return 1;
    }
    return 0;
}

int MoFMadePilotSkillRoll(MECH * mech, int mods)
{
/* 0 == success, positive is MoS, negative is MoF */
    int roll, roll_needed, noxp = 0;

    if (Fallen(mech))
        return 1;
    if (Uncon(mech) || !Started(mech) || Blinded(mech) || MechAutofall(mech))
        return 0;

    if (mods <= -1000) { noxp = 1; mods += 1000;}
    roll = Roll();
    roll_needed =
        FindSPilotPiloting(mech) + mods + MechPilotSkillBase(mech);
    if (In_Character(mech->mynum) && Location(MechPilot(mech)) != mech->mynum)
        roll_needed += 5;
    mech_notify(mech, MECHPILOT, "You make a piloting skill roll!");
    mech_notify(mech, MECHPILOT,
        tprintf("Modified Pilot Skill: BTH %d\tRoll: %d", roll_needed,
            roll));
    if (roll >= roll_needed) {
        if (!noxp && roll_needed > 2)
            AccumulatePilXP(MechPilot(mech), mech, BOUNDED(1,
                    roll_needed - 7, MAX(2, 1 + mods)));
    }
    return roll - roll_needed;
}

void FindXY(float x0, float y0, int bearing, float range, float *x1,
    float *y1)
{
    float xscale, correction;

    correction = (float) (bearing % 60) / 60.0;
    if (correction > 0.5)
	correction = 1.0 - correction;
    correction = -correction * 2.0;	/* 0 - 1 correction */
    xscale = (1.0 + XSCALE * correction) * SCALEMAP;
    *y1 =
	y0 - cos((float) bearing * 6.283185307 / 360.0) * range * SCALEMAP;
    *x1 = x0 + sin((float) bearing * 6.283185307 / 360.0) * range * xscale;
}

float FindRange(float x0, float y0, float z0, float x1, float y1, float z1)
{				/* range in hexes */
    float xscale;
    float XYrange;
    float Zrange;

    xscale = 1.0 / SCALEMAP;
    xscale = xscale * xscale;
    XYrange =
	sqrt(xscale * (x0 - x1) * (x0 - x1) + YSCALE2 * (y0 - y1) * (y0 -
	    y1));
    Zrange = (z0 - z1) / SCALEMAP;
    return sqrt(XYrange * XYrange + Zrange * Zrange);
}

float FindXYRange(float x0, float y0, float x1, float y1)
{				/* range in hexes */
    float xscale;
    float XYrange;

    xscale = 1.0 / SCALEMAP;
    xscale = xscale * xscale;
    XYrange =
	sqrt(xscale * (x0 - x1) * (x0 - x1) + YSCALE2 * (y0 - y1) * (y0 -
	    y1));
    return XYrange;
}

float FindHexRange(float x0, float y0, float x1, float y1)
{
    return FindXYRange(x0, y0, x1, y1);
}

// float FindHexRange(float x0, float y0, float x1, float y1)
// {                            /* range in hexes */
//     float bearing, xscale;
//
//     bearing = (float) (FindBearing(x0, y0, x1, y1) % 60) / 60.0;
//     if (bearing > 0.5)
//      bearing = 1.0 - bearing;
//     bearing = bearing * 2.0; /* 0 - 1 correction */
//     xscale = (1.0 + XSCALE * bearing) / SCALEMAP;
//     xscale = xscale * xscale;
//     return sqrt(xscale * (x0 - x1) * (x0 - x1) + YSCALE2 * (y0 - y1) * (y0 - y1));
// }

/* CONVERSION ROUTINES courtesy Mike :) */
#if 0
void RealCoordToMapCoord(short *hex_x, short *hex_y, float cart_x,
    float cart_y)
{
    float x, y, alpha = 0.288675134, root_3 = 1.732050808;
    int x_count, y_count, x_offset, y_offset;

    /* first scale cart coords to 1 */
    cart_x = cart_x / SCALEMAP;
    cart_y = cart_y / SCALEMAP;

    if (cart_x < alpha) {
	x_count = -2;
	x = cart_x + 5 * alpha;
    } else {
	x_count = (int) ((cart_x - alpha) / root_3);
	x = cart_x - alpha - x_count * root_3;
    }

    y_count = (int) floor(cart_y);
    y = cart_y - y_count;

    if ((x >= 0.0) && (x < (2.0 * alpha))) {
	x_offset = 0;
	y_offset = 0;
    } else if ((x >= (3.0 * alpha)) && (x < (5.0 * alpha))) {
	if ((y >= 0.0) && (y < 0.5)) {
	    x_offset = 1;
	    y_offset = 0;
	} else {
	    x_offset = 1;
	    y_offset = 1;
	}
    } else if ((x >= 2.0 * alpha) && (x < (3.0 * alpha))) {
	if ((y >= 0.0) && (y < 0.5)) {
	    if ((2 * alpha * y) <= (x - 2.0 * alpha)) {
		x_offset = 1;
		y_offset = 0;
	    } else {
		x_offset = 0;
		y_offset = 0;
	    }
	} else if ((2 * alpha * (1.0 - y)) > (x - 2.0 * alpha)) {
	    x_offset = 0;
	    y_offset = 0;
	} else {
	    x_offset = 1;
	    y_offset = 1;
	}
    } else if ((y >= 0.0) && (y < 0.5)) {
	if ((2 * alpha * y) <= (11.0 * alpha - x - 5.0 * alpha)) {
	    x_offset = 1;
	    y_offset = 0;
	} else {
	    x_offset = 2;
	    y_offset = 0;
	}
    } else if ((2 * alpha * (y - 0.5)) <= (x - 5.0 * alpha)) {
	x_offset = 2;
	y_offset = 0;
    } else {
	x_offset = 1;
	y_offset = 1;
    }

    *hex_x = x_count * 2 + x_offset;
    *hex_y = y_count + y_offset;

}

/* if hex_x is even, new_y = (hex_y + 1) * 0.5,
   new_x = hex_x * 3 alpha + 2 alpha.
   if hex_x is odd, then new_x = (hex_x - 1) * 3 alpha + 5 alpha,
   new_y = hex_y.
 */

void MapCoordToRealCoord(int hex_x, int hex_y, float *cart_x,
    float *cart_y)
{
    float alpha = 0.288675134;

    if (hex_x + 2 - (hex_x + 2) / 2 * 2) {	/* hex_x is odd */
	*cart_x = (float) (hex_x - 1.0) * 3.0 * alpha + 5.0 * alpha;
	*cart_y = (float) (hex_y);
    } else {
	*cart_x = (float) (hex_x) * 3.0 * alpha + 2.0 * alpha;
	*cart_y = (float) hex_y + 0.5;
    }
    *cart_x = *cart_x * SCALEMAP;
    *cart_y = *cart_y * SCALEMAP;
}
#endif

/* Picture blatantly ripped from the MUSH code by Dizzledorf and co. If only
   I had found it _before_ reverse-engineering the code :)
     - Focus, July 2002.
 */

/* Convert floating-point cartesian coordinates into hex coordinates.

   To do this, split the hex map into a repeatable region, which itself has
   4 distinct regions, each part of a different hex. (See picture.) The hex
   is normalized so that it is 1 unit high, and so is the repeatable region.
   It works out that the repeatable region is exactly sqrt(3) wide, and can
   be split up in six portions of each 1/6th sqrt(3), called 'alpha'. 
   Section I is 2 alpha wide at the top and bottom, and 3 alpha in the
   middle. Sections II and III are reversed, being 3 alpha at the top and
   bottom of the region, and 2 alpha in the middle. Section IV is 1 alpha in
   the middle and 0 at the top and bottom. The whole region encompasses
   exactly two x-columns and one y-row. All calculations are now done in
   'real' scale, to avoid rounding errors (isn't floating point arithmatic
   fun ?)

   Alpha also returns in the angle of the hexsides, that being 2*alpha
   (flipped or rotated when necessary) making the calculations look
   confusing. ANGLE_ALPHA is alpha (unscaled) for use in angle calculations.

         ________________________
        |        \              /|
        |         \    III     / |
        |          \          /  |
        |           \________/ IV|
        |    I      /        \   |
        |          /   II     \  |
        |         /            \ |
        |________/______________\|


   */

/* Doubles for added accuracy; most calculations are doubles internally
   anyway, so we suffer little to no performance hit. */

#define ALPHA 93.097730906827152        /* sqrt(3) * SCALEMAP */
#define ROOT3 558.58638544096289        /* ROOT3 / 6 */
#define ANGLE_ALPHA 0.28867513459481287 /* sqrt(3) / 6 */
#define FULL_Y (1 * SCALEMAP)
#define HALF_Y (0.5 * FULL_Y)

void RealCoordToMapCoord(short *hex_x, short *hex_y, float cart_x,
    float cart_y)
{
    float x, y;
    int x_count, y_count;

    if (cart_x < ALPHA) {
        /* Special case: we are in section IV of x-column 0 or off the map */
        *hex_x = cart_x <= ALPHA ? -1 : 0;
        *hex_y = floor(cart_y / SCALEMAP);
        return;
    }

    /* 'shift' the map to the left so the repeatable box starts at 0 */
    cart_x -= ALPHA;

    /* Figure out the x-coordinate of the 'repeatable box' we're in. */
    x_count = cart_x / ROOT3;
    /* And the offset inside the box, from the left edge. */
    x = cart_x - x_count * ROOT3;

    /* The repbox holds two x-columns, we want the real X coordinate. */
    x_count *= 2;

    /* Do the same for the y-coordinate; this is easy */
    y_count = floor(cart_y / FULL_Y);
    y = cart_y - y_count * FULL_Y;

    if (x < 2 * ALPHA) {

        /* Clean in area I. Nothing to do */

    } else if (x >= 3 * ALPHA && x < 5 * ALPHA) {
        /* Clean in either area II or III. Up x one, and y if in the lower
           half of the box. */
        x_count++;
        if (y >= HALF_Y)
            /* Area II */
            y_count++;

    } else if (x >= 2 * ALPHA && x < 3 * ALPHA) {
        /* Any of areas I, II and III. */
        if (y >= HALF_Y) {
            /* Area I or II */
            if (2 * ANGLE_ALPHA * (FULL_Y - y) <= x - 2 * ALPHA) {
                /* Area II, up both */
                x_count++;
                y_count++;
            }
        } else {
            /* Area I or III */
            if (2 * ANGLE_ALPHA * y <= x - 2 * ALPHA)
                /* Area III, up only x */
                x_count++;
        }
    } else if (y >= HALF_Y) {
        /* Area II or IV. Up x at least one, maybe two, and y maybe one. */
        x_count++;
        if (2 * ANGLE_ALPHA * (y - HALF_Y) > (x - 5.0 * ALPHA))
            /* Area II */
            y_count++;
        else
            /* Area IV */
            x_count++;
    } else {
        /* Area III or IV, up x at least one, maybe two */
        x_count++;
        if (2 * ANGLE_ALPHA * y > ROOT3 - x)
            /* Area IV */
            x_count++;
    }

    *hex_x = x_count;
    *hex_y = y_count;
}

/* 
   If hex_x is odd, it falls smack in the middle of area III, right at the
   top edge of the repeatable box. Subtract one and multiply by one half
   sqrt(3) (or 3 alpha) to get the repeatable box coordinate, then add 5
   alpha for offset inside the box. The y coordinate is not modified.

   If hex_x is even, just multiply by 3 alpha to get the box coordinate, and
   add 2 alpha for offset inside the box. The center is in the middle of the
   box, so y is increased by half the box height.

 */

void MapCoordToRealCoord(int hex_x, int hex_y, float *cart_x,
    float *cart_y)
{

    if (hex_x % 2) {
        *cart_x = (hex_x - 1.0) * 3.0 * ALPHA + 5.0 * ALPHA;
        *cart_y = hex_y * FULL_Y;
    } else {
        *cart_x = hex_x * 3.0 * ALPHA + 2.0 * ALPHA;
        *cart_y = hex_y * FULL_Y + HALF_Y;
    }
}

/*
   Sketch a 'mech on a Navigate map. Done here since it fiddles directly
   with cartesian coords.

   Navigate is 9 rows high, and a hex is exactly 1*SCALEMAP high, so each
   row is FULL_Y/9 cartesian y-coords high.

   Navigate is 21 hexes wide, at its widest point. This corresponds to the
   hex width, which is 4 * ALPHA, so each column is 4*ALPHA/21 cartesian
   x-coords wide.

   The actual navigate map starts two rows from the top and four columns
   from the left.

 */


#define NAV_ROW_HEIGHT (FULL_Y / 9.0)
#define NAV_COLUMN_WIDTH (4 * ALPHA / 21.0)
#define NAV_Y_OFFSET 2
#define NAV_X_OFFSET 4
#define NAV_MAX_HEIGHT 2+9+2
#define NAV_MAX_WIDTH 4+21+2

void navigate_sketch_mechs(MECH * mech, MAP * map, int x, int y,
    char buff[NAVIGATE_LINES][MBUF_SIZE])
{
    float corner_fx, corner_fy, fx, fy;
    int i, row, column;
    MECH *other;
    char mechtype;

    MapCoordToRealCoord(x, y, &corner_fx, &corner_fy);
    corner_fx -= 2 * ALPHA;
    corner_fy -= HALF_Y;

    for (i = 0; i < map->first_free; i++) {
        if (map->mechsOnMap[i] < 0)
            continue;
        if (!(other = FindObjectsData(map->mechsOnMap[i])))
            continue;
        if (other == mech)
            continue;
        if (MechX(other) != x || MechY(other) != y)
            continue;
        if (!InLineOfSight(mech, other, x, y, 0.5))
            continue;

        fx = MechFX(other) - corner_fx;
        column = fx / NAV_COLUMN_WIDTH + NAV_X_OFFSET;

        fy = MechFY(other) - corner_fy;
        row = fy / NAV_ROW_HEIGHT + NAV_Y_OFFSET;

        if (column < 0 || column > NAV_MAX_WIDTH ||
            row < 0 || row > NAV_MAX_HEIGHT)
            continue;

        buff[row][column] = MechSeemsFriend(mech, other) ? 'x' : 'X';
    }

    /* Draw 'mech last so we always see it. */

    fx = MechFX(mech) - corner_fx;
    column = fx / NAV_COLUMN_WIDTH + NAV_X_OFFSET;

    fy = MechFY(mech) - corner_fy;
    row = fy / NAV_ROW_HEIGHT + NAV_Y_OFFSET;

    if (column < 0 || column > NAV_MAX_WIDTH ||
        row < 0 || row > NAV_MAX_HEIGHT)
        return;

    buff[row][column] = '*';
}

int FindTargetXY(MECH * mech, float *x, float *y, float *z)
{
    MECH *tempMech;

    if (MechTarget(mech) != -1) {
	tempMech = getMech(MechTarget(mech));
	if (tempMech) {
	    *x = MechFX(tempMech);
	    *y = MechFY(tempMech);
	    *z = MechFZ(tempMech);
	    return 1;
	}
    } else if (MechTargX(mech) != -1 && MechTargY(mech) != -1) {
	MapCoordToRealCoord(MechTargX(mech), MechTargY(mech), x, y);
	*z = (float) ZSCALE *(MechTargZ(mech));

	return 1;
    }
    return 0;
}

int global_silence = 0;

#define UGLYTEST \
	  if (num_crits) \
	    { \
	      if (num_crits != (i = GetWeaponCrits (mech, lastweap))) \
		{ \
		  if (whine && !global_silence) \
		    SendError (tprintf ("Error in the numcriticals for weapon on #%d! (Should be: %d, is: %d)", mech->mynum, i, num_crits)); \
		  return -1; \
		} \
	      num_crits = 0; \
	    }


/* ASSERTION: Weapons must be located next to each other in criticals */

/* This is a hacked function. Sorry. */

int FindWeapons_Advanced(MECH * mech, int index, unsigned char *weaparray,
    unsigned char *weapdataarray, int *critical, int whine)
{
    int loop;
    int weapcount = 0;
    int temp, data, lastweap = -1;
    int num_crits = 0, i;

    for (loop = 0; loop < MAX_WEAPS_SECTION; loop++) {
	temp = GetPartType(mech, index, loop);
	data = GetPartData(mech, index, loop);
	if (IsWeapon(temp)) {
	    temp = Weapon2I(temp);
	    if (weapcount == 0) {
		lastweap = temp;
		weapdataarray[weapcount] = data;
		weaparray[weapcount] = temp;
		critical[weapcount] = loop;
		weapcount++;
		num_crits = 1;
		continue;
	    }
	    if (!num_crits || temp != lastweap ||
		(num_crits == GetWeaponCrits(mech, temp))) {
		UGLYTEST;
		weaparray[weapcount] = temp;
		weapdataarray[weapcount] = data;
		critical[weapcount] = loop;
		lastweap = temp;
		num_crits = 1;
		weapcount++;
	    } else
		num_crits++;
	} else
	    UGLYTEST;
    }
    UGLYTEST;
    return (weapcount);
}

int FindAmmunition(MECH * mech, unsigned char *weaparray,
    unsigned short *ammoarray, unsigned short *ammomaxarray,
    unsigned int *modearray)
{
    int loop;
    int weapcount = 0;
    int temp, data, mode;
    int index, i, j, duplicate;

    for (index = 0; index < NUM_SECTIONS; index++)
	for (loop = 0; loop < MAX_WEAPS_SECTION; loop++) {
	    temp = GetPartType(mech, index, loop);
	    if (IsAmmo(temp)) {
		data = GetPartData(mech, index, loop);
		mode = (GetPartMode(mech, index, loop) & PAMMO_MODES);
		temp = Ammo2Weapon(temp);
		duplicate = 0;
		for (i = 0; i < weapcount; i++) {
		    if (temp == weaparray[i] && mode == modearray[i]) {
			if (!(PartIsNonfunctional(mech, index, loop)))
			    ammoarray[i] += data;
			ammomaxarray[i] += FullAmmo(mech, index, loop);
			duplicate = 1;
		    }
		}
		if (!duplicate) {
		    weaparray[weapcount] = temp;
		    if (!(PartIsNonfunctional(mech, index, loop)))
			ammoarray[weapcount] = data;
		    else
			ammoarray[weapcount] = 0;
		    ammomaxarray[weapcount] = FullAmmo(mech, index, loop);
		    modearray[weapcount] = mode;
		    weapcount++;
		}
	    }
	}
    /* Then, prune entries with 0 ammo left */
    for (i = 0; i < weapcount; i++)
	if (!ammoarray[i]) {
	    for (j = i + 1; j < weapcount; j++) {
		weaparray[j - 1] = weaparray[j];
		ammoarray[j - 1] = ammoarray[j];
		ammomaxarray[j - 1] = ammomaxarray[j];
		modearray[j - 1] = modearray[j];
	    }
	    i--;
	    weapcount--;
	}
    return (weapcount);
}

int FindLegHeatSinks(MECH * mech)
{
    int loop;
    int heatsinks = 0;

    for (loop = 0; loop < NUM_CRITICALS; loop++) {
	if (GetPartType(mech, LLEG, loop) == I2Special((HEAT_SINK)) &&
	    !PartIsNonfunctional(mech, LLEG, loop))
	    heatsinks++;
	if (GetPartType(mech, RLEG, loop) == I2Special((HEAT_SINK)) &&
	    !PartIsNonfunctional(mech, RLEG, loop))
	    heatsinks++;
	/*
	 * Added by Kipsta on 8/5/99
	 * Quads can get 'arm' HS in the water too
	 */

	if (MechMove(mech) == MOVE_QUAD) {
	    if (GetPartType(mech, LARM, loop) == I2Special((HEAT_SINK)) &&
		!PartIsNonfunctional(mech, LLEG, loop))
		heatsinks++;
	    if (GetPartType(mech, RARM, loop) == I2Special((HEAT_SINK)) &&
		!PartIsNonfunctional(mech, RLEG, loop))
		heatsinks++;
	}
    }
    return (heatsinks);
}
/* Cloned above function for Mech Leg JJ fixes and some snow tweaks. - DJ */

int FindLegJumpJets(MECH * mech)
{
    int loop;
    int jumpjets = 0;

    for (loop = 0; loop < NUM_CRITICALS; loop++) {
        if (GetPartType(mech, LLEG, loop) == I2Special((JUMP_JET)) &&
            !PartIsNonfunctional(mech, LLEG, loop))
            jumpjets++;
        if (GetPartType(mech, RLEG, loop) == I2Special((JUMP_JET)) &&
            !PartIsNonfunctional(mech, RLEG, loop))
            jumpjets++;
        if (MechMove(mech) == MOVE_QUAD) {
            if (GetPartType(mech, LARM, loop) == I2Special((JUMP_JET)) &&
                !PartIsNonfunctional(mech, LLEG, loop))
                jumpjets++;
            if (GetPartType(mech, RARM, loop) == I2Special((JUMP_JET)) &&
                !PartIsNonfunctional(mech, RLEG, loop))
                jumpjets++;
        }
    }
    return (jumpjets);
}
/* Added for tic support. */

/* returns the weapon index- -1 for not found, -2 for destroyed, -3, -4 */

/* for reloading/recycling */
int FindWeaponNumberOnMech_Advanced(MECH * mech, int number, int *section, int *crit, int sight)
{
    int loop;
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int running_sum = 0;
    int num_weaps;
    int index;

    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (num_weaps <= 0)
	    continue;
	if (number < running_sum + num_weaps) {
	    /* we found it... */
	    index = number - running_sum;
	    if (WeaponIsNonfunctional(mech, loop, WeaponFirstCrit(mech, loop, critical[index]),
		GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loop,
		critical[index])))) > 0) {
		*section = loop;
		*crit = critical[index];
		return TIC_NUM_DESTROYED;
	    } else if (weapdata[index] > 0 && !sight) {
		*section = loop;
		*crit = critical[index];
		return (MechWeapons[weaparray[index]].type == TBEAM) ? TIC_NUM_RECYCLING : TIC_NUM_RELOADING;
	    } else if ((GetPartMode(mech, loop, WeaponFirstCrit(mech, loop, critical[index])) & JAMMED_MODE) && !sight) {
		*section = loop;
		*crit = critical[index];
		return TIC_NUM_JAMMED;
	    } else {
		if (MechSections(mech)[loop].recycle && (MechType(mech) == CLASS_MECH ||
			MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_BSUIT) && !sight) {
		    *section = loop;
		    *crit = critical[index];
		    /* just did a physical attack */
		    return TIC_NUM_PHYSICAL;
		}
		/* The recylce data for the weapon is clear- it is ready to fire! */
		*section = loop;
		*crit = critical[index];
		return weaparray[index];
	    }
	} else
	    running_sum += num_weaps;
    }
    return -1;
}

int FindWeaponNumberOnMech(MECH * mech, int number, int *section, int *crit)
{
    return FindWeaponNumberOnMech_Advanced(mech, number, section, crit, 0);
}

int FindWeaponFromIndex(MECH * mech, int weapindx, int *section, int *crit)
{
    int loop;
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int num_weaps;
    int index;

    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
	for (index = 0; index < num_weaps; index++)
	    if (weaparray[index] == weapindx) {
		*section = loop;
		*crit = critical[index];
		if (!PartIsNonfunctional(mech, loop, index) &&
		    !WpnIsRecycling(mech, loop, index))
		    return 1;
		/* Return if not Recycling/Destroyed */
		/* Otherwise keep looking */
	    }
    }
    return 0;
}

int FindWeaponIndex(MECH * mech, int number)
{
    int loop;
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int running_sum = 0;
    int num_weaps;
    int index;

    if (number < 0)
	return -1;		/* Anti-crash */
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (num_weaps <= 0)
	    continue;
	if (number < running_sum + num_weaps) {
	    /* we found it... */
	    index = number - running_sum;
	    return weaparray[index];
	}
	running_sum += num_weaps;
    }
    return -1;
}

int FindAmmoForWeapon_sub(MECH * mech, int weapindx, int start,
    int *section, int *critical, int nogof, int gof)
{
    int loop;
    int critloop;
    int desired;
    int found = 0;
    desired = I2Ammo(weapindx);
    if ((MechWeapons[weapindx].special & NARC) && (gof & NARC_MODE))
	gof &= ~NARC_MODE;

    /* Can't use LBX ammo as normal, but can use Narc and Artemis as normal */
    for (critloop = 0; critloop < NUM_CRITICALS; critloop++) {
	if (GetPartType(mech, start, critloop) == desired &&
	    !PartIsNonfunctional(mech, start, critloop) && (!nogof ||
		!(GetPartMode(mech, start, critloop) & nogof)) && (!gof ||
		(GetPartMode(mech, start, critloop) & gof))) {
	    *section = start;
	    *critical = critloop;
	    /* This lets us keep looking for the ammo, in case we have */
	    /* multiple places where it is stored.. */
	    if (!PartIsNonfunctional(mech, start, critloop) &&
		GetPartData(mech, start, critloop) > 0) {
		found = 1;
		return 1;
		}
	}
    }
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	if (loop == start)
	    continue;
	for (critloop = 0; critloop < NUM_CRITICALS; critloop++)
	    if (GetPartType(mech, loop, critloop) == desired &&
		!PartIsNonfunctional(mech, loop, critloop) && (!nogof ||
		    !(GetPartMode(mech, loop, critloop) & nogof)) && (!gof
		    || (GetPartMode(mech, loop, critloop) & gof))) {
		*section = loop;
		*critical = critloop;
		/* This lets us keep looking for the ammo, in case we have */
		/* multiple places where it is stored.. */
		if (!PartIsNonfunctional(mech, loop, critloop) &&
		    GetPartData(mech, loop, critloop) > 0) {
		    found = 1;
		    return 1;
		    }
	    }
    }
/*    if (!found && MechWeapons[weapindx].type == TMISSILE &&
	!(GetPartMode(mech, loop, critloop) & SPECIFIC_AMMO)) {
      for (loop = 0; loop < NUM_SECTIONS; loop++) {
        for (critloop = 0; critloop < NUM_CRITICALS; critloop++)
            if (GetPartType(mech, loop, critloop) == desired &&
                !PartIsNonfunctional(mech, loop, critloop)) {
                *section = loop;
                *critical = critloop;
                if (!PartIsNonfunctional(mech, loop, critloop) &&
                    GetPartData(mech, loop, critloop) > 0)
                    return 1;
            }
        }
    } */
    return 0;
}

int FindAmmoForWeapon(MECH * mech, int weapindx, int start, int *section,
    int *critical)
{
    return FindAmmoForWeapon_sub(mech, weapindx, start, section, critical,
	SPECIFIC_AMMO, 0);
}
/*
A Tad complicated.
00 - 11 -> Same Section Datamatch
Each Successive section datamatch is ((Sect + 1) * 12) + Crit.
This is to allow other sections to house Artemis for launchers.
Intended for FASA CT<->HD rule, but is open ended for peace and love
*/
int FindArtemisForWeapon(MECH * mech, int section, int critical)
{
    int critloop, sectloop;
    int desired;
    int sectbase, sectcrit;
    int maxcrit = (CritsInLoc(mech, section));
    desired = I2Special(ARTEMIS_IV);
    if (!(MechSpecials(mech) & ARTEMIS_IV_TECH))
	return 0;
    for (critloop = 0; critloop < maxcrit; critloop++)
        {
	if (GetPartType(mech, section, critloop) == desired && !PartIsNonfunctional(mech, section, critloop))
	    {
	    if (GetPartData(mech, section, critloop) == critical)
	        {
		return 1;
	        }
	    }
        }
    /* Uh oh. Better check the rest of the dood I s'pose */
    sectbase = ((section + 1) * 12);
    sectcrit = (sectbase + critical);
    for (sectloop = 0; sectloop < NUM_SECTIONS; sectloop++)
      {
      maxcrit = (CritsInLoc(mech, sectloop));
      for (critloop = 0; critloop < maxcrit; critloop++)
        {
        if (GetPartType(mech, sectloop, critloop) == desired && !PartIsNonfunctional(mech, sectloop, critloop))
            {
/*	    SendDebug(tprintf("Check : Sect %d Crit %d for %d - Part %d", sectloop, critloop, sectcrit, GetPartData(mech, sectloop, critloop))); */
            if (GetPartData(mech, sectloop, critloop) == sectcrit)
                {
                return 1;
                }
            }
          }
        }
    return 0;
}

int FindCapacitorForWeapon(MECH * mech, int section, int critical)
{
    int critloop;
    int desired;
    int data;
    desired = I2Special(CAPACITOR);
    for (critloop = 0; critloop < NUM_CRITICALS; critloop++)
        {
        if (GetPartType(mech, section, critloop) == desired && !PartIsNonfunctional(mech, section, critloop))
            {
	    if ((data = GetPartData(mech, section, critloop)) == critical)
                {
                return (critloop + 1);
                }
            }
        }
    return 0;
}

int FindDestructiveAmmo(MECH * mech, int *section, int *critical)
{
    int loop;
    int critloop;
    int maxdamage = 0;
    int damage;
    int weapindx;
    int i;
    int type, data, mode;

    for (loop = 0; loop < NUM_SECTIONS; loop++)
	for (critloop = 0; critloop < NUM_CRITICALS; critloop++)
	    if (IsAmmo(GetPartType(mech, loop, critloop)) &&
		!PartIsDestroyed(mech, loop, critloop)) {
		data = GetPartData(mech, loop, critloop);
		type = GetPartType(mech, loop, critloop);
		mode = GetPartMode(mech, loop, critloop);
		weapindx = Ammo2WeaponI(type);
		damage = data * GunStat(weapindx, mode, GUN_DAMAGE) * (IsAcid(weapindx) ? 3 : 1);
		if (MechWeapons[weapindx].special & GAUSS)
		    continue;
		if (IsMissile(weapindx) || IsArtillery(weapindx)) {
		    for (i = 0; MissileHitTable[i].key != -1; i++)
			if (MissileHitTable[i].key == weapindx)
			    damage *= MissileHitTable[i].num_missiles[10];
		}
		if (damage > maxdamage) {
		    *section = loop;
		    *critical = critloop;
		    maxdamage = damage;
		}
	    }
    return (maxdamage);
}

int FindRoundsForWeapon(MECH * mech, int weapindx)
{
    int loop;
    int critloop;
    int desired;
    int found = 0;

    desired = I2Ammo(weapindx);
    for (loop = 0; loop < NUM_SECTIONS; loop++)
	for (critloop = 0; critloop < NUM_CRITICALS; critloop++)
	    if (GetPartType(mech, loop, critloop) == desired &&
		!PartIsNonfunctional(mech, loop, critloop))
		found += GetPartData(mech, loop, critloop);
    return found;
}

const char *quad_locs[NUM_SECTIONS + 1] = {
    "Front Left Leg",
    "Front Right Leg",
    "Left Torso",
    "Right Torso",
    "Center Torso",
    "Rear Left Leg",
    "Rear Right Leg",
    "Head",
    NULL
};

const char *mech_locs[NUM_SECTIONS + 1] = {
    "Left Arm",
    "Right Arm",
    "Left Torso",
    "Right Torso",
    "Center Torso",
    "Left Leg",
    "Right Leg",
    "Head",
    NULL
};

const char *bsuit_locs[NUM_BSUIT_MEMBERS + 1] = {
    "Suit 1",
    "Suit 2",
    "Suit 3",
    "Suit 4",
    "Suit 5",
    "Suit 6",
    "Suit 7",
    "Suit 8",
    NULL
};

const char *veh_locs[NUM_VEH_SECTIONS + 1] = {
    "Left Side",
    "Right Side",
    "Front Side",
    "Aft Side",
    "Turret",
    "Rotor",
    NULL
};

const char *aero_locs[NUM_AERO_SECTIONS + 1] = {
    "Nose",
    "Left Wing",
    "Right Wing",
    "Aft",
    NULL
};

const char *ds_locs[NUM_DS_SECTIONS + 1] = {
    "Nose",
    "Left Wing",
    "Right Wing",
    "Aft",
    NULL
};


const char *ds_spher_locs[NUM_DS_SECTIONS + 1] = {
    "Nose",
    "Left Side",
    "Right Side",
    "Aft",
    NULL
};

const char **ProperSectionStringFromType(int type, int mtype)
{
    switch (type) {
    case CLASS_BSUIT:
	return bsuit_locs;
    case CLASS_MECH:
    case CLASS_MW:
	if (mtype == MOVE_QUAD)
	    return quad_locs;
	return mech_locs;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
    case CLASS_VEH_VTOL:
	return veh_locs;
    case CLASS_AERO:
	return aero_locs;
    case CLASS_SPHEROID_DS:
	return ds_spher_locs;
    case CLASS_DS:
	return ds_locs;
    }
    return NULL;
}

void ArmorStringFromIndex(int index, char *buffer, char type, char mtype)
{
    const char **locs = ProperSectionStringFromType(type, mtype);
    int high = 0;

    high = NumSections(type);
    if (high == 0) {
	strcpy(buffer, "Invalid!!");
	return;
	}
    if (high > 0 && index < high && locs) {
	strcpy(buffer, locs[index]);
	return;
    }
    strcpy(buffer, "Invalid!!");
}

int NumSections(int type)
{
    switch (type) {
    case CLASS_MECH:
    case CLASS_MW:
        return NUM_SECTIONS;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
        return (NUM_VEH_SECTIONS - 1);
    case CLASS_VEH_VTOL:
        return NUM_VEH_SECTIONS;
    case CLASS_AERO:
        return NUM_AERO_SECTIONS;
    case CLASS_SPHEROID_DS:
        return NUM_DS_SECTIONS;
    case CLASS_DS:
        return NUM_DS_SECTIONS;
    case CLASS_BSUIT:
        return NUM_BSUIT_MEMBERS;
    default:
        return 0;
    }
}

int IsInWeaponArc(MECH * mech, float x, float y, float z, int section, int critical)
{
#if 0
    int weaponarc;
    int isrear;
    int ts;

    if (MechType(mech) == CLASS_MECH && (section == LLEG || section == RLEG
	    || (MechMove(mech) == MOVE_QUAD && (section == LARM ||
		    section == RARM)))) {
	ts = MechStatus(mech) & (TORSO_LEFT | TORSO_RIGHT);
	MechStatus(mech) &= ~(ts);
	weaponarc = InWeaponArc(mech, x, y);
	MechStatus(mech) |= ts;
    } else
	weaponarc = InWeaponArc(mech, x, y);

    switch (MechType(mech)) {
    case CLASS_BSUIT:
    case CLASS_MW:
	return 1;
	break;
    case CLASS_MECH:
	isrear = (GetPartMode(mech, section, critical) & REAR_MOUNT) ||
	    ((section == LARM || section == RARM) &&
	    (MechStatus(mech) & FLIPPED_ARMS));
	if (weaponarc == REARARC)
	    return isrear;
	if (isrear)
	    return 0;
	if (weaponarc == FORWARDARC)
	    return 1;
	if (weaponarc == LSIDEARC && section == LARM)
	    return 1;
	if (weaponarc == RSIDEARC && section == RARM)
	    return 1;
	return 0;
        break;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
    case CLASS_VEH_VTOL:
	if (weaponarc >= TURRETARC && section == TURRET)
	    return 1;
	if (weaponarc >= TURRETARC)
	    weaponarc -= TURRETARC;
	if (weaponarc == FORWARDARC && section == FSIDE)
	    return 1;
	if (weaponarc == REARARC && section == BSIDE)
	    return 1;
	if (weaponarc == LSIDEARC && section == LSIDE)
	    return 1;
	if (weaponarc == RSIDEARC && section == RSIDE)
	    return 1;
	return 0;
	break;
    case CLASS_DS:
	switch (weaponarc) {
	case FORWARDARC:
	    return (section == DS_NOSE);
	case REARARC:
	    return (section == DS_AFT);
	case LSIDEARC:
	    return (section == DS_LWING || section == DS_LRWING);
	case RSIDEARC:
	    return (section == DS_RWING || section == DS_RRWING);
	default:
	    return 0;
	}
	break;
    case CLASS_SPHEROID_DS:
	switch (weaponarc) {
	case FORWARDARC:
	    return (section == DS_NOSE || section == DS_LWING ||
		section == DS_RWING);
	case REARARC:
	    return (section == DS_AFT || section == DS_LRWING ||
		section == DS_RRWING);
	case LSIDEARC:
	    return (section == DS_LWING || section == DS_LRWING);
	case RSIDEARC:
	    return (section == DS_RWING || section == DS_RRWING);
	default:
	    return 0;
	}

    case CLASS_AERO:
	isrear = (GetPartMode(mech, section, critical) & REAR_MOUNT);
        /* 'ts' used as temp variable in this instance */
        if ((ts = FindZBearing(MechFX(mech), MechFY(mech), MechFZ(mech), x, y, z)) >
		(MechAngle(mech) + 45) || ts < (MechAngle(mech) - 45))
	    return 0;
	switch (section) {
	case AERO_NOSE:
	    return weaponarc != REARARC;
	case AERO_LWING:
	    return (weaponarc == LSIDEARC || (weaponarc != RSIDEARC &&
		    isrear == (weaponarc == REARARC)));
	case AERO_RWING:
	    return (weaponarc == RSIDEARC || (weaponarc != LSIDEARC &&
		    isrear == (weaponarc == REARARC)));
	case AERO_AFT:
	    return (weaponarc == REARARC);
	default:
	    return 0;
	}
	break;
    }
    return 0;
#endif
    int weaponarc, isrear, ts;
    int wantarc = NOARC;

    if (MechType(mech) == CLASS_MECH && (section == LLEG || section == RLEG
            || (MechIsQuad(mech) && (section == LARM ||
                    section == RARM)))) {
        ts = MechStatus(mech) & (TORSO_LEFT | TORSO_RIGHT);

        MechStatus(mech) &= ~(ts);
        weaponarc = InWeaponArc(mech, x, y);
        MechStatus(mech) |= ts;
    } else
        weaponarc = InWeaponArc(mech, x, y);

    switch (MechType(mech)) {
    case CLASS_MECH:
    case CLASS_BSUIT:
    case CLASS_MW:
        if (GetPartMode(mech, section, critical) & REAR_MOUNT)
            wantarc = REARARC;
        else if (section == LARM && (MechStatus(mech) & FLIPPED_ARMS))
            wantarc = REARARC | LSIDEARC;
        else if (section == LARM)
            wantarc = FORWARDARC | LSIDEARC;
        else if (section == RARM && (MechStatus(mech) & FLIPPED_ARMS))
            wantarc = REARARC | RSIDEARC;
        else if (section == RARM)
            wantarc = FORWARDARC | RSIDEARC;
        else
            wantarc = FORWARDARC;
        break;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
    case CLASS_VTOL:
        switch (section) {
        case TURRET:
            wantarc = TURRETARC;
            break;
        case FSIDE:
            wantarc = FORWARDARC;
            break;
        case LSIDE:
            wantarc = LSIDEARC;
            break;
        case RSIDE:
            wantarc = RSIDEARC;
            break;
        case BSIDE:
            wantarc = REARARC;
            break;
        }
        break;
    case CLASS_SPHEROID_DS:
	/* DS_LR and RR WING's no longer used but oh well */
        ts = FindZBearing(MechFX(mech), MechFY(mech), MechFZ(mech), x, y, z);
        if (ts > (MechAngle(mech) + 45) || ts < (MechAngle(mech) - 45))
            return 0;
        switch (section) {
        case DS_NOSE:
            wantarc = FORWARDARC;
            break;
        case DS_LWING:
            wantarc = LSIDEARC;
            break;
        case DS_RWING:
            wantarc = RSIDEARC;
            break;
        case DS_AFT:
            wantarc = REARARC;
            break;
        }
        break;
    case CLASS_DS:
        ts = FindZBearing(MechFX(mech), MechFY(mech), MechFZ(mech), x, y, z);
        if (ts > (MechAngle(mech) + 45) || ts < (MechAngle(mech) - 45))
            return 0;
        isrear = (GetPartMode(mech, section, critical) & REAR_MOUNT);
        switch (section) {
        case DS_NOSE:
            wantarc = FORWARDARC;
            break;
        case DS_LWING:
	    if (weaponarc & LHALFARC && weaponarc & (isrear ? REARARC : FORWARDARC))
		return 1;
	    else
		return 0;
            break;
        case DS_RWING:
	    if (weaponarc & RHALFARC && weaponarc & (isrear ? REARARC : FORWARDARC))
		return 1;
	    else
		return 0;
            break;
        case DS_AFT:
            wantarc = REARARC;
            break;
        }
        break;
    case CLASS_AERO:
	ts = FindZBearing(MechFX(mech), MechFY(mech), MechFZ(mech), x, y, z);
        if (ts > (MechAngle(mech) + 45) || ts < (MechAngle(mech) - 45))
	    return 0;
        isrear = (GetPartMode(mech, section, critical) & REAR_MOUNT);
        switch (section) {
        case AERO_NOSE:
            wantarc = FORWARDARC;
            break;
        case AERO_LWING:
	    if (weaponarc & LHALFARC && weaponarc & (isrear ? REARARC : FORWARDARC))
		return 1;
	    else
		return 0;
            break;
        case AERO_RWING:
	    if (weaponarc & RHALFARC && weaponarc & (isrear ? REARARC : FORWARDARC))
		return 1;
	    else
		return 0;
            break;
        case AERO_AFT:
            wantarc = REARARC;
            break;
        }
        break;
    }
    return wantarc ? (wantarc & weaponarc) : 0;
}

int GetWeaponCrits(MECH * mech, int weapindx)
{
    return (MechType(mech) ==
	CLASS_MECH) ? (MechWeapons[weapindx].criticals) : 1;
}

int listmatch(char **foo, char *mat)
{
    int i;

    for (i = 0; foo[i]; i++)
	if (!strcasecmp(foo[i], mat))
	    return i;
    return -1;
}

/* Takes care of :
   JumpSpeed
   Numsinks

   TODO: More support(?)
 */

void do_sub_magic(MECH * mech, int loud)
{
    int jjs = 0;
    int hses = 0;
    int wanths, wanths_f;
    int shs_size =
	MechSpecials(mech) & CLAN_TECH ? 2 : MechSpecials(mech) &
	DOUBLE_HEAT_TECH ? 3 : 1;
    int hs_eff = MechSpecials(mech) & (DOUBLE_HEAT_TECH | CLAN_TECH) ? 2 :
      MechSpecials2(mech) & COMPACT_HEAT_TECH ? 2 : 1;
    int i, j;
    int inthses = ((MechType(mech) != CLASS_MECH) ? 50 : MechEngineSize(mech) / 25);
    int dest_hses = 0;
/*    int maxjjs = (int) ((float) MechMaxSpeed(mech) * MP_PER_KPH * 2 / 3); */
    int maxjjs = (WalkingSpeed(MechMaxSpeed(mech)) / KPH_PER_MP);

    if (MechSpecials(mech) & ICE_TECH)
	inthses = 0;
    for (i = 0; i < NUM_SECTIONS; i++)
	for (j = 0; j < CritsInLoc(mech, i); j++)
	    switch (Special2I(GetPartType(mech, i, j))) {
	    case HEAT_SINK:
		hses++;
		if (PartIsNonfunctional(mech, i, j))
		    dest_hses++;
		break;
	    case JUMP_JET:
		jjs++;
		break;
	    }
    hses +=
	MIN(MechRealNumsinks(mech) * shs_size / hs_eff, inthses * shs_size);
    if (jjs > maxjjs) {
	if (loud)
	    SendError(tprintf("Mech #%d - Error in (%s): %d JJs, yet %d maximum available (due to walk MPs)?",
		    mech->mynum, MechType_Ref(mech), jjs, maxjjs));

	jjs = maxjjs;
    }
    MechJumpSpeed(mech) = MP1 * jjs;
    wanths_f = (hses / shs_size) * hs_eff;
    wanths = wanths_f - (dest_hses * hs_eff / shs_size);
    if (loud)
	MechNumOsinks(mech) =
	    wanths - MIN(MechRealNumsinks(mech), inthses * hs_eff);
    if (wanths != MechRealNumsinks(mech) && loud) {
	SendError(tprintf("Mech #%d - Error in (%s): Set HS: %d. Existing HS: %d. Difference: %d. Please %s.",
		mech->mynum, MechType_Ref(mech), MechRealNumsinks(mech),
		wanths, MechRealNumsinks(mech) - wanths,
		wanths <
		MechRealNumsinks(mech) ? "add the extra HS critical(s)" :
		"fix the template"));
    } else
	MechRealNumsinks(mech) = wanths;

    MechNumOsinks(mech) = wanths_f;
}

#define CV(fun) fun(mech) = fun(&opp)

/* Values to take care of:
   - JumpSpeed
   - MaxSpeed
   - Numsinks
   - EngineHeat
   - PilotSkillBase
   - LRS/Tac/ScanRange
   - BTH

   Status:
   - Destroyed

   Critstatus:
   - kokonaan paitsi
   - LEG_DESTROYED / NO_LEGS

   section(s) / basetohit
 */

void do_magic(MECH * mech)
{
    MECH opp;
    int i, j, t, k;
    int mask = 0;

    if (MechType(mech) != CLASS_MECH)
	mask = (TURRET_LOCKED|TURRET_JAMMED);
    if (MechCritStatus(mech) & INFERNO_AMMO)
	MechCritStatus(mech) &= ~(INFERNO_AMMO);
	if (MechCritStatus2(mech) & HITCH_DESTROYED)
		MechCritStatus2(mech) &= ~(HITCH_DESTROYED);
    if (is_aero(mech))
	MechCritStatus2(mech) = 0;

    memcpy(&opp, mech, sizeof(MECH));
    mech_loadnew(GOD, &opp, MechType_Ref(mech));
    MechEngineSizeV(mech) = MechEngineSizeC(&opp);	/* From intact template */
    opp.mynum = -1;
    /* Ok.. It's at perfect condition. Start inflicting some serious crits.. */
    for (i = 0; i < NUM_SECTIONS; i++)
	for (j = 0; j < CritsInLoc(mech, i); j++) {
	    SetPartType(&opp, i, j, GetPartType(mech, i, j));
	    SetPartData(&opp, i, j, 0);
	    SetPartMode(&opp, i, j, 0);
    	    }
    if (MechType(mech) != CLASS_BSUIT)
	do_sub_magic(&opp, 0);
    MechNumOsinks(mech) = MechNumOsinks(&opp);
    for (i = 0; i < NUM_SECTIONS; i++) {
	if (MechType(mech) == CLASS_MECH) {
/*	    MechSChargeCounter(mech) = 0;
	    MechMASCCounter(mech) = 0; */
	    for (j = 0; j < CritsInLoc(mech, i); j++) {
		if (PartIsDestroyed(mech, i, j)) {
		    if (!PartIsDestroyed(&opp, i, j)) {
			if (!IsAmmo((t = GetPartType(mech, i, j)))) {
			    if (!IsWeapon(t))
				HandleMechCrit(&opp, NULL, 0, i, j, t,
				    GetPartData(mech, i, j), 0, 0);
			}
		    }
		} else {
		    t = GetPartType(mech, i, j);
		    if (IsAMS(Weapon2I(t))) {
			if (MechWeapons[Weapon2I(t)].special & CLAT)
			    MechSpecials(mech) |= CL_ANTI_MISSILE_TECH;
			else
			    MechSpecials(mech) |= IS_ANTI_MISSILE_TECH;
		    }
/*		    if (GetPartMode(mech, i, j) & OS_MODE) {
			GetPartMode(mech, i, j) &= ~OS_USED;
		    } */
		    if (GetPartMode(mech, i, j) & INFERNO_MODE) {
			MechCritStatus(mech) |= INFERNO_AMMO;
		    }
		}
	    }
	}
	if (SectIsDestroyed(mech, i))
	    DestroySection(&opp, NULL, 0, i);
	if (MechStall(mech) > 0)
	    UnSetSectBreached(mech, i);	/* Just in case ; this leads to 'unbreachable' legs once you've 'done your time' once */
        MechSections(mech)[i].basetohit = MechSections(&opp)[i].basetohit;
        if (MechSections(mech)[i].config & (SECTION_NARC|SECTION_INARC|SECTION_ACID))
  	    MechSections(mech)[i].config &= ~(SECTION_NARC|SECTION_INARC|SECTION_ACID);
/*
  	    (mech, i, 0);
*/
        if (MechSections(mech)[i].config & STABILIZER_CRIT)
            MechSections(mech)[i].config &= ~STABILIZER_CRIT;
/*        if (MechSections(&opp)[i].config & STABILIZER_CRIT)
            MechSections(&opp)[i].config &= ~STABILIZER_CRIT; */
        for (k = 0; k < CritsInLoc(mech, i); k++) {
            if (GetPartMode(mech, i, k) & JAMMED_MODE)
                GetPartMode(mech, i, k) &= ~JAMMED_MODE;
/*            if (GetPartMode(&opp, i, k) & JAMMED_MODE)
                GetPartMode(&opp, i, k) &= ~JAMMED_MODE; */
        }
    }
    CV(MechJumpSpeed);
    CV(MechMaxSpeed);
    CV(MechRealNumsinks);
    CV(MechEngineHeat);
    CV(MechPilotSkillBase);
    CV(MechBTH);

    MechCritStatus(mech) &= mask;
    MechCritStatus(mech) |= MechCritStatus(&opp) & (~mask);
    MechSChargeCounter(mech) = 0;
    MechMASCCounter(mech) = 0;
    MechStatus(mech) &= ~NARC_ATTACHED;
    MechStatus2(mech) &= ~INARC_ATTACHED;
/*    for (i = 0; i < NUM_SECTIONS; i++)
	MechSections(mech)[i].config &= ~(SECTION_NARC|SECTION_INARC); */
    if (MechType(mech) != CLASS_MECH)
	if (MechCritStatus(mech) & (TURRET_LOCKED|TURRET_JAMMED))
	    MechCritStatus(mech) &= ~(TURRET_LOCKED|TURRET_JAMMED);
/*    for (i = 0; i < NUM_SECTIONS; i++) {
	MechSections(mech)[i].basetohit = MechSections(&opp)[i].basetohit;
        if (MechSections(mech)[i].config & STABILIZER_CRIT)
            MechSections(mech)[i].config &= ~STABILIZER_CRIT;
        if (MechSections(&opp)[i].config & STABILIZER_CRIT)
            MechSections(&opp)[i].config &= ~STABILIZER_CRIT;
	for (j = 0; j < NUM_CRITICALS; j++) {
	    if (GetPartMode(mech, i, j) & JAMMED_MODE)
		GetPartMode(mech, i, j) &= ~JAMMED_MODE;
            if (GetPartMode(&opp, i, j) & JAMMED_MODE)
                GetPartMode(&opp, i, j) &= ~JAMMED_MODE;
	}
    } */
/* Case of undestroying */
    if (!Destroyed(&opp) && Destroyed(mech))
	MechStatus(mech) &= ~DESTROYED;
    else if (Destroyed(&opp) && !Destroyed(mech))
	MechStatus(mech) |= DESTROYED;
    if (!Destroyed(mech) && MechType(mech) != CLASS_MECH)
	EvalBit(MechStatus(mech), FALLEN, Fallen(&opp));
    update_specials(mech);
}

void mech_RepairPart(MECH * mech, int loc, int pos)
{
    int t = GetPartType(mech, loc, pos);
    int m = GetPartMode(mech, loc, pos);

    UnDestroyPart(mech, loc, pos);
    if (IsAmmo(t) && (m & HALFTON_MODE)) {
	GetPartMode(mech, loc, pos) |= HALFTON_MODE;
	}

    if (IsWeapon(t) || IsAmmo(t))
	SetPartData(mech, loc, pos, 0);
    else if (IsSpecial(t)) {
	switch (Special2I(t)) {
	case TARGETING_COMPUTER:
	case HEAT_SINK:
	case LIFE_SUPPORT:
	case COCKPIT:
	case SENSORS:
	case JUMP_JET:
	case ENGINE:
	case GYRO:
	case SHOULDER_OR_HIP:
	case LOWER_ACTUATOR:
	case UPPER_ACTUATOR:
	case HAND_OR_FOOT_ACTUATOR:
	case C3_MASTER:
	case C3_SLAVE:
	case C3I:
	case ECM:
	case BEAGLE_PROBE:
	case ARTEMIS_IV:
	case SUPERCHARGER:
	    /* Magic stuff here :P */
	    if (MechType(mech) == CLASS_MECH)
		do_magic(mech);
	    break;
	}
    }
}

int no_locations_destroyed(MECH * mech)
{
    int i;

    for (i = 0; i < NUM_SECTIONS; i++)
	if (GetSectOInt(mech, i) && SectIsDestroyed(mech, i))
	    return 0;
    return 1;
}

void mech_ReAttach(MECH * mech, int loc)
{
    if (!SectIsDestroyed(mech, loc))
	return;
    UnSetSectDestroyed(mech, loc);
    SetSectInt(mech, loc, GetSectOInt(mech, loc));
    if (is_aero(mech))
	SetSectInt(mech, loc, 1);
    if (MechType(mech) != CLASS_MECH) {
	if (no_locations_destroyed(mech) && IsDS(mech))
	    MechStatus(mech) &= ~DESTROYED;
	return;
    }
    switch (loc) {
    case LLEG:
    case RLEG:
	if (MechCritStatus(mech) & NO_LEGS)
	    MechCritStatus(mech) &= ~NO_LEGS;
	else if (MechCritStatus(mech) & LEG_DESTROYED)
	    MechCritStatus(mech) &= ~LEG_DESTROYED;
	break;
    }
}

/*
 * Added for new flood code by Kipsta
 * 8/4/99
 */

void mech_ReSeal(MECH * mech, int loc)
{
    int i;

    if (SectIsDestroyed(mech, loc))
	return;
    if (!SectIsFlooded(mech, loc))
	return;

    UnSetSectFlooded(mech, loc);

    switch (loc) {
    case LLEG:
    case RLEG:
	if (MechCritStatus(mech) & NO_LEGS)
	    MechCritStatus(mech) &= ~NO_LEGS;
	else if (MechCritStatus(mech) & LEG_DESTROYED)
	    MechCritStatus(mech) &= ~LEG_DESTROYED;
	break;
    case LARM:
    case RARM:
	if (MechMove(mech) == MOVE_QUAD) {
	    if (MechCritStatus(mech) & NO_LEGS)
		MechCritStatus(mech) &= ~NO_LEGS;
	    else if (MechCritStatus(mech) & LEG_DESTROYED)
		MechCritStatus(mech) &= ~LEG_DESTROYED;
	}
	break;

    }

    for (i = 0; i < CritsInLoc(mech, loc); i++)
	if (PartIsDisabled(mech, loc, i) && !PartIsDestroyed(mech, loc, i))
	    mech_RepairPart(mech, loc, i);
}

void mech_Detach(MECH * mech, int loc)
{
    if (SectIsDestroyed(mech, loc))
	return;
    DestroySection(mech, NULL, 0, loc);
}

/* Figures out how much ammo there is when we're 'fully loaded', and
   fills it */
void mech_FillPartAmmo(MECH * mech, int loc, int pos)
{
    int t, to, os = 0;

    t = GetPartType(mech, loc, pos);

    if (!IsAmmo(t) && !(os = (GetPartMode(mech, loc, pos) & OS_MODE)))
	return;
    if (os)
	GetPartMode(mech, loc, pos) &= ~OS_USED;
    else {
        if (!(to = MechWeapons[Ammo2Weapon(t)].ammoperton))
	    return;
        SetPartData(mech, loc, pos, FullAmmo(mech, loc, pos));
    }
}

int AcceptableDegree(int d)
{
    while (d < 0)
	d += 360;
    while (d >= 360)
	d -= 360;
    return d;
}

void MarkForLOSUpdate(MECH * mech)
{
    MAP *mech_map;

    if (!(mech_map = getMap(mech->mapindex)))
	return;
    mech_map->moves++;
    mech_map->mechflags[mech->mapnumber] = 1;
    /* We need to remove this baby from the calculated ranges list.. *sniff* */
    RCache_Remove(mech);
}

void multi_weap_sel(MECH * mech, dbref player, char *buffer, int bitbybit,
    int (*foo) (MECH *, dbref, int, int))
{
    /* Insight: buffer contains stuff in form:
       <num>
       <num>-<num>
       <num>,..
       <num>-<num>,..
     */
    /* Ugly recursive piece of code :> */
    char *c;
    int i1, i2, i3;
    int section, critical;

    skipws(buffer);
    if ((c = strstr(buffer, ","))) {
	*c = 0;
	c++;
    }
    if (sscanf(buffer, "%d-%d", &i1, &i2) == 2) {
	DOCHECK(i1 < 0 ||
	    i1 >= MAX_WEAPONS_PER_MECH,
	    tprintf("Invalid first number in range (%d)", i1));
	DOCHECK(i2 < 0 ||
	    i2 >= MAX_WEAPONS_PER_MECH,
	    tprintf("Invalid second number in range (%d)", i2));
	if (i1 > i2) {
	    i3 = i1;
	    i1 = i2;
	    i2 = i3;
	}
    } else {
	DOCHECK(Readnum(i1, buffer), tprintf("Invalid value: %s", buffer));
	DOCHECK(i1 < 0 ||
	    i1 >= MAX_WEAPONS_PER_MECH,
	    tprintf("Invalid weapon number: %d", i1));
	i2 = i1;
    }
    if (bitbybit / 2) {
	DOCHECK(i2 >= NUM_TICS, tprintf("There are only %d tics!", i2));
    } else {
	DOCHECK(!(FindWeaponNumberOnMech(mech, i2, &section,
		    &critical) != -1),
	    tprintf("Error: the mech doesn't HAVE %d weapons!", i2 + 1));
    }
    if (bitbybit % 2) {
	for (i3 = i1; i3 <= i2; i3++)
	    if (foo(mech, player, i3, i3))
		return;
    } else if (foo(mech, player, i1, i2))
	return;
    if (c)
	multi_weap_sel(mech, player, c, bitbybit, foo);
}

int Roll()
{
    int i = Number(1, 6) + Number(1, 6);

    stat.rolls[i - 2]++;
    stat.totrolls++;
    return i;
}

int MyHexDist(int x1, int y1, int x2, int y2, int tc)
{
    int xd = abs(x2 - x1);
    int yd = abs(y2 - y1);
    int hm;

    /* _the_ base case */
    if (x1 == x2)
	return yd;
    /*
       +
       +
       +
       +
     */
    if ((hm = (xd / 2)) <= yd)
	return (yd - hm) + tc + xd;

    /*
       +     +
       +   +
       + +
       +
     */
    if (!yd)
	return (xd + tc);
    /*
       +
       +
       +   +
       + +
       +
     */
    /* For now, same as above */
    return (xd + tc);
}

/* Major changes to WeaponIsNonfunctional().
   Returns 0 if fully functional.
   Returns 1 if non functional.
   Returns 2 if fully damaged.
   Returns -(# of crits) if partially damaged.
   remember that values 3 means the weapon IS NOT destroyed.  */
int WeaponIsNonfunctional(MECH * mech, int section, int crit, int numcrits)
{
    int sum = 0, disabled = 0, dested = 0;

    if (numcrits <= 0)
	numcrits = GetWeaponCrits(mech, Weapon2I(GetPartType(mech, section, crit)));
    while (sum < numcrits) {
	if (PartIsDestroyed(mech, section, crit + sum))
		dested++;
	else if (PartIsDisabled(mech, section, crit + sum))
		disabled++;
	sum++;
	}
    if (disabled > 0)
	return 1;
    if ((numcrits == 1 && (dested || disabled)) || (numcrits > 1 && (dested + disabled) >= numcrits / 2))
	return 2;
    if (dested)
	return 0 - (dested + disabled);
    return 0;
}

int WeaponFirstCrit(MECH * mech, int section, int crit)
{
/* Beware of non-Weapons passing through (HEAT_SINKs) they can bey nasty when numcrits returns 0 or less. */
/* Should be blocked, but still. On Guard. - DJ */
    int type = (GetPartType(mech, section, crit));
    int numcrits = (GetWeaponCrits(mech, Weapon2I(type)));
    int loop = 0;

    if (IsSpecial(type))
	if (Special2I(type) == HEAT_SINK)
	    numcrits = ((MechSpecials(mech) & CLAN_TECH) ? 2 :
			(MechSpecials(mech) & DOUBLE_HEAT_TECH) ? 3 : 1);
    if (numcrits < 1)
	numcrits = 1;
    if (numcrits == 1)
	return crit;
    for (loop = 0; loop < NUM_CRITICALS; loop++)
	{
	if (GetPartType(mech, section, loop) == type) {
		if (crit < (loop + numcrits))
			{
			return loop;
			break;
			} else {
			loop = (loop + (numcrits - 1));
			}
		}
	}
return crit;
}

int LocHasJointCrit(MECH * mech, int section)

{
    int i = 0;
    int crittype;

    if (MechType(mech) != CLASS_MECH)
	return 0;

    for (i = 0; i < NUM_CRITICALS; i++) {
	crittype = GetPartType(mech, section, i);
	if (IsSpecial(crittype))
		if (PartIsNonfunctional(mech, section, i) && (Special2I(crittype) == SHOULDER_OR_HIP)) {
			return 1;
			break;
			}

    }
    return 0;
}

int MechNumLegs(MECH * mech)
{

    int count = 0;

    if (MechType(mech) != CLASS_MECH)
	return -1;
    if (!SectIsDestroyed(mech, RLEG) && !SectIsFlooded(mech, RLEG) && !SectIsBreached(mech, RLEG))
	count++;
    if (!SectIsDestroyed(mech, LLEG) && !SectIsFlooded(mech, LLEG) && !SectIsBreached(mech, LLEG))
	count++;
    if (MechMove(mech) == MOVE_QUAD)
	{
	if (!SectIsDestroyed(mech, RARM) && !SectIsFlooded(mech, RARM) && !SectIsBreached(mech, RARM))
		count++;
	if (!SectIsDestroyed(mech, LARM) && !SectIsFlooded(mech, LARM) && !SectIsBreached(mech, LARM))
		count++;
	}
return count;
}

int MechNumJointCrits(MECH * mech, int loctype)
{

    int Quad = (MechMove(mech) & MOVE_QUAD);
    int count = 0;
 /* loctype - 1 = Legs | 0 = Arms  */

    if (MechType(mech) != CLASS_MECH)
	return -1;
    if (LocHasJointCrit(mech, (loctype ? LLEG : LARM)))
	count++;
    if (LocHasJointCrit(mech, (loctype ? RLEG : RARM)))
	count++;
    if (Quad) {
	if (LocHasJointCrit(mech, (loctype ? LARM : LLEG)))
		count++;
	if (LocHasJointCrit(mech, (loctype ? RARM : RLEG)))
		count++;
	}
    return count;
}

void CalcLegCritSpeed(MECH * mech)
{
    int actcrits = 0, hipcrits = 0, engcrits = 0;
    int i, crit, newspeed;
    int numlegs = (MechNumLegs(mech));
    int dll = 0, drl = 0, dra = 0, dla = 0;
    int Quad = (MechMove(mech) == MOVE_QUAD);
/*    int destleg = -1;*/

    if (MechType(mech) != CLASS_MECH)
	return;

    if (MechSpecials(mech) & ICE_TECH) {
	if (MechEngineHeat(mech) >= 5)
	    engcrits++;
	if (MechEngineHeat(mech) >= 10)
	    engcrits++;
	}

	MechCritStatus(mech) &= ~(HIP_DAMAGED);
  /* We're giving Quads the benefit of the doubt. First perform actuator -1MP mods, then 1/2 speed mods. Only affects first */
  /* leg loss, but it might help in the cumulation of speed crits to be nice to the buggers. - DJ */

  /* Find the Dested Legs in order to ignore it's crits */
    if (SectIsDestroyed(mech, RLEG) || SectIsFlooded(mech, RLEG) || SectIsBreached(mech, RLEG))
	{
/*	if (destleg == -1 && Quad)
		destleg = RLEG;*/
	drl = 1;
	}
    if (SectIsDestroyed(mech, LLEG) || SectIsFlooded(mech, LLEG) || SectIsBreached(mech, LLEG))
	{
/*	if (destleg == -1 && Quad)
		destleg = LLEG;*/
        dll = 1;
	}
    if (Quad)
	{
	    if (SectIsDestroyed(mech, RARM) || SectIsFlooded(mech, RARM) || SectIsBreached(mech, RARM))
		{
/*		if (destleg == -1 && Quad)
			destleg = RARM;*/
	        dra = 1;
		}
	    if (SectIsDestroyed(mech, LARM) || SectIsFlooded(mech, LARM) || SectIsBreached(mech, LARM))
		{
/*		if (destleg == -1 && Quad)
			destleg = LARM;*/
		dla = 1;
		}
	}
  /* sum up all crits */
    for (i = 0; i < NUM_CRITICALS; i++)
	{
	if (Quad)
		{
		crit = GetPartType(mech, RARM, i);
		if (IsSpecial(crit) && IsActuator(crit) && PartIsNonfunctional(mech, RARM, i) && dra != 1) {
			if ((Special2I(crit) != SHOULDER_OR_HIP) && (!LocHasJointCrit(mech, RARM))) {
				actcrits++;
			} else if (Special2I(crit) == SHOULDER_OR_HIP) {
				hipcrits++;
				MechCritStatus(mech) |= HIP_DAMAGED; } }
	        crit = GetPartType(mech, LARM, i);
	        if (IsSpecial(crit) && IsActuator(crit) && PartIsNonfunctional(mech, LARM, i) && dla != 1) {
	                if ((Special2I(crit) != SHOULDER_OR_HIP) && (!LocHasJointCrit(mech, LARM))) {
	                        actcrits++;
	                } else if (Special2I(crit) == SHOULDER_OR_HIP) {
	                        hipcrits++;
				MechCritStatus(mech) |= HIP_DAMAGED; } }
		}
        crit = GetPartType(mech, RLEG, i);
        if (IsSpecial(crit) && IsActuator(crit) && PartIsNonfunctional(mech, RLEG, i) && drl != 1) {
                if ((Special2I(crit) != SHOULDER_OR_HIP) && (!LocHasJointCrit(mech, RLEG))) {
                        actcrits++;
                } else if (Special2I(crit) == SHOULDER_OR_HIP) {
                        hipcrits++;
				MechCritStatus(mech) |= HIP_DAMAGED; } }
        crit = GetPartType(mech, LLEG, i);
        if (IsSpecial(crit) && IsActuator(crit) && PartIsNonfunctional(mech, LLEG, i) && dll != 1) {
                if ((Special2I(crit) != SHOULDER_OR_HIP) && (!LocHasJointCrit(mech, LLEG))) {
                        actcrits++;
                } else if (Special2I(crit) == SHOULDER_OR_HIP) {
                        hipcrits++;
				MechCritStatus(mech) |= HIP_DAMAGED; } }

	}
  /* Increment actcrits to account for 1MP loss for leg loss - DJ */
    if (!Quad && ((MechCritStatus(mech) & NO_LEGS) || hipcrits >= 2 ))
	{
	SetMaxSpeed(mech, 0);
	return;
	} else if (!Quad && (MechCritStatus(mech) & LEG_DESTROYED)) {
		if (MechCritStatus(mech) & GYRO_DESTROYED) {
			SetMaxSpeed(mech, 0);
			return;
		}
	    if (actcrits > 0 || hipcrits > 0 || engcrits > 0)
		SetMaxSpeed(mech, 0);
	    else
		SetMaxSpeed(mech, MP1);
	    return;
	} else if (Quad && (MechCritStatus(mech) & (NO_LEGS | LEG_DESTROYED))) {
		if (MechCritStatus(mech) & GYRO_DESTROYED) {
			SetMaxSpeed(mech, 0);
			return;
		}
		switch (numlegs)
			{
			case 0:
			case 1:
				SetMaxSpeed(mech, 0);
				break;
			case 2:
				SetMaxSpeed(mech, MP1);
				break;
			case 3:
				actcrits++;
				newspeed = (MechOMaxSpeed(mech) - (MP1 * actcrits));
				i = hipcrits + engcrits;
				while (i > 0)
					{
					newspeed = (newspeed / 2);
					i--;
					}
				SetMaxSpeed(mech, newspeed);
				break;
			default:
				break;
			}
	} else if (MechCritStatus(mech) & GYRO_DESTROYED) {
		if (actcrits > 0 || hipcrits > 0 || engcrits > 0)
			SetMaxSpeed(mech, 0);
		else
			SetMaxSpeed(mech, MP1);
		return;
	} else {
	newspeed = (MechOMaxSpeed(mech) - (MP1 * actcrits));
	i = hipcrits + engcrits;
	while (i > 0)
		{
		newspeed = (newspeed / 2);
		i--;
		}
	SetMaxSpeed(mech, newspeed);
	}
    return;
}

int HeatFactor(MECH * mech)
{
    int factor = 0;

    if (MechType(mech) != CLASS_MECH)
	{
	factor = (((MechSpecials(mech) & ICE_TECH)) ? -1 : 21);
	return factor;
	} else {
	factor = (MechPlusHeat(mech) + (2 * (MechPlusHeat(mech)  - MechMinusHeat(mech))));
	return (((MechStatus2(mech) & NULLSIG_ACTIVE) || StealthAndECM(mech)) ? -1 : factor);
	}
    SendDebug(tprintf("HeatFactor : Invalid heat factor calculation on #%d.", mech->mynum));
}

int HasFuncTAG(MECH * mech)
{
    int l, ll, loopcrit, hasTAG = 0;

    if (!mech)
	return 0;
    if (MechStatus(mech) & ECM_DISTURBANCE)
	return 0;
    for (l = 0; l < NUM_SECTIONS; l++)
         {
         for (ll = 0; ll < NUM_CRITICALS; ll++)
            {
            loopcrit = (GetPartType(mech, l, ll));
	   if (!PartIsNonfunctional(mech, l, ll))
	        if (IsSpecial(loopcrit))
                    if (Special2I(loopcrit) == TAG)
                            hasTAG++;
            }
         }
    return hasTAG;
}

int MechIsSwarmed(MECH * mech)
{
    int i;
    MECH *t;
    MAP *map;

    map = getMap(mech->mapindex);
    if (!map || !mech)
	return 0;

    for (i = 0; i < map->first_free; i++)
        {
        if (!(t = FindObjectsData(map->mechsOnMap[i])) || !map)
                continue;
        if (MechType(t) == CLASS_BSUIT)
	    {
	    if (MechSwarmTarget(t) == mech->mynum) {
		if (MechTeam(t) == MechTeam(mech))
		    return 2;
		else
		    return 1;
	      }
            }
        }
    return 0;
}
/* Dynamic Gun Stats function for allowing DFM/ATM/More to vary satistics based on Ammo/Mode type */
/* Basically pretty simple, if not manual for the programmer to remember/lookup function switches */
/* rangenum = mode of data output	*/
/* 0 = minrange				*/
/* 1 = shortrange			*/
/* 2 = medrange				*/
/* 3 = longrange			*/
/* 4 = damage				*/
/* 5 = heat				*/
/* 6 = vrt				*/
/* GUN_MINRANGE    0
   GUN_SHORTRANGE  1
   GUN_MEDRANGE    2
   GUN_LONGRANGE   3
   GUN_AERORANGE   4
   GUN_DAMAGE      5
   GUN_HEAT        6
   GUN_VRT         7 */

/* For now, now EGunRange crap. Noone uses it. It's Lamer, specially with newtech. Etc.. */

int GunStat(int weapindx, int mode, int num)
{
    int output = -1, vammo = (mode & (VARIABLE_AMMO));
    int heatmod = 1;

    if (num <= GUN_AERORANGE) {
	switch (num) {
	    case GUN_AERORANGE:
		return GetAeroRange(weapindx);
	    case GUN_MINRANGE:
                output = MechWeapons[weapindx].min;
		if (vammo) {
		    if ((mode & DEADFIRE_MODE) && output > 0)
		        output = (output - 2);
		    else if (mode & ATMHE_MODE)
		        output = 0;
		    }
		    return output;
		break;
	    case GUN_SHORTRANGE:
		output = MechWeapons[weapindx].shortrange;
		if (vammo) {
		    if (mode & DEADFIRE_MODE)
		        output = output - 1;
		    else if (mode & ATMER_MODE)
		        output = 9;
                    else if (mode & ATMHE_MODE)
                        output = 3;
		    }
		return output;
		break;
	    case GUN_MEDRANGE:
         	output = MechWeapons[weapindx].medrange;
		if (vammo) {
		    if (mode & DEADFIRE_MODE)
		        output = output - 2;
                    else if (mode & ATMER_MODE)
    	                output = 18;
                    else if (mode & ATMHE_MODE)
                        output = 6;
		    }
		return output;
		break;
	    case GUN_LONGRANGE:
		output = MechWeapons[weapindx].longrange;
		if (vammo) {
		    if (mode & DEADFIRE_MODE)
		        output = output - 3;
                    else if (mode & ATMER_MODE)
                        output = 28;
                    else if (mode & ATMHE_MODE)
                        output = 9;
		    else if (mode & STINGER_MODE)
			output = (output + 7);
		    }
		return output;
		break;
	    }
	}
    if (num == GUN_DAMAGE) {
	output = MechWeapons[weapindx].damage;
	    if (vammo) {
	        if (mode & DEADFIRE_MODE)
		    output++;
	        else if (mode & ATMER_MODE)
		    output--;
	        else if (mode & ATMHE_MODE)
		    output++;
		else if (mode & TRACER_MODE)
		    output--;
		}
	if (MechWeapons[weapindx].type == TBEAM && (mode & CAPPED_MODE))
	    output = (output + 5);
	if (MechWeapons[weapindx].special & RANDDAM)
	    return Number(1, output);
	return output;
	}
    if (num == GUN_HEAT) {
	if (mode & (ULTRA_MODES)) {
	    if (mode & ROTSIX_MODE)
		heatmod = 6;
	    else if (mode & ROTFOUR_MODE)
		heatmod = 4;
	    else if (mode & ULTRA_MODE)
		heatmod = 2;
	    }
	output = MechWeapons[weapindx].heat;
/*	if (MechWeapons[weapindx].type == TBEAM && (mode & CAPPED_MODE))
	    output = (output + 5); */
	if (mode & STINGER_MODE)
	    output += 1;
	return (output * heatmod);
	}
    if (num == GUN_VRT) {
	output = MechWeapons[weapindx].vrt;
	return output;
	}
    return output;
}

int GetAeroRange(int weapindx)
{
char rng = MechWeapons[weapindx].aero_weaprange;

if (rng == AERORNG_NONE)
    return 0;
if (rng == AERORNG_SHORT)
    return AERORNG_SHORT_RNG;
if (rng == AERORNG_MEDIUM)
    return AERORNG_MEDIUM_RNG;
if (rng == AERORNG_LONG)
    return AERORNG_LONG_RNG;
if (rng == AERORNG_EXT)
    return AERORNG_EXT_RNG;
return 0;
}

/* Expandable in future for other items. For now ID'ing Artemis_IV to save/load template PartData - DJ */
int IsDataSpecial(int index)
{
    if (index == ARTEMIS_IV || index == CAPACITOR)
	return 1;
    return 0;
}

int NumCapacitorsOn(MECH * mech)
{
    int count = 0, i, ii;

    for (i = 0; i < NUM_SECTIONS; i++)
	for (ii = 0; ii < NUM_CRITICALS; ii++)
	    if (Special2I(GetPartType(mech, i, ii)) == CAPACITOR)
		if ((GetPartMode(mech, i, ii) & CAPPED_MODE) && !PartIsNonfunctional(mech, i, ii))
		    count++;
    return count;
}

float EconPartWeight(int part, int index, char *name)
{
    int i = -1, p, ispc;
    float sw = 0.0;

    if (!part && !index && !name)
	return -1;
    if (part)
	p = part;
    else if (index)
	p = I2Special(index);
    else if (name) {
        i = -1;
        if (!find_matching_long_part(name, &i, &p)) {
          i = -1;
          if (!find_matching_vlong_part(name, &i, &p))
            return -1;
        }
        if (strstr(name, "Sword") && !strstr(name, "PC."))
            p = I2Special(SWORD);
    }

    if (IsWeapon(p)) {
	if (MechWeapons[Weapon2I(p)].class == WCLASS_PC)
	    sw = (1024 / 32);
	else
            sw = ((float) ((float) ((float) 1024 / 10) * (float) MechWeapons[Weapon2I(p)].weight) / (float) MechWeapons[Weapon2I(p)].criticals);
	}
    else if (IsAmmo(p))
        sw = ((float) 1024 / (float) MechWeapons[Ammo2I(p)].ammoperton);
    else if (IsBomb(p))
        sw = ((float) BombWeight(Bomb2I(p)));
    else if (IsSpecial(p)) /* && p <= I2Special(LAMEQUIP) */
        sw = ((float) specialweight[Special2I(p)]);
    else if (IsCargo(p))
        sw = ((float) cargoweight[Cargo2I(p)]);
    else
        sw = ((float) 1024 / 10);

    return sw;
}

/* This one is a tad strange. Returns -1 if not dumping, else the crit that is dumping (0-11) */

int IsLocDumping(MECH *mech, int loc, int *realloc)
{
    int data = -1;
    int arg, i, l, nloc, weapindx, orloc = -1, orloc2 = -1;;

    if (!mech)
	return -1;
    if (!Dumping(mech))
	return -1;
    if (MechType(mech) == CLASS_MECH) {
	switch (loc) {
	    case RTORSO:
		orloc == RARM;
		orloc2 == RLEG;
		break;
	    case LTORSO:
		orloc2 == RLEG;
		orloc == LARM;
		break;
	    case CTORSO:
		orloc == HEAD;
		break;
	    }
	}

    i = MechType(mech) == CLASS_MECH ? 7 : 5;

    if ((data = DumpData(mech)) == -1) {
	return -1;
    } else {
/* This type is a global AllDump(tm) type.  We have to search the mech location for a match */
    if (data == -1) {
	SendDebug("Logic Error. Somehow data of -1 came through DumpData()");
	return -1;
	}
    arg = data;
    if (!arg) {
            for (l = CritsInLoc(mech, loc) - 1; l >= 0; l--)
                  if (IsAmmo(GetPartType(mech, loc, l)))
                      if (GetPartData(mech, loc, l)) {
			  *realloc = loc;
                          return (l);
			  }
	    if (orloc > -1)
              for (l = CritsInLoc(mech, orloc) - 1; l >= 0; l--)
                  if (IsAmmo(GetPartType(mech, orloc, l)))
                      if (GetPartData(mech, orloc, l)) {
			  *realloc = orloc;
                          return (l);
			}

	    if (orloc2 > -1)
              for (l = CritsInLoc(mech, orloc2) - 1; l >= 0; l--)
                  if (IsAmmo(GetPartType(mech, orloc2, l)))
                      if (GetPartData(mech, orloc2, l)) {
			  *realloc = orloc2;
                          return (l);
			}
        return -1;
    }
    if (arg < 256) {
        nloc = arg - 1;
        if (loc != nloc || orloc != nloc || orloc2 != nloc)
	    return -1;
        l = CritsInLoc(mech, nloc);
        for (i = 0; i < l; i++)
            if (IsAmmo(GetPartType(mech, nloc, i)))
                if (!PartIsDestroyed(mech, nloc, i))
                    if (GetPartData(mech, nloc, i)) {
			*realloc = nloc;
			return (i);
			}
        return -1;
    }
    if (arg < 65536) {
        weapindx = arg / 256;
        for (l = CritsInLoc(mech, loc) - 1; l >= 0; l--)
            if (IsAmmo(GetPartType(mech, loc, l)))
                if (Ammo2WeaponI(GetPartType(mech, loc, l)) == weapindx)
                    if (GetPartData(mech, loc, l)) {
			*realloc = loc;
			return (l);
			}
        if (orloc > -1)
          for (l = CritsInLoc(mech, orloc) - 1; l >= 0; l--)
            if (IsAmmo(GetPartType(mech, orloc, l)))
                if (Ammo2WeaponI(GetPartType(mech, orloc, l)) == weapindx)
                    if (GetPartData(mech, orloc, l)) {
			*realloc = orloc;
			return (l);
			}
	if (orloc2 > -1)
          for (l = CritsInLoc(mech, orloc2) - 1; l >= 0; l--)
            if (IsAmmo(GetPartType(mech, orloc2, l)))
                if (Ammo2WeaponI(GetPartType(mech, orloc2, l)) == weapindx)
                    if (GetPartData(mech, orloc2, l)) {
			*realloc = orloc2;
			return (l);
			}
        return - 1;
    }
    l = (arg >> 16) & 0xFF;
    i = (arg >> 24) & 0xFF;
    if (loc == l || orloc == l || orloc2 == l)
        if (GetPartData(mech, l, i)) {
	    *realloc = l;
	    return (i);
	    }
    return -1;
    }
}

void NarcClear(MECH *mech, int hitloc)
{
int i, stillnarc = 0, stillinarc = 0;

MechSections(mech)[hitloc].config &= ~(SECTION_NARC|SECTION_INARC);

if (MechStatus(mech) & NARC_ATTACHED) {
    for (i = 0 ; i < NUM_SECTIONS ; i++)
	if (MechSections(mech)[i].config & SECTION_NARC)
	    {
	    stillnarc = 1;
	    break;
	    }
    }
if (MechStatus2(mech) & INARC_ATTACHED) {
    for (i = 0 ; i < NUM_SECTIONS ; i++)
	if (MechSections(mech)[i].config & SECTION_INARC)
	    {
	    stillnarc = 1;
	    break;
	    }
    }
if (!stillnarc)
    MechStatus(mech) &= ~(NARC_ATTACHED);
if (!stillinarc)
    MechStatus2(mech) &= ~(INARC_ATTACHED);
return;
}

void AcidClear(MECH *mech, int loc, int bc)
{
	char buf[32];

	if (!(MechSections(mech)[loc].config & SECTION_ACID))
		return;

	MechSections(mech)[loc].config |= ~(SECTION_ACID);
	event_remove_type_data_data(EVENT_ACID, (void *) mech, (void *) loc);

	if (SectIsDestroyed(mech, loc))
		return;

	if (bc) {
		ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
		mech_notify(mech, MECHALL,tprintf("The acid on your %s wears off!", buf));
		MechLOSBroadcast(mech, tprintf("stops fuming from its %s.", buf));
	}
	return;
}

#undef DEBUG_BV
#define HIGH_SKILL	8
#define LOW_SKILL	0
float skillmul[HIGH_SKILL][HIGH_SKILL] = {
{ 2.05 , 2.00 , 1.95 , 1.90 , 1.85 , 1.80 , 1.75 , 1.70 },
{ 1.85 , 1.80 , 1.75 , 1.70 , 1.65 , 1.60 , 1.55 , 1.50 },
{ 1.65 , 1.60 , 1.55 , 1.50 , 1.45 , 1.40 , 1.35 , 1.30 },
{ 1.45 , 1.40 , 1.35 , 1.30 , 1.25 , 1.20 , 1.15 , 1.10 },
{ 1.25 , 1.20 , 1.15 , 1.10 , 1.05 , 1.00 , 0.95 , 0.90 },
{ 1.15 , 1.10 , 1.05 , 1.00 , 0.95 , 0.90 , 0.85 , 0.80 },
{ 1.05 , 1.00 , 0.95 , 0.90 , 0.85 , 0.80 , 0.75 , 0.70 },
{ 0.95 , 0.90 , 0.85 , 0.80 , 0.75 , 0.70 , 0.65 , 0.60 }
};

#define LAZY_SKILLMUL(n)  (n < LOW_SKILL ? LOW_SKILL : n >= HIGH_SKILL - 1 ? HIGH_SKILL - 1 : n)

#if 0

int CalculateBV(MECH *mech)
{
	float defbv = 0.0, offbv = 0.0, maxheat = 0.0, skmul = 1.0;
	int type, move, ms, ms2, plt = 5, gun = 4;
	float internal = 0.0;
	int i, ii;
	int weapindx, parttype, partmode, partdata, tank = 0, bsuit = 0;
	float maxspeed = 0.0, jumpspeed = 0.0;
	int maxmod = 10, jumpmod = 10;
	int bv = 9999;

	if (!mech)
	    return -1;
	if (event_tick - MechBVLast(mech) < 30)
	    return MechBV(mech);
	else
	    MechBVLast(mech) = event_tick;

	type = MechType(mech);
	move = MechMove(mech);
	ms = MechSpecials(mech);
	ms2 = MechSpecials2(mech);
	if (GotPilot(mech)) {
	plt = FindSPilotPiloting(mech);
	gun = FindAverageGunnery(mech);
	}
	if (type == CLASS_VEH_GROUND || type == CLASS_VTOL)
		tank = 1;
	else if (type == CLASS_MW || type == CLASS_BSUIT)
		bsuit =1;

	for (i = 0; i < NUM_SECTIONS; i++) {
		defbv += GetSectArmor(mech, i) * (ms & HARDA_TECH ? 2.0 : 1.0) * (tank || bsuit ? 1.0 : 2.0);
		if (type == CLASS_MECH && (i == CTORSO || i == LTORSO || i == RTORSO)) {
			defbv += GetSectRArmor(mech, i) * (ms & HARDA_TECH ? 4.0 : 2.0);
		}
		if (i == CTORSO && ms2 & TORSOCOCKPIT_TECH) {
			defbv += (GetSectArmor(mech, i) + GetSectRArmor(mech, i)) * (ms & HARDA_TECH ? 4.0 : 2.0);
		}

		internal += GetSectInt(mech, i);
/*		defbv += GetSectInt(mech, i) * (ms & XL_TECH ? ms & CLAN_TECH ? 1.125 : 0.75 : 1.5;*/

		for (ii = 0; ii < CritsInLoc(mech, i); ii++) {
			parttype = GetPartType(mech, i, ii);
			if (IsWeapon(parttype)) {
				weapindx = Weapon2I(parttype);
				if (PartIsNonfunctional(mech, i, ii)) {
					if (type == CLASS_MECH)
						ii += MechWeapons[weapindx].criticals - 1;
					continue;
				}
				partmode = GetPartMode(mech, i, ii);
				if (MechWeapons[weapindx].class == WCLASS_AMS)
					defbv += MechWeapons[weapindx].battlevalue * 30.0 / GunStat(weapindx, partmode, GUN_VRT);
				else if (MechWeapons[weapindx].class == WCLASS_PC)
					offbv += MechWeapons[weapindx].battlevalue * 0.1;
				else {
					offbv += MechWeapons[weapindx].battlevalue *
					 (partmode & REAR_MOUNT ? 0.5 : 1.0) * (partmode & ON_TC ? 1.2 : 1.0) *
					 30 / GunStat(weapindx, partmode, GUN_VRT);
					if (MechWeapons[weapindx].type == TMISSILE)
						if (FindArtemisForWeapon(mech, i, ii))
							offbv += MechWeapons[weapindx].battlevalue * 0.2;
				}
				if (type == CLASS_MECH) {
					if (!(partmode & REAR_MOUNT))
						maxheat += MechWeapons[weapindx].heat *
						(MechWeapons[weapindx].special & OS_WEAP ? 0.25 : 1.0) *
						 ((MechWeapons[weapindx].special & ULTRA) && !(MechWeapons[weapindx].special & RFM) ? 2.0 : 1.0) *
						 (MechWeapons[weapindx].special & STREAK ? 15.0 : 30.0) /
						 GunStat(weapindx, partmode, GUN_VRT);
					ii += MechWeapons[weapindx].criticals - 1;
				}
			} else if (IsAmmo(parttype)) {
				if (PartIsNonfunctional(mech, i, ii) || !(partdata = GetPartData(mech, i, ii)))
					continue;
				partmode = GetPartMode(mech, i, ii);
				weapindx = Ammo2WeaponI(parttype);
				if (MechWeapons[weapindx].class == WCLASS_AMS)
					defbv += MechWeapons[weapindx].abattlevalue * partdata / MechWeapons[weapindx].ammoperton;
				else {
					offbv += MechWeapons[weapindx].abattlevalue * partdata / MechWeapons[weapindx].ammoperton *
					 (partmode & (PIERCE_MODE|SGUIDED_MODE) ? 6.0 : partmode & SWARM1_MODE ? 1.5 : LBX_MODE ? 0.75 : 1.0);
				 }
			}

			if ((IsAmmo(parttype) && !(MechWeapons[Ammo2WeaponI(parttype)].special & GAUSS))
			 && type == CLASS_MECH) {
				if (ms & CLAN_TECH) {
					if (i == CTORSO || i == HEAD || i == RLEG || i == LLEG
					 || (move == MOVE_QUAD && (i == RARM || i == LARM))) {
						 defbv -= 20.0;
					 }
					 continue;
				}
				if (ms & (XL_TECH|XXL_TECH)) {
					defbv -= 20.0;
					continue;
				}
				if (i == RARM || i == LARM) {
					if (!(MechSections(mech)[i].config & CASE_TECH)
					 && !(MechSections(mech)[(i == RARM ? RTORSO : LTORSO)].config & CASE_TECH))
						defbv -= 20.0;
					continue;
				}
				if (i == CTORSO || i == RLEG || i == LLEG || i == HEAD
				 || !(MechSections(mech)[i].config & CASE_TECH)) {
					 defbv -= 20.0;
					 continue;
				}
			}
		}
	}
	defbv += internal * (tank ? 0.5 : bsuit ? 2.0 : 1.5) *
	 	(ms & ICE_TECH ? 0.5 : ms2 & LENG_TECH ? 1.125 :
	 		ms & CLAN_TECH ? ms & XXL_TECH ? 0.75 : ms & XL_TECH ? 1.125 : 1.0 :
	 		ms & XXL_TECH ? 0.5 : ms & XL_TECH ? 0.75 : 1.0)
	 	* (ms & REINFI_TECH ? 2.0 : ms & COMPI_TECH ? 0.5 : 1.0);
	if (type == CLASS_MECH) {
		maxheat += MechJumpSpeed(mech) > 0 ?
		 MAX(MechJumpSpeed(mech) * MP_PER_KPH, 6.0) * (ms & ICE_TECH ? 2.0 : 1.0) : ms & ICE_TECH ? 0.0 : 2.0;
		if (ms & NULLSIG_TECH || ms2 & STEALTHARMOR_TECH)
			maxheat += 10.0;
		defbv -= (maxheat - MechMinusHeat(mech) > 0 ? maxheat - MechMinusHeat(mech) : 0.0) * 5.0;
	}
	if (ms & ECM_TECH)
		defbv += 61.0;
	if (ms & ANGEL_ECM_TECH)
		defbv += 100.0;
	if (ms & BEAGLE_PROBE_TECH)
		offbv += ms & CLAN_TECH ? 12.0 : 10.0;
	if (ms & BLOODHOUND_PROBE_TECH)
		offbv += 25.0;
	if (ms2 & HDGYRO_TECH)
		defbv += 30.0;
	if (type == CLASS_MECH)
		defbv += MechTons(mech);
	else if (tank)
		defbv = defbv * (type == CLASS_VTOL ? 0.4 : move == MOVE_TRACK ? 0.8 :
		move == MOVE_WHEEL ? 0.7 : move == MOVE_HOVER ? 0.6 : 1.0);

	jumpspeed = (type == CLASS_VTOL ? MechMaxSpeed(mech) : MechJumpSpeed(mech));
	maxspeed = MechOMaxSpeed(mech);
	if (ms & TRIPLE_MYOMER_TECH)
		maxspeed = (WalkingSpeed(maxspeed) + MP1) * 1.5;
	if (ms & MASC_TECH || ms2 & SCHARGE_TECH) {
		if (ms & MASC_TECH && ms2 & SCHARGE_TECH)
			maxspeed = WalkingSpeed(maxspeed) * 2.5;
		else
			maxspeed = WalkingSpeed(maxspeed) * 2.0;
	}
	if (ms & HARDA_TECH)
		maxspeed = maxspeed <= MP1 ? maxspeed : maxspeed - MP1;

	maxmod = maxspeed <= MP2 ? 10 : maxspeed <= MP4 ? 11 : maxspeed <= MP6 ? 12
		   : maxspeed <= MP9 ? 13 : maxspeed <= MP13 ? 14 : maxspeed <= MP18 ? 15
		   : maxspeed <= MP24 ? 16 : 17;
	jumpmod = jumpspeed <= 0 ? 10 : jumpspeed <= MP2 ? 11 : jumpspeed <= MP4 ? 12 : jumpspeed <= MP6 ? 13
			: jumpspeed <= MP9 ? 14 : jumpspeed <= MP13 ? 15 : jumpspeed <= MP18 ? 16
			: jumpspeed <= MP24 ? 17 : 18;
	defbv = defbv * ((jumpmod > maxmod ? jumpmod : maxmod)
		+ (ms & NULLSIG_TECH || ms2 & STEALTHARMOR_TECH ? 5 : 0)) / 10;

	if (type == CLASS_MECH && maxheat > MechMinusHeat(mech))
		offbv = offbv * (1 + MechMinusHeat(mech) / maxheat) / 2.0;
	if (type == CLASS_VTOL)
		jumpspeed /= 2;
	offbv = offbv * pow(((MechMaxSpeed(mech) + jumpspeed) * MP_PER_KPH
		+ (ms & MASC_TECH ? 1 : 0) + (ms2 & SCHARGE_TECH ? 1 : 0)
		+ (ms & TRIPLE_MYOMER_TECH ? 1 : 0) - 5) * 0.1 + 1, 1.2);
/*if (gun && plt)
	skmul = skillmul[LAZY_SKILLMUL(gun)][LAZY_SKILLMUL(plt)];

bv = (offbv * 100 + defbv * 100) * skmul * 0.01;*/
bv = offbv + defbv;

return bv;
/*	if (ms & (XL_TECH|XXL_TECH) || ms2 & LENG_TECH) {
		if (ms & CLAN_TECH)
			defbv += internal * (ms & XXL_TECH ? .75 : 1.125);
		else
			defbv += internal * (ms & XXL_TECH ? .5 : ms & XL_TECH ? .75 : 1.125);
	} else
		defbv += internal;*/
}

#else

int CalculateBV(MECH *mech, int gunstat, int pilstat)
{
int defbv = 0, offbv = 0, i, ii, temp, temp2, deduct = 0, offweapbv = 0, defweapbv = 0, armor = 0, intern = 0, weapindx, mostheat = 0, tempheat = 0, mechspec, mechspec2, type, move, pilskl = pilstat, gunskl = gunstat;
int debug1 = 0, debug2 = 0, debug3 = 0, debug4 = 0;
float maxspeed, mul = 1.00;

if (!mech)
    return 0;

if (gunstat == 100 || pilstat == 100) {
    if (event_tick - MechBVLast(mech) < 30)
	return MechBV(mech);
    else 
	MechBVLast(mech) = event_tick;
    }

type = MechType(mech);
move = MechMove(mech);
mechspec = MechSpecials(mech);
mechspec2 = MechSpecials2(mech);
if (gunstat == 100)
    pilskl = FindPilotPiloting(mech);
if (pilstat == 100)
    gunskl = FindAverageGunnery(mech);


for (i = 0; i < NUM_SECTIONS; i++) {
	armor += (debug1 = GetSectArmor(mech, i) * (mechspec & HARDA_TECH ? 200 : 100));
        if (type == CLASS_MECH &&  (i == CTORSO || i == LTORSO || i == RTORSO)) {
	    armor += (debug2 = GetSectRArmor(mech, i) * (mechspec & HARDA_TECH ? 200 : 100));
	    if (mechspec2 & TORSOCOCKPIT_TECH && i == CTORSO)
	        armor += (debug4 = (((GetSectArmor(mech, i) + GetSectRArmor(mech, i)) * 2) * (mechspec & HARDA_TECH ? 200 : 100)));
	}
	if (!is_aero(mech))
	    intern += (debug3 = GetSectInt(mech, i) * (mechspec & COMPI_TECH ? 50 : mechspec & REINFI_TECH ? 200 : 100));
	else
	    intern = (debug3 = AeroSI(mech));
#ifdef DEBUG_BV
        SendDebug(tprintf("Armoradd : %d ArmorRadd : %d Internadd : %d", debug1 / 100, debug2 / 100, debug3 / 100));
        if (mechspec2 & TORSOCOCKPIT_TECH && i == CTORSO)
	    SendDebug(tprintf("TorsoCockpit Armoradd : %d", debug4));
#endif

	debug1 = debug2 = debug3 = debug4 = 0;
	for (ii = 0; ii < CritsInLoc(mech, i); ii++)
	    {
	    if (IsWeapon(temp = GetPartType(mech, i, ii)))
		{
		weapindx = (Weapon2I(temp));
		if (PartIsNonfunctional(mech, i, ii)) {
		    if (type == CLASS_MECH)
		        ii += (MechWeapons[weapindx].criticals - 1);
		    continue;
		}
	    if (MechWeapons[weapindx].class == WCLASS_AMS) {
                defweapbv += (debug1 = (MechWeapons[weapindx].battlevalue * 100) * (float) (3000 / (GunStat(weapindx, GetPartMode(mech, i, ii), GUN_VRT) * 100)));

#ifdef DEBUG_BV
                SendDebug(tprintf("DefWeapBVadd (%s) : %d - Total : %d", MechWeapons[weapindx].name, debug1 / 100, defweapbv / 100));
#endif

            } else {
                offweapbv += (debug1 = (MechWeapons[weapindx].battlevalue * (GetPartMode(mech, i, ii) & REAR_MOUNT ? 50 : 100)) * (float) ((float) 3000 / (float) (GunStat(weapindx, GetPartMode(mech, i, ii), GUN_VRT) * 100)));
		if (MechWeapons[weapindx].type == TMISSILE)
		    if (FindArtemisForWeapon(mech, i, ii))
			offweapbv += (MechWeapons[weapindx].battlevalue * 20);
#ifdef DEBUG_BV
                SendDebug(tprintf("OffWeapBVadd (%s) : %d - Total : %d", MechWeapons[weapindx].name, debug1 / 100, offweapbv / 100));
#endif

            }
	    if (type == CLASS_MECH) {
              if (!(GetPartMode(mech, i, ii) & REAR_MOUNT)) {
                tempheat = ((MechWeapons[weapindx].heat * 100)  * (float) ((float) 3000 / (float) (GunStat(weapindx, GetPartMode(mech, i, ii), GUN_VRT) * 100)));
                if (MechWeapons[weapindx].special & ULTRA)
                    tempheat = (tempheat * 2);
                if (MechWeapons[weapindx].special & STREAK)
                    tempheat = (tempheat / 2);
                mostheat += tempheat;
#ifdef DEBUG_BV
                SendDebug(tprintf("Tempheatadded (%s) : %d - Total : %d", MechWeapons[weapindx].name, tempheat / 100, mostheat / 100));
#endif
                tempheat = 0;
                }
	      }
	    if (type == CLASS_MECH)
  	        ii += (MechWeapons[weapindx].criticals - 1);
	    } else if (IsAmmo(temp)) {
		if (PartIsNonfunctional(mech, i, ii) || !GetPartData(mech, i, ii))
		    continue;
		mul = ((temp2 = GetPartMode(mech, i, ii)) & PIERCE_MODE ? 4 :
		 temp2 & PRECISION_MODE ? 6 :
		 temp2 & (TRACER_MODE|STINGER_MODE|SWARM_MODE|SWARM1_MODE|SGUIDED_MODE) ? 1.5 : 1);
        mul = (mul * ((float) ((float) GetPartData(mech, i, ii) /
         (float) MechWeapons[weapindx = Ammo2WeaponI(temp)].ammoperton)));

#ifdef DEBUG_BV
		SendDebug(tprintf("AmmoBVmul (%s) : %.2f", MechWeapons[weapindx].name, mul));
#endif

		if (MechWeapons[weapindx].class == WCLASS_AMS) {
		    defweapbv += (debug1 = ((MechWeapons[weapindx].abattlevalue * 100) * mul) * (float) ((float) 3000 / (float) (GunStat(weapindx, GetPartMode(mech, i, ii), GUN_VRT) * 100)));

#ifdef DEBUG_BV
		    SendDebug(tprintf("AmmoDefWeapBVadd (%s) : %d - Total : %d", MechWeapons[weapindx].name, debug1 / 100, defweapbv / 100));
#endif

		} else {

#ifdef DEBUG_BV
		    SendDebug(tprintf("Abattlebalue (%s) : %d", MechWeapons[weapindx].name, MechWeapons[weapindx].abattlevalue));
#endif

		    offweapbv += (debug1 = ((MechWeapons[weapindx].abattlevalue * 100) * mul) * (float) ((float) 3000 / (float) (GunStat(weapindx, GetPartMode(mech, i, ii), GUN_VRT) * 100)));

#ifdef DEBUG_BV
		    SendDebug(tprintf("AmmoOffWeapBVadd (%s)  : %d - Total : %d", MechWeapons[weapindx].name, debug1 / 100, offweapbv / 100));
#endif

		    }
	        }
	    if ((IsAmmo(temp) || (IsWeapon(temp) && MechWeapons[(Weapon2I(temp))].special & GAUSS)) && type == CLASS_MECH)
		{
		if (mechspec & CLAN_TECH)
		    if (i == CTORSO || i == HEAD || i == RLEG || i == LLEG) {

#ifdef DEBUG_BV
			SendDebug("20 deduct added for ammo");
#endif

			deduct += 2000;
			continue;
			}
		if (mechspec & (XL_TECH|XXL_TECH|ICE_TECH) || mechspec2 & LENG_TECH) {

#ifdef DEBUG_BV
		    SendDebug("20/2000 deduct added for ammo");
#endif

		    deduct += 2000;
		    continue;
		    }
		if ((i == CTORSO || i == RLEG || i == LLEG || i == HEAD) && !(MechSections(mech)[i].config & CASE_TECH)) {

#ifdef DEBUG_BV
		    SendDebug("20 deduct added for ammo");
#endif

		    deduct += 2000;
		    continue;
		    }
		if ((i == RARM || i == LARM) && (!(MechSections(mech)[i].config & CASE_TECH) && !(MechSections(mech)[(i == RARM ? RTORSO : LTORSO)].config & CASE_TECH))) {

#ifdef DEBUG_BV
		    SendDebug("20 deduct added for ammo");
#endif

		    deduct += 2000;
		    continue;
		    }
		}

	    }
	}
if (type == CLASS_MECH) {
    mostheat += (MechJumpSpeed(mech) > 0 ? MAX((MechJumpSpeed(mech) / MP1) * 100, 300) : 200);
    if (mechspec & NULLSIG_TECH || mechspec2 & STEALTHARMOR_TECH)
	mostheat += 1000;
    if ((temp = (mostheat - (MechActiveNumsinks(mech) * 100))) > 0) {
        deduct += temp * 5;
#ifdef DEBUG_BV
        SendDebug(tprintf("Deduct add for heat : %d", (temp * 5) / 100));
#endif
        }
    }
#ifdef DEBUG_BV
SendDebug(tprintf("DeductTotal : %d", deduct / 100));
#endif

if (mechspec & ECM_TECH)
    defweapbv += 6100;

if (mechspec & BEAGLE_PROBE_TECH) {
    if (mechspec & CLAN_TECH)
	offweapbv += 1200;
    else
        offweapbv += 1000;
    }

if (mechspec2 & HDGYRO_TECH)
    defweapbv += 3000;

if (mechspec & (XL_TECH|XXL_TECH) || mechspec2 & LENG_TECH) {
    if (mechspec & CLAN_TECH || mechspec2 & LENG_TECH)
	mul = 1.125;
    else
	mul = 0.75;
} else if (mechspec & ICE_TECH || MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VEH_NAVAL) {
    mul = 0.5;
} else {
mul = 1.5;
}

#ifdef DEBUG_BV
SendDebug(tprintf("InternMul : %.2f", mul));
#endif

armor = (armor * (MechType(mech) == CLASS_MECH ? 2 : 1));
intern = intern * mul;
mul = 1.00;

#ifdef DEBUG_BV
SendDebug(tprintf("ArmorEnd : %d IntEnd : %d", armor / 100, intern / 100));
#endif

maxspeed = MMaxSpeed(mech);
if (mechspec & MASC_TECH || mechspec2 & SCHARGE_TECH) {
    if (mechspec & MASC_TECH && mechspec2 & SCHARGE_TECH)
	maxspeed = maxspeed * 2.5;
    else
	maxspeed = maxspeed * 1.5;
    }
if (mechspec & TRIPLE_MYOMER_TECH)
    maxspeed = ((WalkingSpeed(maxspeed) + MP1) * 1.5);

if (maxspeed <= MP2) {
        mul = 1.0;
    } else if (maxspeed <= MP4) {
        mul = 1.1;
    } else if (maxspeed <= MP6) {
        mul = 1.2;
    } else if (maxspeed <= MP9) {
        mul = 1.3;
    } else if (maxspeed <= MP13) {
        mul = 1.4;
    } else if (maxspeed <= MP18) {
        mul = 1.5;
    } else if (maxspeed <= MP24) {
        mul = 1.6;
    } else {
        mul = 1.7;
    }

if (IsDS(mech))
    mul = 1.0;
else if (is_aero(mech))
    mul = 1.1;

if (mechspec & NULLSIG_TECH || mechspec2 & STEALTHARMOR_TECH)
    mul += 1.5;
else if (mechspec & DC_STEALTH_TECH)
    mul += .75;
else if (mechspec & FWL_STEALTH_TECH)
    mul += 1.5;
else if (mechspec & CS_STEALTH_TECH)
    mul += 2.0;

#ifdef DEBUG_BV
SendDebug(tprintf("DefBVMul : %.2f", mul));
#endif

defbv = (armor + intern + (MechTons(mech) * 100) + defweapbv);

#ifdef DEBUG_BV
SendDebug(tprintf("DefBV Tonnage added : %d", MechTons(mech)));
#endif

if ((defbv - deduct) < 1)
    defbv = 1;
else
    defbv -= deduct;
if (type != CLASS_MECH)
    defbv = ((defbv * (move == MOVE_TRACK ? 0.8 : move == MOVE_WHEEL ? 0.7 : move == MOVE_HOVER ? 0.6 : move == MOVE_VTOL ? 0.4 : move == MOVE_FOIL || move == MOVE_SUB || move == MOVE_HULL ? 0.5 : 1.0)) - deduct);
defbv = defbv * mul;

#ifdef DEBUG_BV
SendDebug(tprintf("DefBV : %d", defbv / 100));
#endif

if ((type == CLASS_MECH || is_aero(mech)) && mostheat > (MechActiveNumsinks(mech) * 100)) {
#ifdef DEBUG_BV
SendDebug(tprintf("Pre-Heat OffWeapBV : %d", offweapbv / 100));
#endif
    i = (((MechActiveNumsinks(mech) / 100) * offweapbv) / mostheat);
    ii = ((offweapbv - i) / 2);
    offweapbv = i + ii;

#ifdef DEBUG_BV
SendDebug(tprintf("Post-Heat OffWeapBV : %d", offweapbv / 100));
#endif
}
/*
mul = pow(((((MMaxSpeed(mech) / MP1) + (type == CLASS_AERO || type == CLASS_DS ? 0 : (MechJumpSpeed(mech) / MP1)) + (mechspec & MASC_TECH ? 1 : 0) + (mechspec & TRIPLE_MYOMER_TECH ? 1 : 0) + (mechspec2 & SCHARGE_TECH ? 1 : 0) - 5) / 10) + 1), 1.2);
*/
mul = pow((((((IsDS(mech) ? WalkingSpeed(MMaxSpeed(mech)) : MMaxSpeed(mech)) / MP1) + (mechspec & MASC_TECH ? 1 : 0) + (mechspec & TRIPLE_MYOMER_TECH ? 1 : 0) + (mechspec2 & SCHARGE_TECH ? 1 : 0) - 5) / 10) + 1), 1.2);

#ifdef DEBUG_BV
SendDebug(tprintf("DumbMul : %.2f", mul));
#endif

if (mechspec2 & OMNI_TECH)
    mul += .3;

offweapbv = offweapbv * mul;
if (type != CLASS_AERO && type != CLASS_DS && MechJumpSpeed(mech) > 0)
    offweapbv += ((MechJumpSpeed(mech) / MP1) * (100 * (MechTons(mech) / 5)));
offbv = offweapbv;

#ifdef DEBUG_BV
SendDebug(tprintf("OffWeapBVAfter : %d", offweapbv / 100));
SendDebug(tprintf("DefBV : %d OffBV : %d TotalBV : %d", defbv / 100, offbv / 100, (offbv + defbv) / 100));
#endif

mul = (skillmul[LAZY_SKILLMUL(gunskl)][LAZY_SKILLMUL(pilskl)]);

#ifdef DEBUG_BV
SendDebug(tprintf("SkillMul : %.2f (%d/%d)", mul, gunskl, pilskl));
#endif
return ((offbv + defbv) / 100) * mul;
}

#endif

int MechFullNoRecycle(MECH * mech, int num)
{
    int i;

    for (i = 0; i < NUM_SECTIONS; i++) {
	if (num & CHECK_WEAPS && SectHasBusyWeap(mech, i))
	    return 1;
	if (num & CHECK_PHYS && MechSections(mech)[i].recycle > 0)
	    return 2;
	}
    return 0;
}

void SlagWeapon(MECH * wounded, MECH * attacker, int LOS, int hitloc, int critHit, int critType)
{
	int sum = 0;
	int firstcrit = WeaponFirstCrit(wounded, hitloc, critHit);
	int numcrits = GetWeaponCrits(wounded, Weapon2I(critType));

	mech_notify(wounded, MECHALL, "The weapon is completely slagged!");
	if (LOS && wounded != attacker && attacker)
	    mech_notify(attacker, MECHALL, "Your shot completely slags the weapon!");
	while (sum < numcrits) {
	    if (!PartIsDestroyed(wounded, hitloc, firstcrit + sum))
		DestroyWeapon(wounded, hitloc, critType, numcrits, firstcrit + sum);
	    sum++;
	}
}

void DumpCarrier(MECH * mech)
{
dbref thing;
MECH *tmech;

if (!mech)
    return;

if (In_Character(mech->mynum) && mech->mynum != NOTHING && Has_contents(mech->mynum) && mech->mapindex > 0) {
	DOLIST(thing, Contents(mech->mynum)) {
	    tmech = getMech(thing);
	    if (!tmech)
		continue;
	    mech_udisembark(GOD, tmech, my2string(""));
	} 
    }
}

float BombPoints(MECH * mech)
{
float bpoints = 0; 
int i, ii, type;

if (!mech || !is_aero(mech))
    return bpoints;

for (i = 0; i < NUM_SECTIONS; i++)
    for (ii = 0; ii < NUM_CRITICALS; ii++) {
	type = GetPartType(mech, i, ii);
	if (IsBomb(type)) {
	    bpoints += BombStrength(Bomb2I(type)) * GetPartData(mech, i, ii);
	    }
	}
return bpoints; 
}

float BombThrustLoss(MECH * mech)
{
float loss = 0, num;
int bpoints = 0;
int i, ii, type;

if (!mech || !is_aero(mech))
    return loss;

bpoints = BombPoints(mech);

num = bpoints / 10;
return KPH_PER_MP * (num / 5); 
}

float MaxBombPoints(MECH * mech)
{
float maxp = 0;
int i, ii, type;

if (!mech || !is_aero(mech))
    return maxp;

maxp = (MechTons(mech) / 5) * 10;

for (i = 0; i < NUM_SECTIONS; i++)
    for (ii = 0; ii <NUM_CRITICALS; ii++) {
	type = GetPartType(mech, i, ii);
	if (IsSpecial(type)) {
	    if (Special2I(type) == BOMB_RACK)
		maxp += 50;
	    }
	}

return maxp;
}

void DamageAeroSI(MECH * mech, int dmg, MECH * attacker)
{ 
/* Ya, can send 0 for a 'check' also */

if (!mech || !is_aero(mech))
    return;

if ((AeroSI(mech) - dmg) <= 0)
    AeroSI(mech) = 0;
else
    AeroSI(mech) -= dmg;

if (AeroSI(mech) <= 0) {
    mech_notify(mech, MECHALL, "The blast causes last of your craft's structural integrity disappear, blowing it to peices!");
    if (attacker && !Landed(mech) && Started(mech)) {
	mech_notify(attacker, MECHALL, "You shoot the craft from the sky!");
	MechLOSBroadcasti(attacker, mech, "shoots %s from the sky!");
	}
    DestroyMech(mech, attacker, !(!Landed(mech) && Started(mech))); 
    }
}

MECH* FindTargetDBREFFromID(MAP * map, char *id)
{
int loop;
MECH *tempmech;

    if (!map || !id)
	return NULL;

    for (loop = 0; loop < map->first_free; loop++)
        if (map->mechsOnMap[loop] > 0) {
            tempmech = (MECH *) FindObjectsData(map->mechsOnMap[loop]);
            if (tempmech && !strncasecmp(MechID(tempmech), id, 2)) {
		return tempmech;
                }
            }
    return NULL;
}

int GetPartMod(MECH * mech, int t)
{
int val, div, bound;
 
    div = (t && t == Special(GYRO) ? 100 : t && t == Special(ENGINE) ? 20 : 10);
    bound = (t && t == Special(GYRO) ? 3 : t && t == Special(ENGINE) ? 19 : 9);
    val = (t && (t == Special(GYRO) || t == Special(ENGINE)) ? MechEngineSize(mech) : MechTons(mech));

    if (val % div != 0)
	val = val + (div - (val % div));

return BOUNDED(0, (val / div) - 1, bound);
}

int ProperArmor(MECH * mech)
{
/* For now they all use the same basic cargo parts. */
return Cargo(MechSpecials(mech) & FF_TECH ? FF_ARMOR : MechSpecials(mech) & HARDA_TECH ? HD_ARMOR : MechSpecials2(mech) & (LFF_TECH|HFF_TECH) ? FF_ARMOR : MechSpecials2(mech) & STEALTHARMOR_TECH ? STEALTHARM : S_ARMOR);
}

int ProperInternal(MECH * mech)
{
int part = 0;

    if (mudconf.btech_complexrepair) {
	part = (MechSpecials(mech) & ES_TECH ? TON_ESINTERNAL_FIRST : MechSpecials(mech) & REINFI_TECH ? TON_REINTERNAL_FIRST : MechSpecials(mech) & COMPI_TECH ? TON_COINTERNAL_FIRST : TON_INTERNAL_FIRST);
	part += GetPartMod(mech, 0);
    } else {
	part = (MechSpecials(mech) & ES_TECH ? ES_INTERNAL : MechSpecials(mech) & REINFI_TECH ? RE_INTERNAL : MechSpecials(mech) & COMPI_TECH ? CO_INTERNAL : S_INTERNAL);
    }
return Cargo(part);
}

/*
#define SHOULDER_OR_HIP        0
#define UPPER_ACTUATOR         1
#define LOWER_ACTUATOR         2
#define HAND_OR_FOOT_ACTUATOR  3
*/

int alias_part(MECH * mech, int t, int loc)
{
int part = 0;

if (!IsSpecial(t))
    return t;

    if (mudconf.btech_complexrepair) { 
	int tonmod = GetPartMod(mech, t);
	int locmod;
	if (MechIsQuad(mech))
	    locmod = (loc == RARM || loc == LARM || loc == RLEG || loc == LLEG ? 2 : 0);
	else
	    locmod = (loc == RARM || loc == LARM ? 1 : loc == LLEG || loc == RLEG ? 2 : 0);

	part = (locmod && (t == Special(SHOULDER_OR_HIP) || t == Special(UPPER_ACTUATOR))  ? (locmod == 1 ? Cargo(TON_ARMUPPER_FIRST + tonmod) : Cargo(TON_LEGUPPER_FIRST + tonmod)) :
	    locmod && t == Special(LOWER_ACTUATOR) ? (locmod == 1 ? Cargo(TON_ARMLOWER_FIRST + tonmod) : Cargo(TON_LEGLOWER_FIRST + tonmod)) :
	    locmod && t == Special(HAND_OR_FOOT_ACTUATOR) ? (locmod == 1 ? Cargo(TON_ARMHAND_FIRST + tonmod) : Cargo(TON_LEGFOOT_FIRST + tonmod)) :
	    t == Special(ENGINE) && MechSpecials(mech) & XL_TECH ? Cargo(TON_ENGINE_XL_FIRST + tonmod) :
	    t == Special(ENGINE) && MechSpecials(mech) & ICE_TECH ? Cargo(TON_ENGINE_ICE_FIRST + tonmod) :
	    t == Special(ENGINE) && MechSpecials(mech) & CE_TECH ? Cargo(TON_ENGINE_COMP_FIRST + tonmod):
	    t == Special(ENGINE) && MechSpecials(mech) & XXL_TECH ? Cargo(TON_ENGINE_XXL_FIRST + tonmod) :
	    t == Special(ENGINE) && MechSpecials2(mech) & LENG_TECH ? Cargo(TON_ENGINE_LIGHT_FIRST + tonmod) :
	    t == Special(ENGINE) ? Cargo(TON_ENGINE_FIRST + tonmod) :
	    t == Special(HEAT_SINK) && MechSpecials(mech) & (DOUBLE_HEAT_TECH|CLAN_TECH) ? Cargo(DOUBLE_HEAT_SINK) :
	    t == Special(HEAT_SINK) && MechSpecials2(mech) & COMPACT_HEAT_TECH ? Cargo(COMPACT_HEAT_SINK) :
	    t == Special(GYRO) && MechSpecials2(mech) & XLGYRO_TECH ? Cargo(TON_XLGYRO_FIRST + tonmod) :
	    t == Special(GYRO) && MechSpecials2(mech) & HDGYRO_TECH ? Cargo(TON_HDGYRO_FIRST + tonmod) :
	    t == Special(GYRO) && MechSpecials2(mech) & CGYRO_TECH ? Cargo(TON_CGYRO_FIRST + tonmod) :
	    t == Special(GYRO) ? Cargo(TON_GYRO_FIRST + tonmod) :
	    t == Special(SENSORS) ? Cargo(TON_SENSORS_FIRST + tonmod) :
	    t == Special(JUMP_JET) ? Cargo(TON_JUMPJET_FIRST + tonmod) :
	    t);
    } else {
	part = (IsActuator(t) ? Cargo(S_ACTUATOR) :
   	    t == Special(ENGINE) && MechSpecials(mech) & XL_TECH ? Cargo(XL_ENGINE) :
	    t == Special(ENGINE) && MechSpecials(mech) & ICE_TECH ? Cargo(IC_ENGINE) :
	    t == Special(ENGINE) && MechSpecials(mech) & CE_TECH ? Cargo(COMP_ENGINE) :
	    t == Special(ENGINE) && MechSpecials(mech) & XXL_TECH ? Cargo(XXL_ENGINE) :
	    t == Special(ENGINE) && MechSpecials2(mech) & LENG_TECH ? Cargo(LIGHT_ENGINE) :
	    t == Special(HEAT_SINK) && MechSpecials(mech) & (DOUBLE_HEAT_TECH|CLAN_TECH) ? Cargo(DOUBLE_HEAT_SINK) :
	    t == Special(HEAT_SINK) && MechSpecials2(mech) & COMPACT_HEAT_TECH ? Cargo(COMPACT_HEAT_SINK) :
	    t == Special(GYRO) && MechSpecials2(mech) & XLGYRO_TECH ? Cargo(XL_GYRO) :
	    t == Special(GYRO) && MechSpecials2(mech) & HDGYRO_TECH ? Cargo(HD_GYRO) :
	    t == Special(GYRO) && MechSpecials2(mech) & CGYRO_TECH ? Cargo(COMP_GYRO) :
	    t);
    }
return part;
}

int ProperMyomer(MECH * mech)
{
int part;

        part = (MechSpecials(mech) & TRIPLE_MYOMER_TECH ? TON_TRIPLEMYOMER_FIRST : TON_MYOMER_FIRST);
        part += GetPartMod(mech, 0);

return Cargo(part);
}

int ReloadTime(MECH * mech, int loc, int crit, int dir)
{
float mul;
int dif, full;

full = FullAmmo(mech, loc, crit);
dif = (dir ? full - GetPartData(mech, loc, crit) : GetPartData(mech, loc, crit));
mul = ((float) ((float) dif / (float) full));

return ((int) mul * RELOAD_TIME);
}

unsigned long long int GetPartCost(int p)
{
extern unsigned long long int specialcost[SPECIALCOST_SIZE];
extern unsigned long long int ammocost[AMMOCOST_SIZE];
extern unsigned long long int weapcost[WEAPCOST_SIZE];
extern unsigned long long int cargocost[CARGOCOST_SIZE];
extern unsigned long long int bombcost[BOMBCOST_SIZE];
                                                                                                                                                                                      
if (IsWeapon(p))
    return weapcost[Weapon2I(p)];
else if (IsAmmo(p))
    return ammocost[Ammo2I(p)];
else if (IsSpecial(p))
    return specialcost[Special2I(p)];
else if (IsBomb(p))
    return bombcost[Bomb2I(p)];
else if (IsCargo(p))
    return cargocost[Cargo2I(p)];
else
    return 0;
}

#define COST_DEBUG	0
#if COST_DEBUG
#define ADDPRICE(desc, add) \
    { SendDebug(tprintf("AddPrice - %s %d", desc, add)); \
    total += add; }
#else
#define ADDPRICE(desc, add) \
    total += add;
#endif

#define DoArmMath(loc) \
for (i = 0; i < NUM_CRITICALS; i++) { \
    part = GetPartType(mech, loc, i); \
    if (!IsActuator(part)) \
	continue; \
    else if (Special2I(part) == SHOULDER_OR_HIP || Special2I(part) == UPPER_ACTUATOR) \
        ADDPRICE("Shoulder/Upper Actuator", (MechTons(mech) * 100)) \
    else if (Special2I(part) == LOWER_ACTUATOR) \
        ADDPRICE("LowerArm Actuator", (MechTons(mech) * 50)) \
    else if (Special2I(part) == HAND_OR_FOOT_ACTUATOR) \
        ADDPRICE("Hand Actuator", (MechTons(mech) * 80)) \
    }

#define DoLegMath(loc) \
for (i = 0; i < NUM_CRITICALS; i++) { \
    part = GetPartType(mech, loc, i); \
    if (!IsActuator(part)) \
	continue; \
    else if (Special2I(part) == SHOULDER_OR_HIP || Special2I(part) == UPPER_ACTUATOR) \
        ADDPRICE("Hip/Upper Actuator", (MechTons(mech) * 150)) \
    else if (Special2I(part) == LOWER_ACTUATOR) \
        ADDPRICE("LowerLeg Actuator", (MechTons(mech) * 80)) \
    else if (Special2I(part) == HAND_OR_FOOT_ACTUATOR) \
        ADDPRICE("Foot Actuator", (MechTons(mech) * 120)) \
    }

/* Some would say it's better for scode. I prolly would do. But since it's the FASA cals, let's put it in binary. Plus I'm lazy today */
unsigned long long int CalcFasaCost(MECH * mech)
{
int ii, i, part;
unsigned long long int total = 0;
float mod = 1.0;

if (!mech)
    return -1;

if (!(MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VEH_NAVAL || MechType(mech) == CLASS_VTOL ) ||
	is_aero(mech) || IsDS(mech))
    return 0;

if (MechType(mech) == CLASS_MECH) {
/* Cockpit */
    if (MechSpecials2(mech) & SMALLCOCKPIT_TECH)
	ADDPRICE("SmallCockpit", 175000)
    else if (MechSpecials2(mech) & TORSOCOCKPIT_TECH)
	ADDPRICE("TorsoCockpit", 750000)
    else
	ADDPRICE("Cockpit", 200000)
/* Life Support */
    ADDPRICE("LifeSupport", 50000)
/* Sensors */
    ADDPRICE("Sensors", (MechTons(mech) * 2000))
/* Myomer stuffage */
    if (MechSpecials(mech) & TRIPLE_MYOMER_TECH)
	ADDPRICE("TripleMyomer", (MechTons(mech) * 16000))
    else
	ADDPRICE("Myomer", (MechTons(mech) * 2000))
/* Internal Structure */
    if (MechSpecials(mech) & ES_TECH || MechSpecials(mech) & COMPI_TECH)
	ADDPRICE("ES/CO  Internal", (MechTons(mech) * 1600))
    else if (MechSpecials(mech) & REINFI_TECH)
	ADDPRICE("RE Internal", (MechTons(mech) * 6400))
    else
	ADDPRICE("Internal", (MechTons(mech) * 400))
/* Actuators */
    DoArmMath(RARM)
    DoArmMath(LARM)
    DoLegMath(LLEG)
    DoLegMath(RLEG)
/* Gyro */
    i = MechEngineSize(mech);
    if (i % 100)
	i += (100 - (MechEngineSize(mech) % 100));
    i /= 100;

    if (MechSpecials2(mech) & XLGYRO_TECH)
	ADDPRICE("XLGyro", i * 750000)
    else if (MechSpecials2(mech) & CGYRO_TECH)
	ADDPRICE("Compact Gyro", i * 400000)
    else if (MechSpecials2(mech) & HDGYRO_TECH)
	ADDPRICE("HD Gyro", i * 500000)
    else
	ADDPRICE("Gyro", i * 300000)
} else {
    int pamp = 0, turret = 0;
    for (i = 0; i < NUM_SECTIONS; i++)
	for (ii = 0; ii < NUM_CRITICALS; ii++) {
	    if (!(part = GetPartType(mech, i, ii)))
		continue;
	    if (!IsWeapon(part))
		continue;
	    if (i == TURRET)
		turret += crit_weight(mech, part);
	    if (IsEnergy(part))
		pamp += crit_weight(mech, part);
	    }
/* Internals */
    ADDPRICE("TankInternals", MechTons(mech) * 10000)
/* Control Components */
    ADDPRICE("Control Components", (float) 10000 * (float) ((float) 0.05 * (float) MechTons(mech)))
/* Power Amp */
    if (MechSpecials(mech) & ICE_TECH)
	ADDPRICE("Power Amplifiers",  20000 * (float) (((float) pamp / (float) 10) / (float) 1024))
/* Turret */
    ADDPRICE("Turret", (float) 5000 * (float) (((float) turret / (float) 10) / (float) 1024))
/* Lift/Dive Equip */
    if (MechMove(mech) == MOVE_HOVER || MechMove(mech) == MOVE_FOIL || MechMove(mech) == MOVE_SUB)
	ADDPRICE("Lift/Dive Equipment", (float) 20000 * (float) ((float) 0.1 * (float) MechTons(mech)))
    if (MechMove(mech) == MOVE_VTOL)
	ADDPRICE("VTOL Equipment", (float) 40000 * (float) ((float) 0.1 * (float) MechTons(mech)))
}
/* Engine Math */
i = (MechSpecials(mech) & CE_TECH ? 10000 : MechSpecials2(mech) & LENG_TECH ? 15000 : MechSpecials(mech) & XL_TECH ? 20000 :
	MechSpecials(mech) & XXL_TECH ? 100000 : MechSpecials(mech) & ICE_TECH ? 1250 : 5000);
ADDPRICE("Engine", ((i * MechEngineSize(mech) * MechTons(mech)) / 75))

/* Jump Jets */
i = MechJumpSpeed(mech) * MP_PER_KPH;
if (i > 0)
    ADDPRICE("JumpJets", MechTons(mech) * (i * i) * 200)

/* Heat Sinks */
i = MechRealNumsinks(mech);
ii = (MechSpecials(mech) & DOUBLE_HEAT_TECH || MechSpecials(mech) & CLAN_TECH ? 6000 : MechSpecials2(mech) & COMPACT_HEAT_TECH ? 3000 : 2000);
if (!(MechSpecials(mech) & ICE_TECH || MechSpecials(mech) & DOUBLE_HEAT_TECH || MechSpecials(mech) & CLAN_TECH))
	i = BOUNDED(0, i - 10, 500);
ADDPRICE("Heat Sinks",  i * ii)

ii = 0;
for (i = 0; i < NUM_SECTIONS; ++i)
    ii += GetSectOArmor(mech, i);
i = (MechSpecials(mech) & FF_TECH ? 20000 : MechSpecials(mech) & HARDA_TECH ? 15000 : MechSpecials2(mech) & LFF_TECH ? 15000 :
	MechSpecials2(mech) & HFF_TECH ? 25000 : 10000);
#if COST_DEBUG
SendDebug(tprintf("Armor Total %d - Armor Cost %d", ii, i));
#endif
ADDPRICE("Armor", (i / 16) * ii)

/* Parts */
for (i = 0; i < NUM_SECTIONS; i++)
    for (ii = 0; ii < NUM_CRITICALS; ii++) {
	part = GetPartType(mech, i, ii); 
	if (IsActuator(part) || part == EMPTY)
	    continue;
	if (IsSpecial(part))
	    switch (Special2I(part)) {
		case LIFE_SUPPORT:
		case SENSORS:
		case COCKPIT:
		case ENGINE:
		case GYRO:
		case HEAT_SINK:
		case JUMP_JET:
		case FERRO_FIBROUS:
		case ENDO_STEEL:
		case TRIPLE_STRENGTH_MYOMER:
		case HARDPOINT:
		    continue;
		default:
		    break;
		} 
	if (IsAmmo(part)) {
	    part = FindAmmoType(mech, i, ii);
	    ADDPRICE(part_name(part, 0), GetPartCost(part) * GetPartData(mech, i, ii));
	} else {
	    ADDPRICE(part_name(part, 0), GetPartCost(part))
	}
	}

if (MechType(mech) != CLASS_MECH) {
    switch (MechMove(mech)) {
	case MOVE_TRACK:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 100);
	    break;
	case MOVE_WHEEL:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 200);
	    break;
	case MOVE_HOVER:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 50);
	    break;
	case MOVE_VTOL:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 30);
	    break;
	case MOVE_HULL:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 200);
	    break;
	case MOVE_FOIL:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 75);
	    break;
	case MOVE_SUB:
	    mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 50);
	    break;
	} 
    } else {
	mod = (float) 1 + (float) ((float) MechTons(mech) / (float) 100);
    }

#if COST_DEBUG
SendDebug(tprintf("Price Total - %lld Mod - %f", total, mod));
#endif

return ((float) total * (float) mod);
}
