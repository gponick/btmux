#define MIN_TAKEOFF_SPEED 3

#include <math.h>
#include "mech.h"
#include "mech.events.h"
#include "p.mech.sensor.h"
#include "p.mech.update.h"
#include "p.artillery.h"
#include "p.mech.combat.h"
#include "p.mech.utils.h"
#include "p.econ_cmds.h"
#include "p.mech.ecm.h"
#include "p.mech.lite.h"
#include "spath.h"
#include "p.mine.h"
#include "floatsim.h"

struct land_data_type {
    int type;
    int maxvertup;
    int maxvertdown;
    int minhoriz;
    int maxhoriz;
    int launchvert;
    int launchtime;		/* In secs */
    char *landmsg;
    char *landmsg_others;
    char *takeoff;
    char *takeoff_others;
}				/*            maxvertup / maxvertdown / minhoriz / maxhoriz / launchv / launchtime */

#define ISAEROFIGHTER(a) (MechType(a) == CLASS_AERO) 
#define NUM_LAND_TYPES (sizeof(land_data)/sizeof(struct land_data_type))
#define HORIZ_TAKEOFF(a) (MechType(a) == CLASS_DS || MechType(a) == CLASS_AERO)
#define DYNAMIC_LAUNCHTIME(a, b) (HORIZ_TAKEOFF(a) ?  land_data[b].launchtime * (6 / (MechSpeed(a) / MP1)) : land_data[b].launchtime * (MechStatus2(a) & CS_ON_LAND ? 3 : 1))
#define DYNAMIC_LANDTIME(a, b) (HORIZ_TAKEOFF(a) ?  land_data[b].launchtime * ((ISAEROFIGHTER(a) ? 3 : 6) / (MechSpeed(a) / MP1)) : land_data[b].launchtime)
#define MP_PER_VELOCITY 8
#define KPH_PER_VELOCITY(a) (MechZ(a) >= ORBIT_Z ? MP1 : MechZ(a) >= ATMO_Z ? (MP_PER_VELOCITY * KPH_PER_MP) / 2 : MP_PER_VELOCITY * KPH_PER_MP)
#define THRUST_PER_HEADING(a) (ceil(fabsf(MechVelocity(a) / KPH_PER_VELOCITY(a))))
#define THRUST_PER_TIC(a) ((MMaxSpeed(a) * 3) / TURN)
#define SPEED_STALL(mech) ((float) (KPH_PER_VELOCITY(mech) * (MechZ(mech) >= ORBIT_Z ? 0 : 2)))
#define SPEED_OVER(mech)  ((float) (((KPH_PER_VELOCITY(mech) * (MMaxSpeed(mech) / MP1)) * 2)))
#define SPEED_ATMO(mech)	 ((float) (MechZ(mech) >= ORBIT_Z ? 0 : MechZ(mech) > ATMO_Z ? 22 : 44))
#define SI_SAFE_THRUST(mech)	(AeroSI(mech) * MP1)
#define SAFE_THRUST(mech)	((SI_SAFE_THRUST(mech) < WalkingSpeed(MMaxSpeed(mech))) ? SI_SAFE_THRUST(mech) : WalkingSpeed(MMaxSpeed(mech)))

land_data[] = {
    {
	CLASS_VTOL, 10, -60, -15, 15, 5, 0,
	    "You bring your VTOL to a safe landing.", "lands.",
	    "The rotor whines overhead as you lift off into the sky.",
	    "takes off!"}, {
	CLASS_AERO, 10, -30, MIN_TAKEOFF_SPEED * MP1, MP_PER_VELOCITY * KPH_PER_MP * 2, 20, 60,
	    "You land your AeroFighter safely.", "lands safely.",
	    "The Aerofighter launches into the air!",
	    "launches into the air!"}, {
	CLASS_DS, 10, -30, MIN_TAKEOFF_SPEED * MP1, MP_PER_VELOCITY * KPH_PER_MP * 2, 20, 60,
	    "The DropShip lands safely.", "lands safely.",
	    "The DropShip's nose lurches upward, and it starts climbing to the sky!",
	    "starts climbing to the sky!"}, {
	CLASS_SPHEROID_DS, 15, -40, -40, 40, 20, 30,
	    "The DropShip touches down safely.",
	    "touches down, and settles.",
	    "The DropShip slowly lurches upwards as engines battle the gravity..",
	    "starts climbing up to the sky!"}
};

static void aero_takeoff_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    MAP *map = getMap(mech->mapindex);
    int i = -1;
    int count = (int) e->data2;

    if (IsDS(mech))
	for (i = 0; i < NUM_LAND_TYPES; i++)
	    if (MechType(mech) == land_data[i].type)
		break;

    if (MechStatus2(mech) & CS_ON_LAND && MechStatus(mech) & COMBAT_SAFE)
	MechStatus(mech) &= ~(COMBAT_SAFE);

    if (count > 0) {
	if (count > 5) {
	    if (!(count % 10))
		mech_notify(mech, MECHALL, tprintf("Launch countdown: %d.",
			count));
	} else
	    mech_notify(mech, MECHALL, tprintf("Launch countdown: %d.",
		    count));
	if (i >= 0) {
	    if (count == (DYNAMIC_LAUNCHTIME(mech, i) / 4))
		DSSpam_O(mech,
		    "'s engines start to glow with unbearable intensity..");
	    switch (count) {
	    case 10:
		DSSpam_O(mech, "'s engines are almost ready to lift off!");
		break;
	    case 6:
		DSSpam_O(mech,
		    "'s engines generate a tremendous heat wave!");
		ScrambleInfraAndLiteAmp(mech, 2, 0,
		    "The blinding flash of light momentarily blinds you!",
		    "The blinding flash of light momentarily blinds you!");
		break;
	    case 2:
		mech_notify(mech, MECHALL,
		    "The engines pulse out a stream of superheated plasma!");
		DSSpam_O(mech,
		    "'s engines send forth a tremendous stream of superheated plasma!");
		ScrambleInfraAndLiteAmp(mech, 4, 0,
		    "The blinding flash of light blinds you!",
		    "The blinding flash of light blinds you!");
		break;
	    case 1:
		DS_BlastNearbyMechsAndTrees(mech,
		    "You receive a direct hit!",
		    "is caught in the middle of the inferno!",
		    "You are hit by the wave!", "gets hit by the wave!",
		    "are instantly burned to ash!", 400);
		break;
	    }
	}
	MECHEVENT(mech, EVENT_TAKEOFF, aero_takeoff_event, 1, (void *) (count - 1));
	return;
    }
    if (i < 0) {
	if (RollingT(mech) && MechSpeed(mech) < (MIN_TAKEOFF_SPEED * MP1)) {
	    mech_notify(mech, MECHALL,
		"You're moving too slowly to lift off!");
	    return;
	}
	for (i = 0; i < NUM_LAND_TYPES; i++)
	    if (MechType(mech) == land_data[i].type)
		break;
    }

    StopSpinning(mech);
    mech_notify(mech, MECHALL, land_data[i].takeoff);
    MechLOSBroadcast(mech, land_data[i].takeoff_others);
    MechStartFX(mech) = 0;
    MechStartFY(mech) = 0;
    MechStartFZ(mech) = 0;
    if (IsDS(mech))
	SendDebug(tprintf ("DS Takeoff: #%d has lifted off at %d %d on map #%d",
		mech->mynum, MechX(mech), MechY(mech), map->mynum)); 
   if (MechCritStatus(mech) & HIDDEN)
        {
        mech_notify(mech, MECHALL, "You move too much and break your cover!");
        MechLOSBroadcast(mech, "breaks its cover in the brush.");
        MechCritStatus(mech) &= ~(HIDDEN);
        }
    if (MechType(mech) != CLASS_VEH_VTOL) {
	MechVelocity(mech) = (SpheroidDS(mech) ? 2 : 3) * KPH_PER_VELOCITY(mech);
	MechDesiredThrust(mech) = (SpheroidDS(mech) ? 18.75 : 52.5);
	MechThrust(mech) = MechDesiredThrust(mech);
	MechDesiredAngle(mech) = (SpheroidDS(mech) ? 90 : 20);
	MechAngle(mech) = MechDesiredAngle(mech);
	LastSpinRoll(mech) = event_tick;
    } else {
	MechSpeed(mech) = 0;
	MechDesiredSpeed(mech) = 0;
	MechVerticalSpeed(mech) = 0.0;
    }
    if (MechRTerrain(mech) == LIGHT_FOREST || MechRTerrain(mech) == HEAVY_FOREST)
	MechZ(mech) += 3;
    ContinueFlying(mech);
    MaybeMove(mech);
}

