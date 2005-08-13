#include <math.h>

#include "mech.h"
#include "mech.events.h"
#include "p.mech.utils.h"
#include "p.mech.combat.h"
#include "p.mech.los.h"
#include "p.mech.update.h"
#include "p.crit.h"
#include "p.btechstats.h"
#include "p.bsuit.h"

/* 2 battlesuit-specific attacks:
   - attackleg
   - swarm
 */

#define MyHiddenTurns(mech) ((MechType(mech) == CLASS_MW ? 1 : MechType(mech) == CLASS_BSUIT ? 3 : MechType(mech) == CLASS_VTOL ? 4 : 5) * ((MechSpecials2(mech) & HIDDEN_TECH) ? 1 : 2))


/* Stops everyone who's swarming this poor guy */

char *bsuit_name(MECH * mech)
{
    return (MechSpecials(mech) & CLAN_TECH) ? "Point" : "Team";
}

void bsuit_start_recycling(MECH * mech)
{
    int i;

    for (i = 0; i < NUM_BSUIT_MEMBERS; i++)
	if (GetSectInt(mech, i))
	    SetRecycleLimb(mech, i, BSUIT_RECYCLE_TIME);
}

void bsuit_stopswarmer(MECH * mech, MECH * t, int intentional)
{
    MAP *map;

    if (MechSwarmTarget(t) != mech->mynum || MechSwarmTarget(t) <= 0)
	return;
/*    MechSwarmTarget(t) = -1; */
    if (intentional > 0) {
	mech_notify(t, MECHALL,
	    "You let your hold loosen and you drop from the 'mech!");
	mech_notify(mech, MECHALL, tprintf("%s lets go of you!",
		GetMechToMechID(mech, t)));
	MechLOSBroadcasti(t, mech, "lets go of %s!");
	MechSwarmTarget(t) = -1;
    } else {
	int grap = (MechSpecials2(t) & GRAPPLE_CLAW ? 1 : 0);
	if (intentional == -1 && !MadePilotSkillRoll(mech, 4 + grap))
	    return;
	if (intentional == -3 && MadePilotSkillRoll(t, 2 - grap))
	    return;
	mech_notify(t, MECHALL,
	    "The hold loosens and you drop from the 'mech!");
	MechLOSBroadcasti(t, mech, "falls off %s!");
	mech_notify(mech, MECHALL, tprintf("%s falls off!",
		GetMechToMechID(mech, t)));
	if (intentional != -3)
	    DamageMech(t, t, 1, -1, Number(0, NUM_BSUIT_MEMBERS - 1), 0, 0,
	        (intentional == -2 ? (MechTons(mech) / 3) : 11), 0, -1, 0, 0);
	MechSwarmTarget(t) = -1;
    }

    map = getMap(mech->mapindex);
    if (map) {
	MechElev(mech) = GetElev(map, MechX(mech), MechY(mech));
	MechZ(mech) = MechElev(mech) - 1;
	MechFZ(mech) = MechZ(mech) * ZSCALE;
	DropSetElevation(mech, 0);
	}
    bsuit_start_recycling(t);
    MechSpeed(t) = 0;
    MaybeMove(t);
}

int bsuit_countswarmers(MECH * mech)
{
    MAP *map = FindObjectsData(mech->mapindex);
    int count = 0, i, j;
    MECH *t;

    if (!map)
	return 0;
    for (i = 0; i < map->first_free; i++)
	if ((j = map->mechsOnMap[i]) > 0 && i != mech->mapnumber) {
	    if (!(t = FindObjectsData(j)))
		continue;
	    if (MechSwarmTarget(t) != mech->mynum)
		continue;
	    count++;
	}
    return count;
}

void bsuit_stopswarmers(MAP * map, MECH * mech, int intentional)
{
    int i, j;
    MECH *t;

    if (!map || !mech)
	return;
    for (i = 0; i < map->first_free; i++)
	if ((j = map->mechsOnMap[i]) > 0 && i != mech->mapnumber) {
	    if (!(t = FindObjectsData(j)))
		continue;
	    if (MechSwarmTarget(t) != mech->mynum)
		continue;
	    bsuit_stopswarmer(mech, t, intentional);
	}
}

