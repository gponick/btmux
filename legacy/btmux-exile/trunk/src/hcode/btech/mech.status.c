#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "coolmenu.h"
#include "create.h"
#include "mycool.h"
#include "mech.partnames.h"
#include "p.mech.los.h"
#include "p.mech.utils.h"
#include "p.mech.scan.h"
#include "p.mech.status.h"
#include "p.mech.build.h"
#include "p.mech.update.h"
#include "p.mech.tech.commands.h"
#include "p.bsuit.h"
#include "p.template.h"
#include "p.mech.physical.h"
#include "p.btechstats.h"
#include "p.mech.tech.do.h"
#include "mech.sensor.h"

extern int num_def_weapons;
static int doweird = 0;
static char *weirdbuf;

void DisplayTarget(dbref player, MECH * mech)
{
    int arc;
    MECH *tempMech = NULL;
    char location[50];
    char buff[100], buff1[100];

    if (MechTarget(mech) != -1) {
	tempMech = getMech(MechTarget(mech));
	if (tempMech) {
	    if (InLineOfSight(mech, tempMech, MechX(tempMech),
		    MechY(tempMech), FaMechRange(mech, tempMech))) {
		sprintf(buff,
		    "\nTarget: %s\t   Range: %.1f hexes   Bearing: %d deg",
		    GetMechToMechID(mech, tempMech), FaMechRange(mech,
			tempMech), FindBearing(MechFX(mech), MechFY(mech),
			MechFX(tempMech), MechFY(tempMech)));
		notify(player, buff);
		arc =
		    InWeaponArc(mech, MechFX(tempMech), MechFY(tempMech));
		strcpy(buff, tprintf("Target in %s Weapons Arc",
			arc & TURRETARC ? "Turret" : GetArcID(mech, arc)));
		ArmorStringFromIndex(MechAim(mech), location,
		    MechType(tempMech), MechMove(tempMech));
		if (MechAim(mech) == NUM_SECTIONS ||
		    MechAimType(mech) != MechType(tempMech))
		    strcpy(location, "None");
		sprintf(buff1, "\t   Aimed Shot Location: %s", location);
		strcat(buff, buff1);
	    } else
		sprintf(buff, "\nTarget: NOT in line of sight!");
	}
	notify(player, buff);
    } else if (MechTargX(mech) != -1 && MechTargY(mech) != -1) {
	if (MechStatus(mech) & LOCK_BUILDING)
	    notify(player, tprintf("\nTarget: Building at %d %d",
		    MechTargX(mech), MechTargY(mech)));
	else if (MechStatus(mech) & LOCK_HEX)
	    notify(player, tprintf("\nTarget: Hex %d %d", MechTargX(mech),
		    MechTargY(mech)));
	else
	    notify(player, tprintf("\nTarget: %d %d", MechTargX(mech),
		    MechTargY(mech)));

    }

    if (MechSwarmTarget(mech) > 0) {
	tempMech = getMech(MechSwarmTarget(mech));
	if (!tempMech)
		return;
	if (InLineOfSight(mech, tempMech, MechX(tempMech), MechY(tempMech), FaMechRange(mech, tempMech)))
	    notify(player, tprintf("\nSwarmTarget: %s", GetMechToMechID(mech, tempMech)));
	else
	    notify(player, "\nSwarmTarget: NOT in line of sight!");
	}

    if (MechChargeTarget(mech) > 0 && mudconf.btech_newcharge) {
	tempMech = getMech(MechChargeTarget(mech));
	if (!tempMech)
	    return;
	if (InLineOfSight(mech, tempMech, MechX(tempMech), MechY(tempMech),
		FaMechRange(mech, tempMech))) {
	    notify(player, tprintf("\nChargeTarget: %s\t  ChargeTimer: %d",
		    GetMechToMechID(mech, tempMech),
		    MechChargeTimer(mech) / 2));
	} else {
	    notify(player,
		tprintf
		("\nChargeTarget: NOT in line of sight!\t Timer: %d",
	     MechChargeTimer(mech) / 2));
	}
    }
    if (MechPKiller(mech) || MechAutofall(mech) || !MechSLWarn(mech) || MechNoRadio(mech) || (is_aero(mech) && (MechCruise(mech) || MechOT(mech)))) {
   	 notify(player, tprintf("Safeties(%s)  Autofall(%s)  SLWarn(%s) LOSRadio(%s)", MechPKiller(mech) ? "%ch%crOFF%cn" : "%ch%cgON%cn",
            MechAutofall(mech) ? "%ch%crON%cn" : "%ch%cgOFF%cn", MechSLWarn(mech) ? "%ch%cgON%cn" : "%ch%crOFF%cn", MechNoRadio(mech) ? "%ch%crOFF%cn" : "%ch%cgON%cn"));
	 if (is_aero(mech)) {
	    char buff[30];
	    sprintf(buff, " Locked %.2fkph", MechJumpSpeed(mech));
	    notify(player, tprintf("OverThrust(%s) CruiseControl(%s)%s", MechOT(mech) ? "%ch%crON%cn" : "%ch%cgOFF%cn", MechCruise(mech) ? "%ch%cgON%cn" : "%ch%crOFF%cn", MechCruise(mech) ? buff : ""));
	    }
	}
    if (GotPilot(mech) && HasBoolAdvantage(MechPilot(mech), "maneuvering_ace")) {
		notify(player, tprintf("Turn Mode: %s", GetTurnMode(mech) ? "TIGHT" : "NORMAL"));
	}
}

void show_miscbrands(MECH * mech, dbref player)
{

/*   notify(player, tprintf("Radio: %s (%3d range)     Computer: %s (%d Scan / %d LRS / %d Tac)", brands[BOUNDED(1, MechRadio(mech), 5)+RADIO_INDEX].name, (int) MechRadioRange(mech), brands[BOUNDED(1, MechComputer(mech), 5)+COMPUTER_INDEX].name, (int) Mec
   hScanRange(mech), (int) MechLRSRange(mech), (int) MechTacRange(mech))); */
}

void PrintGenericStatus(dbref player, MECH * mech, int own, int usex)
{
    MECH *tempMech = NULL;
    MAP *map = FindObjectsData(mech->mapindex);
    char buff[100];
    char mech_name[100];
    char mech_ref[100];
    char move_type[50];
    float maxspeed = MMaxSpeed(mech);

    if (MechSpecials(mech) & HARDA_TECH)
	maxspeed = (maxspeed <= MP1 ? maxspeed : maxspeed - MP1);

    strncpy(mech_name, usex ? MechType_Name(mech) : silly_atr_get(mech->mynum, A_MECHNAME), MECHNAME_SIZE);
    strncpy(mech_ref, usex ? MechType_Ref(mech) : silly_atr_get(mech->mynum, A_MECHREF), MECHREF_SIZE);
    switch (MechType(mech)) {
    case CLASS_MW:
	notify(player, tprintf("MechWarrior: %-18.18s ID:[%s]",
		Name(player), MechIDS(mech, 0)));
	notify(player, tprintf("MaxSpeed: %3d", (int) maxspeed));
	break;
    case CLASS_BSUIT:
	sprintf(buff, "%s Name: %-18.18s  ID:[%s]   %s Reference: %s",
	    bsuit_name(mech), mech_name, MechIDS(mech, 0),
	    bsuit_name(mech), mech_ref);
	notify(player, buff);
	notify(player,
	    tprintf("MaxSpeed: %3d                  JumpRange: %d",
		(int) maxspeed, JumpSpeedMP(mech, map)));
	show_miscbrands(mech, player);
	if (MechPilot(mech) == -1)
	    notify(player, "Leader: NONE");
	else {
	    sprintf(buff, "%s Leader Name: %-16.16s %s Leader injury: %d",
		bsuit_name(mech), Name(MechPilot(mech)), bsuit_name(mech),
		MechPilotStatus(mech));
	    notify(player, buff);
	}
	Mech_ShowFlags(player, mech, 0, 0);
	if (!Jumping(mech) && !Fallen(mech) && Started(mech) &&
	    (MechChargeTarget(mech) != -1)) {
	    tempMech = getMech(MechChargeTarget(mech));
	    if (tempMech) {
		sprintf(buff, "CHARGING --> %s", GetMechToMechID(mech,
			tempMech));
		notify(player, buff);
	    }
	}
	if (Jumping(mech))
	    sprintf(buff, "JUMPING --> %3d,%3d", MechGoingX(mech),
		MechGoingY(mech));
	if (MechStatus(mech) & DFA_ATTACK) {
	    if (MechDFATarget(mech) != -1) {
		tempMech = getMech(MechDFATarget(mech));
		sprintf(buff,
		    "JUMPING --> %3d,%3d  Death From Above Target: %s",
		    MechGoingX(mech), MechGoingY(mech),
		    GetMechToMechID(mech, tempMech));
	    }
	}
	if (Jumping(mech))
	    notify(player, buff);
	break;
    case CLASS_MECH:
	sprintf(buff, "Mech Name: %-18.18s  ID:[%s]   Mech Reference: %s",
	    mech_name, MechIDS(mech, 0), mech_ref);
	notify(player, buff);
	notify(player,
	    tprintf("Tonnage:   %3d     MaxSpeed: %3d       JumpRange: %d",
		MechTons(mech), (int) maxspeed, JumpSpeedMP(mech,
		    map)));
	show_miscbrands(mech, player);
	if (MechPilot(mech) == -1)
	    notify(player, "Pilot: NONE");
	else {
	    sprintf(buff, "Pilot Name: %-28.28s Pilot Injury: %d",
		Name(MechPilot(mech)), MechPilotStatus(mech));
	    notify(player, buff);
	}
	Mech_ShowFlags(player, mech, 0, 0);
	if (!Jumping(mech) && !Fallen(mech) && Started(mech) &&
	    (MechChargeTarget(mech) != -1)) {
	    tempMech = getMech(MechChargeTarget(mech));
	    if (tempMech) {
		sprintf(buff, "CHARGING --> %s", GetMechToMechID(mech,
			tempMech));
		notify(player, buff);
	    }
	}

    if (Jumping(mech)) {
	sprintf(buff, "JUMPING --> %3d,%3d", MechGoingX(mech), MechGoingY(mech));
	if (MechStatus(mech) & DFA_ATTACK && MechDFATarget(mech) > 0) {
	    tempMech = getMech(MechDFATarget(mech));
	if (tempMech)
	    strcat(buff, tprintf(" Death From Above Target: %s", GetMechToMechID(mech, tempMech)));
        }
	notify(player, buff);
	} 
#if 0
	if (Jumping(mech))
	    sprintf(buff, "JUMPING --> %3d,%3d", MechGoingX(mech),
		MechGoingY(mech));
	if (MechStatus(mech) & DFA_ATTACK) {
	    if (MechDFATarget(mech) != -1) {
		tempMech = getMech(MechDFATarget(mech));
		sprintf(buff,
		    "JUMPING --> %3d,%3d  Death From Above Target: %s",
		    MechGoingX(mech), MechGoingY(mech),
		    GetMechToMechID(mech, tempMech));
	    }
	}
	if (Jumping(mech))
	    notify(player, buff);
#endif
	break;
    case CLASS_VEH_VTOL:
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
    case CLASS_AERO:
    case CLASS_DS:
    case CLASS_SPHEROID_DS:
	switch (MechMove(mech)) {
	case MOVE_TRACK:
	    strcpy(move_type, "Tracked");
	    break;
	case MOVE_WHEEL:
	    strcpy(move_type, "Wheeled");
	    break;
	case MOVE_HOVER:
	    strcpy(move_type, "Hover");
	    break;
	case MOVE_VTOL:
	    strcpy(move_type, "VTOL");
	    break;
	case MOVE_FLY:
	    strcpy(move_type, "Flight");
	    break;
	case MOVE_HULL:
	    strcpy(move_type, "Displacement Hull");
	    break;
	case MOVE_SUB:
	    strcpy(move_type, "Submarine");
	    break;
	case MOVE_FOIL:
	    strcpy(move_type, "Hydrofoil");
	    break;
	default:
	    strcpy(move_type, "Magic");
	    break;
	}
	if (MechMove(mech) != MOVE_NONE && MechMove(mech) != MOVE_FLY) {
	    sprintf(buff,
		"Vehicle Name: %-15.15s  ID:[%s]   Vehicle Reference: %s",
		mech_name, MechIDS(mech, 0), mech_ref);
	    notify(player, buff);
	    sprintf(buff,
		"Tonnage:  %3d   %s: %3d  JumpRange: %3d  Movement Type: %s",
		MechTons(mech),
		is_aero(mech) ? "Max thrust" : "FlankSpeed",
		(int) maxspeed, JumpSpeedMP(mech, map), move_type);
	    notify(player, buff);
	    show_miscbrands(mech, player);
	    if (MechPilot(mech) == -1)
		notify(player, "Pilot: NONE");
	    else {
		sprintf(buff, "Pilot Name: %-28.28s Pilot Injury: %d",
		    Name(MechPilot(mech)), MechPilotStatus(mech));
		notify(player, buff);
	    }
	} else if (MechMove(mech) == MOVE_FLY) {
            sprintf(buff,
                "Vehicle Name: %-15.15s  ID:[%s]   Vehicle Reference: %s",
                mech_name, MechIDS(mech, 0), mech_ref);
            notify(player, buff);
            sprintf(buff,
                "Tonnage:  %3d   %s: %3d  Movement Type: %s",
                MechTons(mech),
                is_aero(mech) ? "Max thrust" : "FlankSpeed",
                (int) maxspeed, move_type);
            notify(player, buff);
            show_miscbrands(mech, player);
            if (MechPilot(mech) == -1)
                notify(player, "Pilot: NONE");
            else {
                sprintf(buff, "Pilot Name: %-28.28s Pilot Injury: %d",
                    Name(MechPilot(mech)), MechPilotStatus(mech));
                notify(player, buff);
            }
	} else {
	    sprintf(buff, "Name: %-15.15s  ID:[%s]   Reference: %s",
		mech_name, MechIDS(mech, 0), mech_ref);
	    notify(player, buff);
	}
	if (MechType(mech) != CLASS_VEH_VTOL && !is_aero(mech))
	    if (GetSectInt(mech, TURRET) &&
		MechCritStatus(mech) & TURRET_LOCKED) notify(player,
		    "     TURRET LOCKED");
        if (MechType(mech) != CLASS_VEH_VTOL && !is_aero(mech))
            if (GetSectInt(mech, TURRET) &&
                MechCritStatus(mech) & TURRET_JAMMED) notify(player,
                    "     TURRET JAMMED");
	if (FlyingT(mech) && Landed(mech))
	    notify(player, "LANDED");
	Mech_ShowFlags(player, mech, 0, 0);
    }
}

