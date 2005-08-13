#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "p.mech.ice.h"
#include "p.mech.utils.h"
#include "p.mine.h"
#include "p.bsuit.h"
#include "p.mech.los.h"
#include "p.mech.update.h"
#include "p.mech.physical.h"
#include "p.mech.combat.h"
#include "p.btechstats.h"
#include "p.mech.hitloc.h"
#include "p.template.h"
#include "p.map.conditions.h"
#include "p.mech.maps.h"
#include "p.mech.pickup.h"
#include "btechstats.h"
#include "btechstats_global.h"

struct {
    char *name;
    char *full;
    int ofs;
} lateral_modes[] = {
    {
    "nw", "Front/Left", 300}, {
    "fl", "Front/Left", 300}, {
    "ne", "Front/Right", 60}, {
    "fr", "Front/Right", 60}, {
    "sw", "Rear/Left", 240}, {
    "rl", "Rear/Left", 240}, {
    "se", "Rear/Right", 120}, {
    "rr", "Rear/Right", 120}, {
    "-", "None", 0}, {
    NULL, 0}
};

const char *LateralDesc(MECH * mech)
{
    int i;

    for (i = 0; MechLateral(mech) != lateral_modes[i].ofs; i++);
    return lateral_modes[i].full;
}



void mech_lateral(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int i;

    cch(MECH_USUALO);
    if (!HasBoolAdvantage(player, "maneuvering_ace"))
        DOCHECK(!((MechType(mech) == CLASS_MECH &&
    	    MechMove(mech) == MOVE_QUAD) || (MechType(mech) == CLASS_VTOL) || (MechMove(mech) == MOVE_HOVER)),
    	    "You cannot alter your lateral movement!");
    DOCHECK((MechCritStatus(mech) & LEG_DESTROYED) && MechType(mech) == CLASS_MECH,
	 "Superglue your leg back on and we can talk.");
    skipws(buffer);
    for (i = 0; lateral_modes[i].name; i++)
	if (!strcasecmp(lateral_modes[i].name, buffer))
	    break;
    DOCHECK(!lateral_modes[i].name, "Invalid mode!");
    if (lateral_modes[i].ofs == MechLateral(mech)) {
	DOCHECK(!ChangingLateral(mech), "You are going that way already!");
	mech_notify(mech, MECHALL, "Lateral mode change aborted.");
	StopLateral(mech);
	return;
    }
    mech_notify(mech, MECHALL,
	tprintf("Wanted lateral movement mode changed to %s.",
	    lateral_modes[i].full));
    StopLateral(mech);
    MECHEVENT(mech, EVENT_LATERAL, mech_lateral_event, LATERAL_TICK, i);
}

void mech_eta(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    int argc, eta_x, eta_y;
    float fx, fy, range;
    int etahr, etamin;
    char *args[3];

    cch(MECH_USUAL);
    mech_map = getMap(mech->mapindex);
    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc == 1, "Invalid number of arguments!");
    switch (argc) {
    case 0:
	DOCHECK(!(MechTargX(mech) >= 0 &&
		MechTarget(mech) < 0),
	    "You have invalid default target for ETA!");
	eta_x = MechTargX(mech);
	eta_y = MechTargY(mech);
	break;
    case 2:
	eta_x = atoi(args[0]);
	eta_y = atoi(args[1]);
	break;
    default:
	notify(player, "Invalid arguments!");
	return;
    }
    MapCoordToRealCoord(eta_x, eta_y, &fx, &fy);
    range = FindRange(MechFX(mech), MechFY(mech), 0, fx, fy, 0);
    if (fabs(MechSpeed(mech)) < 0.1)
	mech_notify(mech, MECHALL,
	    tprintf
	    ("Range to hex (%d,%d) is %.1f.  ETA: Never, mech not moving.",
	 eta_x, eta_y, range));
    else {
	etamin = abs(range / (MechSpeed(mech) / KPH_PER_MP));
	etahr = etamin / 60;
	etamin = etamin % 60;
	mech_notify(mech, MECHALL,
	    tprintf("Range to hex (%d,%d) is %.1f.  ETA: %d:%d.", eta_x,
		eta_y, range, etahr, etamin));
    }
}

float MechCargoMaxSpeed(MECH * mech, float mspeed)
{
    int lugged = 0, mod = 2;
    MAP *map;
    MECH *c;

    if (MechCarrying(mech) > 0) {	/* Ug-lee! */
	MECH *t; 
	if ((t = getMech(MechCarrying(mech))))
	    if (!(MechCritStatus(t) & OWEIGHT_OK))
		MechCritStatus(mech) &= ~LOAD_OK;
	}
    if ((MechCritStatus(mech) & LOAD_OK) &&
	(MechCritStatus(mech) & OWEIGHT_OK) &&
	(MechCritStatus(mech) & SPEED_OK)) {
	mspeed = MechRMaxSpeed(mech);
	if (MechSpecials(mech) & TRIPLE_MYOMER_TECH) {
		if (MechHeat(mech) >= 9.)
			mspeed = (WalkingSpeed(mspeed) + MP1) * 1.5;
	}
	if ((MechStatus(mech) & MASC_ENABLED) || (MechStatus2(mech) & (SCHARGE_ENABLED|SPRINTING)))
    	mspeed = WalkingSpeed(mspeed) * (1.5 + (MechStatus(mech) & MASC_ENABLED ? 0.5 : 0.0) +
		(MechStatus2(mech) & SCHARGE_ENABLED ? 0.5 : 0.0) + (!MoveModeChange(mech) && MechStatus2(mech) & SPRINTING ? 0.5 : 0.0));

	if (!MoveModeChange(mech) && MechStatus2(mech) & SPRINTING && HasBoolAdvantage(MechPilot(mech), "speed_demon"))
            mspeed += MP1 / 2;
	if (MechSpecials(mech) & HARDA_TECH)
	    mspeed = (mspeed <= MP1 ? mspeed : mspeed - MP1); 
	if (InSpecial(mech) && InGravity(mech))
	    if ((map = FindObjectsData(mech->mapindex)))
		mspeed = mspeed * 100 / MAX(50, MapGravity(map));
	return mspeed;
    }

    MechRTonsV(mech) = get_weight(mech);

    if (!(MechCritStatus(mech) & LOAD_OK)) {
	if (MechCarrying(mech) > 0)
	    if ((c = getMech(MechCarrying(mech)))) {
		lugged = BOUNDED(1024, get_weight(c), 102400);
/*		if (MechSpecials(mech) & SALVAGE_TECH)
		    lugged = lugged / 3;
		if (MechSpecials2(mech) & CARRIER_TECH)
		    lugged = lugged / 2;
		if ((MechSpecials(mech) & TRIPLE_MYOMER_TECH) && (MechHeat(mech) >= 9.))
		    lugged = lugged / 2;*/
	    }
	lugged += MechCarriedCargo(mech);

	if (MechSpecials(mech) & CARGO_TECH)
	    lugged -= ((CargoSpace(mech) / 100) * 1024) * 2;

	if ((MechSpecials(mech) & TRIPLE_MYOMER_TECH) && MechHeat(mech) >= 9.)
	    lugged = lugged / 2;

	if (MechMove(mech) == MOVE_BIPED)
	    lugged -= MechTons(mech) * 1024 / 10;

	if (lugged < 0)
	    lugged = 0;

/*	if ((MechType(mech) == CLASS_MECH) || !(MechSpecials(mech) & SALVAGE_TECH))
	    mod = mod * 2;
	lugged += (MechCarriedCargo(mech) / (MechSpecials2(mech) & CARRIER_TECH ? 5 : 1)) * mod / 2;*/
	MechRCTonsV(mech) = lugged;
	MechCritStatus(mech) |= LOAD_OK;
    }
    if (Destroyed(mech))
	mspeed = 0.0;
    else {
	int mv = MechRCTonsV(mech);
	int sv = MechRTons(mech);

/*	if (mv == 1 && !Destroyed(mech))
	    mv = sv;
	else {
	    if (mv > sv)
		mv = mv + (mv - sv) / 2;
	    else
		mv = mv + (sv - mv) / 3;
	}*/
	/*For now, no overspeeding ; there _is_ underspeeding of sorts, though */
/*	if (3 * sv < (MechRCTonsV(mech) + mv))
	    mspeed = 0.0;
	else
	    mspeed =
		MechMaxSpeed(mech) * MechTons(mech) * 1024.0 / MAX(1024 *
		MechTons(mech) + MechRCTonsV(mech) / 3, (MAX(1024,
			mv + MechRCTonsV(mech))));*/
/*		if (mv > sv)
			mspeed = 0.0;
		else if (sv > 0)
			mspeed -= mspeed * mv / sv;*/
		if (mv > 0)
		mspeed = MechEngineSize(mech) * 1536 * MP1 / (mv + MAX(MechTons(mech) * 1024, sv));
    }
    MechRMaxSpeed(mech) = mspeed;
    if (is_aero(mech) || IsDS(mech)) {
	MechRMaxSpeed(mech) = ((WalkingSpeed(MechRMaxSpeed(mech)) - BombThrustLoss(mech)) * 1.5);
	if (MechRMaxSpeed(mech) < 0)
	    MechRMaxSpeed(mech) = KPH_PER_MP;
	}
    MechWalkXPFactor(mech) = MAX(1, (int) mspeed / MP1) * 2;
    MechCritStatus(mech) |= SPEED_OK;
    return MMaxSpeed(mech);
}

