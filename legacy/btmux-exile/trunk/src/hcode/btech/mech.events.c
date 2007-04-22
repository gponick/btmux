#include <math.h>
#include "mech.events.h"
#include "mech.notify.h"
#include "mech.partnames.h"
#include "p.aero.move.h"
#include "p.mech.utils.h"
#include "p.mech.update.h"
#include "p.mech.los.h"
#include "p.btechstats.h"
#include "p.mech.sensor.h"
#include "p.mech.partnames.h"
#include "p.mech.combat.h"

static int factoral(int n)
{
    int i, j = 0;

    for (i = 1; i <= n; i++)
	j += i;
    return j;
}

void mech_standfail_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    mech_notify(mech, MECHALL,
	"%cgYou have finally recovered from your attempt to stand.%c");
}

void mech_fall_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int fallspeed = (int) e->data2;
    int fallen_elev;

    if (Started(mech) && fallspeed >= 0)
	return;
    if (fallspeed <= 0 && (!Started(mech) || !(FlyingT(mech)) ||
	    AeroFuel(mech) <= 0))
	fallspeed -= FALL_ACCEL;
    else
	fallspeed += FALL_ACCEL;
    MarkForLOSUpdate(mech);
    if (MechsElevation(mech) > abs(fallspeed)) {
	MechZ(mech) -= abs(fallspeed);
	MechFZ(mech) = MechZ(mech) * ZSCALE;
	MECHEVENT(mech, EVENT_FALL, mech_fall_event, FALL_TICK, fallspeed);
	return;
    }
    /* Time to hit da ground */
    fallen_elev = factoral(abs(fallspeed));
    mech_notify(mech, MECHALL, "You hit the ground!");
    MechLOSBroadcast(mech, "hits the ground!");
    MechFalls(mech, fallen_elev, 0);
    MechStatus(mech) &= ~JUMPING;
}

/* This is just a 'toy' event */
void mech_lock_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    MAP *map;
    MECH *target;

    if (MechTarget(mech) >= 0) {
	map = getMap(mech->mapindex);
	target = FindObjectsData(MechTarget(mech));
	if (!target)
	    return;
	if (!InLineOfSight(mech, target, MechX(target), MechY(target),
		FlMechRange(map, mech, target)))
	    return;
	mech_notify(mech, MECHALL,
	    tprintf("The sensors acquire a stable lock on %s.",
		GetMechToMechID(mech, target)));
    } else if (MechTargX(mech) >= 0 && MechTargY(mech) >= 0)
	mech_notify(mech, MECHALL,
	    tprintf("The sensors acquire a stable lock on (%d,%d).",
		MechTargX(mech), MechTargY(mech)));

}

/* Various events that don't fit too well to other categories */

/* Basically the update events + some movenement events */
void mech_stabilizing_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    mech_notify(mech, MECHSTARTED,
	"%cgYou have finally stabilized after your jump.%c");
}

void mech_jump_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    MECHEVENT(mech, EVENT_JUMP, mech_jump_event, JUMP_TICK, 0);
    move_mech(mech);
    if (!Jumping(mech))
	StopJump(mech);
}


extern int PilotStatusRollNeeded[];

void mech_recovery_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (Destroyed(mech) || !Uncon(mech))
	return;
    if (handlemwconc(mech, 0)) {
	MechStatus(mech) &= ~UNCONSCIOUS;
	mech_notify(mech, MECHALL, "The pilot regains consciousness!");
	return;
    }
}

void mech_recycle_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int t;

    if ((t = recycle_weaponry(mech)) > 0)
	MaybeRecycle(mech, t);
}

void ProlongUncon(MECH * mech, int len)
{
    int l;

    if (Destroyed(mech))
	return;
    if (!Recovering(mech)) {
	MechStatus(mech) |= UNCONSCIOUS;
	MECHEVENT(mech, EVENT_RECOVERY, mech_recovery_event, len, 0);
	return;
    }
    l = event_last_type_data(EVENT_RECOVERY, (void *) mech) + len;
    event_remove_type_data(EVENT_RECOVERY, (void *) mech);
    MECHEVENT(mech, EVENT_RECOVERY, mech_recovery_event, l, 0);
}

