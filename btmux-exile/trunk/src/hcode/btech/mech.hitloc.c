#include "mech.h"
#include "mech.events.h"
#include "p.mech.utils.h"
#include "p.mech.combat.h"
#include "p.aero.bomb.h"
#include "p.mech.update.h"
#include "p.crit.h"
#include "p.btechstats.h"

#define RotorHit(mech) \
	      if (!Fallen(mech)) \
		{ \
		  mech_notify (mech, MECHALL, "Your rotor is damaged!!"); \
		  LowerMaxSpeed(mech,MP1);\
		}
#define RotorKill(mech) \
if (!Fallen(mech)) \
{\
  mech_notify (mech, MECHALL, "Your rotor is destroyed!!"); \
  if (!Landed(mech)) \
    { \
      mech_notify (mech, MECHALL, "You crash to the ground!"); \
      MechLOSBroadcast (mech, "crashes to the ground!"); \
      MechFalls (mech, MAX(1+MechsElevation(mech),1), 0); \
    } \
  MakeMechFall(mech); \
  SetMaxSpeed(mech,0.0);\
}

int ModifyHeadHit(int hitGroup, MECH * mech)
{
    int newloc = FindPunchLocation(hitGroup, mech);

    if (MechType(mech) != CLASS_MECH)
	return newloc;
    if (newloc != HEAD) {
        mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
        mech_notify(mech, MECHALL, "The cockpit violently shakes from a grazing blow! You are momentarily stunned!");
        if (CrewStunning(mech))
            StopCrewStunning(mech);
        MechLOSBroadcast(mech, "significantly slows down and starts wobbling!");
        MechCritStatus(mech) |= CREW_STUNNED;
        if (MechSpeed(mech) > WalkingSpeed(MechMaxSpeed(mech)))
            MechDesiredSpeed(mech) = WalkingSpeed(MechMaxSpeed(mech));
        MECHEVENT(mech, EVENT_CREWSTUN, mech_crewstun_event, MECHSTUN_TICK, 0);
        }
    return newloc;
} 

int FindPunchLocation(int hitGroup, MECH * mech)
{
    int roll = Number(1, 6);

  if (MechMove(mech) == MOVE_QUAD) {
    switch (hitGroup) {
    case LEFTSIDE:
        switch (roll) {
        case 1:
        case 2:
        	return LTORSO;
		case 3:
		case 4:
            return CTORSO;
        case 5:
        	switch (Number(1,2)) {
				case 1: return LLEG;
				case 2: return LARM;
			}
        case 6:
            return HEAD;
        }
    case BACK:
    case FRONT:
        switch (roll) {
        case 1:
	    switch (Number(1,3)) {
	    case 1:
		return LTORSO;
	    case 2:
		return CTORSO;
	    case 3:
		return RTORSO;
	    }
        case 2:
            return LTORSO;
        case 3:
            return CTORSO;
        case 4:
            return RTORSO;
	case 5:
	    switch (Number(1,2)) {
            case 1:
                return LARM;
            case 2:
                return RARM;
            }
	case 6:
	    return HEAD;
        }
        break;

    case RIGHTSIDE:
        switch (roll) {
        case 1:
        case 2:
        	return RTORSO;
		case 3:
        case 4:
			return CTORSO;
        case 5:
        	switch (Number(1,2)) {
				case 1: return RLEG;
				case 2: return RARM;
			}
        case 6:
            return HEAD;
        }
    }
  } else {
    switch (hitGroup) {
    case LEFTSIDE:
	switch (roll) {
	case 1:
	case 2:
	    return LTORSO;
	case 3:
	    return CTORSO;
	case 4:
	case 5:
	    return LARM;
	case 6:
	    return HEAD;
	}
    case BACK:
    case FRONT:
	switch (roll) {
	case 1:
	    return LARM;
	case 2:
	    return LTORSO;
	case 3:
	    return CTORSO;
	case 4:
	    return RTORSO;
	case 5:
	    return RARM;
	case 6:
	    return HEAD;
	}
	break;

    case RIGHTSIDE:
	switch (roll) {
	case 1:
	case 2:
	    return RTORSO;
	case 3:
	    return CTORSO;
	case 4:
	case 5:
	    return RARM;
	case 6:
	    return HEAD;
	}
    }
   }
    return CTORSO;
}

int FindKickLocation(int hitGroup, MECH * mech)
{
    int roll = Number(1, 6);

  if (MechMove(mech) == MOVE_QUAD)
       {
	switch (hitGroup) {
		case LEFTSIDE:
			switch (roll)
			{
			case 1:
			case 2:
			case 3:
			    return LLEG;
			case 4:
			case 5:
			case 6:
			    return LARM;
			}
		case RIGHTSIDE:
			switch (roll)
			{
			case 1:
			case 2:
			case 3:
			    return RLEG;
			case 4:
			case 5:
			case 6:
			    return RARM;
			}
		case FRONT:
			switch (roll)
			{
			case 1:
			case 2:
			case 3:
			    return RARM;
			case 4:
			case 5:
			case 6:
			    return LARM;
			}
		case BACK:
			switch (roll)
			{
			case 1:
			case 2:
			case 3:
			    return RLEG;
			case 4:
			case 5:
			case 6:
			    return LLEG;
			}
	}
       } else {
	    switch (hitGroup) {
	    case LEFTSIDE:
		return LLEG;
	    case BACK:
	    case FRONT:
		switch (roll) {
		case 1:
		case 2:
		case 3:
		    return RLEG;
		case 4:
		case 5:
		case 6:
		    return LLEG;
		}
	    case RIGHTSIDE:
		return RLEG;
	    }
  }
    return RLEG;
}

int get_bsuit_hitloc(MECH * mech)
{
    int i;
    int table[NUM_BSUIT_MEMBERS];
    int last = 0;

    for (i = 0; i < NUM_BSUIT_MEMBERS; i++)
	if (GetSectInt(mech, i))
	    table[last++] = i;
    if (!last)
	return -1;
    return table[Number(0, last - 1)];
}

int TransferTarget(MECH * mech, int hitloc)
{
    switch (MechType(mech)) {
    case CLASS_BSUIT:
	return get_bsuit_hitloc(mech);
    case CLASS_AERO:
	return -1;
	break;
    case CLASS_MECH:
    case CLASS_MW:
	switch (hitloc) {
	case RARM:
	case RLEG:
	    return RTORSO;
	    break;
	case LARM:
	case LLEG:
	    return LTORSO;
	    break;
	case RTORSO:
	case LTORSO:
	    return CTORSO;
	    break;
        case HEAD:
	    if (MechSpecials2(mech) & TORSOCOCKPIT_TECH)
		return CTORSO;
	    break;
	}
    case CLASS_VTOL:
	switch(hitloc) {
	case LSIDE:
	case RSIDE:
	case FSIDE:
	case BSIDE:
	    return ROTOR;
	    break;
	}
    case CLASS_VEH_NAVAL:
    case CLASS_VEH_GROUND:
 	switch (hitloc) {
	case LSIDE:
	case RSIDE:
	case FSIDE:
	case BSIDE:
	    return (GetSectInt(mech, TURRET) ? TURRET : -1);
	    break;
	case TURRET:
	    return -1;
	    break;
	}
        break;
    }
    return -1;
}