static void aero_landing_event(EVENT * e)
{
MECH *mech = (MECH *) e->data;

    mech_notify(mech, MECHALL, "You complete your landing maneuvers.");
    MechLOSBroadcast(mech, "completes it's landing maneuvers.");
    MechSpeed(mech) = 0;
    MechDesiredSpeed(mech) = 0;
    MechAngle(mech) = 0;
    return;
}


void aero_takeoff(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *map = getMap(mech->mapindex);
    int i, j;

    for (i = 0; i < NUM_LAND_TYPES; i++)
	if (MechType(mech) == land_data[i].type)
	    break;
    if ((j = atoi(buffer)))
	DOCHECK(!WizP(player), "Insufficient access!");
    DOCHECK(TakingOff(mech),
	"The launch sequence has already been initiated!");
    DOCHECK(i == NUM_LAND_TYPES, "This vehicle type cannot takeoff!");
    cch(MECH_USUALO);
    DOCHECK(!FlyingT(mech),
	"Only VTOL, Aerospace fighters and Dropships can take off.");
    DOCHECK(!Landed(mech), "You haven't landed!");
    if (Fallen(mech)) {
	DOCHECK(MechType(mech) == CLASS_VTOL, "The rotor's dead!");
	notify(player, "The engines are dead!");
	return;
    }
    if (AeroFuel(mech) < 1) {
	DOCHECK(MechType(mech) == CLASS_VTOL, "Your VTOL's out of fuel!");
	notify(player,
	    "Your craft's out of fuel! No taking off until it's refueled.");
	return;
    }
    DOCHECK(HORIZ_TAKEOFF(mech) &&
	MechSpeed(mech) < (MIN_TAKEOFF_SPEED * MP1),
	"You're moving too slowly to take off!");
    DOCHECK(MapIsUnderground(map),
	"You cannot take off here without smashing into the ceiling."); 
    if (DYNAMIC_LAUNCHTIME(mech, i) > 0)
	mech_notify(mech, MECHALL,
	    "Launch sequence initiated.. type 'land' to abort it.");
    DSSpam(mech, "starts warming engines for liftoff!");
    if (IsDS(mech))
        SendDebug(tprintf ("DS Takeoff: #%d has started takeoff at %d %d on map #%d",
	    mech->mynum, MechX(mech), MechY(mech), map->mynum));
    if (MechCritStatus(mech) & HIDDEN) {
        mech_notify(mech, MECHALL, "You break your cover to takeoff!");
        MechLOSBroadcast(mech, "breaks its cover as it begins takeoff.");
        MechCritStatus(mech) &= ~(HIDDEN);
        }
    StopHiding(mech);
    if (!j)
	j = DYNAMIC_LAUNCHTIME(mech, i);
    MECHEVENT(mech, EVENT_TAKEOFF, aero_takeoff_event, 1, (void *) j);
}

#define NUM_NEIGHBORS 7

void DS_BlastNearbyMechsAndTrees(MECH * mech, char *hitmsg, char *hitmsg1,
    char *nearhitmsg, char *nearhitmsg1, char *treehitmsg, int damage)
{
    MAP *map = getMap(mech->mapindex);
    int x = MechX(mech), y = MechY(mech), z = MechZ(mech);
    int x1, y1, x2, y2, d;
    int rng = 2;

    for (x1 = x - rng; x1 <= (x + rng); x1++)
	for (y1 = y - rng; y1 <= (y + rng); y1++) {
	    x2 = BOUNDED(0, x1, map->map_width - 1);
	    y2 = BOUNDED(0, y1, map->map_height - 1);
	    if (x1 != x2 || y1 != y2)
		continue;
	    if ((d = MyHexDist(x, y, x1, y1, 0)) > rng)
		continue;
	    d = MAX(1, d);
	    switch (GetRTerrain(map, x1, y1)) {
	    case LIGHT_FOREST:
	    case HEAVY_FOREST:
		if (!find_decorations(map, x1, y1)) {
		    HexLOSBroadcast(map, x1, y1,
			tprintf("%%ch%%crThe trees in $h %s%%cn",
			    treehitmsg));
		    if ((damage / d) > 100) {
			SetTerrain(map, x1, y1, ROUGH);
		    } else {
			add_decoration(map, x1, y1, TYPE_FIRE, FIRE,
			    FIRE_DURATION);
		    }
		}
		break;
	    case SNOW: 
		SetTerrain(map, x1, y1, GRASSLAND);
		HexLOSBroadcast(map, x1, y1, tprintf("%%ch%%cwThe snow in $h melts away!%%cn"));
                break;
	    case ICE: 
		SetTerrain(map, x1, y1, WATER);
		HexLOSBroadcast(map, x1, y1, tprintf("%%ch%%cwThe ice in $h melts away!%%cn"));
                break;

	    }
	}
    MechZ(mech) = z + 6;
/*
    blast_hit_hexesf(map, damage, 5, damage / 2, MechFX(mech),
	MechFY(mech), MechFX(mech), MechFY(mech), hitmsg, hitmsg1,
	nearhitmsg, nearhitmsg1, 0, 4, 4, 1, rng, NULL, NULL);
    MechZ(mech) = z;
*/
}

