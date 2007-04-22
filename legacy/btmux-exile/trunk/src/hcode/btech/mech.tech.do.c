/* All the *_{succ|fail|econ} functions belong here */

#include "mech.h"
#include "event.h"
#include "mech.events.h"
#include "mech.tech.h"
#include "p.econ.h"
#include "p.mech.tech.h"
#include "p.mech.status.h"
#include "p.mech.utils.h"

#define PARTCHECK_SUB(m,a,c) \
AVCHECKM(m,a,c); \
GrabPartsM(m,a,c);

#define PARTCHECK(m,l,a,c) \
{ PARTCHECK_SUB(m, alias_part(m, a, l), c); }

#define PARTCHECKTWO(m,a,c,d,f) \
AVCHECKM(m,a,c); \
AVCHECKM(m,d,f); \
GrabPartsM(m,a,c); \
GrabPartsM(m,d,f);

#define PARTCHECKTHREE(m,a,c,d,f,g,i) \
AVCHECKM(m,a,c); \
AVCHECKM(m,d,f); \
AVCHECKM(m,g,i); \
GrabPartsM(m,a,c); \
GrabPartsM(m,d,f); \
GrabPartsM(m,g,i);


static struct {
    char name;
    char *lname;
    int aflag;
    int rtype;
    int ntype;
    int rspec;
    int nspec;
} ammo_types[] = {
    {
    '-', "normal", 0, -1, -1, 0, 0}, {
    'L', "cluster", LBX_MODE, -1, -1, LBX, 0}, {
    'A', "artemis", ARTEMIS_MODE, TMISSILE, -1, 0, DAR}, {
    'N', "narc", NARC_MODE, TMISSILE, -1, 0, DAR}, {
    'S', "swarm", SWARM_MODE, TMISSILE, -1, IDF, DAR}, {
    '1', "swarm-1", SWARM1_MODE, TMISSILE, -1, IDF, DAR}, {
    'I', "inferno", INFERNO_MODE, TMISSILE, -1, 0, IDF | DAR}, {
    '&', "smoke", SMOKE_MODE, TARTILLERY, TAMMO, DAR, DFM}, {
    'C', "cluster", CLUSTER_MODE, TARTILLERY, TAMMO, DAR, DFM}, {
    'P', "pierce", PIERCE_MODE, -1, -1, AC, 0}, {
    'X', "caseless", CASELESS_MODE, -1, -1, AC, 0}, {
    'F', "incendiary", INCEND_MODE, TARTILLERY, TAMMO, DAR, DFM}, { 
    'T', "tracer", TRACER_MODE, -1, -1, AC, 0}, {
    'G', "sguided", SGUIDED_MODE, TMISSILE, -1, IDF, DAR}, {
    'D', "deadfire", DEADFIRE_MODE, TMISSILE, -1, DFM, 0}, {
    'E', "atm-er", ATMER_MODE, TMISSILE, -1, ATM, 0}, {
    'H', "atm-he", ATMHE_MODE, TMISSILE, -1, ATM, 0}, {
    'R', "precision", PRECISION_MODE, -1, -1, AC, 0}, {
    'Z', "stinger", STINGER_MODE, TMISSILE, -1, IDF, DAR}, {
    0, NULL, 0, 0, 0, 0, 0}
};

int valid_ammo_mode(MECH * mech, int loc, int part, int let)
{
    int w, i;

    if (!IsAmmo(GetPartType(mech, loc, part)) || !let)
	return -1;
    let = toupper(let);
    w = Ammo2I(GetPartType(mech, loc, part));
    for (i = 0; ammo_types[i].name; i++) {
	if (ammo_types[i].name != let)
	    continue;
	if (ammo_types[i].rtype >= 0 &&
	    MechWeapons[w].type != ammo_types[i].rtype)
	    continue;
	if (ammo_types[i].rspec &&
	    !(MechWeapons[w].special & ammo_types[i].rspec)) continue;
	if (ammo_types[i].ntype >= 0 &&
	    MechWeapons[w].type == ammo_types[i].ntype)
	    continue;
	if (ammo_types[i].nspec &&
	    (MechWeapons[w].special & ammo_types[i].nspec)) continue;
	return ammo_types[i].aflag;
    }
    return -1;
}