static int drop_cheat_flag = 0;

void mech_drop(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    float s1;
    int done = 0;
    int swarm = (MechIsSwarmed(mech) == 1);
    int i;

    cch(MECH_USUALO);
    DOCHECK(MechType(mech) == CLASS_BSUIT, "No crawling (yet)!");
    DOCHECK(MechType(mech) != CLASS_MECH &&
	MechType(mech) != CLASS_MW,
	"This vehicle cannot drop like a 'Mech.");
    DOCHECK(Fallen(mech), "You are already prone.");
    DOCHECK(Jumping(mech) ||
	OODing(mech), "You can't drop while jumping!");
    DOCHECK(Standing(mech), "You can't drop while trying to stand up!");

    s1 = MMaxSpeed(mech) / 3.0;
    if (swarm && MechType(mech) == CLASS_MECH) {
        if (MechSections(mech)[RLEG].recycle > 0 ||
	    MechSections(mech)[LLEG].recycle > 0 ||
	    MechSections(mech)[RARM].recycle > 0 ||
	    MechSections(mech)[LARM].recycle > 0) {
	mech_notify(mech, MECHALL,
	    "You have parts recycling.");
	return;
	} else {
	mech_notify(mech, MECHALL,
	    "You attempt to roll all over the ground and shake your swarmer!");
	}

	for (i = 0; i < NUM_SECTIONS; i++)
            SetRecycleLimb(mech, i, PHYSICAL_RECYCLE_TIME);

	if (MadePilotSkillRoll(mech, MechSpeed(mech) / MP1)) {
	    mech_notify(mech, MECHALL,
		"You hit the ground HARD!");
	    MechLOSBroadcast(mech, "drops to the ground!");
	    MechFalls(mech, 1000 + ((fabs(MechSpeed(mech)) > s1 * 2) ? 1 : 2), 1);
	    bsuit_stopswarmers(FindObjectsData(mech->mapindex), mech, 0);
	    done = 1;
	} else {
	    mech_notify(mech, MECHALL, "You fail in your maneuvering!");
	    MechLOSBroadcast(mech, "drops to the ground!");
	    MechFalls(mech, 1000 + ((fabs(MechSpeed(mech)) > s1 * 2) ? 1 : 2), 1);
	    done = 1;
 	} 
    } else {
    if (MechType(mech) != CLASS_MW && fabs(MechSpeed(mech)) > s1 * 2) {
	mech_notify(mech, MECHALL,
	    "You attempt a controlled drop while running.");
	if (MadePilotSkillRoll(mech, 2)) {
	    mech_notify(mech, MECHALL,
		"You hit the ground with minimal damage");
	    MechLOSBroadcast(mech, "drops to the ground!");
	} else {
	    mech_notify(mech, MECHALL, "You fall to the ground hard.");
	    MechFalls(mech, 1002, 1);
	    done = 1;
	}
    } else if (fabs(MechSpeed(mech)) > s1) {
	mech_notify(mech, MECHALL,
	    "You attempt a controlled drop from your fast walk.");
	if (MadePilotSkillRoll(mech, 0)) {
	    mech_notify(mech, MECHALL,
		"You hit the ground with minimal damage");
	    MechLOSBroadcast(mech, "drops to the ground!");
	} else {
	    mech_notify(mech, MECHALL, "You fall to the ground hard");
	    drop_cheat_flag = 1;
	    MechFalls(mech, 1001, 1);
	    drop_cheat_flag = 0;
	    done = 1;
	}
    } else
	mech_notify(mech, MECHALL, "You drop to the ground prone!");
    }
    if (MechCritStatus(mech) & DUG_IN)
	MechCritStatus(mech) &= ~DUG_IN;
    if (MechStatus2(mech) & UNJAMMING)
	MechStatus2(mech) &= ~UNJAMMING;
    MakeMechFall(mech);
    MechDesiredSpeed(mech) = 0;
    MechSpeed(mech) = 0;

/*  StopMoving(mech); */
    MechFloods(mech);
    if (!Jumping(mech)) {
	    if ((MechRTerrain(mech) == WATER) && ((Fallen(mech) && (MechZ(mech) <= -1)) || (MechZ(mech) <= -2))) {
			if (MechStatus(mech) & JELLIED) {
            	MechStatus(mech) &= ~JELLIED;
			    mech_notify(mech, MECHALL, "The water washes away the burning jelly!");
                add_decoration(getMap(mech->mapindex), MechX(mech), MechY(mech), TYPE_FIRE, FIRE, 90);
			}
			for (i = 0 ; i < NUM_SECTIONS ; i++)
				AcidClear(mech, i, 1);
			CheckNoAir(mech);
		}
	}
    if (!done)
	possible_mine_poof(mech, MINE_STEP);
}

void mech_standcheck(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int roll_needed;

    cch(MECH_USUAL);
    DOCHECK(MechType(mech) == CLASS_BSUIT, "You're standing already!");
    DOCHECK(MechType(mech) != CLASS_MECH &&
	MechType(mech) != CLASS_MW,
	"This vehicle cannot stand like a 'Mech.");
    DOCHECK(Jumping(mech), "You're standing while jumping!");
    DOCHECK(OODing(mech), "You're standing while flying!");
    if (MechMove(mech) != MOVE_QUAD) {
	DOCHECK(MechCritStatus(mech) & NO_LEGS,
	    "You have no legs to stand on!");
    } else {
	DOCHECK(MechCritStatus(mech) & NO_LEGS,
	    "You'd be far too unstable!");
    }
    DOCHECK((MMaxSpeed(mech) <= 0),
        "You don't have enough mobility to stand!");
    DOCHECK(MechCritStatus(mech) & GYRO_DESTROYED,
	"You cannot stand with a destroyed gyro!");
    DOCHECK(!Fallen(mech), "You're already standing!");
    DOCHECK(Standrecovering(mech),
	"You're still recovering from your last attempt!");
    roll_needed = FindSPilotPiloting(mech) + MechPilotSkillBase(mech);
    if (MechSpecials(mech) & HARDA_TECH)
	roll_needed += 1;
    if (SectIsDestroyed(mech, LARM))
        roll_needed++;
    if (SectIsDestroyed(mech, RARM))
        roll_needed++;
    if (MechMove(mech) == MOVE_QUAD && !(MechCritStatus(mech) & LEG_DESTROYED))
	roll_needed = 0;
    if (MechType(mech) != CLASS_MW) {
	mech_notify(mech, MECHALL, tprintf("You have a BTH %0d to stand",
		roll_needed));
    }
}


void mech_stand(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    int modifier = 0;

    cch(MECH_USUALO);
    DOCHECK(MechType(mech) == CLASS_BSUIT, "You're standing already!");
    DOCHECK(MechType(mech) != CLASS_MECH &&
	MechType(mech) != CLASS_MW,
	"This vehicle cannot stand like a 'Mech.");
    DOCHECK(Jumping(mech), "You're standing while jumping!");
    DOCHECK(OODing(mech), "You're standing while flying!");
    if (MechMove(mech) != MOVE_QUAD) {
	DOCHECK(MechCritStatus(mech) & NO_LEGS,
	    "You have no legs to stand on!");
    } else {
	DOCHECK(MechCritStatus(mech) & NO_LEGS,
	    "You'd be far too unstable!");
    }
    DOCHECK((MMaxSpeed(mech) <= 0),
	"You don't have enough mobility to stand!");
    DOCHECK(MechCritStatus(mech) & GYRO_DESTROYED,
	"You cannot stand with a destroyed gyro!");
    DOCHECK(!Fallen(mech), "You're already standing!");
    DOCHECK(Standrecovering(mech),
	"You're still recovering from your last attempt!");
    MakeMechStand(mech);
    if (SectIsDestroyed(mech, LARM))
        modifier++;
    if (SectIsDestroyed(mech, RARM))
        modifier++;
    if (MechType(mech) != CLASS_MW && !MadePilotSkillRoll(mech, modifier)
     && (MechMove(mech) != MOVE_QUAD || (MechCritStatus(mech) & LEG_DESTROYED))) {
	mech_notify(mech, MECHALL,
	    "You fail your attempt to stand and fall back on the ground");

	MechFalls(mech, 1, 1);
	MECHEVENT(mech, EVENT_STANDFAIL, mech_standfail_event, MechType(mech) == CLASS_MW ? DROP_TO_STAND_RECYCLE / 3 : StandMechTime(mech), 0);
    } else {
	/* Now we set a counter in goingy to keep him from moving or
	   jumping until he is finished standing */
	mech_notify(mech, MECHALL, "You begin to stand up.");
	MechLOSBroadcast(mech, tprintf("begins to stand."));
	MECHEVENT(mech, EVENT_STAND, mech_stand_event,
	    MechType(mech) ==
	    CLASS_MW ? DROP_TO_STAND_RECYCLE / 3 : StandMechTime(mech), 0);
    }
}

