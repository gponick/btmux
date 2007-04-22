#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "mech.ice.h"
#include "mech.ecm.h"
#include "p.mech.combat.h"
#include "p.mech.startup.h"
#include "p.mech.utils.h"
#include "p.mech.update.h"
#include "p.btechstats.h"
#include "p.mech.physical.h"
#include "p.bsuit.h"
#include "p.mine.h"
#include "p.mech.lite.h"
#include "p.mech.ice.h"
#include "p.eject.h"
#include "p.mech.hitloc.h"

extern int arc_override;

int fiery_death(MECH * mech)
{
    if (MechTerrain(mech) == FIRE) {
	if (is_aero(mech))
	    return 0;
	if (MechMove(mech) == MOVE_VTOL)
	    return 0;
	if (Destroyed(mech))
	    return 0;
	/* Cause various things */
	/* MWs _die_ */
	if (MechType(mech) == CLASS_MW) {
	    mech_notify(mech, MECHALL, "You feel tad bit too warm..");
	    mech_notify(mech, MECHALL, "You faint.");
	    DestroyMech(mech, mech, 0);
	    return 1;
	}
	/* Tanks may die */
	return 0;		/* Dumb idea */
	heat_effect(NULL, mech, 0);
	if (Destroyed(mech))
	    return 1;
    }
    if (MechTerrain(mech) == WATER && MechZ(mech) < 0 &&
        MechType(mech) == CLASS_BSUIT && !(MechSwarmTarget(mech) > 1) && !(MechSpecials2(mech) & WATERPROOF_TECH)) {
            mech_notify(mech, MECHALL, "You lose conciousness as water floods into your suit!");
            DestroyMech(mech, mech, 0);
            return 1;
    }
    return 0;
}

int bridge_w_elevation(MECH * mech)
{
    return -1;
}

void bridge_set_elevation(MECH * mech)
{
    if (MechZ(mech) < (MechUpperElevation(mech) - MoveMod(mech))) {
	if (Overwater(mech))
	    MechZ(mech) = 0;
	else
	    MechZ(mech) = bridge_w_elevation(mech);
	return;
    }
    MechZ(mech) = MechUpperElevation(mech);
}

int DSOkToNotify(MECH * mech)
{
    if (DSLastMsg(mech) > event_tick ||
	(event_tick - DSLastMsg(mech)) >= DS_SPAM_TIME) {
	DSLastMsg(mech) = event_tick;
	return 1;
    }
    return 0;
}

enum {
    JUMP, WALK_WALL, WALK_DROP, WALK_BACK
};

int collision_check(MECH * mech, int mode, int le, int lt)
{
    int e;

    if (Overwater(mech) && le < 0)
	le = 0;
    e = MechElevation(mech);
    if (MechRTerrain(mech) == ICE)
	if (le >= 0)
	    e = 0;
    if (le < (MechUpperElevation(mech) - MoveMod(mech)) && (lt == BRIDGE || lt == DBRIDGE)) {
	if (Overwater(mech))
	    le = 0;
	else
	    le = bridge_w_elevation(mech);
    }
#if 0
    if (MechZ(mech) <= 0 && (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE)) {
	if (Overwater(mech))
	    e = 0;
	else
	    e = bridge_w_elevation(mech);
    }
#endif
    if (e < 0 && Overwater(mech))
	e = 0;
    switch (mode) {
    case JUMP:
	if (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) {
	    if (MechZ(mech) < 0)
		return 1;
	    if (MechZ(mech) == (e - 1))
		return 1;
	    return 0;
	} else if (MechType(mech) == CLASS_VTOL && (MechRTerrain(mech) == LIGHT_FOREST || MechRTerrain(mech) == HEAVY_FOREST) && MechZ(mech) < (e + 2)) {
	    mech_notify(mech, MECHALL, "You notice a few twigs heading your way....");
	    return 1;
	} else
	    return (MechZ(mech) < e);
    case WALK_DROP:
	return (le - e) > MoveMod(mech);
    case WALK_WALL:
	return (e - le) > MoveMod(mech);
    case WALK_BACK:
	if (MechMove(mech) != MOVE_TRACK && MechType(mech) != CLASS_VTOL)
	    return (MechSpeed(mech) < 0 ? abs((le - e)) : 0);
    }
    return 0;
}

void CheckNavalHeight(MECH * mech, int oz);

void move_mech(MECH * mech)
{
    float newx = 0.0, newy = 0.0, dax, day;
    float xy_charge_dist, xscale;
    float jump_pos;

#ifdef ODDJUMP
    float rjump_pos, midmod;
#endif
    int x, y, upd_z = 0;
    MECH *target;
    MAP *mech_map;
    int oi, oz;
    int iced = 0;

    oz = MechZ(mech);
    mech_map = getMap(mech->mapindex);
    if (!mech_map && MechPilot(mech) >= 0)
	mech_map = ValidMap(MechPilot(mech), mech->mapindex);
    if (!mech_map) {
	mech_notify(mech, MECHALL,
	    "You are on an invalid map! Map index reset!");
	MechCocoon(mech) = 0;
	if (Jumping(mech))
	    mech_land(MechPilot(mech), (void *) mech, "");
	mech_shutdown(MechPilot(mech), (void *) mech, "");
	SendError(tprintf("move_mech:invalid map:Mech: %d  Index: %d",
		mech->mynum, mech->mapindex));
	mech->mapindex = -1;
	return;
    }
    if (mudconf.btech_newcharge && MechChargeTarget(mech) > 0)
	if (MechChargeTimer(mech)++ > 60) {
	    mech_notify(mech, MECHALL, "Charge timed out, charge reset.");
	    MechChargeTarget(mech) = -1;
	    MechChargeTimer(mech) = 0;
	    MechChargeDistance(mech) = 0;
	}


	if (Jumping(mech)) {
	    MarkForLOSUpdate(mech);
	    FindComponents(JumpSpeed(mech,
		    mech_map) * MOVE_MOD * MAPMOVEMOD(mech_map),
		MechJumpHeading(mech), &newx, &newy);
	    MechFX(mech) += newx;
	    MechFY(mech) += newy;
	    jump_pos =
		my_sqrt((MechFX(mech) - MechStartFX(mech)),
		(MechFY(mech) - MechStartFY(mech)));
#ifndef ODDJUMP
	    MechFZ(mech) = ((4 * JumpSpeedMP(mech, mech_map) * ZSCALE)
		/ (MechJumpLength(mech) * MechJumpLength(mech))) *
		jump_pos * (MechJumpLength(mech) - jump_pos) +
		MechStartFZ(mech) + jump_pos * (MechEndFZ(mech)
		- MechStartFZ(mech)) / (MechJumpLength(mech) * HEXLEVEL);
#else
	    rjump_pos = MechJumpLength(mech) - jump_pos;
	    if (rjump_pos < 0.0)
		rjump_pos = 0.0;
	    /* New flight path: Make a direct line from the origin to
	       destination, and imagine a 1-x^4 in relation to the 0 as
	       the line. y=1 = JumpTop offset, x=0 = middle of path,
	       x=-1 = beginning, x=1 = end */

	    midmod = jump_pos / MechJumpLength(mech);
	    midmod = (midmod - 0.5) * 2;
	    if (MechJumpTop(mech) >= (1 + JumpSpeedMP(mech, mech_map)))
		midmod = (1.0 - (midmod * midmod)) * MechJumpTop(mech);
	    else
		midmod =
		    (1.0 -
		    (midmod * midmod * midmod * midmod)) *
		    MechJumpTop(mech);
	    MechFZ(mech) =
		(rjump_pos * MechStartFZ(mech) +
		jump_pos * MechEndFZ(mech)) / MechJumpLength(mech) +
		midmod * ZSCALE;
#endif
	    MechZ(mech) = (int) (MechFZ(mech) / ZSCALE + 0.5);
#ifdef JUMPDEBUG
	    SendDebug(tprintf("#%d: %d,%d,%d (%d,%d,%d)", mech->mynum,
		    MechX(mech), MechY(mech), MechZ(mech),
		    (int) MechFX(mech), (int) MechFY(mech),
		    (int) MechFZ(mech)));
#endif
	    if ((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) &&
		collision_check(mech, JUMP, 0, 0) && MechZ(mech) > 0 && MechZ(mech) < MechUpperElevation(mech)) {
		mech_notify(mech, MECHALL,
		    "CRASH!! You crash at the bridge!");
		MechLOSBroadcast(mech, "crashes into the bridge!");
		MechFalls(mech, 1, 0);
		return;
	    }
	    if ((MechX(mech) == MechGoingX(mech)) &&
		(MechY(mech) == MechGoingY(mech))) {
		/*Ok.. in the hex. but no instant landings anymore, laddie */
		/*Range to point of origin is larger than whole jump's length */
		MapCoordToRealCoord(MechX(mech), MechY(mech), &dax, &day);
#ifdef ODDJUMP
		if (my_sqrt(dax - MechStartFX(mech),
			day - MechStartFY(mech)) <=
		    my_sqrt(MechFX(mech) - MechStartFX(mech),
			MechFY(mech) - MechStartFY(mech))) {
#endif
		    LandMech(mech, 0);
		    MechFX(mech) = (float) dax;
		    MechFY(mech) = (float) day;
#ifdef ODDJUMP
		}
#endif
	    }
	    if (MechRTerrain(mech) == ICE) {
		if (oz < -1 && MechZ(mech) >= -1)
		    break_thru_ice(mech);
		else if (oz >= -1 && MechZ(mech) < -1)
		    drop_thru_ice(mech);
	    }
	} else {
    switch (MechMove(mech)) {
    case MOVE_BIPED:
    case MOVE_QUAD:
    case MOVE_HOVER:
    case MOVE_TRACK:
    case MOVE_WHEEL:
	    if (fabs(MechSpeed(mech)) > 0.0) {
            FindComponents(MechSpeed(mech) * MOVE_MOD *
            MAPMOVEMOD(mech_map), MechLateral(mech) + MechFacing(mech), &newx, &newy);
            MechFX(mech) += newx;
            MechFY(mech) += newy;
            upd_z = 1;
            if (MechChargeTarget(mech) > 0 && mudconf.btech_newcharge) {
                xscale = 1.0 / SCALEMAP;
                xscale = xscale * xscale;
                xy_charge_dist = sqrt(xscale * newx * newx + YSCALE2 * newy * newy);
                MechChargeDistance(mech) += xy_charge_dist;
            }
        } else
            return;
        break;
/*    case MOVE_TRACK:
    case MOVE_WHEEL:
	if (fabs(MechSpeed(mech)) > 0.0) {
	    FindComponents(MechSpeed(mech) * MOVE_MOD *
		MAPMOVEMOD(mech_map), MechFacing(mech), &newx, &newy);
	    MechFX(mech) += newx;
	    MechFY(mech) += newy;
	    upd_z = 1;
	} else
	    return;
	break;
    case MOVE_HOVER:
	if (fabs(MechSpeed(mech)) > 0.0) {
	    FindComponents(MechSpeed(mech) * MOVE_MOD *
		MAPMOVEMOD(mech_map), MechLateral(mech) + MechFacing(mech), &newx, &newy);
	    MechFX(mech) += newx;
	    MechFY(mech) += newy;
	    upd_z = 1;
	} else
	    return;
	break;*/
    case MOVE_VTOL:
	if (Landed(mech) || ((fabs(MechSpeed(mech)) == 0.0) && (fabs(MechVerticalSpeed(mech)) == 0.0)))
	    return;
        MarkForLOSUpdate(mech);
	if (MechZ(mech) > ATMO_Z) {
	    mech_notify(mech, MECHALL, "Your craft just does not have the capabilities to go any higher!");
	    MechVerticalSpeed(mech) = 0.0;
	    MechZ(mech) = ATMO_Z;
	    MechFZ(mech) = MechZ(mech) * ZSCALE;
	    return;
	    } 
        FindComponents(MechSpeed(mech) * MOVE_MOD *
            MAPMOVEMOD(mech_map), MechLateral(mech) + MechFacing(mech),
            &newx, &newy);
        MechFX(mech) += newx;
        MechFY(mech) += newy;
        MechFZ(mech) += MechVerticalSpeed(mech) * MOVE_MOD;
        MechZ(mech) = MechFZ(mech) / ZSCALE;
        break;
    case MOVE_SUB:
	MarkForLOSUpdate(mech);
	FindComponents(MechSpeed(mech) * MOVE_MOD * MAPMOVEMOD(mech_map),
	    MechFacing(mech), &newx, &newy);
	MechFX(mech) += newx;
	MechFY(mech) += newy;
	MechFZ(mech) += MechVerticalSpeed(mech) * MOVE_MOD;
	MechZ(mech) = MechFZ(mech) / ZSCALE;
	break;
    case MOVE_FLY:
	if (!Landed(mech)) {
	    MarkForLOSUpdate(mech);
	    MechFX(mech) += MechStartFX(mech) * MOVE_MOD;
	    MechFY(mech) += MechStartFY(mech) * MOVE_MOD;
	    MechFZ(mech) += MechStartFZ(mech) * MOVE_MOD;
	    MechZ(mech) = MechFZ(mech) / ZSCALE;
	    if (MechZ(mech) > 14000) {
		MechZ(mech) = 14000;
		MechFZ(mech) = MechZ(mech) * ZSCALE;
		}
	    if (SpheroidDS(mech)) {
		if (MechZ(mech) < 10 && oz >= 10)
		    DS_LandWarning(mech, 1);
		else if (MechZ(mech) < (ATMO_Z / 2) && oz >= (ATMO_Z / 2))
		    DS_LandWarning(mech, 0);
		else if (MechZ(mech) < ATMO_Z && oz >= ATMO_Z) {
		    if (abs(MechDesiredAngle(mech)) != 90) {
			if (DSOkToNotify(mech)) {
			    mech_notify(mech, MECHALL, "As the craft enters the lower atmosphere, it's nose rises up for a clean landing..");
			    MechLOSBroadcast(mech, tprintf ("starts descending towards %d,%d..", MechX(mech), MechY(mech)));
			} else
			    mech_notify(mech, MECHALL, "Due to low altitude, climbing angle set to 90 degrees.");
			MechDesiredAngle(mech) = -90;
		    }
		    MechStartFX(mech) = 0.0;
		    MechStartFY(mech) = 0.0;
		    DS_LandWarning(mech, -1);
		}
	    }
	    if (MechZ(mech) >= ORBIT_Z && oz < ORBIT_Z) {
		mech_notify(mech, MECHALL, "You leave the upper atmopshere and enter orbit.");
		MechLOSBroadcast(mech, "leaves the upper atmopshere and enters orbit.");
	        MechVelocity(mech) /= 2;
		if (MechStatus2(mech) & CS_ON_LAND)
		    MechStatus(mech) |= COMBAT_SAFE;
	        } else if (MechZ(mech) < ORBIT_Z && oz >= ORBIT_Z) {
		    if (MechSpecials2(mech) & NOREENTRY_TECH) {
			mech_notify(mech, MECHALL, "This craft is not capable of re-entry!");
			MechFZ(mech) = (ORBIT_Z + 1) * ZSCALE;
			MechZ(mech) = MechFZ(mech) / ZSCALE;
			MechAngle(mech) = 90;
			MechDesiredAngle(mech) = 90;
			MechSpeed(mech) = 0;
			MechThrust(mech) = 0;
			MechDesiredThrust(mech) = 0;
			MechStartFX(mech) = 0;
			MechStartFY(mech) = 0;
			MechStartFZ(mech) = 0;
			} else {
			    mech_notify(mech, MECHALL, "You re-enter the upper atmopshere.");
			    MechLOSBroadcast(mech, "re-enters the upper atmosphere.");
			    MechVelocity(mech) *= 2;
			    if (MechStatus2(mech) & CS_ON_LAND)
				MechStatus(mech) &= ~COMBAT_SAFE;
			}
	        } else if (MechZ(mech) < ATMO_Z && oz >= ATMO_Z) {
		    mech_notify(mech, MECHALL, "You leave the upper atmosphere and enter the lower atmosphere.");
		    MechLOSBroadcast(mech, "leaves the upper atmosphere and enters the lower atmosphere.");
		    MechVelocity(mech) *= 2; 
		} else if (MechZ(mech) > ATMO_Z && oz <= ATMO_Z) {
		    mech_notify(mech, MECHALL, "You leave the lower atmosphere and enter the upper atmosphere.");
		    MechLOSBroadcast(mech, "leaves the lower atmosphere and enters the upper atmosphere.");
		    MechVelocity(mech) /= 2; 
		} 
	} else {
	    if (!(fabs(MechSpeed(mech)) > 0.0))
		return;
	    FindComponents(MechSpeed(mech) * MOVE_MOD * MAPMOVEMOD(mech_map), MechFacing(mech), &newx, &newy);
	    MechFX(mech) += newx;
	    MechFY(mech) += newy;
	    upd_z = 1;
	}
	break;
    case MOVE_HULL:
    case MOVE_FOIL:
	if (fabs(MechSpeed(mech)) > 0.0) {
	    FindComponents(MechSpeed(mech) * MOVE_MOD *
		MAPMOVEMOD(mech_map), MechFacing(mech), &newx, &newy);
	    MechFX(mech) += newx;
	    MechFY(mech) += newy;
	    MechZ(mech) = 0;
	    MechFZ(mech) = 0.0;
	} else
	    return;
	break;
    }
    }
    MechLastX(mech) = MechX(mech);
    MechLastY(mech) = MechY(mech);

    RealCoordToMapCoord(&MechX(mech), &MechY(mech), MechFX(mech),
	MechFY(mech));
#ifdef ODDJUMP
    if (Jumping(mech) && MechLastX(mech) == MechGoingX(mech) &&
	MechLastY(mech) == MechGoingY(mech) &&
	(MechX(mech) != MechLastX(mech) || MechY(mech) != MechLastY(mech))) {
	LandMech(mech, 0);
	MechFX(mech) -= newx;
	MechFY(mech) -= newy;
	MechFZ(mech) = MechEndFZ(mech);
	MechX(mech) = MechGoingX(mech);
	MechY(mech) = MechGoingY(mech);
	MapCoordToRealCoord(MechX(mech), MechY(mech), &MechFX(mech),
	    &MechFY(mech));
	MechZ(mech) = MechFZ(mech) / ZSCALE;
    }
#endif
    oi = mech->mapindex;
    CheckEdgeOfMap(mech);
    if (mech->mapindex != oi)
	mech_map = getMap(mech->mapindex);

    if (oi != mech->mapindex || MechLastX(mech) != MechX(mech) ||
	MechLastY(mech) != MechY(mech)) {
	if (MechCritStatus(mech) & HIDDEN)
	    {
	    mech_notify(mech, MECHALL, "You move too much and break your cover!");
	    MechLOSBroadcast(mech, "breaks its cover in the brush.");
	    MechCritStatus(mech) &= ~(HIDDEN);
	    }
	if (!mech || !mech_map) {
	    SendError(tprintf("Invalid pointer (%s) in move_mech()", (!mech ? "mech" : !mech_map ? "mech_map" : "wierd...."))); 
	    if (mech) {
		mech_notify(mech, MECHALL, "You are on an invalid map! Map index reset!");
		MechCocoon(mech) = 0;
		if (Jumping(mech))
		    mech_land(MechPilot(mech), (void *) mech, "");
		mech_shutdown(MechPilot(mech), (void *) mech, "");
		SendError(tprintf("move_mech:invalid map:Mech: %d  Index: %d", mech->mynum, mech->mapindex));
		mech->mapindex = -1;
		}
	    return;
	    }
	StopHiding(mech);
	x = MechX(mech);
	y = MechY(mech);
	MechTerrain(mech) = GetTerrain(mech_map, x, y);
	MechElev(mech) = GetElev(mech_map, x, y);
	if (upd_z) {
	    if (MechRTerrain(mech) == ICE) {
		if (oz < -1 && MechZ(mech) >= -1)
		    break_thru_ice(mech);
		else if (MechZ(mech) == 0)
		    if (possibly_drop_thru_ice(mech))
			iced = 1;
	    }
	    DropSetElevation(mech, 0);
	    /* To fix certain slide-under-ice-effect for _mechs_ */
	    if (MechType(mech) == CLASS_MECH && MechRTerrain(mech) == ICE
		&& oz == -1 && MechZ(mech) == -1) {
		MechZ(mech) = 0;
		MechFZ(mech) = MechZ(mech) * ZSCALE;
	    }
	}
	if (!iced)
	    NewHexEntered(mech, mech_map, newx, newy);
	if (MechX(mech) == x && MechY(mech) == y) {
	    MarkForLOSUpdate(mech);
	    MechFloods(mech);
	    steppable_base_check(mech, x, y);
/*
	    if (MechChargeTarget(mech) > 0 && mudconf.btech_newcharge)
		MechChargeDistance(mech)++;
*/
	    if (In_Character(mech->mynum)) {
		MechHexes(mech)++;
		if (!(MechHexes(mech) % PIL_XP_EVERY_N_STEPS))
		    if (RGotPilot(mech))
			AccumulatePilXP(MechPilot(mech), mech, 1);
	    }
	    domino_space(mech, 0);
	}
    }
    if ((MechMove(mech) == MOVE_VTOL || is_aero(mech)) && !Landed(mech))
	CheckVTOLHeight(mech);
    if (MechType(mech) == CLASS_VEH_NAVAL)
	CheckNavalHeight(mech, oz);
    if (MechChargeTarget(mech) != -1) {	/* CHARGE!!! */
	target = getMech(MechChargeTarget(mech));
	if (target) {
	    if (FaMechRange(mech, target) < .6) {
		ChargeMech(mech, target);
		MechChargeTarget(mech) = -1;
		MechChargeTimer(mech) = 0;
		MechChargeDistance(mech) = 0;
	    }
	} else {
	    mech_notify(mech, MECHPILOT, "Invalid CHARGE target!");
	    MechChargeTarget(mech) = -1;
	    MechChargeDistance(mech) = 0;
	    MechChargeTimer(mech) = 0;
	}
    }
    if (MechCarrying(mech) > 0) {
	target = getMech(MechCarrying(mech));
	if (target && target->mapindex == mech->mapindex) {
	    MirrorPosition(mech, target);
	    SetRFacing(target, MechRFacing(mech));
	}
    }
    bsuit_mirrorswarmers(mech_map, mech);
    fiery_death(mech);
}