void bsuit_mirrorswarmers(MAP * map, MECH * mech)
{
    int i, j;
    MECH *t;

    for (i = 0; i < map->first_free; i++)
	if ((j = map->mechsOnMap[i]) > 0 && i != mech->mapnumber) {
	    if (!(t = FindObjectsData(j)))
		continue;
	    if (MechSwarmTarget(t) != mech->mynum)
		continue;
	    MirrorPosition(mech, t);
	}
}

int bsuit_attack_common_checks(MECH * mech, dbref player)
{
    int i, j;

    DOCHECK1(Jumping(mech), "Unavailable when jumping - sorry.");
    DOCHECK1(MechSwarmTarget(mech) > 0,
	"You are already busy with a special attack!");
    for (i = 0; i < NUM_BSUIT_MEMBERS; i++) {
	DOCHECK1(MechSections(mech)[i].recycle,
	    tprintf("Suit %d is still recovering from attack.", i + 1));
	for (j = 0; j < NUM_CRITICALS; j++)
	    DOCHECK1(WpnIsRecycling(mech, i, j),
		tprintf("Suit %d is still recovering from weapon attack.",
		    i + 1));
    }
    return 0;
}

int bsuit_members(MECH * mech)
{
    int i, j = 0;

    for (i = 0; i < NUM_BSUIT_MEMBERS; i++)
	if (GetSectInt(mech, i))
	    j++;
    return j;
}

int bsuit_findtarget(dbref player, MECH * mech, MECH ** target,
    char *buffer)
{
    int argc;
    char *args[3];
    float range;
    char targetID[2];
    int targetnum;
    MECH *t = NULL;

    DOCHECK1((argc =
	    mech_parseattributes(buffer, args, 3)) > 1,
	"Invalid arguments!");
    switch (argc) {
    case 0:
	DOCHECK1(MechTarget(mech) <= 0,
	    "You do not have a default target set!");
	t = getMech(MechTarget(mech));
	if (!(t)) {
	    mech_notify(mech, MECHALL, "Invalid default target!");
	    MechTarget(mech) = -1;
	    return 1;
	}
	break;
    case 1:
	targetID[0] = args[0][0];
	targetID[1] = args[0][1];
	targetnum = FindTargetDBREFFromMapNumber(mech, targetID);
	DOCHECK1(targetnum <= 0, "That is not a valid target ID!");
	t = getMech(targetnum);
	DOCHECK1(!(t), "Invalid default target!");
	break;
    default:
	notify(player, "Invalid target!");
	return 1;
    }
    DOCHECK1((range =
	    FaMechRange(mech, t)) >= (mudconf.btech_bsuitmode == 0 ? 0.5 : 1.0), "Target out of range!");
    DOCHECK1(Jumping(t), "That target's unreachable right now!");
/*    DOCHECK1(MechType(t) != CLASS_MECH, "That target is of invalid type.");*/
    DOCHECK1(Destroyed(t), "A dead 'mech? C'mon :P");
    *target = t;
    return 0;
}

