#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "map.h"
#include "p.mech.utils.h"
#include "p.mech.los.h"

char *default_contactoptions = "!db";

static char *ac_desc[] = {
    "0 - See enemies and friends, long text, color",
    "1 - See enemies and friends, short text, color",
    "2 - See enemies only, long text, color",
    "3 - See enemies only, short text, color",
    "4 - See enemies and friends, short text, no color",
    "5 - See enemies only, short text, no color",

    "6 - Disabled"
};

static char *c_desc[] = {
    "0 - Very verbose",
    "1 - Short form, the usual one",
    "2 - Short form, the usual one, but do not see buildings",
    "3 - Shorter form"
};

#define EmitECCM(mech)	  ((MechStatus2((mech)) & ECCM_ACTIVE) && Started(mech))
#define EmitECM(mech)	  ((MechStatus((mech)) & ECM_ACTIVE) && Started(mech))
#define Disturbed(mech)   ((MechStatus((mech)) & ECM_DISTURBANCE) && Started(mech))
#define Protected(mech)   ((MechStatus2((mech)) & ECM_PROTECTED) && Started(mech))
#define Jellied(mech)     ((MechStatus(mech) & JELLIED))
#define ProbeOn(mech)	  ((MechStatus2(mech) & BEAGLE_ACTIVE) && Started(mech))
#define UnJamming(mech)   ((MechStatus2(mech) & UNJAMMING) && Started(mech))
#define Stunned(mech)     (CrewStunning(mech) && Started(mech))
#define Evading(mech)	  (MechStatus2(mech) & EVADING)
#define Sprinting(mech)	  (MechStatus2(mech) & SPRINTING)

/* Old System - Changed right after made it. Hee Hee. - DJ 9/08/00 */
/* #define HeatChar(t)    (((MechType(t) == CLASS_MECH) ? (MechPlusHeat(t) * 10.) /  (MechMinusHeat(t) * 10.) : .01) > 2.75 ? '*' : \
	((MechType(t) == CLASS_MECH) ? (MechPlusHeat(t) * 10.) /  (MechMinusHeat(t) * 10.) : .01) > 2.25 ? '+' : \
	((MechType(t) == CLASS_MECH) ? (MechPlusHeat(t) * 10.) /  (MechMinusHeat(t) * 10.) : .01) > 1.75 ? '-' : ' ') */
#define HeatChar(mech) (HeatFactor(mech) > 50 ? '+' : HeatFactor(mech) > 35 ? '-' : ' ')


void show_brief_flags(dbref player, MECH * mech)
{
    notify(player, tprintf("Brief status for %s:", GetMechToMechID(mech,
		mech)));
#ifdef ADVANCED_LOS
    notify(player, tprintf("    (A)utocontacts: %s",
	    ac_desc[mech->brief / 4]));
#endif
    notify(player, tprintf("    (C)ontacts:     %s",
	    c_desc[mech->brief % 4]));
}

void mech_brief(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char c;
    int v;

    cch(MECH_USUALSM);
    skipws(buffer);
    if (!*buffer) {
	show_brief_flags(player, mech);
	return;
    }
    c = *buffer;
    buffer++;
    skipws(buffer);
    DOCHECK(!*buffer, "Argument missing!");
    DOCHECK(Readnum(v, buffer), "Invalid number!");
    switch (toupper(c)) {
#ifdef ADVANCED_LOS
    case 'A':
	DOCHECK(v < 0 || v > 6, "Number out of range!");
	v = BOUNDED(0, v, 6);
	mech->brief = mech->brief % 4;
	mech->brief += v * 4;
	mech_notify(mech, MECHALL,
	    tprintf("Autocontact brevity set to %s.", ac_desc[v]));
	return;
#endif
    case 'C':
	DOCHECK(v < 0 || v > 3, "Number out of range!");
	v = BOUNDED(0, v, 3);
	mech->brief = ((mech->brief / 4) * 4) + v;
	mech_notify(mech, MECHALL, tprintf("Contact brevity set to %s.",
		c_desc[v]));
	return;
    }
}