void CheckNavalHeight(MECH * mech, int oz)
{
    if (MechRTerrain(mech) != WATER && MechRTerrain(mech) != ICE &&
	MechRTerrain(mech) != BRIDGE && MechRTerrain(mech) != DBRIDGE) {
	MechSpeed(mech) = 0.0;
	MechVerticalSpeed(mech) = 0;
	MechDesiredSpeed(mech) = 0.0;
	SetFacing(mech, 0);
	MechDesiredFacing(mech) = 0;
	return;
    }
    if (!oz && MechZ(mech) && MechElev(mech) > 1) {
	MarkForLOSUpdate(mech);
	MechZ(mech) = 0;
	MechLOSBroadcast(mech, "dives!");
	MechZ(mech) = -1;
    }
    if (MechFZ(mech) > 0.0) {
	if (MechVerticalSpeed(mech) > 0 && !MechZ(mech) && oz < 0) {
	    mech_notify(mech, MECHALL,
		"Your sub has reached surface and stops rising.");
	    MechLOSBroadcast(mech, tprintf("surfaces at %d,%d!",
		    MechX(mech), MechY(mech)));
	    /* Possible show-up message? */
	}
	MechZ(mech) = 0;
	MechFZ(mech) = 0.0;
	if (MechVerticalSpeed(mech) > 0)
	    MechVerticalSpeed(mech) = 0;
	return;
    }
    if (MechZ(mech) <= (MechLowerElevation(mech))) {
	MechZ(mech) = MIN(0, MechLowerElevation(mech));
	if (MechElevation(mech) > 0)
	    SendError(tprintf
		("Oddity: #%d managed to wind up on '%c' (%d elev.)",
		    mech->mynum, MechTerrain(mech), MechElev(mech)));
	MechFZ(mech) = ((5.0 * MechZ(mech) - 4) * ZSCALE) / 5.0;
	if (MechMove(mech) == MOVE_SUB) {
	    if (MechVerticalSpeed(mech) < 0) {
		MechVerticalSpeed(mech) = 0;
		mech_notify(mech, MECHALL,
		    "The sub has reached bottom and stops diving.");
	    }
#if 0
	    else
		mech_notify(mech, MECHALL, "The sub has reached bottom.");
#endif
	}
    }
}

void CheckVTOLHeight(MECH * mech)
{
    if (Landed(mech) || MechZ(mech) > ((MechRTerrain(mech) == ICE || MechRTerrain(mech) == WATER) ? 0 : ((MechRTerrain(mech) == HEAVY_FOREST ||
	MechRTerrain(mech) == LIGHT_FOREST) && (IsDS(mech) || is_aero(mech) ? MechVelocity(mech) > 40 : 1)) ? MechElevation(mech) + 2 : MechElevation(mech)))
	return;
    if (InWater(mech) && MechZ(mech) <= 0) {
	mech_notify(mech, MECHALL,
	    "You crash your vehicle into the water!!");
	mech_notify(mech, MECHALL,
	    "Water pours into the cockpit....gulp!");
	DestroyAndDump(mech);
	return;
    }
    if (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE)
	if (MechZ(mech) != (MechElevation(mech) - 1))
	    return;
    aero_land(MechPilot(mech), mech, "");
    if (Landed(mech))
	return;
    if (MechRTerrain(mech) == HEAVY_FOREST || MechRTerrain(mech) == LIGHT_FOREST) {
	mech_notify(mech, MECHALL,
		"You contemplate the meaning of life and decide to return to the ancient homes of your ancestors.");
	mech_notify(mech, MECHALL,
		"Don't poke your eye on that twig..... ");
	} else
	    mech_notify(mech, MECHALL,
		"CRASH!! You smash your toy into the ground!!");
    MechLOSBroadcast(mech, "crashes into the ground!");
    MechFalls(mech, 1 + fabs(MechVerticalSpeed(mech) / MP2), 0);

/*   mech_notify (mech, MECHALL, "Your vehicle is inoperable."); */
    {
    MAP *map;
    if (!(map = getMap(mech->mapindex)))
	return;
/*    MechZ(mech) = MechElevation(mech); */
    MechZ(mech) = Elevation(map, MechX(mech), MechY(mech));
    MechFZ(mech) = ZSCALE * MechZ(mech);
    MechSpeed(mech) = 0.0;
    MechVerticalSpeed(mech) = 0.0;
    MechAngle(mech) = 90;
    MechDesiredAngle(mech) = 90;
    MechVelocity(mech) = 0;
    }
/*   DestroyMech (mech, mech); */
}