void PrintShortInfo(dbref player, MECH * mech)
{
#if 0 /* Pre-HUDINFO port code */
    char buff[100];

    sprintf(buff,
	"LOC: %3d,%3d,%3d  HD: %3d/%3d  SP: %3.1f/%3.1f  HT: %3d/%3d  ST:%c%c%c%c%c",
	MechX(mech), MechY(mech), MechZ(mech), MechFacing(mech),
	MechDesiredFacing(mech), MechSpeed(mech), MechDesiredSpeed(mech),
	(int) (10. * MechPlusHeat(mech)),
	(int) (10. * MechActiveNumsinks(mech)),
	Fallen(mech) ? 'F' : Standing(mech) ? 'f' : ' ',
	Destroyed(mech) ? 'D' : ' ', Jumping(mech) ? 'J' : (MechCritStatus(mech) & DUG_IN) ? 'd' : (MechStatus(mech) & JELLIED) ? 'I' :
	((MechStatus2(mech) & BEAGLE_ACTIVE) && Started(mech)) ? 'A' : ' ',
	Started(mech) ? ' ' : Starting(mech) ? 's' : 'S', (MechStatus(mech) & ECM_DISTURBANCE) ? 'e' : ((MechStatus(mech) & ECM_ACTIVE) && Started(mech)) ? 'E' :
	(MechStatus2(mech) & ECM_PROTECTED) ? 'p' : ((MechStatus2(mech) & ECCM_ACTIVE) && Started(mech)) ? 'P' : ' ' );
    notify(player, buff);
    DisplayTarget(player, mech);
#else
    char buff[100];
    char typespecific[50];

    switch (MechType(mech)) {
    case CLASS_VTOL:
        sprintf(typespecific, " VSPD: %3.1f ", MechVerticalSpeed(mech));
        break;
    case CLASS_MECH:
        sprintf(typespecific, " HT: %3d/%3d ",
            (int) (10. * MechPlusHeat(mech)),
            (int) (10. * MechActiveNumsinks(mech)));
        break;
    case CLASS_AERO:
    case CLASS_DS:
    case CLASS_SPHEROID_DS:
        sprintf(typespecific, " VSPD: %3.1f  ANG: %2d  HT: %3d/%3d ",
            MechVerticalSpeed(mech), MechDesiredAngle(mech),
            (int) (10 * MechPlusHeat(mech)),
            (int) (10 * MechActiveNumsinks(mech)));
        break;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
        if (GetSectOInt(mech, TURRET)) {
            sprintf(typespecific, " TUR: %3d ",
                AcceptableDegree(MechTurretFacing(mech) +
                    MechFacing(mech)));
            break;
        }
        /* FALLTHROUGH */
    default:
        typespecific[0] = '\0';
        break;
    }

    snprintf(buff, 100, 
        "LOC: %3d,%3d,%3d  HD: %3d/%3d  SP: %3.1f/%3.1f  HT: %3d/%3d %s ST:%c%c%c%c%c",
        MechX(mech), MechY(mech), MechZ(mech), MechFacing(mech),
        MechDesiredFacing(mech), MechSpeed(mech), MechDesiredSpeed(mech),
        (int) (10. * MechPlusHeat(mech)),
        (int) (10. * MechActiveNumsinks(mech)),
	typespecific,
        Fallen(mech) ? 'F' : Standing(mech) ? 'f' : ' ',
        Destroyed(mech) ? 'D' : ' ', Jumping(mech) ? 'J' : (MechCritStatus(mech) & DUG_IN) ? 'd' : (MechStatus(mech) & JELLIED) ? 'I' :
        ((MechStatus2(mech) & BEAGLE_ACTIVE) && Started(mech)) ? 'A' : ' ',
        Started(mech) ? ' ' : Starting(mech) ? 's' : 'S', (MechStatus(mech) & ECM_DISTURBANCE) ? 'e' : ((MechStatus(mech) & ECM_ACTIVE) && Started(mech))
? 'E' :
        (MechStatus2(mech) & ECM_PROTECTED) ? 'p' : ((MechStatus2(mech) & ECCM_ACTIVE) && Started(mech)) ? 'P' : ' ' );

    buff[99] = '\0';
    notify(player, buff);
    DisplayTarget(player, mech);


#endif
}


#define HEAT_LEVEL_LGREEN 0
#define HEAT_LEVEL_BGREEN 7
#define HEAT_LEVEL_LYELLOW 13
#define HEAT_LEVEL_BYELLOW 16
#define HEAT_LEVEL_LRED 18
#define HEAT_LEVEL_BRED 24
#define HEAT_LEVEL_TOP 40

#define HEAT_LEVEL_NONE 27