void MaybeRecycle(MECH * mech, int wticks)
{
    int nr, dat;

#undef WEAPON_RECYCLE_DEBUG
#ifdef WEAPON_RECYCLE_DEBUG
	SendDebug(tprintf("MaybeRecycle called on #%d", mech->mynum));
#endif

    if (!(Started(mech) && !Destroyed(mech)))
	return;

#ifdef WEAPON_RECYCLE_DEBUG
	SendDebug(tprintf("MaybeRecycle passed Start/Dest for #%d", mech->mynum));
#endif

    nr = NextRecycle(mech);
    UpdateRecycling(mech);
    if (nr < 0)
	MechLWRT(mech) = event_tick;
    if (nr < 0 || nr > ((wticks + 1) * WEAPON_TICK)) {
	dat = MAX(1, wticks * WEAPON_TICK);
	MECHEVENT(mech, EVENT_RECYCLE, mech_recycle_event, dat, 0);

#ifdef WEAPON_RECYCLE_DEBUG
	SendDebug(tprintf("%6d Recycle event for #%d set in %ds.",
		event_tick, mech->mynum, dat));
#endif
    }
#ifdef WEAPON_RECYCLE_DEBUG
    else
	SendDebug(tprintf("%6d Recycle event for #%d exists at %d secs",
		event_tick, mech->mynum, nr));
#endif
}

struct foo {
    char *name;
    char *full;
    int ofs;
};
extern struct foo lateral_modes[];

void mech_lateral_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int latmode = (int) e->data2;

    if (!mech || !Started(mech))
	return;
    mech_notify(mech, MECHALL,
	tprintf("Lateral movement mode change to %s completed.",
	    lateral_modes[latmode].full));
    MechLateral(mech) = lateral_modes[latmode].ofs;

    if (MechMove(mech) != MOVE_QUAD) {
	if (MechLateral(mech) == 0)
	    StopSideslip(mech);
	else if (!(SideSlipping(mech)))
	    MECHEVENT(mech, EVENT_SIDESLIP, mech_sideslip_event, 1, 0);
	}
}

void mech_sideslip_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int roll;

    if (!mech || !Started(mech))
	return;
    mech_notify(mech, MECHALL, "You make a skill roll while sideslipping!");

    if (!MadePilotSkillRoll(mech, HasBoolAdvantage(MechPilot(mech), "maneuvering_ace") ? -1001 : -1000))
        {
        mech_notify(mech, MECHALL, "You fail and spin out!");
        MechLOSBroadcast(mech, "spins out while sideslipping!");
	MechSpeed(mech) = 0.0;
	roll = Number(0, 5);
	AddFacing(mech, roll * 60);
        SetFacing(mech, AcceptableDegree(MechFacing(mech)));
        MechDesiredFacing(mech) = MechFacing(mech);
	MechDesiredSpeed(mech) = 0.0;
	MechLateral(mech) = 0;
	return;
	}
    MECHEVENT(mech, EVENT_SIDESLIP, mech_sideslip_event, TURN, 0);
}

void mech_crewstun_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (!mech)
	return;
    if (!Started(mech) || Destroyed(mech))
	{
	if (MechCritStatus(mech) & CREW_STUNNED)
	    MechCritStatus(mech) &= ~CREW_STUNNED;
	return;
	}
    if (MechType(mech) != CLASS_MECH)
	mech_notify(mech, MECHALL, "%ch%cgThe crew recovers from their bewilderment!%cn");
    else
	mech_notify(mech, MECHALL, "%ch%cgYou recover from your stunning experience!%cn");

    if (MechCritStatus(mech) & CREW_STUNNED)
	MechCritStatus(mech) &= ~CREW_STUNNED;
}

void mech_nullsig_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (MechCritStatus(mech) & NULLSIG_DESTROYED)
	return;
    if (e->data2)
	{
	MechStatus2(mech) |= NULLSIG_ACTIVE;
	mech_notify(mech, MECHALL, tprintf("%%ch%%crYour Null Signature Device kicks on!%%c"));
        MarkForLOSUpdate(mech);
	return;
	} else {
	MechStatus2(mech) &= ~NULLSIG_ACTIVE;
	mech_notify(mech, MECHALL, tprintf("%%ch%%crYour Null Signature Device finally turns off!%%c"));
	MechWeapHeat(mech) += 10.;
	MarkForLOSUpdate(mech);
	return;
	}
    MarkForLOSUpdate(mech);
    return;
}


