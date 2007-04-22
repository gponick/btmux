#include <math.h>
#include <string.h>
#include "mech.h"
#include "event.h"
#include "mech.events.h"
#include "p.aero.bomb.h"
#include "mech.tech.h"
#include "p.mech.utils.h"
#include "p.mech.tech.h"
#include "p.mech.consistency.h"
#include "p.mech.tech.do.h"
#include "p.mech.build.h"
#include "p.template.h"
#include "p.econ.h"
#include "p.mech.status.h"
#include "p.mech.custom.h"

#define PARTCHECK_SUB(m,a,c) \
AVCHECKM2(m,a,c); \
GrabPartsM(m,a,c);

#define PARTCHECK(m,l,a,c) \
{ PARTCHECK_SUB(m, alias_part(m, a, l), c); }

#define my_parsepart(loc,part) \
switch (tech_parsepart(mech, buffer, loc, part,NULL)) \
{ case -1: notify(player, "Invalid section!");return; \
case -2: notify(player, "Invalid part!");return; }

#define my_parsepart2(loc,part,extra) \
switch (tech_parsepart(mech, buffer, loc, part,extra)) \
{ case -1: notify(player, "Invalid section!");return; \
case -2: notify(player, "Invalid part!");return; }

#define my_parsegun(loc,part) \
switch (tech_parsegun(mech, buffer, loc, part)) \
{ case -1: notify(player, "Invalid gun #!");return; \
  case -2: notify(player, "Invalid object to replace with!");return; \
  case -3: notify(player, "Invalid object type - not matching with original.");return; \
  case -4: notify(player, "Invalid gun location - subscript out of range.");return; }

#define ClanMod(num) \
  MAX(1, (((num) / ((MechSpecials(mech) & CLAN_TECH) ? 2 : 1))))

static int tmp_flag = 0;
static int tmp_loc;
static int tmp_part;

static void tech_check_locpart(EVENT * e)
{
    int loc, pos;
    int l = (int) e->data2;

    UNPACK_LOCPOS(l, loc, pos);
    if (loc == tmp_loc && pos == tmp_part)
	tmp_flag++;
}

static void tech_check_loc(EVENT * e)
{
    int loc;

    loc = (((int) e->data2) % 16);
    if (loc == tmp_loc)
	tmp_flag++;
}

#define CHECK(t,fun) \
  tmp_flag=0;tmp_loc=loc;tmp_part = part; \
  event_gothru_type_data(t, (void *) mech, fun); \
  return tmp_flag

#define CHECKL(t,fun) \
  tmp_flag=0;tmp_loc=loc; \
  event_gothru_type_data(t, (void *) mech, fun); \
  return tmp_flag

#define CHECK2(t,t2,fun) \
  tmp_flag=0;tmp_loc=loc;tmp_part = part; \
  event_gothru_type_data(t, (void *) mech, fun); \
  event_gothru_type_data(t2, (void *) mech, fun); \
  return tmp_flag


/* Replace/reload */
int SomeoneRepairing_s(MECH * mech, int loc, int part, int t)
{
    CHECK(t, tech_check_locpart);
}

#define DAT(t) \
if (SomeoneRepairing_s(mech, loc, part, t)) return 1

int SomeoneRepairing(MECH * mech, int loc, int part)
{
    DAT(EVENT_REPAIR_RELO);
    DAT(EVENT_REPAIR_REPL);
    DAT(EVENT_REPAIR_REPLG);
    DAT(EVENT_REPAIR_REPAP);
    DAT(EVENT_REPAIR_REPAG);
    DAT(EVENT_REPAIR_MOB);
    DAT(EVENT_REPAIR_OADD);
    DAT(EVENT_REPAIR_OREM);
    return 0;
}

/* Fixinternal/armor */
int SomeoneFixingA(MECH * mech, int loc)
{
    CHECKL(EVENT_REPAIR_FIX, tech_check_loc);
}

int SomeoneFixingI(MECH * mech, int loc)
{
    CHECKL(EVENT_REPAIR_FIXI, tech_check_loc);
}

int SomeoneFixing(MECH * mech, int loc)
{
    return SomeoneFixingA(mech, loc) || SomeoneFixingI(mech, loc);
}

/* Reattach */
int SomeoneAttaching(MECH * mech, int loc)
{
    CHECKL(EVENT_REPAIR_REAT, tech_check_loc);
}

/* Reseal
 *
 * Added by Kipsta
 * 8/4/99
 */

int SomeoneResealing(MECH * mech, int loc)
{
    CHECKL(EVENT_REPAIR_RESE, tech_check_loc);
}

int SomeoneScrappingLoc(MECH * mech, int loc)
{
    CHECKL(EVENT_REPAIR_SCRL, tech_check_loc);
}

int SomeoneScrappingPart(MECH * mech, int loc, int part)
{
    DAT(EVENT_REPAIR_SCRP);
    DAT(EVENT_REPAIR_SCRG);
    DAT(EVENT_REPAIR_UMOB);
    return 0;
}

#undef CHECK
#undef CHECK2
#undef DAT

int CanScrapLoc(MECH * mech, int loc)
{
    tmp_flag = 0;
    tmp_loc = loc % 8;
    event_gothru_type_data(EVENT_REPAIR_REPL, (void *) mech,
	tech_check_loc);
    event_gothru_type_data(EVENT_REPAIR_RELO, (void *) mech,
	tech_check_loc);
    return !tmp_flag && !SomeoneFixing(mech, loc);
}

int CanScrapPart(MECH * mech, int loc, int part)
{
    return !(SomeoneRepairing(mech, loc, part));
}

#define tech_gun_is_ok(a,b,c) !PartIsNonfunctional(a,b,c)

extern char *silly_get_uptime_to_string(int);

int ValidGunPos(MECH * mech, int loc, int pos)
{
    unsigned char weaparray_f[MAX_WEAPS_SECTION];
    unsigned char weapdata_f[MAX_WEAPS_SECTION];
    int critical_f[MAX_WEAPS_SECTION];
    int i, num_weaps_f;

    if ((num_weaps_f = FindWeapons_Advanced(mech, loc, weaparray_f, weapdata_f, critical_f, 1)) < 0)
	return 0;
    for (i = 0; i < num_weaps_f; i++)
	if (critical_f[i] == pos)
	    return 1;
    return 0;
}

void tech_checkstatus(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int i = figure_latest_tech_event(mech);
    char *ms;

    DOCHECK(!i, "The mech's ready to rock!");
    ms = silly_get_uptime_to_string(game_lag_time(i));
    notify(player, tprintf("The 'mech has approximately %s until done.",
	    ms));
}