void bsuit_swarm(dbref player, void *data, char *buffer)
{
    MECH *mech = data;
    MECH *target;
    int baseToHit;
    int i;

    cch(MECH_USUALO);
    skipws(buffer);
    DOCHECK(MechJumpSpeed(mech) < 1, "Need jumpjets to swarm!");
    if (!strcmp(buffer, "-")) {
	MECH *tmpmech = getMech(MechSwarmTarget(mech));
	if (MechSwarmTarget(mech) > 0 && tmpmech) {
	    bsuit_stopswarmer(tmpmech, mech, 1);
	    return;
	    } else {
		MechSwarmTarget(mech) = -1;
		return;
	    }
	}
    if (bsuit_attack_common_checks(mech, player))
	return;
    DOCHECK(Stabilizing(mech), "Wait for your sense of balance to catch up!");
    if (bsuit_findtarget(player, mech, &target, buffer))
	return;
    DOCHECK((MechIsSwarmed(target) && MechTeam(mech) == MechTeam(target)), "Only one suit may mount a mech at a time!");
    DOCHECK(MoveModeLock(mech), "Movement modes disallow swarming.");
    DOCHECK(MechType(target) != CLASS_MECH, "That target is of invalid type.");

/* FASA Swarm Attacks Table */
    baseToHit = (bsuit_members(mech) >= 4 ? 3 : 6);
/* FASA rules state 'Modify it by movement and terrain'. */
  if (!(MechCritStatus(mech) & HIDDEN)) {
    if (mudconf.btech_bsuitmode == 0)
	baseToHit += AttackMovementMods(mech);
    baseToHit += TargetMovementMods(mech, target, 0.0);
    switch (MechRTerrain(target)) {
	case WATER:
	case HIGHWATER:
	    baseToHit -= 1;
	    break;
	case LIGHT_FOREST:
	case SMOKE:
	    baseToHit += 1;
	    break;
	case HEAVY_FOREST:
	case HSMOKE:
	    baseToHit += 2;
	    break;
	}
  }
    if (MechStatus2(target) & DODGING) {
	baseToHit += 2;
/*	DodgeOff(target); */
	}

    if (Fallen(target) || !Started(target) || Standing(target))
	baseToHit -= 4;

    if (MechTeam(mech) == MechTeam(target) && (MechSpeed(target) <= MP4))
	baseToHit -= 3;

    if (MechSpecials2(mech) & GRAPPLE_CLAW)
	baseToHit--;

    if (MechTeam(mech) == MechTeam(target)) {
        if (MechSpeed(target) >= MP1)
	    if (!(MadePilotSkillRoll(mech, baseToHit)))
		{
		mech_notify(target, MECHALL, tprintf("%s attempts to mount you!",
		    GetMechToMechID(target, mech)));
		mech_notify(mech, MECHALL, tprintf("Nice try, but you don't succeed in your attempt at mounting %s!",
                    GetMechToMechID(mech, target)));
		bsuit_start_recycling(mech);
		MechCritStatus(mech) &= ~HIDDEN;
		return;
		}
        mech_notify(target, MECHALL, tprintf("%s mounts you!",
  	    GetMechToMechID(target, mech)));
        mech_notify(mech, MECHALL, tprintf("You mount %s!",
	    GetMechToMechID(mech, target)));
        MechSwarmTarget(mech) = target->mynum;
        MechLOSBroadcasti(mech, target, " mounts %s!");
        MechSpeed(mech) = 0.0;
        MechDesiredSpeed(mech) = 0.0;
        SetFacing(mech, 180);
        MechDesiredFacing(mech) = 180;
        MirrorPosition(target, mech);
    } else {
	if (MadePilotSkillRoll(mech, baseToHit))
		{
	        mech_notify(target, MECHALL, tprintf("%s swarms you!",
      		        GetMechToMechID(target, mech)));
	        mech_notify(mech, MECHALL, tprintf("You swarm %s!",
        	        GetMechToMechID(mech, target)));
	        MechSwarmTarget(mech) = target->mynum;
	        MechLOSBroadcasti(mech, target, " swarms %s!");
	        MechSpeed(mech) = 0.0;
	        MechDesiredSpeed(mech) = 0.0;
	        SetFacing(mech, 180);
	        MechDesiredFacing(mech) = 180;
	        MirrorPosition(target, mech);
		} else {
		mech_notify(target, MECHALL, tprintf("%s attempts to swarm you!",
			GetMechToMechID(target, mech)));
		mech_notify(mech, MECHALL,
		    tprintf
		    ("Nice try, but you don't succeed in your attempt at swarming %s!",
		GetMechToMechID(mech, target)));
		}
    }
    MechCritStatus(mech) &= ~HIDDEN;
    bsuit_start_recycling(mech);
}

