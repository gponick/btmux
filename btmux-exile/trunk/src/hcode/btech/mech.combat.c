#define Swap(val1,val2) { rtmp = val1 ; val1 = val2 ; val2 = rtmp; }

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "btmacros.h"
#include "mech.events.h"
#include "create.h"
#include "mech.partnames.h"
#include "p.mech.ice.h"
#include "p.pcombat.h"
#include "p.mech.utils.h"
#include "p.mech.los.h"
#include "p.mech.build.h"
#include "p.map.obj.h"
#include "p.mech.combat.h"
#include "p.btechstats.h"
#include "p.mech.update.h"
#include "p.bsuit.h"
#include "p.artillery.h"
#include "p.mech.c3.h"
#include "p.mine.h"
#include "p.mech.hitloc.h"
#include "p.eject.h"
#include "p.crit.h"
#include "p.mech.ood.h"
#include "p.map.conditions.h"
#include "p.template.h"
#include "p.mech.pickup.h"
#include "mech.sensor.h"
#include "p.ds.bay.h"
#include "autopilot.h"

#define Clustersize(weapindx, mode) \
((!(strstr(MechWeapons[weapindx].name,"Thunderbolt-")) && ((MechWeapons[weapindx].special & (IDF|MRM)) || \
    (GunStat(weapindx, 0, GUN_DAMAGE) == 1) || (MechWeapons[weapindx].special & ATM))) ?  5 : 1)

extern int arc_override;
extern dbref pilot_override;

void mech_safety(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    DOCHECK(MechType(mech) == CLASS_MW,
	"Your weapons dont have safeties.");
    if (buffer && !strcasecmp(buffer, "on")) {
	UnSetMechPKiller(mech);
	mech_notify(mech, MECHALL, "Safeties flipped %ch%cgON%cn.");
	return;
    }
    if (buffer && !strcasecmp(buffer, "off")) {
	SetMechPKiller(mech);
	mech_notify(mech, MECHALL, "Safeties flipped %ch%crOFF%cn.");
	return;
    }

    mech_notify(mech, MECHPILOT, tprintf("Weapon safeties are %%ch%s%%cn",
	    MechPKiller(mech) ? "%crOFF" : "%cgON"));
    return;
}

void mech_cruise(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
                                                                                                                                                                                    
    if (buffer && !strcasecmp(buffer, "on")) {
        SetMechCruise(mech);
        mech_notify(mech, MECHALL, "Cruise Control flipped %ch%cgON%cn.");
        return;
    }
    if (buffer && !strcasecmp(buffer, "off")) {
        UnSetMechCruise(mech);
        mech_notify(mech, MECHALL, "Cruise Control flipped %ch%crOFF%cn.");
        return;
    }
                                                                                                                                                                                    
    mech_notify(mech, MECHPILOT, tprintf("Cruise Control is %%ch%s%%cn",
            MechCruise(mech) ? "%cgON" : "%crOFF"));
    return;
}

void mech_overthrust(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
                                                                                                                                                                                    
    if (buffer && !strcasecmp(buffer, "on")) {
        SetMechOT(mech);
        mech_notify(mech, MECHALL, "Overthrust Control flipped %ch%cgON%cn.");
        return;
    }
    if (buffer && !strcasecmp(buffer, "off")) {
        UnSetMechOT(mech);
        mech_notify(mech, MECHALL, "Overthrust Control flipped %ch%crOFF%cn.");
        return;
    }
                                                                                                                                                                                    
    mech_notify(mech, MECHPILOT, tprintf("Overthrust Control is %%ch%s%%cn",
            MechOT(mech) ? "%cgON" : "%crOFF"));
    return;
}

int IsArtyMech(MECH * mech)
{
    int weapnum, section, critical, weaptype = -2;

    for (weapnum = 0; weaptype != -1; weapnum++) {
	weaptype =
	    FindWeaponNumberOnMech(mech, weapnum, &section, &critical);
	if (IsArtillery(weaptype))
	    return 1;
    }
    return 0;
}

void mech_slwarn(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    if (buffer && !strcasecmp(buffer, "off")) {
        UnSetMechSLWarn(mech);
        mech_notify(mech, MECHALL, "You grow unconcerned with the flashing battlefield lights.");
        return;
    }
    if (buffer && !strcasecmp(buffer, "on")) {
        SetMechSLWarn(mech);
        mech_notify(mech, MECHALL, "You grow steadily more concerned with the battlefield events.");
        return;
    }

	mech_notify(mech, MECHALL, tprintf("Searchlight warnings are : %s",
            MechSLWarn(mech) ? "ON" : "OFF"));
    return;
}

void mech_noradio(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    if (buffer && !strcasecmp(buffer, "off")) {
        UnSetMechNoRadio(mech);
        mech_notify(mech, MECHALL, "You flick ON your Line of Sight comgear.");
        return;
    }
    if (buffer && !strcasecmp(buffer, "on")) {
        SetMechNoRadio(mech);
        mech_notify(mech, MECHALL, "You flick OFF your Line of Sight comgear.");
        return;
    }

	mech_notify(mech, MECHALL, tprintf("Comgear is %s.",
            MechNoRadio(mech) ? "OFF" : "ON"));
    return;
}

void mech_turnmode(dbref player, void *data, char *buffer)
{
	MECH *mech = (MECH *) data;

	if (!GotPilot(mech) || MechPilot(mech) != player) {
		notify(player, "You're not the pilot!");
		return;
	}

	if (!HasBoolAdvantage(player, "maneuvering_ace")) {
		mech_notify(mech, MECHPILOT, "You're not skilled enough to do that.");
		return;
	}

	if (buffer && !strcasecmp(buffer, "tight")) {
		SetTurnMode(mech,1);
		mech_notify(mech, MECHALL, "You brace for tighter turns.");
		return;
	}
	if (buffer && !strcasecmp(buffer, "normal")) {
		SetTurnMode(mech,0);
		mech_notify(mech, MECHALL, "You assume a normal turn mode."); 
		return;
	}
	mech_notify(mech, MECHALL, tprintf("Your turning type is : %s",
		GetTurnMode(mech) ? "TIGHT" : "NORMAL"));
	return;
}

void mech_autofall(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    if (buffer && !strcasecmp(buffer, "off")) {
        UnSetMechAutofall(mech);
        mech_notify(mech, MECHALL, "You decide you'd rather be stable.");
        return;
    }
    if (buffer && !strcasecmp(buffer, "on")) {
        SetMechAutofall(mech);
        mech_notify(mech, MECHALL, "You decide an unstable stance suits you.");
        return;
    }

        mech_notify(mech, MECHALL, tprintf("Mech autofalls are : %s",
            MechAutofall(mech) ? "ON" : "OFF"));
    return;
}

static void mech_check_range(EVENT * e)
{
    MECH *spotter = (MECH *) e->data2, *mech = (MECH *) e->data;
    float range;

    if (!mech)
	return;

    if (MechSpotter(mech) == -1)
	return;

    if (!spotter) {
	mech_notify(mech, MECHALL,
	    "You have lost link with your spotter!");
	MechSpotter(mech) = -1;
	return;
    }
    range = FlMechRange(fl, mech, spotter);
    if (range > 2 * MechRadioRange(spotter) || MechSpotter(spotter) == -1
	|| spotter->mapindex != mech->mapindex) {
	mech_notify(mech, MECHALL,
	    "You have lost link with your spotter!");
	MechSpotter(mech) = -1;
	return;
    }
    MECHEVENT(mech, EVENT_SPOT_CHECK, mech_check_range, SPOT_TICK,
	spotter);
}

static void mech_spot_event(EVENT * e)
{
    MECH *target, *mech = (MECH *) e->data;
    struct spot_data *sd = (struct spot_data *) e->data2;

    target = (MECH *) sd->target;

    if (MechFX(mech) != sd->mechFX && MechFY(mech) != sd->mechFY &&
	MechFX(target) != sd->tarFX && MechFY(target) != sd->tarFY) {
	mech_notify(target, MECHALL,
	    "The data link was not established due to movement!");
	mech_notify(mech, MECHALL,
	    "The data link was not established due to movement!");
	free((void *) e->data2);
	return;
    }
    mech_notify(target, MECHALL, tprintf("Data link established with %s.",
	    GetMechToMechID(target, mech)));
    mech_notify(mech, MECHALL,
	tprintf
	("Data link established with %s, you now have a forward observer.",
     GetMechToMechID(target, mech)));
    MechSpotter(mech) = target->mynum;
    MECHEVENT(mech, EVENT_SPOT_CHECK, mech_check_range, SPOT_TICK, target);
    free((void *) e->data2);
}

void ClearFireAdjustments(MAP * map, dbref mech)
{
    int i;
    MECH *m;

    for (i = 0; i < map->first_free; i++)
	if (map->mechsOnMap[i] >= 0) {
	    if (!(m = getMech(map->mechsOnMap[i])))
		continue;
	    if (m->mynum == mech)
		continue;
	    if (MechSpotter(m) == mech)
		MechFireAdjustment(m) = 0;
	}
}

void mech_spot(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data, *target;
    char *args[5];
    char targetID[3];
    int argc;
/*    int loop = 0, canspot = 0; */
    int LOS = 1;
    dbref targetref;
    float range;
    struct spot_data *dat;
    MAP *mech_map;

    cch(MECH_USUALO);
    mech_map = getMap(mech->mapindex);
    argc = mech_parseattributes(buffer, args, 5);
/*    for (loop = 0; loop < NUM_SECTIONS; loop++) */
/*	{ */
/*        if (FindObj(mech, loop, I2Special(TAG)) > 0) */
/*		{ */
/*		canspot = 1; */
/*		break; */
/*		} */
/*	} */
/*    DOCHECK(canspot != 1, "You may only spot if your equipped with TAG."); */
    DOCHECK(MoveModeLock(mech), "You cannot spot while using a special movement mode.");
    DOCHECK((MechStatus2(mech) & UNJAMMING), "Spotting is not possible while unjamming!");
    DOCHECK(argc != 1, "You may only use mech ID's to set spotter!");
    targetID[0] = args[0][0];
    targetID[1] = args[0][1];
    targetID[2] = 0;
    targetref = FindTargetDBREFFromMapNumber(mech, targetID);
    if (!strcmp(args[0], "-")) {
	if (MechSpotter(mech) == mech->mynum) {
	    mech_notify(mech, MECHALL, "You spot no longer.");
	    ClearFireAdjustments(mech_map, mech->mynum);
	} else
	    mech_notify(mech, MECHALL,
		"You disable the datalink to spotter.");
	MechSpotter(mech) = -1;
	return;
    }
    if (!strcasecmp(targetID, MechIDS(mech, 0))) {
	MechSpotter(mech) = mech->mynum;
	mech_notify(mech, MECHALL, "You are now set as a spotter.");
	return;
    }
    target = getMech(targetref);
    if (target)
	LOS =
     InLineOfSight(mech, target, MechX(target), MechY(target),
	    FlMechRange(mech_map, mech, target));

    DOCHECK((targetref == -1) ||
	MechTeam(target) != MechTeam(mech), "That target does not exist!");

    DOCHECK(MechTeam(target) != MechTeam(mech),
	"The 'mech is not of your own team!");

    DOCHECK(MechSpotter(target) != target->mynum,
	"That 'mech is not set up as spotter!");

    if (IsArtyMech(mech) && !LOS) {
	mech_notify(target, MECHALL,
	    "Someone is trying to establish a data link with you!");
	mech_notify(mech, MECHALL,
	    "You attempt to establish a data link..... please stand by.");
	range = FlMechRange(mech_map, mech, target);
	if (range > 2 * MechRadioRange(target)) {
	    mech_notify(mech, MECHALL,
		"That target is our of data link range!");
	    return;
	}
	Create(dat, struct spot_data, 1);
	dat->mechFY = MechFY(mech);
	dat->mechFX = MechFX(mech);
	dat->tarFX = MechFX(target);
	dat->tarFY = MechFY(target);
	dat->target = (MECH *) target;
	MECHEVENT(mech, EVENT_SPOT_LOCK, mech_spot_event,
	    WEAPON_TICK * ((int) range / 10 + 5), dat);
	return;
    } else
	DOCHECK(!LOS, "You do not have LOS to that target!")
	    MechSpotter(mech) = targetref;
    MechFireAdjustment(mech) = 0;
    mech_notify(mech, MECHALL, tprintf("%s set as spotter.",
	    GetMechToMechID(mech, target)));
}


void mech_target(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *target;
    char *args[5];
    int argc;
    char section[50];
    char type, move, index;

    cch(MECH_USUALO);
    argc = mech_parseattributes(buffer, args, 5);
    DOCHECK(argc != 1, "Invalid number of arguments to function!");
    if (!strcmp(args[0], "-")) {
	MechAim(mech) = NUM_SECTIONS;
	notify(player, "Targetting disabled.");
	return;
    }
    DOCHECK(MechTarget(mech) < 0 ||
	!(target =
FindObjectsData(MechTarget(mech))),
"Error: You need to be locked at something to target it's part!");
    type = MechType(target);
    move = MechMove(target);
    DOCHECK((index =
	    ArmorSectionFromString(type, move, args[0])) < 0,
	"Invalid location!");
    MechAim(mech) = index;
    MechAimType(mech) = type;
    ArmorStringFromIndex(index, section, type, move);
    notify(player, tprintf("%s targetted.", section));
}

/* Varying messages based on the distance to foe, and size of your vehicle
   vs size of the guy targeting you: */

/*-20 (or less), -15 to 15, and 20+ ton difference (targetertons - yourtons)*/

/*Distance: <9, <20, rest */

/* Idea: Tonseverity + 3 * distseverity */
char *ss_messages[] = {
    "You feel you'll have your hands full before too long..",
    "You have a bad feeling about this..",
    "You feel a homicidal maniac is about to pounce you!",

    "You think something is amiss..",
    "You have a slightly bad feeling about this..",
    "You think someone thinks ill of you..",

    "Something makes you somewhat feel uneasy..",
    "Something makes you definitely feel uneasy..",
    "Something makes you feel out of your element.."
};

#define SSDistMod(r) ((r < 9) ? 0 : ((r < 20) ? 1 : 2))
#define SSTonMod(d)  ((d <= -20) ? 0 : (d >= 20) ? 2 : 1)

static void mech_ss_event(EVENT * ev)
{
    MECH *mech = (MECH *) ev->data;
    int i = (int) ev->data2;

    if (Uncon(mech))
	return;
    if (!RGotPilot(mech))
	return;
    mech_notify(mech, MECHPILOT, ss_messages[BOUNDED(0, i, 8)]);
}

void sixth_sense_check(MECH * mech, MECH * target)
{
    float r;
    int d;

    if (!(MechSpecials(target) & SS_ABILITY) ||
	(MechCritStatus(mech) & OBSERVATORIC)) return;
    if (Destroyed(target))
	return;
    if (Roll() > 8)
	return;
    r = FaMechRange(mech, target);
    d = (MechRTons(mech) - MechRTons(target)) / 1024;
    MECHEVENT(target, EVENT_SS, mech_ss_event, Number(1, 3),
	((3 * (SSDistMod(r))) + (SSTonMod(d))));
}

void mech_settarget(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data, *target;
    MAP *mech_map;
    char *args[5];
    char targetID[2];
    int argc;
    int LOS = 1;
    int newx, newy;
    dbref targetref;
    int mode;

    cch(MECH_USUALO);
    argc = mech_parseattributes(buffer, args, 5);
    switch (argc) {
    case 1:
	mech_map = getMap(mech->mapindex);
	if (args[0][0] == '-') {
	    MechTarget(mech) = -1;
	    MechTargX(mech) = -1;
	    MechTargY(mech) = -1;
	    mech_notify(mech, MECHALL, "All locks cleared.");
	    StopLock(mech);
	    if (MechSpotter(mech) == mech->mynum)
		ClearFireAdjustments(mech_map, mech->mynum);
	    return;
	}
	targetID[0] = args[0][0];
	targetID[1] = args[0][1];
	targetref = FindTargetDBREFFromMapNumber(mech, targetID);
	target = getMech(targetref);
	if (target)
	    LOS =
		InLineOfSight(mech, target, MechX(target), MechY(target),
		FlMechRange(mech_map, mech, target));
	else
	    targetref = -1;
	DOCHECK(targetref == -1 ||
	    !LOS, "That is not a valid targetID. Try again.");
/*	DOCHECK(((MechTeam(mech) == MechTeam(target)) && (Started(target)) && (!Destroyed(target))),
		"Friendly fire isn't..., Find another target."); */
	mech_notify(mech, MECHALL, tprintf("Target set to %s.",
		GetMechToMechID(mech, target)));
	StopLock(mech);
	MechTarget(mech) = targetref;
	MechStatus(mech) |= LOCK_TARGET;
	sixth_sense_check(mech, target);
#if LOCK_TICK > 0
	if (!arc_override)
	    MECHEVENT(mech, EVENT_LOCK, mech_lock_event, LOCK_TICK, 0);
#endif
	break;
    case 2:
	/* Targetted a square */
	mech_map = getMap(mech->mapindex);
	newx = atoi(args[0]);
	newy = atoi(args[1]);
	ValidCoord(mech_map, newx, newy);
	MechTarget(mech) = -1;
	MechTargX(mech) = newx;
	MechTargY(mech) = newy;
	MechFireAdjustment(mech) = 0;
	if (MechSpotter(mech) == mech->mynum)
	    ClearFireAdjustments(mech_map, mech->mynum);
	MechTargZ(mech) = Elevation(mech_map, newx, newy);
	notify(player, tprintf("Target coordinates set at (X,Y) %d, %d",
		newx, newy));
	StopLock(mech);
	MechStatus(mech) |= LOCK_TARGET;
#if LOCK_TICK > 0
	if (!arc_override)
	    MECHEVENT(mech, EVENT_LOCK, mech_lock_event, LOCK_TICK, 0);
#endif
	break;
    case 3:
	/* Targetted a square w/ special mode (hex / building) */
	DOCHECK(strlen(args[2]) > 1, "Invalid lock mode!");
	switch (toupper(args[2][0])) {
	case 'B':
	    mode = LOCK_BUILDING;
	    break;
	case 'I':
	    mode = LOCK_HEX_IGN;
	    break;
	case 'C':
	    mode = LOCK_HEX_CLR;
	    break;
	case 'H':
	    mode = LOCK_HEX;
	    break;
	default:
	    notify(player, "Invalid mode selected!");
	    return;
	}
	mech_map = getMap(mech->mapindex);
	newx = atoi(args[0]);
	newy = atoi(args[1]);
	ValidCoord(mech_map, newx, newy);
	MechTarget(mech) = -1;
	MechTargX(mech) = newx;
	MechTargY(mech) = newy;
	MechFireAdjustment(mech) = 0;
	if (MechSpotter(mech) == mech->mynum)
	    ClearFireAdjustments(mech_map, mech->mynum);
	MechTargZ(mech) = Elevation(mech_map, newx, newy);
	switch (mode) {
	case LOCK_HEX:
	    notify(player,
		tprintf("Target coordinates set to hex at (X,Y) %d, %d",
		    newx, newy));
	    break;
	case LOCK_HEX_CLR:
	    notify(player,
		tprintf
		("Target coordinates set to clearing hex at (X,Y) %d, %d",
	     newx, newy));
	    break;
	case LOCK_HEX_IGN:
	    notify(player,
		tprintf
		("Target coordinates set to igniting hex at (X,Y) %d, %d",
	     newx, newy));
	    break;
	default:
	    notify(player,
		tprintf
		("Target coordinates set to building at (X,Y) %d, %d",
	     newx, newy));
	    break;
	}

	StopLock(mech);
	MechStatus(mech) |= mode;
#if LOCK_TICK > 0
	if (!arc_override)
	    MECHEVENT(mech, EVENT_LOCK, mech_lock_event, LOCK_TICK, 0);
#endif
    }
}

void mech_fireweapon(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    char *args[5];
    int argc;
    int weapnum;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    argc = mech_parseattributes(buffer, args, 5);
    DOCHECK(argc < 1, "Not enough arguments to the function");
    weapnum = atoi(args[0]);
    FireWeaponNumber(player, mech, mech_map, weapnum, argc, args, 0);
}

extern int ftflag;

/* Usage:
   mech      = Mech who's looking for people
   mech_map  = Map mech's on
   x,y       = Target hex
   needlos   = Bitvector
   1 = Require LOS
   2 = We actually want a mech that is friendly and has LOS to hex
 */
MECH *find_mech_in_hex(MECH * mech, MAP * mech_map, int x, int y,
    int needlos)
{
    int loop;
    MECH *target;

    for (loop = 0; loop < mech_map->first_free; loop++)
	if (mech_map->mechsOnMap[loop] != mech->mynum &&
	    mech_map->mechsOnMap[loop] != -1) {
	    target = (MECH *)
		FindObjectsData(mech_map->mechsOnMap[loop]);
	    if (!target)
		continue;
	    if (!(MechX(target) == x && MechY(target) == y) &&
		!(needlos & 2)) continue;
	    if (needlos) {
		if (needlos & 1)
		    if (!InLineOfSight(mech, target, x, y,
			    FlMechRange(mech_map, mech, target)))
			continue;
		if (needlos & 2) {
		    if (MechTeam(mech) != MechTeam(target))
			continue;
		    if (!(MechSeesHex(target, mech_map, x, y)))
			continue;
		    if (mech == target)
			continue;
		}
	    }
	    return target;
	}
    return NULL;
}

