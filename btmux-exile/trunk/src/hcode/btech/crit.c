#include <math.h>
#include "config.h"
#include "externs.h"
#include "db.h"
#include "mech.h"
#include "mech.events.h"
#include "mech.sensor.h"
#include "p.econ_cmds.h"
#include "p.mech.update.h"
#include "p.bsuit.h"
#include "autopilot.h"
#include "p.mech.combat.h"
#include "p.mech.utils.h"
#include "p.btechstats.h"
#include "p.mechrep.h"
#include "p.mech.restrict.h"
#include "p.eject.h"
#include "p.template.h"
#include "mech.partnames.h"
#include "p.mech.partnames.h"
#include "p.aero.move.h"
#include "p.mech.pickup.h"
#include "p.mech.sensor.h"
#include "p.artillery.h"
#include "p.mech.status.h"
#include "p.mech.hitloc.h"

void correct_speed(MECH * mech)
{
    float maxspeed = MMaxSpeed(mech);
    int neg = 1;

    if (MechMaxSpeed(mech) < 0.0)
	MechMaxSpeed(mech) = 0.0;
    SetCargoWeight(mech);
    if (MechDesiredSpeed(mech) < -0.1) {
	maxspeed = maxspeed * 2.0 / 3.0;
	neg = -1;
    }
    if (fabs(MechDesiredSpeed(mech)) > maxspeed)
	MechDesiredSpeed(mech) = (float) maxspeed *neg;

    if (fabs(MechSpeed(mech)) > maxspeed)
	MechSpeed(mech) = (float) maxspeed *neg;
}

void explode_unit(MECH * wounded, MECH * attacker)
{
    int j;
    MECH *target;
    dbref i, tmpnext;
    dbref from;

    from = wounded->mynum;

    SAFE_DOLIST(i, tmpnext, Contents(from)) {
	if (Good_obj(i) && Hardcode(i)) {
	    if ((target = getMech(i))) {
		    KillMechContentsIfIC(target->mynum, target);
		    discard_mw(target);
	    }
	}
    }

    KillMechContentsIfIC(wounded->mynum, wounded);
    for (j = 0; j < NUM_SECTIONS; j++) {
	if (GetSectOInt(wounded, j) && !SectIsDestroyed(wounded, j))
	    DestroySection(wounded, attacker, wounded == attacker ? 0 : 1,
		j);
    }
}

void JamMainWeapon(MECH * mech)
{
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int count;
    int loop;
    int ii;
    int tempcrit;
    int maxcrit = 0;
    int maxloc = 0;
    int critfound = 0;
    int critnum = 0;
    unsigned char maxtype = 0;



    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	if (SectIsDestroyed(mech, loop))
	    continue;
	count = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (count > 0) {
	    for (ii = 0; ii < count; ii++) {
		if (!PartIsDestroyed(mech, loop, critical[ii])) {
		    /* tempcrit = GetWeaponCrits(mech, weaparray[ii]); */
		    tempcrit = rand();
		    if (tempcrit > maxcrit) {
			critfound = 1;
			maxcrit = tempcrit;
			maxloc = loop;
			maxtype = weaparray[ii];
			critnum = critical[ii];
		    }
		}
	    }
	}
    }
    if (critfound) {
	GetPartMode(mech, maxloc, critnum) |= JAMMED_MODE;
	mech_notify(mech, MECHALL, tprintf("%%ch%%crYour %s is jammed!%%c",
		&MechWeapons[maxtype].name[3]));
    }
}

void HandleVTOLCrit(MECH * wounded, MECH * attacker, int LOS, int hitloc,
    int num)
{
    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
    switch (random() % 6) {
    case 0:
	/* Crew killed */
	mech_notify(wounded, MECHALL, "Your cockpit is destroyed!!");

	if (!Landed(wounded)) {
	    mech_notify(attacker, MECHALL,
		"You knock the VTOL out of the sky!");
	    MechLOSBroadcast(wounded, "falls down from the sky!");
	    MechFalls(wounded, MechsElevation(wounded), 0);
	}
	if (!Destroyed(wounded)) {
/*
	    DestroyAndDump(wounded);
	    SendDebug(tprintf("#%d has been killed by #%d", wounded->mynum,
		    attacker ? attacker->mynum : wounded->mynum));
*/
	    DestroyMech(wounded, attacker ? attacker : wounded, 1);
	}
	KillMechContentsIfIC(wounded->mynum, wounded);
	break;
    case 1:
	/* Weapon jams, set them recylcling maybe */
	/* hmm. nothing for now, tanks are so weak */
	JamMainWeapon(wounded);
	break;
    case 2:
	/* Engine Hit */
	mech_notify(wounded, MECHALL, "Your engine takes a direct hit!!");
	if (!Landed(wounded)) {
	    if (MechRTerrain(wounded) == GRASSLAND ||
		MechRTerrain(wounded) == ROAD ||
		MechRTerrain(wounded) == BUILDING) {
		if (MadePilotSkillRoll(wounded,
			MechZ(wounded) - MechElevation(wounded))) {
		    mech_notify(wounded, MECHALL, "You land safely!");
		    MechStatus(wounded) |= LANDED;
		    MechZ(wounded) = MechElevation(wounded);
		    MechFZ(wounded) = ZSCALE * MechZ(wounded);
		    SetMaxSpeed(wounded, 0.0);
		    MechVerticalSpeed(wounded) = 0.0;
		}
	    } else {
		mech_notify(wounded, MECHALL, "You crash to the ground!");
		mech_notify(attacker, MECHALL,
		    "You knock the VTOL out of the sky!");
		MechLOSBroadcast(wounded, "falls from the sky!");
		MechFalls(wounded, MechsElevation(wounded), 0);
	    }
	}
	SetMaxSpeed(wounded, 0.0);
	break;
    case 3:
	/* Crew Killed */
	mech_notify(wounded, MECHALL, "Your cockpit is destroyed!!");
	if (!(MechStatus(wounded) & LANDED)) {
	    mech_notify(attacker, MECHALL,
		"You knock the VTOL out of the sky!");
	    MechLOSBroadcast(wounded, "falls from the sky!");
	    MechFalls(wounded, MechsElevation(wounded), 0);
	}
	if (!Destroyed(wounded)) {
/*
	    DestroyAndDump(wounded);
	    SendDebug(tprintf("#%d has been killed by #%d", wounded->mynum,
		    attacker ? attacker->mynum : wounded->mynum));
*/
	    DestroyMech(wounded, attacker ? attacker : wounded, 1);
	}
	KillMechContentsIfIC(wounded->mynum, wounded);
	break;
    case 4:
	/* Fuel Tank Explodes */
	mech_notify(wounded, MECHALL,
	    "Your fuel tank explodes in a ball of fire!");
	if (wounded != attacker)
	    MechLOSBroadcast(attacker, "explodes in a ball of fire!");
	MechZ(wounded) = MechElevation(wounded);
	MechFZ(wounded) = ZSCALE * MechZ(wounded);
	MechSpeed(wounded) = 0.0;
	MechVerticalSpeed(wounded) = 0.0;
	DestroyMech(wounded, attacker, 0);
	explode_unit(wounded, attacker);
	break;
    case 5:
	/* Ammo/Power Plant Explodes */
	mech_notify(wounded, MECHALL, "Your power plant explodes!!");
	MechLOSBroadcast(wounded, "suddenly explodes!");
	MechZ(wounded) = MechElevation(wounded);
	MechFZ(wounded) = ZSCALE * MechZ(wounded);
	MechSpeed(wounded) = 0.0;
	MechVerticalSpeed(wounded) = 0.0;
	DestroyMech(wounded, attacker, 0);
	if (!(MechSections(wounded)[BSIDE].config & CASE_TECH))
	    explode_unit(wounded, attacker);
	else
	    DestroySection(wounded, attacker, LOS, BSIDE);
    }
}

void DestroyMainWeapon(MECH * mech)
{
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int count;
    int loop;
    int ii;
    int tempcrit;
    int maxcrit = 0;
    int maxloc = 0;
    int critfound = 0;
    unsigned char maxtype = 0;



    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	if (SectIsDestroyed(mech, loop))
	    continue;
	count = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (count > 0) {
	    for (ii = 0; ii < count; ii++) {
		if (!PartIsDestroyed(mech, loop, critical[ii])) {
		    /* tempcrit = GetWeaponCrits(mech, weaparray[ii]); */
		    tempcrit = rand();
		    if (tempcrit > maxcrit) {
			critfound = 1;
			maxcrit = tempcrit;
			maxloc = loop;
			maxtype = weaparray[ii];
		    }
		}
	    }
	}
    }
    if (critfound) {
	DestroyWeapon(mech, maxloc, I2Weapon(maxtype), 1, -1);
	mech_notify(mech, MECHALL, tprintf("%%ch%%crYour %s is destroyed!%%c", &MechWeapons[maxtype].name[3]));
    }
}

void HandleFasaVehicleCrit(MECH * wounded, MECH * attacker, int LOS,
    int hitloc, int num)
{
    if (MechMove(wounded) == MOVE_NONE)
	return;

    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
    switch (random() % 6) {
    case 0:
	/* Crew stunned for one turn...treat like a head hit */
	headhitmwdamage(wounded, 1);
	break;
    case 1:
	/* Weapon jams, set them recylcling maybe */
	/* hmm. nothing for now, tanks are so weak */
	JamMainWeapon(wounded);
	break;
    case 2:
	/* Engine Hit */
	mech_notify(wounded, MECHALL,
	    "Your engine takes a direct hit!!  You can't move anymore.");
	SetMaxSpeed(wounded, 0.0);
	break;
    case 3:
	/* Crew Killed */
	mech_notify(wounded, MECHALL,
	    "Your armor is pierced and you are killed instantly!!");
	DestroyMech(wounded, attacker, 0);
	KillMechContentsIfIC(wounded->mynum, wounded);
	break;
    case 4:
	/* Fuel Tank Explodes */
	mech_notify(wounded, MECHALL,
	    "Your fuel tank explodes in a ball of fire!!");
	if (wounded != attacker)
	    MechLOSBroadcast(wounded, "explodes in a ball of fire!");
	DestroyMech(wounded, attacker, 0);
	explode_unit(wounded, attacker);
	break;
    case 5:
	/* Ammo/Power Plant Explodes */
	mech_notify(wounded, MECHALL, "Your power plant explodes!!");
	if (wounded != attacker)
	    MechLOSBroadcast(wounded, "suddenly explodes!");
	DestroyMech(wounded, attacker, 0);
	if (!(MechSections(wounded)[BSIDE].config & CASE_TECH))
	    explode_unit(wounded, attacker);
	else
	    DestroySection(wounded, attacker, LOS, BSIDE);
	break;
    }
}
/* AT2 AeroCrit table Definitions - DJ */
#define AERO_NOCRIT		0
#define AERO_WEAPONCRIT		1
#define AERO_SENSORCRIT		2
#define AERO_HEATSINKCRIT	3
#define AERO_AVIONICSCRIT	4
#define AERO_FCSCRIT		5
#define AERO_GEARCRIT		6
#define AERO_FUELCRIT		7
#define AERO_ENGINECRIT		8
#define AERO_CREWCRIT		9
#define AERO_BOMBCRIT		10
#define AERO_CONTROLCRIT	11

