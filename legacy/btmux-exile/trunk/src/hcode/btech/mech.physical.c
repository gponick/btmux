#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "p.mech.physical.h"
#include "p.mech.combat.h"
#include "p.mech.utils.h"
#include "p.mech.los.h"
#include "p.mech.hitloc.h"
#include "p.bsuit.h"
#include "p.btechstats.h"
#include "p.template.h"

#define PA_PUNCH 1
#define PA_CLUB  2
#define PA_KICK  3
#define PA_AXE   4
#define PA_SWORD 5
#define PA_MACE  6

#define P_LEFT  1
#define P_RIGHT 2

#define Dodging(a) (MechStatus2(a) & DODGING)

#define ARM_PHYS_CHECK(a) \
DOCHECK(MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT, \
  tprintf("You cannot %s without a 'mech!", a)); \
DOCHECK(MechType(mech) != CLASS_MECH, \
  tprintf("You cannot %s with this vehicle!", a));

#define GENERIC_CHECK(a) \
ARM_PHYS_CHECK(a);\
DOCHECK(MechMove(mech) != MOVE_QUAD && (MechCritStatus(mech) & NO_LEGS), \
	"Without legs? Are you kidding?");\
DOCHECK(MechMove(mech) != MOVE_QUAD && (MechCritStatus(mech) & LEG_DESTROYED),\
	"With one leg? Are you kidding?");\
DOCHECK(MechCritStatus(mech) & LEG_DESTROYED,"It'd unbalance you too much in your condition..");\
DOCHECK(MechCritStatus(mech) & NO_LEGS, "Exactly _what_ are you going to kick with?");

#define QUAD_CHECK(a) \
DOCHECK(MechType(mech) == CLASS_MECH && MechMove(mech) == MOVE_QUAD, \
	tprintf("What are you going to %s with, your front right leg?", a))
/*
 * All 'mechs with arms can punch.
 */
static int have_punch(MECH * mech, int loc)
{
    return 1;
}

/*
 * Parse a physical attack command's arguments that allow an arm or both
 * to be specified.  eg. AXE [B|L|R] [ID]
 */

static int get_arm_args(int *using, int *argc, char ***args, MECH * mech,
    int (*have_fn) (MECH * mech, int loc), char *weapon)
{

    if (*argc != 0 && args[0][0][0] != '\0' && args[0][0][1] == '\0') {
	char arm = toupper(args[0][0][0]);

	switch (arm) {
	case 'B':
	    *using = P_LEFT | P_RIGHT;
	    --*argc;
	    ++*args;
	    break;

	case 'L':
	    *using = P_LEFT;
	    --*argc;
	    ++*args;
	    break;

	case 'R':
	    *using = P_RIGHT;
	    --*argc;
	    ++*args;
	}
    }

    switch (*using) {
    case P_LEFT:
	if (!have_fn(mech, LARM)) {
	    mech_notify(mech, MECHALL,
		tprintf("You don't have %s in your left arm!", weapon));
	    return 1;
	}
	break;

    case P_RIGHT:
	if (!have_fn(mech, RARM)) {
	    mech_notify(mech, MECHALL,
		tprintf("You don't have %s in your right arm!", weapon));
	    return 1;
	}
	break;

    case P_LEFT | P_RIGHT:
	if (!have_fn(mech, LARM))
	    *using &= ~P_LEFT;
	if (!have_fn(mech, RARM))
	    *using &= ~P_RIGHT;
	break;
    }

    return 0;
}