int FireSpot(dbref player, MECH * mech, MAP * mech_map, int weaponnum,
    int weapontype, int sight, int section, int critical)
{
    /* Nim 9/11/96 */
    /* Modified by DJ after that, tho */

    float spot_range, range;
    float enemyX, enemyY, enemyZ = 0;
    int LOS, mapx = 0, mapy = 0;
    MECH *target = NULL, *spotter;
    int spotTerrain;
    int found_target = 0;
    int IsGuided = 0;
    int ishex;

    /* No spotter or not IDF weapon lets get outta here */
    if (MechSpotter(mech) == -1 ||
	(!(MechWeapons[weapontype].special & IDF)) ||
	(GetPartMode(mech, section, critical) & DEADFIRE_MODE))
	   return 0;
    spotter = getMech(MechSpotter(mech));
    DOCHECKMP1(!spotter, "There is no spotter avilable to IDF with!");

    if (!(MechSpotter(spotter) == spotter->mynum)) {
	mech_notify(mech, MECH_PILOT, "You do not have a spotter!");
	MechSpotter(mech) = -1;
	return 1;
    }
    DOCHECKMP1(Uncon(spotter), "Your spotter is unconscious!");
    DOCHECKMP1(Blinded(spotter), "Your spotter can't see a thing!");
    IsGuided = (HasFuncTAG(spotter) && (GetPartMode(mech, section, critical) & SGUIDED_MODE));
    /* Is the spotter set to a Mech or to a Hex? */
    if (MechTarget(spotter) != -1) {
	target = getMech(MechTarget(spotter));
	DOCHECKMP1(!target, "Your spotter has invalid target!");
	mapx = MechX(target);
	mapy = MechY(target);
	spot_range = FaMechRange(spotter, target);
        IsGuided = (IsGuided && !(MechStatus2(target) & ECM_PROTECTED) && !(mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_ECM) &&
		!(mech_map->LOSinfo[spotter->mapnumber][target->mapnumber] & MECHLOSFLAG_ECM) &&
		!(mech_map->LOSinfo[mech->mapnumber][spotter->mapnumber] & MECHLOSFLAG_ECM) && (spot_range < 15.0));
	LOS = InLineOfSight_NB(spotter, target, mapx, mapy, spot_range);
	DOCHECKMP1(!LOS, "Your spotter does not have a target in LOS!");
	range = FaMechRange(mech, target);
	spotTerrain = (AddTerrainMod(spotter, target, mech_map, spot_range) + (IsGuided ? 0 : AttackMovementMods(spotter)) + ((Locking(spotter) && MechTargComp(spotter) != TARGCOMP_MULTI) ? 2 : 0));
        if (IsGuided) {
	    if (!Started(target) || Uncon(target) || Blinded(target) || MechMove(target) == MOVE_NONE)
        	spotTerrain += -4;
            if (Fallen(target))
        	spotTerrain += (spot_range <= 1.0) ? -2 : 1;
	}
/*	DOCHECK1(IsArtillery(weapontype) && target, "You can only target hexes with this kind of artillery.");  */
	if (!sight && spotter && target)
	    AccumulateSpotXP(MechPilot(spotter), spotter, target);
        if (IsArtillery(weapontype) && target && spotter) {
            if (!sight) {
                AccumulateArtyXP(MechPilot(spotter), spotter, target);
            }
	    enemyX = MechFX(target);
	    enemyY = MechFY(target);
	    enemyZ = MechFZ(target);
	    ishex = 1;
	} else {
            DOCHECK1(IsArtillery(weapontype) && target, "You can only target hexes with this kind of artillery.");
	    enemyX = MechFX(target);
            enemyY = MechFY(target);
            enemyZ = MechFZ(target);
            ishex = 0;
	}
	FireWeapon(mech, mech_map, (ishex ? NULL : target), 0, weapontype, weaponnum,
	    section, critical, enemyX, enemyY, mapx, mapy, range, spotTerrain, sight, ishex, 0);
	return 1;
    }
    if (!(MechTargX(spotter) >= 0 && MechTargY(spotter) >= 0)) {
	notify(player, "Your spotter has no target set!");
	return 1;
    }
    if (!IsArtillery(weapontype))
	if ((target = find_mech_in_hex(mech, mech_map, MechTargX(spotter),
	    MechTargY(spotter), 0))) {
	    enemyX = MechFX(target);
	    enemyY = MechFY(target);
	    enemyZ = MechFZ(target);
	    mapx = MechX(target);
	    mapy = MechY(target);
	    found_target = 1;
	}
    if (!found_target) {
	target = (MECH *) NULL;
	mapx = MechTargX(spotter);
	mapy = MechTargY(spotter);
	enemyZ = ZSCALE * MechTargZ(spotter);
	MapCoordToRealCoord(mapx, mapy, &enemyX, &enemyY);
    }
    spot_range =
	FindRange(MechFX(spotter), MechFY(spotter), MechFZ(spotter),
	enemyX, enemyY, enemyZ);
    LOS = InLineOfSight_NB(spotter, target, mapx, mapy, spot_range);
    DOCHECK0(!LOS, "That target is not in your spotters line of sight!");
    range =
	FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), enemyX, enemyY,
	enemyZ);
    spotTerrain = ((IsGuided ? 0 : AttackMovementMods(spotter)) + ((Locking(spotter) && MechTargComp(spotter) != TARGCOMP_MULTI) ? 2 : 0));
    FireWeapon(mech, mech_map, target, 0, weapontype, weaponnum, section,
	critical, enemyX, enemyY, mapx, mapy, range, spotTerrain, sight,
	2, 0);
    return 1;
}

#define ARCCHECK(mech,ex,ey,ez,sec,crit,msg) \
if (AeroUnusableArcs(mech)) \
{ int ar; ar = InWeaponArc(mech, ex, ey); \
DOCHECK0((!arc_override && (AeroUnusableArcs(mech) & ar)) || \
	(arc_override && !(arc_override & ar)), \
	"That arc's weapons aren't under your control!"); \
}; \
if (!IsArtillery(weapindx)) DOCHECK0(!IsInWeaponArc (mech, ex, ey, ez, sec, crit), msg);

int FireWeaponNumber(dbref player, MECH * mech, MAP * mech_map,
    int weapnum, int argc, char **args, int sight)
{
    int weaptype;
    dbref target;
    char targetID[2];
    int mapx = 0, mapy = 0, LOS = 0;
    MECH *tempMech = NULL;
    MECH *tempMech2 = NULL;
    int section, critical;
    float range = 0;
    float enemyX, enemyY, enemyZ;
    int ishex = 0;
    int weapindx = FindWeaponIndex(mech, weapnum);

    DOCHECK0(MoveModeLock(mech), "You cannot fire while using a special movement mode.");
    DOCHECK0(MechSpotter(mech) > 0 && MechSpotter(mech) == mech->mynum, "You cannot fire while spotting.");
    DOCHECK0(Digging(mech), "You are too busy diggin in!");
    if (!sight && (MechCritStatus(mech) & HIDDEN)) {
	mech_notify(mech, MECHALL,
	    "You break out of your cover to initiate weapons fire!");
	MechLOSBroadcast(mech, "breaks out of its cover and begins firing rabidly!");
	MechCritStatus(mech) &= ~HIDDEN;
    }
    if (!sight)
	StopHiding(mech);

    DOCHECK0(weapnum < 0,
	"The weapons system chirps: 'Illegal Weapon Number!'");
    weaptype =
	FindWeaponNumberOnMech_Advanced(mech, weapnum, &section, &critical,
	sight);
    DOCHECK0(weaptype == -1,
	"The weapons system chirps: 'Illegal Weapon Number!'");
    if (!sight) {
	DOCHECK0((MechStatus2(mech) & UNJAMMING),
	    "You are currently unjamming a jammed weapon. No other actions may be taken.");
	DOCHECK0((MechCritStatus(mech) & CREW_STUNNED),
	    "You cannot take actions while stunned! That include finding the trigger.");
	DOCHECK0(weaptype == -3,
	    "The weapon system chirps: 'That weapon is still reloading!'");
	DOCHECK0(weaptype == -4,
	    "The weapon system chirps: 'That weapon is still recharging!'");
        DOCHECK0(weaptype == -6,
	    "The weapon system chrips: 'That weapon is currently jammed!'");
	DOCHECK0((IsWater(MechRTerrain(mech)) && (MechZ(mech) <= ((WaterBeast(mech) || Fallen(mech)) ? -1 : -2)) &&
	    (MechWeapons[weapindx].type & (TMISSILE | TARTILLERY | TAMMO))),
	    "That weapon system will not function underwater!");
	DOCHECK0(((InWater(mech) && MechZ(mech) == -1) && (MechType(mech) == CLASS_MECH || MechMove(mech) == MOVE_QUAD)
	    && (MechMove(mech) == MOVE_QUAD ? (section == RLEG || section == LLEG || section == RARM || section == LARM)
            : (section == LLEG || section == RLEG))), "That weapon system is currently submerged!");

	if (MechType(mech) == CLASS_BSUIT) {
	    DOCHECK0(weaptype == -5,
		tprintf("%s member holding the weapon is still recovering from an attack!", bsuit_name(mech)));
	} else
	    DOCHECK0(weaptype == -5,
		"The limb holding that weapon is still retracting from an attack!");

	if (Fallen(mech) && MechType(mech) == CLASS_MECH) {
	    DOCHECK0(section == RLEG || section == LLEG, "You cannot fire leg mounted weapons when prone.");
	    switch (section) {
	    case RARM:
		DOCHECK0(SectHasBusyWeap(mech, LARM) ||
		    MechSections(mech)[LARM].recycle ||
		    SectIsDestroyed(mech, LARM),
		    "You currently can't use your Left Arm to prop yourself up.");
		break;
	    case LARM:
		DOCHECK0(SectHasBusyWeap(mech, RARM) ||
		    MechSections(mech)[RARM].recycle ||
		    SectIsDestroyed(mech, RARM),
		    "Your currently can't use your Right Arm to prop yourself up.");
		break;
	    default:
		DOCHECK0((SectHasBusyWeap(mech, RARM) ||
			MechSections(mech)[RARM].recycle ||
			SectIsDestroyed(mech, RARM))
		    && (SectHasBusyWeap(mech, LARM) ||
			MechSections(mech)[LARM].recycle ||
			SectIsDestroyed(mech, LARM)),
		    "You currently don't have any arms to spare to prop yourself up.");
	    }
	}
    }
    DOCHECK0((MechCritStatus(mech) & DUG_IN) && section != TURRET && !(MechMove(mech) == MOVE_QUAD),
	"Only turret weapons are available while in cover.");
    DOCHECK0(IsAMS(weaptype), "That weapon is defensive only!");
    DOCHECK0(weaptype == -2,
    	"*CLICK*  Nothing happens.");
    DOCHECK0(argc > 3, "Invalid number of arguments!");
    if ((MechWeapons[weaptype].special & IDF) && MechSpotter(mech) != -1 &&
	MechTarget(mech) == -1 && MechTargY(mech) == -1 &&
	MechTargX(mech) == -1 && (!(GetPartMode(mech, section, critical) & DEADFIRE_MODE)))
	if (FireSpot(player, mech, mech_map, weapnum, weaptype, sight,
		section, critical))
	    return 1;
    switch (argc) {
    case 1:
	if (IsCoolant(weapindx) && GetPartMode(mech, section, critical) & HEAT_MODE) {
	    tempMech = mech;
	    mapx = MechX(tempMech);
	    mapy = MechY(tempMech);
	    range = 0.2;
	    LOS = 1;
	} else {
	    DOCHECK0(!FindTargetXY(mech, &enemyX, &enemyY, &enemyZ), "You do not have a default target set!");
    	    ARCCHECK(mech, enemyX, enemyY, enemyZ, section, critical, "Default target is not in your weapons arc!"); 
	    if (MechTarget(mech) != -1) {
	        tempMech = getMech(MechTarget(mech));
		DOCHECK0(!tempMech, "Error in FireWeaponNumber routine");
		DOCHECK0(!IsCoolant(weapindx) && MechTeam(mech) == MechTeam(tempMech) && Started(tempMech) && !Destroyed(tempMech), "Friendly Fire ?, Dont Think so.");
		mapx = MechX(tempMech);
		mapy = MechY(tempMech);
		range = FaMechRange(mech, tempMech);
		LOS = InLineOfSight_NB(mech, tempMech, mapx, mapy, range);
/*	    if ((!(MechWeapons[weaptype].special & IDF)) || (GetPartMode(mech, section, critical) & DEADFIRE_MODE))*/
		DOCHECK0(!LOS, "That target is not in your line of sight!");
	    } else
	    /* default target is a hex */
	    {
	        ishex = 1;
#if 0
	        if (!IsArtillery(weaptype) && MechLockFire(mech))
		    /* look for enemies in the default hex cause they may have moved */
		    if ((tempMech = find_mech_in_hex(mech, mech_map, MechTargX(mech), MechTargY(mech), 0))) {
		        enemyX = MechFX(tempMech);
		        enemyY = MechFY(tempMech);
		        enemyZ = MechFZ(tempMech);
		        mapx = MechX(tempMech);
		        mapy = MechY(tempMech);
		    }
#endif
	        if (!tempMech) {
		    mapx = MechTargX(mech);
		    mapy = MechTargY(mech);
	 	    MechTargZ(mech) = Elevation(mech_map, mapx, mapy);
		    enemyZ = ZSCALE * MechTargZ(mech);
		    MapCoordToRealCoord(mapx, mapy, &enemyX, &enemyY);
	        }
    /* don't check LOS for missile weapons firing at hex number */
	        range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), enemyX, enemyY, enemyZ);
	        LOS = InLineOfSight_NB(mech, tempMech, mapx, mapy, range);
/*	    if (((MechWeapons[weaptype].special & IDF) ? (GetPartMode(mech, section, critical) & DEADFIRE_MODE) : 1) && !IsArtillery(weaptype))*/
	        if (!IsArtillery(weaptype)) {
		    if ((sensors[(int) MechSensor(mech)[0]].sensorname[0] == 'S') || (sensors[(int) MechSensor(mech)[1]].sensorname[0] == 'S'))
		        if (CalculateLOSFlag(mech, tempMech, mech_map, mapx, mapy, 0, range, 0) & MECHLOSFLAG_BLOCK)
			    LOS = 0;
		    DOCHECK0((!LOS), "That hex target is not in your line of sight!");
		    }
	    }
	}
	break;
    case 2:
	/* Fire at the numbered target */
	targetID[0] = args[1][0];
	targetID[1] = args[1][1];
	target = FindTargetDBREFFromMapNumber(mech, targetID);
	DOCHECK0(target == -1, "That is not a valid target ID!");
	tempMech = getMech(target);
	DOCHECK0(!tempMech, "Error in FireWeaponNumber routine!");
	DOCHECK0(!IsCoolant(weapindx) && MechTeam(mech) == MechTeam(tempMech) && Started(tempMech) && !Destroyed(tempMech), "Friendly Fire ?, Dont Think so.");
	enemyX = MechFX(tempMech);
	enemyY = MechFY(tempMech);
	enemyZ = MechFZ(tempMech);
	mapx = MechX(tempMech);
	mapy = MechY(tempMech);
	ARCCHECK(mech, enemyX, enemyY, enemyZ, section, critical,
	    "That target is not in your weapons arc!");
	range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), enemyX, enemyY, enemyZ);
	LOS = InLineOfSight_NB(mech, tempMech, MechX(tempMech), MechY(tempMech), range);
	if (!IsArtillery(weaptype))
	    DOCHECK0(!LOS, "That target is not in your line of sight!");
	break;
    case 3:
	/* Fire at the Map X Y */
	mapx = atoi(args[1]);
	mapy = atoi(args[2]);
	ishex = 1;
	DOCHECK0(mapx < 0 || mapx >= mech_map->map_width || mapy < 0 ||
	    mapy >= mech_map->map_height, "Map coordinates out of range!");
#if 0
	if (!IsArtillery(weaptype))
	    /* look for enemies in that hex... */
	    if ((tempMech = find_mech_in_hex(mech, mech_map, MechTargX(mech),
		MechTargY(mech), 0))) {
		enemyX = MechFX(tempMech);
		enemyY = MechFY(tempMech);
		enemyZ = MechFZ(tempMech);
	    }
#endif
	if (!tempMech) {
	    MapCoordToRealCoord(mapx, mapy, &enemyX, &enemyY);
	    MechTargZ(mech) = Elevation(mech_map, mapx, mapy);
	    enemyZ = ZSCALE * MechTargZ(mech);
	}
	ARCCHECK(mech, enemyX, enemyY, enemyZ, section, critical,
	    "That hex target is not in your weapons arc!");
	/* Don't check LOS for missile weapons */
	range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), enemyX, enemyY, enemyZ);
	LOS = InLineOfSight_NB(mech, tempMech, mapx, mapy, range);
	DOCHECK0(!LOS && MapIsUnderground(mech_map),
 	    "You cannot shoot what you cannot see down here.");
        /*if (((MechWeapons[weaptype].special & IDF) ? (GetPartMode(mech, section, critical) & DEADFIRE_MODE) : 1) && !IsArtillery(weaptype))*/
        if (!IsArtillery(weaptype)) {
            if ((sensors[(int) MechSensor(mech)[0]].sensorname[0] == 'S') || (sensors[(int) MechSensor(mech)[1]].sensorname[0] == 'S'))
                if (CalculateLOSFlag(mech, tempMech, mech_map, mapx, mapy, 0, range, 0) & MECHLOSFLAG_BLOCK)
                    LOS = 0;
	    DOCHECK0((!LOS), "That hex target is not in your line of sight!");
	    }
    }
/*    DOCHECK0(IsArtillery(weaptype) && tempMech, "You can only target hexes with this kind of artillery."); */
    if (IsArtillery(weaptype) && tempMech) {
	enemyX = MechFX(tempMech);
	enemyY = MechFY(tempMech);
	enemyZ = MechFZ(tempMech);
	mapx = MechX(tempMech);
	mapy = MechY(tempMech);
	ishex = 1;
	tempMech = NULL;
    }
    if (tempMech) {
	DOCHECK0(MechSwarmTarget(tempMech) == mech->mynum,
	    "You are unable to use your weapons against a swarmer!");
	DOCHECK0((MechWeapons[weapindx].type == TMISSILE && MechType(mech) == CLASS_BSUIT &&
		MechSwarmTarget(mech) == tempMech->mynum), "You cannot fire missile weapons at your swarm target!");
        if ((MechType(mech) == CLASS_BSUIT) && MechSwarmTarget(mech))
	    if ((tempMech2 = getMech(MechSwarmTarget(mech)))) {
        	DOCHECK0(MechTeam(mech) == MechTeam(tempMech2), "You cannot fire weapons while mounting!");
		DOCHECK0(MechSwarmTarget(mech) != tempMech->mynum, "You can only fire at your swarm target!");
	    }
	DOCHECK0(MechType(tempMech) == CLASS_MW &&
	    MechType(mech) != CLASS_MW &&
	    !MechPKiller(mech),
	    "That's a living, breathing person! Switch off the safety first, if you really want to assassinate the target.");
	DOCHECK0(MechType(tempMech) == CLASS_BSUIT && MechSwarmTarget(tempMech) > 0, "The target is too dispersed and deflected to fire upon!");
    }
    DOCHECK0(MechIsSwarmed(mech) && (section == RTORSO || section == LTORSO || section == CTORSO),
	"You cannot fire torso weapons while mounted!");
    DOCHECK0(MechType(mech) == CLASS_BSUIT && ishex && MechSwarmTarget(mech) > 0,
	"You cannot fire at a hex while mounted!");
    DOCHECK0((!LOS || IsArtillery(weaptype)) && MapIsUnderground(mech_map),
	"The narrow confines prevent performing that action.");
	if (MechType(mech) == CLASS_BSUIT && tempMech &&
	 MechSwarmTarget(mech) == tempMech->mynum && !sight) {
		int swnum = 0, swfire = 0;
		int swindex, hitloc;
		int swrear = 0, swcritical = 0;
		int swcrits = 0;
		int ogint;

		hitloc = FindSwarmHitLocation(&swrear, &swcritical) +1;
/*		hitloc = FindTargetHitLoc(mech, tempMech, &swrear, &swcritical) + 1;*/
		swindex = FindWeaponNumberOnMech(mech, swnum, &section, &critical);
		ogint = GetSectInt(tempMech, hitloc - 1);

		while (swindex != -1) {
			if (swindex > -1 && !IsMissile(swindex)) {
				FireWeapon(mech, mech_map, tempMech, LOS, swindex, swnum, section, critical,
				 enemyX, enemyY, mapx, mapy, range, 1000, sight, ishex, swrear ? -hitloc : hitloc);
			swfire++;
			}
			do {
				swnum++;
				swindex = FindWeaponNumberOnMech(mech, swnum, &section, &critical);
				if (IsMissile(swindex))
					swindex = -2;
			} while (swindex < -1);
		}

/*		if (swindex > -1 && !IsMissile(swindex)) {
			hitloc += 10;
			FireWeapon(mech, mech_map, tempMech, LOS, swindex, swnum, section, critical,
			 enemyX, enemyY, mapx, mapy, range, 1000, sight, ishex, swrear ? -hitloc : hitloc);
		swfire++;
		}*/
		if (swfire == 0)
			mech_notify(mech, MECHALL, "You have no weapons that can fire right now!");
		else {
			if (ogint > GetSectInt(tempMech, hitloc - 1))
				cause_internaldamage(tempMech, mech, LOS, MechPilot(mech), swrear, hitloc - 1, 0, 0, &swcrits, 0, 1);
			cause_armordamage(tempMech, mech, LOS, MechPilot(mech), swrear, 1, hitloc - 1, 0, &swcrits, 0, 1);
			if (MechType(tempMech) == CLASS_MECH && (hitloc - 1) == HEAD
			 && !(MechSpecials2(tempMech) & TORSOCOCKPIT_TECH))
			 	headhitmwdamage(tempMech, 1);
		}
	} else
    FireWeapon(mech, mech_map, tempMech, LOS, weaptype, weapnum, section,
	critical, enemyX, enemyY, mapx, mapy, range, 1000, sight, ishex, 0);
    return (1);
}

void ammo_expedinture_check(MECH * mech, int weapindx, int ns);

int Dump_Decrease(MECH * mech, int loc, int pos, int *hm)
{
    int c, index, weapindx, rem;

#define RUP(a) { if (*hm < a) *hm = a; return a; }
    /* It _is_ ammo, and contains something */

    if (IsAmmo((index = GetPartType(mech, loc, pos))))
	if (!PartIsDestroyed(mech, loc, pos))
	    if ((c = GetPartData(mech, loc, pos))) {
		weapindx = Ammo2WeaponI(index);
		if (MechWeapons[weapindx].ammoperton < DUMP_SPEED) {
		    if ((event_tick % (DUMP_SPEED /
				MechWeapons[weapindx].ammoperton))) RUP(2);
		    /* fine, we remove 1 */
		    rem = 1;
		} else
		    rem =
			MIN(c,
			MechWeapons[weapindx].ammoperton / DUMP_SPEED);
		ammo_expedinture_check(mech, weapindx, rem - 1);
		SetPartData(mech, loc, pos, c - rem);
		if (c <= rem)
		    RUP(1);
		RUP(2);
	    }
    return 0;
}


static void mech_dump_event(EVENT * ev)
{
    MECH *mech = (MECH *) ev->data;
    int arg = (int) ev->data2;
    int loc;
    int i, l;
    int c = -1, d, e = 0;
    char buf[SBUF_SIZE];
    int weapindx;

    if (!Started(mech))
	return;
    i = MechType(mech) == CLASS_MECH ? 7 : 5;
    /* Global ammo droppage */
    if (!arg) {
	for (; i >= 0; i--)
	    for (l = CritsInLoc(mech, i) - 1; l >= 0; l--)
		if (IsAmmo(GetPartType(mech, i, l)))
		    if (GetPartData(mech, i, l))
			c += Dump_Decrease(mech, i, l, &e);
	if (e > 1)
	    MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK,
		arg);
	else
	    mech_notify(mech, MECHALL, "All ammunition dumped.");
	return;
    }
/* Weapontype based dump - DJ (Too many comments bad. Too few bad. Sheesh.  */
    if (arg < 256) {
	loc = arg - 1;
	l = CritsInLoc(mech, loc);
	for (i = 0; i < l; i++)
	    if (IsAmmo(GetPartType(mech, loc, i)))
		if (!PartIsDestroyed(mech, loc, i))
		    if ((d = GetPartData(mech, loc, i)))
			c += Dump_Decrease(mech, loc, i, &e);
	if (e > 1)
	    MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK,
		arg);
	else if (e == 1 && Started(mech)) {
	    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
	    mech_notify(mech, MECHALL,
		tprintf("All ammunition in %s dumped.", buf));
	}
	return;
    }
/* Location based */
    if (arg < 65536) {
	weapindx = arg / 256;
	for (; i >= 0; i--)
	    for (l = CritsInLoc(mech, i) - 1; l >= 0; l--)
		if (IsAmmo(GetPartType(mech, i, l)))
		    if (Ammo2WeaponI(GetPartType(mech, i, l)) == weapindx)
			if (GetPartData(mech, i, l))
			    c += Dump_Decrease(mech, i, l, &e);
	if (e > 1)
	    MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK,
		arg);
	else
	    mech_notify(mech, MECHALL, tprintf("Ammunition for %s dumped!",
		    get_parts_long_name(I2Weapon(weapindx))));
	return;
    }
    l = (arg >> 16) & 0xFF;
    i = (arg >> 24) & 0xFF;
    e = 0;
    if (GetPartData(mech, l, i))
	c += Dump_Decrease(mech, l, i, &e);
    if (e > 1) {
	MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK, arg);
    } else {
	ArmorStringFromIndex(l, buf, MechType(mech), MechMove(mech));
	mech_notify(mech, MECHALL,
	    tprintf("Ammunition in %s crit %i dumped!", buf, i + 1));
    }
}

