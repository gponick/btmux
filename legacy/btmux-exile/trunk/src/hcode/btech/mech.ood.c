#include "mech.h"
#include "mech.events.h"
#include "p.btechstats.h"
#include "p.mech.utils.h"
#include "p.mech.update.h"
#include "p.mech.restrict.h"
#include "p.template.h"
#include "p.mech.combat.h"

void mech_ood_damage(MECH * wounded, MECH * attacker, int damage)
{
    mech_notify(attacker, MECHALL,
	tprintf("%%cgYou hit the cocoon for %d points of damage!%%cn",
	    damage));
    mech_notify(wounded, MECHALL,
	tprintf
	("%%ch%%cyYour cocoon has been hit for %d points of damage!%%cn",
     damage));
    MechCocoon(wounded) = MAX(0, MechCocoon(wounded) - damage);
    if (MechCocoon(wounded))
	return;
    /* Abort the OOD and initiate falling */
    if (MechZ(wounded) > MechElevation(wounded)) {
	if (MechJumpSpeed(wounded) >= MP1) {
	    mech_notify(wounded, MECHALL,
		"You initiate your jumpjets to compensate for the breached cocoon!");
	    MechCocoon(wounded) = -1;
	    return;
	}
	mech_notify(wounded, MECHALL,
	    "Your cocoon has been destroyed - have a nice fall!");
	MechLOSBroadcast(wounded,
	    "starts plummeting down, as the final blast blows the cocoon apart!");
	StopOOD(wounded);
	MECHEVENT(wounded, EVENT_FALL, mech_fall_event, FALL_TICK, -1);
    }
}

void mech_ood_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int mof = 0, roll, roll_needed, para = 0;

    if (!OODing(mech))
	return;
    MarkForLOSUpdate(mech);
    if ((MechsElevation(mech) - DropGetElevation(mech)) > OOD_SPEED) {
	MechZ(mech) -= OOD_SPEED;
	MechFZ(mech) = MechZ(mech) * ZSCALE;
	MECHEVENT(mech, EVENT_OOD, mech_ood_event, OOD_TICK, 0);
	return;
    }
    /* Time to hit da ground */
    mech_notify(mech, MECHALL, "You land on the ground!");

    if (Fallen(mech))
        mof = -10;
    if (Uncon(mech) || !Started(mech) || Blinded(mech) || MechAutofall(mech))
        mof = -20;
    roll = Roll();
    roll_needed = MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW ? FindPilotPiloting(mech) - 1 : FindSPilotPiloting(mech) + MechPilotSkillBase(mech);
 
    if (!Started(mech))
	roll_needed += 10;
    if (MechCocoon(mech) == 1) {
	para = 1;
    } else if (MechCocoon(mech) < 0) {
	roll_needed += 4;
    } else if (MechCocoon(mech) == 0) {
	roll_needed += 10;
    }

    if (MechRTerrain(mech) != GRASSLAND && MechRTerrain(mech) != ROAD) {
	if (MechRTerrain(mech) == WATER || MechRTerrain(mech) == HIGHWATER)
	    roll_needed += 2;
	else
	    roll_needed += 3;
	}

    MechCocoon(mech) = 0;

    if (In_Character(mech->mynum) && Location(MechPilot(mech)) != mech->mynum)
        roll_needed += 99;

    mech_notify(mech, MECHPILOT, "You make a piloting skill roll!");
    mech_notify(mech, MECHPILOT,
        tprintf("Modified Pilot Skill: BTH %d\tRoll: %d", roll_needed, roll)); 
    mof += (roll - roll_needed); 
    if (roll >= roll_needed) {
        if (roll_needed > 2)
            AccumulatePilXP(MechPilot(mech), mech, BOUNDED(1, (abs(mof) + 1) * 2, 20));
    }
    mof += (roll - roll_needed);

    if (mof < 0) {
	if (MechType(mech) == CLASS_MECH) {
	    mech_notify(mech, MECHALL,
		"You are unable to control your momentum and fall on your face!");
	    MechLOSBroadcast(mech,
		"touches down on the ground, twists, and falls down!");
  	    MechFalls(mech, (abs(mof) * (para ? 1 : 2)), 1);
	} else if (MechType(mech) == CLASS_BSUIT) {
	    int i, ii, dam; 
	    mech_notify(mech, MECHALL,
	        "You are unable to control your momentum and crash!");
	    MechLOSBroadcast(mech, "crashes to the ground!");
	    for (i = 0; i < NUM_SECTIONS; i++) {
			dam = 0;
			if (GetSectOInt(mech, i) > 0) {
				for (ii = mof; ii < 0; ii++)
					dam += Number(1,4);
				DamageMech(mech, mech, 0, -1, i, 0, 0, dam, -1, -1, 0, 0);
				MechFloods(mech);
			}
		}
		MechFalls(mech, 0, 1);
	} else {
	    mech_notify(mech, MECHALL,
		"You are unable to control your momentum and crash!");
	    MechLOSBroadcast(mech, "crashes at the ground!");
		MechFalls(mech, (abs(mof) * (para ? 1 : 3)), 1);
	}
    } else if (!para) {
	MechLOSBroadcast(mech, "touches down!");
    } else if (para) {
	MechLOSBroadcast(mech, "touches down and rolls on the ground!");
/*	mech_notify(mech, MECHALL,
	    "As you hit the ground you roll and sponge some damage!");
	MechFalls(mech, (MechTons(mech) / 25), 0); */
    }
    DropSetElevation(mech, 1);
    if (!Fallen(mech))
	domino_space(mech, 2);
    if (WaterBeast(mech) && NotInWater(mech))
	MechDesiredSpeed(mech) = 0.0;

    MechFloods(mech);

    MaybeMove(mech);

}