TECHCOMMANDH(tech_removegun)
{
    TECHCOMMANDB;
    TECHCOMMANDC;
    my_parsegun(&loc, &part);
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! You can assume the gun's gone too!");
    DOCHECK(!IsWeapon(GetPartType(mech, loc, part)), "That's no gun!");
/*    DOCHECK(PartIsDestroyed(mech, loc, part), "That gun's gone already!"); */
    DOCHECK((WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part), GetWeaponCrits(mech,
        Weapon2I(GetPartType(mech, loc, part)))) != 0), "That weapon appears damaged! Try 'removepart'!");
    DOCHECK(!ValidGunPos(mech, loc, part),
	"You can't remove middle of a gun!");
    DOCHECK(SomeoneScrappingPart(mech, loc, part),
	"Someone's scrapping it already!");
    DOCHECK(!CanScrapPart(mech, loc, part),
	"Someone's tinkering with it already!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no additional removals are possible!");
    CHECKREPAIRTIME(player, MechWeapons[Weapon2I(GetPartType(mech, loc, part))].reptime * ClanMod(GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc, part)))));
    /* Ok.. Everything's valid (we hope). */
    if (tech_weapon_roll(player, mech, REMOVEG_DIFFICULTY) < 0) {
	START("Ack! Your attempt is far from perfect, you try to recover the gun..");
	if (tech_weapon_roll(player, mech, REMOVEG_DIFFICULTY) < 0) {
	    START("No good. Consider the part gone.");
	    FAKEREPAIR(MechWeapons[Weapon2I(GetPartType(mech, loc, part))].reptime * ClanMod(GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc, part)))),
		EVENT_REPAIR_SCRG, mech, PACK_LOCPOS_E(loc, part, mod));
	    mod = 3;
	    return;
	}
    }
    START("You start removing the gun..");
    STARTREPAIR(MechWeapons[Weapon2I(GetPartType(mech, loc,
		part))].reptime * ClanMod(GetWeaponCrits(mech,
                Weapon2I(GetPartType(mech, loc, part)))), mech,
	PACK_LOCPOS_E(loc, part, mod), event_mech_removegun,
	EVENT_REPAIR_SCRG);
}


TECHCOMMANDH(tech_removepart)
{
    TECHCOMMANDB;
    TECHCOMMANDC;
    my_parsepart(&loc, &part);
    DOCHECK(((t =
	    GetPartType(mech, loc, part)) == EMPTY || t == HARDPOINT),
	"That location is empty!");
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! You can assume the part's gone too!");
/*    DOCHECK(IsWeapon(t), "That's a gun - use removegun instead!"); */
    DOCHECK(PartIsDestroyed(mech, loc, part), "That part's gone already!");
    DOCHECK((IsWeapon(t) && (WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part), GetWeaponCrits(mech,
        Weapon2I(GetPartType(mech, loc, part)))) == 2)), "That weapon is fully destroyed already!");
    DOCHECK((IsWeapon(t) && (WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part), GetWeaponCrits(mech,
        Weapon2I(GetPartType(mech, loc, part)))) == 0)), "That weapon is fully functional! Try 'removegun'");
    DOCHECK(IsCrap(GetPartType(mech, loc, part)),
	"That type isn't scrappable!");
    DOCHECK(t == Special(ENDO_STEEL) ||
	t == Special(FERRO_FIBROUS) || t == Special(HARDPOINT),
	"That type of item can't be removed!");
    DOCHECK(SomeoneScrappingPart(mech, loc, part),
	"Someone's scrapping it already!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no additional removals are possible!");
    DOCHECK(!CanScrapPart(mech, loc, part),
	"Someone's tinkering with it already!");
    CHECKREPAIRTIME(player, (IsWeapon(t) ? MechWeapons[Weapon2I(GetPartType(mech, loc,part))].reptime  : REMOVEP_TIME));
    /* Ok.. Everything's valid (we hope). */
    START("You start removing the part..");
    if ((IsWeapon(t) ? tech_weapon_roll(player, mech, REMOVEG_DIFFICULTY) :
	tech_roll(player, mech, REMOVEP_DIFFICULTY)) < 0) {
	START
	    ("Ack! Your attempt is far from perfect, you try to recover the part..");
	if ((IsWeapon(t) ? tech_weapon_roll(player, mech, REMOVEG_DIFFICULTY) :
	    tech_roll(player, mech, REMOVEP_DIFFICULTY)) < 0) {
	    START("No good. Consider the part gone.");
	    mod = 3;
	    FAKEREPAIR((IsWeapon(t) ? MechWeapons[Weapon2I(GetPartType(mech, loc,
                part))].reptime  : REMOVEP_TIME), EVENT_REPAIR_SCRP, mech,
		PACK_LOCPOS_E(loc, part, mod));
	    return;
	}
    }
    STARTREPAIR((IsWeapon(t) ? MechWeapons[Weapon2I(GetPartType(mech, loc,
                part))].reptime : REMOVEP_TIME), mech, PACK_LOCPOS_E(loc, part, mod),
	event_mech_removepart, EVENT_REPAIR_SCRP);
}

#define CHECK_S(nloc) \
if (!SectIsDestroyed(mech,nloc)) return 1; \
if (Invalid_Scrap_Path(mech,nloc)) return 1

#define CHECK(tloc,nloc) \
case tloc: CHECK_S(nloc)

int Invalid_Scrap_Path(MECH * mech, int loc)
{
    if (loc < 0)
	return 0;
    if (MechType(mech) != CLASS_MECH)
	return 0;
    switch (loc) {
	CHECK(CTORSO, HEAD);
	CHECK_S(LTORSO);
	CHECK_S(RTORSO);
	break;
	CHECK(LTORSO, LARM);
	break;
	CHECK(RTORSO, RARM);
	break;
    }
    return 0;
}

#undef CHECK
#undef CHECK_S

TECHCOMMANDH(tech_removesection)
{
    TECHCOMMANDB;
    TECHCOMMANDC;
    my_parsepart(&loc, NULL);
    DOCHECK(SectIsDestroyed(mech, loc), "That section's gone already!");
    DOCHECK(Invalid_Scrap_Path(mech, loc),
	"You need to remove the outer sections first!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping it already!");
    DOCHECK(!CanScrapLoc(mech, loc),
	"Someone's tinkering with it already!");
    /* Ok.. Everything's valid (we hope). */
    CHECKREPAIRTIME(player, REMOVES_TIME(MechTons(mech)));
    if (tech_roll(player, mech, REMOVES_DIFFICULTY) < 0)
	mod = 3;
    START("You start removing the section..");
    STARTREPAIR(REMOVES_TIME(MechTons(mech)), mech, PACK_LOCPOS_E(loc, 0, mod),
	event_mech_removesection, EVENT_REPAIR_SCRL);
}


TECHCOMMANDH(tech_replacegun)
{
    TECHCOMMANDB;
    TECHCOMMANDC;

    my_parsegun(&loc, &part);
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneRepairing(mech, loc, part),
	"Someone's repairing that part already!");
    DOCHECK(!IsWeapon(GetPartType(mech, loc, part)), "That's no gun!");
    DOCHECK(!ValidGunPos(mech, loc, part),
	"You can't the replace middle of a gun!");
/*    DOCHECK(!PartIsNonfunctional(mech, loc, part),
	"That gun isn't hurtin'!"); */
    DOCHECK((WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part), GetWeaponCrits(mech,
	Weapon2I(GetPartType(mech, loc, part)))) == 0), "That gun is working just fine!");
    DOCHECK(!(WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part), GetWeaponCrits(mech,
	Weapon2I(GetPartType(mech, loc, part)))) == 2), "That gun is partially intact!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    CHECKREPAIRTIME(player, ((int) (MechWeapons[Weapon2I(GetPartType(mech, loc, part))].reptime * 1.5)) * ClanMod(GetWeaponCrits(mech,Weapon2I(GetPartType(mech, loc, part)))));
    DOTECH_LOCPOS(REPLACE_DIFFICULTY +
	WEAPTYPE_DIFFICULTY(GetPartType(mech, loc, part)), replaceg_fail,
	replaceg_succ, replace_wholeweap,
	((int) (MechWeapons[Weapon2I(GetPartType(mech, loc,
		part))].reptime * 1.5))
	 	* ClanMod(GetWeaponCrits(mech,
		Weapon2I(GetPartType(mech, loc, part)))), mech,
	PACK_LOCPOS_E(loc, part, 0), event_mech_replacegun,
	EVENT_REPAIR_REPLG, "You start replacing the gun..", 1);
}

