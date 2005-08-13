#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "p.mech.utils.h"
#include "p.mech.update.h"
#include "p.mech.combat.h"
#include "p.artillery.h"
#include "p.btechstats.h"
#include "p.crit.h"
#include "p.eject.h"

#define SILLY_TOGGLE_MACRO(neededspecial,setstatus,msgon,msgoff,donthave) \
if (MechSpecials(mech) & (neededspecial)) \
{ if (MechStatus(mech) & (setstatus)) { mech_notify(mech, MECHALL, msgoff); \
MechStatus(mech) &= ~(setstatus); } else { mech_notify(mech,MECHALL, msgon); \
MechStatus(mech) |= (setstatus); } } else notify(player, donthave)

#define SILLY_TOGGLE_MACRO2(neededspecial,setstatus,msgon,msgoff,donthave) \
if (MechSpecials(mech) & (neededspecial)) \
{ if (MechStatus2(mech) & (setstatus)) { mech_notify(mech, MECHALL, msgoff); \
MechStatus2(mech) &= ~(setstatus); } else { mech_notify(mech,MECHALL, msgon); \
MechStatus2(mech) |= (setstatus); } } else notify(player, donthave)

/* Toggles NullSig on / off */
void mech_nullsig(dbref player, MECH * mech, char *buffer)
{
    cch(MECH_USUALO);
    DOCHECK(MechCritStatus(mech) & NULLSIG_DESTROYED,
        "Your Null Signature Device has been destroyed already!");
    if (!(MechSpecials(mech) & NULLSIG_TECH))
	{
	mech_notify(mech, MECHALL, "This toy has no such neat device!");
	return;
	}
    if (NullSigChanging(mech))
	{
	mech_notify(mech, MECHALL, "Your Null Signature system is already changing modes!");
	return;
	}
    if (MechStatus2(mech) & NULLSIG_ACTIVE)
	{
	mech_notify(mech, MECHALL, "You begin the proccess of dis-engaging your Null Signature system.");
	MECHEVENT(mech, EVENT_NULLSIG, mech_nullsig_event, 30, 0);
	return;
	} else {
	mech_notify(mech, MECHALL, "You begin the proccess of engaging your Null Signature system.");
        MECHEVENT(mech, EVENT_NULLSIG, mech_nullsig_event, 30, 1);
        return;
	}
}


void mech_stealtharm(dbref player, MECH * mech, char *buffer)
{
    cch(MECH_USUALO);
    DOCHECK((MechCritStatus(mech) & ECM_DESTROYED) || (MechCritStatus2(mech) & STEALTHARM_DESTROYED),
        "Your ECM has been destroyed already! Yoru StealthArmor is useless!");
    if (!(MechSpecials2(mech) & STEALTHARMOR_TECH))
	{
	mech_notify(mech, MECHALL, "This toy has no such neat device!");
	return;
	}
    if (StealthArmChanging(mech))
	{
	mech_notify(mech, MECHALL, "Your Stealth Armor system is already changing modes!");
	return;
	}
    if (MechStatus2(mech) & STEALTHARM_ACTIVE)
	{
	mech_notify(mech, MECHALL, "You begin the proccess of dis-engaging your Stealth Armor system.");
	MECHEVENT(mech, EVENT_STEALTHARM, mech_stealtharm_event, 30, 0);
	return;
	} else {
	mech_notify(mech, MECHALL, "You begin the proccess of engaging your Stealth Armor system.");
        MECHEVENT(mech, EVENT_STEALTHARM, mech_stealtharm_event, 30, 1);
        return;
	}
}

/* Toggles ECM on / off */
void mech_ecm(dbref player, MECH * mech, char *buffer)
{
    int IsAngel = 0;

    cch(MECH_USUALO);
    DOCHECK(MechCritStatus(mech) & ECM_DESTROYED,
	"Your Guardian ECM has been destroyed already!");
    DOCHECK(MechStatus2(mech) & ECCM_ACTIVE,
	"Your Guardian ECM is already in ECCM mode!");
/*    SILLY_TOGGLE_MACRO(ECM_TECH, ECM_ACTIVE,
	"You turn your Guardian ECM system online.",
	"You turn your Guardian ECM system offline.",
	"Your 'mech isn't equipped with a Guardian ECM system!"); */
if ((MechSpecials(mech) & (ECM_TECH)) || (MechSpecials2(mech) & ANGEL_ECM_TECH))
	{
	IsAngel = (MechSpecials2(mech) & ANGEL_ECM_TECH);
	if (MechStatus(mech) & (ECM_ACTIVE))
		{
		mech_notify(mech, MECHALL,
		    tprintf("You turn your %s ECM system offline.",
			(IsAngel ? "Angel" : "Guardian")));
		MechStatus(mech) &= ~(ECM_ACTIVE);
		} else {
		mech_notify(mech,MECHALL,
		    tprintf("You turn your %s ECM system online.",
			(IsAngel ? "Angel" : "Guardian")));
		MechStatus(mech) |= (ECM_ACTIVE);
		}
	} else {
		notify(player, "Your unit isn't equipped with an ECM system!");
	}

    MarkForLOSUpdate(mech);
}

/* Toggles ECCM on / off */
void mech_eccm(dbref player, MECH * mech, char *buffer)
{
    int IsAngel = 0;

    cch(MECH_USUALO);
    DOCHECK(MechCritStatus(mech) & ECM_DESTROYED,
        "Your Guardian ECM has been destroyed already!");
    DOCHECK(MechStatus(mech) & ECM_ACTIVE,
	"Your Guardian ECM is already in ECM mode!");
/*    SILLY_TOGGLE_MACRO2(ECM_TECH, ECCM_ACTIVE,
        "You turn your Guardian ECM system online in ECCM mode.",
        "You turn your Guardian ECM system offline from ECCM mode.",
        "Your 'mech isn't equipped with a Guardian ECM system!"); */
if ((MechSpecials(mech) & (ECM_TECH)) || (MechSpecials2(mech) & ANGEL_ECM_TECH))
        {
        IsAngel = (MechSpecials2(mech) & ANGEL_ECM_TECH);
        if (MechStatus2(mech) & (ECCM_ACTIVE))
                {
                mech_notify(mech, MECHALL,
                    tprintf("You turn your %s ECM system offline in ECCM mode.",
                        (IsAngel ? "Angel" : "Guardian")));
                MechStatus2(mech) &= ~(ECCM_ACTIVE);
                } else {
                mech_notify(mech,MECHALL,
                    tprintf("You turn your %s ECM system online in ECCM mode.",
                        (IsAngel ? "Angel" : "Guardian")));
                MechStatus2(mech) |= (ECCM_ACTIVE);
                }
        } else {
                notify(player, "Your unit isn't equipped with an ECCM system!");
        }

    MarkForLOSUpdate(mech);
}