void initiate_ood(dbref player, MECH * mech, char *buffer)
{
    char *args[4];
    int x, y, z = ORBIT_Z, argc;

    DOCHECK((argc =
	    mech_parseattributes(buffer, args, 3)) < 2,
	"Invalid attributes!");
    DOCHECK(Readnum(x, args[0]), "Invalid number! (x)");
    DOCHECK(Readnum(y, args[1]), "Invalid number! (y)");
    if (argc == 3)
	DOCHECK(Readnum(z, args[2]), "Invalid number! (z)");
    DOCHECK(OODing(mech), "OOD already in progress!");
    mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d", x, y));
    DOCHECK(MechX(mech) != x || MechY(mech) != y, "Invalid co-ordinates!");
    DOCHECK(Fallen(mech), "No way!");
    DOCHECK(Digging(mech), "The mech isn't ready to be OOD'ed!");
    if (MechStatus2(mech) & CS_ON_LAND)
	MechStatus(mech) &= ~COMBAT_SAFE;
    MechZ(mech) = z;
    MechFZ(mech) = ZSCALE * MechZ(mech);
    MarkForLOSUpdate(mech);
    notify(player, "OOD initiated.");
    if (FlyingT(mech)) {
	MechStatus(mech) &= ~LANDED;
	if (MechZ(mech) < ORBIT_Z) {
	    MechDesiredSpeed(mech) = MechMaxSpeed(mech) / 2;
	    MechVelocity(mech) = 132;
	    } else {
		MechVelocity(mech) = 0;
		MechDesiredSpeed(mech) = 0;
		MechThrust(mech) = 0;
	    }
	if (is_aero(mech))
	    MechDesiredAngle(mech) = -90;
	MaybeMove(mech);
    } else {
	MechCocoon(mech) = MechRTons(mech) / 5 / 1024 + 1;
	StopMoving(mech);
	MECHEVENT(mech, EVENT_OOD, mech_ood_event, OOD_TICK, 0);
    }
}
