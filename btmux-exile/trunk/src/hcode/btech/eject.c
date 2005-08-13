/* Ejection code */
#include <math.h>
#include "mech.h"
#include "mech.events.h"
#include "p.btechstats.h"
#include "p.mechrep.h"
#include "p.mech.restrict.h"
#include "p.mech.update.h"
#include "p.bsuit.h"
#include "autopilot.h"
#include "p.mech.combat.h"
#include "p.mech.utils.h"
#include "p.btechstats.h"
#include "p.econ_cmds.h"
#include "p.mech.los.h"
#include "p.mech.ood.h"
#include "p.mech.tech.h"
#include "p.bsuit.h"
#include "p.mech.tech.commands.h"
#include "p.autopilot_commands.h"

int tele_contents(dbref from, dbref to, int flag, MECH * mech)
{
    dbref i, tmpnext;
    int count = 0;
    int loss;
    SAFE_DOLIST(i, tmpnext, Contents(from)) {
	if (!isPlayer(i))
	    continue;
	if ((flag & TELE_ALL) || !Wiz(i)) {
	if ((flag & TELE_SLAVE) && !Wiz(i)) {
	    s_Slave(i);
/*	    silly_atr_set(i, A_LOCK, ""); */
	}
        if (MechType(mech) == CLASS_BSUIT)
	    loss = mudconf.btech_xploss_bsuit;
        else if (MechType(mech) == CLASS_MW)
	    loss = mudconf.btech_xploss_executed;
        else if (MechTons(mech) >= 80)
	    loss = mudconf.btech_xploss_assault;
	else if (MechTons(mech) > 55)
	    loss = mudconf.btech_xploss_heavy;
	else if (MechTons(mech) > 35)
	    loss = mudconf.btech_xploss_medium;
	else
	    loss = mudconf.btech_xploss_light;

/* Absolete code.  _tankmod has now _bsuit */
/*	if (MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VEH_NAVAL ||
	    MechType(mech) == CLASS_VTOL || MechType(mech) == CLASS_BSUIT)
	    loss = (loss + mudconf.btech_xploss_tankmod); */

	if (flag & TELE_XP && !Wiz(i)) {
	    SendDebug(tprintf("%s (#%d) has been sent to real death and lost %d%% XP.", Name(i), i, (1000 - loss) / 10));
	    lower_xp(i, loss);
  	    }
	if (flag & TELE_LOUD)
	    loud_teleport(i, to);
	else
	    hush_teleport(i, to);
	count++;
    }
    }
    return count;
}

/* Delayed blast event, for various reasons */
static void mech_discard_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    AUTO *a;
    dbref i = mech->mynum;

    if (MechAuto(mech) > 0 && (a = FindObjectsData(MechAuto(mech))) != NULL) {
        StopAutoPilot(a);
        auto_disengage(a->mynum, a, "-1");
        auto_delcommand(a->mynum, a, "-1");
        } else {
	    MechAuto(mech) = -1;
	    }

    c_Hardcode(i);
    handle_xcode(GOD, i, 1, 0);
    s_Going(i);
    s_Dark(i);
    s_Zombie(i);
    hush_teleport(i, USED_MW_STORE);
}
                                                                                                                                                                                      
void discard_mw(MECH * mech)
{
    if (In_Character(mech->mynum))
        MECHEVENT(mech, EVENT_DISCARDMECH, mech_discard_event, 20, 0);
}

void enter_mw_bay(MECH * mech, dbref bay)
{
    tele_contents(mech->mynum, bay, TELE_LOUD | TELE_ALL, mech);	/* Even immortals must get going */
    discard_mw(mech);
}

void pickup_mw(MECH * mech, MECH * target)
{
    dbref mw;

    mw = Contents(target->mynum);
    DOCHECKMA((MechType(mech) != CLASS_MECH) &&
	(MechType(mech) != CLASS_VEH_GROUND) &&
	(MechType(mech) != CLASS_VEH_VTOL) &&
	!(MechSpecials(mech) & SALVAGE_TECH),
	"You can't pick up, period.") if (mw > 0)
	notify(mw,
	    tprintf("%s scoops you up and brings you into the cockpit.",
		GetMechToMechID(target, mech)));
    /* Put the player in the picker uppper and clear him from the map */
    MechLOSBroadcast(mech, tprintf("picks up %s.", GetMechID(target)));
    mech_notify(mech, MECHALL,
	tprintf("You pick up the stray mechwarrior from the field."));
    if (MechTeam(target) != MechTeam(mech))
	tele_contents(target->mynum, mech->mynum, TELE_LOUD | TELE_ALL | TELE_SLAVE, target);
    else
	tele_contents(target->mynum, mech->mynum, TELE_LOUD | TELE_ALL, target);
    discard_mw(target);
}