int FindSwarmHitLocation(int *iscritical, int *isrear)
{
    *isrear = 0;
    *iscritical = 1;

    switch (Roll()) {
    case 12:
    case 2:
	return HEAD;
    case 3:
    case 11:
	*isrear = 1;
	return CTORSO;
    case 4:
	*isrear = 1;
    case 5:
	return RTORSO;
    case 10:
	*isrear = 1;
    case 9:
	return LTORSO;
    case 6:
	return RARM;
    case 8:
	return LARM;
    case 7:
	return CTORSO;
    }
    return HEAD;
}

int crittable(MECH * m, MECH * att, int loc, int tres)
{
    int d;

    if (!GetSectOArmor(m, loc))
	return 1;
    if (att && MechType(att) == CLASS_MW && MechType(m) != CLASS_MW)
	return 0;
    if (MechType(m) != CLASS_MECH && mudconf.btech_vcrit <= 1)
	return 0;
    d = (100 * GetSectArmor(m, loc)) / GetSectOArmor(m, loc);
    if (MechType(m) != CLASS_MECH && (MechSpecials(m) & (XL_TECH|XXL_TECH)))
	return 1;
    if (MechSpecials(m) & ICE_TECH)
	return 1;
    if (d < tres)
	return 1;
/*  More 3030 lameness removed. Miniscule, perhaps, but still unfair. - DJ */
/*    if (d == 100) { */
/*	if (Number(1, 71) == 23) */
/*	    return 1; */
/*	return 0; */
/*    } */
/*    if (d < (100 - ((100 - tres) / 2))) */
/*	if (Number(1, 11) == 6) */
/*	    return 1; */
    return 0;
}