#define SEE_DEAD	0x01
#define SEE_SHUTDOWN	0x02
#define SEE_ALLY	0x04
#define SEE_ENEMA	0x08
#define SEE_TARGET	0x10
#define SEE_BUILDINGS	0x20
#define SEE_NEGNEXT	0x80
#define SEE_INCHAR	0x100

void mech_contacts(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data, *tempMech;
    MAP *mech_map = getMap(mech->mapindex), *tmp_map;
    mapobj *building;
    int loop, i, j, argc, bearing, buffindex = 0;
    char *args[1], bufflist[MAX_MECHS_PER_MAP][120], buff[100];
    int sbuff[MAX_MECHS_PER_MAP];
    float range, rangelist[MAX_MECHS_PER_MAP], fx, fy;
    int mechfound;
    char weaponarc;
    char *mech_name;
    int see_what;
    char *str;
    char move_type[30];
    int arc;
    int losflag;
    int isvb;
    int inlos;
    int IsUsingHUD = 0;		//For the HUD - Kipsta July, 29, 1999

    cch(MECH_USUAL);
    mechfound = 0;
    argc = mech_parseattributes(buffer, args, 1);

    isvb = (mech->brief % 4);
    if (argc > 0) {
	if (args[0][0] == 'h') {
	    IsUsingHUD = 1;
	    see_what =
		(SEE_DEAD | SEE_SHUTDOWN | SEE_ENEMA | SEE_ALLY |
		SEE_TARGET);
	} else {
	    if (args[0][0] == '+') {
		str = silly_atr_get(player, A_CONTACTOPT);
		if (!*str)
		    strcpy(buff, default_contactoptions);
		else {
		    strncpy(buff, str, 50);
		    buff[49] = 0;

		    if (strlen(buff) == 0)
			strcpy(buff, default_contactoptions);
		}
	    } else {
		strncpy(buff, args[0], 50);
		buff[49] = 0;
	    }

	    if (isvb == 1)
		see_what = SEE_BUILDINGS;
	    else
		see_what = 0x0;

	    for (loop = 0; loop < 50 && buff[loop]; loop++) {
		char c;

		c = buff[loop];

		if (c == 'd')

		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_DEAD) : (see_what |= SEE_DEAD);
		else if (c == 's')
		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_SHUTDOWN) : (see_what |= SEE_SHUTDOWN);
		else if (c == 'b')
		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_BUILDINGS) : (see_what |= SEE_BUILDINGS);
		else if (c == 'e')
		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_ENEMA) : (see_what |= SEE_ENEMA);
		else if (c == 'a')
		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_ALLY) : (see_what |= SEE_ALLY);
		else if (c == 't')
		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_TARGET) : (see_what |= SEE_TARGET);
		else if (c == 'i')
		    (see_what & SEE_NEGNEXT) ? (see_what &=
			~SEE_INCHAR) : (see_what |= SEE_INCHAR);
		else if (c == '!') {
		    see_what =
			(SEE_NEGNEXT | SEE_DEAD | SEE_SHUTDOWN | SEE_ENEMA | SEE_ALLY | SEE_TARGET | SEE_INCHAR);
		} else
		    notify(player,
			tprintf("Ignoring %c as contact option.", c));
	    }
	}
    } else {
	see_what =
	    (SEE_DEAD | SEE_SHUTDOWN | SEE_ENEMA | SEE_ALLY | SEE_TARGET);
	if (isvb == 1)
	    see_what |= SEE_BUILDINGS;
    }

    if (!IsUsingHUD && isvb <= 2)
	notify(player, "Line of Sight Contacts:");


    for (loop = 0; loop < mech_map->first_free; loop++) {
	if (!(mech_map->mechsOnMap[loop] != mech->mynum &&
		mech_map->mechsOnMap[loop] != -1))
	    continue;

	tempMech = (MECH *) FindObjectsData(mech_map->mechsOnMap[loop]);

	if (!tempMech)
	    continue;
	if (argc) {
	    if (!(((MechTeam(tempMech) ==
			    MechTeam(mech)) ? (see_what & SEE_ALLY)
			: (see_what & SEE_ENEMA)) ||
		    ((see_what & SEE_TARGET) &&
			(tempMech->mynum == MechTarget(mech)))))
		continue;
	    if (!(((see_what & SEE_SHUTDOWN) || Started(tempMech)) ||
		    Destroyed(tempMech) || ((see_what & SEE_TARGET) &&
			(tempMech->mynum == MechTarget(mech)))))
		continue;
	    if (!(((see_what & SEE_DEAD) || !Destroyed(tempMech)) ||
		    ((see_what & SEE_TARGET) &&
			(tempMech->mynum == MechTarget(mech)))))
		continue;
	    if ((see_what & SEE_INCHAR) && !In_Character(tempMech->mynum))
		continue;
	}
	range = FlMechRange(mech_map, mech, tempMech);
	if (!(losflag =
		InLineOfSight(mech, tempMech, MechX(tempMech),
	    MechY(tempMech), range)))
	    continue;
	if (Good_obj(tempMech->mynum)) {
	    if (!InLineOfSight_NB(mech, tempMech, MechX(tempMech),
		    MechY(tempMech), 0.0)) {
		mech_name = "something";
		inlos = 0;
	    } else {
		mech_name = silly_atr_get(tempMech->mynum, A_MECHNAME);
		inlos = 1;
	    }
	} else
	    continue;
	bearing =
	    FindBearing(MechFX(mech), MechFY(mech), MechFX(tempMech),
	    MechFY(tempMech));
	arc = InWeaponArc(mech, MechFX(tempMech), MechFY(tempMech));
	if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT) {
	    if (arc & FORWARDARC)
		weaponarc = '*';
	    else if (arc & LSIDEARC)
		weaponarc = 'l'; 
	    else if (arc & RSIDEARC)
		weaponarc = 'r';
	    else if (arc & REARARC)
		weaponarc = 'v';
	    else
		weaponarc = ' ';
	} else {
	    if (arc & TURRETARC)
		weaponarc = 't';
	    else if (arc & FORWARDARC)
	        weaponarc = '*';
	    else if (arc & RSIDEARC)
	        weaponarc = 'r';
	    else if (arc & LSIDEARC)
		weaponarc = 'l';
	    else if (arc & REARARC)
		weaponarc = 'v';
	    else
		weaponarc = ' ';
	}
	strcpy(move_type, GetMoveTypeID(MechMove(tempMech)));

	if (isvb) {
	    if (!IsUsingHUD) {
#ifdef SIMPLE_SENSORS
		sprintf(buff,
		    "%s%c[%s]%c %-12.12s x:%3d y:%3d z:%3d r:%4.1f b:%3d s:%5.1f h:%3d S:%c%c%c%c%c%c%s",
#else
		sprintf(buff,
		    "%s%c%c%c[%s]%c %-12.12s x:%3d y:%3d z:%3d r:%4.1f b:%3d s:%5.1f h:%3d%s S:%c%c%c%c%c%c%c%c%s",
#endif
		    tempMech->mynum == MechTarget(mech) ? "%ch%cr" : (inlos
			&& MechTeam(tempMech) !=
			MechTeam(mech)) ? "%ch%cy" : "",
#ifndef SIMPLE_SENSORS
		    (losflag & MECHLOSFLAG_SEESP) ? 'P' : ' ',
		    (losflag & MECHLOSFLAG_SEESS) ? 'S' : ' ',
#endif
		    weaponarc, MechIDS(tempMech,
			MechTeam(mech) == MechTeam(tempMech) ||
			!inlos), move_type[0], mech_name, MechX(tempMech),
		    MechY(tempMech), MechZ(tempMech), range, bearing,
		    is_aero(tempMech) && !Landed(tempMech) ? MechVelocity(tempMech) : MechSpeed(tempMech), MechVFacing(tempMech),
		    is_aero(tempMech) && !Landed(tempMech) ? tprintf(" a:%3d", MechAngle(tempMech)) : "",
		    inlos ? (MechSwarmTarget(tempMech) >
	 	    0 ? 'W' : MechIsSwarmed(tempMech) ? 'w' : Towed(tempMech) ? 'T' :
		    MechCarrying(tempMech) >
		    0 ? 't' : Fallen(tempMech) ? 'F' :
		    Standing(tempMech) ? 'f' : UnJamming(tempMech) ? 'U' : Stunned(tempMech) ?
		    'X' : ' ') : Fallen(tempMech) ? 'f' : ' ',
		    inlos ? Destroyed(tempMech) ? 'D' : IsLit3(tempMech) ?
		    'L' : IsLit2(tempMech) ? 'l' : ' ' : ' ',
		    Jumping(tempMech) ? 'J' : OODing(tempMech) ? 'O' : ' ',
		    Started(tempMech) ? ' ' : Starting(tempMech) ? 's' :
		    'S', ((MechStatus(tempMech) & NARC_ATTACHED) || (MechStatus2(tempMech) & INARC_ATTACHED))
		    ? (MechTeam(tempMech) == MechTeam(mech) ? 'n' : 'N') : ' ',
		    Disturbed(tempMech) ? 'e' : EmitECM(tempMech) ? 'E' :
		    Protected(tempMech) ? 'p' : EmitECCM(tempMech) ? 'P' : ' ', 
		    Evading(tempMech) ? 'm' : Sprinting(tempMech) ? 'M' : Jellied(tempMech) ? 'I' :
		    Dugin(tempMech) ? 'd' : ProbeOn(tempMech) ? 'A' : ' ', Spinning(tempMech) ? 'Z' :
		    HeatChar(tempMech), (tempMech->mynum == MechTarget(mech) || MechTeam(tempMech) != MechTeam(mech)) ? "%c" : "");
	    } else {
#ifdef SIMPLE_SENSORS
		sprintf(buff,
		    "#HUDINFO:CON#%c,%c,%s,%c,%-12.12s,%3d,%3d,%3d,%4.1f,%3d,%4.1f,%3d,%c%c%c%c%c%c",	/* ) <- balance */
#else
		sprintf(buff,
		    "#HUDINFO:CON#%c,%c,%c,%c,%s,%c,%-12.12s,%3d,%3d,%3d,%4.1f,%3d,%4.1f,%3d,%c%c%c%c%c%c%c%c",
#endif
		    (tempMech->mynum == MechTarget(mech)) ? 'T' : (MechTeam
			(tempMech) != MechTeam(mech)) ? 'E' : 'F',
#ifndef SIMPLE_SENSORS
		    (losflag & MECHLOSFLAG_SEESP) ? 'P' : ' ',
		    (losflag & MECHLOSFLAG_SEESS) ? 'S' : ' ',
#endif
		    weaponarc, MechIDS(tempMech,
			MechTeam(mech) == MechTeam(tempMech) ||
			!inlos), move_type[0], mech_name, MechX(tempMech),
		    MechY(tempMech), MechZ(tempMech), range, bearing,
		    is_aero(tempMech) && !Landed(tempMech) ? MechVelocity(tempMech) : MechSpeed(tempMech), MechVFacing(tempMech),
		    inlos ? (MechSwarmTarget(tempMech) >
		    0 ? 'W' : MechIsSwarmed(tempMech) ? 'w' : Towed(tempMech) ? 'T' :
		    MechCarrying(tempMech) >
		    0 ? 't' : Fallen(tempMech) ? 'F' :
		    Standing(tempMech) ? 'f' : ' ') : Fallen(tempMech)
		    ? 'f' : UnJamming(mech) ? 'U' : Stunned(mech) ? 'X' : ' ',
		    inlos ? Destroyed(tempMech) ? 'D' : IsLit2(tempMech) ?
		    'l' : IsLit3(tempMech) ? 'L' : ' ' : ' ',
		    Jumping(tempMech) ? 'J' : OODing(tempMech) ? 'O' : ' ',
		    Started(tempMech) ? ' ' : Starting(tempMech) ? 's' :
		    'S', ((MechStatus(tempMech) & NARC_ATTACHED) || (MechStatus2(tempMech) & INARC_ATTACHED))
		    ? (MechTeam(tempMech) == MechTeam(mech) ? 'n' : 'N') : ' ', Disturbed(tempMech) ? 'e' :
		    EmitECM(tempMech) ? 'E' : Protected(tempMech) ? 'p' : EmitECCM(tempMech) ? 'P' : ' ',
		    Evading(tempMech) ? 'm' : Sprinting(tempMech) ? 'M' :
		    Jellied(tempMech) ? 'I' : Dugin(tempMech) ? 'd' : ProbeOn(tempMech) ? 'A' :' ', 
		    Spinning(tempMech) ? 'Z' : HeatChar(tempMech));
	    }

	    rangelist[buffindex] = range;
	    rangelist[buffindex] +=
		(MechStatus(tempMech) & DESTROYED) ? 10000 : 0;
	    strcpy(bufflist[buffindex++], buff);
	} else {
	    sprintf(buff, "[%s] %-17s  Tonnage: %d", MechIDS(tempMech,
		    MechTeam(tempMech) == MechTeam(mech)), mech_name,
		MechTons(tempMech));
	    notify(player, buff);
	    sprintf(buff, "      Range: %.1f hex\tBearing: %d degrees",
		range, bearing);
	    notify(player, buff);
	    sprintf(buff, "      Speed: %.1f KPH\tHeading: %d degrees",
		MechSpeed(tempMech), MechVFacing(tempMech));
	    notify(player, buff);
	    if (is_aero(tempMech)) {
		sprintf(buff, "      Vlcty: %.1f KPH\t Angle: %d degrees",
		    MechVelocity(tempMech), MechAngle(tempMech));
		notify(player, buff);
		sprintf(buff, "      Thrust: %.1f", MechThrust(tempMech));
		notify(player, buff); 
		}
	    sprintf(buff, "      X, Y: %3d, %3d \tHeat: %.0f deg C.",
		MechX(tempMech), MechY(tempMech), MechHeat(tempMech));
	    notify(player, buff);
	    sprintf(buff, "      Movement Type: %s", move_type);
	    notify(player, buff);
	    notify(player, tprintf("      Mech is in %s Arc",
		    GetArcID(mech, InWeaponArc(mech, MechFX(tempMech),
			    MechFY(tempMech)))));
	    if (MechStatus(tempMech) & DESTROYED)
		notify(player, "      Mech Destroyed");
	    if (!(MechStatus(tempMech) & STARTED))
		notify(player, "      Mech Shutdown");
	    if (Fallen(tempMech))
		notify(player, "      Mech has Fallen!");
	    if (Dugin(tempMech)) {
		if (MechMove(tempMech) == MOVE_QUAD) {
			notify(player, "      Mech is Hulldown!");
		    } else {
			notify(player, "      Mech is Dugdown!");
		    }
		}
	    if (Jellied(tempMech))
		notify(player, "      Mech is Jellied!");
	    if (Evading(tempMech))
		notify(player, "      Mech is Evading!");
	    if (Sprinting(tempMech))
		notify(player, "      Mech is Sprinting!");
	    if (ProbeOn(tempMech))
		notify(player, "      Mech has an Active Probe on");
	    if (Disturbed(tempMech))
		notify(player, "      Mech is ECM disturbed!");
	    if (Protected(tempMech))
		notify(player, "      Mech is ECM protected!"); 
	    if (EmitECM(tempMech))
		notify(player, "      Mech is emitting ECM!");
	    if (EmitECCM(tempMech))
		notify(player, "      Mech is emitting ECCM!");
	    if (Jumping(tempMech))
		notify(player,
		    tprintf("      Mech is Jumping!\tJump Heading: %d",
			MechJumpHeading(tempMech)));
	    notify(player, " ");
	}
    }

    if (see_what & SEE_BUILDINGS) {
	for (building = first_mapobj(mech_map, TYPE_BUILD); building;
	    building = next_mapobj(building)) {

	    MapCoordToRealCoord(building->x, building->y, &fx, &fy);
	    range = FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), fx, fy, ZSCALE * ((i = Elevation(mech_map, building->x, building->y)) + 1));

	    losflag = InLineOfSight(mech, NULL, building->x, building->y, range);
	    if (!losflag || (losflag & MECHLOSFLAG_BLOCK))
		continue;

	    if (!(building->obj && (tmp_map = getMap(building->obj))))
		continue;
	    if (BuildIsInvis(tmp_map))
		continue;
	    if ((j = !can_pass_lock(mech->mynum, tmp_map->mynum, A_LENTER))
		&& BuildIsHidden(tmp_map))
		continue;
	    bearing = FindBearing(MechFX(mech), MechFY(mech), fx, fy);
	    arc = InWeaponArc(mech, fx, fy);
	    if (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT) {
		if (arc & FORWARDARC)
		    weaponarc = '*';
		else if (arc & LSIDEARC)
		    weaponarc = 'l';
		else if (arc & RSIDEARC)
		    weaponarc = 'r';
		else if (arc & REARARC)
		    weaponarc = 'v';
		else
		    weaponarc = ' ';
	    } else {
		if (arc & TURRETARC)
		    weaponarc = 't';
		else if (arc & FORWARDARC)
		    weaponarc = '*';
		else if (arc & RSIDEARC)
		    weaponarc = 'r';
		else if (arc & LSIDEARC)
		    weaponarc = 'l';
		else if (arc & REARARC)
		    weaponarc = 'v';
		else
		    weaponarc = ' ';
	    }

	    mech_name = silly_atr_get(building->obj, A_MECHNAME);
	    if (!mech_name || !*mech_name)
		mech_name = strip_ansi(Name(building->obj));

	    if (!IsUsingHUD) {
#ifdef SIMPLE_SENSORS
		sprintf(buff,
		    "%s%c %-23.23s x:%3d y:%3d z:%2d r:%4.1f b:%3d CF:%4d /%4d S:%c%c%s",
#else
		sprintf(buff,
		    "%s%c%c%c %-23.23s x:%3d y:%3d z:%2d r:%4.1f b:%3d CF:%4d /%4d S:%c%c%s",
#endif
		    j ? "%ch%cy" : "",
#ifndef SIMPLE_SENSORS
		    (losflag & MECHLOSFLAG_SEESP) ? 'P' : ' ',
		    (losflag & MECHLOSFLAG_SEESS) ? 'S' : ' ',
#endif
		    weaponarc, mech_name, building->x, building->y, i,
		    range, bearing, tmp_map->cf, tmp_map->cfmax,
		    (BuildIsSafe(tmp_map) || (j &&
			    BuildIsCS(tmp_map))) ? 'X' : j ? 'x' :
		    BuildIsCS(tmp_map) ? 'C' : ' ',
		    BuildIsHidden(tmp_map) ? 'H' : ' ', j ? "%c" : "");
	    } else {
#ifdef SIMPLE_SENSORS
		sprintf(buff,
		    "#HUDINFO:CON#%c,%c,%-21.21s,%3d,%3d,%3d,%4.1f,%3d,%4d,%4d,%c%c",	/* ) <- balance */
#else
		sprintf(buff,
		    "#HUDINFO:CON#%c,%c,%c,%c,%-21.21s,%3d,%3d,%3d,%4.1f,%3d,%4d,%4d,%c%c",
#endif
		    j ? 'E' : 'F',
#ifndef SIMPLE_SENSORS
		    (losflag & MECHLOSFLAG_SEESP) ? 'P' : ' ',
		    (losflag & MECHLOSFLAG_SEESS) ? 'S' : ' ',
#endif
		    weaponarc, mech_name, building->x, building->y, i,
		    range, bearing, tmp_map->cf, tmp_map->cfmax,
		    (BuildIsSafe(tmp_map) || (j &&
			    BuildIsCS(tmp_map))) ? 'X' : j ? 'x' :
		    BuildIsCS(tmp_map) ? 'C' : ' ',
		    BuildIsHidden(tmp_map) ? 'H' : ' ');
	    }
	    rangelist[buffindex] = range + 20000;
	    strcpy(bufflist[buffindex++], buff);
	}
    }


    if (isvb) {
	for (i = 0; i < buffindex; i++)
	    sbuff[i] = i;
	/* print a sorted list of detected mechs */
	/* use the ever-popular bubble sort */
	for (i = 0; i < (buffindex - 1); i++)
	    for (j = (i + 1); j < buffindex; j++)
		if (rangelist[sbuff[j]] > rangelist[sbuff[i]]) {
		    loop = sbuff[i];
		    sbuff[i] = sbuff[j];
		    sbuff[j] = loop;
		}
	for (loop = 0; loop < buffindex; loop++)
	    notify(player, bufflist[sbuff[loop]]);
    }
    if (!IsUsingHUD && isvb <= 2)
	notify(player, "End Contact List");
}