void aero_land(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data, *target;
    MAP *map = getMap(mech->mapindex);
    int i, t, fld, frnd = 0, foe = 0;
    double horiz = 0.0;
    int vert, vertmin = 0, vertmax = 0, mof, mods = 0;
    
    DOCHECK(MechType(mech) != CLASS_VEH_VTOL &&
	MechType(mech) != CLASS_AERO &&
	!IsDS(mech), "You can't land this type of vehicle.");
    DOCHECK(MechType(mech) == CLASS_VTOL &&
	AeroFuel(mech) <= 0, "You lack fuel to maneuver for landing!");

    for (i = 0; i < NUM_LAND_TYPES; i++)
	if (MechType(mech) == land_data[i].type)
	    break;
    if (i == NUM_LAND_TYPES)
	return;
    DOCHECK((Fallen(mech)) &&
	(MechType(mech) == CLASS_VTOL), "The rotor's dead!");
    DOCHECK((Fallen(mech)) &&
	(MechType(mech) != CLASS_VTOL), "The engines are dead!");
    if (MechStatus(mech) & LANDED) {
	if (TakingOff(mech)) {
	    mech_notify(mech, MECHALL, tprintf("Launch aborted by %s.",
		    Name(player)));
    	    MechLOSBroadcast(mech, "aborts it's takeoff attempt!");
	    if (IsDS(mech))
		SendDebug(tprintf ("DS Takeoff: #%d aborted takeoff at %d %d on map #%d",
			mech->mynum, MechX(mech), MechY(mech), map->mynum));
	    StopTakeOff(mech); 
	    if (MechStatus2(mech) & CS_ON_LAND && !(MechStatus(mech) & COMBAT_SAFE))
		MechStatus(mech) |= COMBAT_SAFE;
	    return;
	}
	notify(player, "You're already landed!");
	return;
    }
    DOCHECK(MechZ(mech) > MechElevation(mech) + 1,
	"You are too high to land here.");
    if (HORIZ_TAKEOFF(mech)) {
	DOCHECK(MechVelocity(mech) > land_data[i].maxhoriz,
	    "You're moving too fast to land!"); 
	DOCHECK(MechVelocity(mech) < land_data[i].minhoriz,
	    "You're moving too slow to land!");
	DOCHECK(MechAngle(mech) < -30,
	    "Your diving to steep and cannot land.");
    } else { 
	DOCHECK(((horiz = my_sqrtm((double) MechSpeed(mech), (double) MechVerticalSpeed(mech))) >= ((double) 1.0 + land_data[i].maxhoriz)),
	    "You're moving too fast to land.");
	DOCHECK(horiz < land_data[i].minhoriz,
	    "You're moving too slowly to land.");
	DOCHECK(((vert = MechVerticalSpeed(mech)) > (vertmax = land_data[i].maxvertup)) || (MechVerticalSpeed(mech) < (vertmin = land_data[i].maxvertdown)),
	    "You are moving too fast to land. ");
	if (MechSpeed(mech) < land_data[i].minhoriz) {
	    if (MechStartFZ(mech) <= 0)
		notify(player, "You're falling, not landing! Pick up some horizontal speed first.");
	    else
		notify(player, "You're climbing not landing!");
	    return;
	    }
	}

    t = MechRTerrain(mech);
    DOCHECK(MechType(mech) == CLASS_VTOL && !(t == GRASSLAND || t == ROAD || t == BUILDING || t == SNOW), "You can't land on this type of terrain.");
 
    if (MechType(mech) != CLASS_VTOL && MechType(mech) != CLASS_AERO && ImproperLZ(mech, MechStatus2(mech) & CS_ON_LAND ? 1 : 0)) {
	mech_notify(mech, MECHALL,
	    "This location is no good for landing!");
	return;
    }

    if (MechType(mech) != CLASS_VTOL) {
/* Terrain Mods */
	if (MechRTerrain(mech) == WATER || MechRTerrain(mech) == ROUGH || MechRTerrain(mech) == BUILDING)
	    mods += 3;
	else if (MechRTerrain(mech) == LIGHT_FOREST)
	    mods += 4;
	else if (MechRTerrain(mech) == HEAVY_FOREST)
	    mods += 5;
	else if (MechRTerrain(mech) == MOUNTAINS)
	    mods += 6;
/* Spheroid Vertical Land Exception */
	if (mods && MechType(mech) == CLASS_SPHEROID_DS)
	    mods /= 2;
/* Status stuff */
	if (AeroCritStatus(mech) & GEAR_DAMAGED)
	    mods += 5;
        if (MMaxSpeed(mech) <= MechOMaxSpeed(mech) / 2)
            mods += 2;
        if (MMaxSpeed(mech) <= 0) {
            if (MechType(mech) == CLASS_SPHEROID_DS)
                mods += 8;
            else
                mods += 4;
	    }
/* A quickie hack for the 'Friendly Airfield' mods */
	for (fld = 0; fld < map->first_free; fld++) {
	    if (map->mechsOnMap[fld] >= 0) {
		if (!(target = getMech(map->mechsOnMap[fld])))
		    continue;
		if (mech->mynum == target->mynum)
		    continue;
		if (!In_Character(target->mynum) || Destroyed(target) || !Started(target) || HiddenMech(target))
		    continue;
		if (FaMechRange(mech, target) > 20)
		    continue;
		if (MechTeam(mech) == MechTeam(target))
		   frnd++;
		if (MechTeam(mech) != MechTeam(target))
		    foe++;
		}
	    }
	if (foe && frnd == 0)
	    mods += 1;
	else if (frnd == 1 && foe == 0)
	    mods -= 1;
	else if (frnd > 1 && foe == 0)
	    mods -= 2;

	if (Spinning(mech))
	    mof = 10;
	else 
	    mof = MoFMadePilotSkillRoll(mech, mods);

	if (mof < 0) {
	    mech_notify(mech, MECHALL, "You fail your attempt to land!");
	    DamageMech(mech, mech, 0, -1, (IsDS(mech) ? DS_NOSE : AERO_NOSE), 0, 0, abs(mof) * 10, 0, -1, 0, 0);
	    } else {
		mech_notify(mech, MECHALL, "You land successfully!");
	    }
	}
    
    if (IsDS(mech))
	SendDebug(tprintf("DS Landing: #%d has landed at %d %d on map #%d",
		mech->mynum, MechX(mech), MechY(mech), map->mynum));

    if (HORIZ_TAKEOFF(mech))
	MECHEVENT(mech, EVENT_LANDING, aero_landing_event, DYNAMIC_LANDTIME(mech, i), 0);

    mech_notify(mech, MECHALL, land_data[i].landmsg);
    MechLOSBroadcast(mech, land_data[i].landmsg_others);
    MechZ(mech) = MechElevation(mech);
    MechFZ(mech) = ZSCALE * MechZ(mech);
    {
    MECH *car;
    if (MechCarrying(mech) > 0) {
	car = getMech(MechCarrying(mech));
	MechZ(car) = MechElevation(mech);
	MechFZ(car) = ZSCALE * MechZ(car);
	}
    }
    MechStatus(mech) |= LANDED;
    MechVerticalSpeed(mech) = 0.0;
    MechThrust(mech) = 0.0;
    MechSpeed(mech) = (HORIZ_TAKEOFF(mech) ? MechVelocity(mech)  : 0);
    MechDesiredSpeed(mech) = MechSpeed(mech);
    MechVelocity(mech) = 0.0;
    if (HORIZ_TAKEOFF(mech)) {
	MechDesiredAngle(mech) = 0;
	MechAngle(mech) = 0;
	} else {
	    MechDesiredAngle(mech) = 90;
	    MechAngle(mech) = 90; 
	}
    MechVerticalSpeed(mech) = 0.0;
    MechStartFX(mech) = 0.0;
    MechStartFY(mech) = 0.0;
    MechStartFZ(mech) = 0.0;
    if (MechStatus2(mech) & CS_ON_LAND)
	MechStatus(mech) |= COMBAT_SAFE;	
    possible_mine_poof(mech, MINE_LAND);
}