void mech_land(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALO);
    if (MechType(mech) != CLASS_MECH && MechType(mech) != CLASS_MW &&
	MechType(mech) != CLASS_BSUIT && MechType(mech) != CLASS_VEH_GROUND) {
	aero_land(player, data, buffer);
	return;
    }
    if (Jumping(mech)) {
	mech_notify(mech, MECHALL,
	    "You abort your full jump and attempt to land early");
	if (MadePilotSkillRoll(mech, 0)) {
	    mech_notify(mech, MECHALL, "You are able to abort the jump.");
        MechLOSBroadcast (mech, "lands abruptly!");
	    LandMech(mech, 1);
	} else {
	    mech_notify(mech, MECHALL, "You don't quite make it.");
	    MechLOSBroadcast(mech,
		"attempts a landing, but crashes to the ground!");
	    MechFalls(mech, 1001, 0);
	    MechDFATarget(mech) = -1;
	    MechGoingX(mech) = MechGoingY(mech) = 0;
	    MechSpeed(mech) = 0;
	    MaybeMove(mech);
	}
    } else
	notify(player, "You're not jumping!");
}

/* Facing related */

void mech_heading(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[2];
    int newheading;
    int numargs = 0;

    cch(MECH_USUALO);
    DOCHECK(is_aero(mech) && TakingOff(mech), "No turning while taking off.");
    DOCHECK(is_aero(mech) && Landing(mech), "No turning while landing.");
    numargs = mech_parseattributes(buffer, args, 2);
    if (numargs == 1) {
	DOCHECK(MechMove(mech) == MOVE_NONE,
	    "This piece of equipment is stationary!");
	DOCHECK((MechStatus2(mech) & BOGDOWN), "You cannot get enough traction to turn while bogged down.");
	DOCHECK(WaterBeast(mech) &&
	    NotInWater(mech),
	    "You are regrettably unable to move at this time. We apologize for the inconvenience.");
	DOCHECK(is_aero(mech) && Spinning(mech) && !Landed(mech),
	    "You are unable to control your craft at the moment.");
	DOCHECK(MechCritStatus(mech) & DUG_IN && MechMove(mech) != MOVE_QUAD,
	    "You are in a hole you dug, unable to move [use speed cmd to get out].");
	DOCHECK(MechCritStatus(mech) & DUG_IN && MechMove(mech) == MOVE_QUAD,
	    "You are currently angling your weaponry over the mound. [use speed cmd to get out].");

	if (Digging(mech)) {
	    mech_notify(mech, MECHALL,
		"You cease your attempts at digging in.");
	    StopDigging(mech);
	}
	if (!isdigit(*args[0])) {
	    notify(player, tprintf("Your current heading is %i.",
		    MechFacing(mech)));
	    return;
	}
	newheading = AcceptableDegree(atoi(args[0]));
	MechDesiredFacing(mech) = newheading;
	mech_notify(mech, MECHALL, tprintf("Heading changed to %d.",
		newheading));
	MaybeMove(mech);
    } else if (numargs == 2) {
	if (!isdigit(*args[0])) {
	    notify(player, tprintf("Your current heading is %i.",
		    MechFacing(mech)));
	    return;
	}

	switch ((toupper(*args[1]))) {
	case 'R':
	    newheading =
		AcceptableDegree(MechFacing(mech) + atoi(args[0]));
	    break;
	case 'L':
	    newheading =
		AcceptableDegree(MechFacing(mech) - atoi(args[0]));
	    break;
	default:
	    mech_notify(mech, MECHALL,
		tprintf("Invalid direction indicator %c. need R|L.",
		    *args[1]));
	    return;
	}
	MechDesiredFacing(mech) = newheading;
	mech_notify(mech, MECHALL, tprintf("Heading changed to %d.",
		newheading));
	MaybeMove(mech);
    } else {
	notify(player, tprintf("Your current heading is %i.",
		MechFacing(mech)));
    }
}

void mech_turret(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1];
    int newheading;

    cch(MECH_USUALO);
    DOCHECK(MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW ||
	MechType(mech) == CLASS_BSUIT || is_aero(mech) ||
	!GetSectInt(mech, TURRET), "You don't have a turret.");
    DOCHECK(MechCritStatus(mech) & TURRET_LOCKED,
	"Your turret is locked in position.");
    DOCHECK(MechCritStatus(mech) & TURRET_JAMMED,
	"Your turret is jammed in position. Try 'unjam -'.");
    if (mech_parseattributes(buffer, args, 1) == 1) {
	newheading = AcceptableDegree(atoi(args[0]) - MechFacing(mech));
	MechTurretFacing(mech) = newheading;
	mech_notify(mech, MECHALL, tprintf("Turret facing changed to %d.",
		AcceptableDegree(MechTurretFacing(mech) +
		    MechFacing(mech))));
    } else {
	notify(player, tprintf("Your turret is currently facing %d.",
		AcceptableDegree(MechTurretFacing(mech) +
		    MechFacing(mech))));
    }
}

void mech_rotatetorso(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[2];

    cch(MECH_USUALO);
    DOCHECK(MechType(mech) == CLASS_BSUIT, "Huh?");
    DOCHECK(MechType(mech) == CLASS_MECH && MechMove(mech) == MOVE_QUAD , "Nope, not in a Quad.");
    DOCHECK(MechType(mech) != CLASS_MECH &&
	MechType(mech) != CLASS_MW, "You don't have a torso.");
    DOCHECK(Fallen(mech),
	"You're lying flat on your face, you can't rotate your torso.");
    if (mech_parseattributes(buffer, args, 2) == 1) {
	switch (args[0][0]) {
	case 'L':
	case 'l':
	    DOCHECK(MechStatus(mech) & TORSO_LEFT,
		"You cannot rotate torso beyond 60 degrees!");
	    if (MechStatus(mech) & TORSO_RIGHT)
		MechStatus(mech) &= ~TORSO_RIGHT;
	    else
		MechStatus(mech) |= TORSO_LEFT;
	    mech_notify(mech, MECHALL, "You rotate your torso left.");
	    break;
	case 'R':
	case 'r':
	    DOCHECK(MechStatus(mech) & TORSO_RIGHT,
		"You cannot rotate torso beyond 60 degrees!");
	    if (MechStatus(mech) & TORSO_LEFT)
		MechStatus(mech) &= ~TORSO_LEFT;
	    else
		MechStatus(mech) |= TORSO_RIGHT;
	    mech_notify(mech, MECHALL, "You rotate your torso right.");
	    break;
	case 'C':
	case 'c':
	    MechStatus(mech) &= ~(TORSO_RIGHT | TORSO_LEFT);
	    mech_notify(mech, MECHALL, "You center your torso");
	    break;
	default:
	    notify(player, "Rotate must have LEFT RIGHT or CENTER");
	    break;
	}
    } else
	notify(player, "Invalid number of arguments!");
    MarkForLOSUpdate(mech);
}

struct {
    char *name;
    int flag;
} speed_tables[] = {
    {
    "walk", 1}, {
    "run", 2}, {
    "stop", 0}, {
    "back", -1}, {
    "cruise", 1}, {
    "flank", 2}, {
    NULL, 0.0}
};

void mech_speed(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1];
    float newspeed, walkspeed, maxspeed;
    int i, flag = 0;

    cch(MECH_USUALO);
    if (RollingT(mech)) {
	DOCHECK(!Landed(mech), "Use thrust command instead!");
    } else if (FlyingT(mech)) {
	DOCHECK(MechType(mech) != CLASS_VTOL,
	    "Use thrust command instead!");
    }
    DOCHECK(is_aero(mech) && TakingOff(mech), "You cannot modify speed while taking off!");
    DOCHECK(is_aero(mech) && Landing(mech), "You cannot modify speed while landing!");
    DOCHECK(MechMove(mech) == MOVE_NONE,
	"This piece of equipment is stationary!");
    DOCHECK(Standing(mech),
	"You are currently standing up and cannot move.");
    DOCHECK((Fallen(mech)) && (MechType(mech) != CLASS_MECH &&
	    MechType(mech) != CLASS_MW),
	"Your vehicle's movement system is destroyed.");
    DOCHECK((MechStatus2(mech) & BOGDOWN),"You are bogged down and unable to move!");
    DOCHECK(Fallen(mech), "You are currently prone and cannot move.");
    DOCHECK(WaterBeast(mech) &&
	NotInWater(mech),
	"You are regrettably unable to move at this time. We apologize for the inconvenience.");
    if (mech_parseattributes(buffer, args, 1) != 1) {
	notify(player, tprintf("Your current speed is %.2f.",
		MechSpeed(mech)));
	return;
    }
    DOCHECK(FlyingT(mech) && AeroFuel(mech) <= 0, "You're out of fuel!");
    maxspeed = MMaxSpeed(mech);
    if (MechMove(mech) == MOVE_VTOL)
	maxspeed =
	    sqrt((float) maxspeed * maxspeed -
	    MechVerticalSpeed(mech) * MechVerticalSpeed(mech));
    maxspeed = maxspeed > 0.0 ? maxspeed : 0.0;