static char *MakeHeatScaleInfo(MECH * mech, char *fillchar)
{
    static char heatstr[128];
    int counter = 0, heat = MechPlusHeat(mech), minheat =
	MechMinusHeat(mech), start = 0;
    char state = 1;

    memset(heatstr, 0, sizeof(char) * 128);

    strcat(heatstr, "%cx%ch");

    if (minheat > HEAT_LEVEL_NONE)
	start = minheat - HEAT_LEVEL_NONE;

    if (heat <= start) {
	heat = 0;
	state = 0;
    } else
	heat -= start;

    if (start)
	strcat(heatstr, "<%cx%ch");
    else
	strcat(heatstr, " %cx%ch");

    for (counter = start; counter < minheat; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    if (state)
	state++;

    strcat(heatstr, "%cg%ch|%c%cg");
    for (; counter < minheat + HEAT_LEVEL_BGREEN; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    if (state)
	state++;

    strcat(heatstr, "%ch");
    for (; counter < minheat + HEAT_LEVEL_LYELLOW; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    if (state)
	state++;

    strcat(heatstr, "%c%cy%ch|%c%cy");
    for (; counter < minheat + HEAT_LEVEL_BYELLOW; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    if (state)
	state++;

    strcat(heatstr, "%ch");
    for (; counter < minheat + HEAT_LEVEL_LRED; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    if (state)
	state++;

    strcat(heatstr, "%c%cr%ch|%c%cr");
    for (; counter < minheat + HEAT_LEVEL_BRED; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    if (state)
	state++;

    strcat(heatstr, "%ch");
    for (; counter < minheat + HEAT_LEVEL_TOP; counter++) {
	strncat(heatstr, &fillchar[(short) state], 1);
	if (heat && !--heat)
	    state = 0;
    }
    strcat(heatstr, "%cw%ch|%c");
    return heatstr;
}

void PrintInfoStatus(dbref player, MECH * mech, int own)
{
    char buff[256];
    char heatstr[9] = ".:::::::";
    MECH *tempMech;
    int f;
    char *tmpstr;

    switch (MechType(mech)) {
    case CLASS_MECH:
	sprintf(buff,
	    "X, Y, Z:%3d,%3d,%3d  Excess Heat:  %3d deg C.  Heat Production:  %3d deg C.",
	    MechX(mech), MechY(mech), MechZ(mech),
	    (int) (10. * MechHeat(mech)),
	    (int) (10. * MechPlusHeat(mech)));
	notify(player, buff);
	sprintf(buff,
	    "Speed:      %%ch%%cg%3d%%cn KPH  Heading:      %%ch%%cg%3d%%cn deg     Heat Sinks:       %3d",
	    (int) (MechSpeed(mech)), MechFacing(mech), MechActiveNumsinks(mech));
	notify(player, buff);
	sprintf(buff,
	    "Des. Speed: %3d KPH  Des. Heading: %3d deg     Heat Dissipation: %3d deg C.",
	    (int) MechDesiredSpeed(mech), MechDesiredFacing(mech),
	    (int) (10. * MechMinusHeat(mech)));
	notify(player, buff);
	tmpstr = silly_atr_get(player, A_HEATCHARS);
	if (!tmpstr || !strlen(tmpstr) ||
	    sscanf(tmpstr, "[%c%c%c%c%c%c%c%c]", &heatstr[0], &heatstr[1],
		&heatstr[2], &heatstr[3], &heatstr[4], &heatstr[5],
		&heatstr[6], &heatstr[7]) == 8) {
	    sprintf(buff, "Temp:%s", MakeHeatScaleInfo(mech, heatstr));
	    notify(player, buff);
	}
	if (MechLateral(mech))
	    notify(player, tprintf("You are moving laterally %s",
		    LateralDesc(mech)));
	break;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
    case CLASS_VEH_VTOL:
    case CLASS_AERO:
    case CLASS_DS:
    case CLASS_SPHEROID_DS:
	sprintf(buff,
	    "X, Y, Z:%3d,%3d,%3d  Heat Sinks:          %3d       %s",
	    MechX(mech), MechY(mech), MechZ(mech), MechActiveNumsinks(mech),
	    is_aero(mech) ? tprintf("Angle:  %%ch%%cg%3d%%cn (%s%3d%s)",
		MechDesiredAngle(mech), (MechAngle(mech) == MechDesiredAngle(mech) ? "%ch%cg" : "%ch%cr"),
		MechAngle(mech), "%cn"
	    ) : "");
	notify(player, buff);
	if (FlyingT(mech) || WaterBeast(mech)) {
	    sprintf(buff,
		"Speed:      %%ch%%cg%3d%%cn KPH  Vertical Speed:      %%ch%%cg%3d%%cn KPH   Thrust: %%ch%%cg%3d%%cn (%s%3d%%cn)",
		(int) (MechSpeed(mech)), (int) (MechVerticalSpeed(mech)),
		(int) (MechDesiredThrust(mech)), MechDesiredThrust(mech) == MechThrust(mech) ? "%ch%cg" : "%ch%cr" , (int) MechThrust(mech));
	    notify(player, buff);
	    sprintf(buff, "Velocity: %%ch%%cg%5d%%cn KPH", (int) MechVelocity(mech));
	    notify(player, buff);
	    f = MAX(0, AeroFuel(mech));
	    if (WaterBeast(mech))
		sprintf(buff, "Des. Speed: %3d KPH  Des. Heading: %3d deg",
		    (int) MechDesiredSpeed(mech), MechDesiredFacing(mech));
	    else
		sprintf(buff,
		    "Heading:    %%ch%%cg%3d%%cn deg  Des. Heading:        %3d deg   Fuel: %d (%.2f %%)",
		    MechFacing(mech), MechDesiredFacing(mech), f,
		    100.0 * f / AeroFuelOrig(mech));
	    notify(player, buff);
	} else if (MechMove(mech) != MOVE_NONE) {
	    sprintf(buff,
		"Speed:      %%ch%%cg%3d%%cn KPH  Heading:      %%ch%%cg%3d%%cn deg",
		(int) (MechSpeed(mech)), MechFacing(mech));
	    notify(player, buff);
	    sprintf(buff, "Des. Speed: %3d KPH  Des. Heading: %3d deg",
		(int) MechDesiredSpeed(mech), MechDesiredFacing(mech));
	    notify(player, buff);

	}
	if (IsDS(mech) || is_aero(mech)) {
	    tmpstr = silly_atr_get(player, A_HEATCHARS);
	    if (!tmpstr || !strlen(tmpstr) || sscanf(tmpstr, "[%c%c%c%c%c%c%c%c]", &heatstr[0], &heatstr[1],
                &heatstr[2], &heatstr[3], &heatstr[4], &heatstr[5],
                &heatstr[6], &heatstr[7]) == 8) {
                sprintf(buff, "Temp:%s", MakeHeatScaleInfo(mech, heatstr));
                notify(player, buff);
            }
	}
	ShowTurretFacing(player, 0, mech);
	break;
    case CLASS_MW:
    case CLASS_BSUIT:
	sprintf(buff,
	    "X, Y, Z:%3d,%3d,%3d  Speed:      %%ch%%cg%3d%%cn KPH  Heading:      %%ch%%cg%3d%%cn deg",
	    MechX(mech), MechY(mech), MechZ(mech), (int) (MechSpeed(mech)),
	    MechFacing(mech));
	notify(player, buff);
	sprintf(buff,
	    "                     Des. Speed: %3d KPH  Des. Heading: %3d deg",
	    (int) MechDesiredSpeed(mech), MechDesiredFacing(mech));
	notify(player, buff);
	break;
    }
    DisplayTarget(player, mech);
    if (MechCarrying(mech) > 0)
	if ((tempMech = getMech(MechCarrying(mech))))
	    notify(player, tprintf("Towing %s.", GetMechToMechID(mech,
			tempMech)));
}

/* Status commands! */
void mech_weapstatus(dbref player, void *data, char *buffer)
{
MECH *mech = (MECH *) data;
int num, sect, crit, count = 0, i, weap_crits, weapindx, dmg, func;

cch(MECH_USUALSM);
num = atoi(buffer);
DOCHECK(num < 0 || FindWeaponNumberOnMech(mech, num, &sect, &crit) == -1, "Invalid weapon number.");
weapindx = Weapon2I(GetPartType(mech, sect, crit));
weap_crits = GetWeaponCrits(mech, weapindx);
dmg = GetPartDamage(mech, sect, crit);
func = WeaponIsNonfunctional(mech, sect, crit, -1);

notify(player, tprintf("%%ch%%cg==== %%c%%ch%%cy[ Weapon #%d - %s ]%%c%%ch%%cg ====%%cn", num, MechWeapons[weapindx].name));
if (func >= 0)
    notify(player, tprintf("%%ch%%cbWeapon is %s",
	func == 0 ? "%ch%cgOperational%cn" :
	func == 1 ? "%ch%cyDisabled%cn" :
	"%ch%crDestroyed%cn"));
else
    notify(player, tprintf("%%ch%%cbWeapon has taken %%ch%%cy%d%%ch%%cb crits%%cn", abs(func)));

if (dmg != 0) {
    if (dmg & MTX_TOHIT)
	notify(player, tprintf("%%ch%%cr+%d To-Hit%%cn", MTX_TOHIT_MOD(dmg)));
    if (dmg & MTX_TODAM)
	notify(player, tprintf("%%ch%%cr-%d Damage%%cn", MTX_TODAM_MOD(dmg)));
    if (dmg & MTX_TOHIT_RANGE)
	notify(player, tprintf("%%ch%%cr-%d To-Hit at Medium and Long range%%cn", MTX_TOHIT_RANGE_MOD(dmg)));
    if (dmg & MTX_HEAT)
	notify(player, tprintf("%%ch%%cr+%d Heat%%cn", MTX_HEAT_MOD(dmg)));
    if (dmg & MTX_JAM)
	notify(player, tprintf("%%ch%%crBTH %d per fire or jam%%cn", MTX_JAM_MOD(dmg)));
    if (dmg & MTX_BOOM)
	notify(player, tprintf("%%ch%%crBTH %d per fire or explode%%cn", MTX_BOOM_MOD(dmg)));
    if (dmg & MTX_MODELOCK)
	notify(player, "%ch%crCannot change ammo modes%c");
}
for (i = crit; i < crit + weap_crits; i++) {
    notify(player, tprintf("CritLoc #%-2d %-20s%s%s", i + 1, &MechWeapons[weapindx].name[3],
	PartIsDisabled(mech, sect, i) ? " (Disabled)" : "",
	PartIsDestroyed(mech, sect, i) ? " (Destroyed)" : ""));
    }
}

void mech_status(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int doweap = 0, doinfo = 0, doarmor = 0, doshort = 0, doheat = 0, loop, usex = 0;
    int i;
    char buf[LBUF_SIZE];

    doweird = 0;
    cch(MECH_USUALSM);
    if (!buffer || !strlen(buffer))
	doweap = doinfo = doarmor = 1;
    else {
	for (loop = 0; buffer[loop]; loop++) {
	    switch (toupper(buffer[loop])) {
	    case 'R':
		doweap = doinfo = doarmor = usex = 1;
		break;
	    case 'A':
		if (toupper(buffer[loop + 1]) == 'R')
		    while (buffer[loop + 1] && buffer[loop + 1] != ' ')
			loop++;
		doarmor = 1;
		break;
	    case 'I':
		doinfo = 1;
		if (toupper(buffer[loop + 1]) == 'N')
		    while (buffer[loop + 1] && buffer[loop + 1] != ' ')
			loop++;
		break;
	    case 'W':
		doweap = 1;
		if (toupper(buffer[loop + 1]) == 'E')
		    while (buffer[loop + 1] && buffer[loop + 1] != ' ')
			loop++;
		break;
	    case 'N':
		doweird = 1;
		break;
	    case 'S':
		doshort = 1;
		break;
	    case 'H':
		doheat = 1;
		break;
	    }
	}
    }
    if (doshort) {
	PrintShortInfo(player, mech);
	return;
    }
    if (doweird) {
	sprintf(buf, "%s %s %d %d/%d/%d %d ", MechType_Ref(mech),
	    MechType_Name(mech), MechTons(mech),
	    (int) (MechMaxSpeed(mech) / MP1) * 2 / 3,
	    (int) (MechMaxSpeed(mech) / MP1),
	    (int) (MechJumpSpeed(mech) / MP1), MechActiveNumsinks(mech));
	weirdbuf = buf;
    } else if (!doheat || (doarmor | doinfo | doweap))
	PrintGenericStatus(player, mech, 1, usex);
    if (doarmor) {
	if (!doweird) {
	    PrintArmorStatus(player, mech, 1);
	    notify(player, " ");
	} else {
	    for (i = 0; i < NUM_SECTIONS; i++)
		if (GetSectOArmor(mech, i)) {
		    if (GetSectORArmor(mech, i))
			sprintf(buf + strlen(buf), "%d|%d|%d ",
			    GetSectOArmor(mech, i), GetSectOInt(mech, i),
			    GetSectORArmor(mech, i));
		    else
			sprintf(buf + strlen(buf), "%d|%d ",
			    GetSectOArmor(mech, i), GetSectOInt(mech, i));
		}
	}
    }
    if (doinfo && !doweird) {
	PrintInfoStatus(player, mech, 1);
	notify(player, " ");
    }
    if (doheat && !doinfo && (MechType(mech) == CLASS_MECH || IsDS(mech) || is_aero(mech))) {
	char *tmpstr, heatstr[9] = ".:::::::";

	tmpstr = silly_atr_get(player, A_HEATCHARS);
	if (!tmpstr || !strlen(tmpstr) ||
	    sscanf(tmpstr, "[%c%c%c%c%c%c%c%c]", &heatstr[0], &heatstr[1],
		&heatstr[2], &heatstr[3], &heatstr[4], &heatstr[5],
		&heatstr[6], &heatstr[7]) == 8) {
	    sprintf(buf, "Temp:%s", MakeHeatScaleInfo(mech, heatstr));
	    notify(player, buf);
	}
    }

    if (doweap)
	PrintWeaponStatus(mech, player);
    if (doweird)
	notify(player, buf);
}

void mech_critstatus(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[1];
    int index;

    cch(MECH_USUALSM);
    if (!CheckData(player, mech))
	return;
    DOCHECK(MechType(mech) == CLASS_MW, "Huh?");
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"You must specify a section to list the criticals for!");
    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    DOCHECK(index == -1, "Invalid section!");
    DOCHECK(!GetSectOInt(mech, index), "Invalid section!");
    CriticalStatus(player, mech, index);
}

static int wspec_weaps[MAX_WEAPONS_PER_MECH];
static int wspec_modes[MAX_WEAPONS_PER_MECH];
static int wspec_weapcount;
static int wspec_isaero;

char *part_name(int type)
{
    char *c;
    static char buffer[SBUF_SIZE];

    if (type == EMPTY)
	return "Empty";
    c = get_parts_long_name(type);
    if (!c)
	return NULL;
    strcpy(buffer, c);
    if (!strcmp(c, "LifeSupport"))
	strcpy(buffer, "Life Support");
    else if (!strcmp(c, "TripleStrengthMyomer"))
	strcpy(buffer, "Triple Strength Myomer");
    else
	strcpy(buffer, c);
    if ((c = strstr(buffer, "Actuator")))
	if (c != buffer)
	    strcpy(c, " Actuator");
    while ((c = strchr(buffer, '_')))
	*c = ' ';
    while ((c = strchr(buffer, '.')))
	*c = ' ';
    return buffer;
}

char *part_name_long(int type)
{
    char *c;
    static char buffer[SBUF_SIZE];

    if (type == EMPTY)
	return "Empty";
    c = get_parts_vlong_name(type);
    if (!c)
	return NULL;
    strcpy(buffer, c);
    if (!strcmp(c, "LifeSupport"))
	strcpy(buffer, "Life Support");
    else if (!strcmp(c, "TripleStrengthMyomer"))
	strcpy(buffer, "Triple Strength Myomer");
    else
	strcpy(buffer, c);
    if ((c = strstr(buffer, "Actuator")))
	if (c != buffer)
	    strcpy(c, " Actuator");
    while ((c = strchr(buffer, '_')))
	*c = ' ';
    while ((c = strchr(buffer, '.')))
	*c = ' ';
    return buffer;
}

char *pos_part_name(MECH * mech, int index, int loop)
{
    int t;
    char *c;

    if (index < 0 || index >= NUM_SECTIONS || loop < 0 ||
	loop >= NUM_CRITICALS) {
	SendError(tprintf("INVALID: For mech #%d, %d/%d was requested.",
		mech->mynum, index, loop));
	return "--?LocationBug?--";
    }
    t = GetPartType(mech, index, loop);
    if (t == Special(HAND_OR_FOOT_ACTUATOR)) {
	if (index == LLEG || index == RLEG || MechMove(mech) == MOVE_QUAD)
	    return "Foot Actuator";
	return "Hand Actuator";
    }
    if (t == Special(SHOULDER_OR_HIP)) {
	if (index == LLEG || index == RLEG || MechMove(mech) == MOVE_QUAD)
	    return "Hip";
	return "Shoulder";
    }
    if (!(c = part_name(t)))
	return "--?ErrorInTemplate?--";
    return c;

}
static char *wspec_fun(int i)
{
    static char buf[MBUF_SIZE];
    int j, m;
    extern char *crit_modes[];

    buf[0] = 0;
    if (!i)
	if (mudconf.btech_erange)
	    sprintf(buf, WSDUMP_MASKS_ER);
	else if (wspec_isaero)
	    sprintf(buf, WSDUMP_MASKS_AERO);
	else
	    sprintf(buf, WSDUMP_MASKS_NOER);
    else {
	i--;
	j = wspec_weaps[i];
	m = (wspec_modes[i] & (VARIABLE_AMMO));

	if (mudconf.btech_erange)
	    sprintf(buf, WSDUMP_MASK_ER, (m ? tprintf("%s (%s)", MechWeapons[j].name, BuildBitString(crit_modes, m))
		: MechWeapons[j].name),
		GunStat(j, m, GUN_HEAT), GunStat(j, m, GUN_DAMAGE),
		GunStat(j, m, GUN_MINRANGE), GunStat(j, m, GUN_SHORTRANGE),
		GunStat(j, m, GUN_MEDRANGE), GunStat(j, m, GUN_LONGRANGE),
		EGunRange(j, m), GunStat(j, m, GUN_VRT));
	else if (wspec_isaero)
	    sprintf(buf, WSDUMP_MASK_AERO, (m ? tprintf("%s (%s)", MechWeapons[j].name, BuildBitString(crit_modes, m))
		: MechWeapons[j].name),
		GunStat(j, m, GUN_HEAT), GunStat(j, m, GUN_DAMAGE),
		GunStat(j, m, GUN_AERORANGE), GunStat(j, m, GUN_VRT));
	else
	    sprintf(buf, WSDUMP_MASK_NOER, (m ? tprintf("%s (%s)", MechWeapons[j].name, BuildBitString(crit_modes, m))
		: MechWeapons[j].name),
		GunStat(j, m, GUN_HEAT), GunStat(j, m, GUN_DAMAGE),
		GunStat(j, m, GUN_MINRANGE), GunStat(j, m, GUN_SHORTRANGE),
		GunStat(j, m, GUN_MEDRANGE), GunStat(j, m, GUN_LONGRANGE),
		GunStat(j, m, GUN_VRT));
    }
    return buf;
}

void mech_weaponspecs(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int loop;
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    unsigned char weaparray[MAX_WEAPS_SECTION];
/*   unsigned char weaps[8 * MAX_WEAPS_SECTION]; */
    int num_weaps;
    int index;
    int duplicate, ii;
    coolmenu *c;

    wspec_weapcount = 0;
    wspec_isaero = (is_aero(mech) && !Landed(mech));
    if (!CheckData(player, mech))
	return;
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	num_weaps = FindWeapons(mech, loop, weaparray, weapdata, critical);
	for (index = 0; index < num_weaps; index++) {
	    duplicate = 0;
	    for (ii = 0; ii < wspec_weapcount; ii++) {
		if (weaparray[index] != wspec_weaps[ii])
		    continue;
		if ((GetPartMode(mech, loop, critical[index]) & (VARIABLE_AMMO)) != wspec_modes[ii])
		    continue;
		duplicate = 1;
		}
	    if (!duplicate && wspec_weapcount < MAX_WEAPONS_PER_MECH) {
		wspec_weaps[wspec_weapcount++] = weaparray[index];
	        wspec_modes[wspec_weapcount - 1] = (GetPartMode(mech, loop, critical[index]) & (VARIABLE_AMMO));
	    }
	}
    }
    DOCHECK(!wspec_weapcount, "You have no weapons!");
    if (strcmp(MechType_Name(mech), MechType_Ref(mech)))
	c =
	    SelCol_FunStringMenuK(1,
	    tprintf("Weapons statistics for %s: %s", MechType_Name(mech),
		MechType_Ref(mech)), wspec_fun, wspec_weapcount + 1);
    else
	c =
	    SelCol_FunStringMenuK(1, tprintf("Weapons statistics for %s",
		MechType_Ref(mech)), wspec_fun, wspec_weapcount + 1);
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}

char *critstatus_func(MECH * mech, char *arg)
{
    static char buffer[MBUF_SIZE];
    int index, i, max_crits;
    int type, data, mode;

    if (!arg || !*arg)
	return "#-1 INVALID SECTION";

    index = ArmorSectionFromString(MechType(mech), MechMove(mech), arg);
    if (index == -1 || !GetSectOInt(mech, index))
	return "#-1 INVALID SECTION";

    buffer[0] = '\0';
    max_crits = CritsInLoc(mech, index);
    for (i = 0; i < max_crits; i++) {
	if (buffer[0])
	    sprintf(buffer, "%s,", buffer);
	sprintf(buffer, "%s%d|", buffer, i + 1);
	type = GetPartType(mech, index, i);
	data = GetPartData(mech, index, i);
	mode = GetPartMode(mech, index, i);
	if (IsAmmo(type)) {
	    strcat(buffer, &MechWeapons[Ammo2WeaponI(type)].name[3]);
	    strcat(buffer, GetAmmoDesc_Model_Mode(Ammo2WeaponI(type),
		    GetPartMode(mech, index, i)));
	    strcat(buffer, " Ammo");
	    sprintf(buffer, "%s|%d", buffer, PartIsNonfunctional(mech,
		    index, i) ? -1 : data);
	} else {
	    if (IsWeapon(type) && (mode & OS_MODE) && !(MechWeapons[Weapon2I(type)].special & OS_WEAP))
		strcat(buffer, "OS ");
	    strcat(buffer, pos_part_name(mech, index, i));
	    if (IsWeapon(type) && (mode & OS_MODE) && (mode & OS_USED))
		strcat(buffer, " (Empty)");
	    sprintf(buffer, "%s|-1", buffer);
	}
	sprintf(buffer, "%s|%d", buffer,
		(PartIsNonfunctional(mech, index, i) && type != EMPTY && (!IsCrap(type) || SectIsDestroyed(mech, index))) ? -1 : 0);
	sprintf(buffer, "%s|%d", buffer,
	    IsWeapon(type) ? 1 : IsAmmo(type) ? 2 : IsActuator(type) ? 3 :
	    IsCargo(type) ? 4 : (IsCrap(type) || type == EMPTY) ? 5 : 0);
    }
    return buffer;
}

char *armorstatus_func(MECH * mech, char *arg)
{
    static char buffer[MBUF_SIZE];
    int index;

    if (!arg || !*arg)
	return "#-1 INVALID SECTION";

    index = ArmorSectionFromString(MechType(mech), MechMove(mech), arg);
    if (index == -1 || !GetSectOInt(mech, index))
	return "#-1 INVALID SECTION";

    buffer[0] = '\0';
    sprintf(buffer, "%d/%d|%d/%d|%d/%d", GetSectArmor(mech, index),
	GetSectOArmor(mech, index), GetSectInt(mech, index),
	GetSectOInt(mech, index), GetSectRArmor(mech, index),
	GetSectORArmor(mech, index));
    return buffer;
}

char *critslot_func(MECH * mech, char *buf_section, char *buf_critnum, char *buf_flag)
{
    int index, crit, flag, type, mode, data;
    static char buffer[MBUF_SIZE];

    index = ArmorSectionFromString(MechType(mech), MechMove(mech), buf_section);
    if (index == -1)
	return "#-1 INVALID SECTION";
    if (!GetSectOInt(mech, index))
	return "#-1 INVALID SECTION";
    crit = atoi(buf_critnum);
    if (crit < 1 || crit > CritsInLoc(mech, index))
	return "#-1 INVALID CRITICAL";
    crit--;
    if (!buf_flag)
	flag = 0;
    else if (strcasecmp(buf_flag, "NAME") == 0)
	flag = 0;
    else if (strcasecmp(buf_flag, "STATUS") == 0)
	flag = 1;
    else if (strcasecmp(buf_flag, "DATA") == 0)
	flag = 2;
    else if (strcasecmp(buf_flag, "MAXAMMO") == 0)
	flag = 3;
    else if (strcasecmp(buf_flag, "AMMOTYPE") == 0)
	flag = 4;
    else if (strcasecmp(buf_flag, "MODE") == 0)
	flag = 5;
    else
	flag = 0;

    type = GetPartType(mech, index, crit);
    mode = GetPartMode(mech, index, crit);
    data = GetPartData(mech, index, crit);

    if (flag == 1) {
	if (PartIsDisabled(mech, index, crit))
	    return "Disabled";
	if (PartIsDestroyed(mech, index, crit))
	    return "Destroyed";
	return "Operational";
    }  else if (flag == 2) {
        snprintf(buffer, MBUF_SIZE, "%d", data);
        return buffer;
    } else if (flag == 3) {
	if (!IsAmmo(type))
	    return "#-1 NOT AMMO";
	snprintf(buffer, MBUF_SIZE, "%d", FullAmmo(mech, index, crit));
	return buffer;
    } else if (flag == 4) {
	if (!IsAmmo(type))
	    return "#-1 NOT AMMO";
	type = FindAmmoType(mech, index, crit);
    } else if (flag == 5) {
	int weapindex;
	if (!(IsAmmo(type) || IsWeapon(type)))
	    return "#-1 NOT AMMO OR WEAPON";
	if (IsWeapon(type))
	    weapindex = Weapon2I(type);
	else
	    weapindex = Ammo2WeaponI(type); 
	snprintf(buffer, MBUF_SIZE, "%c", GetWeaponModeLetter_Model_Mode(weapindex, mode));
	return buffer;
    }

    if (type == EMPTY || IsCrap(type))
	return "Empty";
    if (flag == 0)
	type = alias_part(mech, type, index);
    snprintf(buffer, MBUF_SIZE, "%s", get_parts_vlong_name(type));
    return buffer;
}

#if 0
void CriticalStatus(dbref player, MECH * mech, int index)
{
    int loop, i;
    char buffer[MBUF_SIZE];
    int type, data, mode;
    int max_crits = CritsInLoc(mech, index);
    char **foo;
    int count = 0;
    coolmenu *cm;

    Create(foo, char *, NUM_CRITICALS + 1);

    for (i = 0; i < max_crits; i++) {
/*	loop = ((i % 2) ? (max_crits / 2) : 0) + i / 2; */
	loop = i;
	sprintf(buffer, "%2d ", loop + 1);
	type = GetPartType(mech, index, loop);
	data = GetPartData(mech, index, loop);
	mode = GetPartMode(mech, index, loop);

	if (IsAmmo(type)) {
	    char trash[50];

	    strcat(buffer, &MechWeapons[Ammo2WeaponI(type)].name[3]);
	    strcat(buffer, GetAmmoDesc_Model_Mode(Ammo2WeaponI(type),
		    GetPartMode(mech, index, loop)));
	    strcat(buffer, " Ammo");

	    if (!PartIsNonfunctional(mech, index, loop)) {
		sprintf(trash, " (%d)", data);
		strcat(buffer, trash);
	    }

	} else {
	    if (IsWeapon(type) && (mode & OS_MODE) && !(MechWeapons[Weapon2I(type)].special & OS_WEAP))
		strcat(buffer, "OS ");
	    strcat(buffer, pos_part_name(mech, index, loop));
	    if (IsWeapon(type) && (mode & OS_MODE) && (mode & OS_USED))
		strcat(buffer, " (Empty)");
	}

	if (PartIsDestroyed(mech, index, loop) && type != EMPTY &&
	    (!IsCrap(type) || SectIsDestroyed(mech, index)))
	    strcat(buffer, " (Destroyed)");
	else if (PartIsDisabled(mech, index, loop) && type != EMPTY)
	    strcat(buffer, " (Disabled)");
	if ((MechSpecials2(mech) & OMNI_TECH) && (mode & (HPOINTS)))
	    strcat(buffer, tprintf(" (%s Hardpoint)", (mode & ENERGY_HPOINT) ? "Energy" : (mode & BALL_HPOINT) ? "Ballistic" : (mode & MISSILE_HPOINT) ?
"Missile" ? (mode & SPECIAL_HPOINT) ? "Special" : (mode & GAUSS_HPOINT) ? "Gauss" : "Error"));
	foo[count++] = strdup(buffer);
    }

    ArmorStringFromIndex(index, buffer, MechType(mech), MechMove(mech));
    strcat(buffer, " Criticals");
    cm = SelCol_StringMenu(1, buffer, foo);
    ShowCoolMenu(player, cm);
    KillCoolMenu(cm);
    KillText(foo);
}
#else
void CriticalStatus(dbref player, MECH * mech, int index)
{
    int loop, i;
    char buffer[MBUF_SIZE];
    int type, data, mode;
    int max_crits = CritsInLoc(mech, index);
    char **foo;
    char trash[50];
    int count = 0;
    coolmenu *cm;

    Create(foo, char *, NUM_CRITICALS + 1);

    for (i = 0; i < max_crits; i++) {
/*      loop = ((i % 2) ? (max_crits / 2) : 0) + i / 2; */
        loop = i;
        sprintf(buffer, "%2d ", loop + 1);
        type = GetPartType(mech, index, loop);
        data = GetPartData(mech, index, loop);
        mode = GetPartMode(mech, index, loop);

        if (IsAmmo(type)) {
            strcat(buffer, &MechWeapons[Ammo2WeaponI(type)].name[3]);
            strcat(buffer, GetAmmoDesc_Model_Mode(Ammo2WeaponI(type),
                    GetPartMode(mech, index, loop)));
            strcat(buffer, " Ammo");

            if (!PartIsNonfunctional(mech, index, loop)) {
                sprintf(trash, " (%d)", data);
                strcat(buffer, trash);
        }

        } else {
            if (IsWeapon(type) && (mode & OS_MODE) && !(MechWeapons[Weapon2I(type)].special & OS_WEAP))
                strcat(buffer, "OS ");
            strcat(buffer, pos_part_name(mech, index, loop));
            if (IsWeapon(type) && (mode & OS_MODE) && (mode & OS_USED))
                strcat(buffer, " (Empty)");
        }

        if (PartIsDestroyed(mech, index, loop) && type != EMPTY &&
            (!IsCrap(type) || SectIsDestroyed(mech, index)))
            strcat(buffer, " (Destroyed)");
        else if (PartIsDisabled(mech, index, loop) && type != EMPTY)
            strcat(buffer, " (Disabled)");
	sprintf(buffer, "%-40s ", buffer);
	    sprintf(trash, "%-17s ", " ");
	    strcat(buffer, trash);
        if ((MechSpecials2(mech) & OMNI_TECH) && (mode & (HPOINTS))) {
            sprintf(trash, "%-17s ", (mode & ENERGY_HPOINT) ? "Energy Hardpoint" : (mode & BALL_HPOINT) ? "Ballistic Hardpoint" : (mode & MISSILE_HPOINT) ?
 "Missile Hardpoint" : (mode & SPECIAL_HPOINT) ? "Special Hardpoint" : (mode & GAUSS_HPOINT) ? "Gauss Hardpoint" :
	"Hardpoint Error");
            strcat(buffer, trash);
        } else {
            sprintf(trash, "%-17s ", " ");
            strcat(buffer, trash);
        }

        foo[count++] = strdup(buffer);
    }

    ArmorStringFromIndex(index, buffer, MechType(mech), MechMove(mech));
    strcat(buffer, " Criticals");
    cm = SelCol_StringMenu(3, buffer, foo);
    ShowCoolMenu(player, cm);
    KillCoolMenu(cm);
    KillText(foo);
}
#endif
char *evaluate_ammo_amount(int now, int max)
{
    int f = (now * 100) / MAX(1, max);

    if (f >= 50)
	return "%ch%cg";
    if (f >= 25)
	return "%ch%cy";
    return "%ch%cr";
}

#if 1
void PrintWeaponStatus(MECH * mech, dbref player)
{
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    unsigned char ammoweap[8 * MAX_WEAPS_SECTION];
    unsigned short ammo[8 * MAX_WEAPS_SECTION];
    unsigned short ammomax[8 * MAX_WEAPS_SECTION];
    unsigned int modearray[8 * MAX_WEAPS_SECTION];
    char tmpbuf[MBUF_SIZE];
    int count, ammoweapcount, weapcount = 0;
    int loop;
    int ii, i = 0;
    char weapname[80], *tmpc;
    char weapbuff[120];
    char specbuff[160];
    char tempbuff[160];
    char sensbuff[160];
    char location[80];
    int running_sum = 0;
    int ammo_mode;

    sensbuff[0] = 0;

    if (MechSpecials(mech) & BEAGLE_PROBE_TECH) {
      sprintf(sensbuff + strlen(sensbuff), "BeagleProbe(%s)  ",
	     (sensors[(short) MechSensor(mech)[0]].required_special == BEAGLE_PROBE_TECH ||
              sensors[(short) MechSensor(mech)[1]].required_special == BEAGLE_PROBE_TECH)
	      ? "%ch%crActive%cn" : "%crInactive%cn");
    }

    if (MechSpecials2(mech) & BLOODHOUND_PROBE_TECH) {
      sprintf(sensbuff + strlen(sensbuff), "BloodhoundProbe(%s)  ",
	     (sensors[(short) MechSensor(mech)[0]].required_special == BLOODHOUND_PROBE_TECH ||
              sensors[(short) MechSensor(mech)[1]].required_special == BLOODHOUND_PROBE_TECH)
	      ? "%ch%crActive%cn" : "%crInactive%cn");
    }

    if (MechSpecials(mech) & AA_TECH) {
      sprintf(sensbuff + strlen(sensbuff), "Radar(%s)  ",
	     (sensors[(short) MechSensor(mech)[0]].required_special == AA_TECH ||
              sensors[(short) MechSensor(mech)[1]].required_special == AA_TECH)
	      ? "%ch%crActive%cn" : "%crInactive%cn");
    }

    if (MechSpecials2(mech) & ADV_AA_TECH) {
      sprintf(sensbuff + strlen(sensbuff), "AdvancedRadar(%s)  ",
	     (sensors[(short) MechSensor(mech)[0]].required_special == ADV_AA_TECH ||
              sensors[(short) MechSensor(mech)[1]].required_special == ADV_AA_TECH)
	      ? "%ch%crActive%cn" : "%crInactive%cn");
    }
    notify(player, sensbuff);


#define GETSECTPHYS(a) \
	 (have_axe(mech, a) ? "(Axe)" : have_mace(mech, a) ? "(Mace)" : have_sword(mech, a) ? "(Sword)" : "")

#define SHOWSECTSTAT(a) \
	 (MechSections(mech)[(a)].recycle > 0) ? \
	 tprintf ("%-5d", MechSections(mech)[(a)].recycle) : "%cgReady%c"
    UpdateRecycling(mech);
    if (MechType(mech) == CLASS_MECH && !doweird) {
	tempbuff[0] = 0;
	specbuff[0] = 0;
#define SHOW(part,phys,loc) \
      sprintf(tempbuff + strlen(tempbuff), "%s%s: %s  ", part, phys, loc)

	SHOW(MechMove(mech) == MOVE_QUAD ? "FLLEG" : "LARM",
	    GETSECTPHYS(LARM), SHOWSECTSTAT(LARM));
	SHOW(MechMove(mech) == MOVE_QUAD ? "FRLEG" : "RARM",
	    GETSECTPHYS(RARM), SHOWSECTSTAT(RARM));
	SHOW(MechMove(mech) == MOVE_QUAD ? "RLLEG" : "LLEG",
	    GETSECTPHYS(LLEG), SHOWSECTSTAT(LLEG));
	SHOW(MechMove(mech) == MOVE_QUAD ? "RRLEG" : "RLEG",
	    GETSECTPHYS(RLEG), SHOWSECTSTAT(RLEG));
	notify(player, tempbuff);

	if ((MechSpecials(mech) & NULLSIG_TECH) &&
	    !(MechCritStatus(mech) & NULLSIG_DESTROYED)) {
	    sprintf(specbuff + strlen(specbuff), "Null(%s)  ",
		(MechStatus2(mech) & NULLSIG_ACTIVE) ? "%ch%crOn%cn" :
		"%crOff%cn");
	}

	if ((MechSpecials2(mech) & STEALTHARMOR_TECH) &&
	   !(MechCritStatus2(mech) & STEALTHARM_DESTROYED)) {
		sprintf(specbuff + strlen(specbuff), "SArm(%s)  ",
		(MechStatus2(mech) & STEALTHARM_ACTIVE) ? "%ch%crOn%cn" :
		"%crOff%cn");
	}

        if (((MechSpecials(mech) & ECM_TECH) || (MechSpecials2(mech) & ANGEL_ECM_TECH)) &&
            !(MechCritStatus(mech) & ECM_DESTROYED)) {
            sprintf(specbuff + strlen(specbuff), "%sECM(%s)  ",
		(MechSpecials2(mech) & ANGEL_ECM_TECH) ? "A" : "",
                (MechStatus(mech) & ECM_ACTIVE) ? "%ch%cgOn%cn" :
                (MechStatus2(mech) & ECCM_ACTIVE) ? "%ch%cgECCM%cn" : "%cgOff%cn");
        }

	if (MechSpecials(mech) & SLITE_TECH) {
	    sprintf(specbuff + strlen(specbuff), "SLITE(%s)  ",
		(MechCritStatus(mech) & SLITE_DEST) ? "%cr%chXX%cn"
		: (MechStatus(mech) & SLITE_ON) ? "%ch%cgOn%cn" :
		"%cgOff%cn");
	}

	if (MechSpecials(mech) & (C3_SLAVE_TECH))
	    sprintf(specbuff + strlen(specbuff), "%sC3-S%%cn  ",
		(MechCritStatus(mech) & C3_DESTROYED) ? "%cr" : "%cg");

	if (MechSpecials(mech) & (C3_MASTER_TECH))
	    sprintf(specbuff + strlen(specbuff), "%sC3-M%%cn  ",
		(MechCritStatus(mech) & C3_DESTROYED) ? "%cr" : "%cg");

	if (MechSpecials(mech) & MASC_TECH)
	    sprintf(specbuff + strlen(specbuff), "MASC: %s%d%%cn (%s)  ",
		MechMASCCounter(mech) >
		3 ? "%ch%cr" : MechMASCCounter(mech) >
		0 ? "%ch%cy" : "%cg", MechMASCCounter(mech),
		MechStatus(mech) & MASC_ENABLED ? "%cgOn%cn" : "%crOff%cn");

	if (MechSpecials2(mech) & SCHARGE_TECH)
            sprintf(specbuff + strlen(specbuff), "SCHARGE: %s%d%%cn (%s)",
                MechSChargeCounter(mech) >
                3 ? "%ch%cr" : MechSChargeCounter(mech) >
                0 ? "%ch%cy" : "%cg", MechSChargeCounter(mech),
                MechStatus2(mech) & SCHARGE_ENABLED ? "%cgOn%cn" : "%crOff%cn");
        notify(player, specbuff);

	if (MechStatus(mech) & FLIPPED_ARMS)
	    notify(player,
		"*** Mech arms are flipped into the rear arc ***");
    } else if ((MechType(mech) == CLASS_BSUIT || MechType(mech) == CLASS_VEH_GROUND || MechType(mech) == CLASS_VEH_VTOL || MechType(mech) == CLASS_VEH_NAVAL) && !doweird) {
	*tempbuff = 0;
	if (((MechSpecials(mech) & ECM_TECH) || (MechSpecials2(mech) & ANGEL_ECM_TECH)) &&
	    !(MechCritStatus(mech) & ECM_DESTROYED)) {
	    sprintf(tempbuff, "%sECM(%s)   ",
		(MechSpecials2(mech) & ANGEL_ECM_TECH) ? "A" : "",
		(MechStatus(mech) & ECM_ACTIVE) ? "%ch%cgOn%cn" :
		(MechStatus2(mech) & ECCM_ACTIVE) ? "%ch%cgECCM%cn" : "%cgOff%cn");
	}
        if ((MechSpecials(mech) & NULLSIG_TECH) &&
            !(MechCritStatus(mech) & NULLSIG_DESTROYED)) {
            sprintf(tempbuff, "Null(%s)   ",
                (MechStatus2(mech) & NULLSIG_ACTIVE) ? "%ch%crOn%cn" :
                "%crOff%cn");
        }
	if ((MechSpecials2(mech) & STEALTHARMOR_TECH) &&
	   !(MechCritStatus2(mech) & STEALTHARM_DESTROYED)) {
		sprintf(tempbuff, "SArm(%s)  ",
		(MechStatus2(mech) & STEALTHARM_ACTIVE) ? "%ch%crOn%cn" :
		"%crOff%cn");
	}
	if (MechSpecials(mech) & SLITE_TECH) {
	    sprintf(tempbuff + strlen(tempbuff), "SLITE(%s)  ",
		(MechCritStatus(mech) & SLITE_DEST) ? "%cr%chXX%cn"
		: (MechStatus(mech) & SLITE_ON) ? "%ch%cgOn%cn" :
		"%cgOff%cn");
	}

	if (MechSpecials(mech) & (C3_SLAVE_TECH))
	    sprintf(tempbuff + strlen(specbuff), "%sC3-S%%cn  ",
		(MechCritStatus(mech) & C3_DESTROYED) ? "%cr" : "%cg");

	if (MechSpecials(mech) & (C3_MASTER_TECH))
	    sprintf(tempbuff + strlen(specbuff), "%sC3-M%%cn  ",
		(MechCritStatus(mech) & C3_DESTROYED) ? "%cr" : "%cg");

        if (MechSpecials2(mech) & SCHARGE_TECH) {
            sprintf(tempbuff + strlen(tempbuff), "SCHARGE: %s%d%%cn (%s)",
                MechSChargeCounter(mech) >
                3 ? "%ch%cr" : MechSChargeCounter(mech) >
                0 ? "%ch%cy" : "%cg", MechSChargeCounter(mech),
                MechStatus2(mech) & SCHARGE_ENABLED ? "%cgOn%cn" : "%crOff%cn");
	}
	if (MechType(mech) == CLASS_BSUIT) {
	    for (i = 0; i < NUM_BSUIT_MEMBERS; i++)
		if (GetSectInt(mech, i))
		    break;
		if (i < NUM_BSUIT_MEMBERS)
		    sprintf(tempbuff + strlen(tempbuff), "Team status (special attacks): %s", SHOWSECTSTAT(i)); 
	} else {
	    if (MechSections(mech)[FSIDE].recycle)
		sprintf(tempbuff + strlen(tempbuff), "Vehicle status (charge): %s", SHOWSECTSTAT(FSIDE));
	}
	if (*tempbuff)
	    notify(player, tempbuff);
    }
    ammoweapcount =
	FindAmmunition(mech, ammoweap, ammo, ammomax, modearray);
    if (!doweird) {
	notify(player,
	    "==================WEAPON SYSTEMS===========================AMMUNITION========");
	if (MechType(mech) == CLASS_BSUIT)
	    notify(player,
		"------ Weapon ------- [##]  Holder ------ Status ||---- Ammo Type ---- Rounds");
	else
	    notify(player,
		"------ Weapon ------- [##]  Location ---- Status ||---- Ammo Type ---- Rounds");
    }
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	weapcount = (weapcount + (FindWeapons(mech, loop, weaparray, weapdata, critical)));
	}
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	count = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (count <= 0)
	    continue;
	ArmorStringFromIndex(loop, tempbuff, MechType(mech),
	    MechMove(mech));
	sprintf(location, "%-14.14s", tempbuff);
	if (doweird) {
	    strcpy(location, tempbuff);
	    if ((tmpc = strchr(location, ' ')))
		*tmpc = '_';
	}
	for (ii = 0; ii < count; ii++) {
	    if (IsAMS(weaparray[ii]))
		sprintf(weapbuff, " %-16.16s %c%c%c [%2d]  ",
		    &MechWeapons[weaparray[ii]].name[3],
		    (MechStatus(mech) & AMS_ENABLED) ? ' ' : 'O',
		    (MechStatus(mech) & AMS_ENABLED) ? 'O' : 'F',
		    (MechStatus(mech) & AMS_ENABLED) ? 'N' : 'F',
		    running_sum + ii);
	    else {
		if ((GetPartMode(mech, loop, critical[ii]) & OS_MODE) && !(MechWeapons[weaparray[ii]].special & OS_WEAP))
		    strcpy(tmpbuf, "OS ");
		else
		    tmpbuf[0] = 0;
		strcat(tmpbuf, &MechWeapons[weaparray[ii]].name[3]);
		sprintf(weapbuff, "%-16.16s %c%c%c%c [%2d]  ", tmpbuf,
		    (GetPartMode(mech, loop,
			    critical[ii]) & REAR_MOUNT) ? 'R' : ' ',
		    GetWeaponModeLetter(mech, loop, critical[ii]),
		    (GetWeaponModeLetter(mech, loop, critical[ii]) == 'U' ?
		   ((GetPartMode(mech, loop, critical[ii]) & PIERCE_MODE) ? 'P' :
		    (GetPartMode(mech, loop, critical[ii]) & CASELESS_MODE) ? 'C' :
		    (GetPartMode(mech, loop, critical[ii]) & TRACER_MODE) ? 'T' :
		    (GetPartMode(mech, loop, critical[ii]) & PRECISION_MODE) ? 'R' :
		     ' ') : (GetPartMode(mech, loop, critical[ii]) & HOTLOAD_MODE) ? '*' : ' '),
		    (GetPartMode(mech, loop,
			    critical[ii]) & OS_USED) ? 'E'
		    : (GetPartMode(mech, loop,
			    critical[ii]) & ON_TC) ? 'T' : (MechSections(mech)[loop].config &
		    STABILIZER_CRIT) ? 'S' : ' ',
		    running_sum + ii);
	    }
	    if (doweird)
		sprintf(weirdbuf + strlen(weirdbuf), "%s|%s",
		    &MechWeapons[weaparray[ii]].name[3], location);
	    strcat(weapbuff, location);

	    int weap_stat = WeaponIsNonfunctional(mech, loop, critical[ii],
				GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loop, critical[ii])))); 

	    if (weap_stat == 1)
		strcat(weapbuff, "%crDISABLE%c|| "); 
	    else if (weap_stat == 2)
		strcat(weapbuff, "%ch%cx*****%c  || ");
	    else if (GetPartMode(mech, loop, WeaponFirstCrit(mech, loop, critical[ii])) & JAMMED_MODE)
		strcat(weapbuff, "%cyJAMMED%c || ");
	    else if (weapdata[ii])
		strcat(weapbuff, tprintf(" %2d    || ", weapdata[ii]));
	    else if (weap_stat < 0)
	        strcat(weapbuff, "%cyReady%c  || ");
	    else
		strcat(weapbuff, "%cgReady%c  || ");
	    if ((ii + running_sum) < ammoweapcount) {
                if (ammoweap[ii + running_sum]>num_def_weapons) {
			notify(player,"Broken Template");
			return;
		}
		ammo_mode =
		    GetWeaponModeLetter_Model_Mode(ammoweap[ii +
		running_sum], modearray[ii + running_sum]);
		sprintf(weapname, "%-16.16s %c",
		    &MechWeapons[ammoweap[ii + running_sum]].name[3],
		    ammo_mode);
		sprintf(tempbuff, "  %s%3d%s",
		    evaluate_ammo_amount(ammo[ii + running_sum],
			ammomax[ii + running_sum]), ammo[ii + running_sum],
		    "%cn");
		strcat(weapname, tempbuff);
		if (doweird) {
		    if (ammo_mode && ammo_mode != ' ')
			sprintf(weirdbuf + strlen(weirdbuf), "|%s|%d|%c ",
			    &MechWeapons[ammoweap[ii +
				    running_sum]].name[3],
			    ammo[ii + running_sum], ammo_mode);
		    else
			sprintf(weirdbuf + strlen(weirdbuf), "|%s|%d ",
			    &MechWeapons[ammoweap[ii +
				    running_sum]].name[3],
			    ammo[ii + running_sum]);
		}
	    } else {
		if (doweird)
		    strcat(weirdbuf, " ");
		sprintf(weapname, "   ");
	    }
	    strcat(weapbuff, weapname);
	    if (!doweird)
		notify(player, weapbuff);
	}
	running_sum += count;
    }
    if (ammoweapcount > weapcount)
	{
	for (ii = 0; ii < (ammoweapcount - weapcount); ii++) {
                if (ammoweap[ii + running_sum]>num_def_weapons) {
                  notify(player,"Broken Template2");
                  return;
                }
		ammo_mode = GetWeaponModeLetter_Model_Mode(ammoweap[ii +
                running_sum], modearray[ii + running_sum]);
                sprintf(weapname, "                                                 || %-16.16s %c",
                    &MechWeapons[ammoweap[ii + running_sum]].name[3],
                    ammo_mode);
                sprintf(tempbuff, "  %s%3d%s",
                    evaluate_ammo_amount(ammo[ii + running_sum],
                        ammomax[ii + running_sum]), ammo[ii + running_sum],
                    "%cn");
                strcat(weapname, tempbuff);
                if (doweird) {
                    if (ammo_mode && ammo_mode != ' ')
                        sprintf(weirdbuf + strlen(weirdbuf), "|%s|%d|%c ",
                            &MechWeapons[ammoweap[ii +
                                    running_sum]].name[3],
                            ammo[ii + running_sum], ammo_mode);
                    else
                        sprintf(weirdbuf + strlen(weirdbuf), "|%s|%d ",
                            &MechWeapons[ammoweap[ii +
                                    running_sum]].name[3],
                            ammo[ii + running_sum]);
                    }
          	if (!doweird)
                	notify(player, weapname);
		}
            }
}
#else