void aero_ControlEffect(MECH * mech)
{
    if (Spinning(mech))
	return;
    if (Destroyed(mech))
	return;
    if (Landed(mech))
	return;
    mech_notify(mech, MECHALL, "You lose control of your craft!");
    MechLOSBroadcast(mech, "spins out of control!");
    LastSpinRoll(mech) = event_tick;
    StartSpinning(mech);
/*
    MechStartSpin(mech) = event_tick;
*/
}

void ds_BridgeHit(MECH * mech)
{
    /* Implementation: Kill all players on bridge :-) */
    if (Destroyed(mech))
	return;
    if (In_Character(mech->mynum))
	mech_notify(mech, MECHALL,
	    "SHIT! The shot seems to be coming straight for the bridge!");
    KillMechContentsIfIC(mech->mynum, mech);
}

#define degsin(a) ((double) sin((double) (a) * M_PI / 180.0))
#define degcos(a) ((double) cos((double) (a) * M_PI / 180.0))

double my_sqrt(double x, double y)
{
    if (x < 0)
	x = -x;
    if (y < 0)
	y = -y;
    return sqrt(x * x + y * y);
}

double my_sqrtm(double x, double y)
{
    double d;

    if (x < 0)
	x = -x;
    if (y < 0)
	y = -y;
    if (y > x) {
	d = y;
	y = x;
	x = d;
    }
    return sqrt(x * x - y * y);
}

#define AERO_BONUS 3

void aero_UpdateSpeed_old(MECH * mech)
{
    float xypart;
    float wx, wy, wz;
    float nx, ny, nz;
    float nh;
    float dx, dy, dz;
    float vlen, mod;
    float sp;
    float ab = 0.7;
    float m = 1.0;
    int angle;

    if (MechZ(mech) < ATMO_Z && SpheroidDS(mech))
	if (MechAngle(mech) < 0)
	    angle = -90;
	else
	    angle = 90;
    else
	angle = MechAngle(mech);

    wz = MechDesiredThrust(mech) * degsin(angle);
    if (MechType(mech) == CLASS_AERO) 
	ab = 2.5;
    if (MechZ(mech) < ATMO_Z)
	ab = ab / 2;

/* Tweak Throttle */
if (MechDesiredThrust(mech) != MechThrust(mech)) {
    if (fabsf(MechDesiredThrust(mech) - MechThrust(mech)) <= THRUST_PER_TIC(mech))
        MechThrust(mech) = MechDesiredThrust(mech);
    else if (MechDesiredThrust(mech) > MechThrust(mech))
        MechThrust(mech) += THRUST_PER_TIC(mech);
    else if (MechDesiredThrust(mech) < MechThrust(mech))
        MechThrust(mech) -= THRUST_PER_TIC(mech);
    } 

    /* First, we calculate the vector we want to be going */
    xypart = MechThrust(mech) * degcos(angle);
    if (AeroFuel(mech) < 0) {
	wz = wz / 5.0;
	xypart = xypart / 5.0;
    }
    if (xypart < 0)
	xypart = 0 - xypart;
    m = ACCEL_MOD;
    FindComponents(m * xypart, MechDesiredFacing(mech), &wx, &wy);
    wz = wz * m;

    /* Then, we calculate the present heading / speed */
    nx = MechStartFX(mech);
    ny = MechStartFY(mech);
    nz = MechStartFZ(mech);

    /* Ok, we've present heading / speed */
    /* Next, we make vector from n[xyz] -> w[xyz] */
    dx = wx - nx;
    dy = wy - ny;
    dz = wz - nz;
    vlen = my_sqrt(my_sqrt(dx, dy), dz);
    if (!(vlen > 0.0))
	return;
    if (vlen > (m * ab * MMaxSpeed(mech) / AERO_SECS_THRUST)) {
	mod = (float) ab *m * MMaxSpeed(mech) / AERO_SECS_THRUST / vlen;

	dx *= mod;
	dy *= mod;
	dz *= mod;
	/* Ok.. we've a new modified speed vector */
    }
    nx += dx;
    ny += dy;
    nz += dz;
    /* Then, we need to calculate present heading / speed / verticalspeed */
    nh = (float) atan2(ny, nx) / TWOPIOVER360;
    if (!SpheroidDS(mech))
	SetFacing(mech, AcceptableDegree((int) nh + 90));
    xypart = my_sqrt(nx, ny);
    MechSpeed(mech) = xypart;
    MechVelocity(mech) = my_sqrt(my_sqrt(nx, ny), nz);	/* Whole speed */
    MechVerticalSpeed(mech) = nz;
    if (!SpheroidDS(mech) && fabs(MechSpeed(mech)) < MP1)
	SetFacing(mech, MechDesiredFacing(mech));
    MechStartFX(mech) = nx;
    MechStartFY(mech) = ny;
    MechStartFZ(mech) = nz;
}