/*    if ((MechHeat(mech) >= 9.) && (MechSpecials(mech) & TRIPLE_MYOMER_TECH))
	  maxspeed = ((WalkingSpeed(maxspeed) + MP1) * 1.5);
    if (MechStatus2(mech) & SPRINTING) {
	maxspeed = WalkingSpeed(maxspeed) * 2;
	if (HasBoolAdvantage(MechPilot(mech), "speed_demon") && maxspeed > 0)
            maxspeed += MP1;
    }*/
    walkspeed = WalkingSpeed(maxspeed);
/*    if (MechSpecials(mech) & HARDA_TECH)
	maxspeed = (maxspeed <= MP1 ? maxspeed : maxspeed - MP1);*/
    if (isdigit(args[0][0]) || (args[0][0] == '-' && isdigit(args[0][1])))   {
    newspeed = atof(args[0]);
    flag++;
    } else if (isalpha(args[0][0])) {
      if (newspeed < 0.1) {
	/* Possibly a string speed instead? */
	for (i = 0; speed_tables[i].name; i++)
	    if (!strcasecmp(speed_tables[i].name, args[0])) {
		switch (speed_tables[i].flag) {
		case 0:
		    newspeed = 0.0;
		    flag++;
		    break;
		case -1:
		    newspeed = -walkspeed;
		    flag++;
		    break;
		case 1:
		    newspeed = walkspeed;
		    flag++;
		    break;
		case 2:
		    newspeed = maxspeed;
		    flag++;
		    break;
		}
		break;
	    }
      }
    }
    DOCHECK(flag == 0, "Are you drunk again?");
    if (newspeed > maxspeed)
	newspeed = maxspeed;
    if (newspeed < -walkspeed)
	newspeed = -walkspeed;
    if (Dumping(mech) && newspeed > walkspeed)
	{
	mech_notify(mech, MECHALL, "You are currently dumping ammo and cannot go that fast.");
	return;
	}
    if ((MechCritStatus(mech) & (TAILROTOR_DAMAGED|CREW_STUNNED)) && newspeed > walkspeed)
	{
	if (MechCritStatus(mech) & CREW_STUNNED)
	    mech_notify(mech, MECHALL, "You cannot move faster than cruise speed while stunned!");
	else
	    mech_notify(mech, MECHALL, "Your tail rotor is damaged! You can't go that fast without crashing!");
	return;
	}
/*    if (!Wizard(player) && In_Character(mech->mynum) && MechPilot(mech) != player) {
	if (newspeed < 0.0) {
	    notify(player, "Not being the Pilot of this beast, you cannot move it backwards.");
	    return;
	} else if (newspeed > walkspeed) {
	    notify(player, "Not being the Pilot of this beast, you cannot go faster than walking speed.");
	    return;
	}
    }*/
    if (is_aero(mech) && newspeed > MP1 * 4)
	newspeed = MP1 * 4;
    else if (is_aero(mech) && newspeed < 0 - (MP1 * 4))
	newspeed = 0 - (MP1 * 4);

    MechDesiredSpeed(mech) = newspeed;
    MaybeMove(mech);
    if (fabs(newspeed) > 0.1) {
	if (MechSwarmTarget(mech) > 0) {
	    bsuit_stopswarmer(getMech(MechSwarmTarget(mech)), mech, 1);
	    MechCritStatus(mech) &= ~HIDDEN;
	    }
	if (Digging(mech)) {
	    mech_notify(mech, MECHALL,
		"You cease your attempts at digging in.");
	    StopDigging(mech);
	}
	if (MechCritStatus(mech) & DUG_IN) {
		if (MechMove(mech) == MOVE_QUAD)
			MechLOSBroadcast(mech, "breaks from from it' hulldown position.");
		else
			MechLOSBroadcast(mech, "emerges from its dugin hole.");
		}
	MechCritStatus(mech) &= ~DUG_IN;
    }
    if (MechStatus2(mech) & UNJAMMING)
	if (newspeed > walkspeed)
	    {
	    mech_notify(mech, MECHALL, "You speed up and cease your work to unjam your weapon.");
	    MechLOSBroadcast(mech, "stops unjamming its weapon.");
	    StopUnjamming(mech);
	    }
    mech_notify(mech, MECHALL, tprintf("Desired speed changed to %d KPH",
	    (int) newspeed));
}

void mech_sprint(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int d = 0, i;

    cch(MECH_USUALO);
    DOCHECK(!mudconf.btech_allowsprint,
	"No sprinting allowed on this MUX.");
    DOCHECK(MechMove(mech) == MOVE_NONE,
        "This piece of equipment is stationary!");
    DOCHECK(MechCarrying(mech) > 0,
	"You cannot sprint while towing!");
    DOCHECK(Standing(mech),
        "You are currently standing up and cannot move.");
    DOCHECK(Jumping(mech),
	"You cannot do this while jumping.");
    DOCHECK((Fallen(mech)) && (MechType(mech) != CLASS_MECH &&
            MechType(mech) != CLASS_MW),
        "Your vehicle's movement system is destroyed.");
    DOCHECK(Fallen(mech), "You are currently prone and cannot move.");
    DOCHECK(WaterBeast(mech) &&
        NotInWater(mech),
        "You are regrettably unable to move at this time. We apologize for the inconvenience.");
    DOCHECK(MoveModeChange(mech), "You are already changing movement modes!");
    DOCHECK(MechStatus2(mech) & (EVADING|DODGING), "You cannot perform multiple movement modes!");
    DOCHECK(MechSwarmTarget(mech) > 0, "You cannot sprint while mounted!");
    if (MechType(mech) == CLASS_MECH)
        DOCHECK(SectIsDestroyed(mech, RLEG) || SectIsDestroyed(mech, LLEG) || (MechMove(mech) != MOVE_QUAD ? 0 :
	    SectIsDestroyed(mech, RLEG) || SectIsDestroyed(mech, LLEG)), "That's kind of hard while limping.");

    d |= MODE_SPRINT;
    d |= (MechStatus2(mech) & SPRINTING) ? MODE_OFF : MODE_ON;
    if (d & MODE_ON) {
        if ((i = MechFullNoRecycle(mech, CHECK_BOTH)) > 0) {
            mech_notify(mech, MECHALL, tprintf("You have %s recycling!", (i == 1 ? "weapons" : i == 2 ? "limbs" : "error")));
            return;
            }
        mech_notify(mech, MECHALL, "You begin the process of sprinting.....");
        MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ? TURN / 2 : TURN, d);
    } else {
        mech_notify(mech, MECHALL, "You begin the process of ceasing to sprint.");
        MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ? TURN / 2 : TURN, d);
    }
return;
}

void mech_evade(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int d = 0, i;

    cch(MECH_USUALO);
    DOCHECK(MechMove(mech) == MOVE_NONE,
        "This piece of equipment is stationary!");
    DOCHECK(Standing(mech),
        "You are currently standing up and cannot move.");
    DOCHECK(Jumping(mech),
	"You cannot do this while jumping.");
    DOCHECK((Fallen(mech)) && (MechType(mech) != CLASS_MECH && MechType(mech) != CLASS_MW),
        "Your vehicle's movement system is destroyed.");
    DOCHECK(MechCarrying(mech) > 0,
	"You can't do that while towing");
    DOCHECK(Fallen(mech),
	"You are currently prone and cannot move.");
    DOCHECK(!(MechStatus2(mech) & EVADING) && MechType(mech) == CLASS_MECH && (PartIsNonfunctional(mech, LLEG, 0) || PartIsNonfunctional(mech, RLEG, 0)),
    	"You need both hip functional to evade.");
    DOCHECK(WaterBeast(mech) &&
        NotInWater(mech),
        "You are regrettably unable to move at this time. We apologize for the inconvenience.");
    DOCHECK(MoveModeChange(mech), "You are already changing movement modes!");
    DOCHECK(MechStatus2(mech) & (SPRINTING|DODGING), "You cannot perform multiple movement modes!");
    DOCHECK(MechSwarmTarget(mech) > 0, "You cannot evade while mounted!");
    if (MechType(mech) == CLASS_MECH)
        DOCHECK(SectIsDestroyed(mech, RLEG) || SectIsDestroyed(mech, LLEG) || (MechMove(mech) != MOVE_QUAD ? 0 :
	    SectIsDestroyed(mech, RLEG) || SectIsDestroyed(mech, LLEG)), "That's kind of hard while limping.");
    
    d |= MODE_EVADE;
    d |= (MechStatus2(mech) & EVADING) ? MODE_OFF : MODE_ON;
    if (d & MODE_ON) {
        if ((i = MechFullNoRecycle(mech, CHECK_BOTH)) > 0) {
            mech_notify(mech, MECHALL, tprintf("You have %s recycling!", (i == 1 ? "weapons" : i == 2 ? "limbs" : "error")));
            return;
            }
        mech_notify(mech, MECHALL, "You begin the process of evading.....");
        MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ? TURN / 2 : TURN, d);
    } else {
        mech_notify(mech, MECHALL, "You begin the process of ceasing to evade.");
        MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ? TURN / 2 : TURN, d);
    }