static void char_eject(dbref player, MECH * mech)
{
    MECH *m;
    dbref suit;
    char *d;

    suit = create_object(tprintf("MechWarrior - %s", Name(player)));
    silly_atr_set(suit, A_XTYPE, "MECH");
    s_Hardcode(suit);
    s_Inherit(suit);
    handle_xcode(GOD, suit, 0, 1);
    d = silly_atr_get(player, A_MWTEMPLATE);
    if (!(m = getMech(suit))) {
	SendError(tprintf
	    ("Unable to create special obj for #%d's ejection.", player));
	destroy_object(suit);
	notify(player,
	    "Sorry, something serious went wrong, contact a Wizard (can't create RS object)");
	return;
    }
    if (!mech_loadnew(GOD, m, (!d || !*d ||
		!strcmp(d, "#-1")) ? "MechWarrior" : d)) {
	SendError(tprintf
	    ("Unable to load mechwarrior template for #%d's ejection. (%s)",
		player, (!d || !*d) ? "Default template" : d));
	destroy_object(suit);
	notify(player,
	    "Sorry, something serious went wrong, contact a Wizard (can't load MWTemplate)");
	return;
    }
    silly_atr_set(suit, A_MECHNAME, "MechWarrior");
    MechTeam(m) = MechTeam(mech);
    if (mudconf.btech_mwparent > 0)
	s_Parent(suit, mudconf.btech_mwparent);
    did_it(GOD, suit, 0, NULL, 0, NULL, A_ACLONE, (char **) NULL, 0);
    mech_Rsetmapindex(GOD, (void *) m, tprintf("%d", mech->mapindex));
    mech_Rsetxy(GOD, (void *) m, tprintf("%d %d", MechX(mech),
	    MechY(mech)));
    mech_Rsetteam(GOD, (void *) m, tprintf("%d", MechTeam(mech)));
    hush_teleport(suit, mech->mapindex);
    hush_teleport(player, suit);
    MechLOSBroadcast(m, tprintf("ejected from %s!", GetMechID(mech)));
    s_In_Character(suit);
    initialize_pc(player, m);
    MechPilot(m) = player;
    MechTeam(m) = MechTeam(mech);
    notify(player, "You eject from the unit!");
    if (MechType(mech) == CLASS_MECH) {
	DestroyPart(mech, HEAD, 2);
    }
    if (!Destroyed(mech))
	DestroyAndDump(mech);
}

void mech_eject(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALS);
    DOCHECK(IsDS(mech), "Dropships do not support ejection.");
    DOCHECK((!((MechType(mech) == CLASS_MECH) ||
	    (MechType(mech) == CLASS_VEH_VTOL) ||
	    (MechType(mech) == CLASS_VEH_GROUND)) ||
	    MechSpecials2(mech) & TORSOCOCKPIT_TECH),
	"This unit has no ejection seat!");
    DOCHECK(FlyingT(mech) &&
	!Landed(mech),
	"Regrettably, right now you can only eject when landed, sorry - no parachute :P");
    DOCHECK(!In_Character(mech->mynum), "This unit isn't in character!");
    /* DOCHECK(!mudconf.btech_ic, "This MUX isn't in character!"); */
    DOCHECK(!In_Character(Location(mech->mynum)),
	"Your location isn't in character!");
    DOCHECK(Started(mech) &&
	MechPilot(mech) != player,
	"You aren't in da pilot's seat - no ejection for you!");
    if (!Started(mech)) {
	DOCHECK((char_lookupplayer(GOD, GOD, 0, silly_atr_get(mech->mynum,
			A_PILOTNUM))) != player,
	    "You aren't the official pilot of this thing. Try 'disembark'");
    }
    if (MechType(mech) == CLASS_MECH)
	DOCHECK(PartIsNonfunctional(mech, HEAD, 2),
	    "The parts of cockpit that control ejection are already used. Try 'disembark'");
    /* Ok.. time to eject ourselves */
    char_eject(player, mech);
}