void aero_UpdateSpeed(MECH * mech)
{
float dx, dy, dz, xypart, mod, ab = 2.5, diff, nthrust, maxspeed, funcspeed;
int pol;
	
maxspeed = SAFE_THRUST(mech);
funcspeed = (MechOT(mech) ? MMaxSpeed(mech) : maxspeed);

if (MechCruise(mech) && MechVelocity(mech) != MechJumpSpeed(mech)) {
    diff = (MechJumpSpeed(mech) - MechVelocity(mech));
    if (abs(diff) > (MechZ(mech) >= ORBIT_Z ? 1 : 5) || MechJumpSpeed(mech) == 0.0) {
	if (diff < 22 && diff > 0)
	    diff = 22.0;
	else if (diff >= 22)
	    diff = funcspeed;
	else if (diff > -22 && diff < 0)
	    diff = -22.0;
	else if (diff <= -22)
	    diff = -funcspeed;
 	nthrust = SPEED_ATMO(mech) + diff;
	if (MechVelocity(mech) > 0 ? nthrust > funcspeed : nthrust < -funcspeed)
	    nthrust = MechVelocity(mech) > 0 ? funcspeed : -funcspeed;
	if (abs(MechDesiredThrust(mech) - nthrust) > 5)
 	    aero_thrust(MechPilot(mech), mech, tprintf("%.2f", nthrust)); 
    } else {
	if (MechDesiredThrust(mech) != SPEED_ATMO(mech))
	    aero_thrust(MechPilot(mech), mech, tprintf("atmo"));
    }
}

/* Tweak Throttle */
if (MechDesiredThrust(mech) != MechThrust(mech)) {
    nthrust = MechThrust(mech);
    if (fabsf(MechDesiredThrust(mech) - MechThrust(mech)) <= THRUST_PER_TIC(mech))
	MechThrust(mech) = MechDesiredThrust(mech);
    else if (MechDesiredThrust(mech) > MechThrust(mech))
	MechThrust(mech) += THRUST_PER_TIC(mech);
    else if (MechDesiredThrust(mech) < MechThrust(mech))
	MechThrust(mech) -= THRUST_PER_TIC(mech);
    if (!MechOT(mech) && (MechVelocity(mech) > 0 ? MechThrust(mech) > funcspeed : MechThrust(mech) < -funcspeed)) {
	MechDesiredThrust(mech) = MechVelocity(mech) > 0 ? funcspeed : -funcspeed;
	if (event_tick % 5 == 0)
	    mech_notify(mech, MECHALL, "Somehow you have overthrusted with overthrust engaged! Slowing down ASAP!");
	}
    if (nthrust <= WalkingSpeed(MMaxSpeed(mech)) && MechThrust(mech) > WalkingSpeed(MMaxSpeed(mech)))
	MechLOSBroadcast(mech, "flashes on it's afterburners!");
    else if (nthrust > WalkingSpeed(MMaxSpeed(mech)) && MechThrust(mech) <= WalkingSpeed(MMaxSpeed(mech)))
	MechLOSBroadcast(mech, "'s afterburners putter out.");
    }

/* Degrade Velocity */
if (MechZ(mech) < ORBIT_Z) {
    if (SpheroidDS(mech) && (abs(MechAngle(mech)) >= 70)) {
            if (MechAngle(mech) < 0)
                MechVelocity(mech) += ((KPH_PER_VELOCITY(mech) * 4) / TURN);
            else
                MechVelocity(mech) -= ((KPH_PER_VELOCITY(mech) * 2) / TURN);
        } else {
            MechVelocity(mech) = MAX(0, MechVelocity(mech) - floor((KPH_PER_VELOCITY(mech) * (MechZ(mech) < ATMO_Z ? 4 : 2)) / TURN));
            if ((event_tick - LastSpinRoll(mech)) > TURN && (!SpheroidDS(mech) || (SpheroidDS(mech) && abs(MechAngle(mech)) < 70)) && 
		MechVelocity(mech) < SPEED_STALL(mech) && !Spinning(mech)) {
                mech_notify(mech, MECHALL, "You're going to slow and stall out!");
		LastSpinRoll(mech) = event_tick;
		if (!MadePilotSkillRoll(mech, (Running(mech) ? 1 : 0))) {
                    aero_ControlEffect(mech);
		    }
                } else if ((event_tick - LastSpinRoll(mech)) > TURN && MechVelocity(mech) > SPEED_OVER(mech) && !Spinning(mech)) { 
		    mech_notify(mech, MECHALL, "You're going to fast and stall out!");
		    LastSpinRoll(mech) = event_tick;
		    if (!MadePilotSkillRoll(mech, (Running(mech) ? 1 : 0))) {
			aero_ControlEffect(mech);
			}
		} else if ((event_tick - LastSpinRoll(mech)) > TURN && (fabsf(MechThrust(mech)) > SI_SAFE_THRUST(mech))) {
		    mech_notify(mech, MECHALL, "You're pulling too many G's for too long!");
		    LastSpinRoll(mech) = event_tick;
		    if (!MadePilotSkillRoll(mech, (Running(mech) ? 1 : 0))) {
			DamageAeroSI(mech, 0, NULL);
			headhitmwdamage(mech, 1);
			aero_ControlEffect(mech);
			}
		}
	    }
    } else {
        if ((event_tick - LastSpinRoll(mech)) > TURN && (fabsf(MechThrust(mech)) > SI_SAFE_THRUST(mech))) {
            mech_notify(mech, MECHALL, "You're pulling too many G's for too long!");
            LastSpinRoll(mech) = event_tick;
            if (!MadePilotSkillRoll(mech, (Running(mech) ? 1 : 0))) {
		DamageAeroSI(mech, 0, NULL);
		headhitmwdamage(mech, 1);
		aero_ControlEffect(mech);
		}
	    }
	}

/* Apply velocity */
pol = (MechVelocity(mech) == 0 ? 0 : MechVelocity(mech) > 0 ? 1 : -1); 
if (MechCritStatus(mech) & CHEAD && MechZ(mech) >= ORBIT_Z && (MechFacing(mech) != MechDesiredFacing(mech) || MechAngle(mech) != MechDesiredAngle(mech))) {
    if (MechThrust(mech) > 0)
	MechVelocity(mech) += (((MechThrust(mech) - THRUST_PER_HEADING(mech)) / MP1) * KPH_PER_VELOCITY(mech)) / TURN;
    else
	MechVelocity(mech) += (((MechThrust(mech) + THRUST_PER_HEADING(mech)) / MP1) * KPH_PER_VELOCITY(mech)) / TURN; 
    MechCritStatus(mech) &= ~CHEAD;
} else {
    MechVelocity(mech) += ((MechThrust(mech) / MP1) * KPH_PER_VELOCITY(mech)) / TURN;
} 
if ((MechVelocity(mech) < 0 && pol > 0) || (MechVelocity(mech) > 0 && pol < 0)) {
    MechVelocity(mech) = 0;
    if (MechThrust(mech) <= MP2 && fabsf(MechDesiredThrust(mech)) <= MP2) {
	mech_notify(mech, MECHALL, "Thrusters have successfully stabilized you. You are now floating still.");
	MechDesiredThrust(mech) = (MechZ(mech) < ORBIT_Z ? 18.75 : 0);
	MechThrust(mech) = MechDesiredThrust(mech);
	}
    }
if (MechVelocity(mech) > 4000)
    MechVelocity(mech) = 4000;
else if (MechVelocity(mech) < -4000)
    MechVelocity(mech) = -4000;

dz = MechVelocity(mech) * degsin(MechAngle(mech));
xypart = MechVelocity(mech) * degcos(MechAngle(mech));
if (xypart < 0)
    xypart = 0 - xypart;
if (MechVelocity(mech) < 0)
    FindComponents(xypart, AcceptableDegree(MechFacing(mech) + 180), &dx, &dy); 
else
    FindComponents(xypart, MechFacing(mech), &dx, &dy);

xypart = my_sqrt(dx, dy);
MechSpeed(mech) = xypart;
my_sqrt(my_sqrt(dx, dy), dz);
MechVerticalSpeed(mech) = dz;
MechStartFX(mech) = dx;
MechStartFY(mech) = dy;
MechStartFZ(mech) = dz;
}

