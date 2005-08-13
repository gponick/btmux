#include "mech.h"
#include "p.mech.utils.h"
#include "p.mech.ecm.h"

#define LOUD_ECM		/* No complete stealth */
#undef VERY_LOUD_ECM		/* But no spam, either */

/* 
   Synopsis:
   Guardian ECM (only) is implemented right now. It works by setting
   ECM_ACTIVE on the MechStatus. After that, on each LOS update,
   if a mech is within the r6 and is not friendly, it'll be
   given ECM_DISTURBANCE flag that will go away only when it's OWN 
   turn-ly update sees no bad guys in neighbourhood.

   This file contains only ecm on/off command, at least now, and the
   function for giving a enemy mech info about being tagged for ECM.
 */
/*
   ECCM mode is now implemented and working as it should. Next in Angel ECM 9/08/00  - DJ
*/

/*
  Angel ECM and Bloodhound Probe now in. 11/01/00 - DJ
*/
void cause_ecm(MECH * from, MECH * to)
{ 
    if (!(MechStatus(to) & ECM_DISTURBANCE))
      { 
      if (ecm_count(to, 0) > eccm_count(to, 1)) 
	{
	MechStatus(to) |= ECM_DISTURBANCE;
	MarkForLOSUpdate(to); 
#ifdef LOUD_ECM
        mech_notify(to, MECHALL,
	    "Half your screens are suddenly filled with static!");
#ifdef VERY_LOUD_ECM
        if (MechSpecials(to) & BEAGLE_PROBE_TECH &&
		!(MechCritStatus(to) & BEAGLE_DESTROYED)) mech_notify(to, MECHALL,
		    "Your Beagle Active Probe is inoperative!");
        if (MechSpecials2(to) & BLOODHOUND_PROBE_TECH &&
                !(MechCritStatus(to) & BEAGLE_DESTROYED)) mech_notify(to, MECHALL,
                    "Your Bloodhound Active Probe is inoperative!");
        if (IsC3(to))
	    mech_notify(to, MECHALL, "Your C3 network is inoperative!");
#endif
#endif
	}
      }
    if (!(MechStatus2(to) & ECM_PROTECTED)) 
      {
      if (ecm_count(to, 1) > eccm_count(to, 0))
	{
        MechStatus2(to) |= ECM_PROTECTED;
        MarkForLOSUpdate(to);
/*	if (MechStatus(to) & ECM_ACTIVE) */
/*		mech_notify(to, MECHALL, "Your readouts indicate your CounterMeasures are functioning properly."); */
/*	else	 */
/*		mech_notify(to, MECHALL, "Your scanners indicate you are within an ECM protection bubble."); */ 
        } 
      }
}

void end_ecm_check(MECH * mech)
{
    int i;
    MAP *map;
    MECH *t; 

    if (!(map = FindObjectsData(mech->mapindex)))
	return;
/*    for (i = 0; i < map->first_free; i++) { */
/*	if (!(t = FindObjectsData(map->mechsOnMap[i]))) */
/*	    continue; */
/*	if (MechTeam(t) == MechTeam(mech)) */
/*	    continue; */
/*	if ((MechStatus(t) & ECM_ACTIVE) && */
/*	    FaMechRange(t, mech) < ECM_RANGE) return; */
/*    } */ 
    if (MechStatus(mech) & ECM_DISTURBANCE)
	{
        if (ecm_count(mech, 0) <= eccm_count(mech, 1))
	    {
	    MechStatus(mech) &= ~ECM_DISTURBANCE;
	    MarkForLOSUpdate(mech); 
#ifdef LOUD_ECM
            mech_notify(mech, MECHALL,
	       "All your systems are back to normal again!");
#ifdef VERY_LOUD_ECM
	    if (MechSpecials(mech) & BEAGLE_PROBE_TECH &&
	        !(MechCritStatus(mech) & BEAGLE_DESTROYED)) mech_notify(mech,
	            MECHALL, "Your Beagle Active Probe is operational again!");
            if (MechSpecials(mech) & BLOODHOUND_PROBE_TECH &&
                !(MechCritStatus(mech) & BEAGLE_DESTROYED)) mech_notify(mech,
                    MECHALL, "Your Bloodhound Active Probe is operational again!");
	    if (IsC3(mech))
	    mech_notify(mech, MECHALL,
	        "Your C3 network is operational again!");
#endif
#endif
	    }
	}
    if (MechStatus2(mech) & ECM_PROTECTED)
	{ 
	if (ecm_count(mech, 1) <= eccm_count(mech, 0)) 
		{
		MechStatus2(mech) &= ~ECM_PROTECTED;
		MarkForLOSUpdate(mech);
/*		if (MechStatus(mech) & ECM_ACTIVE) */
/*			mech_notify(mech, MECHALL, "Your readouts indicate your ECM is being Couter-CounterMeasured!"); */
/*		else */
/*			mech_notify(mech, MECHALL, "Your scanners indicate you have left an ECM protection bubble."); */
		} 
	}
} 