#undef SEE_DEAD
#undef SEE_SHUTDOWN
#undef SEE_ALLY
#undef SEE_ENEMA
#undef SEE_TARGET
#undef SEE_BUILDINGS
#undef SEE_NEGNEXT

/* who: 0 for friend, 1 for enemy, 2 for 'self' */
char *getStatusString(MECH * target, int who)
{
    static char statusstr[20];
    int sptr = 0;

    if (Destroyed(target))
        statusstr[sptr++] = 'D';

    if (Starting(target))
        statusstr[sptr++] = 's';
    else if (!Started(target))
        statusstr[sptr++] = 'S';

    if (Standing(target))
        statusstr[sptr++] = 'f';
    else if (Fallen(target))
        statusstr[sptr++] = 'F';

    if (ChangingHulldown(target))
        statusstr[sptr++] = 'h';
    else if (Dugin(target))
        statusstr[sptr++] = 'H';

    if (Towed(target))
        statusstr[sptr++] = 'T';
    else if (MechCarrying(target) > 0)
        statusstr[sptr++] = 't';

    if (Jumping(target))
        statusstr[sptr++] = 'J';

    if (OODing(target))
        statusstr[sptr++] = 'O';

    if (MechHeat(target))
        statusstr[sptr++] = '+';

    if (Jellied(target))
        statusstr[sptr++] = 'I';
/*
    if (Burning(target))
        statusstr[sptr++] = 'B';
*/
    if (IsLit3(target))
        statusstr[sptr++] = 'L';

    if (IsLit2(target))
        statusstr[sptr++] = 'l';

    if (MechSwarmTarget(target) > 0)
        statusstr[sptr++] = 'W';
/*
    if (CarryingClub(target))
        statusstr[sptr++] = 'C';
*/
    if (MechStatus(target) & NARC_ATTACHED || MechStatus2(target) & INARC_ATTACHED) {
        if (who == 1)
            statusstr[sptr++] = 'N';
        else
            statusstr[sptr++] = 'n';
    }

    if (EmitECCM(target))
        statusstr[sptr++] = 'P';

    if (EmitECM(target))
        statusstr[sptr++] = 'E';

    if (Protected(target))
        statusstr[sptr++] = 'p';

    if (Disturbed(target))
        statusstr[sptr++] = 'e';

    if (Spinning(target))
        statusstr[sptr++] = 'X';

    statusstr[sptr] = '\0';
    return statusstr;
}