int FindFasaHitLocation(MECH * mech, int hitGroup, int *iscritical,
    int *isrear, MECH * att)
{
    int roll, hitloc = 0;
    int side, ZB;

    *iscritical = 0;
    roll = Roll();

    if (MechStatus(mech) & COMBAT_SAFE)
	return 0;
    if (MechCritStatus(mech) & DUG_IN)
	if (GetSectOInt(mech, TURRET))
	    if (Number(1, 100) >= 42)
		return TURRET;
    stat.hitrolls[roll - 2]++;
    stat.tothrolls++;
    switch (MechType(mech)) {
    case CLASS_BSUIT:
	if ((hitloc = get_bsuit_hitloc(mech)) < 0)
	    return Number(0, NUM_BSUIT_MEMBERS - 1);
    case CLASS_MW:
    case CLASS_MECH:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		*iscritical = 1;
		return LTORSO;
	    case 3:
		return LLEG;
	    case 4:
	    case 5:
		return LARM;
	    case 6:
		return LLEG;
	    case 7:
		return LTORSO;
	    case 8:
		return CTORSO;
	    case 9:
		return RTORSO;
	    case 10:
		return RARM;
	    case 11:
		return RLEG;
	    case 12:
		if (mudconf.btech_headtopunchtab)
		    return ModifyHeadHit(hitGroup, mech);
		return HEAD;
	    }
	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
		*iscritical = 1;
		return RTORSO;
	    case 3:
		return RLEG;
	    case 4:
	    case 5:
		return RARM;
	    case 6:
		return RLEG;
	    case 7:
		return RTORSO;
	    case 8:
		return CTORSO;
	    case 9:
		return LTORSO;
	    case 10:
		return LARM;
	    case 11:
		return LLEG;
	    case 12:
		if (mudconf.btech_headtopunchtab)
		    return ModifyHeadHit(hitGroup, mech);
		return HEAD;
	    }
	case FRONT:
	case BACK:
	    switch (roll) {
	    case 2:
		*iscritical = 1;
		return CTORSO;
	    case 3:
	    case 4:
		return RARM;
	    case 5:
		return RLEG;
	    case 6:
		return RTORSO;
	    case 7:
		return CTORSO;
	    case 8:
		return LTORSO;
	    case 9:
		return LLEG;
	    case 10:
	    case 11:
		return LARM;
	    case 12:
		if (mudconf.btech_headtopunchtab)
		    return ModifyHeadHit(hitGroup, mech);
		return HEAD;
	    }
	}
	break;
    case CLASS_VEH_GROUND:
	switch (hitGroup) {

	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		/* A Roll on Determining Critical Hits Table */
		*iscritical = 1;
		return LSIDE;
	    case 3:
		if (mudconf.btech_tankfriendly) {
		    if (!Fallen(mech)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			switch (MechMove(mech)) {
			case MOVE_TRACK:
			    mech_notify(mech, MECHALL,
				"One of your tracks is seriously damaged!!");
			    break;
			case MOVE_WHEEL:
			    mech_notify(mech, MECHALL,
				"One of your wheels is seriously damaged!!");
			    break;
			case MOVE_HOVER:
			    mech_notify(mech, MECHALL,
				"Your air skirt is seriously damaged!!");
			    break;
			case MOVE_HULL:
			case MOVE_SUB:
			case MOVE_FOIL:
			    mech_notify(mech, MECHALL,
				"Your speed slows down a lot..");
			    break;
			}
			LowerMaxSpeed(mech, MP2);
		    }
		    return LSIDE;
		}
		/* Cripple tank */
		if (!Fallen(mech)) {
		    mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		    switch (MechMove(mech)) {
		    case MOVE_TRACK:
			mech_notify(mech, MECHALL,
			    "One of your tracks is destroyed, imobilizing your vehicle!!");
			break;
		    case MOVE_WHEEL:
			mech_notify(mech, MECHALL,
			    "One of your wheels is destroyed, imobilizing your vehicle!!");
			break;
		    case MOVE_HOVER:
			mech_notify(mech, MECHALL,
			    "Your lift fan is destroyed, imobilizing your vehicle!!");
			break;
		    case MOVE_HULL:
		    case MOVE_SUB:
		    case MOVE_FOIL:
			mech_notify(mech, MECHALL,
			    "You are halted in your tracks - literally.");
		    }
		    SetMaxSpeed(mech, 0.0);

		    MakeMechFall(mech);
		}
		return LSIDE;
	    case 4:
	    case 5:
		/* MP -1 */
		if (!Fallen(mech)) {
		    mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		    switch (MechMove(mech)) {
		    case MOVE_TRACK:
			mech_notify(mech, MECHALL,
			    "One of your tracks is damaged!!");
			break;
		    case MOVE_WHEEL:
			mech_notify(mech, MECHALL,
			    "One of your wheels is damaged!!");
			break;
		    case MOVE_HOVER:
			mech_notify(mech, MECHALL,
			    "Your air skirt is damaged!!");
			break;
		    case MOVE_HULL:
		    case MOVE_SUB:
		    case MOVE_FOIL:
			mech_notify(mech, MECHALL,
			    "Your speed slows down..");
			break;
		    }
		    LowerMaxSpeed(mech, MP1);
		}
		return LSIDE;
		break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
		/* MP -1 if hover */
		return LSIDE;
	    case 10:
		return (GetSectInt(mech, TURRET)) ? TURRET : LSIDE;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    if (!(MechCritStatus(mech) & TURRET_LOCKED)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			MechCritStatus(mech) |= TURRET_LOCKED;
			mech_notify(mech, MECHALL,
			    "Your turret takes a direct hit and immobilizes!");
		    }
		    return TURRET;
		} else
		    return LSIDE;
	    case 12:
		/* A Roll on Determining Critical Hits Table */
		*iscritical = 1;
		return LSIDE;
	    }
	    break;
	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
		*iscritical = 1;
		return RSIDE;
	    case 3:
		if (mudconf.btech_tankfriendly) {
		    if (!Fallen(mech)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			switch (MechMove(mech)) {
			case MOVE_TRACK:
			    mech_notify(mech, MECHALL,
				"One of your tracks is seriously damaged!!");
			    break;
			case MOVE_WHEEL:
			    mech_notify(mech, MECHALL,
				"One of your wheels is seriously damaged!!");
			    break;
			case MOVE_HOVER:
			    mech_notify(mech, MECHALL,
				"Your air skirt is seriously damaged!!");
			    break;
			case MOVE_HULL:
			case MOVE_SUB:
			case MOVE_FOIL:
			    mech_notify(mech, MECHALL,
				"Your speed slows down a lot..");
			    break;
			}
			LowerMaxSpeed(mech, MP2);
		    }
		    return RSIDE;
		}
		/* Cripple Tank */
		if (!Fallen(mech)) {
		    mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		    switch (MechMove(mech)) {
		    case MOVE_TRACK:
			mech_notify(mech, MECHALL,
			    "One of your tracks is destroyed, imobilizing your vehicle!!");
			break;
		    case MOVE_WHEEL:
			mech_notify(mech, MECHALL,
			    "One of your wheels is destroyed, imobilizing your vehicle!!");
			break;
		    case MOVE_HOVER:
			mech_notify(mech, MECHALL,
			    "Your lift fan is destroyed, imobilizing your vehicle!!");
			break;
		    case MOVE_HULL:
		    case MOVE_SUB:
		    case MOVE_FOIL:
			mech_notify(mech, MECHALL,
			    "You are halted in your tracks - literally.");
		    }
		    SetMaxSpeed(mech, 0.0);

		    MakeMechFall(mech);
		}
		return RSIDE;
	    case 4:
	    case 5:
		/* MP -1 */
		if (!Fallen(mech)) {
		    mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		    switch (MechMove(mech)) {
		    case MOVE_TRACK:
			mech_notify(mech, MECHALL,
			    "One of your tracks is damaged!!");
			break;
		    case MOVE_WHEEL:
			mech_notify(mech, MECHALL,
			    "One of your wheels is damaged!!");
			break;
		    case MOVE_HOVER:
			mech_notify(mech, MECHALL,
			    "Your air skirt is damaged!!");
			break;
		    case MOVE_HULL:
		    case MOVE_SUB:
		    case MOVE_FOIL:
			mech_notify(mech, MECHALL,
			    "Your speed slows down..");
			break;
		    }
		    LowerMaxSpeed(mech, MP1);
		}
		return RSIDE;
	    case 6:
	    case 7:
	    case 8:
		return RSIDE;
	    case 9:
		/* MP -1 if hover */
		if (!Fallen(mech)) {
		    if (MechMove(mech) == MOVE_HOVER) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			mech_notify(mech, MECHALL,
			    "Your air skirt is damaged!!");
			LowerMaxSpeed(mech, MP1);
		    }
		}
		return RSIDE;
	    case 10:
		return (GetSectInt(mech, TURRET)) ? TURRET : RSIDE;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    if (!(MechCritStatus(mech) & TURRET_LOCKED)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			MechCritStatus(mech) |= TURRET_LOCKED;
			mech_notify(mech, MECHALL,
			    "Your turret takes a direct hit and immobilizes!");
		    }
		    return TURRET;
		} else
		    return RSIDE;
	    case 12:
		/* A Roll on Determining Critical Hits Table */
		*iscritical = 1;
		return RSIDE;
	    }
	    break;


	case FRONT:
	case BACK:
	    side = (hitGroup == FRONT ? FSIDE : BSIDE);
	    switch (roll) {
	    case 2:
		/* A Roll on Determining Critical Hits Table */
		*iscritical = 1;
		return side;
	    case 3:
		if (mudconf.btech_tankshield) {
		    if (mudconf.btech_tankfriendly) {
			if (!Fallen(mech)) {
			    mech_notify(mech, MECHALL,
				"%ch%cyCRITICAL HIT!!%c");
			    switch (MechMove(mech)) {
			    case MOVE_TRACK:
				mech_notify(mech, MECHALL,
				    "One of your tracks is seriously damaged!!");
				break;
			    case MOVE_WHEEL:
				mech_notify(mech, MECHALL,
				    "One of your wheels is seriously damaged!!");
				break;
			    case MOVE_HOVER:
				mech_notify(mech, MECHALL,
				    "Your air skirt is seriously damaged!!");
				break;
			    case MOVE_HULL:
			    case MOVE_SUB:
			    case MOVE_FOIL:
				mech_notify(mech, MECHALL,
				    "Your speed slows down a lot..");
				break;
			    }
			    LowerMaxSpeed(mech, MP2);
			}
			return side;
		    }
		    /* Cripple tank */
		    if (!Fallen(mech)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			switch (MechMove(mech)) {
			case MOVE_TRACK:
			    mech_notify(mech, MECHALL,
				"One of your tracks is destroyed, imobilizing your vehicle!!");
			    break;
			case MOVE_WHEEL:
			    mech_notify(mech, MECHALL,
				"One of your wheels is destroyed, imobilizing your vehicle!!");
			    break;
			case MOVE_HOVER:
			    mech_notify(mech, MECHALL,
				"Your lift fan is destroyed, imobilizing your vehicle!!");
			    break;
			case MOVE_HULL:
			case MOVE_SUB:
			case MOVE_FOIL:
			    mech_notify(mech, MECHALL,
				"You are halted in your tracks - literally.");
			}
			SetMaxSpeed(mech, 0.0);

			MakeMechFall(mech);
		    }
		}
		return side;
	    case 4:
		/* MP -1 */
		if (mudconf.btech_tankshield) {
		    if (!Fallen(mech)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			switch (MechMove(mech)) {
			case MOVE_TRACK:
			    mech_notify(mech, MECHALL,
				"One of your tracks is damaged!!");
			    break;
			case MOVE_WHEEL:
			    mech_notify(mech, MECHALL,
				"One of your wheels is damaged!!");
			    break;
			case MOVE_HOVER:
			    mech_notify(mech, MECHALL,
				"Your air skirt is damaged!!");
			    break;
			case MOVE_HULL:
			case MOVE_SUB:
			case MOVE_FOIL:
			    mech_notify(mech, MECHALL,
				"Your speed slows down..");
			    break;
			}
			LowerMaxSpeed(mech, MP1);
		    }
		}
		return side;
	    case 5:
		/* MP -1 if Hovercraft */
		if (!Fallen(mech)) {
		    if (MechMove(mech) == MOVE_HOVER) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			mech_notify(mech, MECHALL,
			    "Your air skirt is damaged!!");
			LowerMaxSpeed(mech, MP1);
		    }
		}
		return side;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
		return side;
	    case 10:
		return (GetSectInt(mech, TURRET)) ? TURRET : side;
	    case 11:
		*iscritical = 1;
		/* Lock turret into place */
		if (GetSectInt(mech, TURRET)) {
		    if (!(MechCritStatus(mech) & TURRET_LOCKED)) {
			mech_notify(mech, MECHALL,
			    "%ch%cyCRITICAL HIT!!%c");
			MechCritStatus(mech) |= TURRET_LOCKED;
			mech_notify(mech, MECHALL,
			    "Your turret takes a direct hit and immobilizes!");
		    }
		    return TURRET;
		} else
		    return side;
	    case 12:
		/* A Roll on Determining Critical Hits Table */
		if (crittable(mech, att, (GetSectInt(mech, TURRET)) ? TURRET : side, mudconf.btech_critlevel))
		    *iscritical = 1;
		return (GetSectInt(mech, TURRET)) ? TURRET : side;
	    }
	}
	break;
    case CLASS_AERO:
        if (att)
            if (((ZB = FindZBearing(MechFX(att), MechFY(att), MechFZ(att), MechFX(mech), MechFY(mech), MechFZ(mech))) < -45) ||
                 (ZB > 45)) {
                switch (roll) {
                    case 2:
                    case 4:
                    case 5:
                    case 7:
                        return AERO_NOSE;
                    case 3:
                    case 6:
                    case 8:
                    case 11:
                        if (hitGroup == LEFTSIDE)
                            return AERO_LWING;
                        else
                            return AERO_RWING;
                    case 9:
                    case 10:
                    case 12:
                        return AERO_AFT;
                    default:
                        return AERO_AFT;
                    }
                }
	switch (hitGroup) {
	case FRONT:
	    switch (roll) {
	    case 2:
	    case 3:
	    case 6:
	    case 7:
	    case 8:
	    case 11:
	    case 12:
		return AERO_NOSE;
	    case 4:
	    case 5:
	    	return AERO_RWING;
	    case 9:
	    case 10:
		return AERO_LWING;
	    }
	    break;
	case LEFTSIDE:
	case RIGHTSIDE:
	    side = ((hitGroup == LEFTSIDE) ? AERO_LWING : AERO_RWING);
	    switch (roll) {
	    case 2:
	    case 4:
	    case 5:
		return AERO_NOSE;
	    case 3:
	    case 6:
	    case 7:
	    case 8:
	    case 11:
		return side;
	    case 9:
	    case 10:
	    case 12:
		return AERO_AFT;
	    }
	    break;
	case BACK:
	    switch (roll) {
	    case 2:
	    case 3:
	    case 6:
	    case 7:
	    case 8:
	    case 11:
	    case 12:
		return AERO_AFT;
	    case 4:
	    case 5:
		return AERO_RWING;
	    case 9:
	    case 10:
		return AERO_LWING;
	    }
	    break;
	}
	break;
    case CLASS_DS:
    case CLASS_SPHEROID_DS:
	switch (hitGroup) {
	case FRONT:
	    switch (roll) {
	    case 2:
	    case 12:
	    case 3:
	    case 11:
		return DS_NOSE;
	    case 5:
		return DS_RWING;
	    case 6:
	    case 7:
	    case 8:
		return DS_NOSE;
	    case 9:
		return DS_LWING;
	    case 4:
	    case 10:
		return (Number(1, 2)) == 1 ? DS_LWING : DS_RWING;
	    }
	case LEFTSIDE:
	case RIGHTSIDE:
	    side = (hitGroup == LEFTSIDE) ? DS_LWING : DS_RWING;
/*
	    if (Number(1, 2) == 2)
		SpheroidToRear(mech, side);
*/
	    switch (roll) {
	    case 2:
		if (crittable(mech, att, DS_NOSE, 30))
		    ds_BridgeHit(mech);
		return DS_NOSE;
	    case 3:
	    case 11:
		if (crittable(mech, att, side, 60))
		    LoseWeapon(mech, side);
		return side;
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
	    case 10:
		return side;
	    case 9:
		return DS_NOSE;
	    case 12:
		if (crittable(mech, att, side, 60))
		    *iscritical = 1;
		return side;
	    }
	case BACK:
	    switch (roll) {
	    case 2:
	    case 12:
		if (crittable(mech, att, DS_AFT, 60))
		    *iscritical = 1;
		return DS_AFT;
	    case 3:
	    case 11:
		return DS_AFT;
	    case 4:
	    case 7:
	    case 10:
		if (crittable(mech, att, DS_AFT, 60))
		    DestroyHeatSink(mech, DS_AFT);
		return DS_AFT;
	    case 5:
		hitloc = DS_RWING;
/*
		SpheroidToRear(mech, hitloc);
*/
		return hitloc;
	    case 6:
	    case 8:
		return DS_AFT;
	    case 9:
		hitloc = DS_LWING;
/*
		SpheroidToRear(mech, hitloc);
*/
		return hitloc;
	    }
	}
	break;
    case CLASS_VEH_VTOL:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		hitloc = ROTOR;
		*iscritical = 1;
		RotorKill(mech);
		break;
	    case 3:
		*iscritical = 1;
		break;
	    case 4:
	    case 5:
		hitloc = ROTOR;
		RotorHit(mech);
		break;
	    case 6:
	    case 7:
	    case 8:
		hitloc = LSIDE;
		break;
	    case 9:
		/* Destroy Main Weapon but do not destroy armor */
		DestroyMainWeapon(mech);
		hitloc = 0;
		break;
	    case 10:
	    case 11:
		hitloc = ROTOR;
		RotorHit(mech);
		break;
	    case 12:
		hitloc = ROTOR;
		*iscritical = 1;
		RotorHit(mech);
		break;
	    }
	    break;

	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
		hitloc = ROTOR;
		*iscritical = 1;
		RotorKill(mech);
		break;
	    case 3:
		*iscritical = 1;
		break;
	    case 4:
	    case 5:
		hitloc = ROTOR;
		RotorHit(mech);
		break;
	    case 6:
	    case 7:
	    case 8:
		hitloc = RSIDE;
		break;
	    case 9:
		/* Destroy Main Weapon but do not destroy armor */
		DestroyMainWeapon(mech);
		break;
	    case 10:
	    case 11:
		hitloc = ROTOR;
		RotorHit(mech);
		break;
	    case 12:
		hitloc = ROTOR;
		*iscritical = 1;
		RotorHit(mech);
		break;
	    }
	    break;

	case FRONT:
	case BACK:
	    side = (hitGroup == FRONT ? FSIDE : BSIDE);

	    switch (roll) {
	    case 2:
		hitloc = ROTOR;
		*iscritical = 1;
		RotorKill(mech);
		break;
	    case 3:
		hitloc = ROTOR;
		RotorKill(mech);
		break;
	    case 4:
	    case 5:
		hitloc = ROTOR;
		RotorHit(mech);
		break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
		hitloc = side;
		break;
	    case 10:
	    case 11:
		hitloc = ROTOR;
		RotorHit(mech);
		break;
	    case 12:
		hitloc = ROTOR;
		*iscritical = 1;
		RotorHit(mech);
		break;
	    }
	    break;
	}
	break;
    case CLASS_VEH_NAVAL:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		hitloc = LSIDE;
		if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
		    *iscritical = 1;
		break;
	    case 3:
	    case 4:
	    case 5:
		hitloc = LSIDE;
		break;
	    case 9:
		hitloc = LSIDE;
		break;
	    case 10:
		if (GetSectInt(mech, TURRET))
		    hitloc = TURRET;
		else
		    hitloc = LSIDE;
		break;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    hitloc = TURRET;
		    if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
			*iscritical = 1;
		} else
		    hitloc = LSIDE;
		break;
	    case 12:
		hitloc = LSIDE;
		*iscritical = 1;
		break;
	    }
	    break;

	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
	    case 12:
		hitloc = RSIDE;
		if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
		    *iscritical = 1;
		break;
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
		hitloc = RSIDE;
		break;
	    case 10:
		if (GetSectInt(mech, TURRET))
		    hitloc = TURRET;
		else
		    hitloc = RSIDE;
		break;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    hitloc = TURRET;
		    if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
			*iscritical = 1;
		} else
		    hitloc = RSIDE;
		break;
	    }
	    break;

	case FRONT:
	case BACK:
	    side = (hitGroup == FRONT ? FSIDE : BSIDE);
	    switch (roll) {
	    case 2:
	    case 12:
		hitloc = side;
		if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
		    *iscritical = 1;
		break;
	    case 3:
		hitloc = side;
		break;
	    case 4:
		hitloc = side;
		break;
	    case 5:
		hitloc = side;
		break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
		hitloc = side;
		break;
	    case 10:
		if (GetSectInt(mech, TURRET))
		    hitloc = TURRET;
		else
		    hitloc = side;
		break;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    hitloc = TURRET;
		    *iscritical = 1;
		} else
		    hitloc = side;
		break;
	    }
	    break;
	}
	break;
    }
    return (hitloc);
}