void PrintWeaponStatus(MECH * mech, dbref player)
{
    unsigned char weaparray[MAX_WEAPS_SECTION];
    unsigned char weapdata[MAX_WEAPS_SECTION];
    int critical[MAX_WEAPS_SECTION];
    unsigned char ammoweap[8 * MAX_WEAPS_SECTION];
    unsigned short ammo[8 * MAX_WEAPS_SECTION];
    unsigned short ammomax[8 * MAX_WEAPS_SECTION];
    unsigned char modearray[8 * MAX_WEAPS_SECTION];
    int count, ammoweapcount;
    int loop;
    int ii, i = 0;
    char weapname[80], *tmpc;
    char weapbuff[120];
    char tempbuff[160];
    char location[80];
    int running_sum = 0;
    short ammo_mode;
    int shown = 0;
    char *loc;
    coolmenu *c = NULL;

    if (MechType(mech) == CLASS_MW)
	return;
    UpdateRecycling(mech);
    ammoweapcount =
	FindAmmunition(mech, ammoweap, ammo, ammomax, modearray);
    addmenu4("#  Weap  Loc Stat");
    addmenu4("#  Weap  Loc Stat");
    addmenu4("#  Weap  Loc Stat");
    addmenu4("Ammo type  Rounds");
    for (loop = 0; loop < NUM_SECTIONS; loop++) {
	count = FindWeapons(mech, loop, weaparray, weapdata, critical);
	if (count <= 0)
	    continue;
	loc =
	    ShortArmorSectionString(MechType(mech), MechMove(mech), loop);
	ii = 0;
	while (ii < count) {
	    for (; shown < 3 && ii < count; shown++) {
		sprintf(weapbuff, "%2d %-5s %-3s ", running_sum + ii,
		    MechWeapons[weaparray[ii]].shortname, loc);
		if (IsAMS(weaparray[ii]))
		    strcat(weapbuff,
			MechStatus(mech) & AMS_ENABLED ? "+" : "-");
		else
		    sprintf(weapbuff + strlen(weapbuff), "%c",
			GetWeaponModeLetter(mech, loop, critical[ii]));
		if (PartIsNonfunctional(mech, loop, critical[ii]) || PartIsDisabled(mech, loop, critical[ii]) ||
                    WeaponIsNonfunctional(mech, loop, critical[ii], GetWeaponCrits(mech, Weapon2I(GetPartType(mech, loop, critical[ii])))) > 0)
			strcat(weapbuff, "%ch%cx***%cn");
		else if (weapdata[ii])
		    strcat(weapbuff, tprintf(" %2d", weapdata[ii]));
		else
		    strcat(weapbuff, "%cg+++%cn");
		addmenu4(weapbuff);
		ii++;
	    }
	    if (shown == 3) {
		if ((i = (ii + running_sum) / 3) < ammoweapcount) {
		    ammo_mode =
			GetWeaponModeLetter_Model_Mode(ammoweap[i],
			modearray[i]);
		    sprintf(weapbuff, "%-10s %c %5d",
			MechWeapons[ammoweap[i]].shortname, ammo_mode,
			ammo[i]);
		} else
		    addmenu4(" ");
		shown = 0;
	    }
	}
	running_sum += count;
    }
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}
#endif