TECHCOMMANDH(tech_repairgun)
{
    TECHCOMMANDB;

    TECHCOMMANDC;
    int w;
    /* Find the gun for us */
    my_parsegun(&loc, &part);
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneRepairing(mech, loc, part),
	"Someone's repairing that part already!");
    DOCHECK(!IsWeapon(GetPartType(mech, loc, part)),
	"That's no gun!");
    DOCHECK(!ValidGunPos(mech, loc, part),
	"You can't repair middle of a gun!");
    DOCHECK(SomeoneScrappingPart(mech, loc, part),
	"Someone's scrapping it already!");
    DOCHECK(GetPartDamage(mech, loc, part) == 0 && !PartIsDisabled(mech, loc, part),
	"That gun is not damaged or disabled!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    CHECKREPAIRTIME(player, ((int) (MechWeapons[Weapon2I(GetPartType(mech, loc, part))].reptime / 2)));
    DOTECH_LOCPOS(REPAIR_DIFFICULTY + WEAPTYPE_DIFFICULTY(GetPartType(mech,
		loc, part)), repairg_fail, repairg_succ, repair_econ,
	((int) (MechWeapons[Weapon2I(GetPartType(mech, loc,
	part))].reptime / 2)), mech, PACK_LOCPOS(loc, part),
	event_mech_repairgun,
	EVENT_REPAIR_REPAP, "You start repairing the weapon..", 1);
}

TECHCOMMANDH(tech_oaddgun)
{
    char *args[4];
    int argc;
    int index;
    int weapindex;
    int loop, temp, crit, i, wcrits, wtype, mode, num_weaps = 0, num_weaps_total= 0, num_weaps_type = 0, num_weaptype_max = 0;
    int isrear = 0;
    int istc = 0;
    int isos = 0;
    int critical[MAX_WEAPS_SECTION];
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    TECHCOMMANDB;
    TECHCOMMANDCOMNI;

    if ((db[mech->mynum].flags & QUIET) && !In_Character(mech->mynum))
	notic = 1;
    DOCHECK(!(MechSpecials2(mech) & OMNI_TECH),
	"You cannot change out omnipods on a normal mech. Get Real.");
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc < 3, "Invalid number of arguments!")
        index =
        ArmorSectionFromString(MechType(mech), MechMove(mech), args[1]);
    if (index == -1) {
        notify(player, "Not a legal area. Must be HEAD, CTORSO");
        notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
        notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
        return;
    }
    weapindex = WeaponIndexFromString(args[0]);
    if (weapindex == -1) {
        notify(player, "That is not a valid weapon!");
        DumpWeapons(player);
        return;
    }
    if ((MechSpecials(mech) & CLAN_TECH) && strstr(MechWeapons[weapindex].name, "IS.")) {
	notify(player, "This units technology is too advanced for that weapon.");
	return;
    }
    if (!(MechSpecials(mech) & CLAN_TECH) && strstr(MechWeapons[weapindex] .name, "CL.")) {
        notify(player, "This units technology is not advanced enough for that weapon.") ;
        return;
    }
    if (strstr(MechWeapons[weapindex].name, "PC.")) {
        notify(player, "The vision of this unit hacking other metal apart with such a weapon is amusing, but not going to happen.");
        return;
    }
    if (args[3]) {
/*        if (args[3][0] == 'T' || args[3][0] == 't' ||
            args[3][1] == 'T' || args[3][1] == 't')
            istc = 1; */
/* Prolly need to make this a FORCED attrib if it has targeting. Do later. */

        if (toupper(args[3][0]) == 'R' || toupper(args[3][1]) == 'R')
            isrear = 1;

        if ((toupper(args[3][0]) == 'O' || toupper(args[3][1]) == 'O') &&
	    (MechWeapons[weapindex].type == TMISSILE))
            isos = 1;
    }
	if (MechWeapons[weapindex].special & OS_WEAP)
	    isos = 1;

    crit = (atoi(args[2]) - 1);
    DOCHECK(crit < 0 || crit >= CritsInLoc(mech, index),
	"Invalid critical.");
/*    crit = (WeaponFirstCrit(mech, index, crit)); */
    DOCHECK(SomeoneRepairing(mech, index, crit),
        "Someone's tinkering with that part already!");
    wcrits = (GetWeaponCrits(mech, weapindex));
    wtype = MechWeapons[weapindex].type;
    for (i = crit; i < (crit + wcrits); i ++) {
	DOCHECK(i < 0 || i >= CritsInLoc(mech, index),
	    "That weapon will not fit there!");
	DOCHECK(!(GetPartType(mech, index, i) == I2Special(HARDPOINT) &&
	   ((wtype == TBEAM) ?
	    (GetPartMode(mech, index, i) & ENERGY_HPOINT) :
            (wtype == TMISSILE) ?
	    (GetPartMode(mech, index, i) & MISSILE_HPOINT) :
            (MechWeapons[weapindex].special & GAUSS) ?
	    (GetPartMode(mech, index, i) & GAUSS_HPOINT) :
	    (GetPartMode(mech, index, i) & BALL_HPOINT))),
	     tprintf("Invalid hardpoint at crit #%d!", i + 1));
	DOCHECK(PartIsNonfunctional(mech, index, i),
	     tprintf("Nonfunctional item at crit #%d!", i + 1));
	}
    DOCHECK(((PodSpace(mech) - (MechWeapons[weapindex].weight * 10)) < 0),
	tprintf("Not enough pod space.  You have %3.2f tons of podspace and that weapon weighs %3.2f tons.",
	(float) (PodSpace(mech) / 100), (float) (MechWeapons[weapindex].weight / 10)));
/* Design demunch rules */
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
        num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
        num_weaps_total = (num_weaps_total + num_weaps);
        for (i = 0; i < num_weaps; i++) {
	   if (MechWeapons[weapindex].class ==
	       MechWeapons[weaparray[i]].class)
	     if (GetPartMode(mech, loop, critical[i]) & HPOINTS)
	   	num_weaps_type++;
	}
    }
    for (loop = 0; WeaponClassTable[loop].wclass != -1; loop++) {
	if (WeaponClassTable[loop].wclass == MechWeapons[weapindex].class) {
	    num_weaptype_max = WeaponClassTable[loop].max_weaps;
	    break;
	    }
	}
    CHECKREPAIRTIME(player, GetWeaponCrits(mech, weapindex) * 5);
    DOCHECK(num_weaps_type >= num_weaptype_max,
	tprintf("Your omnimech specific targeting system can only handle %d weapons for this weapon class.", num_weaptype_max));
    if (!notic) {
        PARTCHECK(mech, index, I2Weapon(weapindex), wcrits);
	}
    for (loop = 0; loop < wcrits; loop++) {
        temp = crit + loop;
        mode = (GetPartMode(mech, index, temp) & HPOINTS);
        DOCHECK(temp < 0 || temp >= CritsInLoc(mech, index),
    	    "Bad critical location! Contact a Wizard!");
        MechSections(mech)[index].criticals[temp].type =
            (I2Weapon(weapindex));
        MechSections(mech)[index].criticals[temp].mode = mode;
        if (isos)
            MechSections(mech)[index].criticals[temp].mode |=
                OS_MODE;
        if (isrear && (index == RTORSO || index == CTORSO || index == LTORSO))
            MechSections(mech)[index].criticals[temp].mode |=
                REAR_MOUNT;
        if (istc)
            MechSections(mech)[index].criticals[temp].mode |=
		ON_TC;
    }
    if (IsAMS(weapindex)) {
        if (MechWeapons[weapindex].special & CLAT)
            MechSpecials(mech) |= CL_ANTI_MISSILE_TECH;
        else
            MechSpecials(mech) |= IS_ANTI_MISSILE_TECH;
    }
    PodSpace(mech) -= (MechWeapons[weapindex].weight * 10);
    if (!notic) {
    FAKEREPAIR(GetWeaponCrits(mech, weapindex) * 5,
		EVENT_REPAIR_OADD, mech, PACK_LOCPOS_E(index, crit, mod));
    }
    do_magic(mech);
    notify(player, "Weapon added.");
}