void aero_UpdateHeading(MECH * mech)
{
int trate = 1, normangle;

    if (Spinning(mech)) {
	MechDesiredThrust(mech) = BOUNDED(0 - MMaxSpeed(mech), MechDesiredThrust(mech) + Number(-4, 4), MMaxSpeed(mech)); 
        MechDesiredAngle(mech) = BOUNDED(-90, MechDesiredAngle(mech) + Number(-5, 5), 90);
        MechDesiredFacing(mech) = AcceptableDegree(MechDesiredFacing(mech) + Number(-10, 10));
	}
    if (HORIZ_TAKEOFF(mech) && MechZ(mech) < ORBIT_Z && MechVelocity(mech) < SPEED_STALL(mech)) {
	MechFZ(mech) -= ZSCALE * 3;
	MechZ(mech) = MechFZ(mech) / ZSCALE;
	}

    if (MechDesiredFacing(mech) != MechFacing(mech) || MechDesiredAngle(mech) != MechAngle(mech)) {
	if (MechZ(mech) >= ORBIT_Z && fabsf(MechThrust(mech)) < THRUST_PER_HEADING(mech))
	    return;
	else
	    MechCritStatus(mech) |= CHEAD;
        if (MechZ(mech) < ORBIT_Z) {
            switch ((int) (fabs(MechVelocity(mech)) / KPH_PER_VELOCITY(mech))) {
                case 1:
		    trate = 3; 
		    break;
                case 2:
                case 3:
		case 4:
		case 5:
                case 6:
                case 7:
		    trate = 2;
		    break;
                default:
                    trate = 1;
                    break;
                break;
                }
	    if (ISAEROFIGHTER(mech) && MechTons(mech) < 80)
		trate += 4 - (MechTons(mech) / 20);
	    if (abs(MechAngle(mech)) > 60)
		trate *= 2.0;
	    else if (abs(MechAngle(mech)) > 30)
		trate *= 1.5;
	    }
	else
            trate = 5;
	} 

    if (MechZ(mech) < ORBIT_Z) {
	if (SpheroidDS(mech) && MechVelocity(mech) < KPH_PER_VELOCITY(mech) * 2)
	    trate *= 5;
	else if (MechZ(mech) >= ATMO_Z)
	    trate *= 2;
	if (HORIZ_TAKEOFF(mech) || (SpheroidDS(mech) && abs(MechAngle(mech)) < 70)) {
	    if (MechAngle(mech) < -45)
		MechVelocity(mech) += (KPH_PER_VELOCITY(mech) / TURN);
	    else if (MechAngle(mech) > 45)
		MechVelocity(mech) -= ((KPH_PER_VELOCITY(mech) * 2) / TURN);
	    }
	}

    if (MechDesiredAngle(mech) != MechAngle(mech)) {
        if (MechDesiredAngle(mech) < MechAngle(mech))
            MechAngle(mech) = (MAX(MechDesiredAngle(mech), MechAngle(mech) - trate));
        else if (MechDesiredAngle(mech) > MechAngle(mech))
            MechAngle(mech) = (MIN(MechDesiredAngle(mech), MechAngle(mech) + trate));
        }

    if (MechDesiredFacing(mech) != MechFacing(mech)) {
	normangle = MechRFacing(mech) - SHO2FSIM(MechDesiredFacing(mech));
	if (normangle < 0)
	    normangle += SHO2FSIM(360); 
        trate = SHO2FSIM(trate);
        if (normangle > SHO2FSIM(180)) {
            AddRFacing(mech, trate);
            if (MechFacing(mech) >= 360)
                 AddFacing(mech, -360);
            normangle += trate;
            if (normangle >= SHO2FSIM(360))
                SetRFacing(mech, SHO2FSIM(MechDesiredFacing(mech)));
        } else {
            AddRFacing(mech, -trate);
            if (MechRFacing(mech) < 0)
                AddFacing(mech, 360);
            normangle -= trate;
            if (normangle < 0)
                SetRFacing(mech, SHO2FSIM(MechDesiredFacing(mech)));
        }
    }

    MarkForLOSUpdate(mech); 
}

int FuelCheck(MECH * mech)
{
    int fuelcost = 1;

    /* We don't do anything particularly nasty to shutdown things */
    if (!Started(mech))
	return 0;

    if (is_aero(mech)) {
	if ((event_tick % TURN) != 0)
	    return 0;
	else if (MechThrust(mech) == 0)
	    return 0;
	else
	    fuelcost = (fabsf(MechThrust(mech)) / MP1);
    } else {
	if (fabs(MechSpeed(mech) + MechVerticalSpeed(mech)) <= MP3) {
	    if ((event_tick % (TURN * 10)) != 0)
		return 0;
	} else if (fabs(MechSpeed(mech) + MechVerticalSpeed(mech)) < MWalkingSpeed(MMaxSpeed(mech))) {
	    if ((event_tick % 3) != 0)
		return 0;
	} else {
	    if ((event_tick % 2) != 0)
		return 0;
	}
    }

    if ((MechType(mech) == CLASS_VTOL) && MechZ(mech) > 20)
	    fuelcost *= 1.0 + ((MechZ(mech) / 20) * 0.5);

    if (AeroFuel(mech) > 0) {
	if (AeroFuel(mech) <= fuelcost)
	    AeroFuel(mech) = 0;
	else
	    AeroFuel(mech) -= fuelcost;
	return 0;
    }
    /* DropShips do not need crash ; they switch to (VERY SLOW) secondary
       power source. */
    if (IsDS(mech)) {
	if (AeroFuel(mech) < 0)
	    return 0;
	AeroFuel(mech)--;
	mech_notify(mech, MECHALL,
	    "As the fuel runs out, the engines switch to backup power.");
	return 0;
    }
    if (AeroFuel(mech) < 0)
	return 1;
    /* Now, the true nastiness begins ;) */
    AeroFuel(mech)--;
    if (!(AeroFuel(mech) % 100) && AeroFuel(mech) >= AeroFuelOrig(mech))
	SetCargoWeight(mech);
    if (MechType(mech) == CLASS_VTOL) {
	MechLOSBroadcast(mech, "'s rotors suddenly stop!");
	mech_notify(mech, MECHALL, "The sound of rotors slowly stops..");
    } else {
	MechLOSBroadcast(mech, "'s engines die suddenly..");
	mech_notify(mech, MECHALL, "Your engines die suddenly..");
    }
    if (!is_aero(mech)) {
	MechSpeed(mech) = 0.0;
	MechDesiredSpeed(mech) = 0.0;
	}
    if (!Landed(mech)) {
	/* Start free-fall */
	if (MechZ(mech) < ORBIT_Z) {
	    mech_notify(mech, MECHALL, "You ponder F = ma, S = F/m, S = at^2 => S=agt^2 in relation to the ground..");
	    MechVerticalSpeed(mech) = 0;
	    /* Hmm. This _can_ be ugly if things crash in middle of fall. Oh well. */
	    mech_notify(mech, MECHALL, "You start free-fall.. Enjoy the ride!");
	    MECHEVENT(mech, EVENT_FALL, mech_fall_event, FALL_TICK, -1);
	    } else {
		mech_notify(mech, MECHALL, "Your engine putters as it runs out of fuel....");
		MechThrust(mech) = 0;
		MechDesiredThrust(mech) = 0;
	    }
	}
    return 1;
}

void aero_update(MECH * mech)
{
    if (Destroyed(mech))
	return;
    if (Started(mech) || Uncon(mech)) {
	UpdatePilotSkillRolls(mech);
    }
    if (Started(mech) || MechPlusHeat(mech) > 0.)
	UpdateHeat(mech);
/*
    if (!(event_tick / 3 % 5)) {
	if (!Spinning(mech))
	    return;
	if (Destroyed(mech))
	    return;
	if (Landed(mech))
	    return;
	if (MadePilotSkillRoll(mech,
		(MechStartSpin(mech) - event_tick) / 15 + 8)) {
	    mech_notify(mech, MECHALL,
		"You recover control of your craft.");
	    StopSpinning(mech);
	}
    }
*/
    if (Started(mech))
	MechVisMod(mech) =
	    BOUNDED(0, MechVisMod(mech) + Number(-40, 40), 100);
    end_ecm_check(mech); 
    end_lite_check(mech);
}

void aero_spinning_event(EVENT * e)
{ 
MECH *mech = (MECH *) e->data;
if (Destroyed(mech) || Landed(mech))
    return;

LastSpinRoll(mech) = event_tick;
if (!MadePilotSkillRoll(mech, (Running(mech) ? 1 : 0))) {
    mech_notify(mech, MECHALL, "You fail to recover your craft!");
    StartSpinning(mech);
    return;
    }
mech_notify(mech, MECHALL, "You recover control of your craft!");
}