void UpdateHeading(MECH * mech)
{
    int offset;
    int normangle;
    int mw_mod = 1;
    float tank_mod = 1;
    float maxspeed, omaxspeed;
    MAP *mech_map;

    if (MechFacing(mech) == MechDesiredFacing(mech))
	return;
    if (MechCritStatus(mech) & GYRO_DESTROYED) {
	mech_notify(mech, MECHPILOT, "Your destroyed gyro prevents you from changing headings");
	MechDesiredFacing(mech) = MechFacing(mech);
	return;
	}
#if 0
		float turnmod = 1.0;

		maxspeed = MMaxSpeed(mech);
		omaxspeed = MechDesiredSpeed(mech);
		normangle = MechRFacing(mech) - SHO2FSIM(MechDesiredFacing(mech));

		if (MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT)
		    offset = SHO2FSIM(120);
		else if (Jumping(mech)) {
		    mech_map = FindObjectsData(mech->mapindex);
	    	    offset = SHO2FSIM(1) * JumpSpeedMP(mech, mech_map) * 6;
		} else {
		    if ((!GetTurnMode(mech) || MechStatus2(mech) & SPRINTING || (MechType(mech) == CLASS_VTOL && MechCritStatus(mech) & TAILROTOR_DAMAGED)) && !(MechStatus2(mech) & EVADING)) {
			turnmod += ((omaxspeed > MechSpeed(mech) ? omaxspeed : MechSpeed(mech)) - 2.5 * MP1) / 5.0 * MP_PER_KPH;
			if (turnmod < 1.0)
 		 	    turnmod = 1.0;
			}
		    offset = SHO2FSIM(1) * 3 * maxspeed * MP_PER_KPH / turnmod;
		}
#endif
    maxspeed = MMaxSpeed(mech);
    if (MechType(mech) == CLASS_VTOL && (MechCritStatus(mech) & TAILROTOR_DAMAGED))
	maxspeed = (maxspeed / 2);
    if (is_aero(mech))
	maxspeed = maxspeed / 3;
    omaxspeed = maxspeed;
    normangle = MechRFacing(mech) - SHO2FSIM(MechDesiredFacing(mech));
    if (MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT)
	mw_mod = 60;
    else if (MechMove(mech) == MOVE_QUAD)
	mw_mod = 2;
    else if (MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VEH_NAVAL || MechType(mech) == CLASS_VTOL) {
	if (MechSpeed(mech) > MP19)
	    tank_mod = .5;
	else if (abs(MechSpeed(mech)) > MP14)
	    tank_mod = .6;
	else if (abs(MechSpeed(mech)) > MP9)
	    tank_mod = .7;
	else if (abs(MechSpeed(mech)) > MP4)
	    tank_mod = .8;
	else
	    tank_mod = 1.5;
	if (MechMove(mech) == MOVE_WHEEL)
	    tank_mod -= .05;
	else if (MechMove(mech) == MOVE_HOVER)
	    tank_mod -= .1;
	if (tank_mod <= .3)
	    tank_mod = .3;
	}
    if (mudconf.btech_fasaturn) {
#define FASA_TURN_MOD 3/2
	if (Jumping(mech))
	    offset = 2 * SHO2FSIM(1) * 2 * 360 * FASA_TURN_MOD / 60;
	else {
	    float ts = MechSpeed(mech);

	    if (ts < 0) {
		maxspeed = maxspeed * 2.0 / 3.0;
		ts = -ts;
	    }
	    if (ts > maxspeed || maxspeed < 0.1)	/* kludge */
		offset = 0;
	    else {
		offset =
		    SHO2FSIM(1) * 2 * 360 * FASA_TURN_MOD / 60 *
		    (maxspeed -
		    ts) * (omaxspeed / maxspeed) * ((int) mw_mod * tank_mod) * MP_PER_KPH / 6;	/* hmm. */
	    }
	}
    } else {
	if (Jumping(mech)) {
	    mech_map = FindObjectsData(mech->mapindex);
	    offset =
		SHO2FSIM(1) * 6 * JumpSpeedMP(mech, mech_map) * ((int) mw_mod * tank_mod);
	} else if (fabs(MechSpeed(mech)) < 1.0)
	    offset = SHO2FSIM(1) * 3 * maxspeed * MP_PER_KPH * ((int) mw_mod * tank_mod);
	else {
	    offset = SHO2FSIM(1) * 2 * maxspeed * MP_PER_KPH * ((int) mw_mod * tank_mod);
	    if ((SHO2FSIM(abs(normangle)) > offset) &&
		IsRunning(MechSpeed(mech), maxspeed)) {
		if (MechSpeed(mech) > maxspeed)
		    offset -= offset / 2 * maxspeed / MechSpeed(mech);
		else
		    offset -=
			offset / 2 * (3.0 * MechSpeed(mech) / maxspeed -
			2.0);
	    }
	}
    }
    /*   offset = offset * 2 * MOVE_MOD; - Twice as fast as this;dunno why - */
    offset = offset * MOVE_MOD;
    if (GetTurnMode(mech) && HasBoolAdvantage(MechPilot(mech), "maneuvering_ace"))
	offset = (offset * 3) / 2;
    if (MechStatus2(mech) & (SPRINTING|EVADING) && !HasBoolAdvantage(MechPilot(mech), "speed_demon"))
	offset = (offset * 2) / 3;
    if (normangle < 0)
	normangle += SHO2FSIM(360);
    if (IsDS(mech) && offset >= SHO2FSIM(10))
	offset = SHO2FSIM(10);
    if (normangle > SHO2FSIM(180)) {
	AddRFacing(mech, offset);
	if (MechFacing(mech) >= 360)
	    AddFacing(mech, -360);
	normangle += offset;
	if (normangle >= SHO2FSIM(360))
	    SetRFacing(mech, SHO2FSIM(MechDesiredFacing(mech)));
    } else {
	AddRFacing(mech, -offset);
	if (MechRFacing(mech) < 0)
	    AddFacing(mech, 360);
	normangle -= offset;
	if (normangle < 0)
	    SetRFacing(mech, SHO2FSIM(MechDesiredFacing(mech)));
    }
    MechCritStatus(mech) |= CHEAD;
    MarkForLOSUpdate(mech);
}

/* MPs lost for heat need to go off walking speed, not off total speed */
#define DECREASE_HEAT(spd) \
tempspeed *= (ceil((rint((maxspeed / 1.5) / MP1) - (spd/MP1) ) * 1.5) * MP1) / maxspeed

#define DECREASE_OLD(spd) \
tempspeed *= (maxspeed - (spd)) / maxspeed
#define INCREASE_OLD(spd) DECREASE_OLD(-(spd))

#define DECREASE_NEW(spd) \
tempspeed *= MP1 / (MP1 + spd)
#define INCREASE_NEW(spd) \
tempspeed *= (MP1 + spd/2) / MP1

#define DECREASE(s) DECREASE_NEW(s)
#define INCREASE(s) INCREASE_NEW(s)

/* If you want to simulate _OLDs, you have to add 1MP in some cases (eww) */

float terrain_speed(MECH * mech, float tempspeed, float maxspeed,
    int terrain, int elev)
{
	float ModMP = 0.0;
	int tons = MechTons(mech);

    switch (terrain) {
    case FIRE:
	if (MechSpecials(mech) & ICE_TECH)
	    ModMP = -MP2;
    case DESERT:
	if (MechType(mech) == CLASS_BSUIT || MechMove(mech) == MOVE_WHEEL || MechType(mech) == CLASS_MW)
	    ModMP = -MP2;
	else
	    ModMP = -MP1;
	break;
    case SNOW:
    if (MechMove(mech) != MOVE_HOVER)
    	ModMP = -MP1;
	if (MechType(mech) == CLASS_VEH_GROUND)
		ModMP = ((MechMove(mech) == MOVE_WHEEL) ? -MP1 : (MechMove(mech) == MOVE_TRACK)
		? MP1 : (MechMove(mech) == MOVE_HOVER) ? 0.0: 0.0);
	ModMP = (!ModMP ? -MP2 : ModMP + -MP2);
	break;
    case ROUGH:
    case BUILDING:
	if (MechType(mech) == CLASS_BSUIT)
	    ModMP = 0;
        else if (MechMove(mech) == MOVE_WHEEL)
	    ModMP = -MP2;
	else if (MechMove(mech) == MOVE_TRACK)
	    ModMP = 0.0;
	else
            ModMP = -MP1;
	break;
    case DBRIDGE:
	if (MechMove(mech) == MOVE_WHEEL)
	    ModMP = -MP5;
	else if (MechMove(mech) == MOVE_TRACK || MechMove(mech) == MOVE_QUAD)
	    ModMP = -MP3;
	else if (MechMove(mech) == MOVE_HOVER)
	    ModMP = -MP6;
	else
	    ModMP = -MP4;
	break;
    case MOUNTAINS:
        if (MechMove(mech) == MOVE_WHEEL)
            ModMP = -MP3;
        else if (MechMove(mech) == MOVE_TRACK || MechType(mech) == CLASS_BSUIT)
            ModMP = -MP1;
        else
            ModMP = -MP2;
        break;
    case LIGHT_FOREST:
	if (MechType(mech) == CLASS_BSUIT)
	    ModMP = 0;
	else if (MechMove(mech) == MOVE_HOVER || MechMove(mech) == MOVE_WHEEL)
  	    ModMP = -MP2;
	else
	    ModMP = -MP1;
	break;
    case HEAVY_FOREST:
	if (MechType(mech) == CLASS_BSUIT)
	    ModMP = -MP1;
	else if (MechMove(mech) == MOVE_HOVER || MechMove(mech) == MOVE_WHEEL)
	    ModMP = -MP3;
	else
	    ModMP = -MP2;
	break;
    case BRIDGE:
    case ROAD:
	if (!(MechStatus2(mech) & SPRINTING)) {
	    /* Track get +1MP, wheel +3MP, movement on Paved Surface - DJ */
            if (MechMove(mech) == MOVE_TRACK)
                    ModMP = MP1;
            else if (MechMove(mech) == MOVE_WHEEL)
                    ModMP = (tons <= 35 ? MP3 : tons <=70 ? MP2 : MP1);
	}
        break;
    case ICE:
	if (MechZ(mech) >= 0)
	    break;
	/* FALLTHRU */
	/* if he's under the ice/bridge, treat as water. */
    case WATER:
	if (MechMove(mech) == MOVE_BIPED || MechMove(mech) == MOVE_QUAD) {
	    if (elev <= -4)
	    ModMP = -MP4;
	    else if (elev == -3)
	    ModMP = -MP3;
	    else if (elev == -2)
		ModMP = -MP2;
	    else if (elev == -1)
		ModMP = -MP1;
		else if (elev == 0)
		ModMP = -MP1;
	}
	break;
       default:
    	return tempspeed;
    }
    if (mudconf.btech_terrainmode == 2) {
	/* 3059 mode */
	if (ModMP < (-MP1)) {
	    tempspeed *= (MP1) / (-ModMP);
	} else if (ModMP < 0.0) {
	    tempspeed *= 2.0 / 3.0;
	} else {
	    INCREASE_OLD(ModMP);
	}
    } else if (mudconf.btech_terrainmode == 1) {

	/* OLD */
	if (ModMP < 0.0) {
	    DECREASE_OLD((-ModMP));
	} else {
	    INCREASE_OLD(ModMP);
	}
    } else if (mudconf.btech_terrainmode == 0) {

	/* NEW */
	if (ModMP < 0.0) {
	    DECREASE_NEW((-ModMP));
	} else {
	    INCREASE_NEW(ModMP);
	}
    } else {
	/* default mode = 0 */
	if (ModMP < 0.0) {
	    DECREASE_NEW((-ModMP));
	} else {
	    INCREASE_NEW(ModMP);
	}
    }

    return tempspeed;
}

void UpdateSpeed(MECH * mech)
{
    float acc, tempspeed, maxspeed;
    MECH *target;

    if (!(!Fallen(mech) && !Jumping(mech) && (MechMaxSpeed(mech) > 0.0)))
	return;
    tempspeed = fabs(MechDesiredSpeed(mech));
    maxspeed = MMaxSpeed(mech);
    if (maxspeed < 0.0)
	maxspeed = 0.0;
    if (MechDesiredSpeed(mech) >= maxspeed)
            MechDesiredSpeed(mech) = maxspeed;
    if (MechHeat(mech) >= 5.) {
	if (MechHeat(mech) >= 25.)
	    DECREASE_HEAT(MP5);
	else if (MechHeat(mech) >= 20.)
	    DECREASE_HEAT(MP4);
	else if (MechHeat(mech) >= 15.)
	    DECREASE_HEAT(MP3);
	else if (MechHeat(mech) >= 10.)
	    DECREASE_HEAT(MP2);
	else if (!((MechSpecials(mech) & TRIPLE_MYOMER_TECH) &&
		MechHeat(mech) >= 9))
	    DECREASE_HEAT(MP1);
    }
    if (MechType(mech) != CLASS_MW && MechMove(mech) != MOVE_VTOL && (MechMove(mech) != MOVE_FLY || Landed(mech)))
	tempspeed = terrain_speed(mech, tempspeed, maxspeed, MechRTerrain(mech), MechElevation(mech));

    if (MechStatus(mech) & JELLIED && (MechType(mech) != CLASS_MECH || MechSpecials(mech) & ICE_TECH))
	tempspeed -= (maxspeed <= MP9 ? MP1 : maxspeed <= MP12 ? MP2 : MP3);

    if (MechCritStatus(mech) & CHEAD) {
#if 0
			float turnmod = 1.0;

			if (MechFacing(mech) != MechDesiredFacing(mech)) {
				if ((!GetTurnMode(mech) || MechStatus2(mech) & SPRINTING ||
				(MechType(mech) == CLASS_VTOL && MechCritStatus(mech) & TAILROTOR_DAMAGED))
				&& !(MechStatus2(mech) & EVADING)) {
					turnmod += (MechDesiredSpeed(mech) - 2.5 * MP1) / 5.0 * MP_PER_KPH;
					if (turnmod < 1.0)
						turnmod = 1.0;
				}
				tempspeed -= tempspeed / 2 / turnmod;
			}
#endif
	if (mudconf.btech_slowdown == 2) {
	    /* _New_ slowdown based on facing vs desired difference */
	    int dif = MechFacing(mech) - MechDesiredFacing(mech);

	    if (dif < 0)
		dif = -dif;
	    if (dif > 180)
		dif = 360 - dif;
	    if (dif) {
		dif = (dif - 1) / 30;
		dif = (dif + 2);	/* whee */
		/* dif = 2 to 7 */
		tempspeed = tempspeed * (10 - dif) / 10;
	    }
	} else if (mudconf.btech_slowdown == 1) {
	    if (MechFacing(mech) != MechDesiredFacing(mech))
		tempspeed = tempspeed * 2.0 / 3.0;
	    else
		tempspeed = tempspeed * 3.0 / 4.0;
	}
        if ((GetTurnMode(mech) && HasBoolAdvantage(MechPilot(mech), "maneuvering_ace")) || (MechStatus2(mech) & (SPRINTING|EVADING) && !HasBoolAdvantage(MechPilot(mech), "speed_demon")))
	    tempspeed = (tempspeed * 2) / 3;
        MechCritStatus(mech) &= ~CHEAD;
    }
    if (MechLateral(mech)) {
#if 0
	    tempspeed = WalkingSpeed(tempspeed);
#endif
        if (MechMove(mech) != MOVE_QUAD) {
	    if (HasBoolAdvantage(MechPilot(mech), "maneuvering_ace")) {
	        DECREASE_OLD(MP2);
            } else {
	        DECREASE_OLD(MP3);
	    }
     	} else {
	    DECREASE_OLD(MP1);	/* In truth 1 MP */
	}
    }
    if (tempspeed <= 0.0)
	tempspeed = 0.0;
    if (MechDesiredSpeed(mech) < 0.)
	tempspeed = -tempspeed;

    if (tempspeed != MechSpeed(mech)) {
#if 0
			if (tempspeed > MechSpeed(mech) && MechType(mech) == CLASS_VEH_GROUND) {
				if ((MechMove(mech) == MOVE_TRACK || MechMove(mech) == MOVE_WHEEL)
				 && (MechRTerrain(mech) == ROAD || MechRTerrain(mech) == BRIDGE)) {
			        if (MechMove(mech) == MOVE_TRACK)
			            maxspeed += MP1;
			        else if (MechMove(mech) == MOVE_WHEEL)
			            maxspeed += (MechTons(mech) <= 35 ? MP3 : MechTons(mech) <=70 ? MP2 : MP1);
			    }
				if (MechSpeed(mech) <= WalkingSpeed(maxspeed))
					maxspeed = WalkingSpeed(maxspeed);
				else
					maxspeed = maxspeed / 3;

			} else if (MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT)
				maxspeed = maxspeed * 3;
			acc = maxspeed * maxspeed * MP_PER_KPH / 60 * MOVE_TICK;
#endif
	if (MechMove(mech) == MOVE_QUAD)
	    acc = maxspeed / 10.;
	else if (MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW)
	    acc = maxspeed / 2.;
	else if (MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VEH_NAVAL || MechType(mech) == CLASS_VTOL)
	    acc = maxspeed / 30.;
	else
	    acc = maxspeed / 20.;
	if (HasBoolAdvantage(MechPilot(mech), "speed_demon"))
	    acc *= 1.25;
	if (tempspeed < MechSpeed(mech)) {
	    /* Decelerating */
	    MechSpeed(mech) -= acc;
	    if (tempspeed > MechSpeed(mech))
		MechSpeed(mech) = tempspeed;
	} else {
	    /* Accelerating */
	    MechSpeed(mech) += acc;
	    if (tempspeed < MechSpeed(mech))
		MechSpeed(mech) = tempspeed;
	}
	}
    if (MechCarrying(mech) > 0) {
	target = getMech(MechCarrying(mech));
	if (target)
	    MechSpeed(target) = MechSpeed(mech);
    }
}