TECHCOMMANDH(tech_oremgun)
{
    char *args[2];
    int argc;
    int index;
    int weapindex;
    int loop, temp, crit, i, wcrits, wtype, mode, origtype;
    int isrear = 0;
    int istc = 0;
    int isos = 0;
    TECHCOMMANDB;
    TECHCOMMANDCOMNI;


    if ((db[mech->mynum].flags & QUIET) && !In_Character(mech->mynum))
	notic = 1;
    DOCHECK(!(MechSpecials2(mech) & OMNI_TECH),
        "You cannot change out omnipods on a normal mech. Get Real.");
    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc != 2, "Invalid number of arguments!")
        index =
        ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    if (index == -1) {
        notify(player, "Not a legal area. Must be HEAD, CTORSO");
        notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
        notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
        return;
    }
    crit = (atoi(args[1]) - 1);
    DOCHECK(crit < 0 || crit >= CritsInLoc(mech, index),
        "Invalid critical.");
    crit = (WeaponFirstCrit(mech, index, crit));
    weapindex = Weapon2I(GetPartType(mech, index, crit));
    DOCHECK(SomeoneRepairing(mech, index, crit),
        "Someone's tinkering with that part already!");
    CHECKREPAIRTIME(player, GetWeaponCrits(mech, weapindex) * 5);
    if (!IsWeapon(weapindex)) {
        notify(player, "That is not a weapon!");
        return;
    }
    wcrits = (GetWeaponCrits(mech, weapindex));
    wtype = MechWeapons[weapindex].type;
    for (i = crit; i < (crit + wcrits); i ++) {
        DOCHECK(i < 0 || i >= CritsInLoc(mech, index),
	    "Error.  That weapon exceeds section criticals. Contact a Wizard.");
        DOCHECK(!(GetPartType(mech, index, i) == I2Weapon(weapindex) &&
            (wtype == TBEAM ? (GetPartMode(mech, index, i) & ENERGY_HPOINT) :
             wtype == TMISSILE ?  (GetPartMode(mech, index, i) & MISSILE_HPOINT) :
	    (MechWeapons[weapindex].special & GAUSS) ? (GetPartMode(mech, index, i) & GAUSS_HPOINT) :
             (GetPartMode(mech, index, i) & BALL_HPOINT))),
             tprintf("Invalid hardpoint at crit #%d!", i + 1));
        DOCHECK(PartIsNonfunctional(mech, index, i),
             tprintf("Nonfunctional item at crit #%d!", i + 1));
        }

    origtype = GetPartType(mech, index, crit);
    for (loop = 0; loop < wcrits; loop++) {
        temp = crit + loop;
        mode = (GetPartMode(mech, index, temp) & HPOINTS);
        DOCHECK(temp < 0 || temp >= CritsInLoc(mech, index),
	    "Bad critical location! Contact a Wizard!");
        MechSections(mech)[index].criticals[temp].type =
	    I2Special(HARDPOINT);
        MechSections(mech)[index].criticals[temp].mode = mode;
    }
    PodSpace(mech) += (MechWeapons[weapindex].weight) * 10;
    if (!notic) {
        AddPartsM(mech, index, origtype, wcrits);
        FAKEREPAIR(GetWeaponCrits(mech, weapindex) * 5,
		EVENT_REPAIR_OREM, mech, PACK_LOCPOS_E(index, crit, mod));
	}
    do_magic(mech);
    notify(player, "Weapon removed.");
}

TECHCOMMANDH(tech_omodammo)
{
    char *args[4];
    int argc;
    int index, crit, isht = 0, isempty = 0, wasempty = 0, mode, weapindex = 0;
    TECHCOMMANDB;
    TECHCOMMANDCOMNI;


    if ((db[mech->mynum].flags & QUIET) && !In_Character(mech->mynum))
	notic = 1;
    DOCHECK(!(MechSpecials2(mech) & OMNI_TECH),
        "You cannot change out omnipods on a normal mech. Get Real.");
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc < 3, "Invalid number of arguments!")
        index =
        ArmorSectionFromString(MechType(mech), MechMove(mech), args[1]);
    if (index == -1) {
        notify(player, "Not a legal area. Must be HEAD, CTORSO");
        notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
        notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
        return;
    }
    if (toupper(args[0][0]) == 'E') {
	isempty = 1;
    } else {
    weapindex = WeaponIndexFromString(args[0]);
    }
    if (weapindex == -1 && !isempty) {
        notify(player, "That is not a valid weapon!");
        DumpWeapons(player);
        return;
    }
    if (weapindex) {
        if ((MechSpecials(mech) & CLAN_TECH) && strstr(MechWeapons[weapindex].name, "IS.")) {
            notify(player, "This units technology is too advanced for that weapon.");
	    return;
        }
        if (!(MechSpecials(mech) & CLAN_TECH) && strstr(MechWeapons[weapindex].name, "CL.")) {
        notify(player, "This units technology is not advanced enough for that weapon.");
        return;
        }
        if (strstr(MechWeapons[weapindex].name, "PC.")) {
            notify(player, "The vision of this unit hacking other metal apart with such a weapon is amusing, but not going to happen.");
            return;
	}
	if (MechWeapons[weapindex].special & OS_WEAP) {
	    notify(player, "OneShot weapontype's cannot have secondary ammo loaded.");
	    return;
	    }
    }
    if ((args[3]) && weapindex)
        if ((toupper(args[3][0]) == 'H') && (strstr(MechWeapons[weapindex].name,
	"Machine") || strstr(MechWeapons[weapindex].name, "Mortar")))
            isht = 1;
    DOCHECK(((PodSpace(mech) < (isht ? 50 : 100)) && !isempty),
	"You do not have enough PodSpace for that!");
    crit = (atoi(args[2]) - 1);
    DOCHECK(crit < 0 || crit >= CritsInLoc(mech, index),
        "Invalid critical.");
    DOCHECK((!(GetPartType(mech, index, crit) == I2Special(HARDPOINT)) && !isempty),
         tprintf("Invalid hardpoint at crit #%d!", crit + 1));
    DOCHECK(!(GetPartMode(mech, index, crit) & HPOINTS),
         tprintf("Invalid hardpoint type at crit #%d!", crit + 1));
    DOCHECK(PartIsNonfunctional(mech, index, crit),
         tprintf("Nonfunctional bin at crit #%d!", crit + 1));
    DOCHECK((isempty && !IsAmmo(GetPartType(mech, index, crit))),
	"You can only changeover ammo crits.");
    DOCHECK((isempty && GetPartData(mech, index, crit) && !notic),
	"Unload all the ammo first.");
    DOCHECK(SomeoneRepairing(mech, index, crit),
        "Someone's tinkering with that part already!");
    CHECKREPAIRTIME(player, 5);
    mode = (GetPartMode(mech, index, crit) & HPOINTS);
    if (isht && !isempty)
	mode |= HALFTON_MODE;
    else if ((GetPartMode(mech, index, crit) & HALFTON_MODE) && isempty)
	isht = 1;
    if (isempty) {
        MechSections(mech)[index].criticals[crit].type = I2Special(HARDPOINT);
	GetPartData(mech, index, crit) = 0;
    } else {
	MechSections(mech)[index].criticals[crit].type = I2Ammo(weapindex);
	GetPartData(mech, index, crit) = 0;
	}
    MechSections(mech)[index].criticals[crit].mode = mode;
    if (!isempty)
        PodSpace(mech) -= (isht ? 50 : 100);
    else
    	PodSpace(mech) += (isht ? 50 : 100);
    if (!notic) {
        FAKEREPAIR(5, (isempty ? EVENT_REPAIR_OREM : EVENT_REPAIR_OADD),
	 mech, PACK_LOCPOS_E(index, crit, mod));
	}
    if (notic && !isempty && IsAmmo(GetPartType(mech, index, crit)))
	SetPartData(mech, index, crit, FullAmmo(mech, index, crit));
    do_magic(mech);
    notify(player, tprintf("Ammo %s.", isempty ? "emptied" : "modified"));
}

