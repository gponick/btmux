#include "mech.h"
#include "mech.events.h"
#include "mech.tech.h"
#include "mech.tech.damages.h"
#include "coolmenu.h"
#include "mycool.h"
#include "p.mech.utils.h"
#include "p.mech.tech.commands.h"
#include "p.mech.status.h"
#include "p.mech.build.h"

/* 0 = type, 1 = loc, 2 = (pos/amnt), 3 = (val) */
short damage_table[MAX_DAMAGES][3];
int damage_last;

const char *repair_need_msgs[] = {
    "Reattachment",
    "Repairs on %s",
    "Temporary glitch fixes on %s",
    "Replacement of entire %s",
    "Reload of %s (%d rounds)",
    "Repairs on armor (%d points)",
    "Repairs on rear armor (%d points)",
    "Repairs on internals (%d points)",
    "Removal of section",
    "Removal of %s",
    "Removal of entire %s",
    "Unload of %s (%d rounds)",
    "Reseal"
};

#define CHECK(loc) check_for_damage(mech,loc)
#define DAMAGE2(a,b) do {\
   damage_table[damage_last][0]=a;\
   damage_table[damage_last++][1]=b;\
 } while (0)
#define DAMAGE3(a,b,c) do {\
   damage_table[damage_last][0]=a;\
   damage_table[damage_last][1]=b;\
   damage_table[damage_last++][2]=c;\
 } while (0)

static int check_for_damage(MECH * mech, int loc)
{
    int a, b, c, d, w = 0;

    if (!is_aero(mech) && SectIsDestroyed(mech, loc)) {
	DAMAGE2(REATTACH, loc);
	return 0;
    }

    /*
     * Added by Kipsta
     * 8/4/99
     */

    if (SectIsFlooded(mech, loc)) {
	DAMAGE2(RESEAL, loc);
	return 0;
    }
    if (!is_aero(mech) && ((a = GetSectInt(mech, loc)) != (b = GetSectOInt(mech, loc))))
	DAMAGE3(FIXINTERNAL, loc, (b - a));
    else {
	if ((a = GetSectArmor(mech, loc)) != (b =
		GetSectOArmor(mech, loc))) DAMAGE3(FIXARMOR, loc, (b - a));
	if ((a = GetSectRArmor(mech, loc)) != (b =
		GetSectORArmor(mech, loc))) DAMAGE3(FIXARMOR_R, loc,
		(b - a));
    }
    for (a = 0; a < NUM_CRITICALS; a++) {
	if (!(b = GetPartType(mech, loc, a)))
	    continue;
	if ((IsAmmo(b) && !PartIsDestroyed(mech, loc, a) && (c = GetPartData(mech, loc, a)) != (d = FullAmmo(mech, loc, a))) ||
		((GetPartMode(mech, loc, a) & OS_MODE) && (GetPartMode(mech, loc, a) & OS_USED) && !PartIsDestroyed(mech, loc, a)))
            DAMAGE3(RELOAD, loc, a);
	if (!PartIsDestroyed(mech, loc, a) && !PartIsDisabled(mech, loc, a) && GetPartDamage(mech, loc, a) == 0)
	    continue;
	if (IsCrap(b))
	    continue;
	/* Destroyed / tempnuke'd part. Either case, it works for us :) */
	if (IsWeapon(b))
	    w = (WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, a), GetWeaponCrits(mech, Weapon2I(b))));
	DAMAGE3(IsWeapon(b) && (GetPartDamage(mech, loc, a) != 0 || PartIsDisabled(mech, loc, a)) ? REPAIRP_T : IsWeapon(b) && w == 2 ? REPAIRG : REPAIRP, loc, a);
	if (IsWeapon(b) && w == 2 || (IsWeapon(b) && (GetPartDamage(mech, loc, a) != 0 || PartIsDisabled(mech, loc, a))))
	    a += GetWeaponCrits(mech, Weapon2I(b)) - 1;
    }
    return 1;
}