void mech_stealtharm_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if ((MechCritStatus(mech) & ECM_DESTROYED) || (MechCritStatus(mech) & STEALTHARM_DESTROYED))
	return;
    if (e->data2)
	{
	MechStatus2(mech) |= STEALTHARM_ACTIVE;
	mech_notify(mech, MECHALL, tprintf("%%ch%%crYour Stealth Armor kicks on!%%c"));
        MarkForLOSUpdate(mech);
	return;
	} else {
	MechStatus2(mech) &= ~STEALTHARM_ACTIVE;
	mech_notify(mech, MECHALL, tprintf("%%ch%%crYour Stealth Armor finally turns off!%%c"));
	MechWeapHeat(mech) += 10.;
	MarkForLOSUpdate(mech);
	return;
	}
    MarkForLOSUpdate(mech);
    return;
}

void mech_unjam_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int section, critical, weaptype;
    int index = (int) e->data2;
    int ammoLoc, ammoCrit;

    if (!mech)
	return;
    if (!Started(mech) || Destroyed(mech) || !(MechStatus2(mech) & UNJAMMING) || Uncon(mech) || Blinded(mech)) {
	MechStatus2(mech) &= ~UNJAMMING;
	return;
	}
    {
    int roll, roll_needed;

    roll = Roll();
    roll_needed = FindSPilotPiloting(mech) + 3 + MechPilotSkillBase(mech);
    mech_notify(mech, MECHALL, tprintf("%%ch%%cgYou complete your attempt at unjamming.%%c"));
    mech_notify(mech, MECHPILOT, "You make a piloting skill roll to unjam!");
    mech_notify(mech, MECHPILOT, tprintf("Modified Pilot Skill: BTH %d\tRoll: %d", roll_needed, roll));
    if (roll >= roll_needed && roll_needed > 2)
	AccumulatePilXP(MechPilot(mech), mech, BOUNDED(1, roll_needed - 7, MAX(2, 1 + 3))); 

    if (index == -1) {
	if (roll >= roll_needed) {
            mech_notify(mech, MECHALL, "You unjam your turret!");
	    MechCritStatus(mech) &= ~TURRET_JAMMED;
	    } else {
            mech_notify(mech, MECHALL, "Your turret.. it just .. won't... budge...");
	    }
	MechStatus2(mech) &= ~UNJAMMING;
	MechLOSBroadcast(mech, "ceases its groovy spinning moves.");
	return;
	}
    FindWeaponNumberOnMech_Advanced(mech, index, &section, &critical, 1);
    weaptype = Weapon2I(GetPartType(mech, section, critical));
    if (roll >= roll_needed) {
        mech_notify(mech, MECHALL, tprintf("You unjam your %s!",
		get_parts_long_name(GetPartType(mech, section, critical))));
        GetPartMode(mech, section, critical) &= ~JAMMED_MODE;
        if (FindAndCheckAmmo(mech, weaptype, section, critical, &ammoLoc, &ammoCrit))
            decrement_ammunition(mech, weaptype, section, critical, ammoLoc, ammoCrit);
        } else {
        mech_notify(mech, MECHALL, tprintf("You can't manage to unjam the %s!",
		get_parts_long_name(GetPartType(mech, section, critical))));
        }
    }

    MechLOSBroadcast(mech, "ceases its wild shagodelic movement.");
    MechStatus2(mech) &= ~UNJAMMING;
}

void mech_move_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (MechType(mech) == CLASS_VTOL)
	if (Landed(mech) || FuelCheck(mech))
	    return;
    UpdateHeading(mech);
    if ((MechCritStatus(mech) & NO_LEGS) || Jumping(mech) || OODing(mech)) {
	if (MechDesiredFacing(mech) != MechFacing(mech))
	    MECHEVENT(mech, EVENT_MOVE, mech_move_event, MOVE_TICK, 0);
	return;
    }
    UpdateSpeed(mech);
    move_mech(mech);
    if (mech->mapindex < 0)
	return;
    if (MechType(mech) == CLASS_VEH_NAVAL) {
	if ((fabs(MechSpeed(mech)) > 0.0 ||
		fabs(MechDesiredSpeed(mech)) > 0.0 ||
		MechDesiredFacing(mech) != MechFacing(mech)) &&
	    (MechRTerrain(mech) == BRIDGE || MechRTerrain(mech) == ICE ||
		MechRTerrain(mech) == WATER || MechRTerrain(mech) == DBRIDGE))
	    MECHEVENT(mech, EVENT_MOVE, mech_move_event, MOVE_TICK, 0);
	return;
    }
    if (fabs(MechSpeed(mech)) > 0.0 || fabs(MechDesiredSpeed(mech)) > 0.0
	|| MechDesiredFacing(mech) != MechFacing(mech) ||
	MechType(mech) == CLASS_VTOL || (MechType(mech) == CLASS_VEH_NAVAL
	    && MechVerticalSpeed(mech) != 0))
	MECHEVENT(mech, EVENT_MOVE, mech_move_event, MOVE_TICK, 0);
}

