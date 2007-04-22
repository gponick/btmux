#include "mech.h"
#include "mech.events.h"
#include "p.map.conditions.h"
#include "p.mech.utils.h"
#include "p.mech.combat.h"
#include "p.artillery.h"
#include "p.btechstats.h"
#include "p.eject.h"
#include "p.mech.sensor.h"
#include "p.crit.h"

void alter_conditions(MAP * map)
{
    int i;
    MECH *mech;

    for (i = 0; i < map->first_free; i++)
	if ((mech = FindObjectsData(map->mechsOnMap[i]))) {
	    UpdateConditions(mech, map);
#if 0
	    mech_notify(mech, MECHALL,
		"You notice a great disturbance in the Force..");
#endif
	}
}

void map_setconditions(dbref player, MAP * map, char *buffer)
{
    char *args[5];
    int vacuum = -1, underground = -1, grav, temp, argc, cloudbase = 40;
    int fl;

    DOCHECK((argc =
	    mech_parseattributes(buffer, args, 5)) < 3,
	"(At least) 3 options required (gravity + temperature + cloudbase)");
    DOCHECK(argc > 5,
	"Too many options! Command accepts only 4 at max (gravity + temperature + cloudbase + vacuum-flag + underground-flag)");
    DOCHECK(Readnum(grav, args[0]),
	"Invalid gravity (must be integer in range of 0 to 255)");
    DOCHECK(grav < 0 ||
	grav > 255,
	"Invalid gravity (must be integer in range of 0 to 255)");
    DOCHECK(Readnum(temp, args[1]),
	"Invalid temperature (must be integer in range of -128 to 127");
    DOCHECK(temp < -128 ||
	temp > 127,
	"Invalid temperature (must be integer in range of -128 to 127");
    if (argc > 2) {
	DOCHECK(Readnum(cloudbase, args[2]),
	    "Invalid cloudbase flag (must be integer, between 50 and 255)");
	DOCHECK(cloudbase < 10 ||
	    cloudbase > 255, "Invalid cloudbase (must be integer, between 10 or 255)");
    }
    if (argc > 3) {
	DOCHECK(Readnum(vacuum, args[3]),
	    "Invalid vacuum flag (must be integer, 0 or 1)");
	DOCHECK(vacuum < 0 ||
	    vacuum > 1, "Invalid vacuum flag (must be integer, 0 or 1)");
    }
    if (argc > 4) {

	DOCHECK(Readnum(underground, args[4]),
	    "Invalid underground flag (must be integer, 0 or 1)");
	DOCHECK(underground < 0 || underground > 1,
	    "Invalid underground flag (must be integer, 0 or 1)");
    }
    fl = (map->flags & (~(MAPFLAG_SPEC | MAPFLAG_VACUUM)));
    if (vacuum > 0)
	fl |= MAPFLAG_VACUUM;
    if (underground > 0)
	fl |= MAPFLAG_UNDERGROUND;
    if (underground == 0)
	fl &= ~MAPFLAG_UNDERGROUND;
    if (fl & MAPFLAG_VACUUM)
	fl |= MAPFLAG_SPEC;
    if (temp < -30 || temp > 50 || grav != 100)
	fl |= MAPFLAG_SPEC;
    map->temp = temp;
    map->grav = grav;
    map->cloudbase = cloudbase;
    map->flags = fl;
    notify(player, "Conditions set!");
    alter_conditions(map);
}

void UpdateConditions(MECH * mech, MAP * map)
{
    if (!mech)
	return;
    MechStatus(mech) &= ~CONDITIONS;
    if (!map)
	return;
    if (!MapUnderSpecialRules(map))
	return;
    MechStatus(mech) |= UNDERSPECIAL;
    if (MapTemperature(map) < -30 || MapTemperature(map) > 50)
	MechStatus(mech) |= UNDERTEMPERATURE;
    if (MapGravity(map) != 100)
	MechStatus(mech) |= UNDERGRAVITY;
    if (MapIsVacuum(map))
	MechStatus(mech) |= UNDERVACUUM;
}

extern int doing_explode;