void MechSliteChangeEvent(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int wType = (int) e->data2;

    if (MechCritStatus(mech) & SLITE_DEST)
	return;

    if (!Started(mech)) {
	MechStatus(mech) &= ~SLITE_ON;
	MechCritStatus(mech) &= ~SLITE_LIT;
	return;
    }

    if (wType == 1) {
	MechStatus(mech) |= SLITE_ON;
	MechCritStatus(mech) |= SLITE_LIT;

	mech_notify(mech, MECHALL,
	    "Your searchlight comes on to full power.");
	MechLOSBroadcast(mech, "turns on a searchlight!");
    } else {
	MechStatus(mech) &= ~SLITE_ON;
	MechCritStatus(mech) &= ~SLITE_LIT;

	mech_notify(mech, MECHALL, "Your searchlight shuts off.");
	MechLOSBroadcast(mech, "turns off a searchlight!");
    }

    MarkForLOSUpdate(mech);
}

void mech_slite(dbref player, MECH * mech, char *buffer)
{
    cch(MECH_USUALO);

    if (!(MechSpecials(mech) & SLITE_TECH)) {
	mech_notify(mech, MECHALL,
	    "Your 'mech isn't equipped with searchlight!");
	return;
    }
    if (MechStatus(mech) & COMBAT_SAFE && MechStatus(mech) & SLITE_ON)
	MechStatus(mech) &= ~SLITE_ON;

    DOCHECK(MechStatus(mech) & COMBAT_SAFE,
	"You cannot use a searchlight while combatsafe.");

    DOCHECK(MechCritStatus(mech) & SLITE_DEST,
	"Your searchlight has been destroyed already!");

    if (SearchlightChanging(mech)) {
	if (MechStatus(mech) & SLITE_ON)
	    mech_notify(mech, MECHALL,
		"Your searchlight is already in the process of turning off.");
	else
	    mech_notify(mech, MECHALL,
		"Your searchlight is already in the process of turning on.");

	return;
    }

    if (MechStatus(mech) & SLITE_ON) {
	mech_notify(mech, MECHALL,
	    "Your searchlight starts to cool down.");
    MECHEVENT(mech, EVENT_SLITECHANGING, MechSliteChangeEvent, 5, 0);
    } else {
	mech_notify(mech, MECHALL, "Your searchlight starts to warm up.");
	MECHEVENT(mech, EVENT_SLITECHANGING, MechSliteChangeEvent, 5, 1);
    }
}

void mech_ams(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);

    SILLY_TOGGLE_MACRO(IS_ANTI_MISSILE_TECH | CL_ANTI_MISSILE_TECH,
	AMS_ENABLED, "Anti-Missile System turned ON",
	"Anti-Missile System turned OFF",
	"This mech is not equipped with AMS");
}

void mech_fliparms(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    DOCHECK(Fallen(mech),
	"You're using your arms to support yourself. Try flipping something else.");
    SILLY_TOGGLE_MACRO(FLIPABLE_ARMS, FLIPPED_ARMS,
	"Arms have been flipped to BACKWARD position",
	"Arms have been flipped to FORWARD position",
	"You cannot flip the arms in this mech");
}

/* Parameters:
   <player,mech,buffer> = parent's input
   nspecisspec, nspec
   6 = check that weapon's ATM
   5 = check that weapon's SRM or LRM for DFM.
   4 = check that weapon's SRM (or SSRM)
   3 = check that weapon's NARC beacon
   2 = check that weapon's LRM
   1 = compare nspec to weapon's special
   0 = compare nspec to weapon's type and ensure it isn't NARCbcn
   -1 = compare nspec to weapon's type & check for Artemis
   mode           = mode to set if nspec check is successful
   onmsg          = msg for turning mode on
   offmsg         = msg for turning mode off
   cant           = system lacks nspec
 */

static int temp_nspecisspec, temp_nspec, temp_mode, temp_norec;
static char *temp_onmsg, *temp_offmsg, *temp_cant;