int OverheatMods(MECH * mech)
{
    int returnValue;

    if (MechHeat(mech) >= 24.) {
	/* +4 to fire... */
	returnValue = 4;
    } else if (MechHeat(mech) >= 17.) {
	/* +3 to fire... */
	returnValue = 3;
    } else if (MechHeat(mech) >= 13.) {
	/* +2 to fire... */
	returnValue = 2;
    } else if (MechHeat(mech) >= 8.) {
	/* +1 to fire... */
	returnValue = 1;
    } else {
	returnValue = 0;
    }
    return (returnValue);
}

void ammo_explosion(MECH * attacker, MECH * mech, int ammoloc, int ammocritnum, int damage)
{
	int i, acid = 0, totint = 0, newloc = ammoloc, eject = 1;

    if (IsAcid(Ammo2WeaponI(GetPartType(mech, ammoloc, ammocritnum))))
	acid = 1;

    if (MechType(mech) == CLASS_MW) {
	mech_notify(mech, MECHALL, "Your weapon's ammo explodes!");
	MechLOSBroadcast(mech, "'s weapon's ammo explodes!");
    } else {
	mech_notify(mech, MECHALL, "Ammunition explosion!");
	if (GetPartMode(mech, ammoloc, ammocritnum) & INFERNO_MODE)
	    MechLOSBroadcast(mech,
		"is suddenly enveloped by a brilliant fireball!");
	else
	    MechLOSBroadcast(mech, "has an internal ammo explosion!");
    }
    DestroyPart(mech, ammoloc, ammocritnum);
    SetPartData(mech, ammoloc, ammocritnum, 0);
    if (!attacker)
	return;
    if (GetPartMode(mech, ammoloc, ammocritnum) & INFERNO_MODE) {
	Inferno_Hit(mech, mech, damage / 4, 0);
	damage = damage / 2;
    }
    if (MechType(mech) == CLASS_BSUIT)
	DamageMech(mech, attacker, 0, -1, ammoloc, 0, 0, damage, 0, -1, 0, 0);
    else {
	if (MechType(mech) == CLASS_MECH) {
	    while (newloc != -1) {
		totint += GetSectInt(mech, newloc);
		if (MechSections(mech)[newloc].config & CASE_TECH) {
		    eject = 0;
		    break;
		    }
		newloc = TransferTarget(mech, newloc);
		}
	    if (eject && totint <= damage) {
		autoeject(MechPilot(mech), mech, attacker);
		}
	}
	DamageMech(mech, attacker, 0, -1, ammoloc, 0, 0, -1, damage, -1, 0, 0);
	}
	if (acid) {
	for (i = 0; i < NUM_SECTIONS; i++) {
	    if ((MechType(mech) == CLASS_MECH && i == HEAD) || SectIsDestroyed(mech, i))
		continue;
	    StartAcidEffect(mech, mech, i, 0, 0, 1);
	    }
	}
    mech_notify(mech, MECHPILOT, "You take personal injury from the ammunition explosion!");
    if (HasBoolAdvantage(MechPilot(mech), "pain_resistance"))
	headhitmwdamage(mech, 1);
    else
        headhitmwdamage(mech, 2);
}

void HandleOverheat(MECH * mech)
{
    int avoided = 0;
    MAP *mech_map;
    int roll = 2, bth = 2, mod;
    int hasinf = (MechCritStatus(mech) & INFERNO_AMMO), realinf;
    int ammoloc, ammocritnum, damage, i, ii;
    float heat;

    heat = MechHeat(mech);
    if ((heat < 14.) || (hasinf && heat < 10.))
	return;
    /* Has it been a TURN already ? */
    if ((MechHeatLast(mech) + TURN) > event_tick)
	return;
    MechHeatLast(mech) = event_tick;

  if (!isPlayer(MechPilot(mech))) {
    roll = Roll();
    if (heat >= 30.) {
    bth = 13;
    mod = 9;
    } else if (heat >= 26.) {
	/* Shutdown avoid on 10+ */
	if (roll >= (bth = 10))
	    avoided = 1;
    } else if (heat >= 22.) {
	/* Shutdown avoid on 8+ */
	if (roll >= (bth = 8))
	    avoided = 1;
    } else if (heat >= 18.) {
	/* Shutdown avoid on 6+ */
	if (roll >= (bth = 6))
	    avoided = 1;
    } else if (heat >= 14.) {
	/* Shutdown avoid on 4+ */
	if (roll >= (bth = 4))
	    avoided = 1;
    }
    if (Started(mech)) {
        mech_notify(mech, MECHALL, "You franticly attempt to override the shutdown process!");
        mech_notify(mech, MECHALL, tprintf("BTH : %d  Roll : %d", bth, roll));
	}
} else {
    mech_notify(mech, MECHALL, "You franticly attempt to override the shutdown process!");
    avoided = char_getskillsuccess(MechPilot(mech), "computer", (heat >= 30. ? 8 : heat >= 26. ? 6 : heat >= 22. ? 4 : heat >=18. ? 2 : 0), 1);
    if (avoided)
	AccumulateComputerXP(MechPilot(mech), mech, 1);
}

    if (!(avoided) && Started(mech)) {
	if (MechStatus(mech) & STARTED)
	    mech_notify(mech, MECHALL, "%ci%crReactor shutting down...%c");
	if (Jumping(mech) || OODing(mech) || (is_aero(mech) &&
		!Landed(mech))) {
	    mech_notify(mech, MECHALL, "%chYou fall from the sky!!!!!%c");
	    MechLOSBroadcast(mech, "falls from the sky!");
	    mech_map = getMap(mech->mapindex);
	    MechFalls(mech, JumpSpeedMP(mech, mech_map), 0);
	    domino_space(mech, 2);
	} else
	    MechLOSBroadcast(mech, "stops in mid-motion!");
	MechStartupLast(mech) = event_tick;
	Shutdown(mech);
	StopMoving(mech);
	StopStand(mech);
    }
    avoided = 0;
    /* Ammo */

  if (!isPlayer(MechPilot(mech))) {
    if (heat >= 19.) {
	roll = Roll();
	if (heat >= 28.) {
	    /* Ammo explosion (Avoid 8+) */
	    if (roll >= (bth = 8))
		avoided = 1;
	} else if (heat >= 23.) {
	    /* Ammo explosion (Avoid 6+) */
	    if (roll >= (bth = 6))
		avoided = 1;
	} else if (heat >= 19.) {
	    /* Ammo explosion (Avoid 4+) */
	    if (roll >= (bth = 4))
		avoided = 1;
	}
	if (!(avoided)) {
	    damage = FindDestructiveAmmo(mech, &ammoloc, &ammocritnum);
	    if (damage) {
		/* BOOM! */
		/* That's going to hurt... */
        	mech_notify(mech, MECHALL, "You have a bad feeling about something.....");
	        mech_notify(mech, MECHALL, tprintf("BTH : %d  Roll : %d", bth, roll));
		ammo_explosion(mech, mech, ammoloc, ammocritnum, damage);
	    } else
		mech_notify(mech, MECHALL,
		    "You have no ammunition, lucky you!");
	}
    }
} else {
    if (heat >= 19. && (damage = FindDestructiveAmmo(mech, &ammoloc, &ammocritnum))) {
	mech_notify(mech, MECHALL, "You roll to avoid an ammunition explosion!");
	avoided = char_getskillsuccess(MechPilot(mech), "computer", (heat >= 28. ? 4 : heat >= 23. ? 2 : 0), 1);
	if (!(avoided)) {
            if (damage) {
                mech_notify(mech, MECHALL, "You have a bad feeling about something.....");
                ammo_explosion(mech, mech, ammoloc, ammocritnum, damage);
            }
        }
    }
}
  if (!isPlayer(MechPilot(mech))) {
    /* Inferno Ammo */
    if ((heat >= 10.) && hasinf) {
	roll = Roll();
	if (heat >= 28.) {
	    /* Inferno explosion (Avoid 12+) */
	    if (roll >= (bth = 12))
		avoided = 1;
	} else if (heat >= 23.) {
	    /* Inferno explosion (Avoid 10+) */
	    if (roll >= (bth = 10))
		avoided = 1;
	} else if (heat >= 19.) {
	    /* Ammo explosion (Avoid 8+) */
	    if (roll >= (bth = 8))
		avoided = 1;
	} else if (heat >= 14.) {
	    /* Ammo explosion (Avoid 6+) */
	    if (roll >= (bth = 6))
		avoided = 1;
	} else if (heat >= 10.) {
	    /* Ammo explosion (Avoid 4+) */
	    if (roll >= (bth = 4))
		avoided = 1;
	}
	if (!(avoided)) {
	    damage = 0;
	    realinf = 0;
            for (i = 0; i < NUM_SECTIONS; i++)
		for (ii = 0; ii < NUM_CRITICALS; ii++) {
		    if (IsAmmo(GetPartType(mech, i, ii)) &&
		 	(GetPartMode(mech, i, ii) & INFERNO_MODE) &&
			(GetPartData(mech, i, ii) > 0)) {
			  realinf = 1;
			  ammoloc = i;
			  ammocritnum = ii;
			  damage = (GetPartData(mech, ammoloc, ammocritnum) *
			  GunStat(Weapon2I(GetPartType(mech, ammoloc, ammocritnum)), GetPartMode(mech, ammoloc, ammocritnum), GUN_DAMAGE));
			  }
		}
	    if (damage && realinf) {
		/* BOOM! */
		/* That's going to hurt... */
        	mech_notify(mech, MECHALL, "You have a bad feeling about something and then remember your inferno ammo.....");
	        mech_notify(mech, MECHALL, tprintf("BTH : %d  Roll : %d", bth, roll));
		ammo_explosion(mech, mech, ammoloc, ammocritnum, damage);
	    } else if (!realinf)
		MechCritStatus(mech) &= ~(INFERNO_AMMO);
	}
    }
} else {
    if ((heat >= 10.) && hasinf) {
	mech_notify(mech, MECHALL, "You roll to avoid an inferno explosion!");
	avoided = char_getskillsuccess(MechPilot(mech), "computer", (heat >= 28. ? 8 : heat >= 23. ? 6 : heat >= 19. ? 4 : heat >= 14. ? 2 : 0), 1);
        if (!(avoided)) {
            damage = 0;
            realinf = 0;
            for (i = 0; i < NUM_SECTIONS; i++)
                for (ii = 0; ii < NUM_CRITICALS; ii++) {
                    if (IsAmmo(GetPartType(mech, i, ii)) &&
                        (GetPartMode(mech, i, ii) & INFERNO_MODE) &&
                        (GetPartData(mech, i, ii) > 0)) {
                          realinf = 1;
                          ammoloc = i;
                          ammocritnum = ii;
                          damage = (GetPartData(mech, ammoloc, ammocritnum) *
                          GunStat(Weapon2I(GetPartType(mech, ammoloc, ammocritnum)), GetPartMode(mech, ammoloc, ammocritnum), GUN_DAMAGE));
                          }
                }
            if (damage && realinf) {
                mech_notify(mech, MECHALL, "You have a bad feeling about something and then remember your inferno ammo.....");
                ammo_explosion(mech, mech, ammoloc, ammocritnum, damage);
            } else if (!realinf)
                MechCritStatus(mech) &= ~(INFERNO_AMMO);
        }
    }
  }
}

/* Enable for heatsink on/off msgs. */
/* #define HEAT_DEBUG_MSGS */

static int EnableSomeHS(MECH * mech, int numsinks) {
    numsinks = MIN(numsinks, (MechSpecials(mech) & (DOUBLE_HEAT_TECH|CLAN_TECH)) ? 4 : 2);
    numsinks = MIN(numsinks, MechDisabledHS(mech));
    if (!numsinks)
	return 0;
    MechDisabledHS(mech) -= numsinks;
    MechMinusHeat(mech) += numsinks; /* We dont check for water and such after enabling them, only the next tic. */
#ifdef HEAT_DEBUG_MSGS
    mech_notify(mech, MECHALL, tprintf("%%cg%d heatsink%s kick%s into action.%%c", numsinks, numsinks == 1 ? "" : "s", numsinks == 1 ? "s" : ""));
#endif
    return numsinks;
}

static int DisableSomeHS(MECH * mech, int numsinks) {
    numsinks = MIN(numsinks, (MechSpecials(mech) & (DOUBLE_HEAT_TECH|CLAN_TECH)) ? 4 : 2);
    numsinks = MIN(numsinks, MechActiveNumsinks(mech));
    if (!numsinks)
	return 0;
    MechDisabledHS(mech) += numsinks;
    MechMinusHeat(mech) -= numsinks; /* Submerged heatsinks silently still dissipate some heat */
#ifdef HEAT_DEBUG_MSGS
    mech_notify(mech, MECHALL, tprintf("%%cy%d heatsink%s hum%s into silence.%%c", numsinks, numsinks == 1 ? "" : "s", numsinks == 1 ? "s" : ""));
#endif
    return numsinks;
}