void mech_dump(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int argc;
    char *args[2];
    int weapnum;
    int weapindx;
    int section;
    int critical;
    int ammoLoc;
    int ammoCrit;
    int loc;
    int i, l, count = 0, d;
    char buf[MBUF_SIZE];
    int type = 0;

    cch(MECH_USUAL);
    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc < 1, "Not enough arguments to the function");
    weapnum = atoi(args[0]);
    DOCHECKMA(IsRunning(MechDesiredSpeed(mech), MMaxSpeed(mech)) || Running(mech), "You cannot dump while running!");
    DOCHECKMA(Jumping(mech), "You cannot dump while jumping!");
    if (!strcasecmp(args[0], "stop")) {
	DOCHECKMA(!Dumping(mech), "You aren't dumping anything!");
	mech_notify(mech, MECHALL, "Ammo dumping halted.");
	StopDump(mech);
	return;
    } else if (!strcasecmp(args[0], "all")) {
	count = 0;
	i = MechType(mech) == CLASS_MECH ? 7 : 5;
	for (; i >= 0; i--)
	    for (l = CritsInLoc(mech, i) - 1; l >= 0; l--)
		if (IsAmmo(GetPartType(mech, i, l)))
		    if (GetPartData(mech, i, l))
			count++;
	DOCHECKMA(!count, "You have no ammo to dump!");
	DOCHECKMA(Dumping_Type(mech, 0),
	    "You're already dumping your ammo!");
	StopDump(mech);
	mech_notify(mech, MECHALL, "Starting dumping of all ammunition..");
	MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK, 0);
	return;
    } else if (!weapnum && strcmp(args[0], "0")) {
	/* Try to find hitloc instead */
	DOCHECKMA(Dumping(mech), "You're already dumping some ammo!");
	loc =
	    ArmorSectionFromString(MechType(mech), MechMove(mech),
	    args[0]);
	DOCHECK(loc < 0, "Invalid location or weapon number!");
	ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
	if (args[1]) {
	    i = atoi(args[1]);
	    i--;
	    if (i >= 0 && i < 12) {
		if (IsAmmo(GetPartType(mech, loc, i)))
		    if (!PartIsDestroyed(mech, loc, i))
			if ((d = GetPartData(mech, loc, i)))
			    count++;
		DOCHECKMA(!count,
		    tprintf("There is no ammunition in %s crit %i!", buf,
			i + 1));
		type = ((i << 8) | loc);
		DOCHECKMA(type & ~0xFFFF,
		    "Internal inconsistency, dump failed!");
		type = type << 16;
		mech_notify(mech, MECHALL,
		    tprintf
		    ("Starting dumping of ammunition in %s crit %i..", buf,
			i + 1));
		MECHEVENT(mech, EVENT_DUMP, mech_dump_event,
		    DUMP_GRAD_TICK, type);
		return;
	    }
	}
	l = CritsInLoc(mech, loc);
	for (i = 0; i < l; i++)
	    if (IsAmmo(GetPartType(mech, loc, i)))
		if (!PartIsDestroyed(mech, loc, i))
		    if ((d = GetPartData(mech, loc, i)))
			count++;
	DOCHECKMA(!count, tprintf("There is no ammunition in %s!", buf));
	type = loc + 1;
	mech_notify(mech, MECHALL,
	    tprintf("Starting dumping of ammunition in %s..", buf));
	MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK, type);
	return;
    }
    weapindx = FindWeaponIndex(mech, weapnum);
    if (weapnum < 0)
	SendError(tprintf
	    ("CHEATER: #%d tried to crash mux with command 'dump %d'!",
		(int) player, weapnum));
    DOCHECKMA(Dumping(mech), "You're already dumping some ammo!");
    DOCHECK(weapindx < 0, "Invalid weapon number!");
    FindWeaponNumberOnMech(mech, weapnum, &section, &critical);
    DOCHECK(MechWeapons[weapindx].type == TBEAM ||
	MechWeapons[weapindx].type == THAND,
	"That weapon doesn't use ammunition!");
    DOCHECK(!FindAmmoForWeapon_sub(mech, weapindx, 0, &ammoLoc, &ammoCrit,
	    0, 0),
	"You don't have any ammunition for that weapon stored on this mech!");
    DOCHECK(GetPartData(mech, ammoLoc, ammoCrit) == 0,
	"You are out of ammunition for that weapon already!");
    type = 256 * weapindx;
    mech_notify(mech, MECHALL, tprintf("Starting dumping %s ammunition..",
	    get_parts_long_name(I2Weapon(weapindx))));
    MECHEVENT(mech, EVENT_DUMP, mech_dump_event, DUMP_GRAD_TICK, type);
#if 0
    while (FindAmmoForWeapon(mech, weapindx, 0, &ammoLoc, &ammoCrit))
	GetPartData(mech, ammoLoc, ammoCrit) = 0;
    mech_notify(mech, MECHALL, tprintf("Ammunition for %s dumped!",
	    get_parts_long_name(I2Weapon(weapindx))));
#endif
}

int AttackMovementMods(MECH * mech)
{
    float maxspeed;
    float speed;
    int base = 0;
    MAP *map;

    if (is_aero(mech) && !Landed(mech)) {
	if (MechThrust(mech) > WalkingSpeed(MMaxSpeed(mech)))
	    return 2;
	else
	    return 0;
	}

    if (MechType(mech) == CLASS_BSUIT)
	return 0;

    maxspeed = MechMaxSpeed(mech);
    if (InSpecial(mech) && InGravity(mech))
	if ((map = FindObjectsData(mech->mapindex)))
	    maxspeed = maxspeed * 100 / MAX(50, MapGravity(map));

    if ((MechHeat(mech) >= 9.) &&
	(MechSpecials(mech) & TRIPLE_MYOMER_TECH))
		maxspeed += 1.5 * MP1;
    if (MechSpecials(mech) & HARDA_TECH)
	maxspeed = (maxspeed <= MP1 ? maxspeed : maxspeed - MP1);
    if (Jumping(mech))
	return 3;
    if ((Fallen(mech) && MechType(mech) == CLASS_MECH) || (!Jumping(mech) && Stabilizing(mech))) {
	if (MechMove(mech) == MOVE_QUAD && !(MechCritStatus(mech) & LEG_DESTROYED) && !Jumping(mech)) {
	    return 0;
	} else {
	    return 2;
	}
    }
    if (fabs(MechSpeed(mech)) > fabs(MechDesiredSpeed(mech)))
	speed = MechSpeed(mech);
    else
	speed = MechDesiredSpeed(mech);
    if (mudconf.btech_fasaturn)
	if (MechFacing(mech) != MechDesiredFacing(mech))
	    base++;
    if (!(fabs(speed) > 0.0))
	return base + 0;
    if (IsRunning(speed, maxspeed))
	return 2;
    return base + 1;
}

int TargetMovementMods(MECH * mech, MECH * target, float range)
{
    float target_speed;
    int returnValue = 0;
    int i, hitgrp;
    float m = 1.0;
    MAP *map = FindObjectsData(target->mapindex);

    if (is_aero(target) && !Landed(target)) {
	hitgrp = FindAreaHitGroup(mech, target);
	switch (hitgrp) {
	    case FRONT:
		returnValue = 1;
		break;
	    case BACK:
		returnValue = 0;
		break;
	    case LEFTSIDE:
	    case RIGHTSIDE:
		returnValue = 2;
		break;
	    default:
		returnValue = 22;
		break;
	    }
	if (fabsf(MechVelocity(target)) <= MP1)
	    returnValue -= 2;
    } else {
	if (Jumping(target))
	    target_speed = JumpSpeed(target, map);
	else
	    target_speed = fabs(MechSpeed(target));

	if (target_speed <= MP2) {
	    /* Mech moved 0-2 hexes */
	    returnValue = 0;
	} else if (target_speed <= MP4) {
	    /* Mech moved 3-4 hexes */
	    returnValue = 1;
	} else if (target_speed <= MP6) {
	    /* Mech moved 5-6 hexes */
	    returnValue = 2;
	} else if (target_speed <= MP9) {
	    /* Mech moved 7-9 hexes */
	    returnValue = 3;
	} else if (target_speed <= MP13 || !mudconf.btech_exttargmod) {
	    returnValue = 4;
	} else if (target_speed <= MP18) {
	    returnValue = 5;
	} else if (target_speed <= MP24) {
	    returnValue = 6;
	} else {
	    /* Moving more than 24 hexes */
	    returnValue = 7;
	    i = MP24;
	    while (((i * 2) <= target_speed)) {
		i = i * 2;
		returnValue++;
		}
	}
    }
    if (!Started(target) || Uncon(target) || Blinded(target) || MechMove(target) == MOVE_NONE)
	returnValue += -4;
    if (!is_aero(mech) && Fallen(target))
	returnValue += (range <= 1.0) ? -2 : 1;
    if (!is_aero(target) && Jumping(target) && MechType(target) != CLASS_BSUIT)
	returnValue++;
    return (returnValue);
}

static void mech_ammowarn_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int data = (int) e->data2;
    int sev = data / 65536;
    int weapindx = data % 65536;

    if (!Started(mech))
	return;
    if (weapindx < 0)
	return;
    mech_notify(mech, MECHALL,
	tprintf("%sWARNING: Ammo for %s is running low.%%c",
	    sev ? "%ch%cr" : "%ch%cy",
	    get_parts_long_name(I2Weapon(weapindx))));
}

void ammo_expedinture_check(MECH * mech, int weapindx, int ns)
{
    int targ = I2Ammo(weapindx);
    int cnt = 0, slots = 0;
    int t, t2;
    int i, j, cl;
    int sev = 0;

    SetWCheck(mech);
    for (i = 0; i < NUM_CRITICALS; i++) {
	cl = CritsInLoc(mech, i);
	for (j = 0; j < cl; j++)
	    if (GetPartType(mech, i, j) == targ) {
		cnt += GetPartData(mech, i, j);
		slots += AmmoMod(mech, i, j);
	    }
    }
    t = BOUNDED(3, (slots * MechWeapons[weapindx].ammoperton) / 8, 30);
    t2 = 2 * t;
    if ((cnt == (t + ns)) || (ns && cnt >= t && cnt < (t + ns)))
	sev = 1;
    else if ((cnt == (t2 + ns)) || (ns && cnt >= t2 && cnt < (t2 + ns)))
	sev = 0;
    else
	return;
    /* Okay, we have case of warning here */
    MECHEVENT(mech, EVENT_AMMOWARN, mech_ammowarn_event, 1,
	(sev * 65536 + weapindx));
}

int FindAndCheckAmmo(MECH * mech, int weapindx, int section, int critical,
    int *ammoLoc, int *ammoCrit)
{
    int mod, nmod = 0;
    int mode = GetPartMode(mech, section, critical);
    int desired, count = 0, i, ii;

    dbref player = GunPilot(mech);

    if (MechWeapons[weapindx].type == TBEAM ||
	MechWeapons[weapindx].type == THAND)
	return 1;
    if (mode & OS_MODE) {
	DOCHECK0(mode & OS_USED,
	    "That weapon has already been used!");
	return 1;
    }
    if (!(mod = (mode & SPECIFIC_AMMO)) || (mode & ULTRA_MODES)) {
        nmod = (~mod) & SPECIFIC_AMMO;
        if (mode & (PRECISION_MODE|PIERCE_MODE|TRACER_MODE|CASELESS_MODE)) {
	    DOCHECK0(!FindAmmoForWeapon_sub(mech, weapindx, section, ammoLoc,
		    ammoCrit, nmod, mod), "You don't have any ammo for that weapon stored on this mech!");
	} else {
	    DOCHECK0(!FindAmmoForWeapon(mech, weapindx, section, ammoLoc,
		    ammoCrit), "You don't have any ammo for that weapon stored on this mech!");
	}
	DOCHECK0(!GetPartData(mech, *ammoLoc, *ammoCrit),
	    "You are out of ammo for that weapon!");
        if (mode & (ULTRA_MODES))
	    {
	    desired = I2Ammo(weapindx);
	    for (i = 0; i < NUM_SECTIONS; i++)
		for (ii = 0; ii < CritsInLoc(mech, i); ii++)
		    {
		    if (!PartIsNonfunctional(mech, i, ii) &&
			 GetPartType(mech, i, ii) == desired &&
		       ((GetPartMode(mech, i, ii) & SPECIFIC_AMMO) ==
		        (GetPartMode(mech, *ammoLoc, *ammoCrit) & SPECIFIC_AMMO)))
				count += GetPartData(mech, i, ii);
		    }
	/*    SendDebug(tprintf("Ammo Count : Mech #%d - WeapSect %d - WeapCrit %d - Ammo %d",
		mech->mynum, section, critical, count)); */
	    if ((GetPartMode(mech, section, critical) & ROTSIX_MODE) && count < 6) {
		GetPartMode(mech, section, critical) &= ~ROTSIX_MODE;
		GetPartMode(mech, section, critical) |= ROTFOUR_MODE;
		}
	    if ((GetPartMode(mech, section, critical) & ROTFOUR_MODE) && count < 4) {
		GetPartMode(mech, section, critical) &= ~ROTFOUR_MODE;
		GetPartMode(mech, section, critical) |= ULTRA_MODE;
		}
	    if ((GetPartMode(mech, section, critical) & ULTRA_MODE) && count < 2) {
		GetPartMode(mech, section, critical) &= ~ULTRA_MODE;
		}
/* 	    if (mode & ULTRA_MODE)
	        {
	        GetPartData(mech, *ammoLoc, *ammoCrit)--;
	        FindAmmoForWeapon(mech, weapindx, section, ammoLoc,
		    ammoCrit);
	        if (!GetPartData(mech, *ammoLoc, *ammoCrit))
		    GetPartMode(mech, section, critical) &= ~ULTRA_MODE;
	        GetPartData(mech, *ammoLoc, *ammoCrit)++;
	        } */

            }
    } else {
	mod = (mode & SPECIFIC_AMMO);
	if (IsArtillery(weapindx)) {
	    nmod = 0;
	    nmod |= (ARTILLERY_MODES & ~mod);
	} else {
	    nmod = 0;
	    nmod |= (SPECIFIC_AMMO & ~mod);
	}
/*	if (mode & OS_MODE) {
	    DOCHECK0(mode & OS_USED,
		"That weapon has already been used!");
	    GetPartMode(mech, section, critical) |= OS_USED;
	    return 1;
	} */
	DOCHECK0(!FindAmmoForWeapon_sub(mech, weapindx, section, ammoLoc,
		ammoCrit, nmod, mod),
	    "You don't have any ammo for that weapon's mode stored on this mech!");
	DOCHECK0(!GetPartData(mech, *ammoLoc, *ammoCrit),
	    "You are out of the special ammo type for that weapon!");
    }
    return 1;
}

void Missile_Hit(MECH * mech, MECH * target, int isrear, int iscritical,
    int weapindx, int num_missiles_hit, int damage, int salvo_size,
    int LOS, int bth, int mode, int real, int realroll)
{
    int this_time;
    int hitloc;

    if (!is_aero(mech) && MechWeapons[weapindx].special & ELRM)
        if (!is_aero(mech) && FaMechRange(mech, target) < GunStat(weapindx, mode, GUN_MINRANGE))
	    num_missiles_hit = num_missiles_hit / 2;
    if (!is_aero(mech) && MechWeapons[weapindx].type == TMISSILE && strstr(MechWeapons[weapindx].name, "Thunderbolt") && !(mode & HOTLOAD_MODE))
        if (FaMechRange(mech, target) < GunStat(weapindx, mode, GUN_MINRANGE))
            damage = damage / 2;
    if (mudconf.btech_glance && bth == realroll && strstr(MechWeapons[weapindx].name, "Thunderbolt"))
        damage = (damage + 1) / 2;
    if (salvo_size == 1 || !real)
      while (num_missiles_hit) {
	hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
	this_time = MIN(salvo_size, num_missiles_hit);
	DamageMech(target, mech, LOS, GunPilot(mech), hitloc, isrear,
	    iscritical, pc_to_dam_conversion(target, weapindx,
		this_time * damage), 0, weapindx, bth, (realroll ? bth == realroll : 0));
	num_missiles_hit -= this_time;
    } else {
      num_missiles_hit = num_missiles_hit * damage;
      while (num_missiles_hit) {
        hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
        this_time = MIN(salvo_size, num_missiles_hit);
        DamageMech(target, mech, LOS, GunPilot(mech), hitloc, isrear,
            iscritical, pc_to_dam_conversion(target, weapindx,
                this_time), 0, weapindx, bth, (realroll ? bth == realroll : 0));
        num_missiles_hit -= this_time;

      }
    }
}

void mech_damage(dbref player, MECH * mech, char *buffer)
{
    char *args[5];
    int damage, clustersize;
    int isrear, iscritical;

    DOCHECK(mech_parseattributes(buffer, args, 5) != 4,
	"Invalid arguments!");
    DOCHECK(Readnum(damage, args[0]), "Invalid damage!");
    DOCHECK(Readnum(clustersize, args[1]), "Invalid cluster size!");
    DOCHECK(Readnum(isrear, args[2]), "Invalid isrear flag!");
    DOCHECK(Readnum(iscritical, args[3]), "Invalid iscritical flag!");
    DOCHECK(damage <= 0 || damage > 1000, "Invalid damage!");
    DOCHECK(clustersize <= 0, "Invalid cluster size!");
    DOCHECK(MechType(mech) == CLASS_MW, "No MW killings!");
    Missile_Hit(mech, mech, isrear, iscritical, 0, clustersize,
	damage / clustersize, 1, 0, 0, 0, 0, 0);
}

void heat_effect(MECH * mech, MECH * tempMech, int heatdam)
{
     int caninf = (mech == tempMech ? 0 : 1);
     int hitgroup = FindAreaHitGroup(mech, tempMech);
     int isrear = (hitgroup == BACK ? 1 : 0);
     int iscritical = 1;
     int hitloc;
     int l = 0, c = 0;

         if (caninf && (heatdam > 0 && MechType(tempMech) != CLASS_MECH && MechType(tempMech) != CLASS_MW
	    && MechType(tempMech) != CLASS_BSUIT && !IsDS(tempMech) && !Destroyed(tempMech) &&
	    MechMove(tempMech) != MOVE_NONE && (!(MechSpecials(tempMech) & CRITPROOF_TECH))))
	        {
		    hitloc = FindHitLocation(tempMech, hitgroup, &iscritical, &isrear, mech);
		    for ( l = 0 ; l < NUM_CRITICALS ; l++ )
			if ((GetPartMode(tempMech, hitloc, l) & INFERNO_MODE) &&
			!PartIsNonfunctional(tempMech, hitloc, l) && (GetPartData(tempMech, hitloc, l) > 0) &&
			IsAmmo(GetPartType(tempMech, hitloc, l)))
				{
				c++;
				break;
				}
		    if (c > 0)
			{
			MechLOSBroadcast(tempMech, "explodes!");
			mech_notify(tempMech, MECHALL, "Your inferno ammo ignites!");
			mech_notify(tempMech, MECHALL, "You explode!");
			DestroyMech(tempMech, mech, 1);
/*			DestroyAndDump(tempMech); */
			explode_unit(tempMech, mech);
			} else {
			    MechLOSBroadcast(tempMech, "is engulfed in flames!");
			    mech_notify(tempMech, MECHALL,
				"The heat's too much for your vehicle! Your engine performance decreases!");
          		    inferno_burn(tempMech, heatdam);
		    	    SendDebug(tprintf("#%d's vehicle has been inferno'ed by #%d ",
			    tempMech->mynum,
				    mech ? mech->mynum : tempMech->mynum));
			}

	         } else {

	    if (heatdam)
	        inferno_burn(tempMech, heatdam);
                mech_notify(tempMech, MECHALL, "You are splattered with burning jelly!");
  	     }
}

/* Burn.. burn in hell! ;> */
void Inferno_Hit(MECH * mech, MECH * hitMech, int missiles, int LOS)
{
    int hmod = (missiles + 1) / 2;

    heat_effect(mech, hitMech, hmod * 10);	/* 3min for _each_ missile */
}

int AMSMissiles(MECH * mech, MECH * hitMech, int incoming, int type,
    int ammoLoc, int ammoCrit, int LOS)
{
    int num_missiles_shotdown, spent;

    num_missiles_shotdown = Roll();
    if (!MechWeapons[type].special & CLAT)
	num_missiles_shotdown = MAX(1, num_missiles_shotdown / 2);
    /* decrement AMS ammo */
/*    spent = 2 * (Number(2, 7)); */
/* BMR Rules for AMS ammo consumption */
    spent = MIN(num_missiles_shotdown, incoming);
    num_missiles_shotdown = MIN(num_missiles_shotdown, incoming);
    if (!(MechWeapons[type].type == TBEAM))
            {
	    if (spent >= GetPartData(hitMech, ammoLoc, ammoCrit))
		GetPartData(hitMech, ammoLoc, ammoCrit) = 0;
  	    else
	    GetPartData(hitMech, ammoLoc, ammoCrit) -= spent;
          } else {
	    MechWeapHeat(hitMech) += (float) num_missiles_shotdown;
             }
    if (num_missiles_shotdown >= incoming) {
	if (LOS)
	    mech_notify(mech, MECHALL, "All of your missiles are shot down by the target!");
	mech_notify(hitMech, MECHALL, tprintf ("Your Anti-Missile System activates and shoots down %d incoming missiles!", num_missiles_shotdown));
	return 0;
    } else {
	mech_notify(mech, MECHALL, tprintf("The target shoots down %d of your missiles!", num_missiles_shotdown));
	mech_notify(hitMech, MECHALL, tprintf ("Your Anti-Missile System activates and shoots down %d incoming missiles!", num_missiles_shotdown));
    }
    return incoming - num_missiles_shotdown;
}

int LocateAMSDefenses(MECH * target, int *AMStype, int *ammoLoc,
    int *ammoCrit)
{
    int AMSsect, AMScrit;
    int i, j, w, t;

    if (!(MechSpecials(target) & (IS_ANTI_MISSILE_TECH |
		CL_ANTI_MISSILE_TECH)) || !Started(target) ||
	!(MechStatus(target) & AMS_ENABLED))
	return 0;
    for (i = 0; i < NUM_SECTIONS; i++) {
	for (j = 0; j < NUM_CRITICALS; j++)
	    if (IsWeapon((t = GetPartType(target, i, j))))
		if (IsAMS(Weapon2I(t)))
		    if (!(PartIsNonfunctional(target, i, j) ||
			    WpnIsRecycling(target, i, j)))
			break;
	if (j < NUM_CRITICALS)
	    break;
    }
    if (i == NUM_SECTIONS)
	return 0;
    w = Weapon2I(t);
    AMSsect = i;
    AMScrit = j;
    *AMStype = w;
    if (!(MechWeapons[w].type == TBEAM))
	    {
	    if (!(FindAmmoForWeapon(target, w, AMSsect, ammoLoc, ammoCrit)))
		return 0;
            if (!(GetPartData(target, *ammoLoc, *ammoCrit)))
		return 0;
	    MechWeapHeat(target) += (float) GunStat(w, GetPartMode(target, AMSsect, AMScrit), GUN_HEAT) +
					 MTX_HEAT_MOD(GetPartDamage(target, AMSsect, AMScrit));
	    }
    SetRecyclePart(target, AMSsect, AMScrit, GunStat(w, GetPartMode(target, AMSsect, AMScrit), GUN_VRT));
    return 1;
}

int MissileHitIndex(MECH * mech, MECH * hitMech, int weapindx, int plus2, int mode, int glance)
{
    int hit_roll;
    MAP *map;
/* #ifndef DFM_AFFECT_BTH */
    int r1, r2, r3, rtmp;
    map = getMap(mech->mapindex);
    if ((mode & DEADFIRE_MODE) || (mode & HOTLOAD_MODE)) {
	r1 = Number(1, 6);
	r2 = Number(1, 6);
	r3 = Number(1, 6);
/*        SendDebug(tprintf("DFM/Hotload MissileHit : One : %d Two : %d Three : %d", r1, r2, r3)); */
	/* Sort 'em to ascending order */
	if (r1 > r2)
	    Swap(r1, r2);
	if (r2 > r3)
	    Swap(r2, r3);
	hit_roll = r1 + r2 - 2;
/*        SendDebug(tprintf("DFM/Hotload MissileHit : Lowest %d %d - Total %d", r1, r2, hit_roll)); */
    } else
/* #endif */
	hit_roll = Roll() - 2;
/* Handled on HitTarget passing now. Tad more efficient. - DJ */
/*    if (plus2 && !(MechStatus(mech) & ECM_DISTURBANCE) &&
		!(MechStatus2(hitMech) & ECM_PROTECTED) && !(map->LOSinfo[mech->mapnumber][hitMech->mapnumber] & MECHLOSFLAG_ECM)) */
    if (plus2)
	hit_roll = (hit_roll > 7) ? 10 : hit_roll + 2;
    if (glance && mudconf.btech_glance)
	hit_roll = (hit_roll < 6) ? 1 : hit_roll - 4;
    if ((MechWeapons[weapindx].special & STREAK) && !(map->LOSinfo[mech->mapnumber][hitMech->mapnumber] & MECHLOSFLAG_ANGELECM))
	return 10;
    return hit_roll;

}