TECHCOMMANDH(tech_mountbomb)
{
char *args[4];
int argc;
int mounttype = 1, parttype, bombs, maxbombs, pdata;
float maxload;
TECHCOMMANDB;
TECHCOMMANDCOMNI;

    DOCHECK(MechType(mech) != CLASS_AERO, "This is not an aerofighter....");
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc < 3 || argc > 4, "Insufficient arguements. Accept 'mountbomb <loc> <Crit#> <Type> [#]'.");

    loc = ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    if (loc == -1) {
        notify(player, "Not a legal area. Must be HEAD, CTORSO");
        notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
        notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
        return;
    }
    part = BOUNDED(1,atoi(args[1]),12);
    part--;
    if (strstr(args[2], "Neutron") && !Wiz(player)) {
	notify(player, "Neutron Bombs not allowed to be mounted by such petty mortals as you.");
	return;
	}
    parttype = FindBombItemCodeFromString(args[2]);
    if (argc == 4)
	bombs = MAX(1,atoi(args[3]));
    pdata = GetPartData(mech, loc, part); 
    maxload = MaxBombPoints(mech);

    if (parttype == -1)
	mounttype = 0;
    parttype = I2Bomb(parttype);
    if (mounttype) {
	DOCHECK(GetPartType(mech, loc, part) != EMPTY, "Can only mount bombs on empty crits. For now.");
	DOCHECK(!IsBomb(parttype), "You must type in the bomb name as seen in cargo listings.");
	maxbombs = BombCap(Bomb2I(parttype));
	if (argc != 4)
	    bombs = maxbombs;
 	else
	    bombs = BOUNDED(1, bombs, maxbombs);

	DOCHECK(bombs + pdata > maxbombs, tprintf("That load type caps at %d and you will go over that load per crit.", maxbombs));
	DOCHECK(BombPoints(mech) + (BombStrength(Bomb2I(parttype)) * bombs) > maxload, tprintf("This craft can only handle %.1f, total bomb strength and currently has %.1f.", maxload, BombPoints(mech)));
	if (In_Character(mech->mynum)) {
	    CHECKREPAIRTIME(player, 1);
            PARTCHECK(mech, loc, parttype, bombs);
	    }
	SetPartType(mech, loc, part, parttype);
	SetPartMode(mech, loc, part, 0);
	SetPartData(mech, loc, part, MAX(GetPartData(mech, loc, part), 0) + bombs);
	notify(player, tprintf("Bomb added to section. %.1f/%.1f points used.", BombPoints(mech), maxload));
    } else {
	DOCHECK(!IsBomb(GetPartType(mech, loc, part)), "Can only unmount bomb crits.....");
	if (In_Character(mech->mynum))
            AddPartsM(mech, loc, GetPartType(mech, loc, part), pdata);
	SetPartType(mech, loc, part, 0);
	SetPartMode(mech, loc, part, 0);
	SetPartData(mech, loc, part, 0);
	notify(player, "Bomb removed from section.");
    }
    SetCargoWeight(mech);
    if (In_Character(mech->mynum))
	FAKEREPAIR(1, EVENT_REPAIR_OADD, mech, PACK_LOCPOS_E(loc, part, mod));
}

TECHCOMMANDH(tech_oaddsp)
{
    char *args[4];
    int argc;
    int index;
    int parttype, temptype;
    int partcrit;
    int partdata = -1;
    int sectcrit, i, pcrits = -1, needdata = 0, weight, temp, mode, empty = 0, isphys = 0;
    int isdhs = 0, hseff = 1, ischs = 0;

    TECHCOMMANDB;
    TECHCOMMANDCOMNI;

    if ((db[mech->mynum].flags & QUIET) && !In_Character(mech->mynum))
	notic = 1;
    DOCHECK(!(MechSpecials2(mech) & OMNI_TECH),
	"You cannot change out omnipods on a normal mech. Get Real.");
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc < 3, "Invalid number of arguments!");
    index = ArmorSectionFromString(MechType(mech), MechMove(mech), args[1]);
    if (index == -1) {
        notify(player, "Not a legal area. Must be HEAD, CTORSO");
        notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
        notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
        return;
    }
    sectcrit = CritsInLoc(mech, index);
    parttype = FindSpecialItemCodeFromString(args[0]);
    if (parttype == -1) {
        if (strcasecmp(args[0], "empty")) {
            notify(player, "That is not a valid special object!");
/*            DumpMechSpecialObjects(player);*/
	    notify(player, "Valid Objects for OADDSP are : Case, Axe, Sword, Mace, BeagleProbe, ArtemisIV, ECM, TAG,");
	    notify(player, "AngelECM, Capacitor, BloodhoundProbe, and HeatSink.");
            return;
        } else {
	empty = 1;
	parttype = HARDPOINT;
	}
    }
    partcrit = BOUNDED(1,atoi(args[2]),12);
    partcrit--;
    if (empty) {
	temptype = (GetPartType(mech, index, partcrit));
	if (IsWeapon(temptype))
	    temptype = Weapon2I(temptype);
	else if (IsAmmo(temptype))
	    temptype = Ammo2I(temptype);
	else if (IsSpecial(temptype))
	    temptype = Special2I(temptype);
	else if (IsBomb(temptype))
	    temptype = Bomb2I(temptype);
	else if (IsCargo(temptype))
	    temptype = Cargo2I(temptype);
        DOCHECK(empty && temptype == HARDPOINT,
	    "Empty an empty slot?");
    } else
	temptype = parttype;
    switch (temptype) {
        case CASE:
	    pcrits = 1;
	    break;
        case HEAT_SINK:
	   isdhs = (MechSpecials(mech) & DOUBLE_HEAT_TECH);
	   ischs = (MechSpecials2(mech) & COMPACT_HEAT_TECH);
	   if (isdhs) {
	     pcrits = ((MechSpecials(mech) & CLAN_TECH) ? 2 : 3);
	     hseff = 2;
	   } else if (ischs){
	     pcrits = 1;
	     hseff = 2;
	   } else {
	     pcrits = 1;
	     hseff = 1;
	   }
        break;
        case AXE:
	case MACE:
	case SWORD:
	    isphys = 1;
	    pcrits = (int) (MechTons(mech) / 15);
	    if (MechTons(mech) % 15)
		pcrits++;
	    break;
        case BEAGLE_PROBE:
	    pcrits = 2;
	    break;
        case ARTEMIS_IV:
	    pcrits = 1;
	    needdata = 1;
	    break;
        case ECM:
	    pcrits = 2;
	    break;
        case TAG:
	    pcrits = 1;
	    break;
        case CAPACITOR:
	    pcrits = 1;
	    needdata = 1;
	    break;
        case ANGEL_ECM:
	    pcrits = 2;
	    break;
        case BLOODHOUND_PROBE:
	    pcrits = 3;
	    break;
	}

    if (empty)
	needdata = 0;

    DOCHECK((pcrits < 1) || (!IsSpecial(I2Special(parttype)) && !empty), "Invalid part name for omni'ing.");

    weight = (int) ((((float) ((temp = crit_weight(mech, I2Special(temptype))) * (float) pcrits) / (float) 1024) * (float) 100));
