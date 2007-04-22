/* Main idea:
   - Use the cheat-variables on MECHstruct to determine when / if
   to do LOS checks (num_mechs_seen)
   - Also, check for range (maxgunrange / etc variables in the
   autopilot struct)

   - If we have target(s):
   - Try to acquire a target with best BTH
   - Decide if it's worth shooting at
   - If yes, try to acquire it, if not possible, check other targets,
   repeat until all guns fired (/ tried to fire)
 */

#include "mech.h"
#include "mech.events.h"
#include "autopilot.h"
#include "mech.sensor.h"
#include "p.mech.sensor.h"
#include "p.mech.utils.h"
#include "p.mech.physical.h"
#include "p.mech.combat.h"
#include "p.mech.advanced.h"
#include "p.bsuit.h"
#include "p.glue.h"

#define AUTOGUN_TICK 1		/* Every 3 secs */
#define AUTOGS_TICK  30		/* How oft to recheck sensors */
#define MAXHEAT      6		/* Last heat we let heat go to */
#define MAX_TARGETS  100

int global_kill_cheat = 0;

static char *my2string(const char * old)
{
    static char new[64];
    strncpy(new, old, 63);
    new[63] = '\0';
    return new;
}

int SearchLightInRange(MECH * mech, MAP *map)
{
MECH *t;
int i;

if (!mech || !map)
    return 0;

for (i = 0; i < map->first_free; i++) {
    if (!(t = FindObjectsData(map->mechsOnMap[i])))
        continue;
    if (!(MechSpecials(t) & SLITE_TECH) || MechCritStatus(mech) & SLITE_DEST)
        continue;
    if (FaMechRange(t, mech) < LITE_RANGE) {
        /* Returning true, but let's differentiate also between being in-arc. */
        if ((MechStatus(t) & SLITE_ON) && InWeaponArc(t, MechFX(mech), MechFY(mech)) & FORWARDARC) {
            if (!(map->LOSinfo[t->mapnumber][mech->mapnumber] & MECHLOSFLAG_BLOCK))
                /* Slite on and, arced, and LoS to you */
                return 3;
            else
                /* Slite on, arced, but LoS blocked */
                return 4;
            } else if (!MechStatus(t) & SLITE_ON && InWeaponArc(t, MechFX(mech), MechFY(mech)) & FORWARDARC) {
                if (!(map->LOSinfo[t->mapnumber][mech->mapnumber] & MECHLOSFLAG_BLOCK))
                    /* Slite off, arced, and LoS to you */
                    return 5;
                else
                    /* Slite off, arced, and LoS blocked */
                    return 6;
            }
        /* Slite is in range of you, but apparently not arced on you. Return tells wether on or off */
        return (MechStatus(t) & SLITE_ON ? 1 : 2);
        }

    }
return 0;
}

int PrefVisSens(MECH * mech, MAP * map, int slite, MECH * target)
{
if (!mech || !map)
    return SENSOR_VIS;

if (MechStatus(mech) & SLITE_ON || MechCritStatus(mech) & SLITE_LIT)
    return SENSOR_VIS;
if (target && MechCritStatus(target) & SLITE_LIT)
    return SENSOR_VIS;

if (map->maplight <= 1 && slite != 3 && slite != 5)
    return SENSOR_LA;

return SENSOR_VIS;
} 