int MissileHitTarget(MECH * mech, int weapindx, MECH * hitMech, int LOS,
    int baseToHit, int roll, int plus2, int reallynarc, int inferno,
    int incoming, int mode, int realroll)
{
    int isrear = 0, iscritical = 0;
    int AMStype, ammoLoc, ammoCrit;
    int hit, wspec = (MechWeapons[weapindx].special);
    int i, j = -1, k, l = 0;
    char LocBuff[20];
    int hitloc;

/*    SendDebug(tprintf("NARC : %d iNARC : %d", (wspec & NARC), (wspec & MUN))); */
    if ((wspec & NARC) && reallynarc) {
	if (roll >= baseToHit) {
        hitloc = FindTargetHitLoc(mech, hitMech, &isrear, &iscritical);
        ArmorStringFromIndex(hitloc, LocBuff, MechType(hitMech), MechMove(hitMech));
            if (wspec & MUN)
		{
		MechStatus2(hitMech) |= INARC_ATTACHED;
		mech_notify(hitMech, MECHALL,
                    tprintf("A iNARC Beacon has been attached to your units %s!", LocBuff));
                mech_notify(mech, MECHALL,
                    tprintf("Your iNARC Beacon attaches to the targets %s!", LocBuff));
		MechSections(hitMech)[hitloc].config |= SECTION_INARC;
		} else {
	        MechStatus(hitMech) |= NARC_ATTACHED;
	        mech_notify(hitMech, MECHALL,
		    tprintf("A NARC Beacon has been attached to your units %s!", LocBuff));
	        mech_notify(mech, MECHALL,
		    tprintf("Your NARC Beacon attaches to the targets %s!", LocBuff));
		MechSections(hitMech)[hitloc].config |= SECTION_NARC;
		}
	}
	return 0;
    }
    /* Attacker is using a missile weapon and the defender is using an
       AMS system.  The AMS system intercepts before the to-hit roll */
    if (LocateAMSDefenses(hitMech, &AMStype, &ammoLoc, &ammoCrit)) {
	if (!(incoming = AMSMissiles(mech, hitMech, incoming, AMStype, ammoLoc, ammoCrit, LOS)))
	    return 0;
    }
    if (roll < baseToHit)
	return incoming;

    for (i = 0; MissileHitTable[i].key >= 0; i++)
	if ((k = MissileHitTable[i].num_missiles[10]) <= incoming &&
	    ((MechWeapons[MissileHitTable[i].key].special & STREAK) == (MechWeapons[weapindx].special & STREAK)))
	    if (k >= l && (j < 0 || MissileHitTable[i].key != weapindx ||
		    k > l)) {
		j = i;
		l = k;
	    }
    if (j < 0)
	return 0;
    hit =
	MIN(incoming, MissileHitTable[j].num_missiles[MissileHitIndex(mech,
	      hitMech, weapindx, plus2, mode, baseToHit == realroll)]);
    if (LOS) {
	mech_notify(mech, MECHALL, tprintf("%%cg%s with %d missile%s!%%c",
		LOS == 1 ? "You hit" : "The swarm hits", hit,
		hit > 1 ? "s" : ""));
    }
    if (inferno)
	Inferno_Hit(mech, hitMech, hit, LOS);
    else
	Missile_Hit(mech, hitMech, isrear, iscritical, weapindx, hit,
	    GunStat(weapindx, mode, GUN_DAMAGE), Clustersize(weapindx, mode), LOS,
	    baseToHit, mode, 1, realroll);
    return incoming - hit;
}

#ifndef BTH_DEBUG
#define BTHBASE(m,t,n)  do { baseToHit = n; } while (0)
#define BTHADD(desc,n)  do { baseToHit += n; } while (0)
#define BTHEND(m)
#else
#define BTHBASE(m,t,n)  do { if (t) sprintf(buf, "#%d -> #%d: Base %d", m->mynum, t->mynum, n); else sprintf(buf, "#%d -> (hex): Base %d", m->mynum, n); baseToHit = n; } while (0)
#define BTHADD(desc,n)  do { i = n ; if (i) { sprintf(buf+strlen(buf), ", %s: %s%d", desc, i>0 ? "+" : "", i); baseToHit += i; } } while (0)
#define BTHEND(m)       SendBTHDebug(tprintf("%s.", buf))
#endif

#define C3Destroyed(mech) (MechCritStatus((mech)) & C3_DESTROYED)
#define Disturbed(mech)   ((MechStatus((mech)) & ECM_DISTURBANCE))

int FindNormalBTH(MECH * mech, MAP * mech_map, int section, int critical,
    int weapindx, float range, MECH * target, int indirectFire, int sight)
{
    MECH *spotter = NULL;
    int baseToHit;
    int mode = GetPartMode(mech, section, critical);
    int hasTAG = 0, IsGuided = 0;
#ifdef BTH_DEBUG
    char buf[LBUF_SIZE];
#endif
    int i;
    int j, rbth = 0;
    float enemyX, enemyY, enemyZ;
    int arc = 0;

    BTHBASE(mech, target, FindPilotGunnery(mech, weapindx));
    if (indirectFire < 1000 && MechSpotter(mech) > 0) {
	spotter = getMech(MechSpotter(mech));
	if (!spotter) {
	    mech_notify(mech, MECHALL, "Error finding your spotter! (notify a wiz)");
    	return 0;
	}
        hasTAG = (HasFuncTAG(spotter));
	IsGuided = (target && MechSpotter(mech) != mech->mynum && MechSpotter(mech) > 0
	    && (mode & SGUIDED_MODE) && hasTAG && !(MechStatus2(target) & ECM_PROTECTED)
	    && !(mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_ECM)
	    && !(mech_map->LOSinfo[spotter->mapnumber][target->mapnumber] & MECHLOSFLAG_ECM)
	    && !(mech_map->LOSinfo[mech->mapnumber][spotter->mapnumber] & MECHLOSFLAG_ECM)
	    && (FaMechRange(spotter, target) < 15.0));
	BTHADD("Spotting", (hasTAG > 0 ? 0 : (FindPilotSpotting(spotter) - 2)));
	BTHADD("SpotterMove", AttackMovementMods(spotter));
	if (spotter && target && FindRange(MechFX(target), MechFY(target), MechFZ(target), MechFX(spotter), MechFY(spotter), MechFZ(spotter)) > 21)
	    BTHADD("SpotterOutOfRange", 30);
    }

    if (Spinning(mech))
	BTHADD("Spinning/OOC", 2);

    if (MechType(mech) == CLASS_MECH && ((i = GetPartDamage(mech, section, critical)) & MTX_TOHIT))
	BTHADD("MTX/ToHit", MTX_HEAT_MOD(i));

    if (MechType(mech) == CLASS_MECH && ((i = GetPartDamage(mech, section, critical)) & MTX_TOHIT_RANGE))
	if (range > GunStat(weapindx, mode, GUN_SHORTRANGE))
	    BTHADD("MTX/ToHitRange", MTX_TOHIT_RANGE_MOD(i));

    /* MW need +2 added per FASA */
    if (target && MechType(target) == CLASS_MW)
	BTHADD("MechWarrior", 2);
    /* BT:RoW Water mod rules - DJ */
    if (target) {
	if ((MechRTerrain(target) == WATER) && (MechZ(target) == -1))
	    BTHADD("WaterMod", -1);
	if ((MechRTerrain(mech) == WATER) && (MechZ(mech) == -1))
	    BTHADD("InWaterMod", 1);
	}
    /* add in to-hit mods from criticals */
    BTHADD("MechBTHMod", MechBTH(mech));
    if (MechMove(mech) == CLASS_MECH && Fallen(mech) && (SectIsDestroyed(mech, LARM) || SectIsDestroyed(mech, RARM)))
	BTHADD("ProneArmGone", 1);

    /* add in to-hit mods for section damage */
    BTHADD("MechLocBTHMod", MechSections(mech)[section].basetohit);

    /* Add in the rangebase.. */
    if ((!is_aero(mech) && (!HotLoading(weapindx, mode) || range >= GunStat(weapindx, mode, GUN_SHORTRANGE))) || (is_aero(mech) && Landed(mech))) {
	if (EGunRange(weapindx, mode) < range) {
	    BTHADD("OutOfRange", 1000);
	} else {
	    if (range <= GunStat(weapindx, mode, GUN_MINRANGE)) {
 	        /* if the target is in minimum range then the BTH is as good as it will get */
		BTHADD("MinRange", (rbth = (GunStat(weapindx, mode, GUN_MINRANGE) - range + 1)));
	    } else if (!((MechSpecials(mech) & (C3_MASTER_TECH | C3_SLAVE_TECH)) && !C3Destroyed(mech) && !Disturbed(mech)) ||
				(EGunRange(weapindx, mode) < range)) {
	        /* if it is not in a c3 network then it can use Extended Range */
		BTHADD("Range", (rbth = FindBaseToHitByRange(weapindx, range, mode, 0, MechTargComp(mech))));
	    } else {
	        /* Dont use extended range with mechs in a c3 network (quick hack doesn't allow own targetting too */
	        BTHADD("C3Range", (rbth = FindBaseToHitByC3Range(weapindx, FindC3Range(mech, target, range), mode, MechTargComp(mech))));
	    }
	}
    } else if (is_aero(mech) && range > GunStat(weapindx, mode, GUN_AERORANGE)) {
	BTHADD("Aero/OutRange", 1000);
    } else if (target && is_aero(mech) && !Landed(mech) && !FlyingT(target) && !OODing(target)) {
	int relat, bearing;
	bearing = FindBearing(MechFX(mech), MechFY(mech), MechFX(target), MechFY(target));
	relat = MechFacing(mech) - bearing;
	relat = AcceptableDegree(relat);
	if (range > 6 || !(relat < 10 || relat > 350))
	    BTHADD("AeroStrike/NoArc", 20);
	else
	    BTHADD("AeroStrike", 2);
    }
    if (is_aero(mech)) {
	if (range <= AERORNG_SHORT_RNG)
	    BTHADD("AeroRange/Short", 0);
	else if (range <= AERORNG_MEDIUM_RNG)
	    BTHADD("AeroRange/Medium", 2);
	else if (range <= AERORNG_LONG_RNG)
	    BTHADD("AeroRange/Long", 4);
	else 
	    BTHADD("AeroRange/Ext", 8);
	}

    if (target && (MechSpecials(target) & STEALTH_TECH)) {
	rbth = (FindBaseToHitByRange(weapindx, range, mode, 1, MechTargComp(mech)));
	if (MechSpecials(target) & FWL_STEALTH_TECH)
	    BTHADD("StealthFWLBonus", (rbth == 0 ? 1 : rbth < 4 ? 2 : rbth >= 4 ? 3 : 0));
		/*	if (rbth > 0 && rbth <= 4) */
	if (MechSpecials(target) & DC_STEALTH_TECH)
	    BTHADD("StealthDCBonus", rbth / 2);
	if (MechSpecials(target) & CS_STEALTH_TECH)
	    BTHADD("StealthCSBonus", ((MMaxSpeed(target) / KPH_PER_MP) -  (MechSpeed(target) / KPH_PER_MP)));
	}

    if (target && (MechStatus2(target) & NULLSIG_ACTIVE)) {
	rbth = (FindBaseToHitByRange(weapindx, range, mode, 1, 0));
	BTHADD("NullSigBonus", (rbth < 2 ? 0 : rbth == 2 ? 1 : rbth > 2 ? 2 : 0));
	} else if (target && (StealthAndECM(target))) {
	    rbth = (FindBaseToHitByRange(weapindx, range, mode, 1, 0));
  	    BTHADD("StealthArmBonus", (rbth < 2 ? 0 : rbth == 2 ? 1 : rbth > 2 ? 2 : 0));
	}

/* Do I need to optimize the event portions of this more? They are macros calling addressed functions the compiler should optimized secondary OR checks out completely */
    if (target && MechStatus2(target) & (SPRINTING|EVADING)) {
	if (MechStatus2(target) & SPRINTING)
	    BTHADD("SprintingTarget", -4);
	if (!Fallen(target) && MechStatus2(target) & EVADING)
	    BTHADD("EvadingTarget", 1); 
/* BTHADD("EvadingTarget", (FindPilotPiloting(target) >= 6 ? 1 : FindPilotPiloting(target) >= 4 ? 2 :
		FindPilotPiloting(target) >= 2 ? 3 : 4) + (HasBoolAdvantage(MechPilot(target), "speed_demon") ? 1 : 0)); */
	} else if (target && MoveModeChange(target)) {
	    int i = MoveModeData(target);
	    if (i & MODE_SPRINT)
		BTHADD("SprintingTargetChanging", -4);
	    if (i & MODE_EVADE)
		BTHADD("EvadingTargetChanging", 1);
	}
    /* Add in the movement modifiers */
    BTHADD("AttackMove", AttackMovementMods(mech));
    if (MechSections(mech)[section].config & STABILIZER_CRIT)
	BTHADD("StabilizerCrit", AttackMovementMods(mech));
    BTHADD("Overheat", OverheatMods(mech));

    if ((mech) && (target))
	if ((MechStatus2(target) & INARC_ATTACHED) && (mode & NARC_MODE))
	    if (!(MechStatus2(target) & ECM_PROTECTED) && !(mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_ECM))
		BTHADD("iNarc", -1);
    if (MechWeapons[weapindx].special & PULSE)
		BTHADD("Pulse", -2);
    if (MechWeapons[weapindx].special & MRM)
		BTHADD("MRM", 1);
    if (MechWeapons[weapindx].special & OS_WEAP)
		BTHADD("Rocket", 1);
    if (MechWeapons[weapindx].special & DAR)
		BTHADD("DelayedWeapon", 1);
    if (MechWeapons[weapindx].special & HVYW)
		BTHADD("HeavyWeapon", 1);
    if (target && (mode & STINGER_MODE)) {
	if (FlyingT(target) && !Landed(target))
	    BTHADD("Stinger (Flying)", -3);
	else if (OODing(target))
	    BTHADD("Stinger (OOD)", -1);
	else if (Jumping(target))
	    BTHADD("Stinger (Jumping)", 0);
	}
    if (target && (MechType(target) == CLASS_VTOL) && (fabs(MechSpeed(target)) > 0.0 || fabs(MechVerticalSpeed(target)) > 0.0))
    	BTHADD("TargetVTOL", 1);
    if (target && MechTargComp(mech) == TARGCOMP_AA) {
	if (!Landed(target) && (FlyingT(target) || Jumping(target) || OODing(target)))
	    BTHADD("TargCompAA/Fly", (MechSpecials(mech) & AA_TECH || MechSpecials2(mech) & ADV_AA_TECH ? -3 : -2));
    	else
	    BTHADD("TargCompAA/Ground", 1);
	}

    if (mode & PIERCE_MODE)
	BTHADD("Pierce", 1);
    if (target) {
	if ((mode & TRACER_MODE) && (mech_map->maplight < 2) && !IsLit(target)) {
	    if (sensors[(int) MechSensor(mech)[0]].sensorname[0] == 'V') {
	        if (mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_SEESP)
		    BTHADD("Tracer", -1);
		} else if (sensors[(int) MechSensor(mech)[1]].sensorname[0] == 'V') {
		    if (mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_SEESS)
                	BTHADD("Tracer", -1);
		}
	}
    }
    if (mode & LBX_MODE)
	BTHADD("LBX", (target && FlyingT(target) ? -3 : -1));
    else
	/* TC that aims nowhere gets -1, TC that aims gets +3 */
	if ((mode & ON_TC) && !(MechCritStatus(mech) & TC_DESTROYED))
		BTHADD("TC", (!target || MechAim(mech) == NUM_SECTIONS || MechAimType(mech) != MechType(target)) ? -1 : 3);
    /* Unstable lock */
    if (MechTargComp(mech) != TARGCOMP_MULTI && (Locking(mech) || (!spotter && target && MechTarget(mech) != target->mynum))) {
	if (FindTargetXY(mech, &enemyX, &enemyY, &enemyZ)) {
	    if ((arc = InWeaponArc(mech, enemyX, enemyY)) & FORWARDARC)
		BTHADD("UnstableLock/Fwarc", 1);
	    else
		BTHADD("UnstableLock", 2);
	} else if (target) {
	    enemyX = MechFX(target);
	    enemyY = MechFY(target);
	    enemyZ = MechFZ(target);
	    if ((arc = InWeaponArc(mech, enemyX, enemyY)) & FORWARDARC)
                BTHADD("UnstableLock/Fwarc", 1);
            else
                BTHADD("UnstableLock", 2);
	}
    }

    if (MechTargComp(mech) == TARGCOMP_MULTI) {
	if (FindTargetXY(mech, &enemyX, &enemyY, &enemyZ)) {
	    if (!((arc = InWeaponArc(mech, enemyX, enemyY)) & FORWARDARC))
		BTHADD("MultiTarg/SideArc", 1);
	} else if (target) {
	    enemyX = MechFX(target);
	    enemyY = MechFY(target);
	    enemyZ = MechFZ(target);
	    if (!((arc = InWeaponArc(mech, enemyX, enemyY)) & FORWARDARC))
		BTHADD("MultiTarg/SideArc", 1);
	}
    }

    if (!target && (!(IsArtillery(weapindx))) && (MechStatus(mech) & (LOCK_HEX | LOCK_BUILDING | LOCK_HEX_IGN | LOCK_HEX_CLR)))
     	BTHADD("HexBonus", -4);
    if (target && C_OODing(target))
	BTHADD("OODbonus", -3);
    /* Indirect fire terrain modifiers */
    if (indirectFire < 1000)
	BTHADD("IDFTerrain", indirectFire);
    if (MechSpotter(mech) == mech->mynum)
	BTHADD("Spotting", 1);
    if (target) {
	if ((MechCritStatus(target) & DUG_IN)
	 && (!mudconf.btech_dig_only_fs || (FindAreaHitGroup(mech, target) == FRONT))
	 && (MechZ(target) >= MechZ(mech)) && !(MechMove(mech) == MOVE_QUAD))
    	BTHADD("DugIn", mudconf.btech_digbonus);
	if (IsDS(target))
	    BTHADD("DSBonus", -3);
	BTHADD("Bsuitbonus", (MechType(target) == CLASS_BSUIT ? 1 : 0));
	if (((MechType(mech) == CLASS_MECH && MechAim(mech) == HEAD)
	 || (MechType(mech) == CLASS_MW && MechAim(mech) == HEAD))
	 && !IsMissile(weapindx))
	 	BTHADD("HeadTarget", ((mode & ON_TC) && !(MechCritStatus(mech) & TC_DESTROYED))
		? (MechAimType(mech) == MechType(target)) ? 0 : 3 : 7);
	if (is_aero(target) && !Landed(target)) {
	    if (!IsGuided)
		BTHADD("TargetMove", TargetMovementMods(mech, target, range));
	} else {
	    if (!IsGuided)
	    	BTHADD("TargetMove", TargetMovementMods(mech, target, range));
	}
	if (mode & PRECISION_MODE && TargetMovementMods(mech, target, range) > 0) {
		rbth = (FindBaseToHitByRange(weapindx, range, mode, 1, MechTargComp(mech)));
		BTHADD("Precision", -(MAX(MIN(rbth == 0 ? 2 : rbth < 4 ? 1 : 0, TargetMovementMods(mech, target, range)), 0)));
	}
	/* Add in the terrain modifier */
	if ((indirectFire >= 1000) && (!(IsArtillery(weapindx)))) {
	    j = AddTerrainMod(mech, target, mech_map, range);
	    if (j < 1000)
		BTHADD("Terrain/Light(Sensor)", j);
	}
/*	if (MechType(mech) == CLASS_BSUIT && (target->mynum == MechSwarmTarget(mech)))
		{
		BTHADD("SwarmerHitSwarmTarget", 0);
		baseToHit = 0;
		} */
    }
    if (!sight)
	BTHEND(mech);
    return baseToHit;
}

int FindArtilleryBTH(MECH * mech, int weapindx, int indirect, float range)
{
    int baseToHit = (11 + (MechWeapons[weapindx].min > range ? (MechWeapons[weapindx].min - range + 1) : 0) +
	((!mech || (Locking(mech) && MechTargComp(mech) != TARGCOMP_MULTI)) ? 1 : 0));
    MECH *spotter;

    if (EGunRange(weapindx, 0) < range)
	return 1000;

    baseToHit += (FindPilotArtyGun(mech) - 4);
    if (indirect) {
	spotter = getMech(MechSpotter(mech));
	if (spotter && spotter != mech)
	    baseToHit += (FindPilotSpotting(spotter) - 4) / 2;
	/* the usual +2, added by +1 make +3 */
	if (indirect && (MechSpotter(mech) == -1 ||
		MechSpotter(mech) == mech->mynum)) baseToHit += 1;
    } else
	baseToHit -= 2;
    return baseToHit - MechFireAdjustment(mech);
}

char *hex_target_id(MECH * mech)
{
    if (MechStatus(mech) & LOCK_HEX_IGN)
	return "at the hex, trying to ignite it";
    if (MechStatus(mech) & LOCK_HEX_CLR)
	return "at the hex, trying to clear it";
    if (MechStatus(mech) & LOCK_HEX)
	return "at the hex";
    if (MechStatus(mech) & LOCK_BUILDING)
	return "at the building at";
    return "at";
}

int possibly_ignite(MECH * mech, MAP * map, int weapindx, int x, int y,
    int bth1, int bth2, int r, int firemod)
{
    char terrain;

    terrain = GetTerrain(map, x, y);
    if (MechWeapons[weapindx].special & PCOMBAT)
	return 0;
    if (((!strcmp(&MechWeapons[weapindx].name[3], "Flamer") && ((bth2 && r < bth2) || (!bth2 && r >= 3 + firemod))) || (MechWeapons[weapindx].type == TBEAM &&
		strcmp(&MechWeapons[weapindx].name[3], "SmallLaser") &&
		strcmp(&MechWeapons[weapindx].name[3], "ERSmallLaser") &&
		((bth2 && r < (bth2 + 3)) || (!bth2 && r >= 6 + firemod)))
	    || (MechWeapons[weapindx].type == TMISSILE &&
		strcmp(&MechWeapons[weapindx].name[3], "SRM-2") &&
		strcmp(&MechWeapons[weapindx].name[3], "StreakSRM-2") &&
		strcmp(&MechWeapons[weapindx].name[3], "GaussRifle") &&
		strcmp(&MechWeapons[weapindx].name[3], "NarcBeacon") &&
		((bth2 && r < (bth2 + 5)) || (!bth2 && r >= 8 + firemod))) || (bth2 < 0)) && ((terrain == LIGHT_FOREST || terrain == HEAVY_FOREST))) {
	fire_hex(mech, x, y, bth2 == 0);
	return 1;
    }
    return 0;
}


int possibly_clear(MECH * mech, int weapindx, int x, int y, int bth1,
    int bth2, int r)
{
    if (MechWeapons[weapindx].special & PCOMBAT)
	return 0;
    if (r > bth1 && strcmp(&MechWeapons[weapindx].name[3], "SmallLaser") &&
	strcmp(&MechWeapons[weapindx].name[3], "ERSmallLaser") &&
	strcmp(&MechWeapons[weapindx].name[3], "MachineGun") &&
	strcmp(&MechWeapons[weapindx].name[3], "AC/2") &&
	strcmp(&MechWeapons[weapindx].name[3], "AC/5") &&
	strcmp(&MechWeapons[weapindx].name[3], "UltraAC/2") &&
	strcmp(&MechWeapons[weapindx].name[3], "UltraAC/5") &&
	strcmp(&MechWeapons[weapindx].name[3], "LB2-XAC") &&
	strcmp(&MechWeapons[weapindx].name[3], "LB5-XAC") &&
	strcmp(&MechWeapons[weapindx].name[3], "SRM-2") &&
	strcmp(&MechWeapons[weapindx].name[3], "StreakSRM-2") &&
	strcmp(&MechWeapons[weapindx].name[3], "NarcBeacon")) {
	clear_hex(mech, x, y, bth2 == 0, weapindx, 1);
	possibly_remove_mines(mech, x, y);
	return 1;
    }
    return 0;
}