/*    SendDebug(tprintf("Pcrits : %d Weight : %d Needdata : %d Parttype : %d CritWeight : %d",
	pcrits, weight, needdata, parttype, temp)); */
    DOCHECK(!weight, "Invalid weight for part. Contact a wiz.");

    DOCHECK(needdata && argc != 4,
	"This part needs specific data to point at a crit!");

    if (needdata && args[3]) {
	partdata = (atoi(args[3]) - 1);
	DOCHECK(partdata < 0 || partdata > sectcrit,
		"Invalid crit for data.");
        }

    DOCHECK(isphys && index != RARM && index != LARM,
	"Physical weapons may only be mounted on arms.");

    DOCHECK(isphys && !OkayCritSectS(index, 0, SHOULDER_OR_HIP),
	"There is no shoulder actuator in that arm. Sorry.");
    DOCHECK(isphys && !OkayCritSectS(index, 1, UPPER_ACTUATOR),
	"There is no upper actuator in that arm. Sorry.");
    DOCHECK(isphys && !OkayCritSectS(index, 2, LOWER_ACTUATOR),
	"There is no lower actuator in that arm. Sorry.");
    DOCHECK(isphys && !OkayCritSectS(index, 3, HAND_OR_FOOT_ACTUATOR),
	"There is no hand actuator in that arm. Sorry.");

    if (needdata) {
	if (parttype == ARTEMIS_IV) {
	    DOCHECK(((!IsMissile(Weapon2I(GetPartType(mech, index, partdata)))) ||
		(!(strstr(MechWeapons[Weapon2I(GetPartType(mech, index, partdata))].name, ".LRM-") ||
		   strstr(MechWeapons[Weapon2I(GetPartType(mech, index, partdata))].name, ".SRM-")))),
		"Artemis can only assist normal missile weapons.");
	    partdata = WeaponFirstCrit(mech, index, partdata);
	} else if (parttype == CAPACITOR) {
	    DOCHECK(!IsWeapon(Weapon2I(GetPartType(mech, index, partdata))) ||
		!(strstr(MechWeapons[Weapon2I(GetPartType(mech, index, partdata))].name, "PPC")),
		"Capacitors can only assist PPC's.");
	    partdata = WeaponFirstCrit(mech, index, partdata);
	    }
	}

    DOCHECK((partcrit < 0) || (partcrit >= sectcrit),
	"Invalid critical.");
    DOCHECK(SomeoneRepairing(mech, index, partcrit),
        "Someone's tinkering with that part already!");
    for (i = partcrit; i < (partcrit + pcrits); i++) {
	DOCHECK(i < 0 || i >= sectcrit,
	    "That item will not fit there!");
	if (empty) {
	    DOCHECK(!isphys && !(IsSpecial(GetPartType(mech, index, i)) &&
	    (GetPartMode(mech, index, i) & SPECIAL_HPOINT)),
	    tprintf("Invalid hardpoint at crit #%d!", i + 1));
	} else {
  	    DOCHECK(!(GetPartType(mech, index, i) == I2Special(HARDPOINT) &&
	    (isphys || (GetPartMode(mech, index, i) & SPECIAL_HPOINT))),
	     tprintf("Invalid hardpoint at crit #%d!", i + 1));
	}
	DOCHECK(PartIsNonfunctional(mech, index, i),
	     tprintf("Nonfunctional item at crit #%d!", i + 1));
	}

    DOCHECK(((PodSpace(mech) - weight) < 0) && !empty,
	tprintf("Not enough pod space.  You have %3.2f tons of podspace and that item weighs %3.2f tons.",
	(float) (PodSpace(mech) / 100), (float) (weight / 10)));
    CHECKREPAIRTIME(player, pcrits * 5);
    if (!notic && !empty) {
        PARTCHECK(mech, index, I2Special(parttype), pcrits);
	}
    for (i = 0; i < pcrits; i++) {
        temp = partcrit + i;
        mode = (GetPartMode(mech, index, temp) & HPOINTS);
        DOCHECK(temp < 0 || temp >= sectcrit,
    	    "Bad critical location! Contact a Wizard!");
        MechSections(mech)[index].criticals[temp].type =
            (I2Special(parttype));
        MechSections(mech)[index].criticals[temp].mode = mode;
        if (partdata == -1)
	    partdata = 0;
	MechSections(mech)[index].criticals[temp].data = partdata;
    }

    if (empty)
	PodSpace(mech) += (weight);
    else
        PodSpace(mech) -= (weight);
    if (!notic) {
        if (empty)
            AddPartsM(mech, index, I2Special(temptype), pcrits);
        FAKEREPAIR(pcrits * 5,
		EVENT_REPAIR_OADD, mech, PACK_LOCPOS_E(index, partcrit, mod));
    }
    switch (parttype) {
	case CASE:
            MechSections(mech)[(MechType(mech) ==
                CLASS_VEH_GROUND) ? BSIDE : index].config |= CASE_TECH;
            notify(player, "CASE Technology added to section.");
            break;
        case HEAT_SINK:
	    notify(player, tprintf("%sHeatsink added to 'Mech.", (isdhs ? "Double " : ischs ?
								  "Compact " : "")));
	    MechRealNumsinks(mech) += hseff;
	    break;
        case ARTEMIS_IV:
            MechSpecials(mech) |= ARTEMIS_IV_TECH;
            notify(player, "Artemis IV Fire-Control System added to 'Mech.");
            notify(player,
                tprintf
                ("System will control the weapon which starts at slot %d.",
             partdata + 1));
            break;
        case CAPACITOR:
            notify(player, "PPC Capacitor added to 'Mech.");
            notify(player,
                tprintf
                ("Capacitor will control the weapon which starts at slot %d.",
             partdata + 1));
            break;
        case ECM:
            MechSpecials(mech) |= ECM_TECH;
            notify(player, "Guardian ECM Suite added to 'Mech.");
            break;
        case ANGEL_ECM:
            MechSpecials2(mech) |= ANGEL_ECM_TECH;
            notify(player, "Angel ECM Suite added to 'Mech.");
            break;
        case BEAGLE_PROBE:
            MechSpecials(mech) |= BEAGLE_PROBE_TECH;
            notify(player, "Beagle Active Probe added to 'Mech.");
            break;
        case BLOODHOUND_PROBE:
            MechSpecials2(mech) |= BLOODHOUND_PROBE_TECH;
            notify(player, "Bloodhound Active Probe added to 'Mech.");
            break;
        case EMPTY:
	if (temptype == HEAT_SINK) {
	    MechRealNumsinks(mech) -= hseff;
	}
        break;
    }
    do_magic(mech);
    notify(player, tprintf("Special %s.", (empty ? "removed" : "added")));
}