int FindAmmoType(MECH * mech, int loc, int part)
{
    int t = GetPartType(mech, loc, part);
    int m = GetPartMode(mech, loc, part);
    int base = -1, damage = 0;

    if (!(m & OS_MODE) && (!IsAmmo(t)))
	return t;
    else if (IsAmmo(t))
        t = Ammo2I(t);
    else if (IsWeapon(t))
	t = Weapon2I(t);

    damage = (GunStat(t, m, GUN_DAMAGE));
 
    if ((m & SGUIDED_MODE) && !(IsArtillery(t)))
	return Cargo(AMMO_LRM_SGUIDED);
    if ((m & STINGER_MODE) && !(IsArtillery(t)))
	return Cargo(AMMO_LRM_STINGER);
    if (!(m & (ATMER_MODE|ATMHE_MODE)) && strstr(MechWeapons[t].name,
	"CL.ATM-"))
	return Cargo(AMMO_ATM);
    if ((m & ATMER_MODE) && !(IsArtillery(t)))
        return Cargo(AMMO_ATMER);
    if ((m & ATMHE_MODE) && !(IsArtillery(t)))
        return Cargo(AMMO_ATMHE);
    if ((m & DEADFIRE_MODE) && !(IsArtillery(t))) {
	if (strstr(MechWeapons[t].name, "LRM")) {
	    return Cargo(LR_DFM_AMMO);
        } else {	
	    return Cargo(SR_DFM_AMMO);
	    }
	}
    if ((m & SMOKE_MODE) && (IsArtillery(t)))
         return Cargo(MORTAR_SMOKE_AMMO);
    if ((m & CLUSTER_MODE) && (IsArtillery(t)))
        return Cargo(MORTAR_CLUSTER_AMMO); 
    if ((m & INCEND_MODE) && (IsArtillery(t)))
	return Cargo(MORTAR_INCENDIARY_AMMO);
    if (strstr(MechWeapons[t].name, "StreakSRM"))
	base = SSRM_AMMO;
    else if (strstr(MechWeapons[t].name, "LRM"))
	base = LRM_AMMO;
    else if (strstr(MechWeapons[t].name, "SRM"))
	base = SRM_AMMO;
    else if (strstr(MechWeapons[t].name, "MRM"))
	base = MRM_AMMO;
    else if (strstr(MechWeapons[t].name, "Mortar"))
	base = MORTAR_AMMO;
    else if (strstr(MechWeapons[t].name, "Thunder"))
	base = THUNDERBOLT_AMMO;
    else if (strstr(MechWeapons[t].name, "LR_DFM"))
	base = LR_DFM_AMMO;
    else if (strstr(MechWeapons[t].name, "SR_DFM"))
	base = SR_DFM_AMMO; 
    else if (strstr(MechWeapons[t].name, "RocketLnchr"))
	base = ROCKET_AMMO;

    if (!(m & (ARTEMIS_MODE|NARC_MODE|LBX_MODE|SWARM_MODE|SWARM1_MODE|INFERNO_MODE|PIERCE_MODE|CASELESS_MODE|TRACER_MODE|PRECISION_MODE)) ||
		 ((MechWeapons[t].type == TARTILLERY) && (m & (INCEND_MODE|SMOKE_MODE|CLUSTER_MODE)))) {
	 if (base < 0)
	    return I2Ammo(t);
	else
	    return Cargo(base);
    }
    if (m & TRACER_MODE) {
        if (damage == 19)
            base = AC20_AMMO_TRACER;
        else if (damage == 9)
            base = AC10_AMMO_TRACER;
        else if (damage == 4)
            base = AC5_AMMO_TRACER;
        else if (damage == 1)
            base = AC2_AMMO_TRACER;
        if (base < 0)
            return I2Ammo(t);
        return Cargo(base);
    }
    if (m & CASELESS_MODE) {
        if (damage == 20)
            base = AC20_AMMO_CASELESS; 
        else if (damage == 10)
            base = AC10_AMMO_CASELESS;
        else if (damage == 5)
            base = AC5_AMMO_CASELESS;
        else if (damage == 2)
            base = AC2_AMMO_CASELESS;
        if (base < 0)
            return I2Ammo(t);
        return Cargo(base);
    } 
    if (m & PIERCE_MODE) {
        if (damage == 20)
            base = AC20_AMMO_PIERCE;
        else if (damage == 10)
            base = AC10_AMMO_PIERCE;
        else if (damage == 5)
            base = AC5_AMMO_PIERCE;
        else if (damage == 2) 
            base = AC2_AMMO_PIERCE;
        if (base < 0)
            return I2Ammo(t);
        return Cargo(base);
    } 
    if (m & PRECISION_MODE) {
        if (damage == 20)
            base = AC20_AMMO_PRECISION;
        else if (damage == 10)
            base = AC10_AMMO_PRECISION;
        else if (damage == 5)
            base = AC5_AMMO_PRECISION;
        else if (damage == 2)
            base = AC2_AMMO_PRECISION;
        if (base < 0)
            return I2Ammo(t);
        return Cargo(base);
    } 
    if (m & LBX_MODE) {
	if (damage == 20)
	    base = LBX20_AMMO;
	else if (damage == 10)
	    base = LBX10_AMMO;
	else if (damage == 5)
	    base = LBX5_AMMO;
	else if (damage == 2)
	    base = LBX2_AMMO;
	if (base < 0)
	    return I2Ammo(t);
	return Cargo(base);
    }

/*    if (!(MechWeapons[t].type == TARTILLERY)) */
/*	{ */
        if (base < 0)
	        return I2Ammo(t);
        if (m & NARC_MODE)
	        return Cargo(base) + NARC_LRM_AMMO - LRM_AMMO;
        if (m & ARTEMIS_MODE)
        	return Cargo(base) + ARTEMIS_LRM_AMMO - LRM_AMMO;
        if (m & SWARM_MODE)
        	return Cargo(base) + SWARM_LRM_AMMO - LRM_AMMO;
        if (m & SWARM1_MODE)
	        return Cargo(base) + SWARM1_LRM_AMMO - LRM_AMMO;
        if (m & INFERNO_MODE)
        	return Cargo(base) + INFERNO_SRM_AMMO - SRM_AMMO;
/*	} else { */
/*	if (m & SMOKE_MODE) */
/*		return Cargo(base) + MORTAR_SMOKE_AMMO - MORTAR_AMMO; */
/*	if (m & CLUSTER_MODE) */
/*		return Cargo(base) + MORTAR_CLUSTER_AMMO - MORTAR_AMMO; */
/*	} */ 
    /* Weird dough, shouldn't happen */
    return Cargo(base);
}