void bsuit_attackleg(dbref player, void *data, char *buffer)
{
    MECH *mech = data;
    MECH *target;
    int leg;
    int baseToHit;
    int motive;
    int r, i;

    cch(MECH_USUALO);
    if (bsuit_attack_common_checks(mech, player))
		return;
    if (bsuit_findtarget(player, mech, &target, buffer))
		return;
/*    DOCHECK(MechJumpSpeed(mech) < 1, "Need jumpjets to attack legs!"); */
    DOCHECK(Stabilizing(mech), "Wait for your sense of balance to catch up!");
    DOCHECK(MoveModeLock(mech), "Movement modes disallow swarming.");
    DOCHECK(MechType(target) == CLASS_MECH && MechCritStatus(target) & NO_LEGS,
		"That mech has no legs to grab!");
    DOCHECK(MechType(target) != CLASS_MECH && MechType(target) != CLASS_VEH_GROUND,
		"There's nothing to attack!");

    switch (bsuit_members(mech)) {
    	case 3:
	    baseToHit = 3;
	    break;
    	case 2:
	    baseToHit = 6;
	    break;
    	case 1:
	    baseToHit = 8;
	    break;
	default:
	    baseToHit = 0;
	    break;
    }
  if (!(MechCritStatus(mech) & HIDDEN)) {
    if (mudconf.btech_bsuitmode == 0)
        baseToHit += AttackMovementMods(mech);
    baseToHit += TargetMovementMods(mech, target, 0.0);

    if (MechStatus2(target) & DODGING) {
		baseToHit += 2;
/*		DodgeOff(target); */
	}
	switch (MechRTerrain(target)) {
		case WATER:
		case HIGHWATER:
			baseToHit -= 1;
			break;
		case LIGHT_FOREST:
		case SMOKE:
			baseToHit += 1;
			break;
		case HEAVY_FOREST:
		case HSMOKE:
			baseToHit += 2;
			break;
	}
  } 

    if (Standing(target) || Fallen(target) || !Started(target))
		baseToHit -= 4;

    if (MechSpecials2(mech) & GRAPPLE_CLAW)
	baseToHit--;

    if (MechType(target) == CLASS_MECH) {
        leg = (Number(0, 1)) ? RLEG : LLEG;
	if (!(GetSectInt(target, leg)))
	    leg = 5 + (6 - leg);
	DOCHECK(!(GetSectInt(target, leg)), "You find no leg to grab?");
	if (MadePilotSkillRoll(mech, baseToHit)) {
			mech_notify(target, MECHALL, tprintf("%s charges your legs!", GetMechToMechID(target, mech)));
			mech_notify(mech, MECHALL, tprintf("You go for %s's legs!",
			GetMechToMechID(mech, target)));
			MechLOSBroadcasti(mech, target, "charges %s's legs!");
			r = Roll();
			if (MechSpecials(target) & ICE_TECH)
				r += 2;
			SendDebug(tprintf("AttackLeg: #%d attackleg #%d - CritRoll %d", mech->mynum, target->mynum, r));
			switch (r) {
				case 8:
				case 9:
					HandleCritical(target, mech, 1, leg, 1, 1, 1);
					break;
				case 10:
				case 11:
					HandleCritical(target, mech, 1, leg, 2, 1, 1);
					break;
				case 12:
				case 13:
					HandleCritical(target, mech, 1, leg, 3, 1, 1);
					break;
				case 14:
					HandleCritical(target, mech, 1, leg, 4, 1, 1);
					break;
				default:
					DamageMech(target, mech, 1, MechPilot(mech), leg, 0, 0, 4, 0, -1, 0, 0);
			}
	    } else {
			mech_notify(target, MECHALL,
			tprintf("%s attempt at hitting your legs was less than exemplary.", GetMechToMechID(target, mech)));
			mech_notify(mech, MECHALL,
			tprintf("Nice try, but you don't succeed in your attempt at hitting %s's legs!", GetMechToMechID(mech, target)));
			MechLOSBroadcasti(mech, target, "fails its attempt at charging %s's legs!");
    	    }
	} else {
		if (MadePilotSkillRoll(mech, baseToHit)) {
			mech_notify(target, MECHALL, tprintf("%s charges your %s!", GetMechToMechID(target, mech),
				MechMove(target) == MOVE_HOVER ? "air skirt" : MechMove(target) == MOVE_WHEEL ? "wheels" :
				MechMove(target) == MOVE_TRACK ? "tracks" : "movement system"));
			mech_notify(mech, MECHALL, tprintf("You go for %s's %s!", GetMechToMechID(mech, target),
				MechMove(target) == MOVE_HOVER ? "air skirt" : MechMove(target) == MOVE_WHEEL ? "wheels" :
				MechMove(target) == MOVE_TRACK ? "tracks" : "movement system"));
			MechLOSBroadcasti(mech, target, tprintf("charges %%s's %s!",
				MechMove(target) == MOVE_HOVER ? "air skirt" : MechMove(target) == MOVE_WHEEL ? "wheels" :
				MechMove(target) == MOVE_TRACK ? "tracks" : "movement system"));

			motive = Roll();
    		if (MechMove(target) == MOVE_WHEEL)
    			motive += 2;
    		else if (MechMove(target) == MOVE_HOVER)
    			motive += 4;
    		if (motive >= 12) {
				mech_notify(target, MECHALL, "%ch%cyCRITICAL HIT!!%c");
				mech_notify(target, MECHALL, tprintf("Your %s heavily damaged!!",
					MechMove(target) == MOVE_HOVER ? "air skirt is" : MechMove(target) == MOVE_WHEEL ? "wheels are" :
					MechMove(target) == MOVE_TRACK ? "tracks are" : "movement system is"));
				LowerMaxSpeed(target, MP2);
				MechPilotSkillBase(target) += 3;
				mech_notify(target, MECHALL, "You are violently bounced around in your cockpit!");
				mech_notify(target, MECHALL, "You attempt to avoid personal injury!");
				if (MadePilotSkillRoll(target, (MechSpeed(target) / MP4))) {
				    mech_notify(target, MECHALL, "Your fancy moves worked!");
				} else {
				    mech_notify(target, MECHALL, "You cute fancy moves failed....");
			    	headhitmwdamage(target, 1);
				}
			} else if (motive >=10) {
				mech_notify(target, MECHALL, "%ch%cyCRITICAL HIT!!%c");
				mech_notify(target, MECHALL, tprintf("Your %s damaged!",
					MechMove(target) == MOVE_HOVER ? "air skirt is" : MechMove(target) == MOVE_WHEEL ? "wheels are" :
					MechMove(target) == MOVE_TRACK ? "tracks are" : "movement system is"));
				LowerMaxSpeed(target, MP1);
				MechPilotSkillBase(target) += 2;
			} else if (motive >= 8) {
				mech_notify(target, MECHALL, tprintf("Your %s hit!",
					MechMove(target) == MOVE_HOVER ? "air skirt is" : MechMove(target) == MOVE_WHEEL ? "wheels are" :
					MechMove(target) == MOVE_TRACK ? "tracks are" : "movement system is"));
				MechPilotSkillBase(target) += 1;
			}
		}
	}
    bsuit_start_recycling(mech);
}