void mech_stand_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    MechLOSBroadcast(mech, "stands up!");
    mech_notify(mech, MECHALL, "You have finally finished standing up.");
    MakeMechStand(mech);
}

void mech_movemode_event(EVENT *e)
{
    MECH *mech = (MECH *) e->data;
    int i = (int) e->data2;
    int dir = (i & MODE_ON ? 1 : i & MODE_OFF ? 0 : 0);

    if (!mech || !Started(mech) || Destroyed(mech)) {
	MechStatus2(mech) &= ~(MOVE_MODES);
	return;
	}
/* SPRINTING flag needs to be removed already. This is to create a dumby-event to allow
   blocking of weapons fire still after a sprinting units takes damage. */
    if (i & MODE_MODEFAIL) {
	mech_notify(mech, MECHALL, "Your brief movement mode hiatus has ended and you may fire once again!");
	return;
	}
    if (dir) {
	if (i & MODE_EVADE) {
	    MechStatus2(mech) |= EVADING;
	    mech_notify(mech, MECHALL, "You bounce chaotically as you maximize your movement mode to evade!");
	    MechLOSBroadcast(mech, "suddenly begins to move erratically performing evasive maneuvers!");
	} else if (i & MODE_SPRINT) {
	    MechStatus2(mech) |= SPRINTING;
	    mech_notify(mech, MECHALL, "You shimmy side to side as you get more speed from your movement mode.");
	    MechLOSBroadcast(mech, "breaks out into a full blown stride as it sprints over the terrain!");
	} else if (i & MODE_DODGE) {
	    if (MechFullNoRecycle(mech, CHECK_PHYS) > 0) {
		mech_notify(mech, MECHALL, "You cannot enter DODGE mode due to physical useage.");
		return;
	    } else {
	        MechStatus2(mech) |= DODGING;
	        mech_notify(mech, MECHALL, "You brace yourself for any oncoming physicals.");
	    }
	}
    } else {
	if (i & MODE_EVADE) {
	    MechStatus2(mech) &= ~EVADING;
	    mech_notify(mech, MECHALL, "Cockpit movement normalizes as you cease your evasive maneuvers.");
	    MechLOSBroadcast(mech, "ceases its evasive behavior and calms down.");
	} else if (i & MODE_SPRINT) {
	    MechStatus2(mech) &= ~SPRINTING;
	    mech_notify(mech, MECHALL, "You feel less seasick as you leave your sprint mode and resume normal movement.");
	    MechLOSBroadcast(mech, "slows down and enters a normal movement mode.");
	} else if (i & MODE_DODGE) {
	    MechStatus2(mech) &= ~DODGING;
	    if (i & MODE_DG_USED)
		mech_notify(mech, MECHALL, "You're dodge maneuver has been used and you are no longer braced for physicals.");
	    else
	        mech_notify(mech, MECHALL, "You loosen up your stance and no longer dodge physicals.");
	}
    }
    if (MechSpeed(mech) > MMaxSpeed(mech) || MechDesiredSpeed(mech) > MMaxSpeed(mech))
	MechDesiredSpeed(mech) = MMaxSpeed(mech);
    return;
}

void mech_plos_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data, *target;
    MAP *map;
    int mapvis;
    int maplight;
    float range;
    int i;

    if (!Started(mech))
	return;
    if (!(map = getMap(mech->mapindex)))
	return;
    MECHEVENT(mech, EVENT_PLOS, mech_plos_event, PLOS_TICK, 0);
    if (!MechPNumSeen(mech) && !((MechSpecials(mech) & AA_TECH) || MechSpecials2(mech) & ADV_AA_TECH))
	return;
    mapvis = map->mapvis;
    maplight = map->maplight;
    MechPNumSeen(mech) = 0;
    for (i = 0; i < map->first_free; i++)
	if (map->mechsOnMap[i] > 0 && map->mechsOnMap[i] != mech->mynum)
	    if (!(map->LOSinfo[mech->mapnumber][i] & MECHLOSFLAG_SEEN)) {
		target = FindObjectsData(map->mechsOnMap[i]);
		if (!target)
		    continue;
		range = FlMechRange(map, mech, target);
		MechPNumSeen(mech)++;
		Sensor_DoWeSeeNow(mech, &map->LOSinfo[mech->mapnumber][i], range, -1, -1, target, mapvis, maplight, map->cloudbase, 1, 0);

	    }
}