TFUNC_LOCPOS(replace_econ)
{
    if (IsAmmo(GetPartType(mech, loc, part)))
	return 0;
    PARTCHECK(mech, loc, GetPartType(mech, loc, part), 1);
    return 0;
}

TFUNC_LOCPOS(replace_wholeweap)
{
    if (!(IsWeapon(Weapon2I(GetPartType(mech, loc, part)))))
	return 0;
    PARTCHECK(mech, loc, GetPartType(mech, loc, part), MechWeapons[Weapon2I(GetPartType(mech, loc, part))].criticals); 
    return 0;
}

TFUNC_LOCPOS_VAL(reload_econ)
{
    int loop, index = 0, count = 1;
    
    if (IsAmmo(GetPartType(mech, loc, part)))
	index = Ammo2I(GetPartType(mech, loc, part));
    else if (IsWeapon(GetPartType(mech, loc, part)))
	index = Weapon2I(GetPartType(mech, loc, part));
    else
	index = GetPartType(mech, loc, part);

    if (index && IsMissile(index))
	{	
            for (loop = 0; MissileHitTable[loop].key != -1; loop++)
	        {
	        if (MissileHitTable[loop].key == index)
	            count = MissileHitTable[loop].num_missiles[10];
	        }	
	if (GetPartMode(mech, loc, part) & OS_MODE) {
	  PARTCHECK(mech, loc, FindAmmoType(mech, loc, part), count);
	} else {
	  PARTCHECK(mech, loc, FindAmmoType(mech, loc, part), (count * (FullAmmo(mech, loc, part) - GetPartData(mech, loc, part))));
	}
	} else {
	PARTCHECK(mech, loc, FindAmmoType(mech, loc, part), (FullAmmo(mech, loc, part) - GetPartData(mech, loc, part)));
	}
    return 0;
}