int ArmorEvaluateSerious(MECH * mech, int loc, int flag, int *opt)
{
    int a, b, c = -1;

    if (flag & 2) {
	if (SectIntsRepair(mech, loc))
	    c = 5;		/* Blue */
	a = (((b =
		  (is_aero(mech) ? AeroSI(mech) : GetSectInt(mech, loc))) + 1) * 100) / ((is_aero(mech) ? AeroSIOrig(mech) : GetSectOInt(mech,
		loc)) + 1);
    } else if (flag & 4) {
	if (SectRArmorRepair(mech, loc))
	    c = 5;		/* Blue */
	a = ((1 + (b =
		    GetSectRArmor(mech,
		     loc))) * 100) / (GetSectORArmor(mech, loc) + 1);
    } else {
	if (SectArmorRepair(mech, loc))
	    c = 5;		/* Blue */
	a = ((1 + (b =
		    GetSectArmor(mech, loc))) * 100) / (GetSectOArmor(mech,
		loc) + 1);
    }
    *opt = b;
    if (c > 0)
	return c;
    if (!b)
	return 4;
    if (a > 90)
	return 0;
    if (a > 60)
	return 1;
    if (a > 33)
	return 2;
    return 3;
}

/* bright green, dark green, bright yellow, dark red, black */

