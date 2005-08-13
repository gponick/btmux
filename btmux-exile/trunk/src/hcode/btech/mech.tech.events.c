#include "mech.h"
#include "event.h"
#include "mech.events.h"
#include "mech.tech.h"
#include "p.econ.h"
#include "p.mech.utils.h"
#include "p.mech.tech.do.h"
#include "p.mech.status.h"

#define VERBOSE_ENDS



static int completely_intact_int(MECH * mech)
{
    int i;

    for (i = 0; i < NUM_SECTIONS; i++)
	if (!GetSectInt(mech, i) && GetSectOInt(mech, i))
	    return 0;
    return 1;
}

void event_mech_removesection(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    char buf[MBUF_SIZE];
    int loc, pos, extra, sects = 0, s;

    /* changed Special2I to Special on AddPartsM statements */
    UNPACK_LOCPOS_E(earg, loc, pos, extra);
    AddPartsM(mech, loc, ProperInternal(mech), (2 * GetSectInt(mech, loc)) / extra);
    AddPartsM(mech, loc, ProperArmor(mech), (2 * GetSectArmor(mech, loc)) / extra);
#if 0
    AddPartsM(mech, loc, Cargo(S_ELECTRONIC), (GetSectInt(mech, loc)) / extra);
#endif
    /* Let's be nice on scrapping and prevent evil things */
    for (s = 0; s < NumSections(MechType(mech)); s++) {
	if (GetSectOInt(mech, s) && !SectIsDestroyed(mech, s))
	    sects++;
	}
    if (sects == 1 || (MechType(mech) == CLASS_MECH && loc == HEAD))
	tele_contents(mech->mynum, Location(mech->mynum), TELE_LOUD|TELE_ALL, mech);
    mech_Detach(mech, loc);

    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    do {
	int i = 0;

	if (Destroyed(mech))
	    i = 1;
	MechStatus(mech) &= ~DESTROYED;
	mech_notify(mech, MECHALL, tprintf("%s has been removed.", buf));
	if (i)
	    MechStatus(mech) |= DESTROYED;
    } while (0);
}

void event_mech_removegun(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc, pos, i, extra;
    char buf[MBUF_SIZE];

    UNPACK_LOCPOS_E(earg, loc, pos, extra);
    for (i = pos; i < (pos + GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc, pos)))); i++)
	DestroyPart(mech, loc, i);
    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    if (extra == 2 && (e->function != very_fake_func)) {
	AddPartsM(mech, loc, FindAmmoType(mech, loc, pos), MechWeapons[Weapon2I(GetPartType(mech, loc, pos))].criticals);
	do {
	    int i = 0;

	    if (Destroyed(mech))
		i = 1;
	    MechStatus(mech) &= ~DESTROYED;
	    mech_notify(mech, MECHALL,
		tprintf("%s from %s has been removed.", pos_part_name(mech,
			loc, pos), buf));
	    if (i)
		MechStatus(mech) |= DESTROYED;
	} while (0);
    } else {
	do {
	    int i = 0;

	    if (Destroyed(mech))
		i = 1;
	    MechStatus(mech) &= ~DESTROYED;
	    mech_notify(mech, MECHALL,
		tprintf("%s from %s has been removed and scrapped.",
		    pos_part_name(mech, loc, pos), buf));
	    if (i)
		MechStatus(mech) |= DESTROYED;
	} while (0);
    }
}


void event_mech_removepart(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc, pos, extra;
    char buf[MBUF_SIZE];

    UNPACK_LOCPOS_E(earg, loc, pos, extra);
    DestroyPart(mech, loc, pos);
    if (MechType(mech) == CLASS_MECH)
	do_magic(mech);
    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    if (extra == 2 && (e->function != very_fake_func)) {
	AddPartsM(mech, loc, FindAmmoType(mech, loc, pos), 1);
	do {
	    int i = 0;

	    if (Destroyed(mech))
		i = 1;
	    MechStatus(mech) &= ~DESTROYED;
	    mech_notify(mech, MECHALL,
		tprintf("%s from %s has been removed.", pos_part_name(mech,
			loc, pos), buf));
	    if (i)
		MechStatus(mech) |= DESTROYED;
	} while (0);
    } else {
	do {
	    int i = 0;

	    if (Destroyed(mech))
		i = 1;
	    MechStatus(mech) &= ~DESTROYED;
	    mech_notify(mech, MECHALL,
		tprintf("%s from %s has been removed and scrapped.",
		    pos_part_name(mech, loc, pos), buf));
	    if (i)
		MechStatus(mech) |= DESTROYED;
	} while (0);
    }
}