static int mech_toggle_mode_sub_func(MECH * mech, dbref player, int index,
    int high)
{
    int section, critical, weaptype, exclude = 0;
    int tsection = -1, tcritical = -1;

    weaptype =
	FindWeaponNumberOnMech_Advanced(mech, index, &section, &critical,
	1);
    DOCHECK0(weaptype == -1,
	"The weapons system chirps: 'Illegal Weapon Number!'");
    DOCHECK0(weaptype == -2,
	"The weapons system chirps: 'That Weapon has been destroyed!'");
#if 0
    DOCHECK0(weaptype == -3,
	"The weapon system chirps: 'That weapon is still reloading!'");
    DOCHECK0(weaptype == -4,
	"The weapon system chirps: 'That weapon is still recharging!'");
    DOCHECK0(weaptype == -5,
	"The limb holding that weapon is still retracting from an attack!");
#endif
    DOCHECK0(GetPartData(mech, section, critical) > 0,
	"You cannot change modes while recycling.");
    DOCHECK0(GetPartDamage(mech, section, WeaponFirstCrit(mech, section, critical)) & MTX_MODELOCK , "The ammo feed mechanism is damaged!");
    DOCHECK0(MechWeapons[weaptype].special & ELRM,
	"ELRM's weaponmode cannot be altered!");
    DOCHECK0(((GetPartMode(mech, section, critical) & CAPPED_MODE) && MechWeapons[weaptype].type == TBEAM),
	"You dare not muck around with such volatile equipment!");
    DOCHECK0(((GetPartMode(mech, section, critical) & (HOTLOAD_MODE|DEADFIRE_MODE)) && (GetPartData(mech, section, critical) > 0)),
	"Your ammo bins are still reloading from your last salvo!");
    DOCHECK0(GetPartMode(mech, section, critical) & OS_MODE,
	"One-shot weapons' mode cannot be altered!");
    if (temp_nspecisspec == -1)
	DOCHECK0(!FindArtemisForWeapon(mech, section, critical),
	    "You have no Artemis system for that weapon.");
    weaptype = Weapon2I(GetPartType(mech, section, critical));
/*    if (GetPartMode(mech, section, critical) & ULTRA_MODE || GetPartMode(mech, section, critical) & PIERCE_MODE)
	{
	mech_notify(mech, MECHALL, tprintf("Weapon %d has been set to normal fire mode.", index));
	GetPartMode(mech, section, critical) &= ~ULTRA_MODE;
	GetPartMode(mech, section, critical) &= ~PIERCE_MODE;
	return 0;
	} */
    if ((temp_nspecisspec == 6 && (MechWeapons[weaptype].special & ATM)) ||
        (temp_nspecisspec == 5 && (MechWeapons[weaptype].type == TMISSILE && (strstr(MechWeapons[weaptype].name, "IS.LRM-")
	|| strstr(MechWeapons[weaptype].name, "IS.SRM-")))) ||
	(temp_nspecisspec == 4 && (MechWeapons[weaptype].type == TMISSILE)
	    && !(MechWeapons[weaptype].type & (IDF | DAR))) ||
	(temp_nspecisspec == 2 && (MechWeapons[weaptype].special & IDF) &&
	    !(MechWeapons[weaptype].special & DAR)) ||
	(temp_nspecisspec == 1 && temp_nspec &&
	    (MechWeapons[weaptype].special & temp_nspec)) ||
	(temp_nspecisspec <= 0 && temp_nspec &&
	    (MechWeapons[weaptype].type == temp_nspec &&
		!(MechWeapons[weaptype].special & NARC)))) {

	if (temp_nspecisspec == 0 && (temp_nspec & TARTILLERY))
	    DOCHECK0((GetPartMode(mech, section,
			critical) & ARTILLERY_MODES) &&
		!(GetPartMode(mech, section, critical) & temp_mode),
		"That weapon has already been set to fire special rounds!");
	if (GetPartMode(mech, section, critical) & temp_mode) {
          if (MechWeapons[weaptype].type != TBEAM) {
            FindAmmoForWeapon_sub(mech, weaptype, 0, &tsection, &tcritical, 0, GetPartMode(mech, section, critical) & AMMO_MODES);
	    if (tsection > -1 && tcritical > -1 && (GetPartData(mech, tsection, tcritical)) && !temp_norec)
	        SetRecyclePart(mech, section, critical, GunStat(weaptype, GetPartMode(mech, section, critical), GUN_VRT));
	} else
	    if (!temp_norec)
	        SetRecyclePart(mech, section, critical, GunStat(weaptype, GetPartMode(mech, section, critical), GUN_VRT));
	    GetPartMode(mech, section, critical) &= ~temp_mode;
	    mech_notify(mech, MECHALL, tprintf(temp_offmsg, index));
	    return 0;
	}
       if ((GetPartMode(mech, section, critical) & (PRECISION_MODE|PIERCE_MODE|CASELESS_MODE|TRACER_MODE)) && (temp_mode & ULTRA_MODE))
           exclude = ((WEAPMODE_EXCLUDE) & ~(PRECISION_MODE|PIERCE_MODE|CASELESS_MODE|TRACER_MODE));
	else
	   exclude = WEAPMODE_EXCLUDE;
        if (MechWeapons[weaptype].type != TBEAM) {
            FindAmmoForWeapon_sub(mech, weaptype, 0, &tsection, &tcritical, 0, GetPartMode(mech, section, critical) & AMMO_MODES);
	    if (tsection > -1 && tcritical > -1 && (GetPartData(mech, tsection, tcritical)) && !temp_norec)
	        SetRecyclePart(mech, section, critical, GunStat(weaptype, GetPartMode(mech, section, critical), GUN_VRT));
	    } else
	    if (!temp_norec)
	        SetRecyclePart(mech, section, critical, GunStat(weaptype, GetPartMode(mech, section, critical), GUN_VRT));
	GetPartMode(mech, section, critical) &= ~(exclude);
	GetPartMode(mech, section, critical) |= temp_mode;
	mech_notify(mech, MECHALL, tprintf(temp_onmsg, index));
	return 0;
    }
    notify(player, temp_cant);
    return 0;
}

static void mech_toggle_mode_sub(dbref player, MECH * mech, char *buffer,
    int nspecisspec, int nspec, int mode, char *onmsg, char *offmsg,
    char *cant, int norec)
{
    char *args[1];

    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"Please specify a weapon number.");
    temp_norec = norec;
    temp_nspecisspec = nspecisspec;
    temp_nspec = nspec;
    temp_mode = mode;
    temp_onmsg = onmsg;
    temp_offmsg = offmsg;
    temp_cant = cant;
    multi_weap_sel(mech, player, args[0], 1, mech_toggle_mode_sub_func);
}

void mech_ultra(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, ULTRA, ULTRA_MODE,
	"Weapon %d has been set to ultra fire mode",
	"Weapon %d has been set to normal fire mode",
	"That weapon cannot be set ULTRA!",
	1);
}

void mech_rottwo(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, ROTARY, ULTRA_MODE,
        "Weapon %d has been set to rotary fire mode",
        "Weapon %d has been set to normal fire mode",
        "That weapon cannot be set ROTARY TWOSHOT!",
        1);
}

void mech_rotfour(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, ROTARY, ROTFOUR_MODE,
        "Weapon %d has been set to rotary fire mode",
        "Weapon %d has been set to normal fire mode",
        "That weapon cannot be set ROTARY FOURSHOT!",
	1);
}

void mech_rotsix(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, ROTARY, ROTSIX_MODE,
        "Weapon %d has been set to rotary fire mode",
        "Weapon %d has been set to normal fire mode",
        "That weapon cannot be set ROTARY SIXSHOT!",
	1);
}



void mech_heat(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, CHEAT, HEAT_MODE,
	"Weapon %d has been set to heat mode",
        "Weapon %d has been set to normal mode",
	"That weapon cannot be set HEAT!",
	1);
}

void mech_explosive(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);

    mech_toggle_mode_sub(player, mech, buffer, 1, NARC, NARC_MODE,
	"Weapon %d has been set to fire explosive rounds",
	"Weapon %d has been set to fire NARC beacons",
	"That weapon cannot be set to fire explosive rounds!",
	1);
}

void mech_lbx(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, LBX, LBX_MODE,
	"Weapon %d has been set to LBX fire mode",
	"Weapon %d has been set to normal fire mode",
	"That weapon cannot be set LBX!",
	1);
}