static void mech_hide_event(EVENT * e)
{
   MECH *mech = (MECH *) e->data;
   MECH *t;
   MAP *map = getMap(mech->mapindex);
   int fail = 0, i;
   int tic = (int) e->data2;

   if (!map)
      return;
   for (i = 0; i < map->first_free; i++) {
        if (map->mechsOnMap[i] <= 0)
            continue;
        if (!(t = FindObjectsData(map->mechsOnMap[i])))
            continue;
	if (MechCritStatus(t) & (CLAIRVOYANT|OBSERVATORIC|INVISIBLE))
	    continue;
        if (MechTeam(t) == MechTeam(mech))
            continue;
        if (!Started(t))
	    continue;
	if (Destroyed(t))
	    continue;
        if (InLineOfSight(t, mech, MechX(mech), MechY(mech), FaMechRange(t,
                    mech)))
            fail = 1;
    }
  if (MechsElevation(mech))
      fail = 1;
  if (fail) {
    mech_notify(mech, MECHALL,
	    "Your spidey sense tingles, telling you this isn't going to work......");
    return;
  } else if (tic < (MyHiddenTurns(mech) * HIDE_TICK)) {
    tic++;
    MECHEVENT(mech, EVENT_HIDE, mech_hide_event, 1, tic);
  } else if (!fail) {
    mech_notify(mech, MECHALL, "You are now hidden!");
    MechCritStatus(mech) |= HIDDEN;
  }
  return;
}

void bsuit_hide(dbref player, void *data, char *buffer)
{
    int i;
    MECH *mech = data;
    MECH *t;
    MAP *map = FindObjectsData(mech->mapindex);

    cch(MECH_USUALO);
    DOCHECK(((MechSpecials2(mech) & HIDDEN_TECH) || (Wizard(player)) || mudconf.btech_extendedhide) ? 0 : MechType(mech) != CLASS_BSUIT && MechType(mech) != CLASS_MW,
	"Your not capable of such curious things.");

    DOCHECK(Hiding(mech), "You are looking for cover already!");
    mech_notify(mech, MECHALL, "You start your attempt at hiding..");

DOCHECK(MechMove(mech) == MOVE_VTOL && !Landed(mech), "You must be landed!");

DOCHECK(!(MechRTerrain(mech) == HEAVY_FOREST || MechRTerrain(mech) == LIGHT_FOREST ||
	      MechRTerrain(mech) == WATER || MechRTerrain(mech) == HIGHWATER || (MechType(mech) == CLASS_BSUIT ? MechRTerrain(mech) == BUILDING : 0)),
	"You cannot hide in this terrain!");

      MECHEVENT(mech, EVENT_HIDE, mech_hide_event, 1, 0);
}