void auto_gun_sensor_event(EVENT * e)
{
    AUTO *a = (AUTO *) e->data;
    MECH *mech = (MECH *) a->mymech;
    MECH *target = NULL;
    MAP *map;
    int flag = (int) e->data2;
    char buf[16];
    int wanted_s[2];
    int rvis;
    int slite, prefvis;
    float trng;
    int set = 0;

    if (!IsMech(mech->mynum) || !IsAuto(a->mynum))
	return;
    if (Destroyed(mech)) {
	DoStopGun(a);
	return;
    }
    if (!Started(mech)) {
	Zombify(a);
	return;
    }
    if (a->flags & AUTOPILOT_LSENS)
	return;
    map = getMap(mech->mapindex);
    if (!map) {
	Zombify(a);
	return;
    }
    if (MechTarget(mech) > 0)
	target = getMech(MechTarget(mech));

    slite = (map->mapvis != 2 ? SearchLightInRange(mech, map) : 0);
    rvis = (map->maplight ? (map->mapvis) : (map->mapvis * (slite ? 1 : 3)));
    prefvis = PrefVisSens(mech, map, slite, target);

    if (target) {
	trng = FaMechRange(mech, target);
	if (!set && HeatFactor(target) > 35 && (int) trng < 15) {
	    wanted_s[0] = SENSOR_IR;
	    wanted_s[1] = ((MechTons(target) >= 60) ? SENSOR_EM : prefvis);
	    set++;
	    }
	if (!set && MechTons(target) >= 60 && (int) trng <= 20) {
	    wanted_s[0] = SENSOR_EM;
	    wanted_s[1] = SENSOR_IR;
	    set++;
	    }
	if (!set && !Landed(target) && (FlyingT(target) || Jumping(target) || OODing(target))) {
	    wanted_s[0] = SENSOR_RA;
	    wanted_s[0] = prefvis;
	    set++;
	    }
	if (!set && (int) trng <= 4 && MechSpecials(mech) & BEAGLE_PROBE_TECH && !(MechCritStatus(mech) & BEAGLE_DESTROYED)) {
	    wanted_s[0] = SENSOR_BP;
	    wanted_s[1] = SENSOR_BP;
	    set++;
	    }
	if (!set && (int) trng <= 8 && MechSpecials2(mech) & BLOODHOUND_PROBE_TECH && !(MechCritStatus(mech) & BEAGLE_DESTROYED)) {
	    wanted_s[0] = SENSOR_BL;
	    wanted_s[1] = SENSOR_BL;
	    set++;
	    } 
	if (!set) {
	    wanted_s[0] = prefvis;
	    wanted_s[1] = (rvis <= 15 ? SENSOR_EM : prefvis);
	    set++;
	    }
	
	}
    if (!set) {
	if (rvis <= 15) {
	    wanted_s[0] = SENSOR_EM;
	    wanted_s[1] = SENSOR_IR;
	} else {
	    wanted_s[0] = prefvis;
	    wanted_s[1] = prefvis;
        }
    }

//    if (!Sees360(mech))
//	wanted_s[1] = wanted_s[0];
    if (wanted_s[0] >= SENSOR_VIS && wanted_s[0] <= SENSOR_BL &&
	wanted_s[1] >= SENSOR_VIS && wanted_s[1] <= SENSOR_BL &&
	(MechSensor(mech)[0] != wanted_s[0] || MechSensor(mech)[1] != wanted_s[1])) {
        memset(buf, '\0', sizeof(char) * 16);
        sprintf(buf, "%c  %c", BOUNDED(SENSOR_VIS, sensors[wanted_s[0]].sensorname[0], SENSOR_BL),
                               BOUNDED(SENSOR_VIS, sensors[wanted_s[1]].sensorname[0], SENSOR_BL));
	mech_sensor(a->mynum, mech, buf);
    }
    if (!flag)
	AUTOEVENT(a, EVENT_AUTOGS, auto_gun_sensor_event, AUTOGS_TICK, 0);
}

int AverageWpnRange(MECH * mech)
{
int loop, count, i;
unsigned char weaparray[MAX_WEAPS_SECTION];
unsigned char weapdata[MAX_WEAPS_SECTION];
int critical[MAX_WEAPS_SECTION];
int tot_weap = 0, tot_rng = 0;

if (!mech)
    return 1;

for (loop = 0; loop < NUM_SECTIONS; loop++) {
    count = FindWeapons(mech, loop, weaparray, weapdata, critical);
    if (count <= 0)
	continue;
    for (i = 0; i < count; i++) {
	if (IsAMS(weaparray[i]))
	    continue;
#if 0
	if (PartIsDisabled(mech, loop, critical[i]) || PartIsDestroyed(mech, loop, critical[i]))
	    continue;
	if (GetMTXCrit(mech, loop, critical[i], 0))
	    continue;
#else
	if (WeaponIsNonfunctional(mech, loop, critical[i], GetWeaponCrits(mech, Weapon2I(weaparray[i]))) > 0)
	    continue;
#endif
	tot_weap++;
	tot_rng += MAX(EGunRange(weaparray[i], GetPartMode(mech, loop, critical[i])), 6);
	}
    
    }
return (tot_rng / tot_weap); 
}