void possibly_ignite_or_clear(MECH * mech, int weapindx, int x, int y,
    int bth1, int bth2)
{
    int r;
    MAP *map;
    int first;

    r = Roll();
    map = FindObjectsData(mech->mapindex);
    if (!map)
	return;
    if (MechStatus(mech) & LOCK_HEX_IGN) {
	if (!(possibly_ignite(mech, map, weapindx, x, y, bth1, bth2, r,
		    0))) if (Number(1, 6) < 3)
		possibly_clear(mech, weapindx, x, y, bth1, bth2, r);
	return;
    }
    if (MechStatus(mech) & LOCK_HEX_CLR) {
	if (!(possibly_clear(mech, weapindx, x, y, bth1, bth2, r)))
	    if (Number(1, 6) < 3)
		possibly_ignite(mech, map, weapindx, x, y, bth1, bth2, r,
		    0);
	return;
    }
    first = Number(1, 2);
    if (first == 1 || bth2 < 0) {
	if (!possibly_ignite(mech, map, weapindx, x, y, bth1, bth2, r, 0))
	    possibly_clear(mech, weapindx, x, y, bth1, bth2, r);
    } else {
	if (!(possibly_clear(mech, weapindx, x, y, bth1, bth2, r)))
	    possibly_ignite(mech, map, weapindx, x, y, bth1, bth2, r, 0);
    }
}

void hex_hit(MECH * mech, int x, int y, int weapindx, int ishit, int mode)
{
    if (!(MechStatus(mech) & (LOCK_BUILDING | LOCK_HEX | LOCK_HEX_IGN | LOCK_HEX_CLR)))
	return;
    /* Ok.. we either try to clear/ignite the hex, or alternatively
       we try to hit building in it */
    if (MechStatus(mech) & LOCK_BUILDING) {
	if (ishit > -1)
	    hit_building(mech, x, y, weapindx, 0, mode, ishit == 0 ? 1 : 0);
    } else {
	possibly_ignite_or_clear(mech, weapindx, x, y, 5, ishit < 0 ? 11 : 0);
	if (MechStatus(mech) & LOCK_HEX) {
	    possibly_blow_ice(mech, weapindx, x, y);
	    possibly_blow_bridge(mech, weapindx, x, y);
	}
    }
}

void decrement_ammunition(MECH * mech, int weapindx, int section,
    int critical, int ammoLoc, int ammoCrit)
{
    if (MechWeapons[weapindx].type == TBEAM ||
	    MechWeapons[weapindx].type == THAND)
	return;
    if (GetPartMode(mech, section, critical) & OS_MODE) {
	GetPartMode(mech, section, critical) |= OS_USED;
	return;
    }
/* ULTRA decrements removed for cleaner mode. ammo_exped moved to prevent spam checks - DJ */
/*    ammo_expedinture_check(mech, weapindx, GetPartMode(mech, section, critical) & (ULTRA_MODES)); */
    if (GetPartData(mech, ammoLoc, ammoCrit))
	GetPartData(mech, ammoLoc, ammoCrit)--;
/* Screw this garbage. We'll do the rest later, though. */
/*    if (GetPartMode(mech, section, critical) & (ULTRA_MODES))
	if (GetPartData(mech, ammoLoc, ammoCrit))
	    GetPartData(mech, ammoLoc, ammoCrit)--; */
}

void SwarmHitTarget(MECH * mech, int weapindx, MECH * hitMech, int LOS,
    int baseToHit, int roll, int plus2, int reallynarc, int inferno,
    int incoming, int fof, int mode, int section, int critical, int realroll)
{
#define MAX_STAR 10
    /* Max # of targets we'll try to hit: 10 */
    MECH *star[MAX_STAR];
    int present_target = 0;
    int missiles;
    int loop;
    MAP *map = FindObjectsData(mech->mapindex);
    float r = 0.0, ran = 0.0, flrange = 0.0;
    MECH *source = mech, *tempMech;
    int i, j;

    for (loop = 0; MissileHitTable[loop].key != -1; loop++)
	if (MissileHitTable[loop].key == weapindx)
	    break;
    if (!(MissileHitTable[loop].key == weapindx))
	return;
    missiles = MissileHitTable[loop].num_missiles[10];
    while (missiles > 0) {
	flrange = flrange + FaMechRange(source, hitMech);
	ran = FaMechRange(mech, hitMech);
	if (flrange > EGunRange(weapindx, mode)) {
	    mech_notify(hitMech, MECHALL, "Luckily, the missiles fall short of you!");
	    return;
	}
	if (!(missiles = MissileHitTarget(mech, weapindx, hitMech,
	    InLineOfSight_NB(mech, hitMech, MechX(mech), MechY(mech),
            ran) ? present_target == 0 ? 1 : 2 : 0, ((fof &&
	    (MechTeam(mech) == MechTeam(hitMech))) ? baseToHit = (baseToHit + 2) : baseToHit),
	    roll, plus2, reallynarc, inferno, missiles, mode, realroll)))
		return;
/*         SendDebug(tprintf("Swarm BTH : Attacker #%d : Source #%d : Target #%d : FoF %d : BaseToHit %d : Roll %d : Range : %3.3f",
                    mech->mynum, source->mynum, hitMech->mynum, fof, baseToHit, roll),
		    FaMechRange(source, hitMech))); */

	/* Try to acquire a new target NOT in the star */
	if (present_target == MAX_STAR)
	    return;
	star[present_target++] = hitMech;
	source = hitMech;
	hitMech = NULL;
	for (i = 0; i < map->first_free; i++)
	    if ((tempMech = FindObjectsData(map->mechsOnMap[i])))
		{
                for (j = 0; j < present_target; j++)
		    if (tempMech == star[j])
			break;
/*                SendDebug(tprintf("Swarm ParseTarget : Attacker #%d : Source #%d : Target #%d : FoF %d : BaseToHit %d : Range : %3.3f",
                    mech->mynum, source->mynum, tempMech->mynum, fof, baseToHit, FaMechRange(source, tempMech))); */
                if (source == tempMech)
	            continue;
                if (j != present_target)
	            continue;
		r = FaMechRange(source, tempMech);
/*		SendDebug(tprintf("Swarm RangeCheck : Source #%d : Target #%d : Range %2.2f : RangeAttrib : %2.2f",
			source->mynum, tempMech->mynum, FaMechRange(source, tempMech), r)); */
		if (r > 2.)
			continue;
		if (MechStatus(tempMech) & COMBAT_SAFE)
			continue;
		if (!hitMech || r < ran)
			{
			    hitMech = tempMech;
			    ran = r;
			}
		}
	if (!hitMech)
	    return;
/*	SendDebug(tprintf("Swarm ReTarget : Attacker #%d : Source #%d : Target #%d : FoF %d : BaseToHit %d : Range : %3.3f",
		mech->mynum, source->mynum, hitMech->mynum, fof, baseToHit, ran)); */
	if (mech != hitMech)
	    mech_notify(hitMech, MECHALL, "The missile-swarm turns towards you!");
	if (InLineOfSight_NB(mech, source, MechX(mech), MechY(mech), FaMechRange(mech, source)))
	    mech_notify(mech, MECHALL,
		tprintf("Your missile-swarm of %d missile%s targets %s!",
		    missiles, missiles > 1 ? "s" : "",
		    mech == hitMech ? "YOU!!" : GetMechToMechID(mech,
			hitMech)));
	MechLOSBroadcasti(mech, hitMech, "'s missile-swarm targets %s!");
        if (present_target != 0)
	    baseToHit = FindNormalBTH(mech, map, section, critical, weapindx, FaMechRange(mech, hitMech), hitMech,
		(InLineOfSight_NB(mech, hitMech, MechX(mech), MechY(mech), FaMechRange(mech, hitMech)) ? 1000 : 2), 0);
	roll = Roll();
    }
}

char *short_hextarget(MECH * mech)
{

    if (MechStatus(mech) & LOCK_HEX_IGN)
	return "ign";
    if (MechStatus(mech) & LOCK_HEX_CLR)
	return "clr";
    if (MechStatus(mech) & LOCK_HEX)
	return "hex";
    if (MechStatus(mech) & LOCK_BUILDING)
	return "bld";
    return "reg";
}

void FireWeapon(MECH * mech, MAP * mech_map, MECH * target, int LOS,
    int weapindx, int weapnum, int section, int critical, float enemyX,
    float enemyY, int mapx, int mapy, float range, int indirectFire,
    int sight, int ishex, int swarmhit)
{
    int smokebrg;
    short smokex, smokey;
    float fsmokex, fsmokey, fsmokex1, fsmokey1;
    int rfmmg = 0;
    int wspec = MechWeapons[weapindx].special;
    int wtype = MechWeapons[weapindx].type;
    int baseToHit, RbaseToHit, temp = 0, temproll = 0, isjam = 0;
    int ammoLoc;
    int ammoCrit;
    int roll, i;
    int r1, r2, r3, rtmp;
    int modifier = 0;
    int isarty = (IsArtillery(weapindx));
    int range_ok = 1;
    int mode = GetPartMode(mech, section, critical);
    int ammoused;
    int ECMblock = 0;
    int glanceon = mudconf.btech_glance;
    char buf[SBUF_SIZE];
    char buf3[SBUF_SIZE];
    char buf2[LBUF_SIZE];

    DOCHECKMA((mode & STINGER_MODE) && ishex, "Stinger missiles cannot shoot hexes!");
    DOCHECKMA((mode & STINGER_MODE) && target && (!FlyingT(target) && !OODing(target) && !Jumping(target) && !Landed(target)),
	"Stinger missiles can only engage airborne targets!");
    /* Find and check Ammunition */

    /* check if self cooling */
    if (IsCoolant(weapindx) && mode & HEAT_MODE)
    	target = mech;

    if (target)
	ECMblock = ((MechStatus(mech) & ECM_DISTURBANCE) ||
                (mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_ECM) ||
                (MechStatus2(target) & ECM_PROTECTED));
    if (!sight)
	{
	if (!FindAndCheckAmmo(mech, weapindx, section, critical, &ammoLoc, &ammoCrit))
	    return;
        mode = GetPartMode(mech, section, critical);
        MechStatus(mech) |= FIRED;
	}

    /*if (!(isarty) || isarty) */
    if (target == mech)
	baseToHit = FindPilotGunnery(mech, weapindx);
    else
	baseToHit = FindNormalBTH(mech, mech_map, section, critical, weapindx, range, target, indirectFire, sight);

    if (glanceon)
        RbaseToHit = baseToHit - 1;
    else
	RbaseToHit = baseToHit;
    if (target && !ishex) {
	range = FaMechRange(mech, target);
	strcpy(buf2, "");
	if (MechAim(mech) != NUM_SECTIONS &&
	    MechAimType(mech) == MechType(target) && !IsMissile(weapindx)) {
	    ArmorStringFromIndex(MechAim(mech), buf3, MechType(target),
		MechMove(target));
	    sprintf(buf2, "'s %s", buf3);
	}
	if (sight) {
		DOCHECKMA(baseToHit >= 900,
		tprintf("You aim %s at %s%s - %s.",
		    &MechWeapons[weapindx].name[3], GetMechToMechID(mech,
			target), buf2, "Out of range."));
	    mech_notify(mech, MECHALL,
		tprintf("You aim %s at %s%s - BTH: %d %s",
		    &MechWeapons[weapindx].name[3], GetMechToMechID(mech,
			target), buf2, baseToHit,
		    MechStatus(target) & PARTIAL_COVER ? "(Partial cover)"
		    : ""));
	    return;
	}
	if (RbaseToHit > 12) {
		DOCHECKMA(baseToHit >= 900,
		tprintf("You aim %s at %s%s - %s.",
		    &MechWeapons[weapindx].name[3], GetMechToMechID(mech,
			target), buf2, "Out of range."));
	    mech_notify(mech, MECHALL,
		tprintf("Fire %s at %s%s - BTH: %d  Roll: Impossible! %s",
		    &MechWeapons[weapindx].name[3], GetMechToMechID(mech,
			target), buf2, baseToHit,
		    MechStatus(target) & PARTIAL_COVER ? "(Partial cover)"
		    : ""));
	    return;
	}
    } else {
	/* Hex target sight info */
	if (sight) {
	    if (baseToHit > 900)
		mech_notify(mech, MECHPILOT,
		    tprintf("You aim your %s at (%d,%d) - %s",
			&MechWeapons[weapindx].name[3], mapx, mapy, "Out of range"));
	    else
		mech_notify(mech, MECHPILOT,
		    tprintf("You aim your %s at (%d,%d) - BTH: %d",
			&MechWeapons[weapindx].name[3], mapx, mapy,
			baseToHit));
	    return;
	}
	if (RbaseToHit > 12) {
	    mech_notify(mech, MECHALL,
		tprintf("Fire %s at (%d,%d) - BTH: %d  Roll: Impossible!",
		    &MechWeapons[weapindx].name[3], mapx, mapy,
		    baseToHit));
	    return;
	}
    }
    if (
/* #ifdef DFM_AFFECT_BTH */
	(mode & DEADFIRE_MODE)
/* #endif */
	    ) {
	r1 = Number(1, 6);
	r2 = Number(1, 6);
	r3 = Number(1, 6);
/*	SendDebug(tprintf("DFM BTH : One : %d Two : %d Three : %d", r1, r2, r3)); */
  /* Sort 'em to ascending order */
	if (r1 > r2)
	    Swap(r1, r2);
	if (r2 > r3)
	    Swap(r2, r3);
	roll = r1 + r2;
/*	SendDebug(tprintf("DFM BTH : Lowest %d %d - Total %d", r1, r2, roll)); */
    } else {
        roll = Roll();
    }
    if (target && MechType(mech) == CLASS_BSUIT && (target->mynum == MechSwarmTarget(mech)))
 	if (roll < baseToHit)
	    roll = baseToHit;
    buf[0] = 0;
    if (LOS)
	sprintf(buf, "Roll: %d ", roll);
    if (target && !ishex) {
	mech_notify(mech, MECHALL,
	    tprintf("You fire %s at %s%s - BTH: %d  %s%s",
		&MechWeapons[weapindx].name[3], GetMechToMechID(mech,
		    target), buf2, baseToHit, buf,
		MechStatus(target) & PARTIAL_COVER ? "(Partial cover)" :
		""));
	SendAttacks(tprintf("#%i attacks #%i (weapon) (%i/%i)", mech->mynum,
		target->mynum, RbaseToHit, roll));
	if (MechStatus2(target) & ATTACKEMIT_MECH)
	    SendAttackEmits(tprintf("#%i attacks #%i (weapon) (%i/%i)", mech->mynum, target->mynum, RbaseToHit, roll));
    } else {
	mech_notify(mech, MECHALL,
	    tprintf("You fire %s %s (%d,%d) - BTH: %d  %s",
		&MechWeapons[weapindx].name[3], hex_target_id(mech), mapx,
		mapy, baseToHit, buf));
	SendAttacks(tprintf("#%i attacks %d,%d (%s) (weapon) (%i/%i)",
		mech->mynum, mapx, mapy, short_hextarget(mech), RbaseToHit, roll));
	{
	MECH *tmpmech;
	int foo;

	for (foo = 0; foo < mech_map->first_free; foo++)
	    if (mech_map->mechsOnMap[foo] >= 0) {
		if (!(tmpmech = getMech(mech_map->mechsOnMap[foo])))
		    continue;
		if (mech->mynum == tmpmech->mynum)
		    continue;
		if (MechX(tmpmech) != mapx && MechY(tmpmech) != mapy)
		    continue;
		if (MechStatus2(tmpmech) & ATTACKEMIT_MECH)
		    SendAttackEmits(tprintf("#%i attacks %d,%d (%s) (weapon) (%i/%i)",
			mech->mynum, mapx, mapy, short_hextarget(mech), RbaseToHit, roll));
	    }
	}
    }
    if (roll < baseToHit && (wspec & STREAK)) {
        if ((target) && (mech) && !(mech_map->LOSinfo[mech->mapnumber][target->mapnumber] & MECHLOSFLAG_ANGELECM)) {
	    SetRecyclePart(mech, section, critical, GunStat(weapindx, mode, GUN_VRT));
	    mech_notify(mech, MECHALL, "Your streak fails to lock on");
	    return;
	} /* else {
            SetRecyclePart(mech, section, critical, GunStat(weapindx, mode, GUN_VRT));
            mech_notify(mech, MECHALL, "Your streak fails to lock on");
            return;
	} */
    }
    if (OODing(mech)) {
	if (MechZ(mech) > MechElevation(mech)) {
	    if (MechJumpSpeed(mech) >= MP1) {
		mech_notify(mech, MECHALL,
		    "You initiate your jumpjets to compensate for the opened cocoon!");
		MechCocoon(mech) = -1;
	    } else {
		mech_notify(mech, MECHALL,
		    "Your action splits open the cocoon - have a nice fall!");
		MechLOSBroadcast(mech,
		    "starts plummeting down, as the cocoon opens!.");
		MechCocoon(mech) = 0;
		StopOOD(mech);
		MECHEVENT(mech, EVENT_FALL, mech_fall_event, FALL_TICK,
		    -1);
	    }
	}
    }
    if (!isarty)
	MechFireBroadcast(mech, ishex ? NULL : target, mapx, mapy,
	    mech_map, &MechWeapons[weapindx].name[3], (roll >= RbaseToHit)
	    && range_ok);
    if (target) {
	if (InLineOfSight(target, mech, MechX(mech), MechY(mech), range))
	    mech_notify(target, MECHALL,
		tprintf("%s has fired a %s at you!",
		    GetMechToMechID(target, mech),
		    &MechWeapons[weapindx].name[3]));
	else
	    mech_notify(target, MECHALL,
		tprintf("Something has fired a %s at you from bearing %d!",
		    &MechWeapons[weapindx].name[3],
		    FindBearing(MechFX(target), MechFY(target),
			MechFX(mech), MechFY(mech))));
    }
    if (!(sight) && ((wspec & DAR) || (wspec & DLR) || strstr(MechWeapons[weapindx].name, "Hyper") || (mode & CASELESS_MODE))) {
	smokebrg = (MechFacing(mech) + ((MechStatus(mech) & TORSO_RIGHT) ? 60 : (MechStatus(mech) & TORSO_LEFT) ? -60 : 0)) - 180;
	smokebrg = (smokebrg < 0 ? smokebrg + 360 : smokebrg);
	MapCoordToRealCoord(MechX(mech), MechY(mech), &fsmokex1, &fsmokey1);
	FindXY(fsmokex1, fsmokey1, smokebrg, 1, &fsmokex, &fsmokey);
	RealCoordToMapCoord(&smokex, &smokey, fsmokex, fsmokey);

/* Something wrong in here with the output of FindXY it present a negative value for either x or y if you are close to the Mapedge.
Kain  -- DJ -> Added broader fix for the othe rmapside. Need to delve into events of smoke when i do VTOL event strafe code - 9/08/00
*/
	if (smokex < 0 || smokex > (mech_map->map_width - 1))
		smokex = 1;
	if (smokey < 0 || smokey > (mech_map->map_height - 1))
		smokey = 1;
        add_decoration(mech_map, smokex, smokey, TYPE_SMOKE, SMOKE, (GunStat(weapindx, mode, GUN_DAMAGE) * 2));
	}

    if (target) {
	int is_miss = 0;

/*	if (ishex == 1 && !isarty && !LOS)
	    if (Number(1, 57) >= 20)
		is_miss = 1; */ /* Rude fingon code */
	if (IsMissile(weapindx) && !is_miss) {
	    HitTarget(mech, weapindx, target, LOS, (mode & INFERNO_MODE) ? -2 : ((wspec & NARC) && (mode & NARC_MODE)) ? -1 : 0, 0,
		((mode & SWARM_MODE) ? -1 : (mode & SWARM1_MODE) ? (ECMblock ? -1 : -2) : 0), ((((mode & ARTEMIS_MODE) ?
		 FindArtemisForWeapon(mech, section, critical) : 0) || ((mode & NARC_MODE) && ((MechStatus(target) & NARC_ATTACHED) ||
		(MechStatus2(target) & INARC_ATTACHED))) ||
		(wspec & ATM)) && !ECMblock), modifier,
		 roll >= RbaseToHit && range_ok, RbaseToHit, 0, mode, section, critical, roll, swarmhit);
		} else if (((temproll = Roll()) == 2 && ((mode & (ULTRA_MODE|CASELESS_MODE)) || (strstr(MechWeapons[weapindx].name, "Hyper")))) ||
			  (temproll <= 3 && (mode & ROTFOUR_MODE)) ||
		          (temproll <= 4 && ((mode & ROTSIX_MODE) || ((mode & ULTRA_MODE) && (wspec & RFM))))) {
		    if (!(wspec & RFMMG) && !(mode & CASELESS_MODE) && (((mode & (ULTRA_MODES)) && (!(wspec & RFM))) || ((wspec & RFM) && (temproll > 2))))
			{
/*			SendDebug(tprintf("UltraJam : #%d Section : %d Critical : %d Roll : %d Ultra/Rot2 : %d Rot4 : %d Rot6 : %d RFM : %d",
				mech->mynum, section, critical, temproll, (mode & ULTRA_MODE) && 1, (mode & ROTFOUR_MODE) && 1,
					(mode & ROTSIX_MODE) && 1, ((mode & ULTRA_MODE) && (wspec & RFM)))); */
			mech_notify(mech, MECHALL,
				tprintf("Your ammo feed jams on your %s!", &(MechWeapons[weapindx].name[3])));
			GetPartMode(mech, section, WeaponFirstCrit(mech, section, critical)) |= JAMMED_MODE;
			isjam = 1;
			} else {
			mech_notify(mech, MECHALL, tprintf("Your ammo feed jams on your %s!", &(MechWeapons[weapindx].name[3])));
			GetPartMode(mech, section, WeaponFirstCrit(mech, section, critical)) |= JAMMED_MODE;
			isjam = 1;
			if (((strstr(MechWeapons[weapindx].name, "Hyper") || (mode & CASELESS_MODE))
			    && (Number(1,12) >= 8)) || ((mode & ULTRA_MODE) && (wspec & (RFM|RFMMG)) && (temproll == 2)))
				{
				mech_notify(mech, MECHALL,
				tprintf("You hear an odd clicking noise from your %s before the round in it's feed explodes!",
				&(MechWeapons[weapindx].name[3])));
/*				DamageMech(mech, mech, 0, -1, section, 0, 0, 0, (GunStat(weapindx, mode, GUN_DAMAGE)),
        	           		-1, 7, 0); */
				DestroyWeapon(mech, section, GetPartType(mech, section, critical), GetWeaponCrits(mech, weapindx), critical);
				}
			}
	} else if (roll >= RbaseToHit && range_ok && !is_miss && !isjam) {
            HitTarget(mech, weapindx, target, LOS, (mode & INFERNO_MODE) ? -2 : ((wspec & NARC) &&
		(mode & NARC_MODE)) ? -1 : ((mode & (ULTRA_MODES)) && (!(wspec & RFMMG))),
		(mode & ON_TC), (mode & LBX_MODE) ? 1 : (mode & PIERCE_MODE) ? 2 : 0,
		((mode & ARTEMIS_MODE) || ((mode & NARC_MODE) && ((MechStatus(target) & NARC_ATTACHED) || (MechStatus2(target) & INARC_ATTACHED)))), 
		modifier, 1, RbaseToHit, (rfmmg = (((wspec & RFMMG) && (mode & (ULTRA_MODES))) ? Number(1,(MechWeapons[weapindx].special & HVYW) ? 12 : 6) :
		(mode & HEAT_MODE) ? -1 : (mode & TRACER_MODE) ? -2 : 0)), mode, section, critical, roll, swarmhit);
	} else if (range_ok)
	    possibly_ignite_or_clear(mech, weapindx, mapx, mapy, 10,
		(mode & INFERNO_MODE) ? -1 : 4);
    } else {
	if (isarty)
	    artillery_shoot(mech, mapx, mapy, weapindx, mode, RbaseToHit <= roll);
	else
	    hex_hit(mech, mapx, mapy, weapindx, (mode & INFERNO_MODE) ? -1 : roll - RbaseToHit, mode);
    }
	/* Multiple crit damage effects */
    if (GetPartDamage(mech, section, critical) & MTX_JAM) {
	if (IsAutocannon(weapindx) && (Roll() < MTX_JAM_MOD(GetPartDamage(mech, section, critical)))) {
	    mech_notify(mech, MECHALL, tprintf("Your %s jams its ammo feed!", &MechWeapons[weapindx].name[3]));
	    GetPartMode(mech, section, WeaponFirstCrit(mech, section, critical)) |= JAMMED_MODE;
	    }
	}
    if (GetPartDamage(mech, section, critical) & (MTX_BOOM)) {
	if (IsEnergy(weapindx)  && (Roll() < MTX_BOOM_MOD(GetPartDamage(mech, section, critical)))) {
		mech_notify(mech, MECHALL, tprintf("Your %s overloads and discharges!", &MechWeapons[weapindx].name[3]));
		DamageMech(mech, mech, 0, -1, section, 0, 0, 0, GunStat(weapindx, mode, GUN_DAMAGE), -1, 0, 0);
		SlagWeapon(mech, mech, 0, section, critical, GetPartType(mech, section, critical));
	} else if ((IsAutocannon(weapindx) || IsArtillery(weapindx) || IsMissile(weapindx)) && 
			(Roll() < MTX_BOOM_MOD(GetPartDamage(mech, section, critical)))) {
		mech_notify(mech, MECHALL, "You hear an odd clicking noise then a thunk....");
		HandleMechCrit(mech, mech, 0, MechWeapons[weapindx].special & GAUSS ? section : ammoLoc,
			MechWeapons[weapindx].special & GAUSS ? critical : ammoCrit, GetPartType(mech, section, critical),
			GetPartData(mech, section, critical), 0, 1);
	}
    }

    /* Handle Heavy Gauss shoot-and-trip-over-heels features - DJ */

    if (!sight && MechSpeed(mech) > 0 && MechWeapons[weapindx].special & GAUSS && strstr(MechWeapons[weapindx].name, "Heavy"))
	{
	mech_notify(mech, MECHALL, "Your not braced for the recoil of such a large weapon!");
	if (!MadePilotSkillRoll(mech, (MechTons(mech) <= 35 ? 2 : MechTons(mech) <= 55 ? 1 : MechTons(mech) <= 75 ? 0 : -1)))
		{
		mech_notify(mech, MECHALL, "The tremendous force knocks your over!");
		MechFalls(mech, 1, 0);
		}
	}

    /* Handle PPC special case rules. Let the thing hit if a fail doest result - DJ */
      if (strstr(MechWeapons[weapindx].name, "PPC") && wtype == TBEAM && !sight)  {
        if (!is_aero(mech) && range < (GunStat(weapindx, mode, GUN_MINRANGE)) && (mode & HOTLOAD_MODE)) {
            temproll = Roll();
	    temp = (range < 1 ? 10 : range < 2 ? 6 : range < 3 ? 3 : 2);
	    if (temproll < temp) {
		mech_notify(mech, MECHALL,
		    tprintf("The particle feedback rips your %s apart!", &MechWeapons[weapindx].name[3]));
		DestroyWeapon(mech, section, GetPartType(mech, section, critical),
			MechWeapons[weapindx].criticals, WeaponFirstCrit(mech, section, critical));
                DamageMech(mech, mech, 0, -1, section, 0, 0, 0, GunStat(weapindx, mode, GUN_DAMAGE), -1, 0, 0);
		}
	    } else if (mode & CAPPED_MODE) {
            temproll = Roll();
            if (temproll <= 2) {
                mech_notify(mech, MECHALL,
                    tprintf("The massive energy channelling rips your %s apart!", &MechWeapons[weapindx].name[3]));
                DestroyWeapon(mech, section, GetPartType(mech, section, critical),
                        MechWeapons[weapindx].criticals, WeaponFirstCrit(mech, section, critical));
/*                DamageMech(mech, mech, 0, -1, section, 0, 0, 0, GunStat(weapindx, mode, GUN_DAMAGE), -1, 0, 0); */
                }
            GetPartMode(mech, section, (temp = (FindCapacitorForWeapon(mech, section, critical))) - 1) &= ~CAPPED_MODE;
            GetPartMode(mech, section, critical) &= ~CAPPED_MODE;
	    if (!NumCapacitorsOn(mech))
	        MechStatus2(mech) &= ~CAPACITOR_ON;
	    MechWeapHeat(mech) += (float) 5.;
	    }
	}
    /* Set recycle times */
    SetRecyclePart(mech, section, critical, GunStat(weapindx, mode, GUN_VRT));

    if (MechSpecials(mech) & ICE_TECH) {
	if (MechWeapons[weapindx].type == TAMMO && !IsCoolant(weapindx) && !IsAcid(weapindx))
	    MechWeapHeat(mech) += (float) (GunStat(weapindx, mode, GUN_HEAT) + MTX_HEAT_MOD(GetPartDamage(mech, section, critical))) / 2;
	else if (IsFlamer(weapindx))
	    MechWeapHeat(mech) += (float) (GunStat(weapindx, mode, GUN_HEAT) + MTX_HEAT_MOD(GetPartDamage(mech, section, critical))) * 2;
	else
	    MechWeapHeat(mech) += (float) (GunStat(weapindx, mode, GUN_HEAT) + MTX_HEAT_MOD(GetPartDamage(mech, section, critical)));
	} else {
	    MechWeapHeat(mech) += (float) (GunStat(weapindx, mode, GUN_HEAT) + MTX_HEAT_MOD(GetPartDamage(mech, section, critical)));
	}

/* We Need You Not - DJ */
/*    if (mode & (ULTRA_MODES))
        {
	MechWeapHeat(mech) += (float) GunStat(weapindx, mode, GUN_HEAT);
	if (type == HEAT)
	    MechWeapHeat(mech) += (float) modifier;
        } */
    if (rfmmg > 0)
	MechWeapHeat(mech) += rfmmg / 2;

    /* Decrement Ammunition */
    /* Let's make this a tad more dynamic, but without inducing a migraine - DJ */
/*    if (!isjam) {
     if (rfmmg > 0)
	{
	if ((GetPartData(mech, ammoLoc, ammoCrit) - (rfmmg * 3)) < 0) {
		GetPartData(mech, ammoLoc, ammoCrit) = 0;
		} else {
		GetPartData(mech, ammoLoc, ammoCrit) = (GetPartData(mech, ammoLoc, ammoCrit) - (rfmmg * 3));
		}
        } else {
        decrement_ammunition(mech, weapindx, section, critical, ammoLoc, ammoCrit);
	}
    } */
    if (!isjam && wtype != TBEAM && wtype != THAND) {
        if (GetPartMode(mech, section, critical) & OS_MODE) {
	    decrement_ammunition(mech, weapindx, section, critical, ammoLoc, ammoCrit);
	} else {
	ammoused = (rfmmg > 0 ? (rfmmg * 3) : (mode & (ULTRA_MODES)) ?
	           ((mode & ROTSIX_MODE) ? 6 : (mode & ROTFOUR_MODE) ? 4 : 2) : 1);
	ammo_expedinture_check(mech, weapindx, ammoused);
        while (ammoused) {
	    if (GetPartData(mech, ammoLoc, ammoCrit) <= 0) {
	        FindAndCheckAmmo(mech, weapindx, section, critical, &ammoLoc, &ammoCrit);
		mode = GetPartMode(mech, section, critical);
		}
	    decrement_ammunition(mech, weapindx, section, critical, ammoLoc, ammoCrit);
	    ammoused--;
	    }
	  }
	}

    /* If we're hotloading, jam the weapon */
    /* Now for my new version to demunch - DJ */
    if (HotLoading(weapindx, mode)) {
	if (Roll() >= 10) {
	    mech_notify(mech, MECHALL, tprintf ("%%ch%%crThe %s rack can't keep up with the hotloading and jams!%%cn", &MechWeapons[weapindx].name[3])); 
	    GetPartMode(mech, section, critical) |= JAMMED_MODE;
	    }
    }
/* Old cheese hotload code removed - DJ */
}