void aero_move_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (FuelCheck(mech))
	return;

    if (!Landed(mech)) {
	/* Returns 1 only if we
	   1) Ran out of fuel, and
	   2) Were VTOL, and
	   3) Crashed
	 */
	if (Started(mech)) {
	    aero_UpdateHeading(mech);
	    if (SpheroidDS(mech) && MechZ(mech) < ORBIT_Z)
		aero_UpdateSpeed_old(mech);
	    else
		aero_UpdateSpeed(mech);
	}
	if (Fallen(mech))
	    MechStartFZ(mech) = MechStartFZ(mech) - 1;
	move_mech(mech);
/* Removing this due to prevalence of AeroDyne DropCarriers */
/*
	if (IsDS(mech) && MechZ(mech) <= (MechElevation(mech) + 5) &&
	    ((event_tick / WEAPON_TICK) % 10) == 0)
	    DS_BlastNearbyMechsAndTrees(mech,
		"You are hit by the DropShip's plasma exhaust!",
		"is hit directly by DropShip's exhaust!",
		"You are hit by the DropShip's plasma exhaust!",
		"is hit by DropShip's exhaust!", "light up and burn.", 8);
*/
	MECHEVENT(mech, EVENT_MOVE, aero_move_event, MOVE_TICK, 0);
    } else if (Landed(mech) && !Fallen(mech) && RollingT(mech)) {
	UpdateHeading(mech);
	UpdateSpeed(mech);
	move_mech(mech);
	if (fabs(MechSpeed(mech)) > 0.0 ||
	    fabs(MechDesiredSpeed(mech)) > 0.0 ||
	    MechDesiredFacing(mech) != MechFacing(mech))
	    if (!FuelCheck(mech))
		MECHEVENT(mech, EVENT_MOVE, aero_move_event, MOVE_TICK, 0);
    }
}

void acid_damage_event(EVENT * e)
{
	MECH *mech = (MECH *) e->data;
	int loc = (int) e->data2;
	int dam = Number(1, 3);
	int max = GetSectInt(mech,loc) + GetSectArmor(mech,loc);
	char locname[32];

	if (!mech)
		return;
	if (!GetSectInt(mech, loc))
		return;
	if (!(MechSections(mech)[loc].config & SECTION_ACID))
		return;

	event_remove_type_data_data(EVENT_ACID, (void *) mech, (void *) loc);
	ArmorStringFromIndex(loc, locname, MechType(mech), MechMove(mech));

	if (dam > max)
		dam = max;

	mech_notify(mech, MECHALL, tprintf("%%ch%%cgThe acid eats further into your %s!%%c", locname));
	MechLOSBroadcast(mech, tprintf("fumes from its %s.", locname));
	DamageMech(mech, mech, 0, -1, loc, 0, 0, dam, 0, -1, 0, 0);

	if (dam != 1 && GetSectInt(mech, loc))
		MECHEVENT(mech, EVENT_ACID, acid_damage_event, ACID_TICK, loc);
	else {
		if (dam == max)
			mech_notify(mech, MECHALL,
				tprintf("%%ch%%cgThe acid has eaten completely through your %s!!%%c", locname));
		else {
			mech_notify(mech, MECHALL, tprintf("%%ch%%cgThe acid wears off your %s.%%c", locname));
			AcidClear(mech, loc, 1);
		}
	}
}

void air_cutoff_event(EVENT * e)
{
	MECH *mech = (MECH *) e->data;

	if (!mech)
		return;
	event_remove_type_data(EVENT_NOAIR, (void *) mech);

	if (!(MechCritStatus(mech) & LIFE_SUPPORT_DESTROYED))
		return;

	if (MechRTerrain(mech) != WATER && MechRTerrain(mech) != HIGHWATER)
		return;
	if (MechZ(mech) > -2 && !(Fallen(mech) && MechZ(mech) <= -1))
		return;

	mech_notify(mech, MECHALL, "You're running out of air!");
	headhitmwdamage(mech, 1);
	MECHEVENT(mech, EVENT_NOAIR, air_cutoff_event, NOAIR_TICK, 0);
}

void very_fake_func(EVENT * e)
{

}