int TargetScore(MECH * mech, MECH * target, int range)
{
/*
int avg_rng;
int bv;

if (!mech || !target)
    return 0;

bv = MechBV(target);
avg_rng = AverageWpnRange(mech); 

return (int) ((float) bv * ((float) 1.0 - ((float) range / (float) avg_rng)));
*/
return MechBV(target);
}

void auto_gun_event(EVENT * e)
{
    AUTO *a = (AUTO *) e->data;
    MECH *mech = (MECH *) a->mymech;
    MECH *targets[MAX_TARGETS], *t;
    int targetscore[MAX_TARGETS];
    int targetbth[MAX_TARGETS];
    int targetrange[MAX_TARGETS];
    int i, j, k, f;
    MAP *map;
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int weapnum = 0, ii, loop, target_count = 0, ttarget_count = 0, count;
    char buf[LBUF_SIZE];
    int b;
    int h;
    int rt = 0;
    int fired = 0;
    int locked = 0;
    float save = 0.0;
    int cheating = 0;
    int locktarg_num = -1;
    int bth, score;

    if (!IsMech(mech->mynum) || !IsAuto(a->mynum))
	return;

    if (Destroyed(mech)) {
	DoStopGun(a);
	return;
    }
    if (!Started(mech)) {
	Zombify(a);
	return;
    }
    if (!(map = getMap(mech->mapindex))) {
	Zombify(a);
	return;
    }

#if 0
    if (!MechNumSeen(mech)) {
	Zombify(a);
	return;
    }

    if (MechType(mech) == CLASS_MECH &&
	(MechPlusHeat(mech) - MechActiveNumsinks(mech)) > MAXHEAT) {
	AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, AUTOGUN_TICK, 0);
	return;
    }