void mech_caseless(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, AC, CASELESS_MODE,
        "Weapon %d has been set to caseless fire mode",
        "Weapon %d has been set to normal fire mode",
        "That weapon cannot be set CASELESS!",
	1);
}
void mech_pierce(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, AC, PIERCE_MODE,
	"Weapon %d has been set to pierce fire mode",
	"Weapon %d has been set to normal fire mode",
	"That weapon cannot be set PIERCE!",
	1);
}
void mech_precision(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, AC, PRECISION_MODE,
        "Weapon %d has been set to precision fire mode",
        "Weapon %d has been set to normal fire mode",
        "That weapon cannot be set PRECISION!",
	1);
}
void mech_tracer(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 1, AC, TRACER_MODE,
        "Weapon %d has been set to tracer fire mode",
        "Weapon %d has been set to normal fire mode",
        "That weapon cannot be set TRACER!",
	1);
}
void mech_sguided(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 2, 0, SGUIDED_MODE,
        "Weapon %d has been set to fire semi-guided missiles.",
        "Weapon %d has been set to fire normal missiles",
        "That weapon cannot be set SGUIDED!",
	1);
}

void mech_stinger(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 2, 0, STINGER_MODE,
        "Weapon %d has been set to fire stinger missiles.",
        "Weapon %d has been set to fire normal missiles",
        "That weapon cannot be set STINGER!",
	1);
}

void mech_deadfire(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 5, 0, DEADFIRE_MODE,
        "Weapon %d has been set to fire deadfire missiles.",
        "Weapon %d has been set to fire normal missiles",
        "That weapon cannot be set DEADFIRE!",
	1);
}
void mech_atmer(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 6, 0, ATMER_MODE,
        "Weapon %d has been set to fire ER ATM missiles.",
        "Weapon %d has been set to fire normal missiles",
        "That weapon cannot be set ATMER!",
	1);
}
void mech_atmhe(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 6, 0, ATMHE_MODE,
        "Weapon %d has been set to fire HE ATM missiles.",
        "Weapon %d has been set to fire normal missiles",
        "That weapon cannot be set ATMHE!",
	1);
}
void mech_artemis(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, -1, TMISSILE, ARTEMIS_MODE,
	"Weapon %d has been set to fire Artemis IV compatible missiles.",
	"Weapon %d has been set to fire normal missiles",
	"That weapon cannot be set ARTEMIS!",
	1);
}

void mech_narc(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 0, TMISSILE, NARC_MODE,
	"Weapon %d has been set to fire Narc Beacon compatible missiles.",
	"Weapon %d has been set to fire normal missiles",
	"That weapon cannot be set NARC!",
	1);
}

void mech_swarm(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 2, 0, SWARM_MODE,
	"Weapon %d has been set to fire Swarm missiles.",
	"Weapon %d has been set to fire normal missiles",
	"That weapon cannot be set to fire Swarm missiles!",
	1);
}

void mech_swarm1(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 2, 0, SWARM1_MODE,
	"Weapon %d has been set to fire Swarm1 missiles.",
	"Weapon %d has been set to fire normal missiles",
	"That weapon cannot be set to fire Swarm1 missiles!",
	1);
}

void mech_inferno(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 4, 0, INFERNO_MODE,
	"Weapon %d has been set to fire Inferno missiles.",
	"Weapon %d has been set to fire normal missiles",
	"That weapon cannot be set to fire Inferno missiles!",
	1);
}

void mech_hotload(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 2, 0, HOTLOAD_MODE,
	"Hotloading for weapon %d has been toggled on.",
	"Hotloading for weapon %d has been toggled off.",
	"That weapon cannot be set to hotload!",
	1);
}

void mech_cluster(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 0, TARTILLERY, CLUSTER_MODE,
	"Weapon %d has been set to fire cluster rounds.",
	"Weapon %d has been set to fire normal rounds",
	"Invalid weapon type!",
	1);
}

void mech_smoke(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 0, TARTILLERY, SMOKE_MODE,
	"Weapon %d has been set to fire smoke rounds.",
	"Weapon %d has been set to fire normal rounds",
	"Invalid weapon type!",
	1);
}

void mech_mine(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 0, TARTILLERY, MINE_MODE,
	"Weapon %d has been set to fire mine rounds.",
	"Weapon %d has been set to fire normal rounds",
	"Invalid weapon type!",
	1);
}

void mech_incendarty(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    cch(MECH_USUALMO);
    mech_toggle_mode_sub(player, mech, buffer, 0, TARTILLERY, INCEND_MODE,
	"Weapon %d has been set to fire indcendiary rounds.",
	"Weapon %d has been set to fire normal rounds.",
	"Invalid weapon type!",
	1);
}

static void mech_mascr_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (MechMASCCounter(mech) > 0) {
	MechMASCCounter(mech)--;
	MECHEVENT(mech, EVENT_MASC_REGEN, mech_mascr_event, MASC_TICK, 0);
    }
}

static void mech_scharger_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    if (MechSChargeCounter(mech) > 0) {
	MechSChargeCounter(mech)--;
	MECHEVENT(mech, EVENT_SCHARGE_REGEN, mech_scharger_event, MASC_TICK, 0);
    }
}

