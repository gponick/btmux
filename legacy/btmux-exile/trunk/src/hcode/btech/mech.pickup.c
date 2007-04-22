#include <math.h>

#include "mech.h"
#include "mech.events.h"
#include "p.mech.utils.h"
#include "p.mech.los.h"
#include "p.eject.h"
#include "p.mech.pickup.h"
#include "p.mech.update.h"
#include "p.crit.h"

void mech_pickup(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *target;
    dbref target_num;
    MAP *newmap;
    int argc;
    char *args[4];

    if (player != GOD)
	cch(MECH_USUAL);
    argc = mech_parseattributes(buffer, args, 1);
    DOCHECK(MoveModeLock(mech),
	"You cannot tow currently in this movement mode!");
    DOCHECK(argc != 1, "Invalid number of arguements.");
    target_num = FindTargetDBREFFromMapNumber(mech, args[0]);
    DOCHECK(target_num == -1, "That target is not in your line of sight.");
    target = getMech(target_num);
    DOCHECK(!target ||
	!InLineOfSight(mech, target, MechX(target), MechY(target),
FaMechRange(mech, target)), "That target is not in your line of sight."); 
    DOCHECK(MechSpecials2(target) & CARRIER_TECH && !MechSpecials2(mech) & CARRIER_TECH,
	"You cannot handle the mass on that carrier.");
    DOCHECK(MechType(target) != CLASS_MW && is_aero(mech) && !(MechSpecials(mech) & SALVAGE_TECH), "You have no hitch on this aerospace craft!"); 
/*    DOCHECK(MechMove(mech) == MOVE_HOVER && !(MechSpecials(mech) & SALVAGE_TECH);
	"You have no hitch on this vehicle!"); */
    DOCHECK(Jumping(mech), "You can't pickup while jumping!");
    DOCHECK(Fallen(mech) || Standing(mech), "You are in no position to pick anything up!");
    DOCHECK(MechType(mech) == CLASS_BSUIT && !(MechSpecials(mech) & SALVAGE_TECH),
	"Uh huh. Not today.");
    DOCHECK(Jumping(target) || (MechType(target) == CLASS_VTOL && !Landed(target)), "With your lasso?");
    DOCHECK(MechZ(mech) > (MechZ(target) + 3),
	"You are too high above the target.");
    DOCHECK(MechZ(mech) < MechZ(target),
	"You are to far underneath the target.");
    DOCHECK(MechX(mech) != MechX(target) ||
	MechY(mech) != MechY(target), "You need to be in the same hex!");
    DOCHECK(Towed(target),
	"That target's already being towed by someone!");
    DOCHECK((MechStall(target) > 0), "That target is in a repair stall!");
    DOCHECK(MechSwarmTarget(target) == mech->mynum,
	"You can't grab hold!");
   DOCHECK(((MechMove(mech) == MOVE_QUAD && !(MechType(target) == CLASS_MW)) && !(MechSpecials(mech) & SALVAGE_TECH)),
         "No arms, only legs...");
   DOCHECK(MechTons(mech) < 5,
         "Uh huh, not today..");
   DOCHECK((!In_Character(mech->mynum)) && (MechType(target)==CLASS_MW),
         "Uh huh, not today..");
   DOCHECK(MechCritStatus(target) & HIDDEN, "You cannot pickup hiding targets....");
    if (MechType(target) == CLASS_MW) {
	pickup_mw(mech, target);
	return;
    } else {
	if (MechType(mech) == CLASS_MECH) {
	    DOCHECKMA((SectIsDestroyed(mech, LARM) && SectIsDestroyed(mech,RARM)),
		"Both arms are destroyed, you can't pick up anything.");
/*	    DOCHECKMA(SectIsDestroyed(mech, RARM),
		"Your right arm is destroyed, you can't pick up anything."); */
	    DOCHECKMA(!(OkayCritSectS(RARM, 3, HAND_OR_FOOT_ACTUATOR) &&
		    OkayCritSectS(RARM, 0, SHOULDER_OR_HIP)) &&
		!(OkayCritSectS(LARM, 3, HAND_OR_FOOT_ACTUATOR) &&
		    OkayCritSectS(LARM, 0, SHOULDER_OR_HIP)),
		"You need functioning arm and hands to pick things up!");
	} else if (MechType(mech) == CLASS_VTOL || MechMove(mech) == MOVE_HOVER) { 
	    DOCHECK(!(MechSpecials(mech) & SALVAGE_TECH && MechType(target) != CLASS_MW),
		"You can't pick that up in this MECH/VEHICLE");
	}
    }
    DOCHECK((MechCritStatus2(mech) & HITCH_DESTROYED), "Your hitch is destroyed!"); 
    DOCHECK(MechCarrying(mech) > 0, "You are already carrying a Mech");
    DOCHECK(((fabs(MechSpeed(mech)) > 1.0) ||
	    (fabs((float) MechVerticalSpeed(mech)) > 1.0)),
	"You are moving too fast to attempt a pickup.");
    DOCHECK(IsDS(target) && !Destroyed(target), "You can't pick that up!");
    DOCHECK(MechMove(target) == MOVE_NONE, "That's simply immobile!");
    DOCHECK(MechTeam(mech) != MechTeam(target) &&
	Started(target), "You can't pick that up!");
    DOCHECK(Moving(target), "You can't pick up a moving target!");
/*    DOCHECK(MechType(target) == CLASS_BSUIT, "There's just too many to pickup."); */
    mech_notify(target, MECHALL,
	tprintf("%s attaches his tow lines to you.",
	    GetMechToMechID(target, mech)));
    mech_notify(mech, MECHALL, tprintf("You attach your tow lines to %s.",
	    GetMechToMechID(mech, target)));
    if (MechCarrying(target) > 0)
	mech_dropoff(GOD, target, "");
    if ((newmap = getMap(target->mapindex)))
	MechLOSBroadcasti(mech, target, "picks up %s!");
    SetCarrying(mech, target->mynum);
    MechSwarmTarget(target) = -1;
    if (MechType(target) == CLASS_MECH) {
	MechStatus(target) |= FALLEN;
	FallCentersTorso(target);
	StopStand(target);
    }
    MechStatus(target) |= TOWED;
    if (!Destroyed(target))
	Shutdown(target);
    /* Adjust the speed involved */
    correct_speed(mech);
/* Prevent Tow Abuse */
 if (MMaxSpeed(mech) <= 1) 
    {
    if (MechCarrying(mech) > 0)
	{
	mech_notify(mech, MECHALL, "You can't move it, so you best lose it."); 
	mech_dropoff(MechPilot(mech), mech, "");
     /* correct_speed(mech); */
	}
    }
  
 SendDebug(tprintf("#%d has picked up #%d", mech->mynum, target->mynum));

}