void mech_punch(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    char *argl[5];
    char **args = argl;
    int argc, ltohit = 4, rtohit = 4;
    int punching = P_LEFT | P_RIGHT;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    ARM_PHYS_CHECK("punch");
    QUAD_CHECK("punch");
    DOCHECK(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    argc = mech_parseattributes(buffer, args, 5);
    if (mudconf.phys_use_pskill)
	ltohit = rtohit = FindPilotPiloting(mech) - 1;

    if (get_arm_args(&punching, &argc, &args, mech, have_punch, "")) {
	return;
    }

    if (punching & P_LEFT) {
	if (SectIsDestroyed(mech, LARM))
	    mech_notify(mech, MECHALL,
		"Your left arm is destroyed, you can't punch with it.");
	else if (!OkayCritSectS(LARM, 0, SHOULDER_OR_HIP))
	    mech_notify(mech, MECHALL,
		"Your left shoulder is destroyed, you can't punch with that arm.");
	else {
	    if (Fallen(mech)) {
		DOCHECK(SectIsDestroyed(mech, RARM),
		    "You need both arms functional to punch while prone.");
		DOCHECK(SectHasBusyWeap(mech, RARM),
		    "You have weapons recycling on your Right Arm.");
		DOCHECK(MechSections(mech)[RARM].recycle,
		    "Your Right Arm is still recovering from your last attack.");
	    }
	    PhysicalAttack(mech, 10, ltohit, PA_PUNCH, argc, args,
		mech_map, LARM);
	}
    }
    if (punching & P_RIGHT) {
	if (SectIsDestroyed(mech, RARM))
	    mech_notify(mech, MECHALL,
		"Your right arm is destroyed, you can't punch with it.");
	else if (!OkayCritSectS(RARM, 0, SHOULDER_OR_HIP))
	    mech_notify(mech, MECHALL,
		"Your right shoulder is destroyed, you can't punch with that arm.");
	else {
	    if (Fallen(mech)) {
		DOCHECK(SectIsDestroyed(mech, LARM),
		    "You need both arms functional to punch while prone.");
		DOCHECK(SectHasBusyWeap(mech, LARM),
		    "You have weapons recycling on your Left Arm.");
		DOCHECK(MechSections(mech)[LARM].recycle,
		    "Your Left Arm is still recovering from your last attack.");
	    }
	    PhysicalAttack(mech, 10, rtohit, PA_PUNCH, argc, args, mech_map, RARM);
	}
    }
}

void mech_club(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    char *args[5];
    int argc;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    ARM_PHYS_CHECK("club");
    QUAD_CHECK("club");
    DOCHECKMA(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    DOCHECKMA(MechRTerrain(mech) != HEAVY_FOREST &&
	MechRTerrain(mech) != LIGHT_FOREST,
	"You can not seem to find any trees around to club with.");
    argc = mech_parseattributes(buffer, args, 5);
    DOCHECKMA(SectIsDestroyed(mech, LARM),
	"Your left arm is destroyed, you can't club.");
    DOCHECKMA(SectIsDestroyed(mech, RARM),
	"Your right arm is destroyed, you can't club.");
    DOCHECKMA(!OkayCritSectS(RARM, 0, SHOULDER_OR_HIP),
	"You can't swing at anything without your right shoulder.");
    DOCHECKMA(!OkayCritSectS(LARM, 0, SHOULDER_OR_HIP),
	"You can't swing at anything without your left shouler.");
    DOCHECKMA(!OkayCritSectS(RARM, 3, HAND_OR_FOOT_ACTUATOR),
	"You can't pick anything up without your right hand.");
    DOCHECKMA(!OkayCritSectS(LARM, 3, HAND_OR_FOOT_ACTUATOR),
	"You can't pick anything up without your left hand.");

    PhysicalAttack(mech, 5,
	(mudconf.phys_use_pskill ? FindPilotPiloting(mech) - 1 : 4), PA_CLUB, argc, args, mech_map, RARM);
}

int have_axe(MECH * mech, int loc)
{
int i, crit = 0, num = 0;

    for (i = 0; i < NUM_CRITICALS; i++)
        {
        crit = (GetPartType(mech, loc, i));
        if ((PartIsNonfunctional(mech, loc, i)) && (crit == I2Special(AXE)))
                return 0;
        else if (!PartIsNonfunctional(mech, loc, i) && (crit == I2Special(AXE)))
                num++;
        }
    if (num > 0)
        return 1;
    else
        return 0;

/*    return FindObj(mech, loc, I2Special(AXE)) >= (MechTons(mech) / 15); */
}

int have_sword(MECH * mech, int loc)
{
int i, crit = 0, num = 0;

    for (i = 0; i < NUM_CRITICALS; i++)
        {
        crit = (GetPartType(mech, loc, i));
        if ((PartIsNonfunctional(mech, loc, i)) && (crit == I2Special(SWORD)))
                return 0;
        else if (!PartIsNonfunctional(mech, loc, i) && (crit == I2Special(SWORD)))
                num++;
        }
    if (num > 0)
        return 1;
    else
        return 0;

/*    return FindObj(mech, loc, I2Special(SWORD)) >= ((MechTons(mech) + 15) / 20); */
}

int have_mace(MECH * mech, int loc)
{
int i, crit = 0, num = 0;

    for (i = 0; i < NUM_CRITICALS; i++)
	{
	crit = (GetPartType(mech, loc, i));
	if ((PartIsNonfunctional(mech, loc, i)) && (crit == I2Special(MACE)))
		return 0;
	else if (!PartIsNonfunctional(mech, loc, i) && (crit == I2Special(MACE)))
		num++;
	}
    if (num > 0)
	return 1;
    else
	return 0;
/*    return FindObj(mech, loc, I2Special(MACE)) >= (MechTons(mech) / 15); */
}

void mech_axe(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    char *argl[5];
    char **args = argl;
    int argc, ltohit = 4, rtohit = 4;
    int using = P_LEFT | P_RIGHT;
    char arm;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    ARM_PHYS_CHECK("axe");
    QUAD_CHECK("axe");
    DOCHECK(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    argc = mech_parseattributes(buffer, args, 5);
    if (mudconf.phys_use_pskill)
	ltohit = rtohit = FindPilotPiloting(mech) - 1;

/*    ltohit += */
/*	2 * MechSections(mech)[LARM].basetohit + (!OkayCritSectS(LARM, 3, */
/*	    HAND_OR_FOOT_ACTUATOR)) ? 1 : 0; */
/*    rtohit += */
/*	2 * MechSections(mech)[RARM].basetohit + (!OkayCritSectS(RARM, 3, */
/*	    HAND_OR_FOOT_ACTUATOR)) ? 1 : 0; */
/*        ltohit += ((!OkayCritSectS(LARM, 1, UPPER_ACTUATOR) ? 2 : 0) + (!OkayCritSectS(LARM, 2, LOWER_ACTUATOR) ? 2 : 0));
        rtohit += ((!OkayCritSectS(RARM, 1, UPPER_ACTUATOR) ? 2 : 0) + (!OkayCritSectS(RARM, 2, LOWER_ACTUATOR) ? 2 : 0));*/

    if (get_arm_args(&using, &argc, &args, mech, have_axe, "an axe")) {
	return;
    }

    if (using & P_LEFT) {
	if (SectIsDestroyed(mech, LARM))
	    mech_notify(mech, MECHALL,
		"Your left arm is destroyed, you can't axe with it.");
	else if (!OkayCritSectS(LARM, 0, SHOULDER_OR_HIP))
	    mech_notify(mech, MECHALL,
		"Your left shoulder is destroyed, you can't axe with that arm.");
	else if (!OkayCritSectS(LARM, 3, HAND_OR_FOOT_ACTUATOR))
	    mech_notify(mech, MECHALL,
		"Your left hand isn't there, you can't axe with that arm.");
	else
	    PhysicalAttack(mech, 5, ltohit, PA_AXE, argc, args, mech_map, LARM);
    }
    if (using & P_RIGHT) {
	DOCHECK(SectIsDestroyed(mech, RARM),
	    "Your right arm is destroyed, you can't axe with it.");
	DOCHECK(!OkayCritSectS(RARM, 0, SHOULDER_OR_HIP),
	    "Your right shoulder is destroyed, you can't axe with that arm.");
	DOCHECK(!OkayCritSectS(RARM, 3, HAND_OR_FOOT_ACTUATOR),
	    "Your right hand isn't there, you can't axe with tht arm.");
	PhysicalAttack(mech, 5, rtohit, PA_AXE, argc, args, mech_map, RARM);
    }
    DOCHECKMA(!using,
	"You may lack the axe, but not the will! Try punch/club until you find one.");
}

void mech_bash(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    char *argl[5];
    char **args = argl;
    int argc, ltohit = 6, rtohit = 6;
    int using = P_LEFT | P_RIGHT;
    char arm;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    ARM_PHYS_CHECK("bash");
    QUAD_CHECK("bash");
    DOCHECK(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    argc = mech_parseattributes(buffer, args, 5);
    if (mudconf.phys_use_pskill)
        ltohit = rtohit = FindPilotPiloting(mech) + 1;

/*    ltohit += */
/*        2 * MechSections(mech)[LARM].basetohit + (!OkayCritSectS(LARM, 3, */
/*            HAND_OR_FOOT_ACTUATOR)) ? 1 : 0; */
/*    rtohit += */
/*        2 * MechSections(mech)[RARM].basetohit + (!OkayCritSectS(RARM, 3, */
/*            HAND_OR_FOOT_ACTUATOR)) ? 1 : 0; */
	/*ltohit += ((!OkayCritSectS(LARM, 1, UPPER_ACTUATOR) ? 2 : 0) + (!OkayCritSectS(LARM, 2, LOWER_ACTUATOR) ? 2 : 0));
        rtohit += ((!OkayCritSectS(RARM, 1, UPPER_ACTUATOR) ? 2 : 0) + (!OkayCritSectS(RARM, 2, LOWER_ACTUATOR) ? 2 : 0));*/

    if (get_arm_args(&using, &argc, &args, mech, have_mace, "a mace")) {
        return;
    }

    if (using & P_LEFT) {
        if (SectIsDestroyed(mech, LARM))
            mech_notify(mech, MECHALL,
                "Your left arm is destroyed, you can't bash with it.");
        else if (!OkayCritSectS(LARM, 0, SHOULDER_OR_HIP))
            mech_notify(mech, MECHALL,
                "Your left shoulder is destroyed, you can't bash with that arm.");
        else if (!OkayCritSectS(LARM, 3, HAND_OR_FOOT_ACTUATOR))
            mech_notify(mech, MECHALL,
                "Your left hand isn't there, you can't bash with that arm.");
        else
            PhysicalAttack(mech, 5, ltohit, PA_MACE, argc, args, mech_map,
                LARM);
    }
    if (using & P_RIGHT) {
        DOCHECK(SectIsDestroyed(mech, RARM),
            "Your right arm is destroyed, you can't bash with it.");
        DOCHECK(!OkayCritSectS(RARM, 0, SHOULDER_OR_HIP),
            "Your right shoulder is destroyed, you can't bash with that arm.");
        DOCHECK(!OkayCritSectS(RARM, 3, HAND_OR_FOOT_ACTUATOR),
            "Your right hand isn't there, you can't bash with tht arm.");
        PhysicalAttack(mech, 5, rtohit, PA_MACE, argc, args, mech_map,
            RARM);
    }
    DOCHECKMA(!using,
        "You may lack the mace, but not the will! Try punch/club until you find one.");
}

void mech_sword(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MAP *mech_map;
    char *argl[5];
    char **args = argl;
    int argc, ltohit = 3, rtohit = 3;
    int using = P_LEFT | P_RIGHT;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    ARM_PHYS_CHECK("chop");
    QUAD_CHECK("chop");
    DOCHECKMA(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    argc = mech_parseattributes(buffer, args, 5);
    if (mudconf.phys_use_pskill)
	ltohit = rtohit = FindPilotPiloting(mech) - 2;

/*      ltohit += */
/*      	2 * MechSections(mech)[LARM].basetohit + (!OkayCritSectS(LARM, 3, */
/*	    HAND_OR_FOOT_ACTUATOR)) ? 1 : 0; */
/*      rtohit += */
/*      	2 * MechSections(mech)[RARM].basetohit + (!OkayCritSectS(RARM, 3, */
/*	    HAND_OR_FOOT_ACTUATOR)) ? 1 : 0; */
        /*ltohit += ((!OkayCritSectS(LARM, 1, UPPER_ACTUATOR) ? 2 : 0) + (!OkayCritSectS(LARM, 2, LOWER_ACTUATOR) ? 2 : 0));
        rtohit += ((!OkayCritSectS(RARM, 1, UPPER_ACTUATOR) ? 2 : 0) + (!OkayCritSectS(RARM, 2, LOWER_ACTUATOR) ? 2 : 0));*/

    if (get_arm_args(&using, &argc, &args, mech, have_sword, "a sword")) {
	return;
    }

    if (using & P_LEFT) {
	if (SectIsDestroyed(mech, LARM))
	    mech_notify(mech, MECHALL,
		"Your left arm is destroyed, you can't use a sword with it.");
	else if (!OkayCritSectS(LARM, 0, SHOULDER_OR_HIP))
	    mech_notify(mech, MECHALL,
		"Your left shoulder is destroyed, you can't use a sword with that arm.");
	else if (!OkayCritSectS(LARM, 3, HAND_OR_FOOT_ACTUATOR))
	    mech_notify(mech, MECHALL,
		"Your left hand seems to be missing, you can't grip your sword very well.");
	else
	    PhysicalAttack(mech, 10, ltohit, PA_SWORD, argc, args,
		mech_map, LARM);
    }
    if (using & P_RIGHT) {
	if (SectIsDestroyed(mech, RARM))
	    mech_notify(mech, MECHALL,
		"Your right arm is destroyed, you can't use a sword with it.");
	else if (!OkayCritSectS(RARM, 0, SHOULDER_OR_HIP))
	    mech_notify(mech, MECHALL,
		"Your right shoulder is destroyed, you can't use a sword with that arm.");
        else if (!OkayCritSectS(RARM, 3, HAND_OR_FOOT_ACTUATOR))
            mech_notify(mech, MECHALL,
                "Your right hand seems to be missing, you can't grip your sword very well.");
	else
	    PhysicalAttack(mech, 10, rtohit, PA_SWORD, argc, args,
		mech_map, RARM);
    }
    DOCHECKMA(!using, "You have no sword to chop people with! Learn Bushido!");
}

void mech_kick(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *target;
    MAP *mech_map;
    char *argl[5], targetID[2];
    char **args = argl;
    int argc;
    int rl = RLEG, ll = LLEG;
    int leg, iwa, targetnum;
    int using = P_RIGHT;
    int actcrits = 0, weightdmg = 5, i = 0, crit = 0;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);

    GENERIC_CHECK("kick");
    argc = mech_parseattributes(buffer, args, 5);
    DOCHECKMA(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");

    if (get_arm_args(&using, &argc, &args, mech, have_punch, "")) {
	return;
    }
/* Check for default target first. If not, we do it the hard way. :( -- DJ */
    if (MechMove(mech) == MOVE_QUAD) {
	switch (argc) {
	    case -1:
	    case  0:
	        DOCHECKMA(MechTarget(mech) == -1, "You do not have a default target set!");
        	target = getMech(MechTarget(mech));
	        DOCHECKMA(!target, "Invalid default target!");
		break;
	    default:
		targetID[0] = args[0][0];
		targetID[1] = args[0][1];
	        targetnum = FindTargetDBREFFromMapNumber(mech, targetID);
	        DOCHECKMA(targetnum == -1, "That is not a valid target ID!");
		target = getMech(targetnum);
        	DOCHECKMA(!target, "Invalid default target!");
	    }
        DOCHECKMA(!target, "Invalid Target!");
        iwa = InWeaponArc(mech, MechFX(target), MechFY(target));
	if (iwa & FORWARDARC) {
		rl = RARM;
	        ll = LARM;
		}
	if (!(iwa & FORWARDARC)) {
		DOCHECK((SectHasBusyWeap(mech, RARM) || SectHasBusyWeap(mech, LARM) || SectHasBusyWeap(mech, RLEG) || SectHasBusyWeap(mech, LLEG)),
		        "You want to mule kick with weapons recycling in your legs?");\
		DOCHECK((MechSections(mech)[RARM].recycle || MechSections(mech)[LARM].recycle ||
			 MechSections(mech)[RLEG].recycle || MechSections(mech)[LLEG].recycle), "All four legs must be prepared in order to mule kick.");
		}
	}

    switch (using) {
    case P_LEFT:
	leg = ll;
	break;

    case P_RIGHT:
	leg = rl;
	break;

    default:
    case P_LEFT | P_RIGHT:
	mech_notify(mech, MECHALL,
	    "What, yer gonna LEVITATE? I Don't Think So.");
	return;
    }
/*    while (i < NUM_CRITICALS)
	{
	crit = (Special2I(GetPartType(mech, leg, i)));
	if ((crit == UPPER_ACTUATOR || crit == LOWER_ACTUATOR) && PartIsNonfunctional(mech, leg, i))
		actcrits++;
	i++;
	}
    weightdmg = (actcrits == 0 ? 5 : (actcrits == 1 ? 10 : actcrits == 2 ? 20 : actcrits == 3 ? 40 : 80));
    PhysicalAttack(mech, weightdmg,
	(mudconf.phys_use_pskill ? FindPilotPiloting(mech) - 2 : 3),
	PA_KICK, argc, args, mech_map, leg);*/
	PhysicalAttack(mech, 5, (mudconf.phys_use_pskill ? FindPilotPiloting(mech) - 2 : 3),
	 PA_KICK, argc, args, mech_map, leg);
}

void mech_charge(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data, *target;
    MAP *mech_map;
    int targetnum;
    char targetID[5];
    char *args[5];
    int argc;

    mech_map = getMap(mech->mapindex);
    cch(MECH_USUALO);
    DOCHECK(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    DOCHECK(MechType(mech) == CLASS_MW ||
	MechType(mech) == CLASS_BSUIT,
	"You cannot charge without a 'mech!");
/*    DOCHECK(MechType(mech) != CLASS_MECH &&
	(MechType(mech) != CLASS_VEH_GROUND ||
	    MechSpecials(mech) & SALVAGE_TECH),
	"You cannot charge with this vehicle!");*/
    if (MechType(mech) == CLASS_MECH) {
	DOCHECK(MechMove(mech) != MOVE_QUAD &&
	    (MechCritStatus(mech) & LEG_DESTROYED),
	    "With one leg? Are you kidding?");
	DOCHECK(MechMove(mech) != MOVE_QUAD &&
	    (MechCritStatus(mech) & NO_LEGS),
	    "Without legs? Are you kidding?");
	DOCHECK(MechCritStatus(mech) & LEG_DESTROYED,
	    "It'd unbalance you too much in your condition..");
	DOCHECK(MechCritStatus(mech) & NO_LEGS,
	    "Exactly _what_ are you going to charge with?");
    }
    argc = mech_parseattributes(buffer, args, 2);
    switch (argc) {
    case 0:
	DOCHECKMA(MechTarget(mech) == -1,
	    "You do not have a default target set!");
	target = getMech(MechTarget(mech));
	if (!target) {
	    mech_notify(mech, MECHALL, "Invalid default target!");
	    MechTarget(mech) = -1;
	    return;
	}
	DOCHECK(((MechTeam(mech) == MechTeam(target)) && (Started(target)) && (!Destroyed(target))) , "Friendly units ? I dont Think so..");
	MechChargeTarget(mech) = MechTarget(mech);
	mech_notify(mech, MECHALL, "Charge target set to default target.");
	break;
    case 1:
	if (args[0][0] == '-') {
	    MechChargeTarget(mech) = -1;
	    MechChargeTimer(mech) = 0;
	    MechChargeDistance(mech) = 0;
	    mech_notify(mech, MECHPILOT, "You are no longer charging.");
	    return;
	}
	targetID[0] = args[0][0];
	targetID[1] = args[0][1];
	targetnum = FindTargetDBREFFromMapNumber(mech, targetID);
	DOCHECKMA(targetnum == -1, "That is not a valid target ID!");
	target = getMech(targetnum);
	if (!target) {
	    mech_notify(mech, MECHALL, "Invalid target data!");
	    return;
	}
	MechChargeTarget(mech) = targetnum;
	mech_notify(mech, MECHALL, tprintf("%s target set to %s.",
		MechType(mech) == CLASS_MECH ? "Charge" : "Ram",
		GetMechToMechID(mech, target)));
	break;
    default:
	notify(player, "Invalid number of arguments!");
    }
}


char *phys_form(int at, int flag)
{
    switch (flag) {
    case 0:
	switch (at) {
	case PA_PUNCH:
	    return "punches";
	case PA_CLUB:
	    return "clubs";
	case PA_MACE:
	    return "bashes";
	case PA_SWORD:
	    return "chops";
	case PA_AXE:
	    return "axes";
	case PA_KICK:
	    return "kicks";
	}
	break;
    default:
	switch (at) {
	case PA_PUNCH:
	    return "punch";
	case PA_SWORD:
	    return "chop";
	case PA_CLUB:
	    return "club";
	case PA_MACE:
	    return "bash";
	case PA_AXE:
	    return "axe";
	case PA_KICK:
	    return "kick";
	}
	break;
    }
    return "??bug??";
}

#define phys_message(txt) \
MechLOSBroadcasti(mech,target,txt)

void phys_succeed(MECH * mech, MECH * target, int at)
{
    phys_message(tprintf("%s %%s!", phys_form(at, 0)));
}

void phys_fail(MECH * mech, MECH * target, int at)
{
    phys_message(tprintf("attempts to %s %%s!", phys_form(at, 1)));
}

void PhysicalAttack(MECH * mech, int damageweight, int baseToHit,
    int AttackType, int argc, char **args, MAP * mech_map, int sect)
{
    MECH *target;
    float range;
    char targetID[2];
    int targetnum, roll;
    char location[20];
    int ts = 0, iwa;
    int glance = 0;

    DOCHECKMA(Fallen(mech) && (AttackType != PA_PUNCH),
	"You can't attack from a prone position.");
    DOCHECKMA(Stabilizing(mech), "You're still stabilizing after your jump.");

/* Ack. Never saw this here until after manually fixing the hatchet-type attacks primary functions to apply mods */
/* In future, try to apply mods here. Will also move the others here once my headache goes away. - DJ 9/05/00 */
/*#define Check(num,okval,mod) \
  if (AttackType == PA_PUNCH || AttackType == PA_KICK) \
    if (MechType(mech) == CLASS_MECH) \
      if (PartIsNonfunctional(mech, sect, num) || GetPartType(mech, sect, num) != \
          I2Special(okval)) \
	baseToHit += mod;*/

#define Check(num,okval,mod) \
  if (MechType(mech) == CLASS_MECH) \
    if (PartIsNonfunctional(mech, sect, num) || GetPartType(mech, sect, num) != I2Special(okval)) \
      baseToHit += mod;

    Check(1, UPPER_ACTUATOR, 2);
    Check(2, LOWER_ACTUATOR, 2);
    Check(3, HAND_OR_FOOT_ACTUATOR, 1);
    DOCHECKMA((MechStatus2(mech) & UNJAMMING),
	"You are currently unjamming a jammed weapon. No other actions may be taken.");
    if (AttackType == PA_KICK)
	DOCHECKMA(PartIsNonfunctional(mech, sect, 0),
	    "Your hip's destroyed, you are unable to kick with the leg.");
    if (SectHasBusyWeap(mech, sect)) {
	ArmorStringFromIndex(sect, location, MechType(mech), MechMove(mech));
	mech_notify(mech, MECHALL, tprintf("You have weapons recycling on your %s.", location));
	return;
    }
    switch (AttackType) {
    case PA_MACE:
    case PA_AXE:
    case PA_SWORD:
    if (AttackType == PA_SWORD) {
		if (MechSections(mech)[LARM].recycle || MechSections(mech)[RARM].recycle)
			baseToHit += 2;
	} else {
	DOCHECKMA(MechSections(mech)[LARM].recycle || MechSections(mech)[RARM].recycle,
	    "You still have arms recovering from another attack.");
	}
	DOCHECKMA(MechSections(mech)[LLEG].recycle || MechSections(mech)[RLEG].recycle,
		"You're still recovering from another attack.");
	break;
    case PA_PUNCH:
	DOCHECKMA(MechSections(mech)[LLEG].recycle ||
	    MechSections(mech)[RLEG].recycle,
	    "You're still recovering from another attack.");
	if (MechSections(mech)[LARM].recycle)
		DOCHECKMA(MechStatus2(mech) & LAXE,
/*		DOCHECKMA(MechSections(mech)[LARM].config & AXED, */
                    "You are recovering from another attack.");
	if (MechSections(mech)[RARM].recycle)
		DOCHECKMA(MechStatus2(mech) & RAXE,
/*                DOCHECKMA(MechSections(mech)[RARM].config & AXED, */
                    "You are recovering from another attack.");
	break;
    case PA_CLUB:
	DOCHECKMA(MechSections(mech)[LLEG].recycle ||
	    MechSections(mech)[RLEG].recycle,
	    "You're still recovering from your kick.");
	/* Check Weapons recycling on LARM because we only checked RARM above. */
	DOCHECKMA(SectHasBusyWeap(mech, LARM),
	    "You have weapons recycling on your Left Arm.");
	break;
    case PA_KICK:
	if (MechMove(mech) == MOVE_QUAD) {
	    DOCHECKMA(MechSections(mech)[LLEG].recycle ||
		MechSections(mech)[RLEG].recycle,
		"Your rear legs are still recovering from your last attack.");
	    DOCHECKMA(MechSections(mech)[RARM].recycle ||
		MechSections(mech)[LARM].recycle,
		"Your front legs are not ready to attack again.");

	} else {
	    DOCHECKMA(MechSections(mech)[LLEG].recycle ||
		MechSections(mech)[RLEG].recycle,
		"Your legs are not ready to attack again.");
	    DOCHECKMA(MechSections(mech)[RARM].recycle ||
		MechSections(mech)[LARM].recycle,
		"Your arms are still recovering from your last attack.");
	}
	break;
    }

    /* major section recycle */
    if (MechSections(mech)[sect].recycle != 0) {
	ArmorStringFromIndex(sect, location, MechType(mech),
	    MechMove(mech));
	mech_notify(mech, MECHALL,
	    tprintf("Your %s is not ready to attack again.", location));
	return;
    }
    switch (argc) {
    case -1:
    case 0:
	DOCHECKMA(MechTarget(mech) == -1,
	    "You do not have a default target set!");
	target = getMech(MechTarget(mech));
	DOCHECKMA(!target, "Invalid default target!");
	DOCHECKMA((range = FaMechRange(mech, target)) >= 1, "Target out of range!");
	DOCHECKMA(!InLineOfSight_NB(mech, target, MechX(target),
		MechY(target), range),
	    "You are unable to hit that target at the moment.");
	DOCHECKMA(((MechTeam(mech) == MechTeam(target)) && (Started(target)) && (!Destroyed(target))) , "Friendly units ? I dont Think so..");
	break;
    default:
	/* Any number of targets, take only the first -- mw 93 Oct */
	targetID[0] = args[0][0];
	targetID[1] = args[0][1];
	targetnum = FindTargetDBREFFromMapNumber(mech, targetID);
	DOCHECKMA(targetnum == -1, "That is not a valid target ID!");
	target = getMech(targetnum);
	DOCHECKMA(!target, "Invalid default target!");
	DOCHECKMA(((MechTeam(mech) == MechTeam(target)) && (Started(target)) && (!Destroyed(target))) , "Friendly units ? I dont Think so..");
	DOCHECKMA((range =
		FaMechRange(mech, target)) >= 1, "Target out of range!");
	DOCHECKMA(!InLineOfSight_NB(mech, target, MechX(target),
		MechY(target), range),
	    "You are unable to hit that target at the moment.");
    }
    DOCHECKMA(((MechTeam(mech) == MechTeam(target)) && (Started(target)) && (!Destroyed(target))) , "Friendly units ? I dont Think so..");

    if (MechMove(target) != MOVE_VTOL && MechMove(target) != MOVE_FLY) {
        DOCHECKMA(MechType(target) == CLASS_BSUIT &&
		  MechSwarmTarget(target) > 1 && AttackType != PA_PUNCH && AttackType != PA_AXE &&
		  AttackType != PA_MACE && AttackType != PA_SWORD,
		"You cannot get a fix on the swarming/mounting battlesuit!");
	DOCHECKMA((AttackType == PA_PUNCH || AttackType == PA_AXE ||
		AttackType == PA_MACE || AttackType == PA_SWORD) &&
	    (MechZ(mech) - 1) > MechZ(target),
	    tprintf("The target is too low in elevation for you to %s",
		AttackType == PA_PUNCH ? "punch at." : AttackType == PA_AXE ? "axe it." :
		AttackType == PA_MACE ? "bash it." : AttackType == PA_SWORD ? "chop it." : "error page a wiz."));
	DOCHECKMA(AttackType == PA_KICK &&
	    MechZ(mech) < MechZ(target),
	    "The target is too high in elevation for you to kick at.");
	DOCHECKMA((AttackType == PA_KICK || AttackType == PA_CLUB || AttackType == PA_AXE ||
		AttackType == PA_SWORD || AttackType == PA_MACE) && MechZ(target) < MechZ(mech) &&
		((MechType(target) == CLASS_MECH && Fallen(target)) || MechType(target) != CLASS_MECH),
		"The target is too low!");
	DOCHECKMA(MechZ(mech) - MechZ(target) > 1 ||
	    MechZ(target) - MechZ(mech) > 1,
	    "You can't attack, the elevation difference is too large.");
    } else {
	DOCHECKMA((AttackType == PA_PUNCH || AttackType == PA_AXE ||
		AttackType == PA_MACE || AttackType == PA_SWORD) &&
	    MechZ(target) - MechZ(mech) > 3,
	    tprintf("The target is too far away for you to %s",
		AttackType == PA_PUNCH ? "punch at." : AttackType == PA_AXE ? "axe it." :
		AttackType == PA_MACE ? "bash it." : AttackType == PA_SWORD ? "chop it." : "error page a wiz."));
	DOCHECKMA(AttackType == PA_KICK &&
	    MechZ(mech) != MechZ(target),
	    "The target is too far away for you to kick at.");
	DOCHECKMA(!(MechZ(target) - MechZ(mech) > -1 &&
		MechZ(target) - MechZ(mech) < 4),
	    "You can't attack, the elevation difference is too large.");
    }

    /* Check weapon arc! */
    /* Theoretically, physical attacks occur only to 'real' forward
       arc, not rottorsoed one, but we let it pass this time */
    /* Eh? Sure looks like kicks are removing the Rottorso arc's....  - DJ */

    if (AttackType == PA_KICK) {
	ts = MechStatus(mech) & (TORSO_LEFT | TORSO_RIGHT);
	MechStatus(mech) &= ~ts;
    }
    iwa = InWeaponArc(mech, MechFX(target), MechFY(target));
    if (AttackType == PA_KICK)
	MechStatus(mech) |= ts;
    if (!(MechSwarmTarget(target) == mech->mynum && MechType(target) == CLASS_BSUIT))
	DOCHECKMA((!(iwa & FORWARDARC) && !(MechMove(mech) == MOVE_QUAD && AttackType == PA_KICK)), "Target is not in forward arc!");

    /* Add in the movement modifiers */
    baseToHit += ((HasBoolAdvantage(MechPilot(mech), "melee_specialist") ? MIN(0, AttackMovementMods(mech) - 1) : AttackMovementMods(mech)) + ((MechMove(mech) == MOVE_QUAD && AttackType == PA_KICK && !(iwa & FORWARDARC)) ? 1 : 0));
    baseToHit += TargetMovementMods(mech, target, 0.0);
    baseToHit += MechType(target) == CLASS_BSUIT ? (AttackType == PA_KICK ? 3 : 1) : 0;
    if (MechStatus2(target) & DODGING) {
	baseToHit += 2;
/*	DodgeOff(target); */
	}
    if ((AttackType == PA_PUNCH || AttackType == PA_AXE || AttackType == PA_MACE ||
	    AttackType == PA_SWORD) && MechType(target) == CLASS_BSUIT &&
	MechSwarmTarget(target) > 0)
	baseToHit += 4;
    DOCHECKMA((AttackType != PA_PUNCH && MechType(target) == CLASS_BSUIT && MechSwarmTarget(target) > 0),
	     "You can hit swarming 'suits only with punches!");
/* Let's finally add terrain mods after all these freakin' years - DJ */
    if (MechRTerrain(target) == LIGHT_FOREST)
	baseToHit += 1;
    if (MechRTerrain(target) == HEAVY_FOREST)
	baseToHit += 2;
    if (MechTerrain(target) == SMOKE)
	baseToHit += 1;
    if (MechTerrain(target) == HSMOKE)
	baseToHit += 2;
    if (MechTerrain(target) == WATER && !Jumping(target) && MechZ(target) != 0)
	baseToHit -= 1;
    if (MechTerrain(mech) == WATER && !Jumping(mech) && MechZ(mech) != 0)
	baseToHit += 1;
    roll = Roll();
    glance = ((roll == (baseToHit - 1) && mudconf.btech_glance) ? 1 : 0);
    switch (AttackType) {
    case PA_PUNCH:
	if (MechElevation(mech) >= MechElevation(target))
		DOCHECKMA((((Fallen(target) && MechType(target) == CLASS_MECH) ||
	    		(MechType(target) != CLASS_MECH &&
		    	 MechSwarmTarget(target) != mech->mynum)) &&
			!IsDS(target) && !Fallen(mech)), tprintf("The target is too low to %s!",
			"punch"
			));
	DOCHECKMA(Jumping(target) || (Jumping(mech) &&
		MechType(target) == CLASS_MECH),
	    "You cannot physically attack a jumping mech!");
	DOCHECKMA(Standing(mech), "You are still trying to stand up!");
	mech_notify(mech, MECHALL,
	     tprintf("You try to punch the %s.  BTH:  %d,\tRoll:  %d",
	             GetMechToMechID(mech, target), baseToHit, roll));
	     mech_notify(target, MECHSTARTED,
		         tprintf("%s tries to punch you!", GetMechToMechID(target,
			 mech)));
	     SendAttacks(tprintf("#%i attacks #%i (punch) (%i/%i)", mech->mynum,
		            target->mynum, baseToHit, roll));
	break;
    case PA_MACE:
    case PA_SWORD:
    case PA_AXE:
    case PA_CLUB:
	DOCHECKMA(Jumping(target) || (Jumping(mech) &&
		MechType(target) == CLASS_MECH),
	    "You cannot physically attack a jumping mech!");
	DOCHECKMA(Standing(mech), "You are still trying to stand up!");
	if (AttackType == PA_CLUB) {
	    mech_notify(mech, MECHALL,
	        tprintf("You try and club %s.  BaseToHit:  %d,\tRoll:  %d",
		    GetMechToMechID(mech, target), baseToHit, roll));
	    mech_notify(target, MECHSTARTED, tprintf("%s tries to club you!",
		    GetMechToMechID(target, mech)));
	    SendAttacks(tprintf("#%i attacks #%i (club) (%i/%i)", mech->mynum,
		    target->mynum, baseToHit, roll));
	} else {
	   mech_notify(mech, MECHALL,
                tprintf
                ("You try to swing your %s at %s.  BTH:  %d,\tRoll:  %d",
             AttackType == PA_SWORD ? "sword" : AttackType == PA_MACE ? "mace" : "axe",
                    GetMechToMechID(mech, target), baseToHit, roll));
            mech_notify(target, MECHSTARTED, tprintf("%s tries to %s you!",
                    GetMechToMechID(target, mech),
                    AttackType == PA_SWORD ? "swing a sword at" : AttackType == PA_MACE ? "bash" : "axe"));
            SendAttacks(tprintf("#%i attacks #%i (%s) (%i/%i)", mech->mynum,
                    target->mynum,
                    AttackType == PA_SWORD ? "sword" : AttackType == PA_MACE ? "mace" : "axe", baseToHit,
                    roll));
        }

	break;
    case PA_KICK:
	DOCHECKMA(Jumping(target) || (Jumping(mech) &&
		MechType(target) == CLASS_MECH),
	    "You cannot physically attack a jumping mech!");
	DOCHECKMA(Standing(mech), "You are still trying to stand up!");
	mech_notify(mech, MECHALL,
	    tprintf("You try and kick %s.  BaseToHit:  %d,\tRoll:  %d",
		GetMechToMechID(mech, target), baseToHit, roll));
	mech_notify(target, MECHSTARTED, tprintf("%s tries to kick you!",
		GetMechToMechID(target, mech)));
	SendAttacks(tprintf("#%i attacks #%i (kick) (%i/%i)", mech->mynum,
		target->mynum, baseToHit, roll));
    }

    /* set the sections to recycling */

    SetRecycleLimb(mech, sect, PHYSICAL_RECYCLE_TIME);
    if (AttackType == PA_AXE || AttackType == PA_SWORD || AttackType == PA_MACE) {
	if (sect == LARM)
	    MechStatus2(mech) |= LAXE;
	else
	    MechStatus2(mech) |= RAXE;
	}
/*	MechSections(mech)[sect].config |= AXED; */
    if (AttackType == PA_PUNCH) {
	if (sect == LARM)
	    MechStatus2(mech) &= ~LAXE;
	else
	    MechStatus2(mech) &= ~RAXE;
	}
/*	MechSections(mech)[sect].config &= ~AXED; */
    if (AttackType == PA_CLUB)
	SetRecycleLimb(mech, LARM, PHYSICAL_RECYCLE_TIME);
    if (roll >= (baseToHit - (mudconf.btech_glance ? 1 : 0))) {	/*  hit the target */
	phys_succeed(mech, target, AttackType);
	PhysicalDamage(mech, target, damageweight, AttackType, sect, glance);
    } else {
	phys_fail(mech, target, AttackType);
	if (MechType(target) == CLASS_BSUIT &&
	    MechSwarmTarget(target) == mech->mynum) {
	    if (!MadePilotSkillRoll(mech, 4)) {
		mech_notify(mech, MECHALL,
		    "Uh oh. You miss the little buggers, but hit yourself!");
		MechLOSBroadcast(mech, "misses, and hits itself!");
		PhysicalDamage(mech, mech, damageweight, AttackType, sect, glance);
	    }
	}
	if (AttackType == PA_KICK || AttackType == PA_MACE) {
	    mech_notify(mech, MECHALL,
		"You miss and try to remain standing!");
	    if (!MadePilotSkillRoll(mech, 0)) {
		mech_notify(mech, MECHALL,
		    "You lose your balance and fall down!");
		MechFalls(mech, 1, 1);
	    }
	}
    }
}

extern int global_physical_flag;

#define MyDamageMech(a,b,c,d,e,f,g,h,i,j) \
        global_physical_flag = 1 ; DamageMech(a,b,c,d,e,f,g,h,i,-1,0,j); \
        global_physical_flag = 0
#define MyDamageMech2(a,b,c,d,e,f,g,h,i,j) \
        global_physical_flag = 2 ; DamageMech(a,b,c,d,e,f,g,h,i,-1,0,j); \
        global_physical_flag = 0

void PhysicalDamage(MECH * mech, MECH * target, int weightdmg,
    int AttackType, int sect, int glance)
{
    int hitloc = 0, damage, hitgroup = 0, isrear, iscritical, i;

	damage = (MechTons(mech) + weightdmg / 2) / weightdmg;
	if (AttackType == PA_SWORD)
		damage++;
	else if (AttackType == PA_MACE)
		damage *= 2;

    if ((MechHeat(mech) >= 9.) &&
	(MechSpecials(mech) & TRIPLE_MYOMER_TECH)) damage = damage * 2;

	if ((MechRTerrain(target) == WATER && (MechZ(target) <= -2 || (Fallen(target) && MechZ(target) < 0)))
	 || MechRTerrain(target) == HIGHWATER)
	 	damage = (damage + 1) / 2;
    if (HasBoolAdvantage(MechPilot(mech), "melee_specialist"))
	damage++;

	if (!OkayCritSectS(sect, 1, UPPER_ACTUATOR))
		damage /= 2;
	if (!OkayCritSectS(sect, 2, LOWER_ACTUATOR))
		damage /= 2;

    switch (AttackType) {
    case PA_PUNCH:
/*	if (sect == LARM && !OkayCritSectS(LARM, 2, LOWER_ACTUATOR)) {
	    damage = damage / 2;
	    if (!OkayCritSectS(LARM, 1, UPPER_ACTUATOR))
		damage = damage / 2;
	} else if (sect == RARM && !OkayCritSectS(RARM, 2, LOWER_ACTUATOR)) {
	    damage = damage / 2;
	    if (!OkayCritSectS(RARM, 1, UPPER_ACTUATOR))
		damage = damage / 2;
	}*/
	hitgroup = FindAreaHitGroup(mech, target);
	isrear = hitgroup == REAR;
        if (MechType(mech) == CLASS_MECH) {
	    if (Fallen(mech)) {
		if ((MechType(target) != CLASS_MECH) || (Fallen(target) && (MechElevation(mech) == MechElevation(target))))
			hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
		else if (!Fallen(target) && (MechElevation(mech) > MechElevation(target)))
			hitloc = FindPunchLocation(hitgroup, target);
		else if (MechElevation(mech) == MechElevation(target))
			hitloc = FindKickLocation(hitgroup, target);
            } else if (MechElevation(mech) < MechElevation(target)) {
			if (Fallen(target) || MechType(target) != CLASS_MECH)
				hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
			else
                		hitloc = FindKickLocation(hitgroup, target);
             	} else hitloc = FindPunchLocation(hitgroup, target);
	} else {
            hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
        }
        break;


    case PA_MACE:
    case PA_SWORD:
    case PA_AXE:
    case PA_CLUB:
	hitgroup = FindAreaHitGroup(mech, target);
	isrear = hitgroup == REAR;
	if (MechType(mech) == CLASS_MECH) {
	    if (MechElevation(mech) < MechElevation(target))
		if (Fallen(target) || MechType(target) != CLASS_MECH)
			hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
		else
			hitloc = FindKickLocation(hitgroup, target);
	    else if (MechElevation(mech) > MechElevation(target))
			hitloc = FindPunchLocation(hitgroup, target);
 	    else
			hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
	} else {
	    hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
	}
	break;

    case PA_KICK:
	hitgroup = FindAreaHitGroup(mech, target);
	isrear = hitgroup == REAR;
	if (Fallen(target) || MechType(target) != CLASS_MECH)
	    hitloc = FindTargetHitLoc(mech, target, &isrear, &iscritical);
	else {
	    hitgroup = FindAreaHitGroup(mech, target);
	    if (MechElevation(mech) > MechElevation(target))
		hitloc = FindPunchLocation(hitgroup, target);
	    else {
		hitloc = FindKickLocation(hitgroup, target);
		if (!GetSectInt(target, hitloc) &&
		    GetSectInt(target, (i =
BOUNDED(0, 5 + (6 - hitloc), NUM_SECTIONS - 1))))
		    hitloc = i;
	    }
	}
	break;
    }
    if (glance && mudconf.btech_glance)
	{
	damage = (damage + 1) / 2;
	if (damage < 1)
	    damage = 1;
	}
    MyDamageMech(target, mech, 1, MechPilot(mech), hitloc,
	(hitgroup == BACK) ? 1 : 0, 0, damage, 0, glance);
    if (MechType(target) == CLASS_BSUIT && MechSwarmTarget(target) > 0 &&
	(AttackType == PA_PUNCH || AttackType == PA_AXE ||
	    AttackType == PA_SWORD || AttackType == PA_MACE))
	    bsuit_stopswarmer(FindObjectsData(MechSwarmTarget(target)),
	    target, 0);
    if (MechType(target) == CLASS_MECH && AttackType == PA_KICK) {
	int attmod = 0, defmod = 0;
	attmod = (MechTons(mech) <= 35 ? 1 : MechTons(mech) <= 55 ? 0 : MechTons(mech) <= 75 ? -1 : -2);
	defmod = (MechTons(target) <= 35 ? 1 : MechTons(target) <= 55 ? 0 : MechTons(target) <= 75 ? -1 : -2);
	if (!Fallen(target) && !MadePilotSkillRoll(target, defmod - attmod)) {
	    mech_notify(target, MECHSTARTED,
		"The kick knocks you to the ground!");
	    MechLOSBroadcast(target, "stumbles and falls down!");
	    MechFalls(target, 1, 0);
	}
    }
}

#define CHARGE_SECTIONS 6
#define DFA_SECTIONS    4

/* 4 if pure FASA */

const int resect[CHARGE_SECTIONS] =
    { LARM, RARM, LLEG, RLEG, LTORSO, RTORSO };

int DeathFromAbove(MECH * mech, MECH * target)
{
    int baseToHit = 5;
    int roll;
    int hitGroup;
    int hitloc;
    int isrear = 0;
    int iscritical = 0;
    int target_damage = 0;
    int mech_damage = 0;
    int spread;
    int i, tmpi;
    char location[50];

    /* Weapons recycling check on each major section */
    for (i = 0; i < DFA_SECTIONS; i++)
	if (SectHasBusyWeap(mech, resect[i])) {
	    ArmorStringFromIndex(resect[i], location, MechType(mech),
		MechMove(mech));
	    mech_notify(mech, MECHALL,
		tprintf("You have weapons recycling on your %s.",
		    location));
	    return 0;
	}

    DOCHECKMA0((mech->mapindex != target->mapindex), "Invalid Target.");

    DOCHECKMA0(((MechTeam(mech) == MechTeam(target)) && (Started(target)) && 
        (!Destroyed(target))), "Friendly units ? I dont Think so..");

    DOCHECKMA0(Dodging(mech) || MoveModeLock(mech), "You cannot use physicals while using a special movement mode.");
    DOCHECKMA0(MechSections(mech)[LLEG].recycle ||
	MechSections(mech)[RLEG].recycle,
	"Your legs are still recovering from your last attack.");
    DOCHECKMA0(MechSections(mech)[RARM].recycle ||
	MechSections(mech)[LARM].recycle,
	"Your arms are still recovering from your last attack.");
    DOCHECKMA0(Jumping(target),
	"Your target is jumping, you cannot land on it.");

    if ((MechType(target) == CLASS_VTOL) || (MechType(target) == CLASS_AERO) ||
	(MechType(target) == CLASS_DS))
    	DOCHECKMA0(!Landed(target), "Your target is airborne, you cannot land on it.");
 
    if (mudconf.phys_use_pskill)
	baseToHit = FindPilotPiloting(mech);
    if (MechStatus2(target) & DODGING) {
	baseToHit += 2;
/*	DodgeOff(target); */
	}
    baseToHit += (HasBoolAdvantage(MechPilot(mech), "melee_specialist") ? MIN(0, AttackMovementMods(mech)) - 1 : AttackMovementMods(mech));
    baseToHit += TargetMovementMods(mech, target, 0.0);
    baseToHit += MechType(target) == CLASS_BSUIT ? 1 : 0;


    DOCHECKMA0(baseToHit > 12,
	tprintf
	("DFA: BTH %d\tYou choose not to attack and land from your jump.",
     baseToHit));
    roll = Roll();
    mech_notify(mech, MECHALL, tprintf("DFA: BTH %d\tRoll: %d", baseToHit,
	    roll));
    MechStatus(mech) &= ~JUMPING;
    MechStatus(mech) &= ~DFA_ATTACK;
    if (roll >= baseToHit) {
	/* OUCH */
	mech_notify(target, MECHSTARTED,
	    tprintf("DEATH FROM ABOVE!!!\n%s lands on you from above!",
		GetMechToMechID(target, mech)));
	mech_notify(mech, MECHALL, "You land on your target legs first!");
	MechLOSBroadcasti(mech, target, "lands on %s!");
	hitGroup = FindAreaHitGroup(mech, target);
	if (hitGroup == BACK)
	    isrear = 1;

	target_damage = 3 * MechTons(mech) / 10;
	if (MechTons(mech) % 10) 
	   target_damage++;

	if (HasBoolAdvantage(MechPilot(mech), "melee_specialist"))
	    target_damage++;
	spread = target_damage / 5;
	for (i = 0; i < spread; i++) {
	    if (Fallen(target) || MechType(target) != CLASS_MECH)
		hitloc =
		    FindHitLocation(target, hitGroup, &iscritical,
		    &isrear, mech);
	    else
		hitloc = FindPunchLocation(hitGroup, target);
	    MyDamageMech(target, mech, 1, MechPilot(mech), hitloc, isrear,
		iscritical, 5, 0, 0);
	}
	if (target_damage % 5) {
	    if (Fallen(target) || MechType(target) != CLASS_MECH)
		hitloc =
		    FindHitLocation(target, hitGroup, &iscritical,
		    &isrear, mech);
	    else
		hitloc = FindPunchLocation(hitGroup, target);
	    MyDamageMech(target, mech, 1, MechPilot(mech), hitloc, isrear,
		iscritical, (target_damage % 5), 0, 0);
	}
	mech_damage = MechTons(mech) / 5;
	spread = mech_damage / 5;
	for (i = 0; i < spread; i++) {
	    hitloc = FindKickLocation(FRONT, target);
	    MyDamageMech2(mech, mech, 0, -1, hitloc, 0, 0, 5, 0, 0);
	}
	if (mech_damage % 5) {
	    hitloc = FindKickLocation(FRONT, target);
	    MyDamageMech2(mech, mech, 0, -1, hitloc, 0, 0,
		(mech_damage % 5), 0, 0);
	}
	if (!Fallen(mech)) {
	    if (!MadePilotSkillRoll(mech, 4)) {
		mech_notify(mech, MECHALL,
		    "Your piloting skill fails and you fall over!!");
		MechLOSBroadcast(mech, "stumbles and falls down!");
		MechFalls(mech, 1, 0);
	    }
	    if (MechType(target) == CLASS_MECH &&
		!MadePilotSkillRoll(target, 2)) {
		mech_notify(target, MECHSTARTED,
		    "Your piloting skill fails and you fall over!!");
		MechLOSBroadcast(target, "stumbles and falls down!");
		MechFalls(target, 1, 0);
	    }
	}
    } else {
	/* Missed DFA attack */
	if (!Fallen(mech)) {
	    mech_notify(mech, MECHALL,
		"You miss your DFA attack and fall on your back!!");
	    MechLOSBroadcast(mech, "misses DFA and falls down!");
	}
	mech_damage = 2 * (MechTons(mech) + 5) / 10;
	spread = mech_damage / 5;
	for (i = 0; i < spread; i++) {
	    hitloc = FindHitLocation(mech, BACK, &iscritical, &tmpi, target);
	    MyDamageMech2(mech, mech, 0, -1, hitloc, 1, iscritical, 5, 0, 0);
	}
	if (mech_damage % 5) {
	    hitloc = FindHitLocation(mech, BACK, &iscritical, &tmpi, target);
	    MyDamageMech2(mech, mech, 0, -1, hitloc, 1, iscritical,
		(mech_damage % 5), 0, 0);
	}
	/* now damage pilot */
	if (!MadePilotSkillRoll(mech, 2)) {
	    mech_notify(mech, MECHALL,
		"You take personal injury from the fall!");
	    headhitmwdamage(mech, 1);
	}
	MechSpeed(mech) = 0.0;
	MechDesiredSpeed(mech) = 0.0;
	MakeMechFall(mech);
	MechZ(mech) = MechElevation(mech);
	MechFZ(mech) = MechZ(mech) * ZSCALE;
	if (MechZ(mech) < 0)
	    MechFloods(mech);
    }
    for (i = 0; i < DFA_SECTIONS; i++)
	SetRecycleLimb(mech, resect[i], PHYSICAL_RECYCLE_TIME);
    return 1;
}

void ChargeMech(MECH * mech, MECH * target)
{
    int baseToHit = 5;
    int roll;
    int hitGroup;
    int hitloc;
    int isrear = 0;
    int iscritical = 0;
    int target_damage;
    int mech_damage;
    int received_damage;
    int inflicted_damage; 
    int spread;
    int i;
    int mech_charge;
    int target_charge;
    int mech_baseToHit;
    int targ_baseToHit;
    int mech_roll;
    int targ_roll;
    int done = 0;
    char location[50];
    int ts, iwa;
    char emit_buff[LBUF_SIZE];

    /* Are they both charging ? */
    if (MechChargeTarget(target) == mech->mynum) {
        /* They are both charging each other */
        mech_charge = 1;
        target_charge = 1;

        /* Check the sections of the first unit for weapons that are cycling */
        done = 0;
        for (i = 0; i < CHARGE_SECTIONS && !done; i++) {
            if (SectHasBusyWeap(mech, resect[i])) {
                ArmorStringFromIndex(resect[i], location, MechType(mech),
                    MechMove(mech));
                mech_notify(mech, MECHALL,
                    tprintf("You have weapons recycling on your %s.",
                        location));
                mech_charge = 0;
                done = 1;
            }
        }

        /* Check the sections of the second unit for weapons that are cycling */
        done = 0;
        for (i = 0; i < CHARGE_SECTIONS && !done; i++) {
            if (SectHasBusyWeap(target, resect[i])) {
                ArmorStringFromIndex(resect[i], location, MechType(target),
                    MechMove(target));
                mech_notify(target, MECHALL,
                    tprintf("You have weapons recycling on your %s.",
                        location));
                target_charge = 0;
                done = 1;
            }
        }

        /* Is the second unit capable of charging */
        if (!Started(target) || Uncon(target) || Blinded(target))
            target_charge = 0;
        /* Is the first unit capable of charging */
        if (!Started(mech) || Uncon(mech) || Blinded(mech))
            mech_charge = 0;

        /* Is the first unit moving fast enough to charge */
        if (MechSpeed(mech) < MP1) {
            mech_notify(mech, MECHALL,
                "You aren't moving fast enough to charge.");
            mech_charge = 0;
        }

        /* Is the second unit moving fast enough to charge */
        if (MechSpeed(target) < MP1) {
            mech_notify(target, MECHALL,
                "You aren't moving fast enough to charge.");
            target_charge = 0;
        }

        /* Check to see if any sections cycling from a previous attack */
        if (MechType(mech) == CLASS_MECH) {
            /* Is the first unit's legs cycling */
            if (MechSections(mech)[LLEG].recycle ||
                MechSections(mech)[RLEG].recycle) {
                mech_notify(mech, MECHALL,
                    "Your legs are still recovering from your last attack.");
                mech_charge = 0;
            } 
            /* Is the first unit's arms cycling */
            if (MechSections(mech)[RARM].recycle ||
                MechSections(mech)[LARM].recycle) {
                mech_notify(mech, MECHALL,
                    "Your arms are still recovering from your last attack.");
                mech_charge = 0;
            }
        } else {
            /* Is the first unit's front side cycling */
            if (MechSections(mech)[FSIDE].recycle) {
                mech_notify(mech, MECHALL,
                    "You are still recovering from your last attack!");
                mech_charge = 0;
            }
        }

        /* Check to see if any sections cycling from a previous attack */
        if (MechType(target) == CLASS_MECH) {
            /* Is the second unit's legs cycling */
            if (MechSections(target)[LLEG].recycle ||
                MechSections(target)[RLEG].recycle) {
                mech_notify(target, MECHALL,
                    "Your legs are still recovering from your last attack.");
                target_charge = 0;
            }
            /* Is the second unit's arms cycling */
            if (MechSections(target)[RARM].recycle ||
                MechSections(target)[LARM].recycle) {
                mech_notify(target, MECHALL,
                    "Your arms are still recovering from your last attack.");
                target_charge = 0;
            }
        } else {
            /* Is the second unit's front side cycling */
            if (MechSections(target)[FSIDE].recycle) {
                mech_notify(target, MECHALL,
                    "You are still recovering from your last attack!");
                target_charge = 0;
            }
        }
  
        /* Is the second unit jumping */
        if (Jumping(target)) {
            mech_notify(mech, MECHALL,
                "Your target is jumping, you charge underneath it.");
            mech_notify(target, MECHALL,
                "You can't charge while jumping, try death from above.");
            mech_charge = 0;
            target_charge = 0;
        }

        /* Is the first unit jumping */
        if (Jumping(mech)) {
            mech_notify(target, MECHALL,
                "Your target is jumping, you charge underneath it.");
            mech_notify(mech, MECHALL,
                "You can't charge while jumping, try death from above.");
            mech_charge = 0;
            target_charge = 0;
        }

        /* Is the second unit fallen and the first unit not a tank */
        if (Fallen(target) && (MechType(mech) != CLASS_VEH_GROUND)) {
            mech_notify(mech, MECHALL,
                "Your target's too low for you to charge it!");
            mech_charge = 0;
        }

        /* Not sure at the moment if I need this here, but I figured
         * couldn't hurt for now */
        /* Is the first unit fallen and the second unit not a tank */
        if (Fallen(mech) && (MechType(target) != CLASS_VEH_GROUND)) {
            mech_notify(target, MECHALL,
                "Your target's too low for you to charge it!");
            target_charge = 0;
        }

        /* If the second unit is a mech it can only charge mechs */
        if ((MechType(target) == CLASS_MECH) && (MechType(mech) != CLASS_MECH)) {
            mech_notify(target, MECHALL, "You can only charge mechs!");
            target_charge = 0;
        }

        /* If the first unit is a mech it can only charge mechs */
        if ((MechType(mech) == CLASS_MECH) && (MechType(target) != CLASS_MECH)) {
            mech_notify(mech, MECHALL, "You can only charge mechs!");
            mech_charge = 0;
        }

        /* If the second unit is a tank, it can only charge tanks and mechs */
        if ((MechType(target) == CLASS_VEH_GROUND) && 
                ((MechType(mech) != CLASS_MECH) && 
                 (MechType(mech) != CLASS_VEH_GROUND))) {
            mech_notify(target, MECHALL, "You can only charge mechs and tanks!");
            target_charge = 0;
        }

        /* If the first unit is a tank, it can only charge tanks and mechs */
        if ((MechType(mech) == CLASS_VEH_GROUND) && 
                ((MechType(target) != CLASS_MECH) && 
                 (MechType(target) != CLASS_VEH_GROUND))) {
            mech_notify(mech, MECHALL, "You can only charge mechs and tanks!");
            mech_charge = 0;
        }

	ts = MechStatus(mech) & (TORSO_LEFT | TORSO_RIGHT);
	MechStatus(mech) &= ~ts;
	if (!(InWeaponArc(mech, MechFX(target),
		MechFY(target)) & FORWARDARC)) {
	    mech_notify(mech, MECHALL,
		"Your charge target is not in your forward arc and you are unable to charge it.");
	    mech_charge = 0;
	}
	MechStatus(mech) |= ts;
	ts = MechStatus(target) & (TORSO_LEFT | TORSO_RIGHT);
	MechStatus(mech) &= ~ts;
	if (!(InWeaponArc(target, MechFX(mech), MechFY(mech)) & FORWARDARC)) {
	    mech_notify(target, MECHALL,
		"Your charge target is not in your forward arc and you are unable to charge it.");
	    target_charge = 0;
	}
	MechStatus(mech) |= ts;
	if (mudconf.btech_newcharge)
	    target_damage = (((((float)
			    MechChargeDistance(mech)) * MP1) -
		    MechSpeed(target) * cos((MechFacing(mech) -
			    MechFacing(target)) * (M_PI / 180.))) *
		MP_PER_KPH) * (MechRTons(mech) / 1024 + 5) / 10;
	else
	    target_damage =
		((MechSpeed(mech) -
	MechSpeed(target) * cos((MechFacing(mech) -
			    MechFacing(target)) * (M_PI / 180.))) *
		MP_PER_KPH) * (MechRTons(mech) / 1024 + 5) / 10;
	if (HasBoolAdvantage(MechPilot(mech), "melee_specialist"))
	    target_damage++;
	if (target_damage <= 0) {
	    mech_notify(mech, MECHPILOT,
		"Your target pulls away from you and you are unable to charge it.");
	    mech_charge = 0;
	}

	if (mudconf.btech_newcharge)
	    mech_damage = (((((float)
			    MechChargeDistance(target)) * MP1) -
		    MechSpeed(mech) * cos((MechFacing(target) -
			    MechFacing(mech)) * (M_PI / 180.))) *
		MP_PER_KPH) * (MechRTons(target) / 1024 + 5) / 10;
	else
	    mech_damage =
		((MechSpeed(target) -
	  MechSpeed(mech) * cos((MechFacing(target) -
			    MechFacing(mech)) * (M_PI / 180.))) *
		MP_PER_KPH) * (MechRTons(target) / 1024 + 5) / 10;
	if (mech_damage <= 0) {
	    mech_notify(target, MECHPILOT,
		"Your target pulls away from you and you are unable to charge it.");
	    target_charge = 0;
	}

	mech_baseToHit = 5;
	mech_baseToHit +=
	    FindPilotPiloting(mech) - FindSPilotPiloting(target);
	mech_baseToHit += (HasBoolAdvantage(MechPilot(mech), "melee_specialist") ? MIN(0, AttackMovementMods(mech) - 1) : AttackMovementMods(mech));
	mech_baseToHit += TargetMovementMods(mech, target, 0.0);
	if (MechStatus2(target) & DODGING) {
	    mech_baseToHit += 2;
/*	    DodgeOff(target); */
	    }
	targ_baseToHit = 5;
	targ_baseToHit +=
	    FindPilotPiloting(target) - FindSPilotPiloting(mech);
	targ_baseToHit += (HasBoolAdvantage(MechPilot(target), "melee_specialist") ? MIN(0, AttackMovementMods(target) - 1 ) : AttackMovementMods(target));
	targ_baseToHit += TargetMovementMods(target, mech, 0.0);

	if (mech_charge)
	    if (mech_baseToHit > 12) {
		mech_notify(mech, MECHALL,
		    tprintf("Charge: BTH %d\tYou choose not to charge.",
			mech_baseToHit));
		mech_charge = 0;
	    }
	if (target_charge)
	    if (targ_baseToHit > 12) {
		mech_notify(target, MECHALL,
		    tprintf("Charge: BTH %d\tYou choose not to charge.",
			targ_baseToHit));
		target_charge = 0;
	    }
	if (!mech_charge && !target_charge) {
	    MechChargeTarget(target) = -1;	/* MechChargeTarget(mech) is set after the return */
	    MechChargeTimer(target) = 0;
	    MechChargeDistance(target) = 0;
	    return;
	}
	mech_roll = Roll();
	targ_roll = Roll();

	if (mech_charge)
	    mech_notify(mech, MECHALL, tprintf("Charge: BTH %d\tRoll: %d",
		    mech_baseToHit, mech_roll));
	if (target_charge)
	    mech_notify(target, MECHALL,
		tprintf("Charge: BTH %d\tRoll: %d", targ_baseToHit,
		    targ_roll));
	if (mech_charge && mech_roll >= mech_baseToHit) {
	    /* OUCH */
	    mech_notify(target, MECHALL,
		tprintf("CRASH!!!\n%s charges into you!",
		    GetMechToMechID(target, mech)));
	    mech_notify(mech, MECHALL,
		"SMASH!!! You crash into your target!");
	    hitGroup = FindAreaHitGroup(mech, target);
	    isrear = (hitGroup == BACK);
            inflicted_damage = target_damage;
	    spread = target_damage / 5;
	    for (i = 0; i < spread; i++) {
		hitloc =
		    FindHitLocation(target, hitGroup, &iscritical,
		    &isrear, mech);
		MyDamageMech(target, mech, 1, MechPilot(mech), hitloc,
		    isrear, iscritical, 5, 0, 0);
	    }
	    if (target_damage % 5) {
		hitloc =
		    FindHitLocation(target, hitGroup, &iscritical,
		    &isrear, mech);
		MyDamageMech(target, mech, 1, MechPilot(mech), hitloc,
		    isrear, iscritical, (target_damage % 5), 0, 0);
	    }
	    hitGroup = FindAreaHitGroup(target, mech);
	    isrear = (hitGroup == BACK);
        if (mudconf.btech_newcharge && mudconf.btech_tl3_charge)
            target_damage =
                (((((float) MechChargeDistance(mech)) * MP1) -
                MechSpeed(target) * 
                cos((MechFacing(mech) - MechFacing(target)) * (M_PI/180.))) *
                MP_PER_KPH) *
                (MechRTons(target) / 1024 + 5) / 20; 
        else
	        target_damage = (MechRTons(target) / 1024 + 5) / 10;	/* REUSED! */

            received_damage = target_damage; 
	    spread = target_damage / 5;
	    for (i = 0; i < spread; i++) {
		hitloc =
		    FindHitLocation(mech, hitGroup, &iscritical, &isrear, target);
		MyDamageMech2(mech, mech, 0, -1, hitloc, isrear,
		    iscritical, 5, 0, 0);
	    }
	    if (target_damage % 5) {
		hitloc =
		    FindHitLocation(mech, hitGroup, &iscritical, &isrear, target);
		MyDamageMech2(mech, mech, 0, -1, hitloc, isrear,
		    iscritical, (target_damage % 5), 0, 0);
	    }
	    MechSpeed(mech) = 0;	/* stop him and let him accelerate again */
            MechDesiredSpeed(mech) = 0;

            snprintf(emit_buff, LBUF_SIZE, "#%i charges #%i (%i/%i) Distance:"
               " %.2f DI: %i DR: %i", mech->mynum, target->mynum, mech_baseToHit,
               mech_roll, MechChargeDistance(mech), inflicted_damage, 
               received_damage);
            SendDebug(emit_buff);

	    if (MechType(mech) == CLASS_MECH && !MadePilotSkillRoll(mech, 2)) {
		mech_notify(mech, MECHALL,
		    "Your piloting skill fails and you fall over!!");
		MechFalls(mech, 1, 1);
	    }
	    if (MechType(mech) == CLASS_MECH && !MadePilotSkillRoll(target, 2)) {
		mech_notify(target, MECHALL,
		    "Your piloting skill fails and you fall over!!");
		MechFalls(target, 1, 1);
	    }
	}
	if (target_charge && targ_roll >= targ_baseToHit) {
	    /* OUCH */
	    mech_notify(mech, MECHALL,
		tprintf("CRASH!!!\n%s charges into you!",
		    GetMechToMechID(mech, target)));
	    mech_notify(target, MECHALL,
		"SMASH!!! You crash into your target!");
	    hitGroup = FindAreaHitGroup(target, mech);
	    isrear = (hitGroup == BACK);
            inflicted_damage = mech_damage;
	    spread = mech_damage / 5;
	    for (i = 0; i < spread; i++) {
		hitloc =
		    FindHitLocation(mech, hitGroup, &iscritical, &isrear, target);
		MyDamageMech(mech, target, 1, MechPilot(target), hitloc,
		    isrear, iscritical, 5, 0, 0);
	    }
	    if (mech_damage % 5) {
		hitloc =
		    FindHitLocation(mech, hitGroup, &iscritical, &isrear, target);
		MyDamageMech(mech, target, 1, MechPilot(target), hitloc,
		    isrear, iscritical, (mech_damage % 5), 0, 0);
	    }
	    hitGroup = FindAreaHitGroup(mech, target);
	    isrear = (hitGroup == BACK);
        if (mudconf.btech_newcharge && mudconf.btech_tl3_charge)
            target_damage =
                (((((float) MechChargeDistance(target)) * MP1) -
                MechSpeed(mech) * 
                cos((MechFacing(target) - MechFacing(mech)) * (M_PI/180.))) *
                MP_PER_KPH) *
                (MechRTons(mech) / 1024 + 5) / 20; 
        else 
	        target_damage = (MechRTons(mech) / 1024 + 5) / 10;
            
            received_damage = target_damage; 
	    spread = target_damage / 5;
	    for (i = 0; i < spread; i++) {
		hitloc =
		    FindHitLocation(target, hitGroup, &iscritical,
		    &isrear, mech);
		MyDamageMech2(target, target, 0, -1, hitloc, isrear,
		    iscritical, 5, 0, 0);
	    }
	    if (mech_damage % 5) {
		hitloc =
		    FindHitLocation(target, hitGroup, &iscritical,
		    &isrear, mech);
		MyDamageMech2(target, target, 0, -1, hitloc, isrear,
		    iscritical, (mech_damage % 5), 0, 0);
	    }
	    MechSpeed(target) = 0;	/* stop him and let him accelerate again */
            MechDesiredSpeed(target) = 0;

            snprintf(emit_buff, LBUF_SIZE, "#%i charges #%i (%i/%i) Distance:"
               " %.2f DI: %i DR: %i", target->mynum, mech->mynum, targ_baseToHit,
               targ_roll, MechChargeDistance(target), inflicted_damage, 
               received_damage);
            SendDebug(emit_buff);

	    if (MechType(mech) == CLASS_MECH && !MadePilotSkillRoll(mech, 2)) {
		mech_notify(mech, MECHALL,
		    "Your piloting skill fails and you fall over!!");
		MechFalls(mech, 1, 1);
	    }
	    if (MechType(target) == CLASS_MECH && !MadePilotSkillRoll(target, 2)) {
		mech_notify(target, MECHALL,
		    "Your piloting skill fails and you fall over!!");
		MechFalls(target, 1, 1);
	    }
	}
	for (i = 0; i < CHARGE_SECTIONS; i++) {
	    SetRecycleLimb(mech, resect[i], PHYSICAL_RECYCLE_TIME);
	    SetRecycleLimb(target, resect[i], PHYSICAL_RECYCLE_TIME);
	}
	MechChargeTarget(target) = -1;	/* MechChargeTarget(mech) reset after return */
	MechChargeTimer(target) = 0;
	MechChargeDistance(target) = 0;
	return;
    }

    /* Check to see if any weapons cycling in any of the sections */
    for (i = 0; i < CHARGE_SECTIONS; i++) {
        if (SectHasBusyWeap(mech, i)) {
            ArmorStringFromIndex(i, location, MechType(mech),
                MechMove(mech));
            mech_notify(mech, MECHALL,
                tprintf("You have weapons recycling on your %s.",
                    location));
            return;
        }
    }

    /* Check if they going fast enough to charge */
    DOCHECKMA(MechSpeed(mech) < MP1,
        "You aren't moving fast enough to charge.");

    /* Check to see if their sections cycling */
    if (MechType(mech) == CLASS_MECH) {
        DOCHECKMA(MechSections(mech)[LLEG].recycle ||
            MechSections(mech)[RLEG].recycle,
            "Your legs are still recovering from your last attack.");
        DOCHECKMA(MechSections(mech)[RARM].recycle ||
            MechSections(mech)[LARM].recycle,
            "Your arms are still recovering from your last attack.");
    } else {
        DOCHECKMA(MechSections(mech)[FSIDE].recycle,
            "You are still recovering from your last attack!");
    }

    /* See if either the target or the attacker are jumping */
    DOCHECKMA(Jumping(target),
        "Your target is jumping, you charge underneath it.");
    DOCHECKMA(Jumping(mech),
        "You can't charge while jumping, try death from above.");

    /* If target is fallen make sure you in a tank */
    DOCHECKMA(Fallen(target) &&
        (MechType(mech) != CLASS_VEH_GROUND),
        "Your target's too low for you to charge it!");

    /* Only mechs can charge mechs */
    DOCHECKMA((MechType(mech) == CLASS_MECH) && 
        (MechType(target) != CLASS_MECH),
        "You can only charge mechs!");

    /* Only tanks can charge tanks and mechs */
    DOCHECKMA((MechType(mech) == CLASS_VEH_GROUND) &&
        ((MechType(target) != CLASS_MECH) &&
        (MechType(target) != CLASS_VEH_GROUND)),
        "You can only charge mechs and tanks!");

    ts = MechStatus(mech) & (TORSO_LEFT | TORSO_RIGHT);
    MechStatus(mech) &= ~ts;
    iwa = InWeaponArc(mech, MechFX(target), MechFY(target));
    MechStatus(mech) |= ts;
    DOCHECKMA(!(iwa & FORWARDARC),
	"Your charge target is not in your forward arc and you are unable to charge it.");

    if (mudconf.btech_newcharge)
	target_damage = (((((float)
			MechChargeDistance(mech)) * MP1) -
		MechSpeed(target) * cos((MechFacing(mech) -
			MechFacing(target)) * (M_PI / 180.))) *
	    MP_PER_KPH) * (MechRTons(mech) / 1024 + 5) / 10 + 1;
    else
	target_damage =
	    ((MechSpeed(mech) - MechSpeed(target) * cos((MechFacing(mech) -
			MechFacing(target)) * (M_PI / 180.))) *
	    MP_PER_KPH) * (MechRTons(mech) / 1024 + 5) / 10 + 1;


    DOCHECKMP(target_damage <= 0,
	"Your target pulls away from you and you are unable to charge it.");
    baseToHit += FindPilotPiloting(mech) - FindSPilotPiloting(target);
    baseToHit += (HasBoolAdvantage(MechPilot(mech), "melee_specialist") ? MIN(0, AttackMovementMods(mech) - 1) : AttackMovementMods(mech));
    baseToHit += TargetMovementMods(mech, target, 0.0);
    DOCHECKMA(baseToHit > 12,
	tprintf("Charge: BTH %d\tYou choose not to charge.", baseToHit));
    roll = Roll();
    mech_notify(mech, MECHALL, tprintf("Charge: BTH %d\tRoll: %d",
	    baseToHit, roll));
    if (roll >= baseToHit) {
	/* OUCH */
	MechLOSBroadcasti(mech, target, tprintf("%ss %%s!",
		MechType(mech) == CLASS_MECH ? "charge" : "ram"));
	mech_notify(target, MECHSTARTED,
	    tprintf("CRASH!!!\n%s %ss into you!", GetMechToMechID(target,
		    mech),
		MechType(mech) == CLASS_MECH ? "charge" : "ram"));
	mech_notify(mech, MECHALL, "SMASH!!! You crash into your target!");
	hitGroup = FindAreaHitGroup(mech, target);
	if (hitGroup == BACK)
	    isrear = 1;
	else
	    isrear = 0;

        inflicted_damage = target_damage;
	spread = target_damage / 5;
	for (i = 0; i < spread; i++) {
	    hitloc =
		FindHitLocation(target, hitGroup, &iscritical, &isrear, mech);
	    MyDamageMech(target, mech, 1, MechPilot(mech), hitloc, isrear,
		iscritical, 5, 0, 0);
	}
	if (target_damage % 5) {
	    hitloc =
		FindHitLocation(target, hitGroup, &iscritical, &isrear, mech);
	    MyDamageMech(target, mech, 1, MechPilot(mech), hitloc, isrear,
		iscritical, (target_damage % 5), 0, 0);
	}
	hitGroup = FindAreaHitGroup(target, mech);
	isrear = (hitGroup == BACK);
    if (mudconf.btech_newcharge && mudconf.btech_tl3_charge)
        mech_damage =
            (((((float) MechChargeDistance(mech)) * MP1) -
            MechSpeed(target) * 
            cos((MechFacing(mech) - MechFacing(target)) * (M_PI/180.))) *
            MP_PER_KPH) *
            (MechRTons(target) / 1024 + 5) / 20; 
    else
	    mech_damage = (MechRTons(target) / 1024 + 5) / 10;

        received_damage = mech_damage;
	spread = mech_damage / 5;
	for (i = 0; i < spread; i++) {
	    hitloc = FindHitLocation(mech, hitGroup, &iscritical, &isrear, target);
	    MyDamageMech2(mech, mech, 0, -1, hitloc, isrear, iscritical, 5,
		0, 0);
	}
	if (mech_damage % 5) {
	    hitloc = FindHitLocation(mech, hitGroup, &iscritical, &isrear, target);
	    MyDamageMech2(mech, mech, 0, -1, hitloc, isrear, iscritical,
		(mech_damage % 5), 0, 0);
	}
	if (MechType(mech) == CLASS_MECH && !MadePilotSkillRoll(mech, 2)) {
	    mech_notify(mech, MECHALL,
		"Your piloting skill fails and you fall over!!");
	    MechFalls(mech, 1, 1);
	}
	if (MechType(target) == CLASS_MECH && !MadePilotSkillRoll(target, 2)) {
	    mech_notify(target, MECHSTARTED,
		"Your piloting skill fails and you fall over!!");
	    MechFalls(target, 1, 1);
	}
	MechSpeed(mech) = 0;	/* stop him and let him accelerate again */
        MechDesiredSpeed(mech) = 0;

            snprintf(emit_buff, LBUF_SIZE, "#%i charges #%i (%i/%i) Distance:"
               " %.2f DI: %i DR: %i", mech->mynum, target->mynum, baseToHit,
               roll, MechChargeDistance(mech), inflicted_damage, 
               received_damage);
            SendDebug(emit_buff);
    }
    if (MechType(mech) == CLASS_MECH) {
	for (i = 0; i < CHARGE_SECTIONS; i++)
	    SetRecycleLimb(mech, resect[i], PHYSICAL_RECYCLE_TIME);
    } else {
	SetRecycleLimb(mech, FSIDE, PHYSICAL_RECYCLE_TIME);
	SetRecycleLimb(mech, TURRET, PHYSICAL_RECYCLE_TIME);
    }
    return;
}