static void mech_masc_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int needed = ((2 * (1 + (MechMASCCounter(mech)++))) + ((MechStatus2(mech) & SCHARGE_ENABLED) ? 1 : 0) + (MechStatus2(mech) & SPRINTING ? 2 : 0));
    int roll = Roll();
    int canll = 0, canrl = 0, canrll = 0, canrrl = 0, i = 0;

    if (!Started(mech) || !(MechSpecials(mech) & MASC_TECH)) {
	MECHEVENT(mech, EVENT_MASC_REGEN, mech_mascr_event, MASC_TICK, 0);
	MechStatus(mech) &= ~MASC_ENABLED;
	return;
	}
    if (needed < 10 && Good_obj(MechPilot(mech)) && WizP(MechPilot(mech)))
	roll = Number(needed + 1, 12);
    mech_notify(mech, MECHALL, tprintf("MASC: BTH %d+, Roll: %d", needed + 1, roll));
    if (roll > needed) {
	MECHEVENT(mech, EVENT_MASC_FAIL, mech_masc_event, MASC_TICK, 0);
	return;
    }
    MechSpecials(mech) &= ~MASC_TECH;
    MechStatus(mech) &= ~MASC_ENABLED;
    for (i = 0; i < 4; i++) {
	if (!PartIsNonfunctional(mech, LLEG, i) && IsActuator(GetPartType(mech, LLEG, i)))
	    canll = 1;
	if (!PartIsNonfunctional(mech, RLEG, i) && IsActuator(GetPartType(mech, RLEG, i)))
	    canrl = 1;
	if (MechMove(mech) == MOVE_QUAD) {
            if (!PartIsNonfunctional(mech, LARM, i) && IsActuator(GetPartType(mech, LARM, i)))
                canrll = 1;
	    if (!PartIsNonfunctional(mech, RARM, i) && IsActuator(GetPartType(mech, RARM, i)))
                canrrl = 1;
   	    }
	}
    if ((!mudconf.btech_masc) || (!canll && !canrl && !canrll && !canrrl)) {
        if (fabs(MechSpeed(mech)) > MP1) {
    	    mech_notify(mech, MECHALL, "Your leg actuators freeze suddenly, and you fall!");
	    MechLOSBroadcast(mech, "stops and falls in mid-step!");
	    MechFalls(mech, 1, 0);
        } else {
	    mech_notify(mech, MECHALL, "Your leg actuators freeze suddenly!");
	    if (MechSpeed(mech) > 0.0)
	        MechLOSBroadcast(mech, "stops suddenly!");
        }
        SetMaxSpeed(mech, 0.0);
        MechDesiredSpeed(mech) = 0.0;
        MechSpeed(mech) = 0.0;
    } else {
	mech_notify(mech, MECHALL, "You suddenly notice less response from your leg actuators!");
	MechLOSBroadcast(mech, "jinks around a little and slows down.");
	if (canrl)
	    while (canrl) {
		if (!PartIsNonfunctional(mech, RLEG, (i = (Number(0, 3))))) {
	            HandleMechCrit(mech, NULL, 0, RLEG, i, GetPartType(mech, RLEG, i), GetPartData(mech, RLEG, i), 1, 1);
		    canrl = 0;
		    }
	        }
        if (canll)
            while (canll) {
                if (!PartIsNonfunctional(mech, LLEG, (i = (Number(0, 3))))) {
                    HandleMechCrit(mech, NULL, 0, LLEG, i, GetPartType(mech, LLEG, i), GetPartData(mech, LLEG, i), 1, 1);
                    canll = 0;
                    }
                }
        if (canrrl)
            while (canrrl) {
                if (!PartIsNonfunctional(mech, RARM, (i = (Number(0, 3))))) {
                    HandleMechCrit(mech, NULL, 0, RARM, i, GetPartType(mech, RARM, i), GetPartData(mech, RARM, i), 1, 1);
                    canrrl = 0;
                    }
                }
        if (canrll)
            while (canrll) {
                if (!PartIsNonfunctional(mech, LARM, (i = (Number(0, 3))))) {
                    HandleMechCrit(mech, NULL, 0, LARM, i, GetPartType(mech, LARM, i), GetPartData(mech, LARM, i), 1, 1);
                    canrll = 0;
                    }
                }
	if (MechDesiredSpeed(mech) > MMaxSpeed(mech))
	    MechDesiredSpeed(mech) = MMaxSpeed(mech);

	}
}

static void mech_scharge_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    int needed = ((2 * (1 + (MechSChargeCounter(mech)++))) + ((MechStatus(mech) & MASC_ENABLED) ? 1 : 0));
    int roll = Roll();
    int crits = 0, i, ii;

    if (!Started(mech) || !(MechSpecials2(mech) & SCHARGE_TECH)) {
	MechStatus2(mech) &= ~SCHARGE_ENABLED;
        MECHEVENT(mech, EVENT_SCHARGE_REGEN, mech_scharger_event, MASC_TICK, 0);
        return;
	}
    if (needed < 10 && Good_obj(MechPilot(mech)) && WizP(MechPilot(mech)))
        roll = Number(needed + 1, 12);
    mech_notify(mech, MECHALL, tprintf("SCHARGE: BTH %d+, Roll: %d",
            needed + 1, roll));
    if (roll > needed) {
        MECHEVENT(mech, EVENT_SCHARGE_FAIL, mech_scharge_event, MASC_TICK, 0);
        return;
    }
    MechSpecials2(mech) &= ~SCHARGE_TECH;
    MechStatus2(mech) &= ~SCHARGE_ENABLED;
    if (MechType(mech) != CLASS_MECH)
	{
        mech_notify(mech, MECHALL, "%ch%cyCRITICAL HIT!!%c");
        mech_notify(mech, MECHALL, "Your engine putters as your supercharger damages its working parts! Your speed is halved!");
	MechLOSBroadcast(mech, "spews extensive amounts of smoke out it's exhaust and slows down.");
        DivideMaxSpeed(mech, 2);
	} else {
	mech_notify(mech, MECHALL, "Your supercharger is overstressed and explodes!");
	    for (i = 0; i < NUM_SECTIONS; i++)
		for (ii = 0; ii < NUM_CRITICALS; ii++)
		    if (IsSpecial(GetPartType(mech, i, ii)) && !PartIsNonfunctional(mech, i, ii))
			if (Special2I(GetPartType(mech, i, ii)) == SUPERCHARGER)
			    HandleMechCrit(mech, NULL, 0, i, ii, GetPartType(mech, i, ii), GetPartData(mech, i, ii), 1, 1);
	crits = (Roll());
	crits = (crits > 11 ? 3 : crits > 9 ? 2 : crits > 7 ? 1 : 0);
	if (crits)
	    for (i = 0; i < NUM_CRITICALS && crits > 0; i++)
		{
		if (IsSpecial(GetPartType(mech, CTORSO, i)) && !PartIsNonfunctional(mech, CTORSO, i))
		    if (Special2I(GetPartType(mech, CTORSO,i)) == ENGINE) {
		        HandleMechCrit(mech, NULL, 0, CTORSO, i, GetPartType(mech, CTORSO, i), GetPartData(mech, CTORSO, i), 1, 1);
			crits--;
			}
		}
	}
    if (MechDesiredSpeed(mech) > MMaxSpeed(mech))
            MechDesiredSpeed(mech) = MMaxSpeed(mech);
}

void mech_masc(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    DOCHECK(!(MechSpecials(mech) & MASC_TECH),
	"Your toy ain't prepared for what you're askin' it!");
    DOCHECK(MMaxSpeed(mech) < MP1, "Uh huh.");
    if (MechStatus(mech) & MASC_ENABLED) {
	mech_notify(mech, MECHALL, "MASC has been turned off.");
	MechStatus(mech) &= ~MASC_ENABLED;
	MechDesiredSpeed(mech) = MechDesiredSpeed(mech) * 3. / 4.;
	StopMasc(mech);
	MECHEVENT(mech, EVENT_MASC_REGEN, mech_mascr_event, MASC_TICK, 0);
	return;
    }
    mech_notify(mech, MECHALL, "MASC has been turned on.");
    MechStatus(mech) |= MASC_ENABLED;
    StopMascR(mech);
    MechDesiredSpeed(mech) = MechDesiredSpeed(mech) * 4. / 3.;
    MECHEVENT(mech, EVENT_MASC_FAIL, mech_masc_event, 1, 0);
}