void HitTarget(MECH * mech, int weapindx, MECH * hitMech, int LOS,
int ultra_mode, int on_tc, int lbx_mode, int plus2,
int modifier, int reallyhit, int bth, int rfmmg, int mode, int section, int critical, int realroll,
int swarmhit)
{
int isrear = 0, iscritical = 0;
int hitloc, suitroll, suit;
int loop;
int roll;
int aim_hit = 0;
int num_missiles_hit;
float range = (FaMechRange(mech, hitMech));
int rbth = (FindBaseToHitByRange(weapindx, range, mode, 1, 0));
int dweap = (((MechWeapons[weapindx].special & GAUSS) && (strstr(MechWeapons[weapindx].name,"Heavy")) && rbth > 0) ?
	     (rbth <= 2 ? (GunStat(weapindx, mode, GUN_DAMAGE) - 5) : (GunStat(weapindx, mode, GUN_DAMAGE) - 15)) :
		GunStat(weapindx, mode, GUN_DAMAGE));
int pmod = (dweap <= 2 ? 5 : dweap <= 5 ? 4 : dweap <= 10 ? 3 : 2);

    if (GetPartMode(mech, section, critical) & MTX_TODAM)
	dweap = MAX(1, dweap - MTX_TODAM_MOD(GetPartDamage(mech, section, critical)));

    if (mudconf.btech_glance && (bth == realroll)) {
	dweap = ((dweap + 1) / 2);
	if (dweap < 1)
	    dweap = 1;
        MechLOSBroadcast(hitMech, "is nicked by a glancing blow!");
        mech_notify(hitMech, MECHALL, "You are nicked by a glancing blow!");
	}
    if ((InWater(hitMech) && (MechZ(hitMech) <= ((WaterBeast(hitMech) || Fallen(hitMech)) ? -1 : -2)))
	&& !(InWater(mech) && (MechZ(mech) <= ((WaterBeast(mech) || Fallen(mech)) ? -1 : -2)))
	&& (MechWeapons[weapindx].type & (TMISSILE | TAMMO))) {
	mech_notify(mech, MECHALL, tprintf("Your %s fire splashes into the water harmlessly!",
		&(MechWeapons[weapindx].name[3])));
	mech_notify(hitMech, MECHALL, tprintf("You notice %s fire splashing harmless above you!",
		&(MechWeapons[weapindx].name[3])));
	return;
	}
/*    if (rfmmg == -2)
	DamMod++; */
    if (MechAim(mech) != NUM_SECTIONS && (!Started(hitMech) || Uncon(hitMech) || Blinded(hitMech) || on_tc)) {
	roll = Roll();
	if (MechAim(mech) == HEAD && roll >= 8)
	    aim_hit = 1;
	else if (roll == 6 || roll == 7 || roll == 8)
	    aim_hit = 1;
    }
    if (!IsArtillery(weapindx) && !IsMissile(weapindx) && (ultra_mode <= 0 && (lbx_mode == 1 ? 0 : 1))) {
	if (swarmhit != 0) {
	    if (swarmhit < 0)
		isrear = 1;
	    hitloc = abs(swarmhit) - 1;
	} else if (aim_hit && !on_tc)
	    hitloc = FindAimHitLoc(mech, hitMech, &isrear, &iscritical);
	else if (aim_hit && on_tc)
	    hitloc = FindTCHitLoc(mech, hitMech, &isrear, &iscritical);
	else
	    hitloc = FindTargetHitLoc(mech, hitMech, &isrear, &iscritical);
	if (rfmmg == -1 && IsFlamer(weapindx)) {
	    dweap = pc_to_dam_conversion(hitMech, weapindx, dweap * 10);
	    if (MechSpecials(hitMech) & ICE_TECH || MechType(hitMech) == CLASS_VEH_GROUND)
		dweap /= 2;
  	    if (MechType(hitMech) == CLASS_MECH) {
	        MechWeapHeat(hitMech) += (float) dweap / 10;
	        mech_notify(hitMech, MECHALL, tprintf("%%ch%%cyYou have been hit for %d points of heat%%c", (dweap)));
		}
	    if (MechSpecials(hitMech) & ICE_TECH || MechType(hitMech) == CLASS_VEH_GROUND) {
		mech_notify(hitMech, MECHALL, tprintf("%%ch%%cyYou lose some speed!%%c"));
		if (fabs(MechSpeed(hitMech)) < dweap)
		    MechSpeed(hitMech) = 0;
		else
		    MechSpeed(hitMech) -= dweap * (MechSpeed(hitMech) > 0 ? 1 : -1);
		}
	    mech_notify(mech, MECHALL, tprintf("%%ch%%cyYou hit for %d points of heat%%c", (dweap)));
    	    } else if (IsCoolant(weapindx) && MechType(hitMech) != CLASS_MW) {
		if (rfmmg == -1) {
			mech_notify(mech, MECHALL, "%ccCoolant washes over your systems!!%c");
			MechWeapHeat(mech) -= (float) dweap;
		} else {
			mech_notify(mech, MECHALL, "%ccYou hit with the stream of coolant!!%c");
			mech_notify(hitMech, MECHALL, "%ccCoolant washes over your systems!!%c");
			MechWeapHeat(hitMech) -= (float) dweap;
		}
		AcidClear(hitMech, hitloc, 1);
	    } else {
                if (IsAcid(weapindx))
                    StartAcidEffect(mech, hitMech, hitloc, 1, 1, 1);
                DamageMech(hitMech, mech, LOS, GunPilot(mech), hitloc, isrear,
                    (lbx_mode == 2 ? pmod : iscritical), pc_to_dam_conversion(hitMech, weapindx,
                    (rfmmg > 0 ? (mudconf.btech_glance && bth == realroll ?
                    MAX(1, rfmmg / 2) : rfmmg) : dweap)),
                    0, weapindx, bth, bth == realroll); 
		if ((IsSpooge(weapindx) || IsMG(weapindx)) && MechType(hitMech) == CLASS_BSUIT) {
		    for (suit = 0; suit < NUM_BSUIT_MEMBERS; suit++) {
			if (suit == hitloc || GetSectInt(hitMech, suit) <= 0)
			    continue;
			suitroll = Roll();
			mech_notify(mech, MECHALL, tprintf("You train your weapon at Suit #%d... BTH: %d Roll: %d.", suit + 1, bth + 1, suitroll));
			if (suitroll >= bth) {
			    if (IsAcid(weapindx))
				StartAcidEffect(mech, hitMech, suit, 1, 1, 1);
			    if (suitroll == bth) {
				dweap /= 2;
				mech_notify(mech, MECHALL, tprintf("You barely nick suit #%d!", suit + 1));
				}
			    DamageMech(hitMech, mech, LOS, GunPilot(mech), suit, isrear,
				(lbx_mode == 2 ? pmod : iscritical), pc_to_dam_conversion(hitMech, weapindx,
				(rfmmg > 0 ? (mudconf.btech_glance && bth == realroll ?
				MAX(1, rfmmg / 2) : rfmmg) : MAX(1, dweap))),
				0, weapindx, bth, bth == realroll);

			    }
			} 
		    }
		}
	return;
    }
    for (loop = 0; MissileHitTable[loop].key != -1; loop++)
	if (MissileHitTable[loop].key == weapindx)
	    break;
    if (!(MissileHitTable[loop].key == weapindx))
	return;
    if (IsMissile(weapindx)) {
	if (lbx_mode < 0)
	    SwarmHitTarget(mech, weapindx, hitMech, LOS, bth,
		reallyhit ? bth + 1 : bth - 1, plus2, ultra_mode != -1,
		ultra_mode == -2,
		MissileHitTable[loop].num_missiles[10],
		lbx_mode == -2, mode, section, critical, realroll);
	else
	    MissileHitTarget(mech, weapindx, hitMech, LOS ? 1 : 0, bth,
		reallyhit ? bth + 1 : bth - 1, plus2, ultra_mode != -1,
		ultra_mode == -2,
		MissileHitTable[loop].num_missiles[10], mode, realroll);
	return;
    }
    num_missiles_hit =
	MissileHitTable[loop].num_missiles[MissileHitIndex(mech, hitMech,
	    weapindx, plus2, mode, bth == realroll)];
    /* Missile weapon.  Multiple Hit locations... */
    if (mode & ROTSIX_MODE)
	num_missiles_hit = (Number(1,6));
    if (mode & ROTFOUR_MODE)
	num_missiles_hit = (Number(1,4));
    if (LOS) {
	if (ultra_mode > 0 || lbx_mode == 1)
	    mech_notify(mech, MECHALL,
		tprintf("%%cgYou hit with %d projectiles!%%c",
		    num_missiles_hit));
	else
	    mech_notify(mech, MECHALL,
		tprintf("%%cgYou hit with %d missiles!%%c",
		    num_missiles_hit));
    }
    if (ultra_mode <= 0)
	Missile_Hit(mech, hitMech, isrear, iscritical, weapindx,
	    num_missiles_hit,
	    lbx_mode == 1 ? 1 : dweap,
	    Clustersize(weapindx, mode), LOS, bth, mode, 1, realroll);
    else
	while (num_missiles_hit) {
		if (swarmhit != 0) {
			if (swarmhit < 0)
				isrear = 1;
			hitloc = abs(swarmhit) - 1;
		} else if (aim_hit && !on_tc)
		hitloc = FindAimHitLoc(mech, hitMech, &isrear, &iscritical);
	    else if (aim_hit && on_tc)
		hitloc = FindTCHitLoc(mech, hitMech, &isrear, &iscritical);
	    else
		hitloc = FindTargetHitLoc(mech, hitMech, &isrear, &iscritical);
	    DamageMech(hitMech, mech, LOS, GunPilot(mech), hitloc, isrear,
		(lbx_mode == 2 ? pmod : iscritical), pc_to_dam_conversion(hitMech, weapindx,
		    dweap), 0, weapindx, bth, bth == realroll);
	    num_missiles_hit--;
	}
}

int FindAreaHitGroup(MECH * mech, MECH * target)
{
    int quadr;

    quadr =
	AcceptableDegree(FindBearing(MechFX(mech), MechFY(mech),
	MechFX(target), MechFY(target)) - MechFacing(target));
#if 0 
    if ((quadr >= 135) && (quadr <= 225))
	return FRONT;
    if ((quadr < 45) || (quadr > 315))
	return BACK;
    if ((quadr >= 45) && (quadr < 135))
	return LEFTSIDE;
    return RIGHTSIDE;
    /* These are 'official' BT arcs */
    if (quadr >= 120 && quadr <= 240)
	return FRONT;
    if (quadr < 30 || quadr > 330)
	return BACK;
    if (quadr >= 30 && quadr < 120)
	return LEFTSIDE;
    return RIGHTSIDE;
#else

    if (MechType(target) == CLASS_MECH && MechMove(target) == MOVE_BIPED) {
	if (quadr <= 30 || quadr >= 330)
	    return BACK;
	if (quadr > 30 && quadr <= 90)
	    return LEFTSIDE;
	if (quadr < 330 && quadr >= 270)
	    return RIGHTSIDE;
	return FRONT;
     } else {
	if (quadr >= 150 && quadr <= 210)
	    return FRONT;
	if (quadr < 150 && quadr > 30)
	    return LEFTSIDE;
	if (quadr > 210 && quadr < 330)
	    return RIGHTSIDE;
	return BACK;
    }
#endif
}

int FindTargetHitLoc(MECH * mech, MECH * target, int *isrear,
    int *iscritical)
{
    int hitGroup;

/*    *isrear = 0; */

    *iscritical = 0;
    hitGroup = FindAreaHitGroup(mech, target);
    if (hitGroup == BACK)
	*isrear = 1;
    if (MechType(target) == CLASS_MECH && (MechStatus(target) & PARTIAL_COVER))
	return FindPunchLocation(hitGroup, target);
    if (MechType(mech) == CLASS_MW && MechType(target) == CLASS_MECH && MechZ(mech) <= MechZ(target))
	return FindKickLocation(hitGroup, target);
    if (MechType(target) == CLASS_MECH && ((MechType(mech) == CLASS_BSUIT && MechSwarmTarget(mech) == target->mynum)))
	return FindSwarmHitLocation(iscritical, isrear);
    return FindHitLocation(target, hitGroup, iscritical, isrear, mech);
}

int FindTCHitLoc(MECH * mech, MECH * target, int *isrear, int *iscritical)
{
    int hitGroup;

    *isrear = 0;
    *iscritical = 0;
    hitGroup = FindAreaHitGroup(mech, target);
    if (hitGroup == BACK)
	*isrear = 1;
    if (MechAimType(mech) == MechType(target))
	switch (MechType(target)) {
	case CLASS_MECH:
	case CLASS_MW:
	    switch (MechAim(mech)) {
	    case RARM:
		if (hitGroup != LEFTSIDE)
		    return RARM;
		break;
	    case LARM:
		if (hitGroup != RIGHTSIDE)
		    return LARM;
		break;
	    case RLEG:
		if (hitGroup != LEFTSIDE &&
		    !(MechStatus(target) & PARTIAL_COVER)) return RLEG;
		break;
	    case LLEG:
		if (hitGroup != RIGHTSIDE &&
		    !(MechStatus(target) & PARTIAL_COVER)) return LLEG;
		break;
	    case RTORSO:
		if (hitGroup != LEFTSIDE)
		    return RTORSO;
		break;
	    case LTORSO:
		if (hitGroup != RIGHTSIDE)
		    return LTORSO;
		break;
	    case CTORSO:

/*        if (hitGroup != LEFTSIDE && hitGroup != RIGHTSIDE) */
		return CTORSO;
	    case HEAD:
		if (Roll() >= 10)
		    return HEAD;
	    }
	    break;
	case CLASS_AERO:
	case CLASS_DS:
	    switch (MechAim(mech)) {
	    case AERO_NOSE:
		if (hitGroup != BACK)
		    return AERO_NOSE;
		break;
	    case AERO_LWING:
		if (hitGroup != RIGHTSIDE)
		    return AERO_LWING;
		break;
	    case AERO_RWING:
		if (hitGroup != LEFTSIDE)
		    return AERO_RWING;
		break;
	    case AERO_AFT:
		if (hitGroup != FRONT)
		    return AERO_AFT;
		break;
	    }
	    break;
	case CLASS_VEH_GROUND:
	case CLASS_VEH_NAVAL:
	case CLASS_VEH_VTOL:
	    switch (MechAim(mech)) {
	    case RSIDE:
		if (hitGroup != LEFTSIDE)
		    return (RSIDE);
		break;
	    case LSIDE:
		if (hitGroup != RIGHTSIDE)
		    return (LSIDE);
		break;
	    case FSIDE:
		if (hitGroup != BACK)
		    return (FSIDE);
		break;
	    case BSIDE:
		if (hitGroup != FRONT)
		    return (BSIDE);
		break;
	    case TURRET:
		return (TURRET);
		break;
	    }
	    break;
	}
    if (MechType(target) == CLASS_MECH &&
	(MechStatus(target) & PARTIAL_COVER)) return
	    FindPunchLocation(hitGroup, target);
    return FindHitLocation(target, hitGroup, iscritical, isrear, mech);
}