TFUNC_LOC_VAL(fixarmor_econ)
{
    PARTCHECK(mech, loc, ProperArmor(mech), *val);
    return 0;
}

TFUNC_LOC_VAL(fixinternal_econ)
{
    PARTCHECK(mech, loc, ProperInternal(mech), *val);
    return 0;
}

TFUNC_LOCPOS(repair_econ)
{
    if (IsAmmo(GetPartType(mech, loc, part)))
	return 0;
#if 0
    PARTCHECKTWO(mech, Cargo(S_ELECTRONIC), 0, PartIsDestroyed(mech, loc, part) ? 3 : 1,
		ProperInternal(mech), 0, PartIsDestroyed(mech, loc, part) ? 3 : 1);
#else
    PARTCHECK(mech, loc, ProperInternal(mech), PartIsDestroyed(mech, loc, part) ? 3 : 1);
#endif
    return 0;
}

TFUNC_LOC(reattach_econ)
{
    if (mudconf.btech_complexrepair) {
	#if 0
	PARTCHECKTHREE(mech, ProperInternal(mech), GetSectOInt(mech, loc), Cargo(S_ELECTRONIC), GetSectOInt(mech, loc), ProperMyomer(mech), 0, 1);
	#else
	if (MechType(mech) == CLASS_MECH) {
	    PARTCHECKTWO(mech, ProperInternal(mech), GetSectOInt(mech, loc), ProperMyomer(mech), 1);
	} else {
	    PARTCHECK(mech, loc, ProperInternal(mech), GetSectOInt(mech, loc));
	}
	#endif
    } else  {
	#if 0
	PARTCHECKTWO(mech, ProperInternal(mech), GetSectOInt(mech, loc),
		       Cargo(S_ELECTRONIC), GetSectOInt(mech, loc));
	#else
	PARTCHECK(mech, loc, ProperInternal(mech), GetSectOInt(mech, loc));
	#endif
    }
    return 0;
}

/*
 * Added for new flood code by Kipsta
 * 8/4/99
 */

TFUNC_LOC(reseal_econ)
{
#if 0
    PARTCHECKTWO(mech, ProperInternal(mech), GetSectOInt(mech, loc),
	Cargo(S_ELECTRONIC), GetSectOInt(mech, loc));
#else
    PARTCHECK(mech, loc, ProperInternal(mech), GetSectOInt(mech, loc));
#endif
    return 0;
}

/* -------------------------------------------- Successes */

/* Replace success is just that ; success, therefore the fake
   functions here */
NFUNC(TFUNC_LOCPOS(replacep_succ));
NFUNC(TFUNC_LOCPOS(replaceg_succ));
NFUNC(TFUNC_LOCPOS_VAL(reload_succ));
NFUNC(TFUNC_LOC_VAL(fixinternal_succ));
NFUNC(TFUNC_LOC_VAL(fixarmor_succ));
NFUNC(TFUNC_LOC(reattach_succ));
NFUNC(TFUNC_LOC_RESEAL(reseal_succ));

/* Repairs _Should_ have some averse effects */
NFUNC(TFUNC_LOCPOS(repairg_succ));
NFUNC(TFUNC_LOCPOS(repairp_succ));

/* -------------------------------------------- Failures */

/* Replace failures give you one chance to roll for object recovery,
   otherwise it's irretrieavbly lost */
TFUNC_LOCPOS(replaceg_fail)
{
    int w = (IsWeapon(GetPartType(mech, loc, part)));
    int num;

    if (( w ? tech_weapon_roll(player, mech, REPLACE_DIFFICULTY) : tech_roll(player, mech, REPLACE_DIFFICULTY)) < 0) {
	notify(player,
	    tprintf("You muck around, wasting the %s in the progress.",
		w ? "weapon" : "part"));
	return -1;
    }
    notify(player,
	tprintf
	("Despite messing the repair, you manage not to waste the %s.",
     w ? "weapon" : "part"));
    num = (w ? ((WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part), 
	   GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc, part)))) == 2) ?
	  (MechWeapons[Weapon2I(GetPartType(mech, loc, part))].criticals) : 1) : 1);	

    AddPartsM(mech, loc, FindAmmoType(mech, loc, part), num);
    return -1;
}