void mech_scharge(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    cch(MECH_USUALMO);
    DOCHECK(!(MechSpecials2(mech) & SCHARGE_TECH),
        "Your toy ain't prepared for what you're askin' it!");
    DOCHECK(MMaxSpeed(mech) < MP1, "Uh huh.");
    if (MechStatus2(mech) & SCHARGE_ENABLED) {
        mech_notify(mech, MECHALL, "SuperCharger has been turned off.");
        MechStatus2(mech) &= ~SCHARGE_ENABLED;
        MechDesiredSpeed(mech) = MechDesiredSpeed(mech) * 3. / 4.;
        StopSCharge(mech);
        MECHEVENT(mech, EVENT_SCHARGE_REGEN, mech_scharger_event, MASC_TICK, 0);
        return;
    }
    mech_notify(mech, MECHALL, "SuperCharger has been turned on.");
    MechStatus2(mech) |= SCHARGE_ENABLED;
    StopSChargeR(mech);
    MechDesiredSpeed(mech) = MechDesiredSpeed(mech) * 4. / 3.;
    MECHEVENT(mech, EVENT_SCHARGE_FAIL, mech_scharge_event, 1, 0);
}

int doing_explode = 0;

static void mech_explode_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;
    MAP *map;
    int extra = (int) e->data2;
    int i, j, damage;
    int z;
    int dam;

    if (Destroyed(mech))
	return;
    if (extra > 256 && !FindDestructiveAmmo(mech, &i, &j))
	return;
    if ((--extra) % 256) {
	mech_notify(mech, MECHALL,
	    tprintf("Self-destruction in %d second%s..", extra % 256,
		extra > 1 ? "s" : ""));
	MECHEVENT(mech, EVENT_EXPLODE, mech_explode_event, 1, extra);
    } else {
	SendDebug(tprintf("#%d explodes.", mech->mynum));
	if (extra >= 256) {
	    SendDebug(tprintf("#%d explodes [ammo]", mech->mynum));
	    mech_notify(mech, MECHALL, "All your ammo explodes!");
	    while ((damage = FindDestructiveAmmo(mech, &i, &j)))
		ammo_explosion(mech, mech, i, j, damage);
	} else {
	    SendDebug(tprintf("#%d explodes [reactor]", mech->mynum));
	    MechLOSBroadcast(mech, "suddenly explodes!");
	    doing_explode = 1;
	    mech_notify(mech, MECHALL,
		"Suddenly you feel great heat overcoming your senses.. you faint.. (and die)");
	    z = MechZ(mech);
	    map = FindObjectsData(mech->mapindex);
	    DestroySection(mech, mech, -1, LTORSO);
	    DestroySection(mech, mech, -1, RTORSO);
	    DestroySection(mech, mech, -1, CTORSO);
	    DestroySection(mech, mech, -1, HEAD);
	    DestroySection(mech, mech, -1, LLEG);
	    DestroySection(mech, mech, -1, RLEG);
	    MechZ(mech) += 6;
	    doing_explode = 0;
	    if (mudconf.btech_engine > 1)
		dam = MAX(MechTons(mech) / 5, MechEngineSize(mech) / 15);
	    else
		dam = MAX(MechTons(mech) / 5, MechEngineSize(mech) / 10);
	    if (mech->mapindex > 0)
		blast_hit_hexesf(map, dam, 1, MAX(MechTons(mech) / 10, MechEngineSize(mech) / 25),
		    MechFX(mech), MechFY(mech), MechFX(mech), MechFY(mech),
		    "%ch%crYou bear full brunt of the blast!%cn",
		    "is hit badly by the blast!",
		    "%ch%cyYou receive some damage from the blast!%cn",
		    "is hit by the blast!", mudconf.btech_engine > 1,
		    mudconf.btech_engine > 1 ? 5 : 3, 5, 1, 2, NULL, NULL);
	    MechZ(mech) = z;
	    headhitmwdamage(mech, 4);
	}
    }
}

void mech_explode(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[3];
    int i;
    int ammoloc, ammocritnum;
    int time = 60;
    int ammo = 1;
    int argc;

    cch(MECH_USUALO);
    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc != 1, "Invalid number of arguments!");
    DOCHECK(!mudconf.btech_engine,
	"The command has been disabled for now. Sorry.");
/*
    if (!strcasecmp(buffer, "stop")) {
	DOCHECK(!Exploding(mech),
	    "Your mech isn't undergoing a self-destruct sequence!");

	StopExploding(mech);
	mech_notify(mech, MECHALL, "Self-destruction sequence aborted.");
	SendDebug(tprintf
	    ("#%d in #%d stopped the self-destruction sequence.", player,
		mech->mynum));
	return;
    }*/
    DOCHECK(Exploding(mech),
	"Your mech is already undergoing a self-destruct sequence!");
/*    if (!strcasecmp(buffer, "ammo")) {

	   Find SOME ammo to explode ; if possible, we engage the 'boom' process

	i = FindDestructiveAmmo(mech, &ammoloc, &ammocritnum);
	DOCHECK(!i, "There is no 'damaging' ammo on your 'mech!");
	time = 2;
	SendDebug(tprintf
	    ("#%d in #%d initiates the ammo explosion sequence.", player,
		mech->mynum));
	MECHEVENT(mech, EVENT_EXPLODE, mech_explode_event, 1, 256 + time);
    } else {*/
	DOCHECK(MechType(mech) != CLASS_MECH,
	    "Only mechs can do the 'big boom' effect.");
	DOCHECK(MechSpecials(mech) & ICE_TECH, "You need a fusion reactor.");
        DOCHECK(Jumping(mech),
            "You are too busy controlling your jump right now.");	

	/* only explode while nothing cycling */
	if ((i = MechFullNoRecycle(mech, CHECK_BOTH)) > 0) {
            mech_notify(mech, MECHALL, tprintf("You have %s recycling!", (i == 1 ? "weapons" : i == 2 ? "limbs" : "error")
));
            return;
        }

	time = 120;
	SendDebug(tprintf
	    ("#%d in #%d initiates the reactor explosion sequence.",
		player, mech->mynum));
	MECHEVENT(mech, EVENT_EXPLODE, mech_explode_event, 1, time);
	ammo = 0;
	MechLOSBroadcast(mech, "loses reactions containment!");
    mech_notify(mech, MECHALL,
	"Self-destruction sequence engaged ; please stand by.");
    mech_notify(mech, MECHALL, tprintf("%s in %d seconds.",
	    ammo ? "The ammunition will explode" :
	    "The reactor will blow up", time));
	MechPilot(mech) = -1;
}