void UpdateHeat(MECH * mech)
{
    int legsinks;
    float maxspeed;
    float intheat;
    float inheat;
    MAP *map;

    if (MechType(mech) != CLASS_MECH && MechType(mech) != CLASS_AERO)
	return;
    inheat = MechHeat(mech);
    maxspeed = MMaxSpeed(mech);
/*    if ((MechHeat(mech) >= 9.) &&
	(MechSpecials(mech) & TRIPLE_MYOMER_TECH))*/
	/* maxspeed += 1.5 * MP1; */
/*	maxspeed = (WalkingSpeed(maxspeed) + MP1) * 1.5;*/
/*	maxspeed = ceil((rint((maxspeed / 1.5) / MP1) + 1) * 1.5) * MP1;*/
/*    if (MechSpecials(mech) & HARDA_TECH)
	maxspeed = (maxspeed <= MP1 ? maxspeed : maxspeed - MP1);*/
    MechPlusHeat(mech) = 0.;
    if (MechTerrain(mech) == FIRE && MechType(mech) == CLASS_MECH)
	MechPlusHeat(mech) += 5.;
    if (MechTerrain(mech) == DESERT && MechType(mech) == CLASS_MECH)
	MechPlusHeat(mech) += 2.;
    if (fabs(MechSpeed(mech)) > 0.0 && !(MechSpecials(mech) & ICE_TECH)) {
	if (MechStatus2(mech) & SPRINTING) {
	    MechPlusHeat(mech) += 3.;
	} else if (MechStatus2(mech) & EVADING) {
	    MechPlusHeat(mech) += 2.;
	} else if (IsRunning(MechSpeed(mech), maxspeed)) {
	    MechPlusHeat(mech) += ((MechSpecials(mech) & XXL_TECH) ? 6. : 2.);
	} else {
	    MechPlusHeat(mech) += ((MechSpecials(mech) & XXL_TECH) ? 4. : 1.);
	}
    } else if (MechSpecials(mech) & XXL_TECH) {
	MechPlusHeat(mech) += 2.;
    }
    if (Jumping(mech) && !is_aero(mech))
	MechPlusHeat(mech) +=
	    (((MechJumpSpeed(mech) * MP_PER_KPH >
	    (MechSpecials(mech) & XXL_TECH ? 6. : 3.)) ? MechJumpSpeed(mech) * MP_PER_KPH :
	    (MechSpecials(mech) & XXL_TECH ? 6. : 3.)) * (MechSpecials(mech) & XXL_TECH ? 2. :
		 1.)) * (MechSpecials(mech) & ICE_TECH ? 2. : 1.);

    if (Started(mech) && !(MechSpecials(mech) & ICE_TECH))
	MechPlusHeat(mech) += (float) MechEngineHeat(mech);

    intheat = MechPlusHeat(mech);

    MechPlusHeat(mech) += MechWeapHeat(mech);

    /* ADD Water effects here */
    if ((InWater(mech) && MechZ(mech) <= -1) || MechRTerrain(mech) == SNOW) {
	legsinks = FindLegHeatSinks(mech);
	legsinks = (legsinks > 4) ? 4 : legsinks;
	if ((MechZ(mech) == -1 && !Fallen(mech)) || MechRTerrain(mech) == SNOW ) {
	    MechMinusHeat(mech) =
		(2 * MechActiveNumsinks(mech) >
		legsinks + MechActiveNumsinks(mech)) ? (float) (legsinks +
		MechActiveNumsinks(mech)) : (float) (2 * MechActiveNumsinks(mech));
	} else {
	    MechMinusHeat(mech) =
		(2 * MechActiveNumsinks(mech) >
		6 + MechActiveNumsinks(mech)) ? (float) (6 +
		MechActiveNumsinks(mech)) : (float) (2 * MechActiveNumsinks(mech));
	}
    } else {
	MechMinusHeat(mech) = (float) (MechActiveNumsinks(mech));
    }
    if (MechStatus(mech) & JELLIED && !(MechSpecials(mech) & ICE_TECH)) {
	MechMinusHeat(mech) = MechMinusHeat(mech) - 6;
	if (MechMinusHeat(mech) < 0)
	    MechMinusHeat(mech) = 0;
    }
    if ((MechStatus2(mech) & NULLSIG_ACTIVE) || StealthAndECM(mech)) {
	MechPlusHeat(mech) += 10.;
    }

    if ((MechStatus2(mech) & CAPACITOR_ON) && Started(mech)) {
	MechPlusHeat(mech) += (float) (NumCapacitorsOn(mech) * 5.);
    }

    if (InSpecial(mech))
	if ((map = FindObjectsData(mech->mapindex)))
	    if (MapUnderSpecialRules(map))
		if (MapTemperature(map) < -30 || MapTemperature(map) > 50) {
		    if (MapTemperature(map) < -30)
			MechMinusHeat(mech) +=
			    (-30 - MapTemperature(map) + 9) / 10;
		    else
			MechMinusHeat(mech) -=
			    (MapTemperature(map) - 50 + 9) / 10;
		}

    /* Handle heat cutoff now */
    /* En/DisableSomeHS() take care of MechMinusHeat also. */
    if (Heatcutoff(mech)) {
	float overheat = MechPlusHeat(mech) - MechMinusHeat(mech);
	if (overheat >= 10.)
	    EnableSomeHS(mech, floor(overheat - 10.) + 1);
	else if (overheat < 9.)
	    DisableSomeHS(mech, floor(9. - overheat) + 1);
    } else if (MechDisabledHS(mech))
    	EnableSomeHS(mech, 100); /* Just enable as many as possible */

    MechHeat(mech) = MechPlusHeat(mech) - MechMinusHeat(mech);

    /* No lowering of heat if heat is under 9 */
    MechWeapHeat(mech) -=
	(MechMinusHeat(mech) - intheat) / WEAPON_RECYCLE_TIME;
    if (MechWeapHeat(mech) < 0.0)
	MechWeapHeat(mech) = 0.0;


    if (MechHeat(mech) < 0.0)
	MechHeat(mech) = 0.0;

    if ((event_tick % TURN) == 0)
	if (MechCritStatus(mech) & LIFE_SUPPORT_DESTROYED || (MechHeat(mech) > 30.)) {
	    if ((MechHeat(mech) > 25.) || (MechSpecials2(mech) & TORSOCOCKPIT_TECH && MechHeat(mech) > 15.)) {
		mech_notify(mech, MECHPILOT,
		    "You take personal injury from heat!!");
		headhitmwdamage(mech,
		    MechCritStatus(mech) & LIFE_SUPPORT_DESTROYED ? 2 : 1);
	    } else if (((MechHeat(mech) >= 15.) || (MechSpecials2(mech) & TORSOCOCKPIT_TECH && MechHeat(mech) > 0.)) ||
			(InVacuum(mech) || (MechRTerrain(mech) == WATER && (MechZ(mech) <= -2 || (Fallen(mech) && MechZ(mech) < 0)))))    {
		mech_notify(mech, MECHPILOT,
		    "You take personal injury from heat!!");
		headhitmwdamage(mech, 1);
	    }
	}
    if (MechHeat(mech) >= 19) {
	if (inheat < 19) {
	    mech_notify(mech, MECHALL,
		"%ch%cr=====================================");
	    mech_notify(mech, MECHALL,
		"Your Excess Heat indicator turns RED!");
	    mech_notify(mech, MECHALL,
		"=====================================%c");
	}
    } else if (MechHeat(mech) >= 14) {
	if (inheat >= 19 || inheat < 14) {
	    mech_notify(mech, MECHALL,
		"%ch%cy=======================================");
	    mech_notify(mech, MECHALL,
		"Your Excess Heat indicator turns YELLOW");
	    mech_notify(mech, MECHALL,
		"=======================================%c");
	}
    } else {
	if (inheat >= 14 || (MechHeat(mech) > 9 && inheat <= 9)) {
	    mech_notify(mech, MECHALL,
		"%cg======================================");
	    mech_notify(mech, MECHALL,
		"Your Excess Heat indicator turns GREEN");
	    mech_notify(mech, MECHALL,
		"======================================%c");
/*		if (!Destroyed(mech) && !Started(mech) && inheat >= 14 && MechHeat(mech) < 14)
			mech_startup(GOD, mech, "");*/
	}
    }
    HandleOverheat(mech);
}

int recycle_weaponry(MECH * mech)
{

    int loop;
    int count, i;
    int crit[MAX_WEAPS_SECTION];
    unsigned char weaptype[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    char location[20];

    int diff = (event_tick - MechLWRT(mech)) / WEAPON_TICK;
    int lowest = 0;

    if (diff < 1) {
	if (diff < 0)
	    MechLWRT(mech) = event_tick;
	return 1;
    }
    MechLWRT(mech) = event_tick;

    if (!Started(mech) || Destroyed(mech))
	return 0;

    arc_override = 1;
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	count = FindWeapons(mech, loop, weaptype, weapdata, crit);
	for (i = 0; i < count; i++) {
	    if (WpnIsRecycling(mech, loop, crit[i])) {
		if (diff >= GetPartData(mech, loop, crit[i])) {
		    GetPartData(mech, loop, crit[i]) = 0;
		    mech_notify(mech, MECHSTARTED, tprintf(MechType(mech) == CLASS_MW ? "%%cgYou are ready to attack again with %s.%%c" :
			    "%%cg%s finished recycling.%%c", &MechWeapons[weaptype[i]].name[3]));
		} else {
		    GetPartData(mech, loop, crit[i]) -= diff;
		    if (GetPartData(mech, loop, crit[i]) < lowest || !lowest)
			lowest = GetPartData(mech, loop, crit[i]);
		}
	    }
	}
	if (MechSections(mech)[loop].recycle && (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_BSUIT ||
	    (MechType(mech) == CLASS_VEH_GROUND) || (MechType(mech) == CLASS_VTOL))) {
	    if (diff >= MechSections(mech)[loop].recycle && !SectIsDestroyed(mech, loop)) {
		MechSections(mech)[loop].recycle = 0;
		ArmorStringFromIndex(loop, location, MechType(mech), MechMove(mech));
		mech_notify(mech, MECHSTARTED, tprintf("%%cg%s%s has finished its attack.%%c",
		    MechType(mech) == CLASS_BSUIT ? "" : "Your ", location));
	    } else {
		MechSections(mech)[loop].recycle -= diff;
		if (MechSections(mech)[loop].recycle < lowest || !lowest)
		    lowest = MechSections(mech)[loop].recycle;
	    }
	}
    }
    arc_override = 0;
    return lowest;
}

int SkidMod(float Speed)
{
    if (Speed < 2.1)
	return -2;
    if (Speed < 4.1)
	return -1;
    if (Speed < 7.1)
	return 0;
    if (Speed < 10.1)
	return 1;
    return 2;
}