void HandleAeroCrit(MECH * wounded, MECH * attacker, int LOS, int hitloc, int num)
{
    int crittype = 0, i = 0, ii = 0, critnum = -1, ZB;

    if (Roll() < 8)
	return;

    if (attacker)
        if (((ZB = FindZBearing(MechFX(attacker), MechFY(attacker), MechFZ(attacker), MechFX(wounded), MechFY(wounded), MechFZ(wounded))) < -45) ||
             (ZB > 45)) {
            switch (Roll()) {
                case 2:
                case 6:
                case 8:
                case 12:
                    crittype = AERO_WEAPONCRIT;
		    break;
                case 3:
                case 11:
		    crittype = AERO_GEARCRIT;
		    break;
                case 4:
		    crittype = AERO_SENSORCRIT;
		    break;
                case 5:
		    crittype = AERO_CREWCRIT;
		    break;
                case 7:
		    crittype = AERO_AVIONICSCRIT;
		    break;
                case 9:
		    crittype = AERO_CONTROLCRIT;
		    break;
                case 10:
                    crittype = AERO_ENGINECRIT;
		    break;
                default:
                    crittype = AERO_CONTROLCRIT;
		    break;
                }
            }
    switch (hitloc)
	{
	case AERO_NOSE:
	    switch (Roll())
		{
		case 2:
		case 5:
		case 9:
		case 12:
		    crittype = AERO_WEAPONCRIT;
		    break;
		case 3:
		    crittype = AERO_SENSORCRIT;
		    break;
		case 4:
		case 10:
		    crittype = AERO_HEATSINKCRIT;
		    break;
		case 6:
		    crittype = AERO_AVIONICSCRIT;
		    break;
		case 7:
		    crittype = AERO_CONTROLCRIT;
		    break;
		case 8:
		    crittype = AERO_FCSCRIT;
		    break;
		case 11:
		    crittype = AERO_GEARCRIT;
		    break;
		}
	    break;
	case AERO_LWING:
	case AERO_RWING:
	    switch (Roll())
		{
		case 2:
		case 6:
		case 12:
		    crittype = AERO_WEAPONCRIT;
		    break;
		case 3:
		case 11:
		    crittype = AERO_GEARCRIT;
		    break;
		case 4:
		    crittype = AERO_SENSORCRIT;
		    break;
		case 5:
		    crittype = AERO_CREWCRIT;
		    break;
		case 7:
		    crittype = AERO_AVIONICSCRIT;
		    break;
		case 8:
		    crittype = AERO_BOMBCRIT;
		    break;
		case 9:
		    crittype = AERO_CONTROLCRIT;
		    break;
		case 10:
		    crittype = AERO_ENGINECRIT;
		    break;
		}
	    break;
	case AERO_AFT:
	    switch (Roll())
		{
		case 2:
		case 5:
		case 9:
		case 12:
		    crittype = AERO_WEAPONCRIT;
		    break;
		case 3:
		case 11:
		    crittype = AERO_HEATSINKCRIT;
		    break;
		case 4:
		case 10:
		    crittype = AERO_FUELCRIT;
		    break;
		case 6:
		case 8:
		    crittype = AERO_ENGINECRIT;
		    break;
		case 7:
		    crittype = AERO_CONTROLCRIT;
		    break;
		}
	    break;
	}
    switch (crittype)
	{
	case AERO_WEAPONCRIT:
            mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            for (i = 0; i < NUM_CRITICALS; i++) {
                if (IsWeapon(GetPartType(wounded, hitloc, i)) && !PartIsNonfunctional(wounded, hitloc, i) && !PartIsDisabled(wounded, hitloc, i) &&
				!PartIsDestroyed(wounded, hitloc, i)) {
			critnum = i;
                        break;
                    }
                }
            if (critnum == -1) {
                mech_notify(wounded, MECHALL, "You luck out and appear to be out of weapons to have destroyed!");
                return;
                }
	    MechLOSBroadcast(wounded, "wobbles as it loses a weapon.");
            mech_notify(wounded, MECHALL, tprintf("Your %s is destroyed!", get_parts_long_name(GetPartType(wounded, hitloc, critnum))));
            DestroyPart(wounded, hitloc, critnum);
            break;
	case AERO_SENSORCRIT:
            mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    if (AeroCritStatus(wounded) & SENSORS_THREECRIT) {
		mech_notify(wounded, MECHALL, "Your sensors are too damaged to degrade more performance!");
	    } else if (AeroCritStatus(wounded) & SENSORS_TWOCRIT) {
		MechCritStatus2(wounded) |= SENSORS_THREECRIT;
		MechBTH(wounded) += 3;
		mech_notify(wounded, MECHALL, "Your sensors are damaged beyond recognition!");
	    } else {
	   	mech_notify(wounded, MECHALL, "Your sensors are damaged!");
		MechBTH(wounded) += 1;
		if (AeroCritStatus(wounded) & SENSORS_ONECRIT)
		    MechCritStatus2(wounded) |= SENSORS_TWOCRIT;
		else
		    MechCritStatus2(wounded) |= SENSORS_ONECRIT;
	    }
	    MechLOSBroadcast(wounded, " is temporarily envoloped in electric discharge as it's sensors frizzle.");
	    break;
	case AERO_HEATSINKCRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            if ((MechSpecials(wounded) & DOUBLE_HEAT_TECH) ||
		(MechSpecials2(wounded) & COMPACT_HEAT_TECH)) {
                MechRealNumsinks(wounded) -= 2;
            } else
                MechRealNumsinks(wounded)--;
            mech_notify(wounded, MECHALL, "You lost a heat sink!");
	    MechLOSBroadcast(wounded, "blows a stream of green coolant out in a trail behind it.");
            break;
	case AERO_AVIONICSCRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    if (AeroCritStatus(wounded) & AVIONICS_THREECRIT) {
                mech_notify(wounded, MECHALL, "Your avionics are too damaged to degrade more performance!");
            } else if (AeroCritStatus(wounded) & AVIONICS_TWOCRIT) {
                MechCritStatus2(wounded) |= AVIONICS_THREECRIT;
                MechPilotSkillBase(wounded) += 3;
                mech_notify(wounded, MECHALL, "Your avionics are damaged beyond recognition!");
            } else {
                mech_notify(wounded, MECHALL, "Your avionics are damaged!");
                MechPilotSkillBase(wounded) += 1;
                if (AeroCritStatus(wounded) & AVIONICS_ONECRIT)
                    MechCritStatus2(wounded) |= AVIONICS_TWOCRIT;
                else
                    MechCritStatus2(wounded) |= AVIONICS_ONECRIT;
            }
	    MechLOSBroadcast(wounded, "is enveloped in a mild electronic discharge as it's avionics gear is hit.");
	    crittype = AERO_CONTROLCRIT;
	case AERO_FCSCRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            if (AeroCritStatus(wounded) & FCS_DESTROYED) {
                mech_notify(wounded, MECHALL, "Your FCS is already destroyed!");
            } else if (AeroCritStatus(wounded) & FCS_TWOCRIT) {
                MechCritStatus2(wounded) |= FCS_DESTROYED;
                MechBTH(wounded) += 50;
                mech_notify(wounded, MECHALL, "Your FCS is destroyed prevening weapons fire!");
            } else {
                mech_notify(wounded, MECHALL, "Your FCS is damaged!");
                MechBTH(wounded) += 2;
                if (AeroCritStatus(wounded) & FCS_ONECRIT)
                    MechCritStatus2(wounded) |= FCS_TWOCRIT;
                else
                    MechCritStatus2(wounded) |= FCS_ONECRIT;
            }
	    MechLOSBroadcast(wounded, "wobbles as a large Fire Control System computer falls out from it's underside.");
            break;
	case AERO_GEARCRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    if (AeroCritStatus(wounded) & GEAR_DAMAGED) {
		mech_notify(wounded, MECHALL, "Your landing gear is already damaged!");
	    } else {
		mech_notify(wounded, MECHALL, "Your landing gear is damaged!");
		MechCritStatus2(wounded) |= GEAR_DAMAGED;
	    }
	    MechLOSBroadcast(wounded, "jinx around from the aerodynamic shock of it's landing gear being hit.");
	    break;
	case AERO_FUELCRIT:
	    MechLOSBroadcast(wounded, "is trailed by a large plume of fire as it's fuel tank is hit!");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    i = ((Roll() + Roll() + Roll()) * 30);
	    if ((AeroFuel(wounded) - i) <= 0)
		AeroFuel(wounded) = 0;
	    else
		AeroFuel(wounded) -= i;
	    mech_notify(wounded, MECHALL, "Your fuel tank takes a hit!");
	    if (Roll() >= 10 && AeroFuel(wounded) > 0) {
		mech_notify(wounded, MECHALL, "Your fuel tanks explodes!");
	        if (wounded != attacker)
                    MechLOSBroadcast(wounded, "explodes in a burning, blazing, great ball of fire!");
                if (Started(wounded) || Starting(wounded))
                    {
		    headhitmwdamage(wounded, 1);
                    autoeject(MechPilot(wounded), wounded, attacker);
 /*                   SendDebug(tprintf("#%d has been killed by #%d", wounded->mynum, attacker->mynum)); */
                    }
                DestroyMech(wounded, attacker ? attacker : wounded, 0);
                explode_unit(wounded, attacker);
                break;

	    }
	case AERO_ENGINECRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    if (AeroCritStatus(wounded) & ENGINE_DESTROYED) {
		mech_notify(wounded, MECHALL, "Your engine is already destroyed!");
	    } else if (AeroCritStatus(wounded) & ENGINE_TWOCRIT) {
		MechCritStatus2(wounded) |= ENGINE_DESTROYED;
		MechEngineHeat(wounded) = 15;
                mech_notify(wounded, MECHALL, "Your engine is destroyed!!");
                if (wounded != attacker && !Destroyed(wounded) && attacker)
                    mech_notify(attacker, MECHALL, "You destroy the engine!!");
	        DestroyMech(wounded, attacker, 1);
	    } else {
		MechEngineHeat(wounded) += 2;
                mech_notify(wounded, MECHALL, "Your engine takes a hit and loses thrust!  It's getting hotter in here!!");
		SetMaxSpeed(wounded, ((WalkingSpeed(MechMaxSpeed(wounded)) - MP2) * 1.5));
                if (MechDesiredSpeed(wounded) > MMaxSpeed(wounded))
                    MechDesiredSpeed(wounded) = MMaxSpeed(wounded);
		if (AeroCritStatus(wounded) & ENGINE_ONECRIT)
		    MechCritStatus2(wounded) |= ENGINE_TWOCRIT;
		else
		    MechCritStatus2(wounded) |= ENGINE_ONECRIT;
            }
	    MechLOSBroadcast(wounded, "loses speed and wobbles as it's engine sputters.");
            break;
	case AERO_CREWCRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    mech_notify(wounded, MECHALL, "Your take personal damage!!");
	    headhitmwdamage(wounded, 1);
	    MechLOSBroadcast(wounded, "take a large WACK in the cockpit!");
	    break;
	case AERO_BOMBCRIT:
	case AERO_CONTROLCRIT:
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            mech_notify(wounded, MECHALL, "The damage causes you to start to spin!!");
	    if (!(MadePilotSkillRoll(wounded, 0)) && !Landed(wounded)) {
		aero_ControlEffect(wounded);
	    } else {
		mech_notify(wounded, MECHALL, "You successfully regain control of the craft!");
	    }
	    MechLOSBroadcast(wounded, "shimmies as it loses some control surface and lift!");
	    break;
	}
}
/* MaxTechTankCrit Definitions (really a tweak of MaxTech AdvGroundVehCritTabls) for ease of life - DJ */
#define TANK_NOCRIT             0
#define TANK_DRIVER_HIT         1
#define TANK_COMMANDER_HIT      2
#define TANK_CREW_STUNNED       3
#define TANK_CREW_KILLED        4
#define TANK_STABILIZER         5
#define TANK_SENSORS            6
#define TANK_CARGO_HIT          7  /* Inimplemented. In part, to laziness. In part, due unfairness on high econ site where */
				        /* Tows are the mainstay of cargo moving. Just not koschers. Besides, adds 1/12 Pading  */
					/* to Sides and rear for tanks. Nice Tankies! - DJ Well. Not Pading. Replaced with Stun */
					/* Reimplemented to remove tow ability from tanks with crit! TANKS ARE POOP!
						-- Reb */
#define TANK_WEAPON_JAM         8
#define TANK_WEAPON_DEST        9
#define TANK_TURRET_JAM         10
#define TANK_TURRET_LOCK        11
#define TANK_TURRET_BLOWN       12
#define TANK_ENGINE_HIT         13
#define TANK_FUEL_TANK          14
#define TANK_AMMOBOOM           15
#define VTOL_COPILOT_HIT	16
#define VTOL_PILOT_HIT	        17
#define VTOL_ROTOR_HIT		18
#define VTOL_ROTOR_DEST		19
#define VTOL_TAILROTOR_HIT	20