void MotiveCritRoll(MECH * mech, MECH * attacker)
{
int roll = Roll();

if (attacker && MechType(attacker) == CLASS_MW)
    return;
if (MechSpecials(mech) & CRITPROOF_TECH)
    return;

roll += (MechMove(mech) == MOVE_WHEEL ? 2 : MechMove(mech) == MOVE_HOVER ? 4 : 0);
roll += (MechSpecials(mech) & (ICE_TECH|XXL_TECH) ? 2 : MechSpecials(mech) & XL_TECH ? 1 : 0);

switch (roll) {
    case 12:
	mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	mech_notify(mech, MECHALL, tprintf("Your %s heavily damaged!!",
	    MechMove(mech) == MOVE_HOVER ? "air skirt is" : MechMove(mech) == MOVE_WHEEL ? "wheels are" : MechMove(mech) == MOVE_TRACK ? "tracks are" : "movement system is"));
	    SetMaxSpeed(mech, 0);
	    MechSpeed(mech) = 0.0;
	    mech_notify(mech, MECHALL, "You are violently bounced around in your cockpit!");
            mech_notify(mech, MECHALL, "You attempt to avoid personal injury!");
            if (MadePilotSkillRoll(mech, (MechSpeed(mech) / MP4))) {
                mech_notify(mech, MECHALL, "Your fancy moves worked!");
            } else {
                mech_notify(mech, MECHALL, "You cute fancy moves failed....");
                headhitmwdamage(mech, 1);
            }
    case 10:
    case 11:
	mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
	mech_notify(mech, MECHALL, tprintf("Your %s damaged!",
	    MechMove(mech) == MOVE_HOVER ? "air skirt is" : MechMove(mech) == MOVE_WHEEL ? "wheels are" : MechMove(mech) == MOVE_TRACK ? "tracks are" : "movement system is"));
	if (MechMaxSpeed(mech) > MP1)
            LowerMaxSpeed(mech, MP1);
	else
	    SetMaxSpeed(mech, 0);
        MechPilotSkillBase(mech) += 2;
    case 8:
    case 9:
	mech_notify(mech, MECHALL, tprintf("Your %s hit!",
            MechMove(mech) == MOVE_HOVER ? "air skirt is" : MechMove(mech) == MOVE_WHEEL ? "wheels are" : MechMove(mech) == MOVE_TRACK ? "tracks are" : "movement system is"));
        MechPilotSkillBase(mech) += 1;
    } 
if (MechSpeed(mech) > MMaxSpeed(mech))
    MechSpeed(mech) = MMaxSpeed(mech);
}