static int check_for_scrappage(MECH * mech, int loc)
{
    int a, b, w = 0;
    int ret = 1;

    if (SectIsDestroyed(mech, loc))
	return 1;

    if (SomeoneScrappingLoc(mech, loc)) {
	DAMAGE2(DETACH, loc);
	return 1;
    }
    for (a = 0; a < NUM_CRITICALS; a++) {
	if (!(b = GetPartType(mech, loc, a)))
	    continue;
	if (PartIsDestroyed(mech, loc, a))
	    continue;
	if (IsCrap(b))
	    continue;
	if (IsAmmo(b) && GetPartData(mech, loc, a)) {
	    DAMAGE3(UNLOAD, loc, a);
	    if (ret && !SomeoneRepairing(mech, loc, a))
		ret = 0;
	    continue;
	}

	if (IsWeapon(b))
		w = (WeaponIsNonfunctional(mech, loc, WeaponFirstCrit(mech, loc, a), GetWeaponCrits(mech, Weapon2I(b))));
	DAMAGE3((IsWeapon(b) && w == 0) ? SCRAPG : SCRAPP, loc, a);
	if (ret && !SomeoneScrappingPart(mech, loc, a))
	    ret = 0;
	if (IsWeapon(b) && w == 0)
	    a += GetWeaponCrits(mech, Weapon2I(b)) - 1;
    }

    if (ret && !Invalid_Scrap_Path(mech, loc))
	DAMAGE2(DETACH, loc);

    return 0;
}

void make_scrap_table(MECH * mech)
{
    int i = 4;

    damage_last = 0;
    if (MechType(mech) == CLASS_MECH) {
	if (check_for_scrappage(mech, RARM))
	    i -= check_for_scrappage(mech, RTORSO);
	if (check_for_scrappage(mech, LARM))
	    i -= check_for_scrappage(mech, LTORSO);
	i -= check_for_scrappage(mech, RLEG);
	i -= check_for_scrappage(mech, LLEG);

	if (!i)
	    check_for_scrappage(mech, CTORSO);

	check_for_scrappage(mech, HEAD);
    } else
	for (i = 0; i < NUM_SECTIONS; i++)
	    if (GetSectOInt(mech, i))
		check_for_scrappage(mech, i);
}

void make_damage_table(MECH * mech)
{
    int i;

    damage_last = 0;
    if (MechType(mech) == CLASS_MECH) {
	if (check_for_damage(mech, CTORSO)) {
	    if (check_for_damage(mech, LTORSO)) {
		CHECK(LARM);
	    }
	    if (check_for_damage(mech, RTORSO)) {
		CHECK(RARM);
	    }
	    CHECK(LLEG);
	    CHECK(RLEG);
	    CHECK(HEAD);
	}
    } else
	for (i = 0; i < NUM_SECTIONS; i++)
	    if (GetSectOInt(mech, i))
		check_for_damage(mech, i);
    if (is_aero(mech) && AeroSI(mech) != AeroSIOrig(mech))
	DAMAGE3(FIXINTERNAL, 0, (AeroSIOrig(mech) - AeroSI(mech)));
}

int is_under_repair(MECH * mech, int i)
{
    int v1 = damage_table[i][1];
    int v2 = damage_table[i][2];

    switch (damage_table[i][0]) {
    case RELOAD:
    case REPAIRP:
    case REPAIRP_T:
    case REPAIRG:
    case UNLOAD:
	return SomeoneRepairing(mech, v1, v2);
    case REATTACH:
	return SomeoneAttaching(mech, v1);
    case RESEAL:
	return SomeoneResealing(mech, v1);
    case FIXARMOR_R:
	return SomeoneFixing(mech, v1 + 8);
    case FIXARMOR:
    case FIXINTERNAL:
	return SomeoneFixing(mech, v1);
    case DETACH:
	return SomeoneScrappingLoc(mech, v1);
    case SCRAPP:
    case SCRAPG:
	return SomeoneScrappingPart(mech, v1, v2);
    }
    return 0;
}