#endif
    if (OODing(mech)) {
	AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, AUTOGUN_TICK, 0);
	return;
	}

    global_kill_cheat = 0;
    for (i = 0; i < map->first_free; i++)
	if (i != mech->mapnumber && (j = map->mechsOnMap[i]) > 0) {
	    if (!(t = getMech(j)))
		continue;
	    if (Destroyed(t))
		continue;
	    if (MechStatus(t) & COMBAT_SAFE)
		continue;
	    if (MechTeam(t) == MechTeam(mech) && t->mynum != a->targ)
		continue;
/*	    if (!(map->LOSinfo[mech-> mapnumber][t->mapnumber] & MECHLOSFLAG_SEEN))
		    continue; */
	    if ((targetrange[target_count] =
		    (int) FlMechRange(map, mech, t)) > 30)
		continue;
	    if (MechType(mech) == CLASS_BSUIT && MechSwarmTarget(mech) > 0 && MechSwarmTarget(mech) != t->mynum)
		continue;

	    ttarget_count++;
	    if (!(a->targ <= 0 || t->mynum == a->targ))
		continue;
	    if (t->mynum == MechTarget(mech))
		locktarg_num = target_count;
	    targets[target_count] = t;
	    if (MechType(mech) == CLASS_MECH && (targetrange[target_count] < 1.0)) {
		char ib[6], tb[10];
		int st_ra = SectIsDestroyed(mech, RARM);
		int st_la = SectIsDestroyed(mech, LARM);
		int st_ll = SectIsDestroyed(mech, LLEG);
		int st_rl = SectIsDestroyed(mech, RLEG);
		int iwa, iwa_nt, ts;

		snprintf(ib, sizeof(char) * 6, "%s", MechIDS(t, 0));
		ts = MechStatus(mech) & (TORSO_LEFT|TORSO_RIGHT);
		MechStatus(mech) &= ~ts;
		iwa_nt = InWeaponArc(mech, MechFX(t), MechFY(t));
		MechStatus(mech) |= ts;
		iwa = InWeaponArc(mech, MechFX(t), MechFY(t));

		/* Check for Upper Body Moves First */
	   	if (iwa & FORWARDARC && !SectHasBusyWeap(mech, RARM) && !MechSections(mech)[RARM].recycle && !st_ra &&
		 	!MechSections(mech)[RLEG].recycle && !MechSections(mech)[LLEG].recycle) {
		    sprintf(tb, "r %s", ib);
		    if (have_axe(mech, RARM))
		        mech_axe(a->mynum, mech, tb);
		    else if (have_mace(mech, RARM))
		        mech_bash(a->mynum, mech, tb);
		    else if (have_sword(mech, RARM))
		        mech_sword(a->mynum, mech, tb);
		    else if (MechRTerrain(mech) == HEAVY_FOREST || MechRTerrain(mech) == LIGHT_FOREST) {
		    	if (OkayCritSectS(RARM, 0, SHOULDER_OR_HIP) && OkayCritSectS(RARM, 0, HAND_OR_FOOT_ACTUATOR))
		    	    mech_club(a->mynum, mech, tb);
		    } else 
		        mech_punch(a->mynum, mech, tb);
		    } else if (iwa & FORWARDARC &&
				!SectHasBusyWeap(mech, LARM) && !MechSections(mech)[LARM].recycle && !st_la &&
		 		!MechSections(mech)[RLEG].recycle && !MechSections(mech)[LLEG].recycle) {
		    sprintf(tb, "l %s", ib);
                    if (have_axe(mech, LARM))
                        mech_axe(a->mynum, mech, tb);
                    else if (have_mace(mech, LARM))
                        mech_bash(a->mynum, mech, tb);
                    else if (have_sword(mech, LARM))
                        mech_sword(a->mynum, mech, tb);
                    else if (MechRTerrain(mech) == HEAVY_FOREST || MechRTerrain(mech) == LIGHT_FOREST) {
		    	if (OkayCritSectS(LARM, 0, SHOULDER_OR_HIP) && OkayCritSectS(LARM, 0, HAND_OR_FOOT_ACTUATOR))
                            mech_club(a->mynum, mech, tb);
                    } else
                        mech_punch(a->mynum, mech, tb);
		    }

		if ((MechMove(mech) == MOVE_QUAD || iwa_nt & FORWARDARC) &&
			!MechSections(mech)[RARM].recycle && !MechSections(mech)[LARM].recycle &&
			!MechSections(mech)[RLEG].recycle && !MechSections(mech)[LLEG].recycle &&
			!st_ll && !st_rl) {
		    if (!MechSections(mech)[RLEG].recycle) { 
			sprintf(tb, "r %s", ib);
		        mech_kick(a->mynum, mech, tb);
		    } else if (!MechSections(mech)[LLEG].recycle) {
			sprintf(tb, "l %s", ib);
			mech_kick(a->mynum, mech, tb);
			}
		    }
		} else if ((MechType(mech) == CLASS_BSUIT) && (targetrange[target_count] < 1.0)) {
		char tb[6];
		sprintf(tb, "%s", MechIDS(t, 0));
		if (MechJumpSpeed(mech) > 0)
		    bsuit_swarm(a->mynum, mech, tb);
		else
		    bsuit_attackleg(a->mynum, mech, tb);
	    	}
	target_count++;
	}

    if (a->flags & AUTOPILOT_ROAMMODE && target_count == 0 && a->commands[a->program_counter] != GOAL_ROAM) {
        auto_disengage(a->mynum, a, "");
        auto_delcommand(a->mynum, a, "-1");
        PG(a) = 0;
        auto_addcommand(a->mynum, a, tprintf("autogun"));
        auto_addcommand(a->mynum, a, tprintf("roam 0 0"));
        auto_engage(a->mynum, a, "");
    }

    if (a->flags & AUTOPILOT_SWARMCHARGE) {
        if (MechSwarmTarget(mech) > 0)
            a->flags &= ~AUTOPILOT_SWARMCHARGE;
        else
	    if (MechTarget(mech) > 0) {
		AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, AUTOGUN_TICK, 0);
		return;
		}
        }

    if (((event_tick % 4) != 0) || (MechType(mech) == CLASS_MECH && (MechPlusHeat(mech) - MechActiveNumsinks(mech)) > MAXHEAT)) {
	AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, AUTOGUN_TICK, 0);
	return;
    }
    MechNumSeen(mech) = ttarget_count;
    if (!target_count) {
	Zombify(a);
	return;
    }
    /* Then, we'll see about our guns.. */
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	count = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (count <= 0)
	    continue;
	for (ii = 0; ii < count; ii++) {
	    weapnum++;
	    if (IsAMS(weaparray[ii]))
		continue;
#if 0
	    /*if (PartIsNonfunctional(mech, loop, critical[ii]))*/
	    if (PartIsDisabled(mech, loop, critical[ii]) || PartIsDestroyed(mech, loop, critical[ii]))
		continue;
	    if (GetMTXCrit(mech, loop, critical[ii],0))
		continue;
#else
	    if (WeaponIsNonfunctional(mech, loop, critical[ii], GetWeaponCrits(mech, Weapon2I(weaparray[ii]))) > 0)
		continue;
#endif
	    if (weapdata[ii])
		continue;
	    if (MechType(mech) == CLASS_MECH &&
		(rt + GunStat(weaparray[ii], GetPartMode(mech, loop, critical[ii]), GUN_HEAT) +
		    MechPlusHeat(mech) - MechActiveNumsinks(mech) + MTX_HEAT_MOD(GetPartDamage(mech, loop, critical[ii]))) > MAXHEAT)
		continue;
	    /* Whee, it's not recycling or anything -> it's ready to fire! */
	    for (i = 0; i < target_count; i++) {
		if (EGunRange(weaparray[ii], GetPartMode(mech, loop, critical[ii])) > targetrange[i]) {
		    score = TargetScore(mech, targets[i], targetrange[i]);
		    bth = FindNormalBTH(mech, map, loop, critical[ii], weaparray[ii], (float) targetrange[i], targets[i], 1000, 0);
		    targetscore[i] = (score / (MAX(1, bth)));
		    targetbth[i] = bth;
/* SendDebug(tprintf("TargetScoring - #%d to #%d scores %d (BV %d BTH %d)", mech->mynum, targets[i]->mynum, targetscore[i], score, bth)); */
		} else {
		    targetscore[i] = 1; /* 999 */
		    targetbth[i] = 20;
		    }
		}
	    for (i = 0; i < (target_count - 1); i++)
		for (j = i + 1; j < target_count; j++)
		    if (targetscore[i] > targetscore[j]) {
			if (locktarg_num == i)
			    locktarg_num = j;
			else if (locktarg_num == j)
			    locktarg_num = i;
			t = targets[i];
			k = targetscore[i];
			f = targetrange[i];
			targets[i] = targets[j];
			targetscore[i] = targetscore[j];
			targetrange[i] = targetrange[j];
			targets[j] = t;
			targetscore[j] = k;
			targetrange[j] = f;
		    }
	    for (i = 0; i < target_count; i++) {
		/* This is .. simple, for now: We don't bother with 10+/12+
		   BTHs (depending on if locked or not) */
		/* Modified to account for BV over BTH - DJ */
		if (locktarg_num >= 0 && i > locktarg_num)
		    break;
		if (targetbth[i] > 13 || targetrange[i] > EGunRange(weaparray[ii], GetPartMode(mech, loop, critical[ii])))
		    continue;
	    	if (MechTeam(mech) == MechTeam(targets[i]) && !IsCoolant(weaparray[ii]))
		    continue; 
		if (!IsInWeaponArc(mech, MechFX(targets[i]),
			MechFY(targets[i]), MechFZ(targets[i]), loop, critical[ii])) {
		    b =
			FindBearing(MechFX(mech), MechFY(mech),
			MechFX(targets[i]), MechFY(targets[i]));
		    if (MechType(mech) == CLASS_MECH) {
			h = MechFacing(mech);
			if (GetPartMode(mech, loop,
				critical[ii]) & REAR_MOUNT) h -= 180;
			h = AcceptableDegree(h);
			h -= b;
			if (h > 180)
			    h -= 360;
			if (h < -180)
			    h += 360;
			if (abs(h) > 120) {
			    /* Not arm weapon and not fliparm'able */
			    if ((loop != LARM && loop != RARM) ||
				!MechSpecials(mech) & FLIPABLE_ARMS)
				continue;
			    /* Woot. We can [possibly] fliparm to aim at foe */
			    if (MechStatus(mech) & (TORSO_LEFT |
				    TORSO_RIGHT))
				    mech_rotatetorso(a->mynum, mech,
				    my2string("center"));
			    if (!(MechStatus(mech) & FLIPPED_ARMS))
				mech_fliparms(a->mynum, mech, my2string(""));
			} else {
			    if (abs(h) < 60) {
				if (MechStatus(mech) & (TORSO_LEFT |
					TORSO_RIGHT))
					mech_rotatetorso(a->mynum, mech,
					my2string("center"));
			    } else if (h < 0) {
				if (!(MechStatus(mech) & TORSO_RIGHT)) {
				    if (MechStatus(mech) & (TORSO_LEFT |
					    TORSO_RIGHT))
					    mech_rotatetorso(a->mynum,
					    mech, my2string("center"));
				    mech_rotatetorso(a->mynum, mech,
					my2string("right"));
				}
			    } else {
				if (!(MechStatus(mech) & TORSO_LEFT)) {
				    if (MechStatus(mech) & (TORSO_LEFT |
					    TORSO_RIGHT))
					    mech_rotatetorso(a->mynum,
					    mech, my2string("center"));
				    mech_rotatetorso(a->mynum, mech,
					my2string("left"));
				}
			    }
			    if (MechStatus(mech) & FLIPPED_ARMS)
				mech_fliparms(a->mynum, mech, my2string(""));
			}
		    } else {
			/* Do we have a turret? */
			if (MechType(mech) == CLASS_MECH ||
			    MechType(mech) == CLASS_MW ||
			    MechType(mech) == CLASS_BSUIT || is_aero(mech)
			    || !GetSectInt(mech, TURRET))
			    continue;
			/* Hrm, is the gun on turret? */
			if (loop != TURRET)
			    continue;
			/* We've a turret to turn! Whoopee! */
			sprintf(buf, "%d", b);
			mech_turret(a->mynum, mech, buf);
		    }
		}
		if (MechTarget(mech) != targets[i]->mynum && !Locking(mech)) {
		    sprintf(buf, "%c%c", MechID(targets[i])[0],
			MechID(targets[i])[1]);
		    mech_settarget(a->mynum, mech, buf);
		    locked = 1;
		    if (a->flags & AUTOPILOT_CHASETARG && a->targ >= -1) {
			/* Do I salivate over my contacts? */
			auto_disengage(a->mynum, a, "");
			auto_delcommand(a->mynum, a, "-1");
			PG(a) = 0;
			auto_addcommand(a->mynum, a, tprintf("autogun"));
			auto_addcommand(a->mynum, a, tprintf("dumbfollow %d", targets[i]->mynum));
			auto_engage(a->mynum, a, "");
			}
		}
/* Bah. Screw ammo. Let's blast any BTH.
		if (targetscore[i] > (10 + ((MechTarget(mech) !=
				targets[i]->mynum) ? 0 : 2) + Number(-1,
			    Number(0, 2))))
		    break;
		if (targetscore[i] > 5 &&
		    (((targetscore[i] >= (7 + (Number(0, 5)))) &&
			    MechWeapons[weaparray[ii]].ammoperton) ||
			(Locking(mech) && Number(1, 6) != 5) ||
			(IsRunning(MechSpeed(mech), MMaxSpeed(mech)) &&
			    Number(1, 4) == 4)))
		    break;
*/
		if (!IsRunning(MechSpeed(mech), MMaxSpeed(mech)) &&
		    IsRunning(MechDesiredSpeed(mech), MMaxSpeed(mech))) {
		    cheating = 1;
		    save = MechDesiredSpeed(mech);
		    MechDesiredSpeed(mech) = MechSpeed(mech);
		}
		if (Uncon(targets[i])) {
		    if (MechAim(mech) == NUM_SECTIONS) {
			sprintf(buf, "h");
			mech_target(a->mynum, mech, buf);
		    }
		} else if (MechAim(mech) != NUM_SECTIONS) {
		    sprintf(buf, "-");
		    mech_target(a->mynum, mech, buf);
		}
		sprintf(buf, "%d", weapnum - 1);
		mech_fireweapon(a->mynum, mech, buf);
		if (cheating) {
		    cheating = 0;
		    MechDesiredSpeed(mech) = save;
		}
		if (global_kill_cheat) {
		    AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, 1, 0);
		    return;
		}
		if (WpnIsRecycling(mech, loop, critical[ii])) {
		    fired++;
		    rt += GunStat(weaparray[ii], GetPartMode(mech, loop, critical[ii]), GUN_HEAT) + MTX_HEAT_MOD(GetPartDamage(mech, loop, critical[ii]));
		    break;
		}
	    }
	}
    }
    AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, AUTOGUN_TICK, 0);
}