static void mech_dig_event(EVENT * e)
{
    MECH *mech = (MECH *) e->data;

    if (!Digging(mech))
	return;
    MechCritStatus(mech) &= ~DIGGING_IN;
    MechCritStatus(mech) |= DUG_IN;
    if (MechMove(mech) == MOVE_QUAD && MechType(mech) == CLASS_MECH)
	{
	mech_notify(mech, MECHALL, "You complete you hull down maneuver.");
	MechLOSBroadcast(mech, "hunkers down into a hull down position.");
	} else {
	mech_notify(mech, MECHALL, "You finish burrowing for cover - only turret weapons are available now.");
	MechLOSBroadcast(mech, "digs itself into the ground.");
	}
}

void mech_dig(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;

    int quad = (MechMove(mech) == MOVE_QUAD && MechType(mech) == CLASS_MECH);

    cch(MECH_USUALO);
    DOCHECK(MechType(mech) == CLASS_MECH && !(MechMove(mech) == MOVE_QUAD), "You just don't quite have the right angle up top....");
    DOCHECK(MechMove(mech) == MOVE_HOVER, "Dig in with what?");
    DOCHECK(fabs(MechSpeed(mech)) > 0.0, "You are moving!");
    DOCHECK(MechFacing(mech) != MechDesiredFacing(mech),
	"You are turning!");
    DOCHECK(MechMove(mech) == MOVE_NONE, "You are stationary!");
    DOCHECK(MechCritStatus(mech) & DUG_IN, "You are already dug in!");
    DOCHECK(Digging(mech), "You are already digging!");
    DOCHECK(OODing(mech), "While dropping? I think not.");
    DOCHECK((MechRTerrain(mech) == ROAD && !quad) ,
	"The surface is slightly too hard for you to dig in.");
    DOCHECK((MechRTerrain(mech) == BUILDING && !quad),
	"The surface is slightly too hard for you to dig in.");
    DOCHECK((MechRTerrain(mech) == WALL && !quad),
	"The surface is slightly too hard for you to dig in.");
    DOCHECK((MechRTerrain(mech) == WATER && !quad), "In water? Who are you kidding?");

    MechCritStatus(mech) |= DIGGING_IN;
    MECHEVENT(mech, EVENT_DIG, mech_dig_event, 20, 0);
    if (quad)
	mech_notify(mech, MECHALL, "You start hunkering into a hull down position.");
    else
	mech_notify(mech, MECHALL, "You start digging yourself in a nice hole..");
}

static int mech_disableweap_func(MECH * mech, dbref player, int index, int high)
{
    int section, critical, weaptype;
    int IsPPC;

    weaptype = FindWeaponNumberOnMech_Advanced(mech, index, &section, &critical, 1);
    DOCHECK0(weaptype == -1, "The weapons system chirps: 'Illegal Weapon Number!'");
    DOCHECK0(weaptype == -2, "The weapons system chirps: 'That Weapon has been destroyed!'");
    weaptype = Weapon2I(GetPartType(mech, section, critical));
    IsPPC = (GunStat(weaptype, GetPartMode(mech, section, critical), GUN_MINRANGE) &&
	     MechWeapons[weaptype].type == TBEAM && strstr(MechWeapons[weaptype].name, "PPC"));
    DOCHECK0(!(MechWeapons[weaptype].special & GAUSS || IsPPC), "You can only disable Gauss or Field Inhibited (PPC) weapons.");
    if (IsPPC) {
        if (GetPartMode(mech, section, critical) & HOTLOAD_MODE) {
                GetPartMode(mech, section, critical) &= ~(AMMO_MODES);
		GetPartMode(mech, section, critical) &= ~HOTLOAD_MODE;
		mech_notify(mech, MECHALL, tprintf("You enable weapon %d's field inhibitor!", index));
		return 0;
		} else {
                GetPartMode(mech, section, critical) &= ~(AMMO_MODES);
		GetPartMode(mech, section, critical) |= HOTLOAD_MODE;
		mech_notify(mech, MECHALL, tprintf("You disable weapon %d's field inhibitor!", index));
		return 0;
		}
	} else {
	int i;
	for (i = critical; i < critical + GetWeaponCrits(mech, weaptype); i++)
	    DisablePart(mech, section, i);
        mech_notify(mech, MECHALL, tprintf("You power down weapon %d.", index));
        return 0;
	}
    return 0;
}

static int mech_capacitate_func(MECH * mech, dbref player, int index, int high)
{
    int section, critical, weaptype;
    int ccrit = 0;
    int IsPPC;

    weaptype =
        FindWeaponNumberOnMech_Advanced(mech, index, &section, &critical,
        1);
    DOCHECK0((MechCritStatus(mech) & CREW_STUNNED),
	"You can't seem to get your wits together enough to manage the controls....");
    DOCHECK0(weaptype == -1,
        "The weapons system chirps: 'Illegal Weapon Number!'");
    DOCHECK0(weaptype == -2,
        "The weapons system chirps: 'That Weapon has been destroyed!'");
    DOCHECK0(GetPartData(mech, section, critical),
	"The weapons system chirps: 'That Weapon is otherwise occupied!'");
    weaptype = Weapon2I(GetPartType(mech, section, critical));
    IsPPC = (MechWeapons[weaptype].type == TBEAM && strstr(MechWeapons[weaptype].name, "PPC"));
    DOCHECK0(!IsPPC,
        "You can only capacitate PPC weapons!.");
    DOCHECK0(!(ccrit = FindCapacitorForWeapon(mech, section, critical)),
	"You cannot capacitate that weapon!");
    ccrit--;
    if ((GetPartMode(mech, section, critical) & CAPPED_MODE) && (GetPartMode(mech, section, ccrit) & CAPPED_MODE))
            {
/*            GetPartMode(mech, section, critical) &= ~(AMMO_MODES); */
            GetPartMode(mech, section, critical) &= ~CAPPED_MODE;
	    GetPartMode(mech, section, ccrit) &= ~CAPPED_MODE;
	    MechStatus2(mech) &= ~CAPACITOR_ON;
	    MechWeapHeat(mech) += 5;
            mech_notify(mech, MECHALL, tprintf("You drain weapon %d's capacitor!", index));
            return 0;
            } else {
/*            GetPartMode(mech, section, critical) &= ~(AMMO_MODES); */
            GetPartMode(mech, section, critical) |= CAPPED_MODE;
	    SetRecyclePart(mech, section, critical, GunStat(weaptype, GetPartMode(mech, section, critical), GUN_VRT));
	    GetPartMode(mech, section, ccrit) |= CAPPED_MODE;
	    MechStatus2(mech) |= CAPACITOR_ON;
            mech_notify(mech, MECHALL, tprintf("You begin to charge weapon %d's capacitor!", index));
            return 0;
            }
    return 0;
}