void NewHexEntered(MECH * mech, MAP * mech_map, float deltax, float deltay)
{
    int elevation, lastelevation;
    int oldterrain;
    int ot, le, ed, done = 0, tt, i;
    int isunder = 0;
    float f;
    MECH * tempmech;

#define MOVE_BACK \
	      MechFX(mech) -= deltax;\
	      MechFY(mech) -= deltay;\
	      MechX(mech) = MechLastX(mech);\
	      MechY(mech) = MechLastY(mech);\
	      MechZ(mech) = lastelevation;\
	      MechFZ(mech) = MechZ(mech) * ZSCALE;\
	      MechTerrain(mech) = ot;\
	      MechElev(mech) = lastelevation;

    le = lastelevation = Elevation(mech_map, MechLastX(mech), MechLastY(mech));
    elevation = MechElevation(mech);
    ot = oldterrain = GetTerrain(mech_map, MechLastX(mech), MechLastY(mech));
    if (ot == ICE && MechZ(mech) >= 0)
		le = lastelevation = 0;
    if (MechZ(mech) < le)
		le = MechZ(mech);

	if (Jumping(mech)) {
	    if (MechRTerrain(mech) == WATER)
			return;
	    if (collision_check(mech, JUMP, 0, 0)) {
			ed = MAX(1, 1 + MechZ(mech) - Elevation(mech_map, MechX(mech), MechY(mech)));
			MOVE_BACK;
			mech_notify(mech, MECHALL, "%chYou attempt to jump over elevation that is too high!!%c");
			if (RGotPilot(mech) && MadePilotSkillRoll(mech, (int) (MechFZ(mech)) / ZSCALE / 3)) {
		    	mech_notify(mech, MECHALL, "%chYou land safely.%c");
			    LandMech(mech, 1);
			} else {
			    mech_notify(mech, MECHALL, "%chYou crash into the obstacle and fall from the sky!!!!!%c");
			    MechLOSBroadcast(mech, "crashes into an obstacle and falls from the sky!");
			    MechFalls(mech, ed + 1000, 0);
			    domino_space(mech, 2);
			}
		}
	    return;
	} else {
	    switch (MechMove(mech)) {
		    case MOVE_BIPED:
		    case MOVE_QUAD:
			    if (MechType(mech) == CLASS_MW) {
				if (oldterrain == WATER || oldterrain == ICE)
				    lastelevation = 0;
				if (MechRTerrain(mech) == WATER || MechRTerrain(mech) == ICE)
				    elevation = 0; 
				}
			    if (!Jumping(mech)) {
			        if ((MechRTerrain(mech) == WATER)
			         && ((Fallen(mech) && (MechZ(mech) <= -1)) || (MechZ(mech) <= -2))) {
						if (MechStatus(mech) & JELLIED) {
			                MechStatus(mech) &= ~JELLIED;
			                mech_notify(mech, MECHALL, "The water washes away the burning jelly!");
			                add_decoration(mech_map, MechX(mech), MechY(mech), TYPE_FIRE, FIRE, 90);
						}
			            for (i = 0 ; i < NUM_SECTIONS ; i++)
							AcidClear(mech, i, 1);
					} else if (MechRTerrain(mech) == WATER && MechZ(mech) == -1) {
						AcidClear(mech, LLEG, 1);
						AcidClear(mech, RLEG, 1);
						if (MechMove(mech) == MOVE_QUAD) {
							AcidClear(mech, LARM, 1);
							AcidClear(mech, RARM, 1);
						}
					}
			    }
				if (collision_check(mech, WALK_WALL, lastelevation, oldterrain)) {
				    MOVE_BACK;
				    mech_notify(mech, MECHALL, "You attempt to climb a hill too steep for you.");
	    			if (MechPilot(mech) == -1
	    			 || (!mudconf.btech_skidcliff
	    			  && MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
					 || (mudconf.btech_skidcliff
					  && MadePilotSkillRoll(mech, SkidMod(fabs(MechSpeed(mech)) / MP1)))) {
						mech_notify(mech, MECHALL, "You manage to stop before crashing.");
	    			} else {
						mech_notify(mech, MECHALL, "You run headlong into the cliff and fall down!!");
						MechLOSBroadcast(mech, "runs headlong into a cliff and falls down!");
						if (!mudconf.btech_skidcliff)
						    MechFalls(mech, (int) (((1 + (MechSpeed(mech)) * MP_PER_KPH) / 4) + 1000), 0);
						else
						    MechFalls(mech, 1001, 0);
	    			}
	    			MechDesiredSpeed(mech) = 0;
	    			MechSpeed(mech) = 0;
	    			MechZ(mech) = lastelevation;
	    			return;
				} else if (collision_check(mech, WALK_DROP, lastelevation, oldterrain)) {
	    			mech_notify(mech, MECHALL, "You notice a large drop in front of you");
	    			if (MechPilot(mech) == -1
	    			 || (!mudconf.btech_skidcliff
	    			  && MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
					 || (mudconf.btech_skidcliff
					  && MadePilotSkillRoll(mech, SkidMod(fabs(MechSpeed(mech)) / MP1)))) {
						mech_notify(mech, MECHALL, "You manage to stop before falling off.");
						MOVE_BACK;
	    			} else {
						mech_notify(mech, MECHALL, "You run off the cliff and fall to the ground below.");
						MechLOSBroadcast(mech, "runs off a cliff and falls to the ground below!");
						MechFalls(mech, ((lastelevation - elevation) + 1000), 0);
						MechDesiredSpeed(mech) = 0;
						MechSpeed(mech) = 0;
	    			}
	    			MechDesiredSpeed(mech) = 0;
	    			MechSpeed(mech) = 0;
	    			return;
				} else if ((MechSpeed(mech) < 0) && (collision_check(mech, WALK_BACK, lastelevation, oldterrain))) {
				    mech_notify(mech, MECHALL, tprintf("You notice a %s behind of you!",
				     (elevation > lastelevation ? "small incline" : "small drop")));
            		if (MechPilot(mech) == -1
            		 || (MadePilotSkillRoll(mech, collision_check(mech, WALK_BACK, lastelevation, oldterrain) - 1))) {
                		mech_notify(mech, MECHALL, "You manage to overcome the obstacle.");
            		} else {
						mech_notify(mech, MECHALL, tprintf("%s",(elevation > lastelevation ?
						 "You stumble on your rear and fall down." : "You fall on your rear off the small incline.")));
                		MechLOSBroadcast(mech, tprintf("%s",(elevation > lastelevation ?
                		 "falls on it's back walking up an incline." : "falls off the back of a small incline.")));
                		MechFalls(mech, (abs(lastelevation - elevation) + 1000), 1);
                		MechDesiredSpeed(mech) = 0;
                		MechSpeed(mech) = 0;
						if (elevation > lastelevation) {
							MOVE_BACK;
						    }
					}
        			return;
				}
				le = elevation - lastelevation;
				le = (le < 0) ? -le : le;
				if (MechZ(mech) != elevation)
				    le = 0;
				if (le > 0) {
				    deltax = (le == 1) ? MP1 : MP2;
				    if (MechSpeed(mech) > 0) {
						MechSpeed(mech) -= deltax;
						if (MechSpeed(mech) < 0)
						    MechSpeed(mech) = 0;
					} else if (MechSpeed(mech) < 0) {
						MechSpeed(mech) += deltax;
						if (MechSpeed(mech) > 0)
						    MechSpeed(mech) = 0;
					}
				}
				if (!Jumping(mech) && !Fallen(mech) && Started(mech) && possibly_domino_space(mech, 0))	{
                    if (MechChargeTarget(mech) != -1) {
                        
                        if (!charge_domino_space(mech)) {

					        if (MadePilotSkillRoll(mech, (possibly_domino_space(mech, 0) - 1) / 2))	{
						        mech_notify(mech, MECHALL, "You manage to stop before plowing headlong into a mess!");
						        MechLOSBroadcast(mech, "comes to a sudden halt!");
						        MechDesiredSpeed(mech) = 0;
           					    MechSpeed(mech) = 0;
						        MOVE_BACK;
						        return;
					        }
					        mech_notify(mech, MECHALL, "You continue on your way into the sea of metal...");
                        }
                    } else {
					    if (MadePilotSkillRoll(mech, (possibly_domino_space(mech, 0) - 1) / 2))	{
						    mech_notify(mech, MECHALL, "You manage to stop before plowing headlong into a mess!");
						    MechLOSBroadcast(mech, "comes to a sudden halt!");
						    MechDesiredSpeed(mech) = 0;
           				    MechSpeed(mech) = 0;
						    MOVE_BACK;
						    return;
                        } 
					    mech_notify(mech, MECHALL, "You continue on your way into the sea of metal...");
                    }
				}
				if (((MechRTerrain(mech) == WATER && MechZ(mech) < 0)
				 ||	((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) && MechZ(mech) < 0)
				 ||	(MechRTerrain(mech) == ICE && MechZ(mech) < 0) || (MechRTerrain(mech) == HIGHWATER))
				 && (MechType(mech) != CLASS_MW) && (MechType(mech) != CLASS_BSUIT)) {
	    			mech_notify(mech, MECHPILOT, "You use your piloting skill to maneuver through the water.");
					if (!MadePilotSkillRoll(mech,
					 ((((MechSpeed(mech) > WalkingSpeed(MMaxSpeed(mech)) ||
					  MechDesiredSpeed(mech) > WalkingSpeed(MMaxSpeed(mech)))
					  && oldterrain != WATER &&	oldterrain != HIGHWATER && oldterrain != ICE && lastelevation >= 0) ? 3 : 0)
					 + (MechRTerrain(mech) == HIGHWATER
					  ? -2 : (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE)
					   ? bridge_w_elevation(mech) : MechElev(mech) > 3
					    ? 1 : (MechElev(mech) - 2))))) {
						mech_notify(mech, MECHALL, "You slip in the water and fall down");
						MechLOSBroadcast(mech, "slips in the water and falls down!");
						MechFalls(mech, 1, 0);
						done = 1;
	    			}
	    			if (MechCritStatus(mech) & LIFE_SUPPORT_DESTROYED && (MechZ(mech) <= 2 || (Fallen(mech) && MechZ(mech) <= 1)))
	    			 	CheckNoAir(mech);

					if (MechStatus2(mech) & SPRINTING) {
						MechStatus2(mech) &= ~SPRINTING;
						MechLOSBroadcast(mech, "breaks out of its sprint as it enters water!");
						mech_notify(mech, MECHALL, "You lose your sprinting momentum as you enter water!");
						MECHEVENT(mech, EVENT_MOVEMODE, mech_movemode_event, TURN, MODE_OFF|MODE_SPRINT|MODE_MODEFAIL);
					}
					if (MechType(mech) == CLASS_BSUIT
					 && ((MechRTerrain(mech) == WATER && elevation < 0 )
					  || ((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE)
					   && (lastelevation < (elevation - 1))))
					 && MechZ(mech) < 0) {
						mech_notify(mech, MECHALL, "You notice a body of water in front of you");
						if (MechPilot(mech) == -1
						 || MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3)) {
							mech_notify(mech, MECHALL, "You manage to stop before falling in.");
							MOVE_BACK;
							MechDesiredSpeed(mech) = 0;
							MechSpeed(mech) = 0;
							return;
						}
					}
					if (!Fallen(mech) && !Jumping(mech) && lastelevation == -2
					 && elevation == -1 && MechTerrain(mech) == ICE) {
						mech_notify(mech, MECHALL, "You break the ice as you emerge from the deep!");
						MechLOSBroadcast(mech, "breaks through the ice as it emerges from deeper waters!");
						SetTerrain(mech_map, MechX(mech), MechY(mech), WATER);
					}
				}
				if (MechRTerrain(mech) == DBRIDGE && MechType(mech) != CLASS_MW && MechSpeed(mech) > MP1) {
		            mech_notify(mech, MECHALL, "You plow your way through the jumbled mass of metal!");
		            if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech,
		             (MechMove(mech) == MOVE_QUAD ? 1 : 2)
		             + (IsRunning(MechSpeed(mech), MMaxSpeed(mech)) ? 1 : 0))) {
		            	mech_notify(mech, MECHALL, "You manage to nimbly dance through the mess!");
		            } else {
		                mech_notify(mech, MECHALL, "You bounce around, but lose precious speed!");
		                MechLOSBroadcast(mech, "swivels around the mass of metal!");
		                MechSpeed(mech) = 0.0;
					}
				}
				break;
    		case MOVE_TRACK:
				if (collision_check(mech, WALK_WALL, lastelevation, oldterrain)) {
				    mech_notify(mech, MECHALL, "You attempt to climb a hill too steep for you.");
	    			if (MechPilot(mech) == -1
	    			 || (!mudconf.btech_skidcliff && MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
					 || (mudconf.btech_skidcliff && MadePilotSkillRoll(mech, SkidMod(fabs(MechSpeed(mech)) / MP1))))
						mech_notify(mech, MECHALL, "You manage to stop before crashing.");
	    			else {
						if (!mudconf.btech_skidcliff) {
						    mech_notify(mech, MECHALL, "You smash into a cliff!!");
						    MechLOSBroadcast(mech, "crashes to a cliff!");
						    MechFalls(mech, (int) (MechSpeed(mech) * MP_PER_KPH / 4), 0);
						} else {
						    mech_notify(mech, MECHALL, "You skid to a violent halt!!");
		    				MechLOSBroadcast(mech, "goes into a skid!");
		    				MechFalls(mech, 0, 0);
						}
	    			}
	    			MOVE_BACK;
	    			MechDesiredSpeed(mech) = 0;
	    			MechSpeed(mech) = 0;
	    			return;
				} else if (collision_check(mech, WALK_DROP, lastelevation, oldterrain)) {
	    			mech_notify(mech, MECHALL, "You notice a large drop in front of you");
	    			if (MechPilot(mech) == -1
	    			 || (!mudconf.btech_skidcliff && MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
					 || (mudconf.btech_skidcliff && MadePilotSkillRoll(mech, SkidMod(fabs(MechSpeed(mech)) / MP1))))
						mech_notify(mech, MECHALL, "You manage to stop before falling off.");
	    			else {
						mech_notify(mech, MECHALL, "You drive off the cliff and fall to the ground below.");
						MechLOSBroadcast(mech, "drives off a cliff and falls to the ground below.");
						MechFalls(mech, lastelevation - elevation, 0);
						domino_space(mech, 2);
						return;
	    			}
	    			MOVE_BACK;
	    			MechDesiredSpeed(mech) = 0;
	    			MechSpeed(mech) = 0;
	    			return;
				} else if ((MechSpeed(mech) < 0) && (collision_check(mech, WALK_BACK, lastelevation, oldterrain))) {
            		mech_notify(mech, MECHALL, tprintf("You notice a %s behind of you!", (elevation > lastelevation ? "small incline" : "small drop")));
		            if (MechPilot(mech) == -1 || (MadePilotSkillRoll(mech, collision_check(mech, WALK_BACK, lastelevation, oldterrain) - 1))) {
		                mech_notify(mech, MECHALL, "You manage to overcome the obstacle.");
            		} else {
            		    mech_notify(mech, MECHALL, tprintf("%s",(elevation > lastelevation
            		     ? "You stumble on your rear and fall down." : "You fall on your rear off the small incline.")));
                		MechLOSBroadcast(mech, tprintf("%s",(elevation > lastelevation
                		 ? "falls on it's back walking up an incline." : "falls off the back of a small incline.")));
                		MechFalls(mech, (abs(lastelevation - elevation) + 1000), 1);
                		MechDesiredSpeed(mech) = 0;
                		MechSpeed(mech) = 0;
                		if (elevation > lastelevation) {
                        	    MOVE_BACK;
				    }
		            }
			        return;
		        }
				if (!(MechSpecials2(mech) & WATERPROOF_TECH) && 
				    ((MechRTerrain(mech) == WATER && elevation < 0 ) ||
				    ((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) && (lastelevation < (elevation - 1))))
				    && MechZ(mech) < 0) {
	    			mech_notify(mech, MECHALL, "You notice a body of water in front of you");
	    			if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
						mech_notify(mech, MECHALL, "You manage to stop before falling in.");
	    			else {
						mech_notify(mech, MECHALL, "You drive into the water and your vehicle becomes inoperable.");
						DestroyMech(mech, mech, 0);
						return;
	    			}
	    			MOVE_BACK;
	    			MechDesiredSpeed(mech) = 0;
	    			MechSpeed(mech) = 0;
	    			return;
				}
	/* New terrain restrictions */
	if (mudconf.btech_newterrain) {
	    tt = MechRTerrain(mech);
	    if ((tt == HEAVY_FOREST) && fabs(MechSpeed(mech)) > MP1 && MechZ(mech) <= GetElev(mech_map, MechX(mech), MechY(mech))) {
#if 0
		mech_notify(mech, MECHALL,
		    "You cruise at a bunch of trees!");
#endif
			mech_notify(mech, MECHALL, "You try to dodge the larger trees..");
			if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, (int) (fabs(MechSpeed(mech)) / MP1 / 6)))
			    mech_notify(mech, MECHALL, "You manage to dodge 'em!");
			else {
			    mech_notify(mech, MECHALL, "You swerve, but not enough! This'll hurt!");
			    MechLOSBroadcast(mech, "cruises headlong at a tree!");
			    f = fabs(MechSpeed(mech));
			    MechSpeed(mech) = MechSpeed(mech) / 2.0;
			    MechFalls(mech, MAX(1, (int) sqrt(f / MP1 / 2)), 0);
			}
	    }
	}
	if (MechRTerrain(mech) == DBRIDGE && MechType(mech) != CLASS_MW && MechSpeed(mech) > MP1) {
        mech_notify(mech, MECHALL, "You plow your way through the jumbled mass of metal!");
        if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, 1 + (IsRunning(MechSpeed(mech), MMaxSpeed(mech)) ? 1 : 0))) {
            mech_notify(mech, MECHALL, "You barely manage to swerve through the mess!");
        } else {
            mech_notify(mech, MECHALL, "You swerve around and lose precious speed!");
            MechLOSBroadcast(mech, "swerves around inside the mass of metal!");
            MechSpeed(mech) = 0.0;
		}
    }
	le = elevation - lastelevation;
	le = (le < 0) ? -le : le;
	if (le > 0) {
	    deltax = (le == 1) ? MP1 : MP3;
	    if (MechSpeed(mech) > 0) {
			MechSpeed(mech) -= deltax;
			if (MechSpeed(mech) < 0)
			    MechSpeed(mech) = 0;
		} else if (MechSpeed(mech) < 0) {
			MechSpeed(mech) += deltax;
			if (MechSpeed(mech) > 0)
			    MechSpeed(mech) = 0;
		}
	}
	break;
