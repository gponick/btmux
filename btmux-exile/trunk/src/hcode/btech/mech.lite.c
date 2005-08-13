#include "mech.h"
#include "p.mech.utils.h" 
#include "p.mech.los.h"

void cause_lite(MECH * mech, MECH * tempMech)
{
    MAP *map;
    int flag; 

    if ((!mech) || (!tempMech))
	return;
    map = getMap(mech->mapindex); 

    /* Selectively and _creatively_ cause light */

    /* Creatively my shoe. You don't even do a LOSInfo check for LoS Blockage. Now you do. - DJ */

    /* uh. huh. right. */
    /* See where from mech is facing, compare it to bearing to to mech */
    /* if within front arc, target's lit */
    if (IsLit2(tempMech))
	return;			/* No senseless waste of effort */
    if (!(InWeaponArc(mech, MechFX(tempMech), MechFY(tempMech)) & FORWARDARC)) 
	return;
    if (map->LOSinfo[mech->mapnumber][tempMech->mapnumber] & MECHLOSFLAG_BLOCK)
	return;

    /* Time for _light_ */
    MechCritStatus(tempMech) |= SLITE_LIT;
    if (tempMech != mech && MechSLWarn(tempMech) && Started(tempMech))
	{
	if (MechStatus(tempMech) & SLITE_ON)
		return;
	else
	    mech_notify(tempMech, MECHALL, tprintf("%%ch%%cyYou are suddenly illuminated by %s!%%c",
		 GetMechToMechID(tempMech, mech)));
	}
}

void end_lite_check(MECH * mech)
{
    MAP *map;
    MECH *t;
    int i;

    if (!IsLit2(mech))
	return;
    if (!(map = FindObjectsData(mech->mapindex)))
	return;
    for (i = 0; i < map->first_free; i++) {
	if (!(t = FindObjectsData(map->mechsOnMap[i])))
	    continue;
	if (t == mech)
	    continue;
	if ((MechStatus(t) & SLITE_ON) && FaMechRange(t, mech) < LITE_RANGE) 
		if (InWeaponArc(t, MechFX(mech), MechFY(mech)) & FORWARDARC)
			if (!(map->LOSinfo[t->mapnumber][mech->mapnumber] & MECHLOSFLAG_BLOCK))
				return;
    }
    MechCritStatus(mech) &= ~SLITE_LIT;
    if (MechSLWarn(mech) && Started(mech)) {
	if (MechStatus(mech) & SLITE_ON)
	    return;
	else
	    mech_notify(mech, MECHALL, tprintf("%%ch%%cyYou are no longer illuminated.%%c"));
    }
}