return;
}

void mech_dodge(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int d = 0, i;

    cch(MECH_USUALO);
    DOCHECK(MechMove(mech) == MOVE_NONE,
        "This piece of equipment is stationary!");
    DOCHECK(Standing(mech),
        "You are currently standing up and cannot move.");
    DOCHECK((Fallen(mech)) && (MechType(mech) != CLASS_MECH &&
            MechType(mech) != CLASS_MW),
        "Your vehicle's movement system is destroyed.");
    DOCHECK(Fallen(mech), "You are currently prone and cannot move.");
    DOCHECK(WaterBeast(mech) &&
        NotInWater(mech),
        "You are regrettably unable to move at this time. We apologize for the inconvenience.");
    DOCHECK(MoveModeChange(mech), "You are already changing movement modes!");
    DOCHECK(MechStatus2(mech) & (SPRINTING|EVADING), "You cannot perform multiple movement modes!");
    DOCHECK(!(HasBoolAdvantage(player, "dodge_maneuver")) || player != MechPilot(mech),
	"You either are not the pilot of this mech, have no Dodge Maneuver adavantage, or both.");
    d |= MODE_DODGE;
    d |= (MechStatus2(mech) & DODGING) ? MODE_OFF : MODE_ON;
    if (d & MODE_ON) {
        if ((i = MechFullNoRecycle(mech, CHECK_PHYS)) > 0) {
            mech_notify(mech, MECHALL, tprintf("You have %s recycling!", (i == 1 ? "weapons" : i == 2 ? "limbs" : "error")));
            return;
            }
        mech_notify(mech, MECHALL, "You begin the process of dodging.....");
        MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, 1, d);
    } else {
        mech_notify(mech, MECHALL, "You begin the process of ceasing to dodge.");
        MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, TURN, d);
    }
return;
}

void mech_vertical(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1], buff[50];
    float newspeed, maxspeed;

    cch(MECH_USUALO);
    DOCHECK(MechType(mech) != CLASS_VEH_VTOL &&
	MechMove(mech) != MOVE_SUB, "This command is for VTOLs only.");
    DOCHECK(MechType(mech) == CLASS_VTOL &&
	AeroFuel(mech) <= 0, "You're out of fuel!");
    DOCHECK(WaterBeast(mech) &&
	NotInWater(mech),
	"You are regrettably unable to move at this time. We apologize for the inconvenience.");
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	tprintf("Current vertical speed is %.2f KPH.",
	    (float) MechVerticalSpeed(mech)));
    newspeed = atof(args[0]);
    maxspeed = (MechStatus2(mech) & UNJAMMING ? WalkingSpeed(MMaxSpeed(mech)) : MMaxSpeed(mech));
    maxspeed =
	sqrt((float) maxspeed * maxspeed -
	MechDesiredSpeed(mech) * MechDesiredSpeed(mech));
    if ((newspeed > maxspeed) || (newspeed < -maxspeed)) {
	sprintf(buff, "Max vertical speed is + %d KPH and - %d KPH",
	    (int) maxspeed, (int) maxspeed);
	notify(player, buff);
    } else {
	DOCHECK(Fallen(mech),
	    "Your vehicle's movement system is destroyed.");
	DOCHECK(MechType(mech) == CLASS_VTOL &&
	    Landed(mech), "You need to take off first.");
	MechVerticalSpeed(mech) = newspeed;
	mech_notify(mech, MECHALL,
	    tprintf("Vertical speed changed to %d KPH", (int) newspeed));;
	MaybeMove(mech);
    }
}

void mech_jump(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *tempMech = NULL;
    MAP *mech_map;
    char *args[3];
    int argc;
    int target;
    char targetID[2];
    short mapx, mapy;
    int bearing;
    float range = 0.0;
    float realx, realy;
    int sz, tz, jps, legjets;

    legjets = FindLegJumpJets(mech);
    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    DOCHECK(OODing(mech) || MechCocoon(mech), "You cannot jump while OODing!");
    DOCHECK((MechType(mech) != CLASS_MECH && MechType(mech) != CLASS_MW &&
	MechType(mech) != CLASS_BSUIT && MechType(mech) != CLASS_VEH_GROUND) && (MechJumpSpeed(mech) > 0),
	"This vehicle cannot jump like a 'Mech.");
    DOCHECK(MoveModeLock(mech),
	"Movement modes disallow jumping.");
    DOCHECK(Dumping(mech),
	"You cannot jump while dumping ammo. That could get a tad dangerous.");
    DOCHECK(MapIsUnderground(mech_map),
	"Going much higher than your are now would put you through the ceiling.");
    DOCHECK(MechCarrying(mech) > 0,
	"You can't jump while towing someone!");
/*    DOCHECK((MechMaxSpeed(mech) - MMaxSpeed(mech)) > MP1,
	"No, with this cargo you won't!"); */
/* Let's do this check a bit cleaner */
    DOCHECK(MechCarrying(mech) > 0 || MechCarriedCargo(mech),
	"No arial maneuvers while carrying cargo or towing....");
    DOCHECK((MechCritStatus(mech) & DUG_IN), "You cannot jump while dug in!");
    DOCHECK((Fallen(mech) && (MechType(mech) != CLASS_VEH_GROUND)), "You can't Jump from a FALLEN position");
    DOCHECK(Jumping(mech), "You're already jumping!");
    DOCHECK(Stabilizing(mech),
	"You haven't stabilized from your last jump yet.");
    DOCHECK(Standing(mech), "You haven't finished standing up yet.");
    DOCHECK(fabs(MechJumpSpeed(mech)) <= 0.0,
	"This mech doesn't have jump jets!");
    argc = mech_parseattributes(buffer, args, 3);
    DOCHECK(argc > 2, "Too many arguments to JUMP function!");
    MechStatus(mech) &= ~DFA_ATTACK;	/* By default no DFA */
    switch (argc) {
    case 0:
	/* DFA current target... */
	DOCHECK((MechType(mech) == CLASS_VEH_GROUND), "Silly Wabbit, DFA's are for mechs!");
	target = MechTarget(mech);
	tempMech = getMech(target);
	if (tempMech) {
	    range = FaMechRange(mech, tempMech);
	    DOCHECK(!InLineOfSight(mech, tempMech, MechX(tempMech),
		    MechY(tempMech), range),
		"Target is not in line of sight!");
	    DOCHECK(((MechTeam(mech) == MechTeam(tempMech)) && (Started(tempMech)) && 
                (!Destroyed(tempMech))), "Friendly units ? I dont Think so..");	
            if ((MechType(tempMech) == CLASS_VTOL) || (MechType(tempMech) == CLASS_AERO) ||
		(MechType(tempMech) == CLASS_DS))
		    DOCHECK(!Landed(tempMech), 
			"Your target is airborne, you cannot land on it."); 
	    mapx = MechX(tempMech);
	    mapy = MechY(tempMech);
	    MechDFATarget(mech) = MechTarget(mech);
	} else {
	    notify(player, "Invalid Target!");
	    return;
	}
	break;
    case 1:
	/* Jump Target */
	DOCHECK((MechType(mech) == CLASS_VEH_GROUND), "Silly Wabbit, DFA's are for mechs!");
	targetID[0] = args[0][0];
	targetID[1] = args[0][1];
	target = FindTargetDBREFFromMapNumber(mech, targetID);
	tempMech = getMech(target);
	DOCHECK(!tempMech, "Target is not in line of sight!");
	range = FaMechRange(mech, tempMech);
	DOCHECK(!InLineOfSight(mech, tempMech, MechX(tempMech),
		MechY(tempMech), range),
	    "Target is not in line of sight!");
	DOCHECK(((MechTeam(mech) == MechTeam(tempMech)) && (Started(tempMech)) && 
            (!Destroyed(tempMech))), "Friendly units ? I dont Think so..");	
        if ((MechType(tempMech) == CLASS_VTOL) || (MechType(tempMech) == CLASS_AERO) ||
	    (MechType(tempMech) == CLASS_DS))
	    DOCHECK(!Landed(tempMech), 
		"Your target is airborne, you cannot land on it."); 
	mapx = MechX(tempMech);
	mapy = MechY(tempMech);
	MechDFATarget(mech) = tempMech->mynum;
	break;
    case 2:
	bearing = atoi(args[0]);
	range = fabs(atof(args[1]));
	FindXY(MechFX(mech), MechFY(mech), bearing, range, &realx, &realy);

	/* This is so we are jumping to the center of a hex */
	/* and the bearing jives with the target hex */
	RealCoordToMapCoord(&mapx, &mapy, realx, realy);
	break;
    }
    DOCHECK(mapx >= mech_map->map_width || mapy >= mech_map->map_height ||
	mapx < 0 || mapy < 0, "That would take you off the map!");
    DOCHECK(MechX(mech) == mapx &&
	MechY(mech) == mapy, "You're already in the target hex.");
    DOCHECK((GetTerrain(mech_map, mapx, mapy) == WATER && GetElev(mech_map, mapx, mapy) > 0 && MechType(mech) == CLASS_VEH_GROUND && MechMove(mech) != MOVE_HOVER),
	"That would land you in a body of water!");
    DOCHECK((MechMove(mech) == MOVE_HOVER && (GetTerrain(mech_map, mapx, mapy) == LIGHT_FOREST || GetTerrain(mech_map, mapx, mapy) == HEAVY_FOREST)),
	"No matter how skilled, Ace, you won't make a forest landing.");
    sz = MechZ(mech);
    tz = (GetTerrain(mech_map, mapx, mapy) == ICE ? 0 : Elevation(mech_map, mapx, mapy));
    jps = (JumpSpeedMP(mech, mech_map) - ((sz == -1 || MechRTerrain(mech) == SNOW || MechRTerrain(mech) == DESERT) ? legjets : 0));
    DOCHECK(((sz == -1 || MechRTerrain(mech) == SNOW || MechRTerrain(mech) == DESERT) && ((range > jps) && (range <= JumpSpeedMP(mech, mech_map)))),
        tprintf("You currently cannot use %d of your leg mounted jump jets due to %s!", legjets,
	GetTerrainName(mech_map, MechX(mech), MechY(mech))));
    DOCHECK(range > jps, "That target is out of range!");
    if ((MechType(mech) != CLASS_BSUIT && MechType(mech) != CLASS_VEH_GROUND)  && tempMech)
	MechStatus(mech) |= DFA_ATTACK;
    /*   MechJumpTop(mech) = BOUNDED(3, (jps - range) + 2, jps - 1); */
    /* New idea: JumpTop = (JP + 1 - range / 3) - in another words,
       SDR jumping for 1 hexes has 8 + 1 = 9 hex elevation in mid-flight,
       SDR jumping for 8 hexes has 8 + 1 - 2 = 7 hex elevation in mid-flight,
       TDR jumping for 4 hexes has 4 + 1 - 1 = 4 hex elevation in mid-flight

       Come to think of it, the last SDR figure was ridiculous. New
       value: 2 * 1 + 2 = 4
     */
    MechJumpTop(mech) = MIN(jps + 1 - range / 3, 2 * range + 2);
    DOCHECK((tz - sz) > jps,
	"That target's too high for you to reach with a single jump!");
    DOCHECK((sz - tz) > jps,
	"That target's low for you to reach with a single jump!");
    DOCHECK(sz < -1, "Glub glub glub.");
    MapCoordToRealCoord(mapx, mapy, &realx, &realy);
    bearing = FindBearing(MechFX(mech), MechFY(mech), realx, realy);

    /* TAKE OFF! */
    if (MechRTerrain(mech) == SNOW)
	if ((Number(1,12) >= 9) && (range > (JumpSpeedMP(mech, mech_map) / 2)))
		SetTerrain(mech_map, MechX(mech), MechY(mech), GRASSLAND);
    if (MechStatus2(mech) & BOGDOWN)
	{
	MechStatus2(mech) &= ~BOGDOWN;
	mech_notify(mech, MECHALL, "The fusion blast manages to melt enough snow to break the bog!");
	}
    MechCocoon(mech) = 0;
    MechJumpHeading(mech) = bearing;
    MechStatus(mech) |= JUMPING;
    MechStartFX(mech) = MechFX(mech);
    MechStartFY(mech) = MechFY(mech);
    MechStartFZ(mech) = MechFZ(mech);
    MechJumpLength(mech) =
	my_sqrt((double) (realx - MechStartFX(mech)),
	(double) (realy - MechStartFY(mech)));
    MechGoingX(mech) = mapx;
    MechGoingY(mech) = mapy;
    MechEndFZ(mech) = ZSCALE * tz;
    MechSpeed(mech) = 0.0;
    if (MechStatus(mech) & DFA_ATTACK)
	mech_notify(mech, MECHALL,
	    "You engage your jump jets for a Death From Above attack!");
    else
	mech_notify(mech, MECHALL, "You engage your jump jets");
    MechSwarmTarget(mech) = -1;
    if (MechStatus2(mech) & UNJAMMING) {
	StopUnjamming(mech);
        MechLOSBroadcast(mech, "ceases its odd gyrations as it launches into the sky.");
    }
    MechLOSBroadcast(mech, "engages its jumpjets and launches into the air!");
    MECHEVENT(mech, EVENT_JUMP, mech_jump_event, JUMP_TICK, 0);
}