char getStatusChar(MECH * mech, MECH * mechTarget, int wCharNum)
{
    char cRet = ' ';

    switch (wCharNum) {
    case 1:
        cRet = MechSwarmTarget(mechTarget) > 0 ? 'W' :
            Towed(mechTarget) ? 'T' : MechCarrying(mechTarget) >
            0 ? 't' : ' ';
        break;
    case 2:
        cRet = Destroyed(mechTarget) ? 'D' :
            IsLit2(mechTarget) ? 'l' : IsLit3(mechTarget) ? 'L' : ' ';
        break;
    case 3:
        cRet = Jumping(mechTarget) ? 'J' : OODing(mechTarget) ? 'O' :
            Fallen(mechTarget) ? 'F' : Standing(mechTarget) ? 'f' :
            ChangingHulldown(mechTarget) ? 'h' : Dugin(mechTarget) ?
            'H' : Spinning(mech) ? 'X' : ' ';
        break;
    case 4:
        cRet = Started(mechTarget) ?
            (MechHeat(mechTarget) ? '+' :
            Jellied(mechTarget) ? 'I' : ' ') :
            Starting(mechTarget) ? 's' : 'S';
        break;
    case 5:
        cRet = MechStatus(mechTarget) & NARC_ATTACHED || MechStatus2(mechTarget) & INARC_ATTACHED ? (MechTeam(mechTarget) ==
            MechTeam(mech) ? 'n' : 'N') :
            EmitECCM(mech) ? 'P' :
            EmitECM(mech) ? 'E' : Protected(mech) ? 'p' :
            Disturbed(mech) ? 'e' :
            ' ';
        break;
    }

    return cRet;
}