static int mech_unjamweap_func(MECH * mech, dbref player, int index,
    int high)
{
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int section, crit, weaptype, data, succ;
    int i = 0, weapcount = 0, loop;

    weaptype = FindWeaponNumberOnMech_Advanced(mech, index, &section, &crit, 1);
    DOCHECK0(weaptype == -1,
        "The weapons system chirps: 'Illegal Weapon Number!'");
    DOCHECK0(weaptype == -2,
        "The weapons system chirps: 'That Weapon has been destroyed!'");
    weaptype = Weapon2I(GetPartType(mech, section, crit));
    data = GetPartData(mech, section, crit);
    DOCHECK0(Jumping(mech), "You want to do what while in midair?");
    DOCHECK0((MechCritStatus(mech) & CREW_STUNNED), "Your stunned and can't find your brains much less the jammed gun.");
    DOCHECK0(IsRunning(MechDesiredSpeed(mech), MMaxSpeed(mech)), tprintf("Your bouncing around too much! Try slowing to %s speed!",
             ((MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ?
		"walking" : "cruising")));
    DOCHECK0(!(GetPartMode(mech, section, crit = WeaponFirstCrit(mech, section, crit)) & JAMMED_MODE),
        "You can only unjam jammed weapons!");
    DOCHECK0((MechStatus2(mech) & UNJAMMING), "Patience, jimmy it to fast and you might break it!");
    DOCHECK0(data, "You have to wait for the weapon to recycle first!");
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
      weapcount = FindWeapons(mech, loop, weaparray, weapdata, critical);
      for (i = 0; i < weapcount; i++)
	{
	if (weapdata[i] && WeaponIsNonfunctional(mech, loop, critical[i], 0) <= 0) {
	    mech_notify(mech, MECHALL, "You cannot unjam with weapons recycling!");
	    return 0;
	    }
       }
       if (MechSections(mech)[loop].recycle && MechType(mech) == CLASS_MECH)
	{
	mech_notify(mech, MECHALL, "You cannot unjam while recovering from physical attacks!");
	return 0;
	}
    }
    MechStatus2(mech) |= UNJAMMING;
    mech_notify(mech, MECHALL, "You start to jimmy the weapon in an attempt to unjam it.");
    MechLOSBroadcast(mech, "begins to furiously jimmy itself.");
    MECHEVENT(mech, EVENT_UNJAM, mech_unjam_event, GunStat(weaptype, GetPartMode(mech, section, crit), GUN_VRT), index);
    return 0;
}

void mech_disableweap(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1];

    cch(MECH_USUALO);
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"Please specify a weapon number.");

    multi_weap_sel(mech, player, args[0], 1, mech_disableweap_func);
}

void mech_capacitate(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1];

    cch(MECH_USUALO);
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"Please specify a weapon number.");
    DOCHECK(MoveModeLock(mech), "Movement modes disallow capacitation.");

    multi_weap_sel(mech, player, args[0], 1, mech_capacitate_func);
}

void mech_unjamweap(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1];
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int i, weapcount, loop;

    cch(MECH_USUALO);
    DOCHECK(MechSpotter(mech) > 0, "You cannot unjam while participating in spotting!");
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1, "Please specify a weapon number.");
        if ((args[0][0] == '-') && MechType(mech) != CLASS_MECH && (MechCritStatus(mech) & TURRET_JAMMED))
	    {
	    DOCHECK(Jumping(mech), "You want to do what while in midair?");
	    DOCHECK((MechCritStatus(mech) & CREW_STUNNED), "Your stunned and can't find your brains much less the jammed gun.");
	    DOCHECK(IsRunning(MechDesiredSpeed(mech), MMaxSpeed(mech)), tprintf("Your bouncing around too much! Try slowing to %s speed!",
	             ((MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_MW) ?
	                "walking" : "cruising")));
	    DOCHECK((MechStatus2(mech) & UNJAMMING), "Patience, jimmy it to fast and you might pop the lid off!");
	    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	      weapcount = FindWeapons(mech, loop, weaparray, weapdata, critical);
 	      for (i = 0; i < weapcount; i++)
	        {
        	if (weapdata[i]) {
	            mech_notify(mech, MECHALL, "You cannot unjam with weapons recycling!");
	            return;
	            }
	        }
	      }
	    MechStatus2(mech) |= UNJAMMING;
	    mech_notify(mech, MECHALL, "You start to jimmy your turret in an attempt to unjam it.");
	    MechLOSBroadcast(mech, "begins to furiously bounce its turret around.");
	    MECHEVENT(mech, EVENT_UNJAM, mech_unjam_event, UNJAM_TICK, -1);
	    return;
	    }
    multi_weap_sel(mech, player, args[0], 1, mech_unjamweap_func);
}

int FindMainWeapon(MECH * mech, int (*callback) (MECH *, int, int, int,
	int))
{
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    int count;
    int loop;
    int ii;
    int tempcrit;
    int maxcrit = 0;
    int maxloc = 0;
    int critfound = 0;
    int maxcount = 0;

    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	if (SectIsDestroyed(mech, loop))
	    continue;
	count = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (count > 0) {
	    for (ii = 0; ii < count; ii++) {
		if (!PartIsDestroyed(mech, loop, critical[ii])) {
		    /* tempcrit = GetWeaponCrits(mech, weaparray[ii]); */
		    tempcrit = rand();
		    if (tempcrit > maxcrit) {
			critfound = 1;
			maxcrit = tempcrit;
			maxloc = loop;
			maxcount = ii;
		    }
		}
	    }
	}
    }
    if (critfound)
	return callback(mech, maxloc, weaparray[maxcount], maxcount,
	    maxcrit);
    else
	return 0;
}