int FindAimHitLoc(MECH * mech, MECH * target, int *isrear, int *iscritical)
{
    int hitGroup;

    *isrear = 0;
    *iscritical = 0;

    hitGroup = FindAreaHitGroup(mech, target);
    if (hitGroup == BACK)
	*isrear = 1;

    if (MechType(target) == CLASS_MECH || MechType(target) == CLASS_MW)
	switch (MechAim(mech)) {
	case RARM:
	    if (hitGroup != LEFTSIDE)
		return (RARM);
	    break;
	case LARM:
	    if (hitGroup != RIGHTSIDE)
		return (LARM);
	    break;
	case RLEG:
	    if (hitGroup != LEFTSIDE &&
		!(MechStatus(target) & PARTIAL_COVER)) return (RLEG);
	    break;
	case LLEG:
	    if (hitGroup != RIGHTSIDE &&
		!(MechStatus(target) & PARTIAL_COVER)) return (RLEG);
	    break;
	case RTORSO:
	    if (hitGroup != LEFTSIDE)
		return (RTORSO);
	    break;
	case LTORSO:
	    if (hitGroup != RIGHTSIDE)
		return (LTORSO);
	    break;
	case CTORSO:
	    return (CTORSO);
	case HEAD:
	    return (HEAD);
    } else if (is_aero(target))
	return MechAim(mech);
    else
	switch (MechAim(mech)) {
	case RSIDE:
	    if (hitGroup != LEFTSIDE)
		return (RSIDE);
	    break;
	case LSIDE:
	    if (hitGroup != RIGHTSIDE)
		return (LSIDE);
	    break;
	case FSIDE:
	    if (hitGroup != BACK)
		return (FSIDE);
	    break;
	case BSIDE:
	    if (hitGroup != FRONT)
		return (BSIDE);
	    break;
	case TURRET:
	    return (TURRET);
	    break;
	}

    if (MechType(target) == CLASS_MECH &&
	(MechStatus(target) & PARTIAL_COVER)) return
	    FindPunchLocation(hitGroup, target);
    return FindHitLocation(target, hitGroup, iscritical, isrear, mech);
}

#define BOOMLENGTH 24
char BOOM[BOOMLENGTH][80] = {
    "                              ________________",
    "                         ____/ (  (    )   )  \\___",
    "                        /( (  (  )   _    ))  )   )\\",
    "                      ((     (   )(    )  )   (   )  )",
    "                    ((/  ( _(   )   (   _) ) (  () )  )",
    "                   ( (  ( (_)   ((    (   )  .((_ ) .  )_",
    "                  ( (  )    (      (  )    )   ) . ) (   )",
    "                 (  (   (  (   ) (  _  ( _) ).  ) . ) ) ( )",
    "                 ( (  (   ) (  )   (  ))     ) _)(   )  )  )",
    "                ( (  ( \\ ) (    (_  ( ) ( )  )   ) )  )) ( )",
    "                 (  (   (  (   (_ ( ) ( _    )  ) (  )  )   )",
    "                ( (  ( (  (  )     (_  )  ) )  _)   ) _( ( )",
    "                 ((  (   )(    (     _    )   _) _(_ (  (_ )",
    "                  (_((__(_(__(( ( ( |  ) ) ) )_))__))_)___)",
    "                  ((__)        \\\\||lll|l||///          \\_))",
    "                           (   /(/ (  )  ) )\\   )",
    "                         (    ( ( ( | | ) ) )\\   )",
    "                          (   /(| / ( )) ) ) )) )",
    "                        (     ( ((((_(|)_)))))     )",
    "                         (      ||\\(|(|)|/||     )",
    "                       (        |(||(||)||||        )",
    "                         (     //|/l|||)|\\\\ \\     )",
    "                       (/ / //  /|//||||\\\\  \\ \\  \\ _)",
    "----------------------------------------------------------------------------"
};

extern int global_kill_cheat;

void KillMechContentsIfIC(dbref aRef, MECH * mech)
{
    global_kill_cheat = 1;
    if (!In_Character(aRef))
	return;
    if (mudconf.btech_ic > 0)
	tele_contents(aRef, AFTERLIFE_DBREF, TELE_XP | TELE_LOUD, mech);
    else
	tele_contents(aRef, AFTERLIFE_DBREF, TELE_LOUD, mech);
}

void DestroyMech(MECH * target, MECH * mech, int bc)
{
    int loop;
    MAP *mech_map;
    MECH *ttarget;
    AUTO *a;

    if (!target)
	return;
    if (!mech)
	mech = target;
    if (Destroyed(target))
	return;
    if (MechStall(mech) > 0)
	return;
    global_kill_cheat = 1;
    if (MechAuto(target) > 0 && (a = FindObjectsData(MechAuto(target))) != NULL) {
	StopAutoPilot(a);
	auto_disengage(a->mynum, a, "-1");
	auto_delcommand(a->mynum, a, "-1");
	} else {
	    MechAuto(target) = -1;
	}

    if (target->mynum > 0 && mech->mynum > 0) {
	SendDebug(tprintf("#%d has been killed by #%d", target->mynum, mech->mynum));
	did_it(mech->mynum, target->mynum, 0, NULL, 0, NULL, A_AMECHDEST, (char **) NULL, 0);
	}
    if (bc) {
	if (mech != target) {
	    mech_notify(mech, MECHALL, "You destroyed the target!");
	    MechLOSBroadcasti(target, mech, "has been destroyed by %s!");
	    } else {
		MechLOSBroadcast(target, "has been destroyed!");
	    }
	}
    for (loop = 0; loop < BOOMLENGTH; loop++)
	mech_notify(target, MECHALL, BOOM[loop]);
    switch (MechType(target)) {
	case CLASS_MW:
	case CLASS_BSUIT:
	    mech_notify(target, MECHALL, "You have been killed!");
	    break;
	default:
	    mech_notify(target, MECHALL, "You have been destroyed!");
	    break;
	}
    mech_map = getMap(target->mapindex);
    if (MechCarrying(target) > 0) {
	if ((ttarget = getMech(MechCarrying(target)))) {
	    mech_notify(ttarget, MECHALL, "Your tow lines go suddenly slack!");
	    mech_dropoff(GOD, target, "");
	    }
	}
    /* shut it down */
    DestroyAndDump(target);
    if (MechType(target) == CLASS_MW) {
	if (In_Character(target->mynum)) {
	    KillMechContentsIfIC(target->mynum, target);
	    discard_mw(target);
	}
    }
}

static char *MyColorStrings[] = { "", "%ch%cg", "%ch%cy", "%cr" };

static char *MyMessageStrings[] = {
    "ERROR%c",
    "low.%c",
    "critical!%c",
    "BREACHED!%c"
};

static inline char *MySeriousColorStr(MECH * mech, int index)
{
    return MyColorStrings[index % 4];
}

static inline char *MySeriousStr(MECH * mech, int index)
{
    return MyMessageStrings[index % 4];
}

static inline int MySeriousnessCheck(MECH * mech, int hitloc)
{
    int orig, new;

    if (!(orig = GetSectOArmor(mech, hitloc)))
	return 0;
    if (!(new = GetSectArmor(mech, hitloc)))
	return 3;
    if (new < orig / 4)
	return 2;
    if (new < orig / 2)
	return 1;
    return 0;
}

static inline int MySeriousnessCheckR(MECH * mech, int hitloc)
{
    int orig, new;

    if (!(orig = GetSectORArmor(mech, hitloc)))
	return 0;
    if (!(new = GetSectRArmor(mech, hitloc)))
	return 3;
    if (new < orig / 4)
	return 2;
    if (new < orig / 2)
	return 1;
    return 0;
}


int cause_armordamage(MECH * wounded, MECH * attacker, int LOS,
    int attackPilot, int isrear, int iscritical, int hitloc, int damage,
    int *crits, int glance, int swarmhit)
{
    int intDamage = 0, r;
    int seriousness = 0;
    int sectarmor = ((isrear ? GetSectRArmor(wounded, hitloc) : GetSectArmor(wounded, hitloc)) - damage);
    int tmp = 0;
    char locname[20];
    int critok = 0;

    if (MechType(attacker) == CLASS_BSUIT && MechSwarmTarget(attacker) == wounded->mynum && (swarmhit == 1))
    	critok = 1;

    if (MechType(wounded) == CLASS_MW)
	return (damage > 0) ? damage : 0;
    if ((iscritical > 1) && (sectarmor < 0))
		iscritical = 1;
    if ((MechSpecials(wounded) & HARDA_TECH) && damage > 0)
	{
	if (iscritical <= 1) {
		tmp = damage;
		damage = ((damage + 1) / 2);
		if (damage > GetSectArmor(wounded, hitloc))
		    tmp = (tmp - (GetSectArmor(wounded, hitloc) * 2));
	}
	if (iscritical > 1)
		iscritical = 1;
	}
    /* Now decrement armor, and if neccessary, handle criticals... */
    if (MechType(wounded) == CLASS_MECH && isrear && (hitloc == CTORSO ||
	    hitloc == RTORSO || hitloc == LTORSO)) {
	intDamage = damage - GetSectRArmor(wounded, hitloc);
	if (intDamage > 0) {
	    SetSectRArmor(wounded, hitloc, 0);
	    if (intDamage != damage)
		seriousness = 3;
	} else {
	    seriousness = MySeriousnessCheckR(wounded, hitloc);
	    SetSectRArmor(wounded, hitloc, GetSectRArmor(wounded,
		    hitloc) - damage);
	    seriousness =
		(seriousness == MySeriousnessCheckR(wounded,
	  hitloc)) ? 0 : MySeriousnessCheckR(wounded, hitloc);
	}
    } else {
	/* Silly stuff */
	/*
	   SetSectArmor(wounded, hitloc, MAX(0, intDamage =  GetSectArmor(wounded, hitloc) - damage));
	   intDamage = abs(intDamage);
	 */
	intDamage = damage - GetSectArmor(wounded, hitloc);
	if (intDamage > 0) {
	    SetSectArmor(wounded, hitloc, 0);
	    if (is_aero(wounded)) {
			DamageAeroSI(wounded, (int) ceil((float) intDamage / 2), attacker);
			(*crits) += 1;
			iscritical = 1;
			if (AeroSI(wounded) <= 0)
            		    return intDamage / 2;
		}
	    if (intDamage != damage)
		seriousness = 3;
	} else {
	    if (is_aero(wounded) && GetSectArmor(wounded, hitloc) == 0) {
			DamageAeroSI(wounded, (int) ceil((float) intDamage / 2), attacker);
			(*crits) += 1;
			iscritical = 1;
                        if (AeroSI(wounded) <= 0)
                            return damage / 2;
	    } else {
	        seriousness = MySeriousnessCheck(wounded, hitloc);
	        SetSectArmor(wounded, hitloc, GetSectArmor(wounded,
		        hitloc) - damage);
	        seriousness =
		    (seriousness == MySeriousnessCheck(wounded,
	      	    hitloc)) ? 0 : MySeriousnessCheck(wounded, hitloc);
		}
	}

	if (!GetSectArmor(wounded, hitloc))
	    MechFloodsLoc(wounded, hitloc, MechZ(wounded));
    }
    if (iscritical || critok) {
	if (iscritical > 1 && !(MechSpecials(wounded) & HARDA_TECH)) {
	    r = (Roll() - (iscritical - 2));
	    SendDebug(tprintf("PierceCrit - #%d attacking #%d - Roll: %d Mod: -%d", attacker->mynum, wounded->mynum, r, iscritical - 2));
	    if (!In_Character(attacker->mynum) && iscritical > 1)
	        mech_notify(attacker, MECHALL, tprintf("PierceRoll - BTH: 8 Roll: %d (Mod: -%d)", r, iscritical - 2)); 
	    stat.critrolls[(r + (iscritical - 2)) - 2]++;
	    stat.totcrolls++;
	    if (MechSpecials(wounded) & ICE_TECH)
		r += 4;
	} else {
	    r = Roll();
            stat.critrolls[r - 2]++;
	    stat.totcrolls++;
	    if ((MechSpecials(wounded) & HARDA_TECH) && (sectarmor > 0))
	        r -= 2;
            if (glance && mudconf.btech_glance)
	        r -= 2;
	    if (MechSpecials(wounded) & ICE_TECH)
 	        r += 2;
	}
	r = BOUNDED(2, r, 12);
	switch (r) {
	case 4:
	case 5:
	case 6:
	case 7:
	    if (MechSpecials2(wounded) & FRAGILE_TECH) {
		HandleCritical(wounded, attacker, LOS, hitloc, 1, 1, 1);
		(*crits) += 1;
	    }
	    break;
	case 8:
	case 9:
	    HandleCritical(wounded, attacker, LOS, hitloc, 1, 1, 1);
	    (*crits) += 1;
	    break;
	case 10:
	case 11:
	    HandleCritical(wounded, attacker, LOS, hitloc, 2, 1, 1);
	    (*crits) += 2;
	    break;
	case 12:
	    HandleCritical(wounded, attacker, LOS, hitloc, 3, 1, 1);
	    (*crits) += 3;
	    break;
	default:
	    break;
	}
	iscritical = 0;
    }

    if (seriousness > 0) {
	if (hitloc < 0) hitloc = 0;
	mech_notify(wounded, MECHALL, tprintf("%sWARNING: %s%s Armor %s",
		MySeriousColorStr(wounded, seriousness),
		ShortArmorSectionString(MechType(wounded), MechMove(wounded), hitloc),
		isrear ? " (Rear)" : "",
		MySeriousStr(wounded, seriousness)));
	}
    if ((MechSpecials(wounded) & HARDA_TECH) && intDamage > 0)
        return (tmp ? tmp : intDamage * 2);
    else
	return intDamage > 0 ? intDamage : 0;
}