TECHCOMMANDH(tech_replacepart)
{
    TECHCOMMANDB;

    TECHCOMMANDC;
    my_parsepart(&loc, &part);
    DOCHECK((t =
	    GetPartType(mech, loc, part)) == EMPTY,
	"That location is empty!");
    DOCHECK(!PartIsDestroyed(mech, loc, part) && !PartIsDisabled(mech, loc, part),
	"That part looks ok to me..");
    DOCHECK(IsCrap(GetPartType(mech, loc, part)),
	"That part isn't hurtin'!");
    DOCHECK((IsWeapon(t) && WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, part),
	GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loc, part)))) == 2),
	"That's a fully destroyed weapon! Use replacegun instead.");
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneRepairing(mech, loc, part),
	"Someone's repairing that part already!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    CHECKREPAIRTIME(player, (IsWeapon(t) ? ((int) (MechWeapons[Weapon2I(GetPartType(mech, loc, part))].reptime)) : REPLACEPART_TIME));
    DOTECH_LOCPOS(REPLACE_DIFFICULTY +
	PARTTYPE_DIFFICULTY(GetPartType(mech, loc, part)), replacep_fail,
	replacep_succ, replace_econ,
	(IsWeapon(t) ? ((int) (MechWeapons[Weapon2I(GetPartType(mech, loc,
        part))].reptime)) : REPLACEPART_TIME), mech,
	PACK_LOCPOS(loc, part), event_mech_repairpart, EVENT_REPAIR_REPL,
	"You start replacing the part..", (IsWeapon(t) ? 1 : 0));
}

TECHCOMMANDH(tech_repairpart)
{
    TECHCOMMANDB;

    TECHCOMMANDC;
    my_parsepart(&loc, &part);
    DOCHECK(((t =
	    GetPartType(mech, loc, part)) == EMPTY || t == HARDPOINT),
	"That location is empty!");
    DOCHECK(PartIsDestroyed(mech, loc, part),
	"That part is gone for good!");
    DOCHECK(PartIsDisabled(mech, loc, part),
	"That part can't be repaired yet!");
#if 0
    DOCHECK(!PartTempNuke(mech, loc, part), "That part isn't hurtin'!");
#endif
    DOCHECK(IsCrap(GetPartType(mech, loc, part)),
	"That part isn't hurtin'!");
    DOCHECK(IsWeapon(t), "That's a weapon! Use repairgun instead.");
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneRepairing(mech, loc, part),
	"Someone's repairing that part already!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    CHECKREPAIRTIME(player, REPAIRPART_TIME);
    DOTECH_LOCPOS(REPAIR_DIFFICULTY + PARTTYPE_DIFFICULTY(GetPartType(mech,
		loc, part)), repairp_fail, repairp_succ, repair_econ,
	REPAIRPART_TIME, mech, PACK_LOCPOS(loc, part),
	event_mech_repairpart, EVENT_REPAIR_REPAP,
	"You start repairing the part..", 0);
}

TECHCOMMANDH(tech_reload)
{
    int atype;
    int partmode;

    TECHCOMMANDB;
    TECHCOMMANDD;
    my_parsepart2(&loc, &part, &atype);
    partmode = GetPartMode(mech, loc, part);
/*    DOCHECK(MechType(mech) == CLASS_BSUIT,
	"You can't reload a Battlesuit!"); */
    DOCHECK(!IsAmmo((t = GetPartType(mech, loc, part))) && !(partmode & OS_MODE), "That's no ammo!");
    DOCHECK(PartIsDestroyed(mech, loc, part), "The ammo compartment is destroyed ; repair/replacepart it first.");
    DOCHECK(PartIsDisabled(mech, loc, part), "The ammo compartment is disabled ; repair/replacepart it first.");
    if (partmode & OS_MODE) {
      DOCHECK((!(partmode & OS_USED)), "That particular ammo compartment doesn't need reloading.");
    } else {
      DOCHECK((now = GetPartData(mech, loc, part)) == (full = FullAmmo(mech, loc, part)), "That particular ammo compartment doesn't need reloading.");
    }
    DOCHECK(SomeoneRepairing(mech, loc, part), "Someone's playing with that part already!");
    DOCHECK(SectIsDestroyed(mech, loc), "That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc), "That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneScrappingLoc(mech, loc), "Someone's scrapping that section - no repairs are possible!");
    if (atype) {
	DOCHECK((t = (valid_ammo_mode(mech, loc, part, atype))) < 0, "That is invalid ammo type for this weapon!");
        if (strstr(MechWeapons[Ammo2WeaponI(GetPartType(mech, loc, part))].name, "Streak")) {
            if (t & INFERNO_MODE)
	        t &= ~INFERNO_MODE;
	}
        if (partmode & OS_MODE) {
	    GetPartMode(mech, loc, part) |= OS_USED;
	    partmode |= OS_USED;
	} else {
	    SetPartData(mech, loc, part, 0);
	}
	GetPartMode(mech, loc, part) &= ~(AMMO_MODES);
	GetPartMode(mech, loc, part) |= t;
        if ((GetPartMode(mech, loc, part) & INFERNO_MODE) && !(MechCritStatus(mech) & INFERNO_AMMO))
	    MechCritStatus(mech) |= INFERNO_AMMO;
	if (partmode & HALFTON_MODE)
		GetPartMode(mech, loc, part) |= HALFTON_MODE;
	}
    change = 0;
    CHECKREPAIRTIME(player, (MechType(mech) == CLASS_BSUIT ? BSUIT_RELOAD_TIME : ReloadTime(mech, loc, part, 1)));
    DOTECH_LOCPOS_VAL(RELOAD_DIFFICULTY, reload_fail, reload_succ,
	reload_econ, &change, (MechType(mech) == CLASS_BSUIT ? BSUIT_RELOAD_TIME : ReloadTime(mech, loc, part, 1)),
	 mech, PACK_LOCPOS_E(loc, part, change), event_mech_reload, EVENT_REPAIR_RELO,
	"You start reloading the ammo compartment..");
}

TECHCOMMANDH(tech_unload)
{
    TECHCOMMANDB;

    TECHCOMMANDD;
    my_parsepart(&loc, &part);
    DOCHECK(!IsAmmo((t =
		GetPartType(mech, loc, part))), "That's no ammo!");
    DOCHECK(PartIsDestroyed(mech, loc, part),
	"The ammo compartment is destroyed ; repair/replacepart it first.");
    DOCHECK(PartIsDisabled(mech, loc, part),
	"The ammo compartment is disabled ; repair/replacepart it first.");
    DOCHECK(!(now =
	    GetPartData(mech, loc, part)),
	"That particular ammo compartment is empty already.");
    DOCHECK(SomeoneRepairing(mech, loc, part),
	"Someone's playing with that part already!");
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    if ((full = FullAmmo(mech, loc, part)) == now)
	change = 2;
    else
	change = 1;
    CHECKREPAIRTIME(player, (MechType(mech) == CLASS_BSUIT ? BSUIT_RELOAD_TIME : ReloadTime(mech, loc, part, 0)));
    if (tech_roll(player, mech, REMOVES_DIFFICULTY) < 0)
	mod = 3;
    START("You start unloading the ammo compartment..");
    STARTREPAIR((MechType(mech) == CLASS_BSUIT ? BSUIT_RELOAD_TIME : ReloadTime(mech, loc, part, 0)),
	mech, PACK_LOCPOS_E(loc, part, change), event_mech_reload, EVENT_REPAIR_RELO);
}