int DropGetElevation(MECH * mech)
{
    if (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) {
	if (MechZ(mech) < (MechElev(mech))) {
	    if (Overwater(mech))
		return 0;
	    return bridge_w_elevation(mech);
	}
	return MechElevation(mech);
    }
    if (Overwater(mech) || (MechRTerrain(mech) == ICE && MechZ(mech) >= 0))
	return MAX(0, MechElevation(mech));
    else
	return MechElevation(mech);
}

void DropSetElevation(MECH * mech, int wantdrop)
{
    if (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) {
	bridge_set_elevation(mech);
	return;
    }
    MechZ(mech) = DropGetElevation(mech);
    MechFZ(mech) = MechZ(mech) * ZSCALE;
    if (wantdrop)
	if (MechRTerrain(mech) == ICE && MechZ(mech) >= 0)
	    possibly_drop_thru_ice(mech);
}

void LandMech(MECH * mech, int intentional)
{
    MECH *target;
    int dfa = 0, done = 0, i;

    /*
     * Added check to see if we're actually awake when we try to land
     * - Kipsta
     * - 8/3/99
     */

    if (Uncon(mech)) {
	mech_notify(mech, MECHALL,
	    "Your lack of conciousness makes you fall to the ground. Not like you can read this anyway.");
	MechFalls(mech, 1, 0);
	dfa = 1;
	done = 1;
    } else {
	/* Handle DFA attack */
	if (MechStatus(mech) & DFA_ATTACK) {
	    /* is the target here? */
	    target = getMech(MechDFATarget(mech));
	    if (target) {
		if (MechX(target) == MechX(mech) &&
		    MechY(target) == MechY(mech)) dfa =
			DeathFromAbove(mech, target);
		else
		    mech_notify(mech, MECHPILOT,
			"Your DFA target has moved!");
	    } else
		mech_notify(mech, MECHPILOT,
		    "Your target has become invalid.");
	}
	if (!dfa)
	    mech_notify(mech, MECHALL, "You finish your jump.");
	/* Check piloting rolls, etc. */
	if (MechCritStatus(mech) & (GYRO_DESTROYED|GYRO_DAMAGED)) {
	    mech_notify(mech, MECHPILOT,
		"Your damaged gyro makes it harder to land");
            if (!MadePilotSkillRoll(mech, 0)) {
                mech_notify(mech, MECHALL,
                    "Your damaged gyro has caused you to fall upon landing!");
                MechLOSBroadcast(mech,
                    "lands, unbalanced, and falls down!");
                dfa = 1;
                MechFalls(mech, 1, 0);
                done = 1;
            } 
	} else if (MechCritStatus(mech) & LEG_DESTROYED) {
	    mech_notify(mech, MECHPILOT,
		"Your missing leg makes it harder to land");
	    if (!MadePilotSkillRoll(mech, 0)) {
		mech_notify(mech, MECHALL,
		    "Your missing leg has caused you to fall upon landing!");
		MechLOSBroadcast(mech,
		    "lands, unbalanced, and falls down!");
		dfa = 1;
		MechFalls(mech, 1, 0);
		done = 1;
	    }
	} else if (MechSections(mech)[RLEG].basetohit ||
	    MechSections(mech)[LLEG].basetohit) {
	    mech_notify(mech, MECHPILOT,
		"Your damaged leg actuators make it harder to land");
	    if (!MadePilotSkillRoll(mech, 0)) {
		mech_notify(mech, MECHALL,
		    "Your damaged leg actuators have caused you to fall upon landing!");
		MechLOSBroadcast(mech, "lands, stumbles, and falls down!");
		dfa = 1;
		done = 1;
		MechFalls(mech, 1, 0);
	    }
	}
    }

    if (MechType(mech) == CLASS_VEH_GROUND && (MechTerrain(mech) == LIGHT_FOREST || MechTerrain(mech) == HEAVY_FOREST || MechTerrain(mech) == ROUGH))
	{
	if (MechTerrain(mech) == ROUGH)
		mech_notify(mech, MECHALL, "You attempt to land in the rocks!");
	else
		mech_notify(mech, MECHALL, "You attempt to use your jets to maneuver through the trees!");
	if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, (MechTerrain(mech) == HEAVY_FOREST ? 4 : (MechTerrain(mech) == LIGHT_FOREST) ? 1 : -1)))
		mech_notify(mech, MECHALL, "You manage to land without damage!");
	else {
		mech_notify(mech, MECHALL, "You manage to land without exploding! This will hurt a little.....");
		MechLOSBroadcast(mech, "creams itself into a pile of trees upon landing!");
		MechFalls(mech, MAX(1, (int) sqrt(MechJumpSpeed(mech) / MP1 / 2)), 0);
	}
	}
    if (!dfa && !Fallen(mech))
	if (!domino_space(mech, 1))
	    MechLOSBroadcast(mech, "lands gracefully.");
    MechStatus(mech) &= ~JUMPING;
    MechStatus(mech) &= ~DFA_ATTACK;
    MechDFATarget(mech) = -1;
    MechGoingX(mech) = MechGoingY(mech) = 0;
    MechSpeed(mech) = 0;
    StopJump(mech);		/* Kill the event for moving 'round */
    MaybeMove(mech);		/* Possibly start movin' on da ground */
    if (MechType(mech) == CLASS_MECH && MechIsSwarmed(mech) && !intentional && !MechAutofall(mech)) {
	bsuit_stopswarmers(FindObjectsData(mech->mapindex), mech, -1);
	for (i = 0; i < NUM_SECTIONS; i++)
            SetRecycleLimb(mech, i, PHYSICAL_RECYCLE_TIME); 
	}
    DropSetElevation(mech, 1);
    if (MechTerrain(mech) == ICE)
	possibly_drop_thru_ice(mech);
    MECHEVENT(mech, EVENT_JUMPSTABIL, mech_stabilizing_event,
	JUMP_TO_HIT_RECYCLE, 0);
    if (!done)
	possible_mine_poof(mech, MINE_LAND);
    MechFloods(mech);
}