static char *armordamcolorstr[] = { "%ch%cg", "%cg", "%ch%cy",
    "%cr", "%ch%cx", "%ch%cb"
};				/* last on is for armor being repaired */
static char *armordamltrstr = "OoxX*?";

char *PrintArmorDamageColor(MECH * mech, int loc, int flag)
{
    int a;

    return armordamcolorstr[ArmorEvaluateSerious(mech, loc, flag, &a)];
}

char *PrintArmorDamageString(MECH * mech, int loc, int flag)
{
    int a;
    static char foo[6];

    for (a = 0; a < 4; a++)
	foo[a] = 0;
    foo[0] = armordamltrstr[ArmorEvaluateSerious(mech, loc, flag, &a)];
    if (flag & 1) {
	if (flag & 8)
	    sprintf(foo, "%1d", (flag & 32) ? ((a + 9) / 10) : a);
	else if (flag & 128)
	    sprintf(foo, "%3d", a);
	else
	    sprintf(foo, "%2d", a);
	if ((flag & 16) && foo[0] == ' ')
	    foo[0] = '0';
    } else
	foo[1] = (flag & 8 ? 0 : foo[0]);
    return foo;
}

char *ArmorKeyInfo(dbref player, int keyn, int owner)
{
    static char str[20];

    if (owner) {
	str[0] = 0;
	return str;
    }
    if (keyn == 1) {
	strcpy(str, "Key");
	return str;
    }
    if (keyn > 6) {
	strcpy(str, "   ");
	return str;
    }
    sprintf(str, "%s%c%c%%c ", armordamcolorstr[6 - keyn],
	armordamltrstr[6 - keyn], armordamltrstr[6 - keyn]);
    return str;
}