void HandleMaxTechVehicleCrit(MECH * wounded, MECH * attacker, int LOS,
    int hitloc, int num)
{
    int i = 0, ii = 0, critnum = -1, critsection = -1, weapindx, damage;
    int crittype = 0, noweap = 0;
    int IsVTOL = (MechType(wounded) == CLASS_VEH_VTOL);
    char critname[30];
    int chartroll = Roll();

    if (MechMove(wounded) == MOVE_NONE)
        return;

    if (MechSpecials(wounded) & ICE_TECH && chartroll < 6)
    	chartroll = Roll();

    switch (hitloc) {
	case FSIDE:
	    switch (chartroll)
		{
		/*case 5:*/
		case 6:
		    if (IsVTOL)
			crittype = VTOL_COPILOT_HIT;
		    else
		        crittype = TANK_DRIVER_HIT;
		    break;
		case 7:
		    crittype = TANK_WEAPON_JAM;
		    break;
		case 8:
		    crittype = TANK_STABILIZER;
		    break;
		case 9:
		    crittype = TANK_SENSORS;
		    break;
		case 10:
		    if (IsVTOL)
			crittype = VTOL_PILOT_HIT;
		    else
		        crittype = TANK_COMMANDER_HIT;
		    break;
		case 11:
		    crittype = TANK_WEAPON_DEST;
		    break;
		case 12:
		    crittype = TANK_CREW_KILLED;
		    break;
		break;
		}
	    break;
	case RSIDE:
	case LSIDE:
            switch (chartroll)
                {
/*		case 5:*/
                case 6:
                      crittype = TANK_CARGO_HIT;
/*		      crittype = TANK_CREW_STUNNED;*/
                    break;
                case 7:
                    crittype = TANK_WEAPON_JAM;
                    break;
                case 8:
                    crittype = TANK_CREW_STUNNED;
                    break;
                case 9:
                    crittype = TANK_STABILIZER;
                    break;
                case 10:
                    crittype = TANK_WEAPON_DEST;
                    break;
                case 11:
                    crittype = TANK_ENGINE_HIT;
                    break;
                case 12:
                    crittype = TANK_FUEL_TANK;
                    break;
		break;
                }
	    break;
	case BSIDE:
            switch (chartroll)
                {
		/*case 5:*/
                case 6:
                    crittype = TANK_WEAPON_JAM;
                    break;
                case 7:
                      crittype = TANK_CARGO_HIT;
/*		      crittype = TANK_CREW_STUNNED;*/
                    break;
                case 8:
                    crittype = TANK_STABILIZER;
                    break;
                case 9:
                    crittype = TANK_WEAPON_DEST;
                    break;
                case 10:
                    crittype = TANK_ENGINE_HIT;
                    break;
                case 11:
                    crittype = TANK_AMMOBOOM;
                    break;
                case 12:
                    crittype = TANK_FUEL_TANK;
                    break;
		break;
                }
	    break;
	case TURRET:
            switch (chartroll)
                {
		/*case 4:
		case 5:*/
                case 6:
                    crittype = TANK_STABILIZER;
                    break;
                case 7:
                    crittype = TANK_TURRET_JAM;
                    break;
                case 8:
                    crittype = TANK_WEAPON_JAM;
                    break;
                case 9:
                    crittype = TANK_TURRET_LOCK;
                    break;
                case 10:
                    crittype = TANK_WEAPON_DEST;
                    break;
                case 11:
                    crittype = TANK_TURRET_BLOWN;
                    break;
                case 12:
                    crittype = TANK_AMMOBOOM;
                    break;
		break;
                }
	    break;
	case ROTOR:
	    switch (chartroll)
		{
		/*case 5:*/
                case 6:
                case 7:
                case 8:
                    crittype = VTOL_ROTOR_HIT;
                    break;
                case 9:
                case 10:
                    crittype = VTOL_TAILROTOR_HIT;
                    break;
                case 11:
                case 12:
                    crittype = VTOL_ROTOR_DEST;
                    break;
                break;
		}
	    break;
	}
   switch (crittype) {
	case VTOL_COPILOT_HIT:
		strcpy(critname, "VTOL Copilot Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    mech_notify(wounded, MECHALL, "The CoPilot is wounded! Targeting performance decreases!");
	    MechLOSBroadcast(wounded, "wobbles in midair momentarily!");
	    MechBTH(wounded) += 1;
            headhitmwdamage(wounded, 1);
	    break;
	case VTOL_PILOT_HIT:
		strcpy(critname, "VTOL Pilot Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            mech_notify(wounded, MECHALL, "The Pilot is wounded! Piloting performance decreases and you lose some elevation!");
            MechLOSBroadcast(wounded, "wobbles in midair momentarily!");
            headhitmwdamage(wounded, 1);
            MechPilotSkillBase(wounded) += 2;
	    if (!Landed(wounded))
		MechZ(wounded) -= 3;
	    if (MechZ(wounded) < 0)
		MechZ(wounded) = 0;
	    CheckVTOLHeight(wounded);
            break;
	case VTOL_ROTOR_HIT:
		strcpy(critname, "VTOL Rotor Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            mech_notify(wounded, MECHALL, "Your rotor has been damaged! You abruptly slow down!");
	    SetMaxSpeed(wounded, ((WalkingSpeed(MechMaxSpeed(wounded)) - MP1) * 1.5));
	    MechLOSBroadcast(wounded, "jinks side to side as it's propellers are damaged!");
	    correct_speed(wounded);
	    break;
	case VTOL_ROTOR_DEST:
		strcpy(critname, "VTOL Rotor Destruction");
	    if (!Fallen(wounded)) {
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		mech_notify(wounded, MECHALL, "Your rotor is destroyed!!");
		if (!Landed(wounded)) {
		    if (MechCarrying(wounded) > 0) {
			MECH *ttarget;
			if ((ttarget = getMech(MechCarrying(wounded)))) {
			    mech_notify(ttarget, MECHALL, "Your tow lines go suddenly slack!");
			    mech_dropoff(GOD, wounded, "");
            		    }
		        }
 	            mech_notify (wounded, MECHALL, "You crash to the ground!");
		    MechLOSBroadcast(wounded, "suddenly loses lift as it's rotors are destroyed!");
	            MechLOSBroadcast (wounded, "crashes to the ground!");
	            MechFalls(wounded, MAX(1 + MechsElevation(wounded),1), 0);
  		    }
	        MakeMechFall(wounded);
	        SetMaxSpeed(wounded, 0.0);
		}
	    break;
	case VTOL_TAILROTOR_HIT:
		strcpy(critname, "VTOL Tailrotor Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    MechLOSBroadcast(wounded, "begins to jimmy around as it's tailrotor is nailed!");
	    if (MechCritStatus(wounded) & TAILROTOR_DAMAGED) {
		mech_notify(wounded, MECHALL, "Luckily your Tail Rotor can't take much more damage.");
		break;
		}
	    mech_notify(wounded, MECHALL, "Your tailrotor takes a direct hit!");
	    MechCritStatus(wounded) |= TAILROTOR_DAMAGED;
	    correct_speed(wounded);
	    break;
	case TANK_NOCRIT:
		strcpy(critname, "No Crit");
	    break;
	case TANK_AMMOBOOM:
		strcpy(critname, "Ammo Boom");
            mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            for (i = 0; i < NUM_CRITICALS; i++) {
                if (IsAmmo(GetPartType(wounded, hitloc, i)) && GetPartData(wounded, hitloc, i)) {
                        critnum = i;
			critsection = hitloc;
                        break;
                    }
            }
            if (critnum == -1)
                for (ii = 0; ii < NUM_SECTIONS; ii++) {
                    if (ii == hitloc)
                        continue;
                    for (i = 0; i < NUM_CRITICALS; i++) {
                        if (IsAmmo(GetPartType(wounded, ii, i)) && GetPartData(wounded, ii, i)) {
                            critnum = i;
                            critsection = ii;
                            break;
                            }
                        }
                    if (critnum > -1)
                        break;
                }
            if (critnum == -1) {
                mech_notify(wounded, MECHALL, "You luck out and appear to have no ammunition to explode!");
		crittype = TANK_WEAPON_DEST;
                } else {
                mech_notify(wounded, MECHALL, tprintf("Your %s ammo explodes!", get_parts_long_name(GetPartType(wounded, critsection, critnum))));
                weapindx = Ammo2WeaponI(GetPartType(wounded, critsection, critnum));
	        damage = GetPartData(wounded, critsection, critnum)
	        	* GunStat(weapindx, GetPartMode(wounded, critsection, critnum), GUN_DAMAGE)
	        	* (IsAcid(weapindx) ? 3 : 1);
	        if (IsMissile(weapindx) || IsArtillery(weapindx)) {
	            for (i = 0; MissileHitTable[i].key != -1; i++)
	                if (MissileHitTable[i].key == weapindx)
        	            damage *= MissileHitTable[i].num_missiles[10];
        		    }
	        if (MechWeapons[weapindx].special & (GAUSS | NOBOOM)) {
	            if (MechWeapons[weapindx].special & GAUSS)
        	        mech_notify(wounded, MECHALL, "One of your Gauss Rifle Ammo feeds is destroyed");
	            DestroyPart(wounded, critsection, critnum);
		    break;
	        } else if (damage) {
		    if (MechSections(wounded)[critsection].config & CASE_TECH) {
			mech_notify(wounded, MECHALL, "Your aft armor blows off as your CASE equipment protects you!");
			MechLOSBroadcast(wounded, "blows it's Aft armor as its CASE vents the splooge!");
			SetSectArmor(wounded, critsection, 0);
			} else {
			MechLOSBroadcast(wounded, "jumps into the air as it's aft is ripped by flames!");
               		ammo_explosion(attacker, wounded, critsection, critnum, damage);
			KillMechContentsIfIC(wounded->mynum, wounded);
			break;
			}
                } else {
                mech_notify(wounded, MECHALL, "You have no ammunition left in that location, lucky you!");
                DestroyPart(wounded, critsection, critnum);
		break;
    	        }
	        headhitmwdamage(wounded, 1);
		}
		/* Intentional Fallthrough for side effects */
	case TANK_DRIVER_HIT:
		strcpy(critname, "Driver Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    mech_notify(wounded, MECHALL, "Your Driver is hit by shrapnel!");
	    MechLOSBroadcast(wounded, "swerves side to side violently!");
	    MechPilotSkillBase(wounded) += 2;
	    if (crittype == TANK_DRIVER_HIT) {
	            headhitmwdamage(wounded, 1);
		    break;
		    }
	case TANK_COMMANDER_HIT:
		strcpy(critname, "Commander Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            mech_notify(wounded, MECHALL, "The Commander is wounded! Crew performance decreaees!");
            MechPilotSkillBase(wounded) += 1;
	    MechLOSBroadcast(wounded, "slows down momentarily...");
	    MechBTH(wounded) += 1;
            headhitmwdamage(wounded, 1);
		/* Intentional Fallthrough for side effects */
    case TANK_CREW_STUNNED:
        strcpy(critname, "Crew Stunned");
	    if (crittype == TANK_CREW_STUNNED)
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            if (crittype == TANK_CREW_STUNNED)
		    mech_notify(wounded, MECHALL, "The vehicle violently shakes, stunning the crew!");
  	    else if (crittype ==  TANK_COMMANDER_HIT)
		    mech_notify(wounded, MECHALL, "The crew is confused while the commander is temporarily down!");
	    else
		    mech_notify(wounded, MECHALL, "The explosion rattles the crew! You are stunned!");
	    if (CrewStunning(wounded))
		StopCrewStunning(wounded);
	    MechLOSBroadcast(wounded, "significantly slows down and starts swerving!");
	    MechCritStatus(wounded) |= CREW_STUNNED;
            if (MechSpeed(wounded) > WalkingSpeed(MechMaxSpeed(wounded)))
	        MechDesiredSpeed(wounded) = WalkingSpeed(MechMaxSpeed(wounded));
            MECHEVENT(wounded, EVENT_CREWSTUN, mech_crewstun_event, CREWSTUN_TICK, 0);
	    if (crittype == TANK_CREW_STUNNED)
	        headhitmwdamage(wounded, 1);
	    break;
	case TANK_CREW_KILLED:
		strcpy(critname, "Crew Killed");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    mech_notify(wounded, MECHALL, "Shrapnel fills the crew cabin! Your buddies all die!");
/*	    i = ((char_getskillsuccess(MechPilot(wounded), "Perception", 0, 1) > 0) && (char_getskillsuccess(MechPilot(wounded), "Acrobatics", 0, 1) > 0));
	    if (i)
		mech_notify(wounded, MECHALL, "You manage to dodge the shrapnel as it swarms around you!");
	    else {
		mech_notify(wounded, MECHALL, "You manage to dodge the... uh.. ooops. You didn't."); */
                mech_notify(wounded, MECHALL, "Your armor is pierced and you are killed gruesomely!!");
/*		 } */
            DestroyMech(wounded, attacker, 0);
/*	    if (!i) */
		MechLOSBroadcast(wounded, "suddenly stops motionless as blood splatters onto the windows!");
                KillMechContentsIfIC(wounded->mynum, wounded);
	    break;
	case TANK_STABILIZER:
		strcpy(critname, "Stabilizer Hit");
	    if (!(MechSections(wounded)[hitloc].config & STABILIZER_CRIT)) {
	            mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	            mech_notify(wounded, MECHALL, tprintf("Your %s weapon stabilizers are knocked out!", (hitloc == RSIDE ?
			"Right Side" : hitloc == LSIDE ? "Left Side" : hitloc == FSIDE ? "Front Side" : hitloc == BSIDE ? "Aft" :
			hitloc == TURRET ? "Turret" : "Unknown")));
		    MechLOSBroadcast(wounded, tprintf("exhibits odd behaviour on it's %s...", (hitloc == RSIDE ?
		        "Right Side" : hitloc == LSIDE ? "Left Side" : hitloc == FSIDE ? "Front Side" : hitloc == BSIDE ? "Aft" :
			 hitloc == TURRET ? "Turret" : "Unknown")));
		    MechSections(wounded)[hitloc].config |= STABILIZER_CRIT;
		    /*headhitmwdamage(wounded, 1);*/
			/*mech_notify(wounded, MECHALL,
			    "You are violently bounced around in your cockpit!");
			mech_notify(wounded, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(wounded, (MechSpeed(wounded) / MP4))) {
			    mech_notify(wounded, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(wounded, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(wounded, 1);
			}*/
		    break;
		}
	case TANK_SENSORS:
		strcpy(critname, "Sensor Hit");
	    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
            mech_notify(wounded, MECHALL, "Your screens fizzle as your sensors take a direct hit!");
            if (!(MechCritStatus(wounded) & SENSORS_DAMAGED)) {
                MechLRSRange(wounded) = ((MechLRSRange(wounded) * 2) / 3);
                MechTacRange(wounded) = ((MechLRSRange(wounded) * 2) / 3);
                MechScanRange(wounded) = ((MechLRSRange(wounded) * 2) / 3);
                MechBTH(wounded) += 1;
                MechCritStatus(wounded) |= SENSORS_DAMAGED;
		MechLOSBroadcast(wounded, "is engulfed in electric discharge!");
                mech_notify(wounded, MECHALL,
                    "Your sensors have been damaged!!");
            } else {
                MechLRSRange(wounded) /= 2;
                MechTacRange(wounded) /= 2;
                MechScanRange(wounded) /= 2;
                MechBTH(wounded) += 1;
		MechLOSBroadcast(wounded, "is engulfed in electric discharge!");
                mech_notify(wounded, MECHALL,
                    "Your sensors have been FURTHER damaged!!");
            }
	    /*headhitmwdamage(wounded, 1);*/
	    break;
	case TANK_CARGO_HIT:
		if (!(MechCritStatus2(wounded) & HITCH_DESTROYED) &&
		(MechMove(wounded) == MOVE_TRACK || MechMove(wounded) == MOVE_WHEEL ||
		(MechMove(wounded) == MOVE_VTOL && (MechStatus(wounded) & SALVAGE_TECH)))) {
        	strcpy(critname, "Hitch Destroyed");
	          mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
/*            mech_notify(wounded, MECHALL, "Your cargo compartment is breached! You lose your entire manifest!"); */
		MechCritStatus2(wounded) |= HITCH_DESTROYED;
	     	mech_notify(wounded, MECHALL, "Your hitch is hit!");
		if (MechCarrying(wounded) > 0)
			mech_dropoff(GOD, wounded, "");
		break; }
	/* carry on baby */
	case TANK_WEAPON_JAM:
	case TANK_WEAPON_DEST:
            mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	    for (i = 0; i < NUM_CRITICALS; i++) {
		if ((IsWeapon(GetPartType(wounded, hitloc, i)) && !PartIsNonfunctional(wounded, hitloc, i)) && (Roll() == (i + 1))) {
			critnum = i;
			critsection = hitloc;
  			break;
		    }
		if (IsWeapon(GetPartType(wounded, hitloc, i)) && !PartIsNonfunctional(wounded, hitloc, i))
			noweap = 0;
		if (critnum == -1 && noweap == 0 && i == (NUM_CRITICALS - 1) && ii < 11)
		    {
		    i = 0;
		    ii++;
		    }
		}
	    if (critnum == -1)
		for (ii = 0; ii < NUM_SECTIONS; ii++) {
		    if (ii == hitloc)
			continue;
                    for (i = 0; i < NUM_CRITICALS; i++) {
            	        if (IsWeapon(GetPartType(wounded, ii, i)) && !PartIsNonfunctional(wounded, ii, i)) {
                    	    critnum = i;
			    critsection = ii;
                       	    break;
	                    }
                	}
		    if (critnum > -1)
			break;
		}
	    if (critnum == -1) {
		mech_notify(wounded, MECHALL, "You luck out and appear to be out of weapons to have destroyed!");
			/*mech_notify(wounded, MECHALL,
			    "You are violently bounced around in your cockpit!");
			mech_notify(wounded, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(wounded, (MechSpeed(wounded) / MP4))) {
			    mech_notify(wounded, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(wounded, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(wounded, 1);
			}*/

	        return;
		}
	    if (crittype == TANK_WEAPON_JAM && !(GetPartMode(wounded, critsection, critnum) & JAMMED_MODE))
		{
			strcpy(critname, "Weapon Jam");
		mech_notify(wounded, MECHALL, tprintf("Your %s is jammed!", get_parts_long_name(GetPartType(wounded, critsection, critnum))));
		mech_notify(attacker, MECHALL, tprintf("You cause a %s to jam!", get_parts_long_name(GetPartType(wounded, critsection, critnum))));
		GetPartMode(wounded, critsection, critnum) |= JAMMED_MODE;
		} else {
			strcpy(critname, "Weapon Destruction");
                mech_notify(wounded, MECHALL, tprintf("Your %s is destroyed!", get_parts_long_name(GetPartType(wounded, critsection, critnum))));
		mech_notify(attacker, MECHALL, tprintf("You destroy a %s!", get_parts_long_name(GetPartType(wounded, critsection, critnum))));
		DestroyPart(wounded, critsection, critnum);
			/*mech_notify(wounded, MECHALL,
			    "You are violently bounced around in your cockpit!");
			mech_notify(wounded, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(wounded, (MechSpeed(wounded) / MP4))) {
			    mech_notify(wounded, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(wounded, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(wounded, 1);
			}*/

		}
	    break;
	case TANK_TURRET_JAM:
	case TANK_TURRET_LOCK:
		if (crittype == TANK_TURRET_JAM)
			strcpy(critname, "Turret Jam");
		else
			strcpy(critname, "Turret Lock");
		if (crittype == TANK_TURRET_JAM && (MechCritStatus(wounded) & TURRET_JAMMED))
			crittype = TANK_TURRET_LOCK;
                mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		if (MechCritStatus(wounded) & TURRET_LOCKED)
			{
			mech_notify(wounded, MECHALL,
			    "Your turret clanks even more but it's already locked!");
			mech_notify(wounded, MECHALL,
			    "You are violently bounced around in your cockpit!");
			mech_notify(wounded, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(wounded, (MechSpeed(wounded) / MP4))) {
			    mech_notify(wounded, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(wounded, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(wounded, 1);
			}
			break;
			}
		mech_notify(wounded, MECHALL, tprintf("Your turret takes a direct hit! Crap! It's %s!",
		    (crittype == TANK_TURRET_JAM ? "jammed" : "locked")));
		MechLOSBroadcast(wounded, tprintf("begins to rattle as it's turret is %s!",
		     (crittype == TANK_TURRET_JAM ? "jammed" : "locked")));
		if (crittype == TANK_TURRET_JAM) {
		    MechCritStatus(wounded) |= TURRET_JAMMED;
		} else {
		    MechCritStatus(wounded) |= TURRET_LOCKED;
		    if (MechCritStatus(wounded) & TURRET_JAMMED)
			MechCritStatus(wounded) &= ~TURRET_JAMMED;
		     }
		if (crittype == TANK_TURRET_LOCK)
			mech_notify(wounded, MECHALL,
			    "You are violently bounced around in your cockpit!");
			mech_notify(wounded, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(wounded, (MechSpeed(wounded) / MP4))) {
			    mech_notify(wounded, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(wounded, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(wounded, 1);
			}
		break;
	case TANK_TURRET_BLOWN:
		strcpy(critname, "Turret Destruction");
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		mech_notify(wounded, MECHALL, "The shot shears your turret at the rivets! This is gonna hurt!");
		MechLOSBroadcast(wounded, "suddenly has it's turret enveloped in searing flames as it soars into the air!");
		DestroySection(wounded, attacker, LOS, hitloc);
		break;
	case TANK_FUEL_TANK:
		strcpy(critname, "Fuel Tank Hit");
		if (MechSpecials(wounded) & ICE_TECH)
		    {
                    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		    mech_notify(wounded, MECHALL, "Your Combustion Engine sparks and ignites its fuel source!!");
	            if (wounded != attacker)
		            MechLOSBroadcast(wounded, "explodes in a burning, blazing, great ball of fire!");
                            if (Started(wounded) || Starting(wounded))
                                {
			        headhitmwdamage(wounded, 1);
                                autoeject(MechPilot(wounded), wounded, attacker);
/*                                SendDebug(tprintf("#%d has been killed by #%d", wounded->mynum, attacker->mynum)); */
                                }
	            DestroyMech(wounded, attacker ? attacker : wounded, 0);
	            explode_unit(wounded, attacker);
 		    break;
		    }
	case TANK_ENGINE_HIT:
		strcpy(critname, "Engine Hit");
                mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	        mech_notify(wounded, MECHALL, "Your engine putters as the shot cracks its working parts!");
		MechLOSBroadcast(wounded, "stops to a standstill and spews odd colored smoke out its aft!");
	        /* DivideMaxSpeed(wounded, 2); */
		SetMaxSpeed(wounded, 0.0);
			mech_notify(wounded, MECHALL,
			    "You are violently bounced around in your cockpit!");
			mech_notify(wounded, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(wounded, (MechSpeed(wounded) / MP4))) {
			    mech_notify(wounded, MECHALL,
				"Your fancy moves worked!");
				headhitmwdamage(wounded, 1);
			} else {
			    mech_notify(wounded, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(wounded, 2);
			}
		if (MechSpecials(wounded) & ICE_TECH && Roll() >= 10) {
			mech_notify(wounded, MECHALL, "Uh oh....");
			MechLOSBroadcast(wounded, "blows sky high in a pillar of fire!");
			DestroyMech(wounded, attacker, 0);
			explode_unit(wounded, attacker);
			break;
		}
		if (crittype == TANK_FUEL_TANK && Roll() >= BOOM_BTH && mudconf.btech_stackpole &&
			(Started(wounded) || Starting(wounded))) {
			mech_notify(wounded, MECHALL, "Uh oh....");
			HexLOSBroadcast(FindObjectsData(wounded->mapindex), MechX(wounded), MechY(wounded),
				"%ch%crThe hit destroys the last safety systems, releasing the fusion reaction!%cn");
/*		        SendDebug(tprintf("#%d has been killed by #%d", wounded->mynum, attacker->mynum)); */
			DestroyMech(wounded, attacker ? attacker : wounded, 0);
			explode_unit(wounded, attacker);
			MechZ(wounded) += 6;
			ScrambleInfraAndLiteAmp(wounded, 4, 0,
			    "The blinding flash of light blinds you!",
			    "The blinding flash of light blinds you!");
			blast_hit_hexesf(FindObjectsData(wounded->mapindex), MAX(MechTons(wounded) / 10,
			    MechEngineSize(wounded) /25), 3, MAX(MechTons(wounded) / 10, MechEngineSize(wounded) / 25),
			    MechFX(wounded), MechFY(wounded), MechFX(wounded), MechFY(wounded),
			    "%ch%crYou bear full brunt of the blast!%cn", "is hit badly by the blast!",
			    "%ch%cyYou receive some damage from the blast!%cn", "is hit by the blast!",
			    mudconf.btech_engine > 1, 3, 5, 1, 2, NULL, NULL);
			MechZ(wounded) -= 6;
		}
	        break;
	}
}

void HandleVehicleCrit(MECH * wounded, MECH * attacker, int LOS,
    int hitloc, int num)
{
    if (MechMove(wounded) == MOVE_NONE)
	return;
    if (hitloc == TURRET) {
	if (Number(1, 3) == 2) {
	    if (!(MechCritStatus(wounded) & TURRET_LOCKED)) {
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		MechCritStatus(wounded) |= TURRET_LOCKED;
		mech_notify(wounded, MECHALL,
		    "Your turret takes a direct hit and immobilizes!");
	    }
	    return;
	}
    } else
	switch (Number(1, 10)) {
	case 1:
	case 2:
	case 3:
	case 4:
	    if (!Fallen(wounded)) {
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		switch (MechMove(wounded)) {
		case MOVE_TRACK:
		    mech_notify(wounded, MECHALL,
			"One of your tracks is damaged!!");
		    break;
		case MOVE_WHEEL:
		    mech_notify(wounded, MECHALL,
			"One of your wheels is damaged!!");
		    break;
		case MOVE_HOVER:
		    mech_notify(wounded, MECHALL,
			"Your air skirt is damaged!!");
		    break;
		case MOVE_HULL:
		case MOVE_SUB:
		case MOVE_FOIL:
		    mech_notify(wounded, MECHALL,
			"Your speed slows down..");
		    break;
		}
		LowerMaxSpeed(wounded, MP1);
	    }
	    return;
	    break;
	case 5:
	    if (!Fallen(wounded)) {
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		switch (MechMove(wounded)) {
		case MOVE_TRACK:
		    mech_notify(wounded, MECHALL,
			"One of your tracks is destroyed, imobilizing your vehicle!!");
		    break;
		case MOVE_WHEEL:
		    mech_notify(wounded, MECHALL,
			"One of your wheels is destroyed, imobilizing your vehicle!!");
		    break;
		case MOVE_HOVER:
		    mech_notify(wounded, MECHALL,
			"Your lift fan is destroyed, imobilizing your vehicle!!");
		    break;
		case MOVE_HULL:
		case MOVE_SUB:
		case MOVE_FOIL:
		    mech_notify(wounded, MECHALL,
			"You are halted in your tracks - literally.");
		}
		SetMaxSpeed(wounded, 0.0);

		MakeMechFall(wounded);
	    }
	    return;
	    break;
	}
    mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
    switch (random() % 6) {
    case 0:
	/* Crew stunned for one turn...treat like a head hit */
	headhitmwdamage(wounded, 1);
	break;
    case 1:
	/* Weapon jams, set them recylcling maybe */
	/* hmm. nothing for now, tanks are so weak */
	JamMainWeapon(wounded);
	break;
    case 2:
	/* Engine Hit */
	mech_notify(wounded, MECHALL,
	    "Your engine takes a direct hit!!  You can't move anymore.");
	SetMaxSpeed(wounded, 0.0);
	break;
    case 3:
	/* Crew Killed */
	mech_notify(wounded, MECHALL,
	    "Your armor is pierced and you are killed instantly!!");
	DestroyMech(wounded, attacker, 0);
	KillMechContentsIfIC(wounded->mynum, wounded);
	break;
    case 4:
	/* Fuel Tank Explodes */
	mech_notify(wounded, MECHALL,
	    "Your fuel tank explodes in a ball of fire!!");
	if (wounded != attacker)
	    MechLOSBroadcast(wounded, "explodes in a ball of fire!");
	DestroyMech(wounded, attacker, 0);
	explode_unit(wounded, attacker);
	break;
    case 5:
	/* Ammo/Power Plant Explodes */
	mech_notify(wounded, MECHALL, "Your power plant explodes!!");
	if (wounded != attacker)
	    MechLOSBroadcast(wounded, "suddenly explodes!");
	DestroyMech(wounded, attacker, 0);
	explode_unit(wounded, attacker);
	break;
    }
}

int HandleMechCrit(MECH * wounded, MECH * attacker, int LOS, int hitloc,
    int critHit, int critType, int critData, int realcrit, int notif)
{
    int weapindx, damage, loop, destroycrit, weapon_slot;
    MAP *map = FindObjectsData(wounded->mapindex);
    char locname[20];
    char critname[30];

    if (realcrit) {
	if (hitloc != -1)
		ArmorStringFromIndex(hitloc, locname, MechType(wounded), MechMove(wounded));
	else
		strcpy(locname, "Invalid");
	strcpy(critname, pos_part_name(wounded, hitloc, critHit));
    }

    if (realcrit && wounded && attacker)
	if (wounded->mynum == attacker->mynum)
		realcrit = 0;
    if (!attacker || (attacker ? attacker->mynum < 0 : 0) || Destroyed(wounded))
	realcrit = 0;

    if (critType == EMPTY ||
        critType == Special(CASE) || critType == Special(FERRO_FIBROUS) ||
        critType == Special(ENDO_STEEL) || critType == Special(TRIPLE_STRENGTH_MYOMER) ||
        critType == Special(MASC) || critType == Special(HARDPOINT) || critType == Special(STEALTHARM))
	    return 1;

    if (notif)
        mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!!%c");

    if (IsAmmo(critType)) {
	/* BOOM! */
	/* That's going to hurt... */
	weapindx = Ammo2WeaponI(critType);
	damage = critData * GunStat(weapindx, GetPartMode(wounded, hitloc, critHit), GUN_DAMAGE)
		* (IsAcid(weapindx) ? 3 : 1);
	if (IsMissile(weapindx) || IsArtillery(weapindx)) {
	    for (loop = 0; MissileHitTable[loop].key != -1; loop++)
		if (MissileHitTable[loop].key == weapindx)
		    damage *= MissileHitTable[loop].num_missiles[10];
	}
	if (MechWeapons[weapindx].special & (GAUSS | NOBOOM)) {
	    if (MechWeapons[weapindx].special & GAUSS)
		mech_notify(wounded, MECHALL,
		    "One of your Gauss Rifle Ammo feeds is destroyed");
	    DestroyPart(wounded, hitloc, critHit);
	} else if (damage) {
	    ammo_explosion(attacker, wounded, hitloc, critHit, damage);
	} else {
	    mech_notify(wounded, MECHALL,
		"You have no ammunition left in that location, lucky you!");
	    DestroyPart(wounded, hitloc, critHit);
	}
	SetPartData(wounded, hitloc, critHit, 0);
	return 1;
    }

    if (PartIsDisabled(wounded, hitloc, critHit) && !IsWeapon(critType)) {
	DestroyPart(wounded, hitloc, critHit);
	return 1;
    }

    if (IsWeapon(critType)) {
#if 0
	if (!mudconf.btech_mtxcrit) {
	    mech_notify(wounded, MECHALL, tprintf("Your %s has been destroyed!!", &MechWeapons[Weapon2I(critType)].name[3]));
	if (WeaponIsNonfunctional(wounded, hitloc, WeaponFirstCrit(wounded, hitloc, critHit), GetWeaponCrits(wounded, Weapon2I(critType))) == 0)
		mech_notify(wounded, MECHALL, tprintf("Your %s ceases to function!!", &MechWeapons[Weapon2I(critType)].name[3]));
	if (LOS && wounded != attacker && attacker)
	    mech_notify(attacker, MECHALL, tprintf("You destroy a %s!!", &MechWeapons[Weapon2I(critType)].name[3]));
	if ((WeaponIsNonfunctional(wounded, hitloc, WeaponFirstCrit(wounded, hitloc, critHit), GetWeaponCrits(wounded, Weapon2I(critType))) == 0) &&
		 LOS && wounded != attacker && attacker)
                mech_notify(attacker, MECHALL, tprintf("Your shot disables the %s!!", &MechWeapons[Weapon2I(critType)].name[3]));
	/* horrid reuse of this int. ohwell, watch if i care. -foo 
	weapindx = (PartIsNonfunctional(wounded, hitloc, critHit) ||
		    WeaponIsNonfunctional(wounded, hitloc, WeaponFirstCrit(wounded, hitloc, critHit), GetWeaponCrits(wounded,Weapon2I(critType))) ||
		    (PartTempNuke(wounded, hitloc, critHit) == FAIL_DESTROYED && !mudconf.btech_mtxcrit)); */
	/* Have to destroy all the weapons of this type in this section */
	DestroyWeapon(wounded, hitloc, critType, GetWeaponCrits(wounded, Weapon2I(critType)), critHit);
/*	if ((MechWeapons[Weapon2I(critType)].special & GAUSS) && !weapindx) {  Laf, I guess it doesn't matter now
	    mech_notify(wounded, MECHALL,
		"It explodes for 20 points damage");
	    if (LOS && wounded != attacker && attacker)
		mech_notify(attacker, MECHALL,
		    "Your target is covered with a large electrical discharge");
	    if (attacker)
		DamageMech(wounded, attacker, 0, -1, hitloc, 0, 0, 0, 20, -1, 7, 0);
	} else if (IsAMS(Weapon2I(critType))) {
	    MechStatus(wounded) &= ~AMS_ENABLED;
	    MechSpecials(wounded) &=
		~(IS_ANTI_MISSILE_TECH | CL_ANTI_MISSILE_TECH);
	} */
	if (IsAMS(Weapon2I(critType))) {
            MechStatus(wounded) &= ~AMS_ENABLED;
            MechSpecials(wounded) &= ~(IS_ANTI_MISSILE_TECH | CL_ANTI_MISSILE_TECH);
            }
	return 1;
    } else {
		int sum = 0, count = 0, destweap = 0;
		int firstcrit = WeaponFirstCrit(wounded, hitloc, critHit);
		int numcrits = GetWeaponCrits(wounded, Weapon2I(critType));

		mech_notify(wounded, MECHALL, tprintf("Your %s has been hit!!", &MechWeapons[Weapon2I(critType)].name[3]));
		if (LOS && wounded != attacker && attacker)
		    mech_notify(attacker, MECHALL, tprintf("You hit a %s!!", &MechWeapons[Weapon2I(critType)].name[3]));
		/* flooded weapons */
		if (PartIsDisabled(wounded, hitloc, critHit) && !PartTempNuke(wounded, hitloc, critHit))
		    destweap = 1;
		else if (IsAMS(Weapon2I(critType))) {
		    MechStatus(wounded) &= ~AMS_ENABLED;
		    MechSpecials(wounded) &= ~(IS_ANTI_MISSILE_TECH|CL_ANTI_MISSILE_TECH);
		    destweap = 1;
		} else {
		    mtxcrit = WeaponIsNonfunctional(wounded, hitloc, firstcrit, numcrits);
		    DisablePart(wounded, hitloc, critHit);
		    if (mtxcrit == 1) { 
			if (Roll() == 12) {
			    DestroyPart(wounded, hitloc, critHit);
			} else {
			    SetPartTempNuke(wounded, hitloc, critHit, DESTROYED_DAMAGE);
			}
		    } else if (mtxcrit <= 0) { 
			switch (Roll() - mtxcrit) {
			    case 2:
			    case 3:
				SetPartTempNuke(wounded, hitloc, critHit, MINIMAL_DAMAGE);
				break;
			    case 4:
			    case 5:
				SetPartTempNuke(wounded, hitloc, critHit, LIGHT_DAMAGE);
				break;
			    case 6:
			    case 7:
				SetPartTempNuke(wounded, hitloc, critHit, MEDIUM_DAMAGE);
				break;
			    case 8:
			    case 9:
				SetPartTempNuke(wounded, hitloc, critHit, HEAVY_DAMAGE);
				break;
			    case 10:
			    case 11:
				mech_notify(wounded, MECHALL, "The weapon ceases to function!!");
				DestroyPart(wounded, hitloc, critHit);
				SetPartTempNuke(wounded, hitloc, critHit, DESTROYED_DAMAGE);
				break;
			    default:
				destweap = 1;
				break;
			    }
			}
			/* Now check again, and destroy if necessary */
		    while (sum < numcrits) {
			if (PartIsDestroyed(wounded, hitloc, (firstcrit + sum)) || PartTempNuke(wounded, hitloc, (firstcrit + sum)))
				count++;
			    sum++;
			}
		    if (count > numcrits / 2)
			destweap = 1;
		}
		/* weapon go boom! */
		if (destweap)
		    SlagWeapon(wounded, attacker, LOS, hitloc, critHit, critType);
		return 1;
		}
#else
	int firstcrit = WeaponFirstCrit(wounded, hitloc, critHit);
	int numcrits = GetWeaponCrits(wounded, Weapon2I(critType));
	int stat_before, stat_after;

	stat_before = WeaponIsNonfunctional(wounded, hitloc, firstcrit, numcrits);
	DestroyWeapon(wounded, hitloc, critType, GetWeaponCrits(wounded, Weapon2I(critType)), critHit);
	mech_notify(wounded, MECHALL, tprintf("Your %s has been hit!!", &MechWeapons[Weapon2I(critType)].name[3]));
	stat_after = WeaponIsNonfunctional(wounded, hitloc, firstcrit, numcrits);

	if (stat_before <= 0 && stat_after > 0)
	    mech_notify(wounded, MECHALL, tprintf("Your %s is no longer operational.", &MechWeapons[Weapon2I(critType)].name[3]));
	else if (stat_before > 0 && stat_after > 0)
	    mech_notify(wounded, MECHALL, tprintf("Your %s is smashed up and takes more smashing.", &MechWeapons[Weapon2I(critType)].name[3]));
	else if (stat_before <= 0 && stat_after <= 0)
	    mech_notify(wounded, MECHALL, tprintf("Your %s takes damage but is still operational.", &MechWeapons[Weapon2I(critType)].name[3]));

	if (LOS && wounded != attacker && attacker)
	    mech_notify(attacker, MECHALL, tprintf("You hit a %s!!", &MechWeapons[Weapon2I(critType)].name[3]));
	if (IsAMS(Weapon2I(critType))) {
	    MechStatus(wounded) &= ~AMS_ENABLED;
	    MechSpecials(wounded) &= ~(IS_ANTI_MISSILE_TECH|CL_ANTI_MISSILE_TECH);
	    return 1;
	    }
	if (stat_after <= 0) {
	    int effect = 0;
	    int weaptype = MechWeapons[Weapon2I(critType)].type;

	    switch (Roll() + abs(stat_after)) {
		case 2:
		case 3:
		    break;
		case 4:
		case 5:
		   effect |= MTX_TOHIT1;
		    mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s takes Moderate Damage :%%cn cumulative +1 BTH.",
			&MechWeapons[Weapon2I(critType)].name[3]));
		   break;
		case 6:
		case 7:
		    if (weaptype == TBEAM) {
			effect |= MTX_TODAM1|MTX_TOHIT1_RANGE;
			    mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s takes Focus Misalignment damage :%%cn cumulative -1 Damage and +1 BTH at medium and long range.",
				&MechWeapons[Weapon2I(critType)].name[3]));
		    } else if (weaptype == TAMMO) {
			effect |= MTX_JAM1;
		        mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s takes Barrell Damage :%%cn Roll to jam on low BTH.",
			    &MechWeapons[Weapon2I(critType)].name[3]));
	 	    } else if (weaptype == TMISSILE) {
			effect |= MTX_TOHIT1_RANGE;
			mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s takes damage to ranging its' systems :%%cn cumulative +1 BTH at medium and long range.",
			    &MechWeapons[Weapon2I(critType)].name[3]));
		    }
		    break;
		case 8:
		case 9:
		    if (weaptype == TBEAM) {
			effect |= MTX_HEAT1|MTX_BOOM1;
			mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s takes Crystal Damage :%%cn cumulative +1 Heat and roll or explode on low BTH.",
			    &MechWeapons[Weapon2I(critType)].name[3]));
		    } else {
			effect |= MTX_MODELOCK|MTX_BOOM1;
			mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s takes ammo feed damage :%%cn cumulative roll or explode on low BTH and cannot change ammo modes.",
			    &MechWeapons[Weapon2I(critType)].name[3]));
		    }
		    break;
		case 10:
		case 11:
		case 12:
		    mech_notify(wounded, MECHALL, tprintf("%%ch%%crYour %s can't handle the damage rate and slags :%%cn The weapon is toast!",
			    &MechWeapons[Weapon2I(critType)].name[3]));
		    SlagWeapon(wounded, attacker, LOS, hitloc, critHit, critType);
		    return 1;
		}
	    int tmp = GetPartDamage(wounded, hitloc, firstcrit);
	    if (effect & MTX_TOHIT1) {
		if (tmp & MTX_TOHIT3)
		    effect |= MTX_TOHIT4;
		if (tmp & MTX_TOHIT2)
		    effect |= MTX_TOHIT3;
		if (tmp & MTX_TOHIT1)
		    effect |= MTX_TOHIT2;
	    }
	    if (effect & MTX_TODAM1) {
		if (tmp & MTX_TODAM3)
		    effect |= MTX_TODAM4;
		if (tmp & MTX_TODAM2)
		    effect |= MTX_TODAM3;
		if (tmp & MTX_TODAM1)
		    effect |= MTX_TODAM2;
	    }
	    if (effect & MTX_TOHIT1_RANGE) {
		if (tmp & MTX_TOHIT3_RANGE)
		    effect |= MTX_TOHIT4_RANGE;
		if (tmp & MTX_TOHIT2_RANGE)
		    effect |= MTX_TOHIT3_RANGE;
		if (tmp & MTX_TOHIT1_RANGE)
		    effect |= MTX_TOHIT2_RANGE;
	    }
	    if (effect & MTX_HEAT1) {
		if (tmp & MTX_HEAT3)
		    effect |= MTX_HEAT4;
		if (tmp & MTX_HEAT2)
		    effect |= MTX_HEAT3;
		if (tmp & MTX_HEAT1)
		    effect |= MTX_HEAT2;
	    }
	    if (effect & MTX_BOOM1) {
		if (tmp & MTX_BOOM3)
		    effect |= MTX_BOOM4;
		if (tmp & MTX_BOOM2)
		    effect |= MTX_BOOM3;
		if (tmp & MTX_BOOM1)
		    effect |= MTX_BOOM2;
	    }
	    if (effect & MTX_JAM1) {
		if (tmp & MTX_JAM3)
		    effect |= MTX_JAM4;
		if (tmp & MTX_JAM2)
		    effect |= MTX_JAM3;
		if (tmp & MTX_JAM1)
		    effect |= MTX_JAM2;
	    }
	GetPartDamage(wounded, hitloc, firstcrit) |= effect;
	return 1;
	} 
#endif
	}

    if (IsSpecial(critType)) {
	destroycrit = 1;
        if (destroycrit && (IsActuator(critType) || Special2I(critType) == SENSORS))
            DestroyPart(wounded, hitloc, critHit);
	switch (Special2I(critType)) {
	case LIFE_SUPPORT:
	    MechCritStatus(wounded) |= LIFE_SUPPORT_DESTROYED;
	    if (notif)
	            mech_notify(wounded, MECHALL, "Your life support has been destroyed!!");
	    if (realcrit)
		    MechLOSBroadcast(wounded, "erupts a plume of near transparent smoke from it's life support system!");

		if ((MechRTerrain(wounded) == WATER
		 && (MechZ(wounded) <= -2 || (Fallen(wounded) && MechZ(wounded) <= -1)))
	 	 || MechRTerrain(wounded) == HIGHWATER)
		 	CheckNoAir(wounded);
	    break;
	case COCKPIT:
	    /* Destroy Mech for now, but later kill pilot as well */
	    if (notif)
		mech_notify(wounded, MECHALL, "Your cockpit is destroyed!!  Your body is fried!!!");
	    if (!Destroyed(wounded)) {
/*
		Destroy(wounded);
		SendDebug(tprintf("#%d has been killed by #%d",
			wounded->mynum,
			attacker ? attacker->mynum : wounded->mynum));
*/
		DestroyMech(wounded, attacker ? attacker : wounded, 1);
	    }
	    if (LOS && attacker)
		mech_notify(attacker, MECHALL,
		    "You destroy the cockpit!!  The pilot's blood splatters down the sides!!!");
            if (realcrit)
		    MechLOSBroadcast(wounded, "halts eerily as a hint of red can be seen through the cockpit glass!");
	    MechPilot(wounded) = -1;
	    KillMechContentsIfIC(wounded->mynum, wounded);
	    break;
	case SENSORS:
	    if (!(MechCritStatus(wounded) & SENSORS_DAMAGED)) {
		MechLRSRange(wounded) /= 2;
		MechTacRange(wounded) /= 2;
		MechScanRange(wounded) /= 2;
		MechBTH(wounded) += 2;
		MechCritStatus(wounded) |= SENSORS_DAMAGED;
		if (notif)
	 	    mech_notify(wounded, MECHALL, "Your sensors have been damaged!!");
	    } else {
		MechLRSRange(wounded) = 0;
		MechTacRange(wounded) = 0;
		MechScanRange(wounded) = 0;
		MechBTH(wounded) += (MechSpecials2(wounded) & TORSOCOCKPIT_TECH ? 2 : 73);
		if (MechSpecials2(wounded) & TORSOCOCKPIT_TECH) {
		    if (notif)
 		        mech_notify(wounded, MECHALL, "Your sensors have been further damaged!");
		    MechPilotSkillBase(wounded) += 4;
		} else {
		if (notif)
		    mech_notify(wounded, MECHALL, "Your sensors have been destroyed!!");
		}
	    }
	    if (MechSpecials2(wounded) & TORSOCOCKPIT_TECH)
		if ((GetPartType(wounded, HEAD, 0) == I2Special(SENSORS)) && (PartIsNonfunctional(wounded, HEAD, 0)) &&
		    (GetPartType(wounded, HEAD, 4) == I2Special(SENSORS)) && (PartIsNonfunctional(wounded, HEAD, 4)))
			MechPilotSkillBase(wounded) += 4;
            if (realcrit)
		    MechLOSBroadcast(wounded, "stumbles momentarily as a chaotic electric storm englufs it!");
	    break;
	case HEAT_SINK:
	    if (MechSpecials(wounded) & DOUBLE_HEAT_TECH) {
	      MechRealNumsinks(wounded) -= 2;
	      DestroyWeapon(wounded, hitloc, critType, (char) 3, critHit);
	      destroycrit = 0;
	    } else if (MechSpecials(wounded) & CLAN_TECH) {
	      MechRealNumsinks(wounded) -= 2;
	      DestroyWeapon(wounded, hitloc, critType, (char) 2, critHit);
	      destroycrit = 0;
	    } else if (MechSpecials2(wounded) & COMPACT_HEAT_TECH) {
	      MechRealNumsinks(wounded) -= 2;
	    } else
	      MechRealNumsinks(wounded)--;
	    if (notif)
	      mech_notify(wounded, MECHALL, "You lost a heat sink!");
            if (realcrit)
	      MechLOSBroadcast(wounded, "spews gas and green fluid as its armor plating is gashed open!");
	    break;
	case JUMP_JET:
	    MechJumpSpeed(wounded) -= MP1;
	    if (MechJumpSpeed(wounded) < 0)
		MechJumpSpeed(wounded) = 0;
	    if (notif)
	        mech_notify(wounded, MECHALL, "One of your jump jet engines has shut down!");
	    if (attacker && MechJumpSpeed(wounded) < MP1 &&
		Jumping(wounded)) {
		mech_notify(wounded, MECHALL,
		    "Losing your last Jump Jet you fall from the sky!!!!!");
  	 	MechLOSBroadcast(wounded, "falls from the sky!");
		MechFalls(wounded, 1, 0);
		domino_space(wounded, 2);
	    }
            if (realcrit)
		    MechLOSBroadcast(wounded, "flares out a tendril of searing plasma!");
	    break;
	case ENGINE:
	    if (MechEngineHeat(wounded) < 10) {
		MechEngineHeat(wounded) += 5;
		if (MechSpecials(wounded) & ICE_TECH) {
		    mech_notify(wounded, MECHALL, "Your engine takes a hit!  You lose some power!!");
		    CalcLegCritSpeed(wounded);
		    if (realcrit)
			MechLOSBroadcast(wounded, "starts spewing black smoke from its torso!");
		} else {
		mech_notify(wounded, MECHALL, "Your engine shielding takes a hit!  It's getting hotter in here!!");
	        if (realcrit)
			MechLOSBroadcast(wounded, "experiences a mild electromagnetic discharge!");
		}
	    } else if (MechEngineHeat(wounded) < 15) {
		MechEngineHeat(wounded) = 15;
		mech_notify(wounded, MECHALL, "Your engine is destroyed!!");
		if (wounded != attacker &&
		    !(MechStatus(wounded) & DESTROYED) && attacker)
		    mech_notify(attacker, MECHALL, "You destroy the engine!!");
                if (realcrit) {
		    if (MechSpecials(wounded) & ICE_TECH)
			MechLOSBroadcast(wounded, "spews more smoke then comes to a standstill!");
		    else
			MechLOSBroadcast(wounded, "spooges plasma all over itself!");
		}
	    }
	    if (realcrit && !Destroyed(wounded) && MechSpecials(wounded) & ICE_TECH &&
		Roll() >= (MechEngineHeat(wounded) >= 15 ? 10 : MechEngineHeat(wounded) >= 10 ? 11 : 12)) {
		MAP *boomap = FindObjectsData(wounded->mapindex);

		mech_notify(wounded, MECHALL, "Uh oh....");
		MechLOSBroadcast(wounded, "blows sky high in a pillar of fire!");
                if (!MapIsUnderground(boomap))
                    autoeject(MechPilot(wounded), wounded, attacker);
                DestroySection(wounded, attacker, LOS, RTORSO);
                DestroySection(wounded, attacker, LOS, CTORSO);
                DestroySection(wounded, attacker, LOS, HEAD);
		DestroySection(wounded, attacker, LOS, LLEG);
		DestroySection(wounded, attacker, LOS, RLEG);
                break;
	 	}
 	    if (MechEngineHeat(wounded) >= 15)
		DestroyMech(wounded, attacker, 1);
	    break;
	case TARGETING_COMPUTER:
	    if (!(MechCritStatus(wounded) & TC_DESTROYED)) {
	        if (notif)
		    mech_notify(wounded, MECHALL, "Your Targeting Computer is Destroyed!");
		MechCritStatus(wounded) |= TC_DESTROYED;
	    }
            if (realcrit)
		    MechLOSBroadcast(wounded, "jiggles as some electronic boards can be seen through rented armor!");
	    break;
	case NULLSIG:
	    if (!(MechCritStatus(wounded) & NULLSIG_DESTROYED)) {
		if (notif)
		    mech_notify(wounded, MECHALL, "Your Null Signature Device is damaged and knocked out!");
		MechCritStatus(wounded) |= NULLSIG_DESTROYED;
		MechStatus2(wounded) &= ~NULLSIG_ACTIVE;
	    } else {
		if (notif)
	            mech_notify(wounded, MECHALL, "Part of your Null Signature system is further damaged!");
		}
	    if (realcrit)
		    MechLOSBroadcast(wounded, "is enveloped is vapor and electricity as it's baffles are blown off!");
	    break;
/*	case STEALTHARM:
	    if (!(MechCritStatus2(wounded) & STEALTHARM_DESTROYED)) {
		if (notif)
		    mech_notify(wounded, MECHALL, "Your Stealth Armor is damaged and knocked out!");
		MechCritStatus2(wounded) |= STEALTHARM_DESTROYED;
		MechStatus2(wounded) &= ~STEALTHARM_ACTIVE;
	    } else {
		if (notif)
	            mech_notify(wounded, MECHALL, "Part of your Stealth Armor is further damaged!");
		}
	    if (realcrit)
		    MechLOSBroadcast(wounded, "is enveloped is vapor and electricity as electronic in it's armor are damaged!");
	    break; */
	case GYRO:
	    if ((MechSpecials2(wounded) & HDGYRO_TECH) && !(MechCritStatus(wounded) & HDGYRO_SPONGE)) {
		MechCritStatus(wounded) |= HDGYRO_SPONGE;
		MechPilotSkillBase(wounded) += 2;
		if (notif)
		    mech_notify(wounded, MECHALL, "Your heavy gyro sponges some damage!");
	    } else if (!(MechCritStatus(wounded) & GYRO_DAMAGED)) {
		MechCritStatus(wounded) |= GYRO_DAMAGED;
		if (MechSpecials2(wounded) & HDGYRO_TECH)
		    MechPilotSkillBase(wounded) += 1;
		else
		    MechPilotSkillBase(wounded) += 3;
		if (notif)
		    mech_notify(wounded, MECHALL, "Your Gyro has been damaged!");
		if (attacker)
		    if (!MadePilotSkillRoll(wounded, 0) &&
			!Fallen(wounded)) {
			if (!Jumping(wounded) && !OODing(wounded)) {
			    mech_notify(wounded, MECHALL,
				"You lose your balance and fall down!");
			    MechLOSBroadcast(wounded,
				"stumbles and falls down.");
			    MechFalls(wounded, 1, 0);
			} else {
			    mech_notify(wounded, MECHALL,
				"You fall from the sky!!!!!");
			    MechLOSBroadcast(wounded,
				"falls from the sky!");
			    MechFalls(wounded, JumpSpeedMP(wounded, map),
				0);
			    domino_space(wounded, 2);
			}
		    }
	    } else if (!(MechCritStatus(wounded) & GYRO_DESTROYED)) {
		MechCritStatus(wounded) |= GYRO_DESTROYED;
		mech_notify(wounded, MECHALL,
		    "Your Gyro has been destroyed!!");
		/*SetMaxSpeed(wounded, 0.0); -- BAH feg code -- Reb */
			CalcLegCritSpeed(wounded);
		if (attacker) {
		    if (!Fallen(wounded) && !Jumping(wounded) &&
			!OODing(wounded)) {
			mech_notify(wounded, MECHALL,
			    "You fall and you can't get up!!");
			MechLOSBroadcast(wounded, "is knocked over!");
			MechFalls(wounded, 1, 0);
		    } else if (!Fallen(wounded)
			&& (Jumping(wounded) || OODing(wounded))) {
			mech_notify(wounded, MECHALL,
			    "You fall from the sky!!!!!");
			MechLOSBroadcast(wounded, "falls from the sky!");
			MechFalls(wounded, JumpSpeedMP(wounded, map), 0);
			domino_space(wounded, 2);
		    }
		}
	    } else {
		mech_notify(wounded, MECHALL,
		    "Your destroyed gyro takes another hit!");
	    }
            if (realcrit)
		    MechLOSBroadcast(wounded, "drips mercury out it's side as you notice moving machinery through a hole.");
	    break;
	case SHOULDER_OR_HIP:
	    if (MechMove(wounded) != MOVE_QUAD && (hitloc == LARM ||
		    hitloc == RARM)) {
		MechSections(wounded)[hitloc].basetohit = 4;
		if (notif)
		    mech_notify(wounded, MECHALL, "Your shoulder joint takes a hit and is frozen!");
	    } else if (hitloc == RLEG || hitloc == LLEG ||
		(MechMove(wounded) == MOVE_QUAD && (hitloc == LARM ||
			hitloc == RARM))) {
		if (!(MechCritStatus(wounded) & HIP_DAMAGED)) {
		    MechCritStatus(wounded) |= HIP_DAMAGED;
	         /* DivideMaxSpeed(wounded, 2); */
		    CalcLegCritSpeed(wounded);
		    MechPilotSkillBase(wounded) += 2;
		    if (notif)
		        mech_notify(wounded, MECHALL, "Your hip takes a direct hit and freezes up!!");
		} else if (MechCritStatus(wounded) & HIP_DAMAGED) {
			if (MechMove(wounded) == MOVE_QUAD) {
		             /* DivideMaxSpeed(wounded, 2); */
				CalcLegCritSpeed(wounded);
				MechPilotSkillBase(wounded) += 2;
				if (notif)
				    mech_notify(wounded, MECHALL, "Another hip takes a direct hit and freezes up!!");
				} else {
			     /* SetMaxSpeed(wounded, 0.0); */
				CalcLegCritSpeed(wounded);
				MechPilotSkillBase(wounded) += 2;
				if (notif)
				    mech_notify(wounded, MECHALL, "Both your hips have been destroyed!! You are immobilized!");
				}
			    }
		}
            if (realcrit)
		    MechLOSBroadcast(wounded, "loses some coordination as you notice a joint bearing myomer bundles is exposed!");
	    break;
	case LOWER_ACTUATOR:
	    if (MechMove(wounded) != MOVE_QUAD && (hitloc == LARM ||
		    hitloc == RARM)) {
		if (!LocHasJointCrit(wounded, hitloc))
			MechSections(wounded)[hitloc].basetohit += 1;
		if (notif)
		    mech_notify(wounded, MECHALL, tprintf("Your lower %s arm actuator is destroyed!!",
			hitloc == LARM ? "left" : "right"));
	    } else if (hitloc == RLEG || hitloc == LLEG ||
		(MechMove(wounded) == MOVE_QUAD && (hitloc == LARM ||
			hitloc == RARM))) {
		if (MechSections(wounded)[hitloc].basetohit < 3) {
		    MechSections(wounded)[hitloc].basetohit += 1;
		    MechPilotSkillBase(wounded) += 1;
	        /*  LowerMaxSpeed(wounded, MP1); */
		    CalcLegCritSpeed(wounded);
		    if (notif)
		        mech_notify(wounded, MECHALL, "One of your leg actuators is destroyed!!");
		    if (attacker && !Jumping(wounded) && !OODing(wounded)
			&& !MadePilotSkillRoll(wounded, 0) && !Fallen(wounded)) {
			mech_notify(wounded, MECHALL,
			    "You lose your balance and fall down!");
			MechLOSBroadcast(wounded,
			    "stumbles and falls down!");
			MechFalls(wounded, 1, 0);
		    }
		}
	    }
            if (realcrit)
		    MechLOSBroadcast(wounded, "loses some coordination as you notice a limb bearing myomer bundles is exposed!");
	    break;
	case UPPER_ACTUATOR:
	    if (MechMove(wounded) != MOVE_QUAD && (hitloc == LARM ||
		    hitloc == RARM)) {
		if (!(LocHasJointCrit(wounded, hitloc)))
			MechSections(wounded)[hitloc].basetohit += 1;
		if (notif)
		    mech_notify(wounded, MECHALL, tprintf("Your %s upper arm actuator is destroyed!!",
			hitloc == LARM ? "left" : "right"));
	    } else if (hitloc == RLEG || hitloc == LLEG ||
		(MechMove(wounded) == MOVE_QUAD && (hitloc == LARM ||
			hitloc == RARM))) {
		if (MechSections(wounded)[hitloc].basetohit < 3) {
		    MechSections(wounded)[hitloc].basetohit += 1;
		    MechPilotSkillBase(wounded) += 1;
		 /* LowerMaxSpeed(wounded, MP1); */
		    CalcLegCritSpeed(wounded);
		    if (notif)
		        mech_notify(wounded, MECHALL, "One of your leg actuators is destroyed!!");
		    if (attacker && !Jumping(wounded) && !OODing(wounded)
			&& !MadePilotSkillRoll(wounded, 0) &&
			!Fallen(wounded)) {
			mech_notify(wounded, MECHALL,
			    "You lose your balance and fall down!");
			MechLOSBroadcast(wounded,
			    "loses balance and falls down!");
			MechFalls(wounded, 1, 0);
		    }
		}
	    }
            if (realcrit)
		    MechLOSBroadcast(wounded, "loses some coordination as you notice a limb bearing myomer bundles is exposed!");
	    break;
	case HAND_OR_FOOT_ACTUATOR:
	    if (MechMove(wounded) != MOVE_QUAD && (hitloc == LARM || hitloc == RARM)) {
		    if (notif)
		        mech_notify(wounded, MECHALL, "Your hand actuator is destroyed!!");
			if (MechCarrying(wounded) > 0)
				mech_dropoff(GOD, wounded, "");
		}
	    if (hitloc == LLEG || hitloc == RLEG ||
		(MechMove(wounded) == MOVE_QUAD && (hitloc == LARM ||
			hitloc == RARM))) {
		if (MechSections(wounded)[hitloc].basetohit < 3) {
		    MechSections(wounded)[hitloc].basetohit += 1;
		    MechPilotSkillBase(wounded) += 1;
	 	 /* LowerMaxSpeed(wounded, MP1); */
		    CalcLegCritSpeed(wounded);
		    if (notif)
		        mech_notify(wounded, MECHALL, "Your foot actuator is destroyed!!");
		    if (attacker && !Jumping(wounded) && !OODing(wounded)
			&& !MadePilotSkillRoll(wounded, 0) &&
			!Fallen(wounded)) {
			mech_notify(wounded, MECHALL,
			    "You lose your balance and fall down!");
			MechLOSBroadcast(wounded,
			    "loses balance and falls down!");
			MechFalls(wounded, 1, 0);
		    }
		}
	    }
            if (realcrit)
		    MechLOSBroadcast(wounded, "loses some coordination as you notice a limb bearing myomer bundles is exposed!");
	    break;
	case C3_MASTER:
	case C3_SLAVE:
	case C3I:
	    MechCritStatus(wounded) |= C3_DESTROYED;
	    if (notif)
	        mech_notify(wounded, MECHALL, "Your C3 system has been destroyed!");
            if (realcrit)
		    MechLOSBroadcast(wounded, "loses a few blinking lights as some large electronic equipment is destroyed!");
	    break;
	case ECM:
	    MechCritStatus(wounded) |= ECM_DESTROYED;
	    if (notif)
	        mech_notify(wounded, MECHALL, "Your GuardianECM system has been destroyed!");
	    MechStatus(wounded) &= ~ECM_ACTIVE;
	    MechStatus2(wounded) &= ~STEALTHARM_ACTIVE;
            if (realcrit)
		    MechLOSBroadcast(wounded, "is maimed as a large electronic orb is sheared off!");
	    break;
        case ANGEL_ECM:
            MechCritStatus(wounded) |= ECM_DESTROYED;
	    if (notif)
                mech_notify(wounded, MECHALL, "Your Angel ECM system has been destroyed!");
            MechStatus(wounded) &= ~ECM_ACTIVE;
	    MechStatus2(wounded) &= ~STEALTHARM_ACTIVE;
            if (realcrit)
                    MechLOSBroadcast(wounded, "is maimed as a large electronic orb is sheared off!");
            break;
	case BEAGLE_PROBE:
	    MechCritStatus(wounded) |= BEAGLE_DESTROYED;
	    MechSpecials(wounded) &= ~BEAGLE_PROBE_TECH;
	    if (notif)
 		mech_notify(wounded, MECHALL, "Your Beagle Probe has been destroyed!");
	    if (sensors[(short) MechSensor(wounded)[0]].required_special ==
		BEAGLE_PROBE_TECH ||
		sensors[(short) MechSensor(wounded)[1]].required_special ==
		BEAGLE_PROBE_TECH) {
		if (sensors[(short)
			MechSensor(wounded)[0]].required_special ==
		    BEAGLE_PROBE_TECH) MechSensor(wounded)[0] = 0;
		if (sensors[(short)
			MechSensor(wounded)[1]].required_special ==
		    BEAGLE_PROBE_TECH) MechSensor(wounded)[1] = 0;
		MarkForLOSUpdate(wounded);
	    }
            if (realcrit)
	            MechLOSBroadcast(wounded, "is maimed as a small electronic orb is sheared off!");
	    break;
        case BLOODHOUND_PROBE:
            MechCritStatus(wounded) |= BEAGLE_DESTROYED;
            MechSpecials2(wounded) &= ~BLOODHOUND_PROBE_TECH;
	    if (notif)
                mech_notify(wounded, MECHALL, "Your Bloodhound Probe has been destroyed!");
            if (sensors[(short) MechSensor(wounded)[0]].required_special ==
                BLOODHOUND_PROBE_TECH ||
                sensors[(short) MechSensor(wounded)[1]].required_special ==
                BLOODHOUND_PROBE_TECH) {
                if (sensors[(short)
                        MechSensor(wounded)[0]].required_special ==
                    BLOODHOUND_PROBE_TECH) MechSensor(wounded)[0] = 0;
                if (sensors[(short)
                        MechSensor(wounded)[1]].required_special ==
                    BLOODHOUND_PROBE_TECH) MechSensor(wounded)[1] = 0;
                MarkForLOSUpdate(wounded);
            }
            if (realcrit)
                    MechLOSBroadcast(wounded, "is maimed as a small electronic orb is sheared off!");
            break;
	case ARTEMIS_IV:
	    weapon_slot = GetPartData(wounded, hitloc, critHit);
	    if (weapon_slot > NUM_CRITICALS) {
		SendError(tprintf("Artemis IV error on mech %d",
			wounded->mynum));
		break;
	    }
	    GetPartMode(wounded, hitloc, weapon_slot) &= ~ARTEMIS_MODE;
	    if (notif)
        	    mech_notify(wounded, MECHALL, "Your Artemis IV system has been destroyed!");
            if (realcrit)
		    MechLOSBroadcast(wounded, "is maimed as a electronics orb near it's missile rack is sheared off!");
	    break;
	case AXE:
	    if (notif)
	        mech_notify(wounded, MECHALL, "Your axe has been destroyed!");
 	    if (realcrit)
		    MechLOSBroadcast(wounded, "loses it's axe as it's shaft is broken!");
	    break;
	case SWORD:
	    if (notif)
  	        mech_notify(wounded, MECHALL, "Your sword has been destroyed!");
            if (realcrit)
		    MechLOSBroadcast(wounded, "loses it's sword as the hilt is broken!");
	    break;
	case MACE:
	    if (notif)
	        mech_notify(wounded, MECHALL, "Your mace has been destroyed!");
            if (realcrit)
		    MechLOSBroadcast(wounded, "stumbles as it's cumbersome mace is removed!");
	    break;
	case DS_AERODOOR:
 	    mech_notify(wounded, MECHALL, "One of the aero doors is destroyed useless!");
	    break;
	case DS_MECHDOOR:
	    mech_notify(wounded, MECHALL,
		"One of the 'mech doors is destroyed useless!");
	    break;
	case SUPERCHARGER:
	    if (notif)
	        mech_notify(wounded, MECHALL, "Your supercharger device is destryoed!");
	    MechSpecials2(wounded) &= ~SCHARGE_TECH;
	    MechStatus2(wounded) &= ~SCHARGE_ENABLED;
	    if (realcrit)
		MechLOSBroadcast(wounded, "squirts some slimey purple fluid which ignites in a plume of flame.");
	    break;
	}

	if (destroycrit)
	    DestroyPart(wounded, hitloc, critHit);
    }
    return 1;
}