/* Flooding code. Once we're in water, this is checked
   now and then (basically when DamageMech'ed and/or
   depth changes and/or we fall) */

void MechFloodsLoc(MECH * mech, int loc, int lev)
{
    char locbuff[32];;

    if ((GetSectArmor(mech, loc) && (GetSectRArmor(mech, loc) ||
		!GetSectORArmor(mech, loc))) || !GetSectInt(mech, loc))
	return;
    if (!InWater(mech))
	return;
    if (lev >= 0)
	return;
    /* No armor, and in water. */
    if (lev == -1 && (!Fallen(mech) && loc != LLEG && loc != RLEG &&
	    (MechMove(mech) != MOVE_QUAD || (loc != LARM && loc != RARM))))
	return;
    if (MechType(mech) != CLASS_MECH)
	return;

    if (SectIsFlooded(mech, loc))	//This means were already flooded... @yeah -Kipsta. 8/4/99
	return;

    /* Woo, valid target. */
    ArmorStringFromIndex(loc, locbuff, MechType(mech), MechMove(mech));
    mech_notify(mech, MECHALL,
	tprintf
	("%%ch%%crWater floods into your %s disabling everything that was there!%%c",
     locbuff));
    MechLOSBroadcast(mech,
	tprintf("has a gaping hole in %s, and water pours in!", locbuff));

    /* Changed from DestroySection to DestroyParts for flooding
       8/3/99
       - Kipsta -
     */

    SetSectFlooded(mech, loc);
    DestroyParts(mech, mech, loc, 1, 1);

}

void MechFloods(MECH * mech)
{
    int i;
    int elev = MechElevation(mech);

    if (!InWater(mech))
	return;
    if (MechType(mech) != CLASS_MECH && MechType(mech) != CLASS_BSUIT && !(MechSpecials2(mech) & WATERPROOF_TECH)) {
	if ((MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VTOL) && MechMove(mech) != MOVE_HOVER) {
		if (MechTerrain(mech) == WATER && MechElev(mech) > 0) {
	                mech_notify(mech, MECHALL, "Water floods your engine and your unit becomes inoperable.");
        	        DestroyMech(mech, mech, 0);
        	        return;

			}
		}
	return;
	}
    if (MechZ(mech) >= 0)
	return;
    for (i = 0; i < NUM_SECTIONS; i++)
	MechFloodsLoc(mech, i, elev);
}

void MechFalls(MECH * mech, int levels, int seemsg)
{
    int roll, spread, i, hitloc, hitGroup = 0;
    int isrear = 0, damage, iscritical = 0, revspeed;
    int intentional = 0;
    MAP *map;

    /* damage pilot */
    MechCocoon(mech) = 0;
    intentional = (MechAutofall(mech) || levels > 1000);
    if (levels > 1000)
	levels = (levels - 1000);
    if (MechType(mech) == CLASS_BSUIT) {
	mech_notify(mech, MECHPILOT, "You take injury from the fall!");
	headhitmwdamage(mech, 2);
    } else {
	if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW || seemsg)
	    mech_notify(mech, MECHPILOT, "You try to avoid taking personal damage in the fall.");
	else
	    mech_notify(mech, MECHPILOT, "You try to avoid taking personal damage.");
	if (MadePilotSkillRoll(mech, levels) == 0) {
	    if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW || seemsg)
		mech_notify(mech, MECHPILOT, "You take personal injury from the fall!");
	    else
		mech_notify(mech, MECHPILOT, "You take personal injury!");
	    headhitmwdamage(mech, 1);
 	    }
    }
    revspeed = (MechSpeed(mech) < 0 ? 1 : 0);
    MechSpeed(mech) = 0;
    MechDesiredSpeed(mech) = 0;
    if (Jumping(mech)) {
	MechStatus(mech) &= ~JUMPING;
	MechStatus(mech) &= ~DFA_ATTACK;
	StopJump(mech);
	MECHEVENT(mech, EVENT_JUMPSTABIL, mech_stabilizing_event,
	    JUMP_TO_HIT_RECYCLE, 0);
    }
    if (MechCritStatus(mech) & DUG_IN)
	MechCritStatus(mech) &= ~DUG_IN;

    if (MechStatus2(mech) & UNJAMMING)
	MechStatus2(mech) &= ~UNJAMMING;

    if (MoveModeChange(mech))
	StopMoveMode(mech);

    if (MechStatus2(mech) & SPRINTING) {
	int d = MODE_SPRINT|MODE_OFF;
	MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ? TURN / 2 : TURN, d);
	}
    if (MechStatus2(mech) & EVADING) {
	int d = MODE_EVADE|MODE_OFF;
	MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ? TURN / 2 : TURN, d); 
	}

    if (MechMove(mech) == MOVE_VTOL || MechMove(mech) == MOVE_FLY) {
	MechAngle(mech) = 90;
	MechDesiredAngle(mech) = 90;
	MechVelocity(mech) = 0;
	MechVerticalSpeed(mech) = 0;
	MechGoingY(mech) = 0;
	MechStartFX(mech) = 0.0;
	MechStartFY(mech) = 0.0;
	MechStartFZ(mech) = 0.0;
	MechStatus(mech) |= LANDED;
	StopMoving(mech);
    } else
	MaybeMove(mech);
    if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW)
	MakeMechFall(mech);
    if (seemsg)
	MechLOSBroadcast(mech, "falls down!");
    DropSetElevation(mech, 1);
    MechFZ(mech) = MechZ(mech) * ZSCALE;

    roll = Number(1, 6);
    switch (roll) {
    case 1:
	hitGroup = FRONT;
	break;
    case 2:
	AddFacing(mech, 60);
	hitGroup = RIGHTSIDE;
	break;
    case 3:
	AddFacing(mech, 120);
	hitGroup = RIGHTSIDE;
	break;
    case 4:
	AddFacing(mech, 180);
	hitGroup = BACK;
	break;
    case 5:
	AddFacing(mech, 240);
	hitGroup = LEFTSIDE;
	break;
    case 6:
	AddFacing(mech, 300);
	hitGroup = LEFTSIDE;
	break;
    }
    if (hitGroup == BACK && (MechType(mech) != CLASS_VTOL && !revspeed))
	isrear = 1;
    SetFacing(mech, AcceptableDegree(MechFacing(mech)));
    MechDesiredFacing(mech) = MechFacing(mech);
    if (MechType(mech) == CLASS_BSUIT)
    	damage = levels / 10;
    else if (!InWater(mech) && MechRTerrain(mech) != HIGHWATER)
	damage = (levels * (MechTons(mech) + 5)) / 10;
    else
	damage = (levels * (MechTons(mech) + 5)) / 20;
    if (InSpecial(mech))
	if ((map = FindObjectsData(mech->mapindex)))
	    if (MapUnderSpecialRules(map))
		damage = damage * MIN(100, MapGravity(map)) / 100;

    if (MechType(mech) == CLASS_MW)
	damage *= 40;

    spread = damage / 5;
   if (MechType(mech) != CLASS_MW && MechType(mech) != CLASS_BSUIT) {
    for (i = 0; i < spread; i++) {
	hitloc = (MechType(mech) == CLASS_VTOL ? (revspeed ? BSIDE : FSIDE) : FindHitLocation(mech, hitGroup, &iscritical, &isrear, NULL));
	if (hitloc < 0) hitloc = 0;
	DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, 5, -1,
	    -1, 0, 0);
	MechFloods(mech);
    	}
    if (damage % 5) {
	hitloc = (MechType(mech) == CLASS_VTOL ? (revspeed ? BSIDE : FSIDE) : FindHitLocation(mech, hitGroup, &iscritical, &isrear, NULL));
	DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical,
	    (damage % 5), -1, -1, 0, 0);
	MechFloods(mech);
    	}
    } else if (MechType(mech) == CLASS_BSUIT && damage > 0) {
	for (i = 0; i < NUM_SECTIONS; i++) {
	    if (GetSectOInt(mech, i) > 0) {
		DamageMech(mech, mech, 0, -1, i, 0, 0, (damage * Number(1,3)), -1, -1, 0, 0);
		MechFloods(mech);
	    }
	}
    }
    if (!drop_cheat_flag && !intentional)
	bsuit_stopswarmers(FindObjectsData(mech->mapindex), mech, 0);
    possible_mine_poof(mech, MINE_FALL);
    MechTurnDamage(mech) = 0;
    if (MechCarrying(mech) > 0)
    	mech_dropoff(GOD, mech, "");
    if (MechMove(mech) == MOVE_VTOL || MechMove(mech) == MOVE_FLY) {
	if (MechStatus2(mech) & CS_ON_LAND)
	    MechStatus(mech) |= COMBAT_SAFE; 
	}
    MarkForLOSUpdate(mech);
}