static void char_disembark(dbref player, MECH * mech)
{
    MECH *m;
    dbref suit;
    char *d;
    MAP *mymap;
    int initial_speed;

    suit = create_object(tprintf("MechWarrior - %s", Name(player)));
    silly_atr_set(suit, A_XTYPE, "MECH");
    s_Hardcode(suit);
    s_Inherit(suit);
    handle_xcode(GOD, suit, 0, 1);
    d = silly_atr_get(player, A_MWTEMPLATE);
    if (!(m = getMech(suit))) {
	SendError(tprintf
	    ("Unable to create special obj for #%d's disembarkation.",
		player));
	destroy_object(suit);
	notify(player,
	    "Sorry, something serious went wrong, contact a Wizard (can't create RS object)");
	return;
    }
    if (!mech_loadnew(GOD, m, (!d || !*d ||
		!strcmp(d, "#-1")) ? "MechWarrior" : d)) {
	SendError(tprintf
	    ("Unable to load mechwarrior template for #%d's disembarkation. (%s)",
		player, (!d || !*d) ? "Default template" : d));
	destroy_object(suit);
	notify(player,
	    "Sorry, something serious went wrong, contact a Wizard (can't load MWTemplate)");
	return;
    }
    silly_atr_set(suit, A_MECHNAME, "MechWarrior");
    MechTeam(m) = MechTeam(mech);
    if (mudconf.btech_mwparent > 0)
	s_Parent(suit, mudconf.btech_mwparent);
    did_it(GOD, suit, 0, NULL, 0, NULL, A_ACLONE, (char **) NULL, 0);
    mech_Rsetmapindex(GOD, (void *) m, tprintf("%d", mech->mapindex));
    mech_Rsetxy(GOD, (void *) m, tprintf("%d %d", MechX(mech),
	    MechY(mech)));
    MechZ(m) = MechZ(mech);
    MechFZ(m) = MechZ(m) * ZSCALE;
    mech_Rsetteam(GOD, (void *) m, tprintf("%d", MechTeam(mech)));
    hush_teleport(suit, mech->mapindex);
    hush_teleport(player, suit);
    s_In_Character(suit);
    initialize_pc(player, m);
    MechPilot(m) = player;
    MechTeam(m) = MechTeam(mech);
    mymap = getMap(m->mapindex);
    if ((MechZ(m) > (Elevation(mymap, MechX(m), MechY(m)) + 1)) &&
	(MechZ(m) > 0)) {
	notify(player,
	    "You open the hatch and jump out with your trusty parachute....");
	MechLOSBroadcast(m, tprintf("parachutes out of %s.",
		GetMechID(mech)));
    	initiate_ood(player, mech, tprintf("%d %d %d", MechX(mech), MechY(mech), MechZ(mech)));
    } else {
	MechLOSBroadcast(m, tprintf("climbs out of %s!", GetMechID(mech)));
	notify(player, "You climb out of the unit.");
    }

}


void mech_disembark(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALS);
    DOCHECK(!((MechType(mech) == CLASS_MECH) ||
	    (MechType(mech) == CLASS_VEH_VTOL) ||
	    (MechType(mech) == CLASS_VEH_GROUND) ||
	    is_aero(mech) || IsDS(mech)),
	"The door ! The door ? The Door ?!? Where's the exit in this damned thing ?");

/*  DOCHECK(FlyingT(mech) && !Landed(mech), "What, in the air ? Are you suicidal ?"); */
    DOCHECK(!IsDS(mech) && !In_Character(mech->mynum), "This unit isn't in character!");
    DOCHECK(mudconf.btech_ic != 1, "This MUX isn't in character!");
    DOCHECK(!In_Character(Location(mech->mynum)),
	"Your location isn't in character!");
    DOCHECK(Started(mech) && (MechPilot(mech) == player) &&
	(MechType(mech) == CLASS_MECH),
	"With you still tied up to this Neurohelmet ? I dont think so.");
    DOCHECK(fabs(MechSpeed(mech)) > 25.,
	"Are you suicidal ? That thing is moving too fast !");
    /* Ok.. time to disembark ourselves */
    char_disembark(player, mech);
}