void DestroyParts(MECH * attacker, MECH * wounded, int hitloc, int breach,
    int IsDisable)
{
    float oldjs;
    int i;
    int critType;
    int nhs = 0, LOS = 0;
    int destedsect = !GetSectInt(wounded, hitloc);
    int z, dam;
    MAP *map;

    if (!(MechType(wounded) == CLASS_MECH || MechType(wounded) == CLASS_MW
	    || MechType(wounded) == CLASS_BSUIT)) {
	for (i = 0; i < CritsInLoc(wounded, hitloc); i++)
	    if (GetPartType(wounded, hitloc, i) &&
		!PartIsDestroyed(wounded, hitloc, i)) {
		if (IsDisable == 1)
		    DisablePart(wounded, hitloc, i);
		else
		    DestroyPart(wounded, hitloc, i);
	    }
	return;
    }
    oldjs = MechJumpSpeed(wounded);
    for (i = 0; i < CritsInLoc(wounded, hitloc); i++) {
	    critType = GetPartType(wounded, hitloc, i);
	    if (IsSpecial(critType) && !PartIsNonfunctional(wounded, hitloc, i) && !IsCrap(critType)) {
		switch (Special2I(critType)) {
		case UPPER_ACTUATOR:
		case LOWER_ACTUATOR:
		case HAND_OR_FOOT_ACTUATOR:
		    if ((hitloc == RLEG || hitloc == LLEG) || ((hitloc == RARM || hitloc == LARM) && MechMove(wounded) == MOVE_QUAD)) {
			if (!PartIsNonfunctional(wounded, hitloc, i))
				MechPilotSkillBase(wounded) += 1;

			if (!destedsect && !IsDisable && !MadePilotSkillRoll(wounded, 0)) {
			    mech_notify(wounded, MECHALL,
				"You lose your balance and fall down!");
			    MechLOSBroadcast(wounded,
				"loses balance and falls down!");
			    MechFalls(wounded, 1, 0);
			}
		    }
		    break;
		case SHOULDER_OR_HIP:
		    if ((hitloc == RLEG || hitloc == LLEG) || ((hitloc == RARM || hitloc == LARM) && MechMove(wounded) == MOVE_QUAD)) {
			if (!PartIsNonfunctional(wounded, hitloc, i))
				MechPilotSkillBase(wounded) += 2;

			if (!destedsect && !IsDisable && !MadePilotSkillRoll(wounded, 0)) {
			    mech_notify(wounded, MECHALL,
				"You lose your balance and fall down!");
			    MechLOSBroadcast(wounded,
				"loses balance and falls down!");
			    MechFalls(wounded, 1, 0);
			}
		    }
		    break;
		case HEAT_SINK:
		    if (MechSpecials(wounded) & DOUBLE_HEAT_TECH) {
			if ((nhs++) % 3 == 2)
			    MechRealNumsinks(wounded)++;
		    }
		    if (MechSpecials2(wounded) & COMPACT_HEAT_TECH)
			MechRealNumsinks(wounded) -= 2;
		      else
			MechRealNumsinks(wounded)--;
		    break;
		case JUMP_JET:
		    MechJumpSpeed(wounded) -= MP1;
		    if (MechJumpSpeed(wounded) < 0)
			MechJumpSpeed(wounded) = 0;
		    if (attacker && MechJumpSpeed(wounded) == 0 &&
			Jumping(wounded)) {
			mech_notify(wounded, MECHALL,
			    "Losing your last Jump Jet you fall from the sky!!!!!");
			if (LOS && wounded != attacker)
			    mech_notify(attacker, MECHALL,
				"Your hit knocks the 'Mech from the sky");
			MechLOSBroadcast(wounded, "falls from the sky!");
			MechFalls(wounded, (int) (oldjs * MP_PER_KPH), 0);
			domino_space(wounded, 2);
		    }
		    break;
		case ENGINE:
		    if (MechEngineHeat(wounded) < 10)
			MechEngineHeat(wounded) += 5;
		    else if (MechEngineHeat(wounded) < 15) {
			MechEngineHeat(wounded) = 15;
			if (attacker) {
			    mech_notify(wounded, MECHALL,
				"Your engine is destroyed!!");
			    if (wounded != attacker)
				mech_notify(attacker, MECHALL,
				    "You destroy the engine!!");
/* SendDebug(tprintf("#%d has been killed by #%d", wounded->mynum, attacker->mynum)); */
			    DestroyMech(wounded, attacker ? attacker : wounded, 1);
			    if (mudconf.btech_stackpole &&
				(MechBoomStart(wounded) + MAX_BOOM_TIME)
				>= event_tick && Roll() >= BOOM_BTH
				&& !doing_explode && (Started(wounded) || Starting(wounded))) {
				z = MechZ(wounded);
				map = FindObjectsData(wounded->mapindex);
				HexLOSBroadcast(map, MechX(wounded),
				    MechY(wounded),
				    "%ch%crThe hit destroys last safety systems, releasing the fusion reaction!%cn");
				if (!MapIsUnderground(map))
				    autoeject(MechPilot(wounded), wounded, attacker);
				DestroySection(wounded, attacker, LOS,
				    LTORSO);
				DestroySection(wounded, attacker, LOS,
				    RTORSO);
				DestroySection(wounded, attacker, LOS,
				    CTORSO);
				/* Need to autoeject before the explosion reaches the head -CM */
				/* Also need to track kill. Autoeject set dest, DestMech checked for !Dest - DJ */
				DestroySection(wounded, attacker, LOS,
				    HEAD);
/*	Bah. Smoking boots are dramatic while leaving minimal functional salvage. I favor effect over canon-FASA - DJ */
/*				DestroySection(wounded, attacker, LOS, LLEG);	//Added legs to destruction during explosion. According to real  */
/*				DestroySection(wounded, attacker, LOS, RLEG);	//rules the mech turns int a pile of sludge... not even smoking boots. */
				DestroySection(wounded, attacker, LOS, LLEG);
				DestroySection(wounded, attacker, LOS, RLEG);
				MechZ(wounded) += 6;
				dam =
				    MAX(MechTons(wounded) / 10,
				    MechEngineSize(wounded) / 25);

				/*
				 * Added in IR and LA blinding during a stackpole
				 * -Kipsta
				 * 8/4/99
				 */

				ScrambleInfraAndLiteAmp(wounded, 4, 0,
				    "The blinding flash of light blinds you!",
				    "The blinding flash of light blinds you!");

				blast_hit_hexesf(map, dam, 3,
				    MAX(MechTons(wounded) / 10,
					MechEngineSize(wounded) / 25),
				    MechFX(wounded), MechFY(wounded),
				    MechFX(wounded), MechFY(wounded),
				    "%ch%crYou bear full brunt of the blast!%cn",
				    "is hit badly by the blast!",
				    "%ch%cyYou receive some damage from the blast!%cn",
				    "is hit by the blast!",
				    mudconf.btech_engine > 1, 3, 5, 1, 2, NULL, NULL);
				MechZ(wounded) = z;
				headhitmwdamage(wounded, 4);
			    }
			}
			DestroyMech(wounded, attacker, 1);
		    }
		    break;
		case NULLSIG:
		    if (!(MechCritStatus(wounded) & NULLSIG_DESTROYED)) {
			if (attacker)
			    mech_notify(wounded, MECHALL,
				"Part of your Null Signature Device is damaged and knocked out!");
			MechCritStatus(wounded) |= NULLSIG_DESTROYED;
			MechStatus2(wounded) &= ~NULLSIG_ACTIVE;
		    } else {
			mech_notify(wounded, MECHALL,
				"Your Null Signature system is further damaged!");
		    }
		    break;
		case TARGETING_COMPUTER:
		    if (!(MechCritStatus(wounded) & TC_DESTROYED)) {
			if (attacker)
			    mech_notify(wounded, MECHALL,
				"Your Targeting Computer is Destroyed!");
			MechCritStatus(wounded) |= TC_DESTROYED;
		    }
		    break;
		default:
		    HandleMechCrit(wounded, attacker, LOS, hitloc, i, critType, GetPartData(wounded, hitloc, i), 0, 0);
		    break;
		}
	      }
        if (!PartIsDestroyed(wounded, hitloc, i) && !IsCrap(critType)) {
            if (IsDisable == 1)
                DisablePart(wounded, hitloc, i);
            else if (PartIsDisabled(wounded, hitloc, i)) {
                DestroyPart(wounded, hitloc, i);
            } else
                DestroyPart(wounded, hitloc, i);
	}
    }
    if (MechType(wounded) == CLASS_MECH || MechType(wounded) == CLASS_MW) {
	if (breach && ((hitloc == HEAD && !(MechSpecials2(wounded) & TORSOCOCKPIT_TECH)) ||
		       (hitloc == CTORSO && (MechSpecials2(wounded) & TORSOCOCKPIT_TECH)))) {
	    if (InVacuum(wounded))	//Changed to check for vacuum. Kipsta. 8/4/99
		mech_notify(wounded, MECHALL,
		    "You are exposed to vacuum!");
	    else
		mech_notify(wounded, MECHALL,
		    "Water floods into your cockpit!");

	    KillMechContentsIfIC(wounded->mynum, wounded);
	    DestroyMech(wounded, attacker, 0);
	    return;
	}
	if (MechMove(wounded) != MOVE_QUAD)
	    if (hitloc == LARM || hitloc == RARM)
		return;
	if (hitloc == RLEG || hitloc == LLEG || hitloc == LARM ||
	    hitloc == RARM) {
	    if (!(MechCritStatus(wounded) & LEG_DESTROYED)) {
		MechCritStatus(wounded) |= LEG_DESTROYED;
		MechCritStatus2(wounded) &= ~(EVADING|SPRINTING);
		if (MechType(wounded) == CLASS_MECH)
			if (MechMove(wounded) == MOVE_QUAD)
				{
				StopLateral(wounded);
				MechLateral(wounded) = 0;
				MechPilotSkillBase(wounded) -= 5;
				CalcLegCritSpeed(wounded);
				} else {
			     /* SetMaxSpeed(wounded, MP1); */
				CalcLegCritSpeed(wounded);
				}
			else
		    DivideMaxSpeed(wounded, 3);
		if (breach && InVacuum(wounded))
		    return;
		if (attacker) {
		    if (!Fallen(wounded) && !Jumping(wounded) &&
			!OODing(wounded)) {
			if (MechMove(wounded) != MOVE_QUAD) {
			    mech_notify(wounded, MECHALL,
				"You fall down with only one leg!!");
			    MechLOSBroadcast(wounded,
				"falls down with only one leg!");
			    MechFalls(wounded, 1, 0);
			} else {
			    mech_notify(wounded, MECHALL,
				"You fall down, unbalanced by loss of a leg!!");
			    MechLOSBroadcast(wounded,
				"falls down, unbalanced by the loss of a leg!");
			    MechFalls(wounded, 1, 0);
			}
		    }
		}
	    } else {
		if (MechCritStatus(wounded) & NO_LEGS)
		    return;
		if (MechMove(wounded) == MOVE_QUAD)
			{
			if (MechNumLegs(wounded) == 2) {
			     /* SetMaxSpeed(wounded, MP1); */
			        CalcLegCritSpeed(wounded);
			} else {
				MechCritStatus(wounded) |= NO_LEGS;
				MechCritStatus2(wounded) &= ~(EVADING|SPRINTING);
			     /* SetMaxSpeed(wounded, 0); */
				CalcLegCritSpeed(wounded);
			     }
			} else {
			MechCritStatus(wounded) |= NO_LEGS;
			MechCritStatus2(wounded) &= ~(EVADING|SPRINTING);
		     /* SetMaxSpeed(wounded, 0); */
			CalcLegCritSpeed(wounded);
			}


		if (breach && InVacuum(wounded))
		    return;
		if (attacker) {
		    if (!Fallen(wounded) && !Jumping(wounded) &&
			!OODing(wounded)) {
			if (MechMove(wounded) != MOVE_QUAD) {
			    mech_notify(wounded, MECHALL,
				"You fall down with no legs!!");
			    MechLOSBroadcast(wounded,
				"falls down, legless!");
			    MechFalls(wounded, 1, 0);
			} else {
			    mech_notify(wounded, MECHALL,
				"You fall down with only 2 legs remaining!!");
			    MechLOSBroadcast(wounded, "falls down!");
			    MechFalls(wounded, 1, 0);
			}
		    }
		    StopStand(wounded);
		}
	    }
	}
    }
    if (breach)
	if (MechType(wounded) == CLASS_VEH_GROUND ||
	    MechType(wounded) == CLASS_VEH_NAVAL) DestroyMech(wounded,
		attacker, 0);
}

int BreachLoc(MECH * attacker, MECH * mech, int hitloc)
{
    char buf[SBUF_SIZE];

    if (!InSpecial(mech))
	return 0;
    if (!InVacuum(mech))
	return 0;
    if (SectIsDestroyed(mech, hitloc) || SectIsBreached(mech, hitloc))
	return 0;
    ArmorStringFromIndex(hitloc, buf, MechType(mech), MechMove(mech));
    mech_notify(mech, MECHALL, tprintf("Your %s has been breached!", buf));
    SetSectBreached(mech, hitloc);
    DestroyParts(attacker, mech, hitloc, 1, 1);
    return 1;
}

int PossiblyBreach(MECH * attacker, MECH * mech, int hitloc)
{
    if (!InSpecial(mech))
	return 0;
    if (Roll() < 10)
	return 0;
    return BreachLoc(attacker, mech, hitloc);
}