char *damages_func(MECH * mech)
{
    static char buffer[MBUF_SIZE];
    int i;

    if (unit_is_fixable(mech))
	make_damage_table(mech);
    else
	make_scrap_table(mech);

    buffer[0] = '\0';
    if (!damage_last)
	return "";
    for (i = 0; i < damage_last; i++) {
	/* Ok... i think we want: */
	/* repairnum|location|typenum|data|fixing? */
	if (i)
	    sprintf(buffer, "%s:", buffer);
	sprintf(buffer, "%s%d|%s|%d|", buffer, i + 1,
	    ShortArmorSectionString(MechType(mech), MechMove(mech),
		damage_table[i][1]), (int) damage_table[i][0]);
	switch (damage_table[i][0]) {
	case REPAIRP:
	case REPAIRP_T:
	case REPAIRG:
	case SCRAPP:
	case SCRAPG:
	    sprintf(buffer, "%s%s", buffer, pos_part_name(mech,
		    damage_table[i][1], damage_table[i][2]));
	    break;
	case RELOAD:
	    sprintf(buffer, "%s%s:%d", buffer, pos_part_name(mech,
		    damage_table[i][1], damage_table[i][2]), ((GetPartMode(mech,
		    damage_table[i][1], damage_table[i][2]) & OS_USED) ? 1 :
		    (FullAmmo(mech,
		    damage_table[i][1],
		    damage_table[i][2]) - GetPartData(mech,
		    damage_table[i][1], damage_table[i][2]))));
	    break;
	case UNLOAD:
	    sprintf(buffer, "%s%s:%d", buffer, pos_part_name(mech,
		    damage_table[i][1], damage_table[i][2]),
		GetPartData(mech, damage_table[i][1], damage_table[i]
		    [2]));
	    break;
	case FIXARMOR:
	case FIXARMOR_R:
	case FIXINTERNAL:
	    sprintf(buffer, "%s%d", buffer, damage_table[i][2]);
	    break;
	default:
	    sprintf(buffer, "%s-", buffer);
	}
	sprintf(buffer, "%s|%d", buffer, is_under_repair(mech, i));
    }
    return buffer;
}

void show_mechs_damage(dbref player, void *data, char *buffer)
{
    MECH *mech = data;
    coolmenu *c = NULL;
    int i, j, v1, v2;
    char buf[MBUF_SIZE];
    char buf2[MBUF_SIZE];
    int isds;

    //    TECHCOMMANDC;
    DOCHECK(!(Tech(player)),"Insufficient clearance to access the command.");
    DOCHECK(!mech, "Error has occured in techcommand ; please contact a wiz");
    DOCHECK(Starting(mech) && !Wiz(player), "The mech's starting up! Please stop the sequence first.");
    DOCHECK(Started(mech) && !Wiz(player), "The mech's started up ; please shut it down first.");
    if (unit_is_fixable(mech))
	make_damage_table(mech);
    else
	make_scrap_table(mech);
    DOCHECK(!damage_last &&
	MechType(mech) == CLASS_MECH,
	"The 'mech is in pristine condition!");
    DOCHECK(!damage_last, "It's in pristine condition!");
    addline();
    cent(tprintf("Damage for %s", GetMechID(mech)));
    addline();
    for (i = 0; i < damage_last; i++) {
	v1 = damage_table[i][1];
	v2 = damage_table[i][2];
	switch (damage_table[i][0]) {
	case REATTACH:
	case DETACH:
	case RESEAL:
	    strcpy(buf, repair_need_msgs[(int) damage_table[i][0]]);
	    break;
	case REPAIRP:
	case REPAIRP_T:
	case REPAIRG:
	case SCRAPP:
	case SCRAPG:
	    sprintf(buf, repair_need_msgs[(int) damage_table[i][0]],
		pos_part_name(mech, v1, v2));
	    break;
	case RELOAD:
	    sprintf(buf, repair_need_msgs[(int) damage_table[i][0]],
		pos_part_name(mech, v1, v2), ((GetPartMode(mech,
		v1, v2) & OS_MODE) ? 1 : (FullAmmo(mech, v1,
		    v2) - GetPartData(mech, v1, v2))));
	    break;
	case UNLOAD:
	    sprintf(buf, repair_need_msgs[(int) damage_table[i][0]],
		pos_part_name(mech, v1, v2), GetPartData(mech, v1, v2));
	    break;
	case FIXARMOR:
	case FIXARMOR_R:
	case FIXINTERNAL:
	    sprintf(buf, repair_need_msgs[(int) damage_table[i][0]],
		damage_table[i][2]);
	    break;
	}
	j = is_under_repair(mech, i);
	sprintf(buf2, "%%ch%s%-2d:%s %%cn%s%s", j ? "%cg" : "%cy", i + 1,
	    ShortArmorSectionString(MechType(mech), MechMove(mech), v1),
	    buf, j ? " (*)" : "");
	vsi(buf2);
    }
    addline();
    vsi
	("(*) / %ch%cgGreen%cn = Job already done. %ch%cyYellow%cn = To be done.");
    addline();
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}