void mech_thrash(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *t;
    MAP *map;
    int SuitsInHexNum[10];
    int count = 0, damage = 0, this_time = 0;
    int limbs = (4 - (SectIsDestroyed(mech, LARM) + SectIsDestroyed(mech, RARM) +
		      SectIsDestroyed(mech, LLEG) + SectIsDestroyed(mech, RLEG)));
    int i, j;
    float range;


    map = FindObjectsData(mech->mapindex);
    cch(MECH_USUALO);
/*    DOCHECK(1, "Command Disabled."); */
    DOCHECK(MechRTerrain(mech) != GRASSLAND && MechRTerrain(mech) != ROAD,
	"This attack can be done only on clear/paved terrains.");
    DOCHECK(!limbs, "You can't roll around like a maniac without any leverage.");
/*    DOCHECK(MechMove(mech) == MOVE_QUAD,
	"Woof Woof. Roll over, spot! Not...."); */
    DOCHECK(!Fallen(mech), "This command works only when you are fallen.");
/*    DOCHECK(!bsuit_countswarmers(mech),
	"Nobody's swarming you at the moment! Trashing would be pointless."); */
    for (i = 0; i < NUM_SECTIONS; i++)
	{
	if (!SectIsDestroyed(mech, i))
            DOCHECK(SectHasBusyWeap(mech, i), "You have weapons recycling!");
            DOCHECK(MechSections(mech)[i].recycle, "You are still recovering from your last attack.");
	}
    MechLOSBroadcast(mech, "starts thrashing around like a spasticated freak!");
    mech_notify(mech, MECHALL, "You flail around like a maniac in a straightjacket!");
/*    bsuit_stopswarmers(map, mech, -3); */
    if (!map || !mech)
        return;
    for (i = 0; i < 10; i++)
	SuitsInHexNum[i] = -1;
    for (i = 0; i < map->first_free; i++) {
	if (count == 10)
	    break;
        if ((j = map->mechsOnMap[i]) > 0 && i != mech->mapnumber)  {
            if (!(t = FindObjectsData(j)))
                continue;
            if (((range = FaMechRange(mech, t)) <= 0.5) && (MechSwarmTarget(t) != mech->mynum))
                if (MechType(t) == CLASS_BSUIT) {
	          SuitsInHexNum[count] = (t->mynum);
	          count++;
	  }
	}
    }
    damage = (limbs == 4 ? (MechTons(mech) / 3) : ((MechTons(mech) / 12) * limbs));
    if (SuitsInHexNum[0] > 0 && count > 0)
      while (damage > 0)
	{
	this_time = (damage < 5 ? damage : 5);
	damage = (damage - this_time);
	t = getMech(SuitsInHexNum[(j = Number(0, (count - 1)))]);
	if (SuitsInHexNum[j] < 0 || !t)
	    {
	    SendDebug(tprintf("Thrashing Mech #%d damaging non-existent unit! (Null Pointer)",
		mech->mynum));
	    break;
	    }
	mech_notify(mech, MECHALL, "You sense something going 'SQUISH' under you!");
	mech_notify(t, MECHALL, "You notice a large mech limb coming right at you!");
	DamageMech(t, mech, 1, MechPilot(mech), Number(0, NUM_BSUIT_MEMBERS - 1), 0, 0, this_time, 0, -1, 0, 0);
	if (damage <= 0)
	    damage = 0;
	}
    for (i = 0; i < NUM_SECTIONS; i++)
	SetRecycleLimb(mech, i, PHYSICAL_RECYCLE_TIME);
    MechStatus(mech) &=~ FALLEN;
    if (!MadePilotSkillRoll(mech, 2)) {
	mech_notify(mech, MECHALL, "You hurt yourself in the process, you spaz.");
	MechLOSBroadcast(mech, "is slightly hurt in the process!");
	MechFalls(mech, 1, 0);
    }
    MechStatus(mech) |= FALLEN;
}