/* Params: a = location, b = flag (0/1=owner, 2 = internal, 4 = rear, 8 = minifield), 16 = zero-padding, 32 = MW, 64 = show spaces if destroyed loc */

char *show_armor(MECH * mech, int loc, int flag)
{
    static char foo[32];

    if (!GetSectInt(mech, loc) && (flag & 64) && !is_aero(mech))
	sprintf(foo, (flag & 32) ? " " : (flag & 128) ? "   " : "  ");
    else
	sprintf(foo, "%s%s%%c", PrintArmorDamageColor(mech, loc, flag),
	    PrintArmorDamageString(mech, loc, flag));
    return foo;
}

/* Fancy idea.. :-) */

/* Just get da desc from string, strtok'ed with \ns. */

/* 1-6 at beginning of line = key */

/* &+num = armor */

/* &-num = rear armor  */

/* && = & */

/* &:num = internal */

/* &(stuff = len 1 column */

/* &)stuff = len 3 column (DSs) */

/* @<num><char> Shown only if loc <num> is intact */

/* !<num><num2><char> Shown only if loc <num> or loc <num2> is intact */

/* 0 on empty line ends the script */

char *mechdesc =
    "1         FRONT                REAR                INTERNAL\n"
    "2         @7(&+7@7)                 @7(@7*@7*@7)                  @7(&:7@7)\n"
    "3      @2/&+2!24|&+4!43|&+3@3\\           @2/&-2!24|&-4!34|&-3@3\\            @2/&:2!24|&:4!34|&:3@3\\\n"
    "4     @0(&+0!05/ !54|!64| !16\\&+1@1)         @0(   !54|  !64|   @1)          @0(&:0!05/ !54|!64| !16\\&:1@1)\n"
    "5       @5/  @5/@6\\  @6\\               @5/  @6\\                @5/  @5/@6\\  @6\\\n"

    "6      @5(&+5@5/  @6\\&+6@6)             @5/    @6\\              @5(&:5@5/  @6\\&:6@6)\n0";

char *quaddesc =
    "7         FRONT                REAR                INTERNAL\n"
    "1           @7_@7_@7_                                      @7_@7_@7_\n"
    "2   @5_@5_@5_  @2_@2_@7/&+7 @7\\@3_  @6_@6_@6_                       @5_@5_@5_  @2_@2_@7/&:7 @7\\@3_  @6_@6_@6_\n"
    "3  @5( &+5@5\\@2(&+2!24|&+4!43|&+3@3)@6/&+6 @6)     @2(&-2!24|&-4!43|&-3@3)      @5( &:5@5\\@2(&:2!24|&:4!43|&:3@3)@6/&:6 @6)\n"
    "4   @5\\ @5\\!05(&+0@0/    @1\\&+1!16)@6/ @6/                       @5\\ @5\\!05(&:0@0/    @1\\&:1!16)@6/ @6/\n"
    "5    @5\\ @5\\@0|@0|      @1|@1|@6/ @6/                         @5\\ @5\\@0|@0|      @1|@1|@6/ @6/\n"

    "6    @5/@5_!05/@0_@0_@0\\    @1/@1_@1_!16\\@6_@6\\                         @5/@5_!05/@0_@0_@0\\    @1/@1_@1_!16\\@6_@6\\\n0";


char *mwdesc =
    "1                       @7(&:7@7)\n"
    "2                     @2/&:2@4|&:4@4|&:3@3\\\n"

    "6                    @0(&:0@0/ @4_ @1\\&:1@1)\n"
    "7                     @5(&:5@5/ @6\\&:6@6)\n0";


/*
   DROPSHIP
   AERODINE     SUB        SHIP
   .--.         --       ./99\.
   ,`.99.'.     =|99|=    |'.--.`|
   |.|__|.|     |\__/|    |`|99|'|
   | | 99 | |   | |99| |   |\`--'/|
   | |:  :| |   |99||99|   |99><99|
   |'99|--|99`|   \|--|/    |./||\.|
   |  .'99`.  |   =|99|=    || 99 ||
   `|`_\~~/_`|'              `~~~~'



   Fighter
   /^^\
   /|`99'|\
   |     |_|.--.|_|     |
   |      /||99||\      |
   |    /'.-|--|-.`\    |
   |---'99| |99| |99`---|
   `--_____\||||/_____--'
   `=24='
 */


char *shipdesc =
    "7         FRONT                    INTERNAL\n"
    "1         @2.@2/&+2@2\\@2.                    @2.@2/&:2@2\\@2. \n"
    "2        @2|@2'@4.@4-@4-@4.@2`@2|                  @2|@2'@4.@4-@4-@4.@2`@2|\n"
    "3        @2|@2`@4|&+4@4|@2'@2|                  @2|@2`@4|&:4@4|@2'@2|\n"
    "4        !20|!04\\@4`@4-@4-@4'!14/!12|                  !20|!04\\@4`@4-@4-@4'!14/!12|  \n"
    "5        @0|&+0@0>@1<&+1@1|                  @0|&:0@0>@1<&:1@1|  \n"
    "7        @0|!03.!03/!03|!13|!13\\!13.@1|                  @0|!03.!03/!03|!13|!13\\!13.@1|  \n"
    "7        @0|@3| &+3 @3|@1|                  @0|@3| &:3 @3|@1|  \n"

    "7         !03`@3~@3~@3~@3~!13'                    !03`@3~@3~@3~@3~!13' \n"
    "0";

char *foildesc =
    "7         FRONT                    INTERNAL\n"
    "7         @2.@2/@2\\@2.                       @2.@2/@2\\@2.\n"
    "1      @2_@2_@2_@2|&+2@2|@2_@2_@2_                 @2_@2_@2_@2|&:2@2|@2_@2_@2_\n"
    "2      @2-@2-@2-@2|!24>!24<@2|@2-@2-@2-                 @2-@2-@2-@2|!24>!24<@2|@2-@2-@2-\n"
    "3        !04|@4|&+4@4|!14|                     !04|@4|&:4@4|!14| \n"
    "4     @0_@0_@0|&+0!01:!01:&+1@1|@1_@1_               @0_@0_@0|&:0!01:!01:&:1@1|@1_@1_\n"
    "5     @0-@0-@0_@0|!03'&+3!13`@1|@1_@1-@1-               @0-@0-@0_@0|!03'&:3!13`@1|@1_@1-@1-\n"

    "7        !03`@3~@3~@3~@3~!13'                     !03`@3~@3~@3~@3~!13'\n"
    "0";

char *subdesc =
    "7        FRONT                     INTERNAL\n"
    "1         @2-@2-                         @2-@2-      \n"
    "1       @2=@2|&+2@2|@2=                     @2=@2|&:2@2|@2=    \n"
    "1       !02|!24\\!24_!24_!24/!12|                     !02|!24\\!24_!24_!24/!12|    \n"
    "1      @0| @4|&+4@4| @4|                   @0| @4|&:4@4| @4|   \n"
    "1      @0|&+0@0|@1|&+1@1|                   @0|&:0@0|@1|&:1@1|   \n"
    "1       @0\\!03|@3-@3-!13|@1/                     @0\\!03|@3-@3-!13|@1/    \n"

    "1       !03=@3|&+3@3|!13=                     !03=@3|&:3@3|!13=    \n"
    "0";

char *aerodesc =
    "7              /^^\\\n"
    "1            /|`&+0'|\\\n"
    "2     |     |_|.--.|_|     |\n"
    "3     |      /||  ||\\      |\n"
    "4     |    /'.-|--|-.`\\    |\n"
    "5     |---'&+1| |&:0| |&+2`---|\n"

    "6     `--_____\\||||/_____--'\n"
    "7             `=&+3='\n0";

#if 0
char *aerodesc =
    "1              %ch%cg/^\\%cn\n"
    "2             %ch%cg|   |%cr-----%cn&+0%cn\n"
    "3           %ch%cmo %cg| %cyx%cr-%cg|%cr-%cmo%cr---%cn&+4%cn\n"
    "4          %ch%cg_%cb|%cg/  %cyx  %cg\\%cb|%cg_%cn\n"
    "5     %ch%cm|--~~%cn&+1%ch%cg_ %cn&+3  %ch%cg_%cn&+2%ch%cm~~--|%cn\n"

    "6     %ch%cm|--~~~~ %cg\\ %cy| %cg/ %cm~~~~--|%cn\n"
    "7              %ch%cyO%crO%cyO%cr------%cn&+5\n0";

char *aerodesc =
    "1      (No fancy picture yet, nyah :-))\n"
    "2                Nose: &+0\n" "3        LWing: &+1     RWing: &+2\n"
    "4         Fusel: &+3 Cockpit: &+4\n" "5              Engine: &+5\n0";
#endif