TECHCOMMANDH(tech_fixarmor)
{
    int ochange;

    TECHCOMMANDB;

    TECHCOMMANDD;
    DOCHECK(tech_parsepart_advanced(mech, buffer, &loc, NULL, NULL, 1) < 0,
	"Invalid section!");
    if (loc >= 8) {
	from = GetSectRArmor(mech, loc % 8);
	to = GetSectORArmor(mech, loc % 8);
    } else {
	from = GetSectArmor(mech, loc);
	to = GetSectOArmor(mech, loc);
    }
    DOCHECK(SectIsDestroyed(mech, loc % 8),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc % 8),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneFixingA(mech, loc) ||
	SomeoneFixingI(mech, loc % 8),
	"Someone's repairing that section already!");
    DOCHECK(GetSectInt(mech, loc % 8) != GetSectOInt(mech, loc % 8),
	"The internals need to be fixed first!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    from = MIN(to, from);
    DOCHECK(from == to, "The location doesn't need armor repair!");
    change = to - from;
    ochange = change;
    CHECKREPAIRTIME(player, (FIXARMOR_TIME * ((change > 0) ? (change) : (1))));
    DOTECH_LOC_VAL_S(FIXARMOR_DIFFICULTY, fixarmor_fail, fixarmor_succ,
	fixarmor_econ, &change, FIXARMOR_TIME * ochange, loc,
	EVENT_REPAIR_FIX, mech, "You start fixing the armor..");
    STARTIREPAIR((FIXARMOR_TIME * ((change > 0) ? (change) : (1))), mech, (change * 16 + loc),
	event_mech_repairarmor, EVENT_REPAIR_FIX,
	((change > 0) ? (change) : (1)));
}

TECHCOMMANDH(tech_fixinternal)
{
    TECHCOMMANDB int ochange;

    TECHCOMMANDC;
    my_parsepart(&loc, NULL);
    from = GetSectInt(mech, loc);
    to = GetSectOInt(mech, loc);
    DOCHECK(from == to, "The location doesn't need internals' repair!");
    change = to - from;
    DOCHECK(is_aero(mech) && loc != 0,
	"You must target the Nose to fix Strucutral Integrity/Internals on an Aerospace unit.");
    DOCHECK(SectIsDestroyed(mech, loc),
	"That part's blown off! Use reattach first!");
    DOCHECK(SectIsFlooded(mech, loc),
	"That location has been flooded! Use reseal first!");
    DOCHECK(SomeoneFixing(mech, loc),
	"Someone's repairing that section already!");
    DOCHECK(SomeoneScrappingLoc(mech, loc),
	"Someone's scrapping that section - no repairs are possible!");
    ochange = change;
    CHECKREPAIRTIME(player, (FIXINTERNAL_TIME * ((change > 0) ? (change) : (1))));
    DOTECH_LOC_VAL_S(FIXINTERNAL_DIFFICULTY, fixinternal_fail,
	fixinternal_succ, fixinternal_econ, &change,
	FIXINTERNAL_TIME * ochange, loc, EVENT_REPAIR_FIX, mech,
	"You start fixing the internals..");
    STARTIREPAIR((FIXINTERNAL_TIME * ((change > 0) ? (change) : (1))),
	mech, (change * 16 + loc),
	event_mech_repairinternal, EVENT_REPAIR_FIXI,
	((change > 0) ? (change) : (1)));
}


#define CHECK(tloc,nloc) \
case tloc:if (SectIsDestroyed(mech,nloc))return 1;break;

int Invalid_Repair_Path(MECH * mech, int loc)
{
    if (MechType(mech) != CLASS_MECH)
	return 0;
    switch (loc) {
	CHECK(HEAD, CTORSO);
	CHECK(LTORSO, CTORSO);
	CHECK(RTORSO, CTORSO);
	CHECK(LARM, LTORSO);
	CHECK(RARM, RTORSO);
	CHECK(LLEG, CTORSO);
	CHECK(RLEG, CTORSO);
    }
    return 0;
}

int unit_is_fixable(MECH * mech)
{
    int i;

    if (mudconf.btech_laxcore) {
	if (MechType(mech) == CLASS_MECH)
	    if (SectIsDestroyed(mech, CTORSO) && SectIsDestroyed(mech, LTORSO) && SectIsDestroyed(mech, RTORSO))
		return 0;
	    else
		return 1;
	}

    for (i = 0; i < NUM_SECTIONS; i++)
    {
	if (!GetSectOInt(mech, i))
	    continue;
	if (!SectIsDestroyed(mech, i))
	    continue;
	if (MechType(mech) == CLASS_MECH)
	    if (i == CTORSO && (SectIsDestroyed(mech, LTORSO) || SectIsDestroyed(mech, RTORSO)))
		return 0;
/*	if (MechType(mech) == CLASS_VTOL) */
/*	    if (i != ROTOR) */
/*		return 0; */
	if (MechType(mech) == CLASS_VEH_GROUND) {
	    if ((!SectIsDestroyed(mech, RSIDE) && !SectIsDestroyed(mech, LSIDE)) || (!SectIsDestroyed(mech, FSIDE) &&
		!SectIsDestroyed(mech, BSIDE)))
		{
		return 1;
	     } else {
		return 0;
		}
	}
	if (MechType(mech) == CLASS_VTOL) {
            if ((!SectIsDestroyed(mech, RSIDE) && !SectIsDestroyed(mech, LSIDE)) || (!SectIsDestroyed(mech, FSIDE) &&
                !SectIsDestroyed(mech, BSIDE)))
                {
                return 1;
             } else {
                return 0;
                }
	}
    }
    return 1;
}

TECHCOMMANDH(tech_reattach)
{
    TECHCOMMANDB;

    TECHCOMMANDC;
    my_parsepart(&loc, NULL);
    DOCHECK(is_aero(mech), "Aerospace units don't have missing sections.....");
    DOCHECK(!SectIsDestroyed(mech, loc), "That section isn't destroyed!");
    DOCHECK(Invalid_Repair_Path(mech, loc),
	"You need to reattach adjacent locations first!");
    DOCHECK(SomeoneAttaching(mech, loc),
	"Someone's attaching that section already!");
    DOCHECK(!unit_is_fixable(mech),
	"You see nothing to reattach it to (read:unit is cored).");
    CHECKREPAIRTIME(player,  REATTACH_TIME(MechTons(mech)));
    DOTECH_LOC(REATTACH_DIFFICULTY, reattach_fail, reattach_succ,
	reattach_econ, REATTACH_TIME(MechTons(mech)), mech, loc, event_mech_reattach,
	EVENT_REPAIR_REAT, "You start replacing the section..");
}

/*
 * Reseal
 * Added by Kipsta
 * 8/4/99
 */

TECHCOMMANDH(tech_reseal)
{
    TECHCOMMANDB;

    TECHCOMMANDC;
    my_parsepart(&loc, NULL);
    DOCHECK(SectIsDestroyed(mech, loc), "That section is destroyed!");
    DOCHECK(!SectIsFlooded(mech, loc), "That has not been flooded!");
    DOCHECK(Invalid_Repair_Path(mech, loc),
	"You need to reattach adjacent locations first!");
    DOCHECK(SomeoneResealing(mech, loc),
	"Someone's sealing that section already!");
    CHECKREPAIRTIME(player, RESEAL_TIME);
    DOTECH_LOC(RESEAL_DIFFICULTY, reseal_fail, reseal_succ, reseal_econ,
	RESEAL_TIME, mech, loc, event_mech_reseal, EVENT_REPAIR_RESE,
	"You start resealing the section.");
}

TECHCOMMANDH(tech_magic)
{
    TECHCOMMANDB;

    TECHCOMMANDC;
    notify(player, "Doing the magic..");
    do_magic(mech);
    mech_int_check(mech, 1);
    notify(player, "Done!");
}