int FindHitLocation(MECH * mech, int hitGroup, int *iscritical,
    int *isrear, MECH * att)
{
    int roll, hitloc = 0, ftac = 0;
    int side;
    int QuadTable = (MechMove(mech) == MOVE_QUAD || Fallen(mech));
    int ZB;

    roll = Roll();

    if (mudconf.btech_fasacrit > 0)
	return FindFasaHitLocation(mech, hitGroup, iscritical, isrear, att);


    if (MechStatus(mech) & COMBAT_SAFE)
	return 0;
    if (MechCritStatus(mech) & DUG_IN && !(MechMove(mech) == MOVE_QUAD) && hitGroup == FRONT)
	if (GetSectOInt(mech, TURRET))
	    if (Number(1, 100) >= 33)
		return TURRET;
    stat.hitrolls[roll - 2]++;
    stat.tothrolls++;
    switch (MechType(mech)) {
    case CLASS_BSUIT:
	if ((hitloc = get_bsuit_hitloc(mech)) < 0)
	    return Number(0, NUM_BSUIT_MEMBERS - 1);
    case CLASS_MW:
    case CLASS_MECH:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
	    if (mudconf.btech_fasacrit < 0) {
		switch (Roll()) {
		    case 2: ftac = LTORSO; break;
		    case 3: ftac = LLEG; break;
		    case 4:
		    case 5: ftac = LARM; break;
		    case 6: ftac = LLEG; break;
		    case 7: ftac = LTORSO; break;
		    case 8: ftac = CTORSO; break;
		    case 9: ftac = RTORSO; break;
		    case 10: ftac = RARM; break;
		    case 11: ftac = RLEG; break;
		    case 12: ftac = LTORSO; break;
		    }
		if (crittable(mech, att, ftac, mudconf.btech_critlevel))
		    *iscritical = 1;
		return ftac;
		} else if (crittable(mech, att, LTORSO, mudconf.btech_critlevel)) {
		    *iscritical = 1;
		    return LTORSO;
		} else { 
	  	    return LTORSO;
		}
	    case 3:	return LLEG;
	    case 4:
	    case 5:	return LARM;
	    case 6:	return LLEG;
	    case 7:	return LTORSO;
	    case 8:	return CTORSO;
	    case 9:	return RTORSO;
	    case 10:	return RARM;
	    case 11:	return RLEG;
	    case 12:	if (mudconf.btech_headtopunchtab)
			    return ModifyHeadHit(hitGroup, mech);
			else
			    return HEAD;
	    }
	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
	    if (mudconf.btech_fasacrit < 0) {
		switch (Roll()) {
		    case 2: ftac = RTORSO; break;
		    case 3: ftac = RLEG; break;
		    case 4:
		    case 5: ftac = RARM; break;
		    case 6: ftac = RLEG; break;
		    case 7: ftac = RTORSO; break;
		    case 8: ftac = CTORSO; break;
		    case 9: ftac = LTORSO; break;
		    case 10: ftac = LARM; break;
		    case 11: ftac = LLEG; break;
		    case 12: ftac = RTORSO; break;
		    }
		if (crittable(mech, att, ftac, mudconf.btech_critlevel))
		    *iscritical = 1;
		return ftac;
		} else if (crittable(mech, att, RTORSO, mudconf.btech_critlevel)) {
		    *iscritical = 1;
		    return RTORSO;
		} else {
	    	    return RTORSO;
		}
	    case 3:		return RLEG;
	    case 4:
	    case 5:		return RARM;
	    case 6:		return RLEG;
	    case 7:		return RTORSO;
	    case 8:		return CTORSO;
	    case 9:		return LTORSO;
	    case 10:	return LARM;
	    case 11:	return LLEG;
	    case 12:	if (mudconf.btech_headtopunchtab)
			    return ModifyHeadHit(hitGroup, mech);
			else
			    return HEAD;
	    }
	case FRONT:
            switch (roll) {
            case 2:
            if (mudconf.btech_fasacrit < 0) {
		switch (Roll()) {
		    case 2: ftac = CTORSO; break;
		    case 3: ftac = QuadTable ? RLEG : RARM; break;
		    case 4: ftac = RARM; break;
		    case 5: ftac = QuadTable ? RARM : RLEG; break;
		    case 6: ftac = RTORSO; break;
		    case 7: ftac = CTORSO; break;
		    case 8: ftac = LTORSO; break;
		    case 9: ftac = QuadTable ? LARM : LLEG; break;
		    case 10: ftac = LARM; break;
		    case 11: ftac = QuadTable ? LLEG : LARM; break;
		    case 12: ftac = CTORSO; break;
		    }
	    if (crittable(mech, att, ftac, mudconf.btech_critlevel))
		*iscritical = 1;
	    return ftac;
	    } else if (crittable(mech, att, CTORSO, mudconf.btech_critlevel)) {
                *iscritical = 1;
                return CTORSO;
	    } else {
		return CTORSO;
	    }
            case 3:		return QuadTable ? RLEG : RARM;
            case 4:		return RARM;
            case 5: 	return QuadTable ? RARM : RLEG;
            case 6: 	return RTORSO;
            case 7: 	return CTORSO;
            case 8: 	return LTORSO;
            case 9: 	return QuadTable ? LARM : LLEG;
            case 10:	return LARM;
            case 11:	return QuadTable ? LLEG : LARM;
            case 12:	if (mudconf.btech_headtopunchtab)
			    return ModifyHeadHit(hitGroup, mech);
			else
			    return HEAD;
            }
	case BACK:
	    switch (roll) {
	    case 2:
	    if (mudconf.btech_fasacrit < 0) {
		switch (Roll()) {
		    case 2: ftac = CTORSO; break;
		    case 3: ftac = RARM; break;
		    case 4: ftac = QuadTable ? RLEG : RARM;
		    case 5: ftac = RLEG; break;
		    case 6: ftac = RTORSO; break;
		    case 7: ftac = CTORSO; break;
		    case 8: ftac = LTORSO; break;
		    case 9: ftac = LLEG; break;
		    case 10: ftac = QuadTable ? LLEG : LARM; break;
	    	    case 11: ftac = LARM; break;
		    case 12: ftac = CTORSO; break;
		    }
	    if (crittable(mech, att, ftac, mudconf.btech_critlevel))
		*iscritical = 1;
  	    return ftac;
  	    } else if (crittable(mech, att, CTORSO, mudconf.btech_critlevel)) {
		*iscritical = 1;
		return CTORSO;
	    } else {
		return CTORSO;
	    }
	    case 3:		return RARM;
	    case 4:	    return QuadTable ? RLEG : RARM;
	    case 5:	    return RLEG;
	    case 6:		return RTORSO;
	    case 7:		return CTORSO;
	    case 8:		return LTORSO;
	    case 9:	    return LLEG;
	    case 10:    return QuadTable ? LLEG : LARM;
	    case 11:    return LARM;
	    case 12:	if (mudconf.btech_headtopunchtab)
		    	    return ModifyHeadHit(hitGroup, mech);
			else
			    return HEAD;
	    }
	}
	break;
    case CLASS_VEH_GROUND:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		if (crittable(mech, att, LSIDE, mudconf.btech_critlevel))
		    *iscritical = 1;
		return LSIDE;
	    case 3:
		if (mudconf.btech_fasacrit >= 0 && MechMove(mech) == MOVE_HOVER) {
			mech_notify(mech, MECHALL,
			    "Your are violently bounced around in your cockpit!");
			mech_notify(mech, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(mech, (MechSpeed(mech) / MP4))) {
			    mech_notify(mech, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(mech, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(mech, 1);
			}
			switch(Number(2,12))
				{
				case 2:
					*iscritical = 1;
					break;
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
					mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
		       	                mech_notify(mech, MECHALL, "Your air skirt is damaged!!");
					LowerMaxSpeed(mech, MP1);
					break;
				case 9:
				case 10:
				case 11:
				case 12:
                                        mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
					mech_notify(mech, MECHALL, "Your air skirt is heavily damaged!");
					LowerMaxSpeed(mech, MP2);
					break;
				}
		} else if (mudconf.btech_fasacrit < 0)
		    MotiveCritRoll(mech, att);
		return LSIDE;
	    case 4:
		return LSIDE;
	    case 5:
		return FSIDE;
	    case 6:
	    case 7:
	    case 8:
		return LSIDE;
	    case 9:
		return BSIDE;
	    case 10:
	    case 11:
		return (GetSectOInt(mech, TURRET)) ? TURRET : LSIDE;
	    case 12:
		if (GetSectOInt(mech, TURRET)) {
		    if (crittable(mech, att, TURRET, mudconf.btech_critlevel))
			*iscritical = 1;
		    return TURRET;
		} else {
		    if (crittable(mech, att, LSIDE, mudconf.btech_critlevel))
			*iscritical = 1;
		    return LSIDE;
		}
	    }
	    break;
	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
		if (crittable(mech, att, RSIDE, mudconf.btech_critlevel))
		    *iscritical = 1;
		return RSIDE;
	    case 3:
                if (mudconf.btech_fasacrit >= 0 && MechMove(mech) == MOVE_HOVER) {
			mech_notify(mech, MECHALL,
			    "Your are violently bounced around in your cockpit!");
			mech_notify(mech, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(mech, (MechSpeed(mech) / MP4))) {
			    mech_notify(mech, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(mech, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(mech, 1);
			}
                        switch(Number(2,12))
                                {
                                case 2:
                                        *iscritical = 1;
                                        break;
      				case 3:
				case 4:
                                case 5:
                                case 6:
                                case 7:
                                case 8:
                                        mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
                                        mech_notify(mech, MECHALL, "Your air skirt is damaged!!");
                                        LowerMaxSpeed(mech, MP1);
                                        break;
				case 9:
                                case 10:
                                case 11:
                                case 12:
                                        mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
                                        mech_notify(mech, MECHALL, "Your air skirt is heavily damaged!");
                                        LowerMaxSpeed(mech, MP2);
                                        break;
                                }
		} else if (mudconf.btech_fasacrit < 0)
		    MotiveCritRoll(mech, att);
		return RSIDE;
	    case 4:
		return RSIDE;
	    case 5:
		return FSIDE;
	    case 6:
	    case 7:
	    case 8:
		return RSIDE;
	    case 9:
		return BSIDE;
	    case 10:
	    case 11:
		return (GetSectOInt(mech, TURRET)) ? TURRET : RSIDE;
	    case 12:
		if (GetSectOInt(mech, TURRET)) {
		    if (crittable(mech, att, TURRET, mudconf.btech_critlevel))
			*iscritical = 1;
		    return TURRET;
		} else {
		    if (crittable(mech, att, RSIDE, mudconf.btech_critlevel))
			*iscritical = 1;
		    return RSIDE;
		}
	    }
	    break;

	case FRONT:
	case BACK:
	    side = (hitGroup == FRONT ? FSIDE : BSIDE);
	    switch (roll) {
	    case 2:
		if (crittable(mech, att, FSIDE, mudconf.btech_critlevel))
		    *iscritical = 1;
		return side;
	    case 3:
                if (mudconf.btech_fasacrit >= 0 && MechMove(mech) == MOVE_HOVER) {
			mech_notify(mech, MECHALL,
			    "Your are violently bounced around in your cockpit!");
			mech_notify(mech, MECHALL,
			    "You attempt to avoid personal injury!");
			if (MadePilotSkillRoll(mech, (MechSpeed(mech) / MP4))) {
			    mech_notify(mech, MECHALL,
				"Your fancy moves worked!");
			} else {
			    mech_notify(mech, MECHALL,
				"You cute fancy moves failed....");
			    headhitmwdamage(mech, 1);
			}
                        switch(Number(2,12))
                                {
                                case 2:
                                case 3:
                                        *iscritical = 1;
                                        break;
				case 4:
                                case 5:
                                case 6:
                                case 7:
				case 8:
					mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
                                        mech_notify(mech, MECHALL, "Your air skirt is damaged!!");
					LowerMaxSpeed(mech, MP1);
                                        break;
				case 9:
                                case 10:
                                case 11:
                                case 12:
                                        mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
                                        mech_notify(mech, MECHALL, "Your air skirt is heavily damaged!");
                                        LowerMaxSpeed(mech, MP2);
                                        break;
                                }
                        } else if (mudconf.btech_fasacrit < 0)
			    MotiveCritRoll(mech, att);
		return side;
	    case 4:
		return side;
	    case 5:
		return (hitGroup == FRONT ? RSIDE : LSIDE);
	    case 6:
	    case 7:
	    case 8:
		return side;
	    case 9:
		return (hitGroup == FRONT ? LSIDE : RSIDE);
	    case 10:
	    case 11:
		return (GetSectOInt(mech, TURRET)) ? TURRET : side;
		case 12:
		    if (GetSectOInt(mech, TURRET)) {
			if (crittable(mech, att, TURRET, mudconf.btech_critlevel))
			    *iscritical = 1;
		        return TURRET;
		    } else {
			if (crittable(mech, att, side, mudconf.btech_critlevel))
				*iscritical =1;
		        return side;
	    	    }
	    }
	}
	break;

    case CLASS_AERO:
	if (att && att != mech)
	    if (((ZB = FindZBearing(MechFX(att), MechFY(att), MechFZ(att), MechFX(mech), MechFY(mech), MechFZ(mech))) < -45) ||
	         (ZB > 45)) {
		switch (roll) {
		    case 2:
		    case 4:
		    case 5:
		    case 7:
			return AERO_NOSE;
		    case 3:
		    case 6:
		    case 8:
		    case 11:
			if (hitGroup == LEFTSIDE)
			    return AERO_LWING;
			else
			    return AERO_RWING;
		    case 9:
		    case 10:
		    case 12:
			return AERO_AFT;
		    default:
			return AERO_AFT;
		    }
		}
        switch (hitGroup) {
        case FRONT:
            switch (roll) {
            case 2:
            case 3:
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
                return AERO_NOSE;
            case 4:
            case 5:
                return AERO_RWING;
            case 9:
            case 10:
                return AERO_LWING;
            }
            break;
        case LEFTSIDE:
        case RIGHTSIDE:
            side = ((hitGroup == LEFTSIDE) ? AERO_LWING : AERO_RWING);
            switch (roll) {
            case 2:
            case 4:
            case 5:
                return AERO_NOSE;
            case 3:
            case 6:
            case 7:
            case 8:
            case 11:
                return side;
            case 9:
            case 10:
            case 12:
                return AERO_AFT;
            }
            break;
        case BACK:
            switch (roll) {
            case 2:
            case 3:
            case 6:
            case 7:
            case 8:
            case 11:
            case 12:
                return AERO_AFT;
            case 4:
            case 5:
                return AERO_RWING;
            case 9:
            case 10:
                return AERO_LWING;
            }
            break;
        }
	break;
    case CLASS_DS:
    case CLASS_SPHEROID_DS:
        if (att && att != mech)
            if (((ZB = FindZBearing(MechFX(att), MechFY(att), MechFZ(att), MechFX(mech), MechFY(mech), MechFZ(mech))) < -45) ||
                 (ZB > 45)) {
                switch (roll) {
		    case 2:
		    case 3:
		    case 4:
			return DS_NOSE;
		    case 10:
		    case 11:
		    case 12:
			return DS_AFT;
		    case 5:
		    case 6:
		    case 7:
		    case 8:
		    case 9:
                        if (hitGroup == LEFTSIDE)
                            return DS_LWING;
                        else
                            return DS_RWING;
                    }
                }
	switch (hitGroup) {
	case FRONT:
	    switch (roll) {
	    case 2:
	    case 3:
	    case 6:
	    case 7:
	    case 8:
	    case 11:
	    case 12:
		return DS_NOSE;
	    case 4:
	    case 5:
		return DS_RWING;
	    case 9:
	    case 10:
		return DS_LWING; 
	    }
	case LEFTSIDE:
	case RIGHTSIDE:
	    side = (hitGroup == LEFTSIDE) ? DS_LWING : DS_RWING;
	    switch (roll) {
	    case 2:
	    case 3:
	    case 4:
		return DS_NOSE;
	    case 10:
	    case 11:
	    case 12:
		return DS_AFT;
	    case 5:
	    case 6:
	    case 7:
	    case 8:
	    case 9:
		return side;
	    }
	case BACK:
	    switch (roll) {
	    case 2:
	    case 3:
	    case 6:
	    case 7:
	    case 8:
	    case 11:
	    case 12:
		return DS_AFT;
	    case 4:
	    case 5:
		return DS_RWING;
	    case 9:
	    case 10:
		return DS_LWING;
	    }
	}
	break;
    case CLASS_VEH_VTOL:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		hitloc = LSIDE;
		*iscritical = 1;
		break;
	    case 3:
	    case 4:
		hitloc = LSIDE;
		break;
	    case 5:
		hitloc = FSIDE;
		break;
	    case 6:
	    case 7:
	    case 8:
		hitloc = LSIDE;
		break;
	    case 9:
		hitloc = BSIDE;
		break;
	    case 10:
	    case 11:
/*	    case 12:*/
		hitloc = ROTOR;
/*		*iscritical = 1; */
		break;
	    case 12:
		hitloc = ROTOR;
		*iscritical = 1;
		break;
	    }
	    break;

	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
		hitloc = RSIDE;
		*iscritical = 1;
		break;
	    case 3:
	    case 4:
		hitloc = RSIDE;
		break;
	    case 5:
		hitloc = FSIDE;
		break;
	    case 6:
	    case 7:
	    case 8:
		hitloc = RSIDE;
		break;
	    case 9:
		hitloc = BSIDE;
		break;
	    case 10:
	    case 11:
/*	    case 12:*/
		hitloc = ROTOR;
/*		*iscritical = 1;*/
		break;
	    case 12:
		hitloc = ROTOR;
		*iscritical = 1;
		break;
	    }
	    break;

	case FRONT:
	case BACK:
	    side = (hitGroup == FRONT ? FSIDE : BSIDE);
	    switch (roll) {
	    case 2:
		hitloc = side;
		*iscritical = 1;
		break;
	    case 3:
		hitloc = side;
		break;
	    case 4:
		hitloc = side;
		break;
	    case 5:
		hitloc = (hitGroup == FRONT ? RSIDE : LSIDE);
		break;
	    case 6:
	    case 7:
	    case 8:
		hitloc = side;
		break;
	    case 9:
		hitloc = (hitGroup == FRONT ? LSIDE : RSIDE);;
		break;
	    case 10:
	    case 11:
/*	    case 12:*/
		hitloc = ROTOR;
/*		*iscritical = 1;*/
		break;
	    case 12:
		hitloc = ROTOR;
		*iscritical = 1;
		break;
	    }
	    break;
	}
	break;
    case CLASS_VEH_NAVAL:
	switch (hitGroup) {
	case LEFTSIDE:
	    switch (roll) {
	    case 2:
		hitloc = LSIDE;
		if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
		    *iscritical = 1;
		break;
	    case 3:
	    case 4:
	    case 5:
		hitloc = LSIDE;
		break;
	    case 9:
		hitloc = LSIDE;
		break;
	    case 10:
		if (GetSectInt(mech, TURRET))
		    hitloc = TURRET;
		else
		    hitloc = LSIDE;
		break;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    hitloc = TURRET;
		    if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
			*iscritical = 1;
		} else
		    hitloc = LSIDE;
		break;
	    case 12:
		hitloc = LSIDE;
		*iscritical = 1;
		break;
	    }
	    break;

	case RIGHTSIDE:
	    switch (roll) {
	    case 2:
	    case 12:
		hitloc = RSIDE;
		if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
		    *iscritical = 1;
		break;
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
		hitloc = RSIDE;
		break;
	    case 10:
		if (GetSectInt(mech, TURRET))
		    hitloc = TURRET;
		else
		    hitloc = RSIDE;
		break;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    hitloc = TURRET;
		    if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
			*iscritical = 1;
		} else
		    hitloc = RSIDE;
		break;
	    }
	    break;

	case FRONT:
	case BACK:
	    switch (roll) {
	    case 2:
	    case 12:
		hitloc = FSIDE;
		if (crittable(mech, att, hitloc, mudconf.btech_critlevel))
		    *iscritical = 1;
		break;
	    case 3:
		hitloc = FSIDE;
		break;
	    case 4:
		hitloc = FSIDE;
		break;
	    case 5:
		hitloc = FSIDE;
		break;
	    case 6:
	    case 7:
	    case 8:
	    case 9:
		hitloc = FSIDE;
		break;
	    case 10:
		if (GetSectInt(mech, TURRET))
		    hitloc = TURRET;
		else
		    hitloc = FSIDE;
		break;
	    case 11:
		if (GetSectInt(mech, TURRET)) {
		    hitloc = TURRET;
		    *iscritical = 1;
		} else
		    hitloc = FSIDE;
		break;
	    }
	    break;
	}
	break;
    }
    return (hitloc);
}