void aero_setcruise(dbref player, void *data, char *arg)
{
    MECH *mech = (MECH *) data;
    char *args[2];
    int argc;
    float speed;

    DOCHECK(Landed(mech), "You're landed!");
    cch(MECH_USUALO);
    DOCHECK(TakingOff(mech), "You cannot modify thrust while taking off!");
    DOCHECK(Landing(mech), "You cannot modify thrust while landing!");
    if ((argc = mech_parseattributes(arg, args, 1)) == 0) {
        notify(player, tprintf("Your current cruise value is %.2f and your velocity is %.2f. Cruise Control is %s.", MechJumpSpeed(mech), MechVelocity(mech), 
		MechCruise(mech) ? "%cgON%cn" : "%crOFF%cn"));
        return;
    }
    DOCHECK(argc !=1, "Invalid number of arguments.");
    if (strcmp(args[0], "info") == 0) {
	notify(player, tprintf("In your current zone : Stall Speed = %.2f OverSpeed = %.2f", (float) SPEED_STALL(mech), (float) SPEED_OVER(mech)));
	return;
     } else if (strcmp(args[0], "current") == 0) {
	MechJumpSpeed(mech) = MechVelocity(mech);
	notify(player, tprintf("Cruise Speed set to %.2f.", MechJumpSpeed(mech)));
	return;
     } 
    speed = atof(args[0]);
    if (speed < SPEED_STALL(mech))
	speed = SPEED_STALL(mech);
    if (speed > SPEED_OVER(mech)) 
	speed = SPEED_OVER(mech);

    MechJumpSpeed(mech) = speed;
    notify(player, tprintf("Cruise Speed set to %.2f.", MechJumpSpeed(mech)));
}

void aero_thrust(dbref player, void *data, char *arg)
{
    MECH *mech = (MECH *) data;
    char *args[3];
    int argc;
    float newspeed, maxspeed;

    DOCHECK(Landed(mech), "You're landed!");
    cch(MECH_USUALO);
    DOCHECK(TakingOff(mech), "You cannot modify thrust while taking off!");
    DOCHECK(Landing(mech), "You cannot modify thrust while landing!");
    if ((argc = mech_parseattributes(arg, args, 2)) == 0) {
	notify(player, tprintf("Your current thrust is %.2f (Throttled : %.2f Velocity : %.2f).", MechThrust(mech), MechDesiredThrust(mech), MechVelocity(mech)));
	return;
    }
    if (strcmp(args[0], "hover") == 0)
	newspeed = 18.75;
    else if (strcmp(args[0], "atmo") == 0)
	newspeed = SPEED_ATMO(mech);
    else if (strcmp(args[0], "turn") == 0)
	newspeed = THRUST_PER_HEADING(mech);
    else if (strcmp(args[0], "rturn") == 0)
	newspeed = 0 - THRUST_PER_HEADING(mech);
    else if (strcmp(args[0], "safe") == 0)
	newspeed = SAFE_THRUST(mech);
    else if (strcmp(args[0], "over") == 0)
	newspeed = MMaxSpeed(mech);
    else if (strcmp(args[0], "rsafe") == 0)
	newspeed = 0 - SAFE_THRUST(mech);
    else if (strcmp(args[0], "rover") == 0)
	newspeed = 0 - MMaxSpeed(mech);
    else if (argc > 1 && strcmp(args[0], "+") == 0)
	newspeed = BOUNDED(0 - MMaxSpeed(mech), MechThrust(mech) + atof(args[1]), MMaxSpeed(mech));
    else if (argc > 1 && strcmp(args[0], "-") == 0)
	newspeed = BOUNDED(0 - MMaxSpeed(mech), MechThrust(mech) - atof(args[1]), MMaxSpeed(mech)); 
    else 
	newspeed = atof(args[0]);
    maxspeed = MMaxSpeed(mech);
    if (!(maxspeed > 0.0))
	maxspeed = 0.0;
    DOCHECK(Fallen(mech), "Your engine's dead, no way to thrust!");
    DOCHECK(newspeed < 0 && SpheroidDS(mech) && MechZ(mech) < ORBIT_Z,
	"Doh, thrust backwards.. where's your sense of adventure?"); 
    if (AeroFuel(mech) <= 0)
	newspeed = MAX(MIN(newspeed, MP1), -MP1);
    if (newspeed > maxspeed || newspeed < -maxspeed) {
	notify(player, tprintf("Maximum thrust: %.2f (%.2f kb/sec2)", maxspeed, maxspeed / 10));
	return;
    }
    DOCHECK(!MechOT(mech) && (MechVelocity(mech) > 0 ? newspeed > SAFE_THRUST(mech) : newspeed < (0 - SAFE_THRUST(mech))), "You cannot exceed safe thrust! (This includes SI threshold) Turn 'OVERTHRUST ON' if you want to!"); 
    MechDesiredThrust(mech) = newspeed;
    mech_notify(mech, MECHALL, tprintf("Throttle set to %.2f.", MechDesiredThrust(mech)));
    MaybeMove(mech);
}

void aero_vheading(dbref player, void *data, char *arg, int flag)
{
    char *args[1];
    int i = 0;
    MECH *mech = (MECH *) data;


    cch(MECH_USUALO);
    if (mech_parseattributes(arg, args, 1) != 1) {
	notify(player, tprintf("Present angle: %d degrees.",
		MechAngle(mech)));
	return;
    }
    DOCHECK(Spinning(mech), "You are unable to control your craft at the moment.");
    i = flag * atoi(args[0]);
    if (abs(i) > 90)
	i = 90 * flag;
    DOCHECK(abs(i) != 90 && MechZ(mech) < ATMO_Z &&
	SpheroidDS(mech), tprintf("You can go only up / down at <%d z!",
	    ATMO_Z));
    if (i >= 0)
	mech_notify(mech, MECHALL,
	    tprintf("Climbing angle set to %d degrees.", i));
    else
	mech_notify(mech, MECHALL,
	    tprintf("Diving angle set to %d degrees.", 0 - i));
    MechDesiredAngle(mech) = i;
}

void aero_climb(dbref player, MECH * mech, char *arg)
{
    aero_vheading(player, mech, arg, 1);
}

void aero_dive(dbref player, MECH * mech, char *arg)
{
    aero_vheading(player, mech, arg, -1);
}

enum {
    NO_ERROR, INVALID_TERRAIN, UNEVEN_TERRAIN, BLOCKED_LZ
};

char *reasons[] = {
    "Improper terrain",
    "Uneven ground",
    "Blocked landing zone"
};


int ImproperLZ(MECH * mech, int check_dyn)
{
    int dirs[7][2] = {
	{0, 0},
	{0, -1},
	{1, 0},
	{1, 1},
	{0, 1},
	{-1, 1},
	{-1, 0}
    };
    int x1, y1, x2, y2, x = MechX(mech), y = MechY(mech);
    int i;
    MAP *map = getMap(mech->mapindex);

/*
    if (GetRTerrain(map, x, y) != GRASSLAND &&
	GetRTerrain(map, x, y) != ROAD) return INVALID_TERRAIN;
*/
  if (IsDS(mech)) {
    for (i = 1; i < NUM_NEIGHBORS; i++) {
	x1 = x + dirs[i][0];
	y1 = y + dirs[i][1];
	if (x % 2 && !(x1 % 2))
	    y1--;
	x2 = BOUNDED(0, x1, map->map_width - 1);
	y2 = BOUNDED(0, y1, map->map_height - 1);
	if (x1 != x2 || y1 != y2)
	    return UNEVEN_TERRAIN;
	if (Elevation(map, x1, y1) != Elevation(map, x, y))
	    return UNEVEN_TERRAIN;
    }
  }
    if (is_blocked_lz(mech, map, x, y, check_dyn, 0))
	return BLOCKED_LZ;
    return 0;
}