int mechs_in_hex(MAP * map, int x, int y, int friendly, int team)
{
    MECH *mech;
    int i, cnt = 0;

    for (i = 0; i < map->first_free; i++)
	if ((mech = FindObjectsData(map->mechsOnMap[i]))) {
	    if (MechX(mech) != x || MechY(mech) != y)
		continue;
	    if (Destroyed(mech))
		continue;
	    if (!(MechSpecials2(mech) & CARRIER_TECH) && IsDS(mech) && (Landed(mech) || !Started(mech))) {
		cnt += 2;
		continue;
	    }
	    if (MechType(mech) != CLASS_MECH)
		continue;
	    if (Jumping(mech))
		continue;
	    if (friendly < 0 || ((MechTeam(mech) == team) == friendly))
		cnt++;
	}
    return cnt;
}

enum {
    NORMAL, PUNCH, KICK
};

void cause_damage(MECH * att, MECH * mech, int dam, int table)
{
    int hitGroup, isrear, iscrit = 0, hitloc = 0;
    int i, sp = (dam - 1) / 5;

    if (!dam)
	return;
    if (att == mech)
	hitGroup = FRONT;
    else
	hitGroup = FindAreaHitGroup(att, mech);
    isrear = (hitGroup == BACK);
    if (Fallen(mech))
	table = NORMAL;
    for (i = 0; i <= sp; i++) {
	switch (table) {
	case NORMAL:
	    hitloc = FindHitLocation(mech, hitGroup, &iscrit, &isrear, att);
	    break;
	case PUNCH:
	    FindPunchLoc(mech, hitloc, hitGroup, iscrit, isrear);
	    break;
	case KICK:
	    FindKickLoc(mech, hitloc, hitGroup, iscrit, isrear);
	    break;
	}
	if (dam <= 0)
	    return;
	DamageMech(mech, att, (att == mech) ? 0 : 1,
	    (att == mech) ? -1 : MechPilot(att), hitloc, isrear, iscrit,
	    dam > 5 ? 5 : dam, 0, -1, 0, 0);
	dam -= 5;
    }
}

int domino_space_in_hex(MAP * map, MECH * me, int x, int y, int friendly,
    int mode, int cnt)
{
    int tar = Number(0, cnt - 1), i, head, td;
    MECH *mech = NULL;
/*    int team = MechTeam(me); */

    for (i = 0; i < map->first_free; i++)
	if ((mech = FindObjectsData(map->mechsOnMap[i]))) {
	    if (MechX(mech) != x || MechY(mech) != y)
		continue;
	    if (mech == me)
		continue;
	    if (IsDS(mech) && (Landed(mech) || !Started(mech))) {
		tar -= 2;
	    } else {
		if (!Started(mech))
		    continue;
		if (MechType(mech) != CLASS_MECH)
		    continue;
		if (Jumping(mech))
		    continue;
/*		if (friendly < 0 || ((MechTeam(mech) == team) == friendly))
		    tar--; */
		else
		    continue;
	    }
	    if (tar <= 0)
		break;
	}
    if (i == map->first_free)
	return 0;
    /* Now we got a mech we hit, accidentally or otherwise */
    /* Next, we figure out what'll happen */

    /* 'wannabe-charge' is entirely based on the directional difference */
    /* Multiplied by the speed - if both go in same direction at same speed,
       nothing untoward happens (unlikely, though) */
    /* Jumping to a hex with multiple guys is BAD Thing(tm), though */

    switch (mode) {
    case 1:
    case 2:
	head = MechJumpHeading(me);
	td = JumpSpeedMP(me, map) * (MechRTons(me) / 1024 + 5) / 10;
	break;
    default:
	head = MechFacing(me) + MechLateral(me);
	td =
	    fabs(((MechSpeed(me) - MechSpeed(mech) * cos((head -
			   (MechFacing(mech) +
				MechLateral(mech))) * (M_PI / 180.))) *
		MP_PER_KPH) * (MechRTons(me) / 1024 + 5) / 15);
	break;
    }
    if (td > 10)
	td = 10 + (td - 10) / 3;
    if (td <= 1)		/* No point in 1pt hits */
	return 0;
    switch (mode) {
    case 1:
    case 2:
	mech_notify(me, MECHALL, tprintf("You land on %s!",
		GetMechToMechID(me, mech)));
	mech_notify(mech, MECHALL, tprintf("%s lands on you!",
		GetMechToMechID(mech, me)));
	MechLOSBroadcasti(me, mech, "lands on %s!");
	if (IsDS(mech)) {
	    cause_damage(me, mech, td / 5, PUNCH);
	    cause_damage(me, me, td, KICK);
	} else {
	    cause_damage(me, mech, td /5, PUNCH);
	    cause_damage(me, me, td, KICK);
	}
	return 1;
    }
    mech_notify(me, MECHALL, tprintf("You run at %s!", GetMechToMechID(me,
		mech)));
    mech_notify(mech, MECHALL, tprintf("%s runs at you!",
	    GetMechToMechID(mech, me)));
    MechLOSBroadcasti(me, mech, "bumps at %s!");
    if (!IsDS(mech)) {
	cause_damage(me, mech, td / 5, NORMAL);
	cause_damage(me, me, td, NORMAL);
    } else {
	cause_damage(me, mech, td / 5, NORMAL);
	cause_damage(me, me, td, NORMAL);
    }
    MechChargeTarget(me) = -1;
    MechChargeTimer(me) = 0;
    MechChargeDistance(me) = 0;
    return 1;
}

int domino_space(MECH * mech, int mode)
{
    MAP *map = FindObjectsData(mech->mapindex);
/*    int cnt, fcnt; */
    int cnt;

    if (!map)
	return 0;
    if (MechType(mech) != CLASS_MECH)
	return 0;
    cnt = mechs_in_hex(map, MechX(mech), MechY(mech), -1, 0);
    if (cnt <= 2)
	return 0;
    /* Possible nastiness */
/*    if ((fcnt =
	    mechs_in_hex(map, MechX(mech), MechY(mech), 1,
	    MechTeam(mech))) > 2)
	return domino_space_in_hex(map, mech, MechX(mech), MechY(mech), 1,
	    mode, fcnt);
    else if (cnt > 6) */
    if (cnt > 2)
/*	return domino_space_in_hex(map, mech, MechX(mech), MechY(mech), 0,
	    mode, cnt - fcnt); */
	return domino_space_in_hex(map, mech, MechX(mech), MechY(mech), 0,
	    mode, cnt);
    return 0;
}

int possibly_domino_space(MECH *mech, int mode)
{
    MAP *map = FindObjectsData(mech->mapindex);
/*    int cnt, fcnt; */
    int cnt;

    if (!map)
        return 0;
    if (MechType(mech) != CLASS_MECH)
        return 0;
    cnt = mechs_in_hex(map, MechX(mech), MechY(mech), -1, 0);
    if (cnt <= 2)
        return 0;
    /* Possible nastiness */
/*    if ((fcnt =
            mechs_in_hex(map, MechX(mech), MechY(mech), 1,
            MechTeam(mech))) > 2)
        return fcnt;
    else if (cnt > 6) */
    if (cnt > 2)
/*	return cnt - fcnt;  */
	return cnt;
    return 0;
}

int charge_domino_space(MECH * mech) {

    MECH * target;

    target = getMech(MechChargeTarget(mech));

    if (target) {
        if ((MechX(mech) == MechX(target)) &&
            (MechY(mech) == MechY(target)))
            return 1;
        else
            return 0;
    } else {
        return 0;
    }

    return 0;
}

#if 0
void mech_turnpercent(dbref player, void *data, char *buffer)
{
MECH *mech = (MECH *) data;
char *args[2];
int newperc;
int numargs = 0;

cch(MECH_USUALO);
numargs = mech_parseattributes(buffer, args, 1);
if (numargs == 0) {
    notify(player, tprintf("Your current turn rate value is %d%%.", MechTurnPercent(mech)));
    return;
    }
newperc = atoi(args[0]);
DOCHECK(newperc < 1 || newperc > 0, "Invalid value must be between 1 and 100.");
MechTurnTrack(mech) = newperc;
notify(player, tprintf("Your turn rate value is now set to %d%.", MechTurnTrack(mech)));
return;
}
#endif