/* mode of 0 counts hostile ecm/eccm count. mode of 1 counts friendly ecm/eccm count. - DJ */
int ecm_count(MECH * mech, int mode)
{ 
   MAP *map; 
   int count = 0, i = 0;
   MECH *t;
   map = getMap(mech->mapindex);

   if (StealthAndECM(mech))
	count = 20;
   else 
     for (i = 0; i < map->first_free; i++)
	{
   	if (!(t = FindObjectsData(map->mechsOnMap[i])) || !map)
		continue;
/*	if (mode ? MechTeam(t) != MechTeam(mech) : MechTeam(t) == MechTeam(mech)) */
/*		continue; */
	if (MechStatus(t) & ECM_ACTIVE)
	    if (FaMechRange(t, mech) < ECMRange(t))
		{
		if ((mode > 0) ? (MechTeam(mech) == MechTeam(t)) : (MechTeam(mech) != MechTeam(t)))
			{
			count++;		
			if (MechSpecials2(t) & ANGEL_ECM_TECH)
			    count++;
			}
		continue;
		}
	}
   return count;
}

int eccm_count(MECH * mech, int mode)
{
   MAP *map;
   int count = 0, i = 0;
   MECH *t;
   map = getMap(mech->mapindex);

   for (i = 0; i < map->first_free; i++) 
        {
        if (!(t = FindObjectsData(map->mechsOnMap[i])) || !map)
                continue;
/*        if (MechTeam(t) != MechTeam(mech)) */
/*                continue; */
        if (MechStatus2(t) & ECCM_ACTIVE)
	    if ((FaMechRange(t, mech) < ECMRange(t)) || (t == mech)) 
                {
		if ((mode > 0) ? (MechTeam(mech) == MechTeam(t)) : (MechTeam(mech) != MechTeam(t)))
			{
	                count++;
			if (MechSpecials2(t) & ANGEL_ECM_TECH)
                            count++; 
			}
                continue;
                }
        }
   return count;
}

int ecm_affect_hex(MECH * mech, int x, int y, int z, int *IsAngel)
{
    int ecm = 0, eccm = 0, i;
    MAP * map;
    MECH *t;
    float fx, fy, fz, tfx, tfy, tfz;

    *IsAngel = 0;
    MapCoordToRealCoord(x, y, &fx, &fy);
    fz = z; 
    map = getMap(mech->mapindex);

    if (z < 0)
	return -1;
    if (!mech)
	return -1;
    if (!map)
	return -1;
/* Let's not fry a CPU out for this feature - DJ */
    if ((MechStatus(mech) & COMBAT_SAFE) || !(MechStatus(mech) & STARTED))
	return -1; 
    if (MechMove(mech) == MOVE_NONE)
	return -1;
    if (MechType(mech) == CLASS_MW)
	return -1;
    if (MechStatus(mech) & COMBAT_SAFE)
	return -1;

    for (i = 0; i < map->first_free; i++)
	    { 
/* Sorry. I could care less about your LOS line to a dead or shutdown mech being ECM affected */
/* Problem? Don't die. Don't shutdown. The sacrifices we must make for performance - DJ */ 
        if (!(t = FindObjectsData(map->mechsOnMap[i]))) 
           continue;
	if (In_Character(mech->mynum) != In_Character(t->mynum))
	   continue;
	if ((MechStatus(t) & COMBAT_SAFE) || !(MechStatus(t) & STARTED)) 
	   continue; 
        if (MechMove(t) == MOVE_NONE)
	   continue;
	if (MechType(t) == CLASS_MW)
	   continue; 
	if (MechStatus(t) & ECM_ACTIVE) { 
	   if (MechTeam(mech) != MechTeam(t))
		{
		MapCoordToRealCoord(MechX(t), MechY(t), &tfx, &tfy);
		tfz = MechZ(t);
		if (FindRange(tfx, tfy, tfz, fx, fy, fz) < ECMRange(t))
		    {
		    ecm++;
		    if (MechSpecials2(t) & ANGEL_ECM_TECH)
		        {
                        ecm++;
		        *IsAngel = 1;
			}
		    }
		}			
	} else if (MechStatus2(t) & ECCM_ACTIVE) {
	   if (MechTeam(mech) == MechTeam(t))
		{
		MapCoordToRealCoord(MechX(t), MechY(t), &tfx, &tfy);
		tfz = MechZ(t);		
		if (FindRange(tfx, tfy, tfz, fx, fy, fz) < ECMRange(t))
		    {
		    eccm++;
		    if (MechSpecials2(t) & ANGEL_ECM_TECH)
                        eccm++;
		    }
		}
	    }
	}
    return (ecm > eccm ? 1 : 0);
}

/* mech_ecm moved to mech.advanced.c */