case MOVE_WHEEL:
	if (collision_check(mech, WALK_WALL, lastelevation, oldterrain)) {
	    mech_notify(mech, MECHALL, "You attempt to climb a hill too steep for you.");
	    if (MechPilot(mech) == -1 || (!mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
			(int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
		|| (mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
SkidMod(fabs(MechSpeed(mech)) / MP1))))
		mech_notify(mech, MECHALL,
		    "You manage to stop before crashing.");
	    else {
		if (!mudconf.btech_skidcliff) {
		    mech_notify(mech, MECHALL, "You smash into a cliff!!");
		    MechLOSBroadcast(mech, "crashes to a cliff!");
		    MechFalls(mech,
			(int) (MechSpeed(mech) * MP_PER_KPH / 4), 0);
		} else {
		    mech_notify(mech, MECHALL,
			"You skid to a violent halt!");
		    MechLOSBroadcast(mech, "skids to a halt!");
		    MechFalls(mech, 0, 0);
		}
	    }
	    MOVE_BACK;
	    MechDesiredSpeed(mech) = 0;
	    MechSpeed(mech) = 0;
	    return;
	} else if (collision_check(mech, WALK_DROP, lastelevation,
		oldterrain)) {
	    mech_notify(mech, MECHALL,
		"You notice a large drop in front of you");
	    if (MechPilot(mech) == -1 || (!mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
			(int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
		|| (mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
SkidMod(fabs(MechSpeed(mech)) / MP1))))
		mech_notify(mech, MECHALL,
		    "You manage to stop before falling off.");
	    else {
		mech_notify(mech, MECHALL,
		    "You drive off the cliff and fall to the ground below.");
		MechLOSBroadcast(mech,
		    "drives off a cliff and falls to the ground below.");
		MechFalls(mech, lastelevation - elevation, 0);
		domino_space(mech, 2);
		return;
	    }
	    MOVE_BACK;
	    MechDesiredSpeed(mech) = 0;
	    MechSpeed(mech) = 0;
	    return;
	}  else if ((MechSpeed(mech) < 0) && (collision_check(mech, WALK_BACK, lastelevation, oldterrain))) {
            mech_notify(mech, MECHALL, tprintf("You notice a %s behind of you!",
                (elevation > lastelevation ? "small incline" : "small drop")));
            if (MechPilot(mech) == -1 || (MadePilotSkillRoll(mech,
                collision_check(mech, WALK_BACK, lastelevation, oldterrain) - 1))) {
                mech_notify(mech, MECHALL,
                    "You manage to overcome the obstacle.");
            } else {
                mech_notify(mech, MECHALL,
                    tprintf("%s",(elevation > lastelevation ? "You stumble on your rear and fall down." :
                        "You fall on your rear off the small incline.")));
                MechLOSBroadcast(mech,
                    tprintf("%s",(elevation > lastelevation ? "falls on it's back walking up an incline." :
                        "falls off the back of a small incline.")));
                MechFalls(mech, (abs(lastelevation - elevation) + 1000), 1);
                MechDesiredSpeed(mech) = 0;
                MechSpeed(mech) = 0;
                if (elevation > lastelevation)
                        {
                        MOVE_BACK;
                        }
            }
        return;
        }
	if (!(MechSpecials2(mech) & WATERPROOF_TECH) &&
		((MechRTerrain(mech) == WATER && elevation < 0 ) ||
		((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) && (lastelevation < (elevation - 1))))
		&& MechZ(mech) < 0) {
	    mech_notify(mech, MECHALL,
		"You notice a body of water in front of you");
	    if (MechPilot(mech) == -1 ||
		MadePilotSkillRoll(mech,
(int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
		mech_notify(mech, MECHALL,
		    "You manage to stop before falling in.");
	    else {
		mech_notify(mech, MECHALL,
		    "You drive into the water and your vehicle becomes inoperable.");
		DestroyMech(mech, mech, 0);
		return;
	    }
	    MOVE_BACK;
	    MechDesiredSpeed(mech) = 0;
	    MechSpeed(mech) = 0;
	    return;
	}
	/* New terrain restrictions */
	if (mudconf.btech_newterrain) {
	    tt = MechRTerrain(mech);
	    if ((tt == HEAVY_FOREST || tt == LIGHT_FOREST) &&
		fabs(MechSpeed(mech)) > MP1 && MechZ(mech) <= GetElev(mech_map, MechX(mech), MechY(mech))) {
#if 0
		mech_notify(mech, MECHALL,
		    "You cruise at a bunch of trees!");
#endif
		mech_notify(mech, MECHALL,
		    "You try to dodge the larger trees..");
		if (MechPilot(mech) == -1 ||
		    MadePilotSkillRoll(mech,
(tt == HEAVY_FOREST ? 3 : 0) + (fabs(MechSpeed(mech)) / MP1 / 6)))
		    mech_notify(mech, MECHALL, "You manage to dodge 'em!");
		else {
		    mech_notify(mech, MECHALL,
			"You swerve, but not enough! This'll hurt!");
		    MechLOSBroadcast(mech, "cruises headlong at a tree!");
		    f = fabs(MechSpeed(mech));
		    MechSpeed(mech) = MechSpeed(mech) / 2.0;
		    MechFalls(mech, MAX(1, (int) sqrt(f / MP1 / 2)), 0);
		}
	    } else if ((tt == ROUGH) && fabs(MechSpeed(mech)) > MP1) {
#if 0
		mech_notify(mech, MECHALL,
		    "You cruise at some rough terrain!");
#endif
		mech_notify(mech, MECHALL, "You try to avoid the rocks..");
		if (MechPilot(mech) == -1 ||
		    MadePilotSkillRoll(mech,
(int) (fabs(MechSpeed(mech)) / MP1 / 6)))
		    mech_notify(mech, MECHALL, "You manage to dodge 'em!");
		else {
		    mech_notify(mech, MECHALL,
			"You swerve, but not enough! This'll hurt!");
		    MechLOSBroadcast(mech, "cruises headlong at a rock!");
		    f = fabs(MechSpeed(mech));
		    MechSpeed(mech) = MechSpeed(mech) / 2.0;
		    MechFalls(mech, MAX(1, (int) sqrt(f / MP1 / 2)), 0);
		}
	    }
	}
        if (MechRTerrain(mech) == DBRIDGE && MechType(mech) != CLASS_MW && MechSpeed(mech) > MP1) {
                mech_notify(mech, MECHALL, "You plow your way through the jumbled mass of metal!");
                if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, 3 + (IsRunning(MechSpeed(mech), MMaxSpeed(mech)) ? 1 : 0))) {
                    mech_notify(mech, MECHALL, "You barely manage to swerve through the mess!");
                } else {
                    mech_notify(mech, MECHALL, "You swerve around and lose precious speed!");
                    MechLOSBroadcast(mech, "swerves around inside the mass of metal!");
                    MechSpeed(mech) = 0.0;
		}
        }
	le = elevation - lastelevation;
	le = (le < 0) ? -le : le;
	if (le > 0) {
	    deltax = (le == 1) ? MP1 : MP4;
	    if (MechSpeed(mech) > 0) {
		MechSpeed(mech) -= deltax;
		if (MechSpeed(mech) < 0)
		    MechSpeed(mech) = 0;
	    } else if (MechSpeed(mech) < 0) {
		MechSpeed(mech) += deltax;
		if (MechSpeed(mech) > 0)
		    MechSpeed(mech) = 0;
	    }
	}
	break;
    case MOVE_HULL:
    case MOVE_FOIL:
    case MOVE_SUB:
	if ((MechRTerrain(mech) != WATER && (MechRTerrain(mech) != BRIDGE && MechRTerrain(mech) != DBRIDGE && MechRTerrain(mech) != ICE))
	    || MechElev(mech) <= (abs(MechZ(mech)) + (MechMove(mech) ==
		    MOVE_FOIL ? -1 : 0))) {
	    /* Run aground */
	    MechElev(mech) = le;
	    MechTerrain(mech) = ot;
	    mech_notify(mech, MECHALL,
		"You attempt to get too close with ground!");
	    if (MechPilot(mech) == -1 ||
		MadePilotSkillRoll(mech,
(int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3)) {
		mech_notify(mech, MECHALL,
		    "You manage to stop before crashing.");
		MOVE_BACK;
	    } else {
		mech_notify(mech, MECHALL, "You smash into the ground!!");
		MechLOSBroadcast(mech, "smashes aground!");
		MechFalls(mech, (int) (MechSpeed(mech) * MP_PER_KPH / 4), 0);
	    }
	    MechSpeed(mech) = 0;
	    MechDesiredSpeed(mech) = 0;
	    MechVerticalSpeed(mech) = 0;
	    return;
	}
	if (MechRTerrain(mech) == ICE && MechZ(mech) == 0) {
	  mech_BreakIce(mech, mech_map);
	  deltax = MP2;
	    if (MechSpeed(mech) > 0) {
	      MechSpeed(mech) -= deltax;
	      if (MechSpeed(mech) < 0)
	      MechSpeed(mech) = 0;
	    } else if (MechSpeed(mech) < 0) {
	      MechSpeed(mech) += deltax;
	      if (MechSpeed(mech) > 0)
		MechSpeed(mech) = 0;
	    }
	}
	if (elevation > 0)
	    elevation = 0;
	break;
    case MOVE_HOVER:
	if (oldterrain == WATER || oldterrain == ICE)
	    lastelevation = 0;
	if (MechRTerrain(mech) == WATER || MechRTerrain(mech) == ICE)
	    elevation = 0; 
	isunder = ((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) && 
		       ((oldterrain == WATER || oldterrain == ICE) || ((oldterrain == BRIDGE || oldterrain == DBRIDGE) && lastelevation == 0)));

	if (collision_check(mech, WALK_WALL, lastelevation, oldterrain) && !isunder) {
	    MechElev(mech) = le;
	    MechTerrain(mech) = ot;
	    mech_notify(mech, MECHALL,
		"You attempt to climb a hill too steep for you.");
	    if (MechPilot(mech) == -1 || (!mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
			(int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
		|| (mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
SkidMod(fabs(MechSpeed(mech)) / MP1))))
		mech_notify(mech, MECHALL,
		    "You manage to stop before crashing.");
	    else {
		if (!mudconf.btech_skidcliff) {
		    mech_notify(mech, MECHALL, "You smash into a cliff!!");
		    MechLOSBroadcast(mech, "smashes into a cliff!");
		    MechFalls(mech, (int) (MechSpeed(mech) * MP_PER_KPH / 4), 0);
		} else {
		    mech_notify(mech, MECHALL,
			"You skid to a violent halt!");
		    MechLOSBroadcast(mech, "Skids to a halt!");
		    MechFalls(mech, 0, 0);
		}
	    }
	    MOVE_BACK;
	    MechDesiredSpeed(mech) = 0;
	    MechSpeed(mech) = 0;
	    return;
	} else if (collision_check(mech, WALK_DROP, lastelevation,
		oldterrain) && !isunder) {
	    mech_notify(mech, MECHALL,
		"You notice a large drop in front of you");
	    if (MechPilot(mech) == -1 || (!mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
			(int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
		|| (mudconf.btech_skidcliff &&
		    MadePilotSkillRoll(mech,
SkidMod(fabs(MechSpeed(mech)) / MP1))))
		mech_notify(mech, MECHALL,
		    "You manage to stop before falling off.");
	    else {
		mech_notify(mech, MECHALL,
		    "You drive off the cliff and fall to the ground below.");
		MechLOSBroadcast(mech,
		    "drives off a cliff and falls to the ground below.");
		MechFalls(mech, lastelevation - elevation, 0);
		domino_space(mech, 2);
		return;
	    }
	    MOVE_BACK;
	    MechDesiredSpeed(mech) = 0;
	    MechSpeed(mech) = 0;
	    return;
	}  else if ((MechSpeed(mech) < 0) && (collision_check(mech, WALK_BACK, lastelevation, oldterrain)) && !isunder) {
            mech_notify(mech, MECHALL, tprintf("You notice a %s behind of you!",
                (elevation > lastelevation ? "small incline" : "small drop")));
            if (MechPilot(mech) == -1 || (MadePilotSkillRoll(mech,
                collision_check(mech, WALK_BACK, lastelevation, oldterrain) - 1))) {
                mech_notify(mech, MECHALL,
                    "You manage to overcome the obstacle.");
            } else {
                mech_notify(mech, MECHALL,
                    tprintf("%s",(elevation > lastelevation ? "You stumble on your rear and fall down." :
                        "You fall on your rear off the small incline.")));
                MechLOSBroadcast(mech,
                    tprintf("%s",(elevation > lastelevation ? "falls on it's back walking up an incline." :
                        "falls off the back of a small incline.")));
                MechFalls(mech, (abs(lastelevation - elevation) + 1000), 1);
                MechDesiredSpeed(mech) = 0;
                MechSpeed(mech) = 0;
                if (elevation > lastelevation) {
                        MOVE_BACK;
                        }
            }
        return;
        }
	tt = MechRTerrain(mech);
	if ((tt == HEAVY_FOREST || tt == LIGHT_FOREST) &&
	    fabs(MechSpeed(mech)) > MP1 && MechZ(mech) <= GetElev(mech_map, MechX(mech), MechY(mech))) {
#if 0
	    mech_notify(mech, MECHALL, "You cruise at a bunch of trees!");
#endif
	    mech_notify(mech, MECHALL,
		"You try to dodge the larger trees..");
	    if (MechPilot(mech) == -1 ||
		MadePilotSkillRoll(mech,
(tt == HEAVY_FOREST ? 3 : 0) + (fabs(MechSpeed(mech)) / MP1 / 6)))
		mech_notify(mech, MECHALL, "You manage to dodge 'em!");
	    else {
		mech_notify(mech, MECHALL,
		    "You swerve, but not enough! This'll hurt!");
		MechLOSBroadcast(mech, "cruises headlong at a tree!");
		f = fabs(MechSpeed(mech));
		MechSpeed(mech) = MechSpeed(mech) / 2.0;
		MechFalls(mech, MAX(1, (int) sqrt(f / MP1 / 2)), 0);
	    }
	}
        if (MechRTerrain(mech) == DBRIDGE && MechType(mech) != CLASS_MW && MechSpeed(mech) > MP1) {
                mech_notify(mech, MECHALL, "You plow your way through the jumbled mass of metal!");
                if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, 4 + (IsRunning(MechSpeed(mech), MMaxSpeed(mech)) ? 1 : 0))) {
                    mech_notify(mech, MECHALL, "You barely manage to swerve through the mess!");
                } else {
                    mech_notify(mech, MECHALL, "You swerve around and lose precious speed!");
                    MechLOSBroadcast(mech, "swerves around inside the mass of metal!");
                    MechSpeed(mech) = 0.0;
		}
        }
	le = elevation - lastelevation;
	le = (le < 0) ? -le : le;
	if (le > 0) {
	    deltax = (le == 1) ? MP1 : MP5;
	    if (MechSpeed(mech) > 0) {
		MechSpeed(mech) -= deltax;
		if (MechSpeed(mech) < 0)
		    MechSpeed(mech) = 0;
	    } else if (MechSpeed(mech) < 0) {
		MechSpeed(mech) += deltax;
		if (MechSpeed(mech) > 0)
		    MechSpeed(mech) = 0;
	    }
	}
	break;
    case MOVE_VTOL:
    case MOVE_FLY:
	if (Landed(mech) && collision_check(mech, WALK_WALL, lastelevation, oldterrain)) {
            mech_notify(mech, MECHALL, "You attempt to climb a hill too steep for you.");
            if (MechPilot(mech) == -1 || (!mudconf.btech_skidcliff && MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
                || (mudconf.btech_skidcliff && MadePilotSkillRoll(mech, SkidMod(fabs(MechSpeed(mech)) / MP1))))
                mech_notify(mech, MECHALL, "You manage to stop before crashing.");
            else {
                if (!mudconf.btech_skidcliff) {
                    mech_notify(mech, MECHALL, "You smash into a cliff!!");
                    MechLOSBroadcast(mech, "crashes to a cliff!");
                    MechFalls(mech, (int) (MechSpeed(mech) * MP_PER_KPH / 4), 0);
                } else {
                    mech_notify(mech, MECHALL, "You skid to a violent halt!");
                    MechLOSBroadcast(mech, "skids to a halt!");
                    MechFalls(mech, 0, 0);
                }
            }
            MOVE_BACK;
            MechDesiredSpeed(mech) = 0;
            MechSpeed(mech) = 0;
            return;
        } else if (Landed(mech) && collision_check(mech, WALK_DROP, lastelevation, oldterrain)) {
            mech_notify(mech, MECHALL, "You notice a large drop in front of you");
            if (MechPilot(mech) == -1 || (!mudconf.btech_skidcliff && MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
                || (mudconf.btech_skidcliff && MadePilotSkillRoll(mech, SkidMod(fabs(MechSpeed(mech)) / MP1))))
                mech_notify(mech, MECHALL, "You manage to stop before falling off.");
            else {
                mech_notify(mech, MECHALL, "You drive off the cliff and fall to the ground below.");
                MechLOSBroadcast(mech, "drives off a cliff and falls to the ground below.");
                MechFalls(mech, lastelevation - elevation, 0);
                domino_space(mech, 2);
                return;
            }
            MOVE_BACK;
            MechDesiredSpeed(mech) = 0;
            MechSpeed(mech) = 0;
            return;
        }  else if (Landed(mech) && (MechSpeed(mech) < 0) && (collision_check(mech, WALK_BACK, lastelevation, oldterrain))) {
            mech_notify(mech, MECHALL, tprintf("You notice a %s behind of you!", (elevation > lastelevation ? "small incline" : "small drop")));
            if (MechPilot(mech) == -1 || (MadePilotSkillRoll(mech, collision_check(mech, WALK_BACK, lastelevation, oldterrain) - 1))) {
                mech_notify(mech, MECHALL, "You manage to overcome the obstacle.");
            } else { 
                mech_notify(mech, MECHALL, tprintf("%s",(elevation > lastelevation ? "You stumble on your rear and fall down." : "You fall on your rear off the small incline.")));
                MechLOSBroadcast(mech, tprintf("%s",(elevation > lastelevation ? "falls on it's back walking up an incline." : "falls off the back of a small incline.")));
                MechFalls(mech, (abs(lastelevation - elevation) + 1000), 1);
                MechDesiredSpeed(mech) = 0;
                MechSpeed(mech) = 0;
                if (elevation > lastelevation) {
                        MOVE_BACK;
                        } 
            }
        return;
        }

        if (!(MechSpecials2(mech) & WATERPROOF_TECH) && Landed(mech) && ((MechRTerrain(mech) == WATER && elevation < 0 ) ||
	    ((MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == DBRIDGE) && (lastelevation < (elevation - 1))))
	    && MechZ(mech) < 0) {
            mech_notify(mech, MECHALL, "You notice a body of water in front of you");
            if (MechPilot(mech) == -1 || MadePilotSkillRoll(mech, (int) (fabs((MechSpeed(mech)) + MP1) / MP1) / 3))
                mech_notify(mech, MECHALL, "You manage to stop before falling in.");
            else {
                mech_notify(mech, MECHALL, "You drive into the water and your vehicle becomes inoperable.");
                DestroyMech(mech, mech, 0);
                return;
            }
            MOVE_BACK;
            MechDesiredSpeed(mech) = 0;
            MechSpeed(mech) = 0;
            return;
        } else if (Landed(mech) && MechSpeed(mech) > MP1 && MechRTerrain(mech) != ROAD && MechRTerrain(mech) != BRIDGE && MechRTerrain(mech) != GRASSLAND) {
            mech_notify(mech, MECHALL, "You go where no flying thing has ever gone before..");
            if (!Landing(mech) && RGotPilot(mech) && MadePilotSkillRoll(mech, 5)) {
                mech_notify(mech, MECHALL, "You stop in time!");
                MOVE_BACK;
            } else {
                mech_notify(mech, MECHALL,
                    "Eww.. You've a bad feeling about this.");
                MechLOSBroadcast(mech, "crashes!");
                MechFalls(mech, (Landing(mech) ? 20 : 1), 0);
            }
            MechDesiredSpeed(mech) = 0;
            MechSpeed(mech) = 0;
	    MechAngle(mech) = 0;
	    MechDesiredAngle(mech) = 0;
            return;
        } 

	if (MechRTerrain(mech) == LIGHT_FOREST || MechRTerrain(mech) == HEAVY_FOREST)
	    elevation = MechElevation(mech) + 2;
	else
	    elevation = MechElevation(mech);

	if (collision_check(mech, JUMP, 0, 0)) {
	    MechFX(mech) -= deltax;
	    MechFY(mech) -= deltay;
	    MechX(mech) = MechLastX(mech);
	    MechY(mech) = MechLastY(mech);
	    MechZ(mech) = lastelevation;
	    MechFZ(mech) = MechZ(mech) * ZSCALE;
	    MechElev(mech) = le;
	    MechTerrain(mech) = ot;
	    mech_notify(mech, MECHALL,
		"You attempt to fly over elevation that is too high!!");
	    if (MechPilot(mech) == -1 || (MadePilotSkillRoll(mech, (int) ((is_aero(mech) ? MechVelocity(mech) : MechSpeed(mech)) / MP3) + (ot == GRASSLAND || ot == ROAD || ot == BUILDING ? 2 : 0)))) {
		mech_notify(mech, MECHALL, "You land as safely as you can.");
		MechFalls(mech, 1, 0);
		MechStatus(mech) |= LANDED; 
		if (MechStatus2(mech) & CS_ON_LAND)
		    MechStatus(mech) |= COMBAT_SAFE;
		MechSpeed(mech) = 0.0;
		MechVerticalSpeed(mech) = 0.0;
		MechVelocity(mech) = 0.0;
		MechAngle(mech) = 0;
	    } else {
		mech_notify(mech, MECHALL,
		    "You crash into the obstacle and fall from the sky!!!!!");
		MechLOSBroadcast(mech,
		    "crashes into an obstacle and falls from the sky!");
		MechFalls(mech, MechsElevation(mech) + 1, 0);
		domino_space(mech, 2);
	    }
	}
	break;
    }
    }
    if ((MechRTerrain(mech) == SNOW) && (MechMove(mech) != MOVE_HOVER && MechMove(mech) != MOVE_VTOL && MechMove(mech) != MOVE_FLY &&
		MechType(mech) != CLASS_MW))
            {
                mech_notify(mech, MECHPILOT,
                "You use your piloting skill to maneuver through the snow.");
                        if (!MadePilotSkillRoll(mech, (MechSpeed(mech) / MP5)))
                            {
                                if ((MechMove(mech) == MOVE_BIPED) || (MechMove(mech) == MOVE_QUAD))
                                        {
                                   mech_notify(mech, MECHALL, "You feel the ground underneath you give!");
				   mech_notify(mech, MECHALL, "You are bogged down in the snow!");
                                   MechLOSBroadcast(mech, "stops moving and flails wildly in the snow!");
                                   MechSpeed(mech) = 0.0;
				   MechDesiredSpeed(mech) = 0;
				   MechDesiredFacing(mech) = MechFacing(mech);
				   MechStatus2(mech) |= BOGDOWN;
				   if (MechCarrying(mech) > 0)
					{
					tempmech = getMech(MechCarrying(mech));
					if (tempmech)
					    MechSpeed(tempmech) = 0.0;
					}
                                   done = 1;
				} else {
                                   mech_notify(mech, MECHALL,
                                   "You slide into a snow encrusted ditch!");
				   mech_notify(mech, MECHALL, "You lose all traction!");
                                   MechLOSBroadcast(mech,
                                   "slides into a snow encrusted ditch and flounders in the snow!");
                                   MechSpeed(mech) = 0.0;
				   MechDesiredSpeed(mech) = 0;
				   MechStatus2(mech) |= BOGDOWN;
				   if (MechCarrying(mech) > 0)
                                        {
                                        tempmech = getMech(MechCarrying(mech));
                                        if (tempmech)
                                            MechSpeed(tempmech) = 0.0;
                                        }
                                   done = 1;
				  }
                            }
             }

    if (!done)
	possible_mine_poof(mech, MINE_STEP);
}

void CheckDamage(MECH * wounded)
{
    /* should be called from UpdatePilotSkillRolls */
    /* this is so that a roll will be made only when the mech takes damage */
    /* turndamage is also cleared every 30 secs in UpdatePilotSkillRolls */
    int modifier = 1;

    if (MechTurnDamage(wounded) >= 20 && !IsDS(wounded)) {
	if (!Jumping(wounded) && !Fallen(wounded) && !OODing(wounded)) {
	    mech_notify(wounded, MECHALL, "You stagger from the damage!");
	while (MechTurnDamage(wounded) >= 40) {
	    modifier++;
	    MechTurnDamage(wounded) -= 20;
	    }
	if (MechTons(wounded) >= 80)
	    modifier--;
	else if (MechTons(wounded) <= 35)
	    modifier++;
	if (!MadePilotSkillRoll(wounded, modifier)) {
	    mech_notify(wounded, MECHALL,
	        "You fall over from all the damage!!");
	    MechLOSBroadcast(wounded,
	        "falls down, staggered by the damage!");
	    MechFalls(wounded, 1, 0);
            if (MiscEventing(wounded))
                StopMiscEvent(wounded);
	    }
	}
	MechTurnDamage(wounded) = 0;
    }
}

void UpdatePilotSkillRolls(MECH * mech)
{
    int makeroll = 0, grav = 0;
    float maxspeed;

    if (((event_tick % TURN) == 0) && !Fallen(mech) && !Jumping(mech) &&
	!OODing(mech))
	/* do this once a turn (30 secs), only if mech is standing */
    {
	maxspeed = MMaxSpeed(mech);

	if (!Started(mech))
	    makeroll = 4;
/*
	if ((MechHeat(mech) >= 9.) &&
	    (MechSpecials(mech) & TRIPLE_MYOMER_TECH))
	     maxspeed =
		ceil((rint((MMaxSpeed(mech) / 1.5) / MP1) + 1) * 1.5) * MP1;
	/* maxspeed += 1.5 * MP1; */
/*    if (MechSpecials(mech) & HARDA_TECH)
	maxspeed = (maxspeed <= MP1 ? maxspeed : maxspeed - MP1);*/
	if (InSpecial(mech) && InGravity(mech) && !MoveModeChange(mech))
	    if (MechSpeed(mech) > MMaxSpeed(mech) &&
		MechType(mech) == CLASS_MECH) {
		grav = 1;
		makeroll = 1;
	    }
	if (IsRunning(MechSpeed(mech), maxspeed) &&
	    ((MechCritStatus(mech) & GYRO_DAMAGED) ||
		(MechCritStatus(mech) & HIP_DAMAGED)))
	    makeroll = 1;

	if (MechStatus2(mech) & BOGDOWN)
		{
		if (!MadePilotSkillRoll(mech, 0))
			{
			mech_notify(mech, MECHALL, "You can't manage to break through the bog!");
			} else {
			mech_notify(mech, MECHALL, "You break free of the bog!");
			MechLOSBroadcast(mech, "breaks free of the bog!");
			MechStatus2(mech) &= ~BOGDOWN;
			}

		}
	if (makeroll) {
	    if (!MadePilotSkillRoll(mech, (makeroll - 1))) {
		if (grav) {
		    mech_notify(mech, MECHALL,
			"Your legs take some damage!");
		    if (MechMove(mech) == MOVE_QUAD) {
			if (!SectIsDestroyed(mech, LARM))
			    DamageMech(mech, mech, 0, -1, LARM, 0, 0, 0,
				(MechSpeed(mech) -
				    MechMaxSpeed(mech)) / MP1 + 1, -1, 0, 0);
			if (!SectIsDestroyed(mech, RARM))
			    DamageMech(mech, mech, 0, -1, RARM, 0, 0, 0,
				(MechSpeed(mech) -
				    MechMaxSpeed(mech)) / MP1 + 1, -1, 0, 0);

		    }
		    if (!SectIsDestroyed(mech, LLEG))
			DamageMech(mech, mech, 0, -1, LLEG, 0, 0, 0,
			    (MechSpeed(mech) - MechMaxSpeed(mech)) / MP1 +
			    1, -1, 0, 0);
		    if (!SectIsDestroyed(mech, RLEG))
			DamageMech(mech, mech, 0, -1, RLEG, 0, 0, 0,
			    (MechSpeed(mech) - MechMaxSpeed(mech)) / MP1 +
			    1, -1, 0, 0);
		} else {
		    mech_notify(mech, MECHALL,
			"Your damaged mech falls as you try to run");
		    MechLOSBroadcast(mech, "falls down.");
		    MechFalls(mech, 1, 0);
		}
	    }
	}
    }
    if (MechType(mech) == CLASS_MECH)
	CheckDamage(mech);
    if ((event_tick % TURN) == 0)
	MechTurnDamage(mech) = 0;
}


/* This function is called once every second for every mech in the game */
void mech_update(dbref key, void *data)
{
    MECH *mech = (MECH *) data;

    if (!mech)
	return;
    MechStatus(mech) &= ~FIRED;
    if (is_aero(mech)) {
	aero_update(mech);
	return;
    }
    if (Started(mech) || Uncon(mech))
	UpdatePilotSkillRolls(mech);
    if (Started(mech) || MechPlusHeat(mech) > 0.1)
	UpdateHeat(mech);
    if (Started(mech))
	MechVisMod(mech) =
	    BOUNDED(0, MechVisMod(mech) + Number(-40, 40), 100);
    end_ecm_check(mech);
    end_lite_check(mech);
}

void CheckNoAir(MECH * mech)
{
	if (!mech)
		return;
	if (!(MechCritStatus(mech) & LIFE_SUPPORT_DESTROYED))
		return;
	if (event_count_type_data(EVENT_NOAIR, mech) > 0)
		return;

	mech_notify(mech, MECHALL, "The water cuts off your air supply!");
	headhitmwdamage(mech, 1);
	MECHEVENT(mech, EVENT_NOAIR, air_cutoff_event, NOAIR_TICK, 0);
}