void mech_udisembark(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *target;
    int newmech, n;
    MAP *mymap;
/*    int initial_speed; */
    int i;


/*    DOCHECK(MechType(mech) != CLASS_BSUIT, "This is not a battlesuit!"); */
    DOCHECK(In_Character(mech->mynum) && !Wiz(player) &&
	(char_lookupplayer(GOD, GOD, 0, silly_atr_get(mech->mynum,
		    A_PILOTNUM)) != player), "This isn't your mech!");

    newmech = Location(mech->mynum);
    DOCHECK(!(Good_obj(newmech) &&
	    Hardcode(newmech)), "You're not being carried!");
    DOCHECK(!(target = getMech(newmech)), "Not being carried!");
    DOCHECK(target->mapindex == -1, "You are not on a map.");
    n = figure_latest_tech_event(mech);
    DOCHECK(n,
        "This 'Mech is still under repairs (see checkstatus for more info)");
    DOCHECK(abs(MechSpeed(target)) > 0,
	"You cannot leave while the carrier is moving!");
/*
    DOCHECK(MechCritStatus(target) & HIDDEN,
	"You cannot leave while the carrier is hidden!");
*/
    mech_Rsetmapindex(GOD, (void *) mech, tprintf("%d",
	    (int) target->mapindex));
    mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d", MechX(target),
	    MechY(target)));
    MechZ(mech) = MechZ(target);
    MechFZ(mech) = ZSCALE * MechZ(mech);
    mymap = getMap(mech->mapindex);
    DOCHECK(!mymap, "Major map error possible. Prolly should contact a wizard.");
    loud_teleport(mech->mynum, mech->mapindex);
/*    if (!Destroyed(mech) && char_lookupplayer(GOD, GOD, 0, silly_atr_get(mech->mynum, A_PILOTNUM)) != NOTHING) */
    if (!Destroyed(mech) && Location(player) == mech->mynum) {
	MechPilot(mech) = player;
	Startup(mech);
	}
    MarkForLOSUpdate(mech);
    MarkForLOSUpdate(target);
    SetCargoWeight(mech);
    UnSetMechPKiller(mech);
    MechLOSBroadcast(mech, "powers up!");
    EvalBit(MechSpecials(mech), SS_ABILITY, ((MechPilot(mech) > 0 &&
		isPlayer(MechPilot(mech))) ? char_getvalue(MechPilot(mech),
		"Sixth_Sense") : 0));
    MechComm(mech) = DEFAULT_COMM;
    if (isPlayer(MechPilot(mech)) && !(db[mech->mynum].flags & QUIET)) {
	MechComm(mech) =
	    char_getskilltarget(MechPilot(mech), "Comm-Conventional", 0);
	MechPer(mech) =
	    char_getskilltarget(MechPilot(mech), "Perception", 0);
    } else {
	MechComm(mech) = 6;
	MechPer(mech) = 6;
    }
    MechCommLast(mech) = 0;
    UnZombifyMech(mech);
    CargoSpace(target) += (MechTons(mech) * 100);
    MarkForLOSUpdate(target);

    if (MechCritStatus(target) & HIDDEN) {
	MechCritStatus(target) &= ~HIDDEN;
	MechLOSBroadcast(target, "becomes visible as it is disembarked from.");
	}

    if (!FlyingT(mech) && (MechZ(mech) > Elevation(mymap, MechX(mech), MechY(mech))) && (MechZ(mech) > 0)) {
	notify(player,
	    "You open the hatch and drop out of the unit....");
	MechLOSBroadcast(mech, tprintf("drops out of %s and begins falling to the ground.",
		GetMechID(target)));
/*	initial_speed =
	    ((MechSpeed(target) + MechVerticalSpeed(target)) / MP1) / 2 +
	    4;
	MECHEVENT(mech, EVENT_FALL, mech_fall_event, FALL_TICK,
	    -initial_speed);  -- No no no, we're already OODing -- Reb */
/*        MechCocoon(mech) = MechRTons(mech) / 5 / 1024 + 1; */
    	initiate_ood(player, mech, tprintf("%d %d %d", MechX(mech), MechY(mech), MechZ(mech)));
    } else {
        if (MechType(mech) == CLASS_BSUIT) {
	    MechLOSBroadcast(mech, tprintf("climbs out of %s!",
	    	GetMechID(target)));
	    notify(player, "You climb out of the unit.");
	} else {
            if (Destroyed(target) || !Started(target)) {
	        MechLOSBroadcast(mech, tprintf("smashes open the ramp door and emerges from %s!",
	    	    GetMechID(target)));
	        notify(player, "You smash open the door and break out.");
                    MechFalls(mech, 4, 0);
                    if (MiscEventing(mech))
                        StopMiscEvent(mech);
		} else {
	        MechLOSBroadcast(mech, tprintf("emerges from the ramp out of %s!",
	    	    GetMechID(target)));
	        notify(player, "You emerge from the unit loading ramp.");
		if (Landed(mech) && MechZ(mech) > Elevation(mymap, MechX(mech), MechY(mech)) && FlyingT(mech))
		    MechStatus(mech) &= ~LANDED;
		}
	}
    if (MechSpeed(target) > 0) {
		int bth = TargetMovementMods(mech, target, 0) - 1;

		if (MechStatus2(target) & SPRINTING)
			bth += 2;
		if (MechStatus2(target) & EVADING)
			bth += 2;
		if (MechType(target) == CLASS_VTOL)
			bth++;

		if (!MadePilotSkillRoll(mech, bth)) {
			mech_notify(mech, MECHALL, "You hit the ground hard!!");
			MechFalls(mech, bth * 2 / 3, 0);
		} else
			mech_notify(mech, MECHALL, "You hit the ground moving!");
	}
	}
	if (MechType(mech) == CLASS_BSUIT)
		bsuit_start_recycling(mech);
	else if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW) {
		for (i = 0; i < NUM_SECTIONS; i++)
			SetRecycleLimb(mech, i, PHYSICAL_RECYCLE_TIME);
	} else if (MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VTOL) {
		for (i = 0; i < NUM_SECTIONS; i++)
			if (i == ROTOR)
			    continue;
			else
			    SetRecycleLimb(mech, i, PHYSICAL_RECYCLE_TIME);
	}
	fix_pilotdamage(mech, MechPilot(mech));
	correct_speed(target);
}