int cause_internaldamage(MECH * wounded, MECH * attacker, int LOS,
    int attackPilot, int isrear, int hitloc, int intDamage, int weapindx,
    int *crits, int glance, int swarmhit)
{
    int r = Roll();
    int critok = 1;

    if ((MechSpecials(wounded) & REINFI_TECH) && intDamage > 0) {
	intDamage = (intDamage + 1) / 2;
	r = (r - 1);
    } else if (MechSpecials(wounded) & COMPI_TECH) {
	intDamage = intDamage * 2;
    }
    if (glance && mudconf.btech_glance)
	r = (r - 2);
    if (MechSpecials(wounded) & ICE_TECH)
        r += 2;
    r = BOUNDED(2, r, 12);
    /* Critical hits? */
    stat.critrolls[r - 2]++;
    stat.totcrolls++;
    if (!(*crits) && critok) {
        if (MechType(wounded) == CLASS_VEH_GROUND || MechType(wounded) == CLASS_VEH_NAVAL ||
	    MechType(wounded) == CLASS_VTOL) {
	      HandleCritical(wounded, attacker, LOS, hitloc, 1, 1, 0);
	} else {
	switch (r) {
	case 4:
	case 5:
	case 6:
	case 7:
	    if (MechSpecials2(wounded) & FRAGILE_TECH)
		HandleCritical(wounded, attacker, LOS, hitloc, 1, 1, 0);
	    break;
	case 8:
	case 9:
	    HandleCritical(wounded, attacker, LOS, hitloc, 1, 1, 0);
	    break;
	case 10:
	case 11:
	    HandleCritical(wounded, attacker, LOS, hitloc, 2, 1, 0);
	    break;
	case 12:
	    switch (hitloc) {
	    case RARM:
	    case LARM:
	    case RLEG:
	    case LLEG:
		/* Limb blown off */
		mech_notify(wounded, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		mech_notify(wounded, MECHALL, "The lucky shot sheers your limb off!");
		DestroySection(wounded, attacker, LOS, hitloc);
		if (MechType(wounded) != CLASS_MW)
		    intDamage = 0;
		break;
	    default:
		/* Ouch */
		HandleCritical(wounded, attacker, LOS, hitloc, 3, 1, 0);
		break;
	    }
	default:
	    break;
	    /* No critical hit */
	}
	}
      }
    /* Hmm.. This should be interesting */
    if (MechType(wounded) == CLASS_MECH && intDamage &&
	GetSectInt(wounded, hitloc) == GetSectOInt(wounded, hitloc))
	MechBoomStart(wounded) = event_tick;
    if (GetSectInt(wounded, hitloc) <= intDamage) {
	intDamage -= GetSectInt(wounded, hitloc);
	DestroySection(wounded, attacker, LOS, hitloc);
/*    if (Destroyed(wounded)) */

/*      intDamage = 0;        */
    } else {
	SetSectInt(wounded, hitloc, GetSectInt(wounded,	hitloc) - intDamage);
	intDamage = 0;
    }
    if ((MechSpecials(wounded) & REINFI_TECH) && intDamage > 0)
        intDamage = (intDamage * 2);
    else if (MechSpecials(wounded) & COMPI_TECH)
        intDamage = ((intDamage + 1) / 2) ;
    return intDamage;
}

int global_physical_flag = 0;

void DamageMech(MECH * wounded, MECH * attacker, int LOS, int attackPilot,
    int hitloc, int isrear, int iscritical, int damage, int intDamage,
    int cause, int bth, int glance)
{
    char locationBuff[20];
    char notificationBuff[80];
    char rearMessage[10];
    int transfer = 0;
    int was_transfer = 0;
    int kill = 0;
    MAP *map;
    int crits = 0;
    int critHit, adamage, weapindx, realloc, combatsafe = 0, i;

    /* if:
       damage = -1 && intDamage>0
       - ammo expl
       damage = -2 && intDamage>0
       - transferred ammo expl
       damage = n && intDamage = 0
       - usual damage
       damage = n && intDamage = -1/-2
       - usual damage + transfer/+red enable */
    /* if damage>0 && !intDamage usual dam. */
/*    map = getMap(attacker->mapindex); */

    if (cause > 0 && MechType(wounded) == MOVE_NONE && MechWeapons[cause].type == TARTILLERY) {
	mech_notify(attacker, MECHALL, "Your artillery is deflected by the tower.");
	return;
	}

    map = getMap(wounded->mapindex);
    if (MechIsSwarmed(wounded) == 2 && (hitloc == RTORSO || hitloc == LTORSO || (hitloc == CTORSO && isrear)) && MechType(wounded) == CLASS_BSUIT) {
	MECH *tempmech = NULL;
        int i;
        if (map && wounded) {
        for (i = 0; i < map->first_free; i++)
            {
            if (!(tempmech = FindObjectsData(map->mechsOnMap[i])) || !map)
                continue;
            if (MechType(tempmech) == CLASS_BSUIT)
                {
                if (MechSwarmTarget(tempmech) == wounded->mynum)
/*                    if (MechTeam(tempmech) == MechTeam(wounded)) */ {
			wounded = tempmech;
			hitloc = get_bsuit_hitloc(wounded);
			break;
		    }
                }
            }
        }
    }
    if (map && MapIsCS(map)) {
	combatsafe = 1;
    } else if (IsDS(wounded) && (MechStatus2(wounded) & CS_ON_LAND) && MechStatus(wounded) & COMBAT_SAFE)  {
	combatsafe = 1;
	for (i = 0; i < NUM_BAYS; i++) {
	    if (AeroBay(wounded, i) <= 1)
		continue;
	    if (!DS_Bay_Is_EnterOK(wounded, AeroBay(wounded, i))) {
		combatsafe = 0;
		break;
		}
	}
    } else if (MechStatus(wounded) & COMBAT_SAFE) {
	combatsafe = 1;
    }

    if (combatsafe) {
	if (wounded != attacker)
	    mech_notify(attacker, MECHALL,
		"Your efforts only scratch the paint!");
	return;
    }

    if (MechType(wounded) == CLASS_VEH_GROUND && hitloc == TURRET && !GetSectInt(wounded, hitloc)) {
	mech_notify(attacker, MECHALL, "Your shot zings through the air right where the turret would be!");
	MechLOSBroadcast(wounded, "narrowly misses a shot destined for it's destroyed turret!");
	return;
 	}

/* Maxtech VTOL rotor rules - DJ */
    if (damage > 1 && MechType(wounded) == CLASS_VTOL && hitloc == ROTOR)
	/* Successive changes to House Rules for 25percent damge rather than 1 */
        damage = ((damage + 3) / 3);
    if (intDamage > 1 && MechType(wounded) == CLASS_VTOL && hitloc == ROTOR)
        intDamage = ((intDamage + 3) / 3);
    if (is_aero(wounded))
	if ((damage * 10) > GetSectOArmor(wounded, hitloc)) {
	    if (!iscritical)
		iscritical = 1;
 	    crits++;
	}
    if (MechType(wounded) == CLASS_MW || MechType(wounded) == CLASS_MECH || MechType(wounded) == CLASS_VEH_GROUND || MechType(wounded) == CLASS_VEH_NAVAL || MechType(wounded) == CLASS_VTOL)
	transfer = 1;
    if ((damage > 0 || intDamage > 0) && MechStatus2(wounded) & SPRINTING) {
	MechStatus2(wounded) &= ~SPRINTING;
	MechLOSBroadcast(wounded, "breaks out of its sprint as it takes damage!");
	mech_notify(wounded, MECHALL, "You lose your sprinting momentum as you take damage!");
	if (!MoveModeChange(wounded))
	    MECHEVENT(wounded, EVENT_MOVEMODE, mech_movemode_event, TURN, MODE_OFF|MODE_SPRINT|MODE_MODEFAIL);
	}

    if ((damage > 0 || intDamage > 0) && (MoveModeLock(wounded) && !(MoveModeData(wounded) & (MODE_EVADE|MODE_DODGE|MODE_MODEFAIL|MODE_OFF)))) {
	StopMoveMode(wounded);
	mech_notify(wounded, MECHALL, "Your movement mode changes are cancelled as you take damage!");
	}

    if (MechCritStatus(wounded) & HIDDEN) {
	mech_notify(wounded, MECHALL, "Your cover is ruined as you take damage!");
	MechLOSBroadcast(wounded, "loses it's cover as it takes damage.");
	MechCritStatus(wounded) &= ~HIDDEN;
	}

    if (damage > 0 && intDamage == 0) {
	if (!global_physical_flag)
	    AccumulateGunXP(attackPilot, attacker, wounded, damage,  1,
		cause, bth);
	else if (global_physical_flag == 1)
	    if (!Destroyed(wounded) && In_Character(wounded->mynum) &&
		MechTeam(wounded) != MechTeam(attacker))
		if (MechType(wounded) != CLASS_MW ||
		    MechType(attacker) ==
		    CLASS_MW) AccumulatePilXP(attackPilot, attacker,
			damage / 3);
	damage = dam_to_pc_conversion(wounded, cause, damage);
    }

    if (In_Character(wounded->mynum))
	MechBV(wounded) = CalculateBV(wounded, 100, 100);

    if (isrear) {
	if (MechType(wounded) == CLASS_MECH) {
	    strcpy(rearMessage, "(Rear)");
            if (Dumping(wounded))
	      if ((critHit = IsLocDumping(wounded, hitloc, &realloc)) > -1)
	    /* Uh oh.... */
		if (Number(2,12) >= 6)
		/* Damnit, Damnit, Sonofabitch! */
		   {
		   if (!IsAmmo(GetPartType(wounded, realloc, critHit)))
			mech_notify(wounded, MECHALL, "ERROR in your mech! An ammo explosion has occured on a nonAmmo part!");
		   else {
                        mech_notify(attacker, MECHALL, "You ignite your targets dumping ammo!");
		        mech_notify(wounded, MECHALL, "Your currently dumping ammo ignites from a rear shot!");
		        weapindx = (Ammo2WeaponI(GetPartType(wounded, realloc, critHit)));
		        adamage = (GetPartData(wounded, realloc, critHit)
		        	* GunStat(weapindx, GetPartMode(wounded, realloc, critHit), GUN_DAMAGE))
		        	* (IsAcid(weapindx) ? 3 : 1);
			if (adamage > 0 && attacker && wounded && realloc > -1 && critHit > -1)
		            ammo_explosion(attacker, wounded, realloc, critHit, adamage);
		        }
		   }
	} else {
	    if (hitloc == FSIDE)
		hitloc = BSIDE;
	    *rearMessage = '\0';
	    isrear = 0;
	}
    } else
	*rearMessage = '\0';
    /* Damage something else, ok? */
    if (damage < 0) {
	switch (damage) {
	case -2:
	    was_transfer = 1;
	case -1:
	    transfer = 1;
	    break;
	}
	damage = 0;
    } else if (intDamage < 0) {
	switch (intDamage) {
	case -2:
	    was_transfer = 1;
	case -1:
	    transfer = 1;
	    break;
	}
	intDamage = 0;
    }

/*   while (SectIsDestroyed(wounded, hitloc) && !kill) */
    while (((!is_aero(wounded) && !GetSectInt(wounded, hitloc)) ||
	(is_aero(wounded) && !GetSectArmor(wounded, hitloc))) && !kill) {
	if (transfer && (hitloc = TransferTarget(wounded, hitloc)) >= 0 &&
	    (MechType(wounded) == CLASS_MECH ||
		MechType(wounded) == CLASS_MW ||
		MechType(wounded) == CLASS_BSUIT || MechType(wounded) == CLASS_VEH_GROUND ||
		MechType(wounded) == CLASS_VTOL || MechType(wounded) == CLASS_VEH_NAVAL ||
		is_aero(wounded))) {
	    DamageMech(wounded, attacker, LOS, attackPilot, hitloc, isrear,
		iscritical, damage == -1 ? -2 : damage,
		transfer == 1 ? -2 : damage, cause, bth, glance);
	    return;
	} else {
	    if (!((MechType(wounded) == CLASS_MECH ||
			MechType(wounded) == CLASS_MW ||
			MechType(wounded) == CLASS_BSUIT ||
			MechType(wounded) == CLASS_VEH_GROUND ||
	                MechType(wounded) == CLASS_VTOL ||
			MechType(wounded) == CLASS_VEH_NAVAL ||
			is_aero(wounded)) &&
		    (hitloc = TransferTarget(wounded, hitloc)) >= 0)) {
		if (is_aero(wounded) && !Destroyed(wounded)) {
		    /* Hurt SI instead. */
		    if (AeroSI(wounded) <= ((int) (ceil((float) intDamage / 2)))) {
			AeroSI(wounded) = 0;
			kill = 1;
			} else {
			AeroSI(wounded) -= ((int) (ceil((float) intDamage / 2)));
			if (!iscritical)
			    iscritical = 1;
		        crits++;
			kill = -1;
		    }
		} else
		    return;
	    }
	    /* Nyah. Damage transferred to waste, shooting a dead mech? */
	}

    }
    if (C_OODing(wounded) && Roll() > 4) {
	mech_ood_damage(wounded, attacker,
	    damage + (intDamage < 0 ? 0 : intDamage));
	return;
    }
    if (hitloc != -1) {
	ArmorStringFromIndex(hitloc, locationBuff, MechType(wounded),
	    MechMove(wounded));
	sprintf(notificationBuff, "for %d points of damage in the %s %s",
	    damage + (intDamage < 0 ? 0 : intDamage), locationBuff,
	    rearMessage);
    } else
	sprintf(notificationBuff,
	    "for %d points of damage in the structure.",
	    ((int) (ceil((float) damage / 2))));
    /*  if (LOS && attackPilot != -1) */
    if (LOS) {
	if (!was_transfer)
	    mech_notify(attacker, MECHALL, tprintf("%%cgYou hit %s%%c",
		    notificationBuff));
	else
	    mech_notify(attacker, MECHALL,
		tprintf("%%cgDamage transfer.. %s%%c", notificationBuff));
    }
    if (MechType(wounded) == CLASS_MW && !was_transfer)
	if (damage > 0)
	    if (!(damage = armor_effect(wounded, cause, hitloc, damage, intDamage)))
		return;
    if (damage > 0 || intDamage > 0)
	mech_notify(wounded, MECHALL, tprintf("%%ch%%cyYou have been hit %s%s%%c", notificationBuff, was_transfer ? "(transfer)" : ""));
    /* Always a good policy :-> */
    if (damage > 0 && intDamage <= 0 && !was_transfer)
	MechTurnDamage(wounded) += damage;
    if ((damage > 0 && MechType(attacker) != CLASS_MW
	&& !(MechType(attacker) == CLASS_BSUIT && MechSwarmTarget(attacker) == wounded->mynum)
	&& hitloc == HEAD && MechType(wounded) == CLASS_MECH
       && (!(MechSpecials2(wounded) & TORSOCOCKPIT_TECH)))) {

/*      mech_notify (wounded, MECHALL,
   "You take 10 points of Lethal damage!!"); */
	headhitmwdamage(wounded, 1);
    }
    if (is_aero(wounded) && AeroSI(wounded) <= 0) {
	    DamageAeroSI(wounded, 0, attacker);
	    return;
    }
    if (damage > 0) {
      if ((MechSpecials(wounded) & SLITE_TECH) && !(MechCritStatus(wounded) & SLITE_DEST))
	if ((MechType(wounded) == CLASS_MECH && (hitloc == LTORSO || hitloc == CTORSO || hitloc == RTORSO) && !isrear) ||
	    ((MechType(wounded) == CLASS_VEH_GROUND || MechType(wounded) == CLASS_VEH_NAVAL) && (GetSectOInt(wounded, TURRET) > 0 ? hitloc == TURRET : hitloc == FSIDE)))
	    if (Roll() > 6) {
                if ((MechStatus(wounded) & SLITE_ON) || (Roll() > 5)) {
                    MechCritStatus(wounded) |= SLITE_DEST;
                    MechStatus(wounded) &= ~SLITE_ON;
                    MechLOSBroadcast(wounded,
                        "'s searchlight is blown apart!");
                    mech_notify(wounded, MECHALL,
                        "%ch%cyYour searchlight is destroyed!%cn");

		}
	    }
#if 0
 	if (MechType(wounded) == CLASS_MECH) {
	    if (!isrear && (MechSpecials(wounded) & SLITE_TECH) &&
		!(MechCritStatus(wounded) & SLITE_DEST) &&
		(hitloc == LTORSO || hitloc == CTORSO || hitloc == RTORSO)) {
		/* Possibly destroy the light */
		if (Roll() > 6) {
		    if ((MechStatus(wounded) & SLITE_ON) || (Roll() > 5)) {
			MechCritStatus(wounded) |= SLITE_DEST;
			MechStatus(wounded) &= ~SLITE_ON;
			MechLOSBroadcast(wounded,
			    "'s searchlight is blown apart!");
			mech_notify(wounded, MECHALL,
			    "%ch%cyYour searchlight is destroyed!%cn");
		    }
		}
	    }
	}
#endif
	intDamage += cause_armordamage(wounded, attacker, LOS, attackPilot, isrear, iscritical, hitloc, damage, &crits, glance, 0);
	if (intDamage >= 0)
	    MechFloodsLoc(wounded, hitloc, MechZ(wounded));
	if (intDamage > 0 && !is_aero(wounded)) {
	    intDamage = cause_internaldamage(wounded, attacker, LOS, attackPilot, isrear, hitloc, intDamage, cause, &crits, glance, 0);
	    if (!intDamage && !SectIsDestroyed(wounded, hitloc))
		BreachLoc(attacker, wounded, hitloc);
	} else
	    PossiblyBreach(attacker, wounded, hitloc);
	if (intDamage > 0 && transfer &&
	    (MechType(wounded) != CLASS_BSUIT)) {
	    if ((hitloc = TransferTarget(wounded, hitloc)) >= 0)
		DamageMech(wounded, attacker, LOS, attackPilot, hitloc, isrear, iscritical, intDamage, -2, cause, bth, glance);
	}
    } else
	/* Cause _INTERNAL_ HAVOC! :-) */
	/* Non-CASE things get _really_ hurt */
    {
	if (intDamage > 0) {
	    if (is_aero(wounded)) {
		intDamage = cause_armordamage(wounded, attacker, LOS, attackPilot, isrear, iscritical, hitloc, intDamage, &crits, glance, 0);
	        if (is_aero(wounded) && AeroSI(wounded) <= 0) {
	  	    DamageAeroSI(wounded, 0, attacker);
		    return;
		    }
	    } else {
		intDamage = cause_internaldamage(wounded, attacker, LOS, attackPilot, isrear, hitloc, intDamage, cause, &crits, glance, 0);
	    }
	    if (!SectIsDestroyed(wounded, hitloc))
		PossiblyBreach(attacker, wounded, hitloc);
	    if (intDamage > 0 && transfer && !((MechSections(wounded)[hitloc].config & CASE_TECH) || (MechSpecials(wounded) & CLAN_TECH))) {
		if ((hitloc = TransferTarget(wounded, hitloc)) >= 0) {
		    if (!is_aero(wounded)) {
			DamageMech(wounded, attacker, LOS, attackPilot,
			    hitloc, isrear, iscritical, -2, intDamage,
			    cause, bth, glance);
		    } else {
			DamageMech(wounded, attacker, LOS, attackPilot,
			    hitloc, isrear, iscritical, intDamage, -2,
			    cause, bth, glance);
			}
		}
	    }
	}
    }
}

/* this takes care of setting all the criticals to CRIT_DESTROYED */
void DestroyWeapon(MECH * wounded, int hitloc, int type, char numcrits, int crit)
{
    int i = 0, ii;
    char sum = 0;
    int ammoLoc = -1, ammoCrit = -1, IsExplode = 0, ammo = 0, IsPPC = 0, IsGauss = 0, disable = 0;
    int weapindx, damage, loop, mode, data, AmmoWeap;

    if (crit < 0) {
	for (i = 0; i < 20; i++) {
	    if (GetPartType(wounded, hitloc, (ii = Number(0,NUM_CRITICALS - 1))) == type && !PartIsDestroyed(wounded, hitloc, ii)) {
		crit = ii;
		break;
		}
	    }
	if (crit < 0)
	    return;
	}
    mode = GetPartMode(wounded, hitloc, WeaponFirstCrit(wounded, hitloc, crit));
    data = GetPartData(wounded, hitloc, WeaponFirstCrit(wounded, hitloc, crit));
    type = GetPartType(wounded, hitloc, crit);
    weapindx = (Weapon2I(type));
    AmmoWeap = (!(MechWeapons[weapindx].type == TBEAM || MechWeapons[weapindx].type == THAND));

    if (IsWeapon(type)) {
        IsPPC = (strstr(MechWeapons[weapindx].name, "PPC") && MechWeapons[weapindx].type == TBEAM && (mode & CAPPED_MODE));
	IsGauss = (MechWeapons[weapindx].special & GAUSS);
	if (AmmoWeap) {
	    FindAmmoForWeapon_sub(wounded, weapindx, 0, &ammoLoc, &ammoCrit, 0, 0);
	    if (ammoLoc < 0 || ammoLoc >= NUM_SECTIONS || ammoCrit < 0 || ammoCrit >= NUM_CRITICALS)
		SendError(tprintf("Mech #%d error in DestroyWeapon - ammoLoc %d ammoCrit %d", wounded->mynum, ammoLoc, ammoCrit));
	    else
		ammo = (IsGauss ? 1 : GetPartData(wounded, ammoLoc, ammoCrit));
	    }
	IsExplode = ((IsPPC || IsGauss || ((mode & (DEADFIRE_MODE|HOTLOAD_MODE|JAMMED_MODE)) && ammo)) ? 1 : 0);
	}

    if (IsWeapon(type) && !IsExplode) {
	    DestroyPart(wounded, hitloc, crit);
	} else {
	for (i = WeaponFirstCrit(wounded, hitloc, crit); i < NUM_CRITICALS; i++) {
	    if (GetPartType(wounded, hitloc, i) == type) {
		if (PartIsDisabled(wounded, hitloc, i))
		    disable++;
		DestroyPart(wounded, hitloc, i);
		sum++;
	        if (sum == numcrits) {
	            if (IsExplode) {
	                damage = (IsGauss ? (strstr(MechWeapons[weapindx].name, "Light") ? 16 :
				   strstr(MechWeapons[weapindx].name, "Heavy") ? 25 : 20) :
			         (GunStat(weapindx, GetPartMode(wounded, hitloc, WeaponFirstCrit(wounded, hitloc, i)), GUN_DAMAGE)));
			if (IsMissile(weapindx))
	                    for (loop = 0; MissileHitTable[loop].key != -1; loop++) {
	                        if (MissileHitTable[loop].key == weapindx)
	                            damage *= MissileHitTable[loop].num_missiles[10];
	                        }
 			if (IsGauss && !disable)  {
			    mech_notify(wounded, MECHALL, tprintf("It explodes for %d damage!", damage));
                            MechLOSBroadcast(wounded, "is engulfed in a storm of electricity and magnetism!");
			    DamageMech(wounded, wounded, 0, -1, hitloc, 0, 0, 0, damage, -1, 7, 0);
			} else if (!IsGauss) {
			    mech_notify(wounded, MECHALL, "The currently loaded round explodes!");
                            MechLOSBroadcast(wounded, "flares in a spectacular light as a loaded weapon explodes!");
			    DamageMech(wounded, wounded, 0, -1, hitloc, 0, 0, 0, damage, -1, 7, 0);
			}
		        }
		    return;
		    }
      }
    }
  }
}

int CountWeaponsInLoc(MECH * mech, int loc)
{
    int i;
    int j, sec, cri;
    int count = 0;

    j = FindWeaponNumberOnMech(mech, 1, &sec, &cri);
    for (i = 2; j != -1; i++) {
	if (sec == loc)
	    count++;
	j = FindWeaponNumberOnMech(mech, i, &sec, &cri);
    }
    return count;
}

int FindWeaponTypeNumInLoc(MECH * mech, int loc, int num)
{
    int i;
    int j, sec, cri;
    int count = 0;

    j = FindWeaponNumberOnMech(mech, 1, &sec, &cri);
    for (i = 2; j != -1; i++) {
	if (sec == loc) {
	    count++;
	    if (count == num)
		return j;
	}
	j = FindWeaponNumberOnMech(mech, i, &sec, &cri);
    }
    return -1;
}

void LoseWeapon(MECH * mech, int hitloc)
{
    /* Look for hit locations.. */
    int i = CountWeaponsInLoc(mech, hitloc);
    int a, b;

    if (!i)
	return;
    a = random() % i + 1;
    b = FindWeaponTypeNumInLoc(mech, hitloc, a);
    if (b < 0)
	return;
    DestroyWeapon(mech, hitloc, I2Weapon(b), GetWeaponCrits(mech, b), -1);
    mech_notify(mech, MECHALL, tprintf("%%ch%%crYour %s is destroyed!%%c",
	    &MechWeapons[b].name[3]));
}

int FindObj(MECH * mech, int loc, int type)
{
    int count = 0, i;

    for (i = 0; i < NUM_CRITICALS; i++)
	if (GetPartType(mech, loc, i) == type)
	    if (!PartIsNonfunctional(mech, loc, i))
		count++;
    return count;
}

void DestroyHeatSink(MECH * mech, int hitloc)
{
    /* This can be done easily, or this can be done painfully. */
    /* Let's try the painful way, it's more fun that way. */
    int num;
    int i = I2Special(HEAT_SINK);

    if (FindObj(mech, hitloc, i)) {
	num =
	    MechSpecials(mech) & DOUBLE_HEAT_TECH ? (char) 3 :
	    MechSpecials(mech) & CLAN_TECH ? (char) 2 : (char) 1;
	DestroyWeapon(mech, hitloc, i, num, -1);
	MechRealNumsinks(mech) -= (1 + num != 1);
	mech_notify(mech, MECHALL,
	    "The computer shows a heatsink died due to the impact.");
    }
}

void StartAcidEffect(MECH * mech, MECH * hitMech, int hitloc, int notifatt, int notifdef, int bc)
{
	char locname[32];

	if (SectIsDestroyed(hitMech, hitloc))
		return;

	if (notifatt)
		mech_notify(mech, MECHALL, "%ch%cgYou hit with a stream of acid!%c");
	if (notifdef)
		mech_notify(hitMech, MECHALL, "%ch%cgYou are hit by a stream of acid!%c");

	if (!(MechSections(hitMech)[hitloc].config & SECTION_ACID)) {
		if (bc) {
			ArmorStringFromIndex(hitloc, locname, MechType(hitMech), MechMove(hitMech));
			MechLOSBroadcast(hitMech, tprintf("begins to fume from its %s.", locname));
		}
		MechSections(hitMech)[hitloc].config |= SECTION_ACID;
		MECHEVENT(hitMech, EVENT_ACID, acid_damage_event, ACID_TICK, hitloc);
	}
}

void DestroySection(MECH * wounded, MECH * attacker, int LOS, int hitloc)
{
    char areaBuff[30];
    char foobuff[MBUF_SIZE];
    int i, j, nuke = 1;

    /* Prevent the rare occurance of a section getting destroyed twice */
    if (SectIsDestroyed(wounded, hitloc))
	return;

    /* Ouch. They got toasted */
    SetSectArmor(wounded, hitloc, 0);
    SetSectInt(wounded, hitloc, 0);
    SetSectRArmor(wounded, hitloc, 0);
    SetSectDestroyed(wounded, hitloc);
    if (attacker) {
	ArmorStringFromIndex(hitloc, areaBuff, MechType(wounded),
	    MechMove(wounded));
	if (LOS >= 0)
	    mech_notify(wounded, MECHALL,
		tprintf("Your %s has been destroyed!", areaBuff));
	sprintf(foobuff, "'s %s has been destroyed!", areaBuff);
	MechLOSBroadcast(wounded, foobuff);
	if (MechCarrying(wounded) > 0 && MechMove(wounded) == MOVE_BIPED && (hitloc == LARM || hitloc == RARM))
		mech_dropoff(GOD, wounded, "");
    }
    if ((MechType(wounded) == CLASS_VEH_GROUND || MechType(wounded) == CLASS_VEH_VTOL) && MechSpeed(wounded) > MP1) {
		mech_notify(wounded, MECHALL, "You are thrown about your cockpit!");
		mech_notify(wounded, MECHALL, "You attempt to avoid personal injury!");
		if (MadePilotSkillRoll(wounded, -2 +(MechSpeed(wounded) / MP2))) {
			mech_notify(wounded, MECHALL, "Your fancy moves worked!");
		} else {
			mech_notify(wounded, MECHALL, "Your cute fancy moves failed....");
			headhitmwdamage(wounded, 1);
		}
	}
    NarcClear(wounded, hitloc);
    AcidClear(wounded, hitloc, 0);
    DestroyParts(attacker, wounded, hitloc, 0, 0);

/* OK. The actualy section is now handled. Now for zeroed/dest logic checks */
    for (i = 0; i < NUM_SECTIONS; i++)
	if ((GetSectOInt(wounded, i) && (!SectIsDestroyed(wounded, i))) || (!(In_Character(wounded->mynum)))) {
	    nuke = 0;
	    break;
	    }

    if (MechType(wounded) == CLASS_MW && (attacker ? MechType(attacker) == CLASS_MW : 1)) {
	mwlethaldam(wounded, 4);
    }

    if (MechType(wounded) == CLASS_MW || MechType(wounded) == CLASS_MECH) {
	if (hitloc == LTORSO)
	    DestroySection(wounded, attacker, LOS, LARM);
	else if (hitloc == RTORSO)
	    DestroySection(wounded, attacker, LOS, RARM);
	else if (hitloc == CTORSO || (hitloc == HEAD && !(MechSpecials2(wounded) & TORSOCOCKPIT_TECH))) {
	    if (!Destroyed(wounded)) {
		DestroyMech(wounded, attacker, 1);
		}
	    if ((MechSpecials2(wounded) & TORSOCOCKPIT_TECH ? hitloc == CTORSO : hitloc == HEAD) && attacker)
		if (In_Character(wounded->mynum)) {
		    for (j = 0; j < FREQS; j++) {
			wounded->freq[j] = 0;
			wounded->freqmodes[j] = 0;
			wounded->chantitle[j][0] = 0;
		    }
		    KillMechContentsIfIC(wounded->mynum, wounded);
		}
	}
    }

/* IIRC correctly this check should be MUTE */
    if (is_aero(wounded)) {
	/* Aero handling is trivial ; No destruction whatsoever, for now. */
	/* With one exception.. */
	if (hitloc == AERO_NOSE && MechType(wounded) == CLASS_AERO) {
	    if (!Destroyed(wounded))
		DestroyMech(wounded, attacker, 1);
	    for (j = 0; j < FREQS; j++) {
		wounded->freq[j] = 0;
		wounded->freqmodes[j] = 0;
		wounded->chantitle[j][0] = 0;
	    }
	    KillMechContentsIfIC(wounded->mynum, wounded);
	}
    }

    if (MechType(wounded) == CLASS_BSUIT && bsuit_members(wounded) <= 0) {
	if (!Destroyed(wounded)) {
	    DestroyMech(wounded, attacker, 1);
	    }
    }

    if (MechType(wounded) == CLASS_VEH_GROUND || MechType(wounded) == CLASS_VEH_NAVAL || MechType(wounded) == CLASS_VTOL) {
	if (!Destroyed(wounded) && hitloc != TURRET) {
	    if (MechSections(wounded)[hitloc].config & CASE_TECH) {
		mech_notify(wounded, MECHALL, "CASE in that section saves you from certain death!");
	    } else {
		if (!Uncon(wounded)) {
		    notify(MechPilot(wounded), "You autoeject before the blast kills you!");
		    autoeject(MechPilot(wounded), wounded, attacker); 
		    }
	        KillMechContentsIfIC(wounded->mynum, wounded);
	    }
	    if (!Destroyed(wounded))
		DestroyMech(wounded, attacker, 1);
	}
    }

    /* Ensure the template's timely demise */
    if (nuke) {
	if (In_Character(wounded->mynum)) {
	    for (j = 0; j < FREQS; j++) {
		wounded->freq[j] = 0;
		wounded->freqmodes[j] = 0;
		wounded->chantitle[j][0] = 0;
		}
	    KillMechContentsIfIC(wounded->mynum, wounded);
	    DumpCarrier(wounded);
	    if (!IsDS(wounded) && !is_aero(wounded))
		discard_mw(wounded);
	    }
	}
}

char *setarmorstatus_func(MECH * mech, char *sectstr, char *typestr,
    char *valuestr)
{
    int index, type, value;


    if (!sectstr || !*sectstr)
	return "#-1 INVALID SECTION";

    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), sectstr);
    if (index == -1 || !GetSectOInt(mech, index))
	return "#-1 INVALID SECTION";

    if ((value = atoi(valuestr)) < 0 || value > 255)
	return "#-1 INVALID ARMORVALUE";

    switch (type = atoi(typestr)) {
    case 0:
	SetSectArmor(mech, index, value);
	break;
    case 1:
	SetSectInt(mech, index, value);
	break;
    case 2:
	SetSectRArmor(mech, index, value);
	break;
    default:
	return "#-1 INVALID ARMORTYPE";
    }
    return "1";
}

int dodamage_func(dbref player, MECH * mech, int totaldam, int clustersize, int direction, int iscritical, char *mechmsg, char *mechbroadcast)
{
                                                                                                                                                                                      
    int hitloc = 1, this_time, isrear = 0, dummy = 0;
    int *dummy1 = &dummy, *dummy2 = &dummy;

    if (totaldam <= 0 || clustersize <= 0 || direction < 0 || !mech)
	return 0;
                                                                                                                                                                                      
    if (direction < 8) {
        hitloc = direction;
    } else if (direction < 16) {
        hitloc = direction - 8;
        isrear = 1;
    } else if (direction > 21) {
        return 0;
    }
                                                                                                                                                                                      
    if (mechmsg && *mechmsg)
        mech_notify(mech, MECHALL, mechmsg);
    if (mechbroadcast && *mechbroadcast)
        MechLOSBroadcast(mech, mechbroadcast);
    while (totaldam) {
        if (direction > 18)
            isrear = 1;
        if (direction > 15)
            hitloc =
                FindHitLocation(mech, ((direction - 1) & 3) + 1, dummy1, dummy2, NULL);
        this_time = MIN(clustersize, totaldam);
        DamageMech(mech, mech, 0, -1, hitloc, isrear, iscritical, this_time, 0, 0, 0, 0);
        totaldam -= this_time;
    }
    return 1;
}