int ImproperLZ_MapHex(MECH * mech, MAP * map, int x, int y)
{
    int dirs[7][2] = {
        {0, 0},
        {0, -1},
        {1, 0},
        {1, 1},
        {0, 1},
        {-1, 1},
        {-1, 0}
    };
    int x1, y1, x2, y2;
    int i;

    if (!map)
	return BLOCKED_LZ;

/*
    if (GetRTerrain(map, x, y) != GRASSLAND && GetRTerrain(map, x, y) != ROAD)
	return INVALID_TERRAIN;
*/
  if (IsDS(mech)) {
    for (i = 1; i < NUM_NEIGHBORS; i++) {
        x1 = x + dirs[i][0];
        y1 = y + dirs[i][1];
        if (x % 2 && !(x1 % 2))
            y1--;
        x2 = BOUNDED(0, x1, map->map_width - 1);
        y2 = BOUNDED(0, y1, map->map_height - 1);
        if (x1 != x2 || y1 != y2)
            return UNEVEN_TERRAIN;
        if (Elevation(map, x1, y1) != Elevation(map, x, y))
            return UNEVEN_TERRAIN;
    }
  }
    if (is_blocked_lz(mech, map, x, y, 1, 0))
        return BLOCKED_LZ;

return 0;
}

static char *colorstr(int serious)
{
    if (serious == 1)
	return "%ch%cr";
    if (serious == 0)
	return "%ch%cy";
    return "";
}

void DS_LandWarning(MECH * mech, int serious)
{
    int ilz = ImproperLZ(mech, MechStatus2(mech) & CS_ON_LAND ? 1 : 0);

    if (!ilz)
	return;
    ilz--;
    mech_notify(mech, MECHALL, tprintf("%sWARNING: %s - %s%%cn",
	    colorstr(serious), reasons[ilz],
	    serious == 1 ? "CLIMB UP NOW!!!" : serious ==
	    0 ? "No further descent is advisable." :
	    "Please do not even consider landing here."));
}

void aero_checklz(dbref player, MECH * mech, char *buffer)
{
    int ilz;

    cch(MECH_USUAL);
    DOCHECK(Landed(mech),
	"You are landed already.. it looks good, doesn't it?");
    ilz = ImproperLZ(mech, MechStatus2(mech) & CS_ON_LAND ? 1 : 0);
    DOCHECKMA(!ilz, tprintf("The hex (%d,%d) looks good enough for a landing.", MechX(mech), MechY(mech)));
    ilz--;
    mech_notify(mech, MECHALL,
	tprintf("The hex (%d,%d) doesn't look good for landing: %s.",
	    MechX(mech), MechY(mech), reasons[ilz]));
}

#define CHECK_AEROCRIT(a,b,c) (AeroCritStatus(a) & b ? c : "%ch%cgOOO%cn")
void aero_status(dbref player, MECH * mech, char *buffer)
{
    char buf[1024];
    cch(MECH_USUALSM);

    notify(player, "%ch%cm/=== [ CritType ] =======|==1==|==2==|==3==|==4==\\%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Avionics               %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, AVIONICS_ONECRIT, "%ch%cr +1%cn"), CHECK_AEROCRIT(mech, AVIONICS_TWOCRIT, "%ch%cr +2%cn"), CHECK_AEROCRIT(mech, AVIONICS_THREECRIT, " %ch%cr +5%cn"));
    notify(player, buf);
    
    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Engine                 %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, ENGINE_ONECRIT, "%ch%cr 2 %cn"), CHECK_AEROCRIT(mech, ENGINE_TWOCRIT, "%ch%cr 4 %cn"), CHECK_AEROCRIT(mech, ENGINE_DESTROYED, "%ch%crDST%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn FCS                    %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, FCS_ONECRIT, "%ch%cr +2%cn"), CHECK_AEROCRIT(mech, FCS_TWOCRIT, "%ch%cr +4%cn"), CHECK_AEROCRIT(mech, FCS_DESTROYED, "%ch%crDST%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Gear                   %%ch%%cm|%%cn %s %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, GEAR_DAMAGED, "%ch%cr +5%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Life Support           %%ch%%cm|%%cn %s %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, LIFESUPPORT_DAMAGED, "%ch%cr +2%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Sensors                %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %%ch%%cx---%%cn %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, SENSORS_ONECRIT, "%ch%cr +1%cn"), CHECK_AEROCRIT(mech, SENSORS_TWOCRIT, "%ch%cr +2%cn"), CHECK_AEROCRIT(mech, SENSORS_THREECRIT, " %ch%cr +5%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Thrusters : Right      %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, RTHRUST_ONECRIT, "%ch%cr +1%cn"), CHECK_AEROCRIT(mech, RTHRUST_TWOCRIT, "%ch%cr +2%cn"), CHECK_AEROCRIT(mech, RTHRUST_THREECRIT, " %ch%cr +3%cn"), CHECK_AEROCRIT(mech, RTHRUST_DESTROYED, "%ch%crDST%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|========================|=====|=====|=====|=====|%cn");

    snprintf(buf, sizeof(char) * 1024, 
	"%%ch%%cm|%%cn Thrusters : Left       %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn %s %%ch%%cm|%%cn",
	CHECK_AEROCRIT(mech, LTHRUST_ONECRIT, "%ch%cr +1%cn"), CHECK_AEROCRIT(mech, LTHRUST_TWOCRIT, "%ch%cr +2%cn"), CHECK_AEROCRIT(mech, LTHRUST_THREECRIT, " %ch%cr +3%cn"), CHECK_AEROCRIT(mech, LTHRUST_DESTROYED, "%ch%crDST%cn"));
    notify(player, buf);

    notify(player, "%ch%cm|=== [ Damage Thresholds ] ======================|%cn");
    snprintf(buf, sizeof(char) * 1024,
	"%%ch%%cm|%%cn %%ch%%cyNose%%cn ( %d ) %%ch%%cyLWing%%cn ( %d ) %%ch%%cyRWing%%cn ( %d ) %%ch%%cyAft%%cn ( %d )   %%ch%%cm|%%cn",
	((GetSectOArmor(mech, ISAEROFIGHTER(mech) ? AERO_NOSE : DS_NOSE) + 9) / 10), ((GetSectOArmor(mech, ISAEROFIGHTER(mech) ? AERO_LWING : DS_LWING) + 9) / 10),
	((GetSectOArmor(mech, ISAEROFIGHTER(mech) ? AERO_RWING : DS_RWING) + 9) / 10), ((GetSectOArmor(mech, ISAEROFIGHTER(mech) ? AERO_AFT : DS_AFT) + 9) / 10));
    notify(player, buf);

    notify(player, "%ch%cm\\================================================/%cn");
    
}