void HandleCritical(MECH * wounded, MECH * attacker, int LOS, int hitloc,
    int num, int realcrit, int tac)
{
    int i;
    int critHit;
    int critType, critData;
    int count, index;
    int critList[NUM_CRITICALS];

    if (MechType(wounded) == CLASS_BSUIT)
	return;
    if (MechStatus(wounded) & COMBAT_SAFE)
	return;
    if (MechSpecials(wounded) & CRITPROOF_TECH)
	return;
    if (MechType(wounded) == CLASS_MW && Number(1, 2) == 1)
	return;
    if (MechType(wounded) != CLASS_MECH && !mudconf.btech_vcrit)
	return;
    /* AccumulateGunXPold(MechPilot(attacker), attacker, wounded, num, 3, -1, 7);*/
    if (is_aero(wounded)) {
	for (i = 0; i < num; i++)
	    HandleAeroCrit(wounded, attacker, LOS, hitloc, num);
	return;
	}
    if (MechType(wounded) == CLASS_VEH_GROUND ||
	MechType(wounded) == CLASS_VEH_NAVAL ||
	 (mudconf.btech_fasacrit == -1 && MechType(wounded) == CLASS_VEH_VTOL)) {
	if (mudconf.btech_fasacrit == 0) {
	    for (i = 0; i < num; i++)
		HandleVehicleCrit(wounded, attacker, LOS, hitloc, num);
	    return;
	} else if (mudconf.btech_fasacrit > 0) {
	    for (i = 0; i < num; i++)
		HandleFasaVehicleCrit(wounded, attacker, LOS, hitloc, num);
	    return;
	} else if (mudconf.btech_fasacrit < 0) {
	    for (i = 0; i < num; i++)
		HandleMaxTechVehicleCrit(wounded, attacker, LOS, hitloc, num);
	    return;
	}
    }
    if (IsDS(wounded))
	return;
    if (MechType(wounded) == CLASS_VEH_VTOL && mudconf.btech_fasacrit > -1) {
	for (i = 0; i < num; i++)
	    HandleVTOLCrit(wounded, attacker, LOS, hitloc, num);
	return;
    }
    while (num > 0) {
	count = 0;
	while (count == 0) {
	    for (i = 0; i < NUM_CRITICALS; i++) {
		critType = GetPartType(wounded, hitloc, i);
		if (!PartIsDestroyed(wounded, hitloc, i)
		    && critType != EMPTY && critType != Special(CASE)
		    && critType != Special(FERRO_FIBROUS)
		    && critType != Special(ENDO_STEEL)
		    && critType != Special(TRIPLE_STRENGTH_MYOMER)
		    && critType != Special(MASC)
		    && critType != Special(STEALTHARM)
		    && critType != Special(HARDPOINT)) {
		    critList[count] = i;
		    count++;
		}
	    }
	    if (!count)	{	/* transfer Crit to next location - no longer */
		if (!mudconf.btech_floatcrit)
		    return;
	    	if (tac)
	 	    return;
		if (TransferTarget(wounded, hitloc) != -1)
		    HandleCritical(wounded, attacker, LOS, TransferTarget(wounded, hitloc), num, realcrit, tac);
		return;
		}
	}

	index = random() % count;
	critHit = critList[index];	/* This one should be linear */

	critType = GetPartType(wounded, hitloc, critHit);
	critData = GetPartData(wounded, hitloc, critHit);

	if (HandleMechCrit(wounded, attacker, LOS, hitloc, critHit, critType, critData, realcrit, 1))
	    num--;
    }
}