void mech_embark(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *target, *towee = NULL;
    dbref target_num;
    MAP *newmap;
    int argc, i;
    char *args[4];

    if (player != GOD)
	cch(MECH_USUAL);
/*    DOCHECK((MechType(mech) != CLASS_MW) && (MechType(mech) != CLASS_BSUIT),
	"... Why not just TELL it you love it?"); */
/*    DOCHECK(MechCarrying(mech) > 0, "You cannot embark while towing a unit."); */
    if (MechType(mech) == CLASS_MW) {
	argc = mech_parseattributes(buffer, args, 1);
	DOCHECK(argc != 1, "Invalid number of arguements.");
	target_num = FindTargetDBREFFromMapNumber(mech, args[0]);
	DOCHECK(target_num == -1,
	    "That target is not in your line of sight.");
	target = getMech(target_num);
	DOCHECK(!target ||
	    !InLineOfSight(mech, target, MechX(target), MechY(target),
		FaMechRange(mech, target)),
	    "That target is not in your line of sight.");
	DOCHECK(MechZ(mech) > (MechZ(target) + 1),
	    "You are too high above the target.");
	DOCHECK(MechZ(mech) < (MechZ(target) - 1),
	    "You can't reach that high !");
	DOCHECK(MechX(mech) != MechX(target) ||
	    MechY(mech) != MechY(target),
	    "You need to be in the same hex!");
	DOCHECK((!In_Character(mech->mynum)) || (!In_Character(target->mynum)),
	    "You don't really see a way to get in there.");
	DOCHECK(MechType(target) == CLASS_MECH && SectIsDestroyed(target,
		(MechSpecials2(target) & TORSOCOCKPIT_TECH ? CTORSO : HEAD)),
		"The hatch seems to be.... not there anymore.");
	DOCHECK((MechType(target) == CLASS_VEH_GROUND || MechType(target) == CLASS_VTOL) &&
		!unit_is_fixable(target), "You can't find and entrance amid the mass of twisted metal.");
	DOCHECK(!can_pass_lock(mech->mynum, target->mynum, A_LENTER), "Locked. Damn !");
	DOCHECK(fabs(MechSpeed(target)) > 15.,
	    "Are you suicidal ? That thing is moving too fast !");
	mech_notify(mech, MECHALL, tprintf("You climb into %s.",
		GetMechID(target)));
	MechLOSBroadcast(mech, tprintf("climbs into %s.",
		GetMechID(target)));
	tele_contents(mech->mynum, target->mynum, TELE_LOUD | TELE_ALL, mech);
	discard_mw(mech);
	return;
    }
    /* What heppens with a Bsuit squad? Or even bigger more painfull ones! */
    /* Check if the vechile has cargo capacity, or is an Omni Mech */
    argc = mech_parseattributes(buffer, args, 1);
    DOCHECK(argc != 1, "Invalid number of arguements.");
    target_num = FindTargetDBREFFromMapNumber(mech, args[0]);
    DOCHECK(target_num == -1, "That target is not in your line of sight.");
    target = getMech(target_num);
    DOCHECK(!target ||
	!InLineOfSight(mech, target, MechX(target), MechY(target),
FaMechRange(mech, target)), "That target is not in your line of sight.");
/*    DOCHECK(!Started(target), "That unit isn't even started.  Makes opening the doors difficult."); */
/*    DOCHECK(MechType(mech) != CLASS_MW && Destroyed(target), "That target is destroyed.  Makes opening the doors difficult."); */
    DOCHECK(MechCarrying(mech) == target_num, "You cannot embark what your towing!");
    DOCHECK(Fallen(mech) || Standing(mech), "Help! I've fallen and I can't get up!");
    DOCHECK(!Started(mech) || Destroyed(mech), "Ha Ha Ha.");
    DOCHECK(MechZ(mech) > MechZ(target), "You are too high above the target.");
    DOCHECK(MechZ(mech) < MechZ(target), "You can't reach that high!");
    DOCHECK(Jumping(mech), "You cannot do that while jumping!");
    DOCHECK(Jumping(target), "You cannot do that while it is jumping!");
    DOCHECK(MechX(mech) != MechX(target) || MechY(mech) != MechY(target), "You need to be in the same hex!");
    DOCHECK(MechSpecials2(mech) & CARRIER_TECH && (IsDS(target) ? IsDS(mech) : 1), "You're a bit bulky to do that yourself.");
    DOCHECK(MechCritStatus(mech) & HIDDEN, "You cannot embark while hidden."); 
    DOCHECK((!In_Character(mech->mynum)) || (!In_Character(target->mynum)),
	"You don't really see a way to get in there.");
    DOCHECK(MechTons(mech) > CarMaxTon(target), "You are too large for that class of carrier.");
    DOCHECK(MechType(mech) != CLASS_BSUIT && !(MechSpecials2(target) & CARRIER_TECH),
		"This unit can't handle your mass.");
    for (i = 0; i < NUM_SECTIONS; i++) {
	DOCHECK(SectHasBusyWeap(mech, i), "You have weapon(s) recycling!");
	DOCHECK(MechSections(mech)[i].recycle, "You have limb(s) recycling!");
	}
    DOCHECK(MechTeam(mech) != MechTeam(target), "Locked. Damn !");
    DOCHECK(fabs(MechSpeed(target)) > 0,
	"Are you suicidal ? That thing is moving too fast !");
    DOCHECK(MMaxSpeed(mech) < MP1, "You are to overloaded to enter.");
    DOCHECK((MechTons(mech) * 100) > CargoSpace(target),
	    "Not enough cargospace for you!");
    if (MechCarrying(mech) > 0) {
	DOCHECK(!(towee = getMech(MechCarrying(mech))), "Internal error caused by towed unit! Contact a wizard!");
	DOCHECK(MechTons(towee) > CarMaxTon(target), "You're towed unit is  too large for that class of carrier.");
	DOCHECK(((MechTons(mech) + MechTons(towee)) * 100) > CargoSpace(target),
            "Not enough cargospace for you and your towed unit!");
	}
    if (MiscEventing(mech))
	StopMiscEvent(mech);
    newmap = getMap(mech->mapindex);
    if (MechType(mech) == CLASS_BSUIT) {
        mech_notify(mech, MECHALL, tprintf("You climb into %s.", GetMechID(target)));
        MechLOSBroadcast(mech, tprintf("climbs into %s.", GetMechID(target)));
    } else {
        mech_notify(mech, MECHALL, tprintf("You climb up the entry ramp into %s.", GetMechID(target)));
        MechLOSBroadcast(mech, tprintf("climbs up the entry ramp into %s.", GetMechID(target)));
	if (towee && MechCarrying(mech) > 0) {
        mech_notify(towee, MECHALL, tprintf("You are drug up the entry ramp into %s.", GetMechID(target)));
        MechLOSBroadcast(towee, tprintf("is drug up the entry ramp into %s.", GetMechID(target)));
	    }
    }

    MarkForLOSUpdate(mech);
    MarkForLOSUpdate(target);

    if (MechCritStatus(target) & HIDDEN) {
	MechCritStatus(target) &= ~HIDDEN;
	MechLOSBroadcast(target, "becomes visible as it is embarked into.");
	}

    mech_Rsetmapindex(GOD, (void *) mech, tprintf("%d", (int) -1));
    mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d", 0, 0));
//    remove_mech_from_map(newmap, mech);
    loud_teleport(mech->mynum, target->mynum);
    CargoSpace(target) -= (MechTons(mech) * 100);
    Shutdown(mech);
    if (towee && MechCarrying(mech) >0) {
	MarkForLOSUpdate(towee);
	mech_Rsetmapindex(GOD, (void *) towee, tprintf("%d", (int) -1));
	mech_Rsetxy(GOD, (void *) towee, tprintf("%d %d", 0, 0));
//	remove_mech_from_map(newmap, towee);
	loud_teleport(towee->mynum, target->mynum);
	CargoSpace(target) -= (MechTons(towee) * 100);
	Shutdown(towee);
	SetCarrying(mech, -1);
	MechStatus(towee) &= ~TOWED;
	}
    correct_speed(target);
}