#if 0
char *spher_ds_desc =
    "7          FRONT\n" "1          @0_@0_@0_@0_@0_@0_@0_\n"
    "2         @0/!01`!01.&)+0!02,!02'@0\\\n"
    "3        @1/   !01|@0~!02|   @2\\                     _______\n"
    "4       @1|&)+1 !12| !12|&)+2 @2|                    | &):0 |\n"
    "5        @1\\   !13|@3_!23|   @2/                     ~~~~~~~\n"
    "6         @3\\!13,!13'&)+3!23`!23.@3/\n"

    "7          @3~@3~@3~@3~@3~@3~@3~\n0";
#endif

char *spher_ds_desc =
    "7          FRONT\n" "1          _______\n"
    "2         /`.&)+0,'\\\n"
    "3        /   |~|   \\                     _______\n"
    "4       |&)+1 | |&)+2 |                    | &):0 |\n"
    "5        \\   |_|   /                     ~~~~~~~\n"
    "6         \\,'&)+3`./\n"

    "7          ~~~~~~~\n0";

#if 0
char *spher_ds_desc =
    "7          FRONT\n" "1          @1_@1_@1_@1_@1_@1_@1_\n"
    "2         @1/!15`!15.&)+5!05,!05'@0\\\n"
    "3        @1|&)+1@1|@1~@0|&)+0@0|\n"
    "4        !12|  !12|   !03|  !03|\n"
    "5        @2|&)+2@2|@4_@3|&)+3@3|\n"
    "6         @2\\!24,!24'&)+4!34`!34.@3/\n"

    "7          @4~@4~@4~@4~@4~@4~@4~\n0";
#endif

char *aerod_ds_desc =
    "7           .--.\n" 
    "1         ,`&)+0 '.\n"
    "2         |.|__|.|\n"
    "3        | | ?? | |                      _______\n"
    "4        | |:  :| |                      | &):0 |\n"
    "5       |&)+1|--|&)+2|                     ~~~~~~~\n"
    "6       |  .&)+3 .  |\n" 
    "7       `|`_\\~~/_`|'\n0";

char *vehdesc =
    "1          FRONT                                INTERNAL\n"
    "2         @0,@0`!02.&+2!12,@1'@1.                              @0,@0`!02.&:2!12,@1'@1.\n"
    "3        @0|  !02|@4_@4_!12|  @1|                            @0|  !02|@4_@4_!12|  @1|\n"
    "4        @0|  @4|&+4@4|  @1|                            @0|  @4|&:4@4|  @1|\n"
    "5        @0|&+0@4|@4~@4~@4|&+1@1|                            @0|&:0@4|@4~@4~@4|&:1@1|\n"
    "6         @0\\!03,!03'&+3!13`!13.@1/                              @0\\!03,!03'&:3!13`!13.@1/\n"

    "7          @3~@3~@3~@3~@3~@3~                                @3~@3~@3~@3~@3~@3~\n0";

#ifdef HEAVY_TANK
char *hvyvehhdesc =
    "7          FRONT                                INTERNAL\n"
    "1          @1_@1_@1_@1_@1_@1_@1_                               @1_@1_@1_@1_@1_@1_@1_\n"
    "2         @1/!15`!15.&)+5!05,!05'@0\\                              @1/!15`!15.&):5!05,!05'@0\\\n"
    "3        @1|&)+1@1|@1~@0|&)+0@0|                              @1|&):1@1|@1~@0|&):0@0|\n"
    "4        !12|  !12|   !03|  !03|                              !12|  !12|   !03|  !03|\n"
    "5        @2|&)+2@2|@4_@3|&)+3@3|                              @2|&):2@2|@4_@3|&):3@3|\n"
    "6         @2\\!24,!24'&)+4!34`!34.@3/                               @2\\!24,!24'&):4!34`!34.@3/\n"

    "7          @4~@4~@4~@4~@4~@4~@4~                                @4~@4~@4~@4~@4~@4~@4~\n0";
#endif

/* Turretless vehicle pic */
char *veh_not_desc =
    "1          FRONT                                INTERNAL\n"
    "2         @0,@0`!02.&+2!12,@1'@1.                              @0,@0`!02.&:2!12,@1'@1.\n"
    "3        @0|  !02|@2_@2_!12|  @1|                            @0|  !02|@2_@2_!12|  @1|\n"
    "4        @0|  @0|  @1|  @1|                            @0|  @0|  @1|  @1|\n"
    "5        @0|&+0@0|!01~!01~@1|&+1@1|                            @0|&:0@0|!01~!01~@1|&:1@1|\n"
    "6         @0\\!03,!03'&+3!13`!13.@1/                              @0\\!03,!03'&:3!13`!13.@1/\n"

    "7          @3~@3~@3~@3~@3~@3~                                @3~@3~@3~@3~@3~@3~\n0";

#if 0
char *vtoldesc =
    "7     .   ...   .\n" "1     \\\\ `___` //\n" "2      \\\\ &+2.//\n"
    "3     __\\\\ _ //__\n" "4    (&+0|(&+5 )|&+1)\n"
    "5    *--|//  \\\\--*\n" "6       // &+3 \\\\\n"
    "7      //\\___/ \\\\\n" "7      ~        ~\n0";
#endif

char *vtoldesc =
    "7        FRONT                               INTERNAL\n"
    "7     @5.   @2.@2.   @5.                            @5.   @2.@2.   @5.   \n"
    "1     @5\\@5\\ @2`@2_@2_@2` @5/@5/                            @5\\@5\\ @2`@2_@2_@2` @5/@5/   \n"
    "2      @5\\@5\\@2.&+2@2.@5/@5/                              @5\\@5\\@2.&:2@2.@5/@5/    \n"
    "3     @0_@0_@5\\@5\\  @5/@5/@1_@1_                            @0_@0_@5\\@5\\  @5/@5/@1_@1_   \n"
    "4    @0(&+0!05|@5(&+5@5)!15|&+1@1)                          @0(&:0!05|@5(&:5@5)!15|&:1@1)  \n"
    "5    @0*@0-@0-@0|@5/@5/ @5\\@5\\@1-@1-@1*                          @0*@0-@0-@0|@5/@5/ @5\\@5\\@1-@1-@1*  \n"
    "6       @5/@5/&+3 @5\\@5\\                               @5/@5/&:3 @5\\@5\\    \n"

    "7      @5/@5/@3\\@3_@3_@3/ @5\\@5\\                             @5/@5/@3\\@3_@3_@3/ @5\\@5\\   \n"
    "7      @5~       @5~                             @5~       @5~   \n0";

char *bsuitdesc =
    "7 SQUAD STATUS\n"
    "7  Member#      @01  @12  @23  @34  @45  @56  @67  @78\n"
    "7   Health      &:0 &:1 &:2 &:3 &:4 &:5 &:6 &:7\n"

    "7   Armor       &+0 &+1 &+2 &+3 &+4 &+5 &+6 &+7\n" "7\n0";

void PrintArmorStatus(dbref player, MECH * mech, int owner)
{
    char tmpbuf[1024];
    char *p, *q, *r;
    char *destbuf;
    int flag;
    int gflag = 0;
    int odd = 0;

    /* All we need is proper source for stuff */
    if (!*(destbuf = silly_atr_get(mech->mynum, A_MECHSTATUS)))
	switch (MechType(mech)) {
	case CLASS_MW:
	    strcpy(tmpbuf, mwdesc);
	    gflag |= 8 | 32 | 64;
	    break;
	case CLASS_MECH:
	    if (MechMove(mech) == MOVE_QUAD)
		strcpy(tmpbuf, quaddesc);
	    else
		strcpy(tmpbuf, mechdesc);
	    gflag |= 64;
	    break;
	case CLASS_BSUIT:
	    strcpy(tmpbuf, bsuitdesc);
	    gflag |= 64;
	    break;
	case CLASS_VEH_VTOL:
	    strcpy(tmpbuf, vtoldesc);
	    gflag |= 64;
	    break;
	case CLASS_AERO:
	    gflag |= 16 | 64;
	    strcpy(tmpbuf, aerodesc);
	    break;
	case CLASS_DS:
	    strcpy(tmpbuf, aerod_ds_desc);
	    break;
	case CLASS_SPHEROID_DS:
	    gflag |= 64;
	    strcpy(tmpbuf, spher_ds_desc);
	    break;
	case CLASS_VEH_GROUND:
	    if (GetSectOInt(mech, TURRET))
		strcpy(tmpbuf, vehdesc);
	    else
		strcpy(tmpbuf, veh_not_desc);
	    gflag |= 64;
	    break;
	case CLASS_VEH_NAVAL:
	    if (MechMove(mech) == MOVE_FOIL)
		strcpy(tmpbuf, foildesc);
	    else if (MechMove(mech) == MOVE_HULL)
		strcpy(tmpbuf, shipdesc);
	    else
		strcpy(tmpbuf, subdesc);
	    gflag |= 64;
	    break;
	default:
	    strcpy(tmpbuf,
		" This 'toy' is of unknown type to me. It has yet to be templated\n for status.\n0");
	    break;
    } else
	strcpy(tmpbuf, destbuf);
    p = strtok(tmpbuf, "\n");
    while (p && *p != '0') {
	if (*p >= '1' && *p <= '7')
	    strcpy(destbuf, ArmorKeyInfo(player, (int) *p - '0', owner));
	else
	    destbuf[0] = 0;
	r = &destbuf[strlen(destbuf)];
	for (q = p + 1; *q; q++)
	    if (*q == '@' && isdigit(*(q + 1))) {
		q++;
		if (GetSectInt(mech, (int) (*q - '0')))
		    *r++ = *(q + 1);
		else
		    *r++ = ' ';
		q++;
	    } else if (*q == '!' && isdigit(*(q + 1)) && isdigit(*(q + 2))) {
		q++;
		if (GetSectInt(mech, (int) (*q - '0')) ||
		    GetSectInt(mech, (int) (*(q + 1) - '0')))
		    *r++ = *(q + 2);
		else
		    *r++ = ' ';
		q++;
		q++;
	    } else if (*q == '&' && (*(q + 1) == '+' || *(q + 1) == '-' ||
		    *(q + 1) == ':' || *(q + 1) == '(' || *(q + 1) == ')')) {
		if (*(q + 1) == '(') {
		    gflag |= 8;
		    q++;
		    odd = 1;
		}
		if (*(q + 1) == ')') {
		    gflag |= 128;
		    q++;
		    odd = 1;
		}
		/* Geez, we got armor token to distribute here. */
		flag = owner;
		q++;
		switch (*q) {
		case '-':
		    flag += 2;
		case ':':
		    flag += 2;
		    break;
		}
		strcpy(r, show_armor(mech, (int) (*(q + 1) - '0'),
			flag | gflag));
		r += strlen(r);
		if (odd) {
		    gflag = 0;
		    odd = 0;
		}
		q++;
	    } else
		*r++ = *q;
	*r = 0;
	notify(player, destbuf);
	p = strtok(NULL, "\n");
    }
}


void mech_showpods(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    int i, narc = 0, inarc = 0;
    char LocBuff[LBUF_SIZE], InfoBuff[SBUF_SIZE] = { '\0' };

    cch(MECH_USUALSM);
    DOCHECK(!((MechStatus(mech) & NARC_ATTACHED) || (MechStatus2(mech) & INARC_ATTACHED)) && !Wizard(player),
	"You have no pods attached!");

    notify(player, tprintf("---------- Pod listing for %s ----------", Name(Location(player))));
    for (i = 0; i < NUM_SECTIONS; i++) {
	if (MechSections(mech)[i].config & SECTION_NARC)
	    narc++;
        if (MechSections(mech)[i].config & SECTION_INARC)
	    inarc++;
	if (!narc && !inarc)
	    continue;
        if (narc)
	    strcat(InfoBuff, "NARC");
	if (inarc && narc)
	    strcat(InfoBuff, " INARC");
        else if (inarc)
	    strcat(InfoBuff, "INARC");
        ArmorStringFromIndex(i, LocBuff, MechType(mech), MechMove(mech));
	notify(player, tprintf("%-15s | %s", LocBuff, InfoBuff));
        narc = 0;
	inarc = 0;
	InfoBuff[0] = '\0';
    }
	notify(player, "---------------------------------");
}