static void fix_entry(dbref player, MECH * mech, int n)
{
    char buf[MBUF_SIZE];
    char *c;
    int first;

    /* whee */
    n--;
    c = ShortArmorSectionString(MechType(mech), MechMove(mech), damage_table[n][1]);
    switch (damage_table[n][0]) {
    case REPAIRP_T:
	first = WeaponFirstCrit(mech, damage_table[n][1], damage_table[n][2]);
	sprintf(buf, "%s %d", c, first + 1);
	tech_repairgun(player, mech, buf);
	break;
    case REPAIRG:
	first = WeaponFirstCrit(mech, damage_table[n][1], damage_table[n][2]);
	sprintf(buf, "%s %d", c, first + 1);
	tech_replacegun(player, mech, buf);
	break;
    case REPAIRP:
	sprintf(buf, "%s %d", c, damage_table[n][2] + 1);
	tech_replacepart(player, mech, buf);
	break;
    case RELOAD:
	sprintf(buf, "%s %d", c, damage_table[n][2] + 1);
	tech_reload(player, mech, buf);
	break;
    case REATTACH:
	sprintf(buf, "%s", c);
	tech_reattach(player, mech, buf);
	break;
    case RESEAL:
	sprintf(buf, "%s", c);
	tech_reseal(player, mech, buf);
	break;
    case FIXARMOR:
	sprintf(buf, "%s", c);
	tech_fixarmor(player, mech, buf);
	break;
    case FIXARMOR_R:
	sprintf(buf, "%s r", c);
	tech_fixarmor(player, mech, buf);
	break;
    case FIXINTERNAL:
	sprintf(buf, "%s", c);
	tech_fixinternal(player, mech, buf);
	break;
    case DETACH:
	sprintf(buf, "%s", c);
	tech_removesection(player, mech, buf);
	break;
    case SCRAPP:
	sprintf(buf, "%s %d", c, damage_table[n][2] + 1);
	tech_removepart(player, mech, buf);
	break;
    case SCRAPG:
	sprintf(buf, "%s %d", c, damage_table[n][2] + 1);
	tech_removegun(player, mech, buf);
	break;
    case UNLOAD:
	sprintf(buf, "%s %d", c, damage_table[n][2] + 1);
	tech_unload(player, mech, buf);
	break;
    }
}

void tech_fix(dbref player, void *data, char *buffer)
{
    MECH *mech = data;
    int n = atoi(buffer);
    int low, high;
    int isds;

    skipws(buffer);
    //    TECHCOMMANDC;
    DOCHECK(!(Tech(player)),"Insufficient clearance to access the command.");
    DOCHECK(!mech, "Error has occured in techcommand ; please contact a wiz");
    DOCHECK(Starting(mech) && !Wiz(player), "The mech's starting up! Please stop the sequence first.");
    DOCHECK(Started(mech) && !Wiz(player), "The mech's started up ; please shut it down first.");
   if (unit_is_fixable(mech))
	make_damage_table(mech);
    else
	make_scrap_table(mech);
    DOCHECK(!damage_last &&
	MechType(mech) == CLASS_MECH,
	"The 'mech is in pristine condition!");
    DOCHECK(!damage_last, "It's in pristine condition!");
    if (sscanf(buffer, "%d-%d", &low, &high) == 2) {
	DOCHECK(low < 1 || low > damage_last, "Invalid low #!");
	DOCHECK(high < 1 || high > damage_last, "Invalid high #!");
	for (n = low; n <= high; n++)
	    fix_entry(player, mech, n);
	return;
    }
    DOCHECK(n < 1 || n > damage_last, "Invalid #!");
    fix_entry(player, mech, n);
}