void event_mech_repairarmor(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc = earg % 16;
    int amount = (earg / 16) % 256;
    int player = ((int) e->data2) / PLAYERPOS;
    char buf[MBUF_SIZE];

    if (loc >= 8) {
	SetSectRArmor(mech, loc % 8, MIN(GetSectRArmor(mech, loc % 8) + 1,
		GetSectORArmor(mech, loc % 8)));
    } else {
	SetSectArmor(mech, loc, MIN(GetSectArmor(mech, loc) + 1,
		GetSectOArmor(mech, loc)));
    }
    amount--;
    if (amount < 0)
	return;
    if (amount <= 0) {
	ArmorStringFromIndex(loc % 8, buf, MechType(mech), MechMove(mech));
	if (loc >= 8) {
	    do {
		int i = 0;

		if (Destroyed(mech))
		    i = 1;
		MechStatus(mech) &= ~DESTROYED;
		mech_notify(mech, MECHALL,
		    tprintf("%s's rear armor repairs have been finished.",
			buf));
		if (i)
		    MechStatus(mech) |= DESTROYED;
	    } while (0);
	} else {
	    do {
		int i = 0;

		if (Destroyed(mech))
		    i = 1;
		MechStatus(mech) &= ~DESTROYED;
		mech_notify(mech, MECHALL,
		    tprintf("%s's armor repairs have been finished.",
			buf));
		if (i)
		    MechStatus(mech) |= DESTROYED;
	    } while (0);
	}

	if (MechType(mech) != CLASS_MECH && completely_intact_int(mech))
	    do_magic(mech);
	return;
    }
    REPAIREVENT(FIXARMOR_TIME, mech, (amount * 16 + loc),
	event_mech_repairarmor, EVENT_REPAIR_FIX);
}

void event_mech_repairinternal(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc = earg % 16;
    int amount = (earg / 16) % 256;
    int player = ((int) e->data2) / PLAYERPOS;
    char buf[MBUF_SIZE];

    if (is_aero(mech))
	AeroSI(mech)++;
    else
	SetSectInt(mech, loc, GetSectInt(mech, loc) + 1);

    if (GetSectInt(mech, loc) > GetSectOInt(mech, loc)) {
	if (is_aero(mech))
	    AeroSI(mech) = AeroSIOrig(mech);
	else
	    SetSectInt(mech, loc, GetSectOInt(mech, loc));
	}
    amount--;
    if (amount < 0)
	return;
    if (amount <= 0) {
	ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
	do {
	    int i = 0;

	    if (Destroyed(mech))
		i = 1;
	    MechStatus(mech) &= ~DESTROYED;
	    mech_notify(mech, MECHALL,
		tprintf("%s's internal repairs have been finished.", buf));
	    if (i)
		MechStatus(mech) |= DESTROYED;
	} while (0);
	if (MechType(mech) != CLASS_MECH && completely_intact_int(mech))
	    do_magic(mech);
	return;
    }
    REPAIREVENT(FIXINTERNAL_TIME, mech, (amount * 16 + loc),
	event_mech_repairinternal, EVENT_REPAIR_FIXI);
}

void event_mech_reattach(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    char buf[MBUF_SIZE];

    /* Basically: Unset the limb destroyed, without doing a thing to
       damaged parts */
    mech_ReAttach(mech, earg);
    ArmorStringFromIndex(earg, buf, MechType(mech), MechMove(mech));
    if (completely_intact_int(mech))
	do_magic(mech);
    do {
	int i = 0;

	if (Destroyed(mech))
	    i = 1;
	MechStatus(mech) &= ~DESTROYED;
	mech_notify(mech, MECHALL, tprintf("%s has been reattached.",
		buf));
	if (i)
	    MechStatus(mech) |= DESTROYED;
    } while (0);
}

/*
 * Added for new flood code by Kipsta
 * 8/4/99
 */