void autoeject(dbref player, MECH * mech, MECH * attacker)
{
    MECH *m;
    dbref suit;
    char *d;

    cch(MECH_USUALS);
    DOCHECK(!isPlayer(MechPilot(mech)),
	"No autoejection for non-players.");
    DOCHECK(!In_Character(mech->mynum),
	"Unit not in character, so no autoeject needed!");
    if (mudconf.btech_ic != 1)
	return;
    DOCHECK(!In_Character(Location(mech->mynum)),
	"Your location isn't in character!");
    DOCHECK(MechSpecials2(mech) & TORSOCOCKPIT_TECH,
	"This unit has no ejection system!");
	DOCHECK(!GotPilot(mech), "Crap! None of you are the pilot!");
	suit = create_object(tprintf("MechWarrior - %s", Name(player)));
    silly_atr_set(suit, A_XTYPE, "MECH");
    s_Hardcode(suit);
    s_Inherit(suit);
    handle_xcode(GOD, suit, 0, 1);
    d = silly_atr_get(player, A_MWTEMPLATE);
    if (!(m = getMech(suit))) {
	SendError(tprintf("Unable to create special obj for #%d's ejection.", player));
	destroy_object(suit);
	notify(player, "Sorry, something serious went wrong, contact a Wizard (can't create RS object)");
	return;
    }
    if (!mech_loadnew(GOD, m, (!d || !*d || !strcmp(d, "#-1")) ? "MechWarrior" : d)) {
	SendError(tprintf("Unable to load mechwarrior template for #%d's ejection. (%s)", player, (!d || !*d) ? "Default template" : d));
	destroy_object(suit);
	notify(player, "Sorry, something serious went wrong, contact a Wizard (can't load MWTemplate)");
	return;
    }
    silly_atr_set(suit, A_MECHNAME, "MechWarrior");
    MechTeam(m) = MechTeam(mech);
    if (mudconf.btech_mwparent > 0)
	s_Parent(suit, mudconf.btech_mwparent);
    did_it(GOD, suit, 0, NULL, 0, NULL, A_ACLONE, (char **) NULL, 0);
    mech_Rsetmapindex(GOD, (void *) m, tprintf("%d", mech->mapindex));
    mech_Rsetxy(GOD, (void *) m, tprintf("%d %d", MechX(mech), MechY(mech)));
    mech_Rsetteam(GOD, (void *) m, tprintf("%d", MechTeam(mech)));
    hush_teleport(suit, mech->mapindex);
    hush_teleport(player, suit);
    MechLOSBroadcast(m, tprintf("ejected from %s!", GetMechID(mech)));
    s_In_Character(suit);
    MechCritStatus(m) &= ~(PC_INITIALIZED);
    MechType(m) = CLASS_MW;
    initialize_pc(player, m);
    MechPilot(m) = player;
    initiate_ood(player, m, tprintf("%d %d %d", MechX(m), MechY(m), 30));
    notify(player, "You eject from the unit!");
    if (MechType(mech) == CLASS_MECH) {
	DestroyPart(mech, HEAD, 2);
    }
    if (!Destroyed(mech)) {
	if (attacker)
	    DestroyMech(mech, attacker, 0);
	else
	    DestroyAndDump(mech);
	}
}