TFUNC_LOCPOS(repairg_fail)
{
    notify(player,
	"Your repair fails.. all the parts are wasted for good.");
    return -1;
}

/* Replacepart = Replacegun, for now */
TFUNC_LOCPOS(replacep_fail)
{
    return replaceg_fail(player, mech, loc, part);
}

/* Repairpart = Repairgun, for now */
TFUNC_LOCPOS(repairp_fail)
{
    return repairg_fail(player, mech, loc, part);
}

/* Reload fail = ammo is wasted and some time, but no averse effects (yet) */
TFUNC_LOCPOS_VAL(reload_fail)
{
    notify(player, "You fumble around, wasting the ammo in the progress.");
    return -1;
}

/* Fixarmor/fixinternal failure means that at least 1, or at worst
   _all_, points are wasted */
TFUNC_LOC_VAL(fixarmor_fail)
{
    int tot = 0;
    int should = *val;

    if (tech_roll(player, mech, FIXARMOR_DIFFICULTY) >= 0)
	tot += 50;
    tot += random() % 40 + 5;
    tot = (tot * should) / 100;
    if (tot == 0)
	tot = 1;
    if (tot == should)
	tot = should - 1;
    notify(player,
	tprintf
	("Your armor patching isn't exactly perfect.. You managed to fix %d out of %d.",
     tot, should));
    *val = tot;
    return 0;
}

TFUNC_LOC_VAL(fixinternal_fail)
{
    int tot = 0;
    int should = *val;

    if (tech_roll(player, mech, FIXARMOR_DIFFICULTY) >= 0)
	tot += 50;
    tot += random() % 40 + 5;
    tot = (tot * should) / 100;
    if (tot == 0)
	tot = 1;
    if (tot == should)
	tot = should - 1;
    notify(player,
	tprintf
	("Your internal patching isn't exactly perfect.. You managed to fix %d out of %d.",
     tot, should));
    *val = tot;
    return 0;
}

/* Reattach has 2 failures:
   - if you succeed in second roll, it takes just 1.5x time
   - if you don't, some (random %) of stuff is wasted and nothing is
   done (yet some techtime goes nonetheless */
TFUNC_LOC(reattach_fail)
{
    int tot;

    if (tech_roll(player, mech, REATTACH_DIFFICULTY) >= 0)
	return 0;
    tot = random() % 90 + 5;
    notify(player,
	tprintf
	("Despite your disastrous failure, you recover %d%% of the materials.",
     tot));
    tot = (tot * GetSectOInt(mech, loc)) / 100;
    if (tot == 0)
	tot = 1;
    if (tot == GetSectOInt(mech, loc))
	tot = GetSectOInt(mech, loc) - 1;
#if 0
    AddPartsM(mech, loc, Cargo(S_ELECTRONIC), 0, tot);
#endif
    AddPartsM(mech, loc, ProperInternal(mech), tot);
    if (mudconf.btech_complexrepair && MechType(mech) == CLASS_MECH)
	AddPartsM(mech, loc, ProperMyomer(mech), 1);
    return -1;
}

/*
 * Added by Kipsta for flooding code
 * 8/4/99
 */

TFUNC_LOC_RESEAL(reseal_fail)
{
    int tot;

    if (tech_roll(player, mech, RESEAL_DIFFICULTY) >= 0)
	return 0;
    tot = random() % 90 + 5;
    notify(player,
	tprintf
	("You don't manage to get all the water out and seal the section, though you recover %d%% of the materials.",
     tot));
    tot = (tot * GetSectOInt(mech, loc)) / 100;
    if (tot == 0)
	tot = 1;
    if (tot == GetSectOInt(mech, loc))
	tot = GetSectOInt(mech, loc) - 1;
#if 0
    AddPartsM(mech, loc, Cargo(S_ELECTRONIC), tot);
#endif
    AddPartsM(mech, loc, ProperInternal(mech), tot);
    return -1;
}