void event_mech_reseal(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    char buf[MBUF_SIZE];

    mech_ReSeal(mech, earg);
    ArmorStringFromIndex(earg, buf, MechType(mech), MechMove(mech));
    mech_notify(mech, MECHALL, tprintf("%s has been resealed.", buf));
}

void event_mech_replacegun(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc, pos, i, extra;
    char buf[MBUF_SIZE];

    UNPACK_LOCPOS_E(earg, loc, pos, extra);
    for (i = pos;
	i < (pos + GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc,
			pos)))); i++) {
	mech_RepairPart(mech, loc, i);
    }
    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    do {
	int i = 0;

	if (Destroyed(mech))
	    i = 1;
	MechStatus(mech) &= ~DESTROYED;
	mech_notify(mech, MECHALL, tprintf("%s on %s has been replaced.",
		pos_part_name(mech, loc, pos), buf));
	if (i)
	    MechStatus(mech) |= DESTROYED;
    } while (0);
}

void event_mech_repairgun(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc, pos, i;
    char buf[MBUF_SIZE];

    UNPACK_LOCPOS(earg, loc, pos);
    for (i = pos; i < (pos + GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc, pos)))); i++) {
#if 0
	mech_RepairPart(mech, loc, i);
#else
	UnDisablePart(mech, loc, i);
	SetPartDamage(mech, loc, i, 0);
#endif
    }
    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    do {
	int i = 0;

	if (Destroyed(mech))
	    i = 1;
	MechStatus(mech) &= ~DESTROYED;
	mech_notify(mech, MECHALL, tprintf("%s on %s has been repaired.", pos_part_name(mech, loc, pos), buf));
	if (i)
	    MechStatus(mech) |= DESTROYED;
    } while (0);
}

void event_mech_repairpart(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc, pos;
    char buf[MBUF_SIZE];

    UNPACK_LOCPOS(earg, loc, pos);
    mech_RepairPart(mech, loc, pos);
    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    do {
	int i = 0;

	if (Destroyed(mech))
	    i = 1;
	MechStatus(mech) &= ~DESTROYED;
	mech_notify(mech, MECHALL, tprintf("%s on %s has been repaired.",
		pos_part_name(mech, loc, pos), buf));
	if (i)
	    MechStatus(mech) |= DESTROYED;
    } while (0);
}

void event_mech_reload(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int earg = (int) (e->data2) % PLAYERPOS;
    int loc, pos, extra;
    int loop, index = 0, count = 1;
    char buf[MBUF_SIZE];

    UNPACK_LOCPOS_E(earg, loc, pos, extra);
    if (extra) {
/*	SetPartData(mech, loc, pos, 0); */
	if (extra > 1)
	    { 
	    if (IsAmmo(GetPartType(mech, loc, pos)))
	        index = Ammo2I(GetPartType(mech, loc, pos)); 
	    else
		index = GetPartType(mech, loc, pos);
	    if (IsMissile(Ammo2WeaponI(GetPartType(mech, loc, pos)))) 
	            for (loop = 0; MissileHitTable[loop].key != -1; loop++)
        	        {
	                if (MissileHitTable[loop].key == index)
        	            count = MissileHitTable[loop].num_missiles[10];
	                } 
	    AddPartsM(mech, loc, FindAmmoType(mech, loc, pos), (count * GetPartData(mech, loc, pos))); 
	    }
	SetPartData(mech, loc, pos, 0);
    } else
	mech_FillPartAmmo(mech, loc, pos);
    ArmorStringFromIndex(loc, buf, MechType(mech), MechMove(mech));
    do {
	int i = 0;

	if (Destroyed(mech))
	    i = 1;
	MechStatus(mech) &= ~DESTROYED;
	mech_notify(mech, MECHALL, tprintf("%s on %s has been %sloaded.",
		pos_part_name(mech, loc, pos), buf, extra ? "un" : "re"));
	if (i)
	    MechStatus(mech) |= DESTROYED;
    } while (0);
    do_magic(mech);
}

void event_mech_mountbomb(EVENT * e)
{
/*    MECH *mech = (MECH *) e->data; */
/*    int earg = (int) (e->data2) % PLAYERPOS; */
}

void event_mech_umountbomb(EVENT * e)
{
/*    MECH *mech = (MECH *) e->data; */
/*    int earg = (int) (e->data2) % PLAYERPOS; */
}