void mech_dropoff(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    MECH *target;
    MAP *newmap;
    dbref aRef;
    int x, y;

    if (player != GOD)
	cch(MECH_USUAL);
    DOCHECK(MechCarrying(mech) <= 0, "You aren't carrying a mech!");
    aRef = MechCarrying(mech);
    SetCarrying(mech, -1);
    target = getMech(aRef);
    DOCHECK(!target, "You were towing invalid target!");
    MechStatus(target) &= ~TOWED;	/* Reset the Towed flag */
    mech_notify(mech, MECHALL, "You drop the mech you were carrying.");
    mech_notify(target, MECHALL, "You have been released from towing.");
    if ((newmap = getMap(target->mapindex))) {
	MechLOSBroadcasti(mech, target, "drops %s!");
	if (FlyingT(mech) && ((x = MechZ(target)) > (y =
		    Elevation(newmap, MechX(target), MechY(target)))) && !Landed(mech)) {
	    mech_notify(mech, MECHALL,
		"Maybe you should have done this closer to the ground.");
	    mech_notify(target, MECHALL,
		"You wish he had done that a might bit closer to the ground.");
	    MechLOSBroadcast(target, "falls through the sky.");
	    MECHEVENT(target, EVENT_FALL, mech_fall_event, FALL_TICK, -1);
	}
    }
    MechSpeed(target) = 0;
    MechDesiredSpeed(target) = 0;
    correct_speed(mech);
    TowDump(target);
    SendDebug(tprintf("#%d has dropped off #%d", mech->mynum, target->mynum));
}
