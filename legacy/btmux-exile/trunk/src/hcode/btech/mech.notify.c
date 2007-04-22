#include <sys/file.h>

#include "mech.h"
#include "mech.events.h"
#include "autopilot.h"
#include "p.mech.los.h"
#include "p.mine.h"
#include "p.mech.utils.h"
#include "p.mech.build.h"
#include "p.mech.startup.h"
#include "p.autopilot_command.h"
#include "p.btechstats.h"
#include "p.mech.sensor.h"
#include "p.map.obj.h"

void sendchannelstuff(MECH * mech, int freq, char *msg);

const char *GetAmmoDesc_Model_Mode(int model, int mode)
{
    if (!IsArtillery(model)) {
        if (mode & PRECISION_MODE)
	    return " Precision";
	if (mode & PIERCE_MODE)
	    return " Piercing";
	if (mode & TRACER_MODE)
	    return " Tracer";
	if (mode & SGUIDED_MODE)
	    return " SemiGuided";
        if (mode & STINGER_MODE)
	    return " Stinger";
	if (mode & ATMER_MODE)
	    return " ATM ER";
	if (mode & ATMHE_MODE)
	    return " ATM HE";
	if (mode & CASELESS_MODE)
	    return " Caseless";
	if (mode & LBX_MODE)
	    return " Shotgun";
	if ((mode & CAPPED_MODE) && MechWeapons[model].type == TBEAM)
	    return " Capacitated";
	if (mode & ARTEMIS_MODE)
	    return " Artemis IV";
	if (mode & NARC_MODE)
	    return (MechWeapons[model].special & NARC) ? " Explosive" :
		" Narc compatible";
	if (mode & SWARM_MODE)
	    return " Swarm";
	if (mode & SWARM1_MODE)
	    return " Swarm1";
	if (mode & INFERNO_MODE)
	    return " Inferno";
	if (mode & DEADFIRE_MODE)
	    return " Deadfire";
    } else {
	if (mode & CLUSTER_MODE)
	    return " Cluster";
	if (mode & SMOKE_MODE)
	    return " Smoke";
	if (mode & MINE_MODE)
	    return " Mine";
	if (mode & INCEND_MODE)
	    return " Incendiary";
    }
    return "";
}


char GetWeaponModeLetter_Model_Mode(int model, int mode)
{
    if ((mode & ULTRA_MODE) && !(MechWeapons[model].special & ROTARY))
	return 'U';
    if (MechWeapons[model].special & ROTARY)
      {
      if (mode & ULTRA_MODE)
	return '2';
      else if (mode & ROTFOUR_MODE)
	return '4';
      else if (mode & ROTSIX_MODE)
	return '6';
      }
    if (mode & CASELESS_MODE)
	return 'C';
    if (!(mode & AMMO_MODES))
	return ' ';
    if (IsArtillery(model)) {
	if (mode & CLUSTER_MODE)
	    return 'C';
	if (mode & SMOKE_MODE)
	    return 'S';
	if (mode & MINE_MODE)
	    return 'M';
	if (mode & INCEND_MODE)
	    return 'F';
    } else {
	if (mode & DEADFIRE_MODE)
	    return 'D';
	if (mode & TRACER_MODE)
	    return 'T';
	if (mode & SGUIDED_MODE)
	    return 'G';
        if (mode & STINGER_MODE)
	    return 'Z';
        if (mode & ATMER_MODE)
	    return 'E';
	if (mode & ATMHE_MODE)
	    return 'H';
	if (mode & HEAT_MODE)
	    return 'F';
        if (mode & PRECISION_MODE)
	    return 'R';
	if (mode & PIERCE_MODE)
	    return 'P';
	if (mode & LBX_MODE)
	    return 'L';
	if ((mode & CAPPED_MODE) && MechWeapons[model].type == TBEAM)
	    return 'X';
	if (mode & ARTEMIS_MODE)
	    return 'A';
	if (mode & NARC_MODE)
	    return (MechWeapons[model].special & NARC) ? 'X' : 'N';
	if (mode & INFERNO_MODE)
	    return 'I';
	if (mode & SWARM_MODE)
	    return 'W';
	if (mode & SWARM1_MODE)
	    return '1';
        if (mode & HOTLOAD_MODE)
            return '*';
	if (mode & OS_MODE)
	    return 'O';
    }
    return ' ';
}

char GetWeaponModeLetter(MECH * mech, int loop, int crit)
{
    return GetWeaponModeLetter_Model_Mode(Weapon2I(GetPartType(mech, loop,
		crit)), GetPartMode(mech, loop, crit));
}

const char *GetMoveTypeID(int movetype)
{
    static char buf[20];

    switch (movetype) {
    case MOVE_QUAD:
	strcpy(buf, "QUAD");
	break;
    case MOVE_BIPED:
	strcpy(buf, "BIPED");
	break;
    case MOVE_TRACK:
	strcpy(buf, "TRACKED");
	break;
    case MOVE_WHEEL:
	strcpy(buf, "WHEELED");
	break;
    case MOVE_HOVER:
	strcpy(buf, "HOVER");
	break;
    case MOVE_VTOL:
	strcpy(buf, "VTOL");
	break;
    case MOVE_FLY:
	strcpy(buf, "FLY");
	break;
    case MOVE_HULL:
	strcpy(buf, "HULL");
	break;
    case MOVE_SUB:
	strcpy(buf, "SUBMARINE");
	break;
    case MOVE_FOIL:
	strcpy(buf, "HYDROFOIL");
	break;
    default:
	strcpy(buf, "Unknown");
	break;
    }
    return buf;
}

struct {
    char *onmsg;
    char *offmsg;
    int flag;
    int infolvl;
} temp_flag_info_struct[] = {
    {
    "DESTROYED", NULL, DESTROYED, 1}, {
    NULL, "SHUTDOWN", STARTED, 1}, {
    "Torso is 60 degrees right", NULL, TORSO_RIGHT, 0}, {
    "Torso is 60 degrees left", NULL, TORSO_LEFT, 0}, {
    NULL, NULL, 0}
};

struct {
    int team;
    char *ccode;
} obs_team_color[] = {
    {
    1, "%cw" }, {
    2, "%cc" }, {
    3, "%cm" }, {
    4, "%cb" }, {
    5, "%cy" }, {
    6, "%cg" }, {
    7, "%cr" }, {
    8, "%ch%cx" }, {
    9, "%ch%cw" }, {
   10, "%ch%cc" }, {
   11, "%ch%cm" }, {
   12, "%ch%cb" }, {
   13, "%ch%cy" }, {
   14, "%ch%cg" }, {
   15, "%ch%cr" }, {
    0, "%ch%cw" }
};

void Mech_ShowFlags(dbref player, MECH * mech, int spaces, int level)
{
    char buf[MBUF_SIZE];
    int i, temp;
    MECH *tempMech = NULL;

    for (i = 0; i < spaces; i++)
	buf[i] = ' ';
    buf[spaces] = 0;
    if (Fallen(mech)) {
	switch (MechMove(mech)) {
	case MOVE_BIPED:
	    strcpy(buf + spaces, "FALLEN");
	    break;
	case MOVE_QUAD:
	    strcpy(buf + spaces, "FALLEN");
	    break;
	case MOVE_TRACK:
	    strcpy(buf + spaces, "TRACK DESTROYED");
	    break;
	case MOVE_WHEEL:
	    strcpy(buf + spaces, "AXLE DESTROYED");
	    break;
	case MOVE_HOVER:
	    strcpy(buf + spaces, "LIFT FAN DESTROYED");
	    break;
	case MOVE_VTOL:
	    strcpy(buf + spaces, "ROTOR DESTROYED");
	    break;
	case MOVE_FLY:
	    strcpy(buf + spaces, "ENGINE DESTROYED");
	    break;
	case MOVE_HULL:
	    strcpy(buf + spaces, "ENGINE ROOM DESTROYED");
	    break;
	case MOVE_SUB:
	    strcpy(buf + spaces, "ENGINE ROOM DESTROYED");
	    break;
	case MOVE_FOIL:
	    strcpy(buf + spaces, "FOIL DESTROYED");
	    break;
	}
	notify(player, buf);
    }
    if (MechStatus(mech) & COMBAT_SAFE) {
	strcpy(buf + spaces, tprintf("%%ch%%cbCOMBAT SAFE%%c"));
	notify(player, buf);
    }
    if (MechCritStatus(mech) & SLITE_DEST) {
	strcpy(buf + spaces, tprintf("%%ch%%crSEARCHLIGHT DESTROYED%%c"));
	notify(player, buf);
    }
    if (MechStatus2(mech) & BOGDOWN) {
	strcpy(buf + spaces, tprintf("%%ch%%cbBOGGED%%c"));
	notify(player, buf);
    }
    if ((MechStatus2(mech) & BEAGLE_ACTIVE) && Started(mech)) {
	strcpy(buf + spaces, tprintf("%%ch%%cbACTIVE PROBE ON%%c"));
	notify(player, buf);
    }
    if (MechCritStatus(mech) & HIDDEN) {
	strcpy(buf + spaces, tprintf("%%ch%%cgHIDDEN%%c"));
	notify(player, buf);
    }
    if (MechStatus(mech) & ECM_DISTURBANCE) {
	strcpy(buf + spaces, tprintf("%%ch%%crAFFECTED BY ECM%%c"));
	notify(player, buf);
    }
    if ((MechStatus(mech) & NARC_ATTACHED) && !(MechStatus2(mech) & INARC_ATTACHED)) {
        strcpy(buf + spaces, tprintf("%%ch%%cbNARC ATTACHED%%c"));
        notify(player, buf);
    }
    if (MechStatus2(mech) & INARC_ATTACHED) {
        strcpy(buf + spaces, tprintf("%%ch%%cbiNARC_ATTACHED%%c"));
        notify(player, buf);
    }
    if (MechCritStatus(mech) & CREW_STUNNED) {
	strcpy(buf + spaces, tprintf("%%ch%%crCREW STUNNED%%c"));
	notify(player, buf);
    }
    if ((MechCritStatus(mech) & TAILROTOR_DAMAGED) && MechType(mech) == CLASS_VTOL) {
	strcpy(buf + spaces, tprintf("%%cg%%cbTAILROTOR DAMAGED%%c"));
	notify(player, buf);
    }
    if ((MechCritStatus2(mech) & HITCH_DESTROYED) && MechType(mech) != CLASS_AERO) {
	strcpy(buf + spaces, tprintf("%%cmHITCH DESTROYED%%c"));
	notify(player, buf);
    }
    if (MechStatus2(mech) & UNJAMMING) {
	strcpy(buf + spaces, tprintf("%%ch%%cyUNJAMMING WEAPON%%c"));
	notify(player, buf);
    }
    if ((temp = (MechIsSwarmed(mech))) == 1) {
	strcpy(buf + spaces, tprintf("%%ch%%cySWARMED BY BATTLESUITS%%c"));
	notify(player, buf);
    } else if (temp == 2) {
	strcpy(buf + spaces, tprintf("%%ch%%cbMOUNTED BY BATTLESUITS%%c"));
	notify(player, buf);
    }
    if (MechSwarmTarget(mech) > 0) {
        tempMech = getMech(MechSwarmTarget(mech));
	if (tempMech) {
            if (MechTeam(mech) != MechTeam(tempMech))
	        strcpy(buf + spaces, tprintf("%%ch%%crSWARMING A BATTLEMECH%%c"));
	    else

	        strcpy(buf + spaces, tprintf("%%ch%%cbMOUNTING A BATTLEMECH%%c"));
	    notify(player, buf);
	    }
	}
    if (MechStatus2(mech) & ECM_PROTECTED) {
	strcpy(buf + spaces, tprintf("%%ch%%cgPROTECTED BY ECM%%c"));
	notify(player, buf);
    }
    if (MechStatus(mech) & JELLIED) {
	strcpy(buf + spaces, tprintf("%%ch%%crINFERNO JELLIED%%c"));
	notify(player, buf);
    }
    if (MechCritStatus(mech) & DUG_IN) {
	if (MechMove(mech) == MOVE_QUAD) {
		strcpy(buf + spaces, tprintf("%%ch%%cgHULL DOWN%%c"));
		notify(player, buf);
        } else {
		strcpy(buf + spaces, tprintf("%%ch%%cgDUG DOWN%%c"));
		notify(player, buf);
	}
    }

    if (MechStatus2(mech) & DODGING) {
	strcpy(buf + spaces, tprintf("%%ch%%crDODGING%%c"));
	notify(player, buf);
	}
    if (MechStatus2(mech) & EVADING) {
	strcpy(buf + spaces, tprintf("%%ch%%crEVADING%%c"));
	notify(player, buf);
	}
    if (SideSlipping(mech)) {
	strcpy(buf + spaces, tprintf("%%ch%%cySIDESLIPPING%%c"));
	notify(player,buf);
	}
    if (MechStatus2(mech) & SPRINTING) {
	strcpy(buf + spaces, tprintf("%%ch%%crSPRINTING%%c"));
	notify(player, buf);
	}
    if (MoveModeChange(mech)) {
	strcpy(buf + spaces, tprintf("%%ch%%cyCHANGING MOVEMENT MODE%%c"));
	notify(player, buf);
	}
    if (IsLit3(mech)) {
	strcpy(buf + spaces, tprintf("%%cySearchlight is on%%c"));
	notify(player, buf);
    } else if (IsLit2(mech)) {
	strcpy(buf + spaces, tprintf("%%cyILLUMINATED%%c"));
	notify(player, buf);
    }

    for (i = 0; temp_flag_info_struct[i].flag; i++)
	if (temp_flag_info_struct[i].infolvl >= level) {
	    if (MechStatus(mech) & temp_flag_info_struct[i].flag) {
		if (temp_flag_info_struct[i].onmsg) {
		    strcpy(buf + spaces, temp_flag_info_struct[i].onmsg);
		    notify(player, buf);
		}
	    } else {
		if (temp_flag_info_struct[i].offmsg) {
		    strcpy(buf + spaces, temp_flag_info_struct[i].offmsg);
		    notify(player, buf);
		}
	    }
	}
}

const char *GetArcID(MECH *mech, int arc)
{
    static char buf[20];
    int hasarm = (MechType(mech) == CLASS_MECH || MechType(mech) == CLASS_MW || MechType(mech) == CLASS_BSUIT);

/*
    if (arc > TURRETARC)
	arc -= TURRETARC;
*/

    if (arc & FORWARDARC)
        strcpy(buf, "Forward");
    else if (arc & RSIDEARC)
        strcpy(buf, hasarm ? "Right Arm" : "Right Side");
    else if (arc & LSIDEARC)
        strcpy(buf, hasarm ? "Left Arm" : "Left Side");
    else if (arc & REARARC)
        strcpy(buf, "Rear");
    else
        strcpy(buf, "NO");

    return buf;
}

const char *GetMechToMechID_base(MECH * see, MECH * mech, int i)
{
    char *mname;
    static char ids[SBUF_SIZE];
    int inlos;

    if (Good_obj(mech->mynum)) {
	if (i < 2 && (i == 1 ||
		!InLineOfSight_NB(see, mech, MechX(mech), MechY(mech),
		    0.0))) {
	    mname = "something";
	    inlos = 0;
	} else {
	    mname = silly_atr_get(mech->mynum, A_MECHNAME);
	    inlos = 1;
	}
    } else
	return "";
    sprintf(ids, "%s [%s]", mname, MechIDS(mech,
	    MechTeam(see) == MechTeam(mech) || !inlos));
    return ids;
}

const char *GetMechToMechID(MECH * see, MECH * mech)
{
    char *mname;
    static char ids[SBUF_SIZE];

    if (Good_obj(mech->mynum)) {
	if (!InLineOfSight_NB(see, mech, MechX(mech), MechY(mech), 0.0))
	    mname = "something";
	else
            mname = silly_atr_get(mech->mynum, A_MECHNAME);
    } else
	return "";
    sprintf(ids, "%s [%s]", mname, MechIDS(mech,
	    MechTeam(see) == MechTeam(mech)));
    return ids;
}

const char *GetMechID(MECH * mech)
{
    char *mname;
    static char ids[SBUF_SIZE];

    if (Good_obj(mech->mynum))
	mname = silly_atr_get(mech->mynum, A_MECHNAME);
    else
	return "";
    sprintf(ids, "%s [%s]", mname, MechIDS(mech, 0));
    return ids;
}

void mech_set_channelfreq(dbref player, void *data, char *buffer)
{
    int chn = -1;
    int freq;
    MECH *mech = (MECH *) data;
    MECH *t;
    MAP *map = getMap(mech->mapindex);
    int i, j;

    if (!map)
	{
	SendError(tprintf("Invalid map! #%d trying to set freq without mapindex.", mech->mynum));
	notify(player, "Cannot change radio information while in a carrier.");
	return;
	}
    /* UH, this is code that _pretends_ it works :-) */
    cch(MECH_MAP);
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    chn = toupper(*buffer) - 'A';
    DOCHECK(chn < 0 || chn >= MFreqs(mech), "Invalid channel-letter!");
    buffer++;
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    DOCHECK(*buffer != '=', "Missing =!");
    buffer++;
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    freq = atoi(buffer);
    DOCHECK(!freq && strcmp(buffer, "0"), "Invalid frequency!");
    DOCHECK(freq < 0, "Are you trying to kid me?");
    DOCHECK(freq > 999999,
	"Invalid frequency - range is from 0 to 999999.");
    notify(player, tprintf("Channel %c set to %d.", 'A' + chn, freq));
    mech->freq[chn] = freq;
/* To catch possible cheating freq-acquireance - DJ */
    if (freq > 0)
        for (i = 0; i < map->first_free; i++) {
        if (!(t = FindObjectsData(map->mechsOnMap[i])))
            continue;
        if (t == mech)
            continue;
        if (MechTeam(t) == MechTeam(mech))
          continue;
        for (j = 0; j < MFreqs(t); j++)
            if (t->freq[j] == freq && !(t->freqmodes[j] & FREQ_SCAN))
	        SendFreqs(tprintf("ALERT: Possible abuse by #%d (Team %d) setting freq %d matching #%d (Team %d)!",
			mech->mynum, MechTeam(mech), freq, t->mynum, MechTeam(t)));
    }
}

void mech_set_channeltitle(dbref player, void *data, char *buffer)
{
    int chn = -1;
    MECH *mech = (MECH *) data;

    cch(MECH_MAP);
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    chn = toupper(*buffer) - 'A';
    DOCHECK(chn < 0 || chn >= MFreqs(mech), "Invalid channel-letter!");
    buffer++;
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    DOCHECK(*buffer != '=', "Missing =!");
    buffer++;
    skipws(buffer);
    if (!*buffer) {
	mech->chantitle[chn][0] = 0;
	notify(player, tprintf("Channel %c title cleared.", 'A' + chn));
	return;
    }
    strncpy(mech->chantitle[chn], buffer, CHTITLELEN);
    mech->chantitle[chn][CHTITLELEN] = 0;
    notify(player, tprintf("Channel %c title set to set to %s.", 'A' + chn,
	    buffer));
}

/*                    1234567890123456 */
const char tempc[] = "xrgybmcwXRGYBMCW";

static char *ccode(MECH * m, int i, int obs, int team)
{
    int t = m->freqmodes[i] / FREQ_REST;
    static char buf[6];
    int ii;

  if (!obs) {
    if (!t) {
	strcpy(buf, "");
	return buf;
    };
    if (t < 9) {
	sprintf(buf, "%%c%c", tempc[t - 1]);
	return buf;
    }
    sprintf(buf, "%%ch%%c%c", ToLower(tempc[t - 1]));
  } else {
    for (ii = 0; ii < 15; ii++) {
	if (team == obs_team_color[ii].team)
     	    sprintf(buf, "%s", obs_team_color[ii].ccode);
    }
    if (buf == NULL)
	sprintf(buf, "%s", obs_team_color[0].ccode);
  }
    return buf;
}

void mech_set_channelmode(dbref player, void *data, char *buffer)
{
    int chn = -1, nm = 0, i;
    MECH *mech = (MECH *) data;
    char buf[SBUF_SIZE];

    cch(MECH_MAP);
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    chn = toupper(*buffer) - 'A';
    DOCHECK(chn < 0 || chn >= MFreqs(mech), "Invalid channel-letter!");
    buffer++;
    skipws(buffer);
    DOCHECK(!*buffer, "Invalid input!");
    DOCHECK(*buffer != '=', "Missing =!");
    buffer++;
    skipws(buffer);
    if (!buffer || !*buffer) {
	mech->freqmodes[chn] = 0;
	notify(player, tprintf("Channel %c <send> mode set to analog.",
		'A' + chn));
	return;
    }
    while (buffer && *buffer) {
	switch (*buffer) {
	case 'D':
	case 'd':
	    DOCHECK(MechType(mech) == CLASS_MW,
		"This unit has no digital equipment.");
	    nm |= FREQ_DIGITAL;
	    break;
	case 'I':
	case 'i':
	    DOCHECK(!(MechRadioInfo(mech) & RADIO_INFO),
		"This unit is unable to use info functionality.");
	    nm |= FREQ_INFO;
	    break;
	case 'U':
	case 'u':
	    nm |= FREQ_MUTE;
	    break;
	case 'E':
	case 'e':
	    DOCHECK(!(MechRadioInfo(mech) & RADIO_RELAY),
		"This unit is unable to relay.");
	    nm |= FREQ_RELAY;
	    break;
	case 'S':
	case 's':
	    DOCHECK(!(MechRadioInfo(mech) & RADIO_SCAN),
		"This unit is unable to scan.");
	    nm |= FREQ_SCAN;
	    break;
	default:
	    for (i = 0; tempc[i]; i++)
		if (*buffer == tempc[i]) {
		    nm = nm % FREQ_REST + FREQ_REST * (i + 1);
		    break;
		}
	    if (!tempc[i])
		buffer = NULL;
	    break;
	}
	if (buffer)
	    buffer++;
    }
    DOCHECK(!(nm & FREQ_DIGITAL) &&
	(nm & FREQ_RELAY),
	"Error: Need digital transfer for relay to work.");
    DOCHECK(!(nm & FREQ_DIGITAL) &&
	(nm & FREQ_INFO),
	"Error: Need digital transfer for transfer info to work.");
    mech->freqmodes[chn] = nm;
    i = 0;

/*   if (nm & FREQ_DIGITAL) buf[i++] = 'D'; */
    if (nm & FREQ_INFO)
	buf[i++] = 'I';
    if (nm & FREQ_MUTE)
	buf[i++] = 'U';
    if (nm & FREQ_RELAY)
	buf[i++] = 'E';
    if (nm & FREQ_SCAN)
	buf[i++] = 'S';
    if (!i)
	buf[i++] = '-';
    if (nm / FREQ_REST) {
	sprintf(buf + i, "/color:%c", tempc[nm / FREQ_REST - 1]);
	i = strlen(buf);
    }
    buf[i] = 0;
    notify(player, tprintf("Channel %c <send> mode set to %s (flags:%s).",
	    'A' + chn, nm & FREQ_DIGITAL ? "digital" : "analog", buf));
}

void mech_list_freqs(dbref player, void *data, char *buffer)
{
    int i;
    MECH *mech = (MECH *) data;

    /* UH, this is code that _pretends_ it works :-) */
    notify(player, "# -- Mode -- Frequency -- Comtitle");
    for (i = 0; i < MFreqs(mech); i++)
	notify(player, tprintf("%c    %c%c%c%c    %-9d    %s", 'A' + i,
		mech->freqmodes[i] & FREQ_DIGITAL ? 'D' : 'A',
		mech->freqmodes[i] & FREQ_RELAY ? 'R' : '-',
		mech->freqmodes[i] & FREQ_MUTE ? 'M' : '-',
		mech->
		freqmodes[i] & FREQ_SCAN ? 'S' : mech->freqmodes[i] >=
		FREQ_REST ? tempc[mech->freqmodes[i] / FREQ_REST -
		    1] : mech->freqmodes[i] & FREQ_INFO ? 'I' : '-',
		mech->freq[i], mech->chantitle[i]));
}

void mech_sendchannel(dbref player, void *data, char *buffer)
{
    /* Basically, this is sorta routine 'sendchannel <letter>=message' code */
    MECH *mech = (MECH *) data;
    int fail = 0;
    int argc;
    int chn = 0;
    char *args[3];
    int i;

    cch(MECH_USUALS);
    DOCHECK(Destroyed(mech) ||
	!MechRadioRange(mech), "Your communication gear is inoperative.");
/*    DOCHECK(MechCritStatus(mech) & OBSERVATORIC,
	"You can't use the radios."); */
    if ((argc = silly_parseattributes(buffer, args, 3)) != 3)
	fail = 1;
    if (!fail && strlen(args[0]) > 1)
	fail = 1;
    if (!fail && args[0][0] >= 'a' && args[0][0] <= 'z')
	chn = args[0][0] - 'a';
    if (!fail && args[0][0] >= 'A' && args[0][0] <= 'Z')
	chn = args[0][0] - 'Z';
    if (!fail && (chn >= MFreqs(mech) || chn < 0))
	fail = 1;
    if (!fail)
	for (i = 0; args[2][i]; i++)
	    DOCHECK((BOUNDED(32, args[2][i], 255)) != args[2][i],
		"Invalid: No control characters in radio messages, please.");
    DOCHECK(fail, "Invalid format! Usage: sendchannel <letter>=<string>");
    /* hrm, here I ought to put real channel code in. sorta lazy, though */
    sendchannelstuff(mech, chn, args[2]);
    explode_mines(mech, mech->freq[chn]);
}

#define number(x,y) (rand()%(y-x+1)+x)

static void do_scramble(char *buffo, int ch, int bth)
{
    int i;

    for (i = 0; buffo[i]; i++) {
	if (number(1, 100) > ch && Roll() < (bth + 5)) {
	    if (number(1, 2) == 1)
		buffo[i] -= number(1, 10);
	    else
		buffo[i] += number(1, 10);
	}
	buffo[i] = (unsigned char) BOUNDED(33, buffo[i], 255);
    }
}

#define my_modify(n,fact) (100 - (100 - (n)) / (fact))

static int comm_num_to_conn;
static int comm_is[MAX_MECHS_PER_MAP][MAX_MECHS_PER_MAP];
static int comm_done[MAX_MECHS_PER_MAP];
static MECH *comm_mech[MAX_MECHS_PER_MAP];
static int comm_best;
static int comm_best_path[MAX_MECHS_PER_MAP];
static int comm_path[MAX_MECHS_PER_MAP];

void ScrambleMessage(char *buffo, int range, int sendrange, int recvrrange,
    char *handle, char *msg, int bth, int *isxp, int under_ecm,
    int digmode)
{
    int mr, i;
    char *header = NULL;
    char buf[LBUF_SIZE];

    *isxp = 0;

    if (digmode > 1 && comm_best > 1) {
	int bearing;

	strncpy(buf, "{R-path:", LBUF_SIZE);
	for (i = 1; i < comm_best; i++) {
	    if (i > 1)
		strcat(buf, "/");
	    bearing =
		FindBearing(MechFX(comm_mech[comm_best_path[i]]),
		MechFY(comm_mech[comm_best_path[i]]),
		MechFX(comm_mech[comm_best_path[i - 1]]),
		MechFY(comm_mech[comm_best_path[i - 1]]));
	    snprintf(buf + strlen(buf), LBUF_SIZE, "[%c%c]-h:%d",
		MechID(comm_mech[comm_best_path[i]])[0],
		MechID(comm_mech[comm_best_path[i]])[1], bearing);
	}
	strcat(buf, "} ");
	header = buf;
    }
    if (handle && *handle)
	snprintf(buffo, LBUF_SIZE, "%s<%s> %s", header ? header : "", handle, msg);
    else
	snprintf(buffo, LBUF_SIZE, "%s%s", header ? header : "", msg);
    if ((!digmode && (range >= sendrange || range >= recvrrange)) ||
	under_ecm) {
	if (!digmode) {

	    mr = MAX(recvrrange, (sendrange + recvrrange) / 2);
	    if (sendrange < range) {
		do_scramble(buffo, (100 * sendrange) / MAX(1, range), bth);
		*isxp = 1;
	    }
	    if (mr < range) {
		do_scramble(buffo, my_modify((100 * mr) / MAX(1, range),
			2), bth);
		*isxp = 1;
	    }
	}
	if (under_ecm && range >= 1) {
	    do_scramble(buffo, Number(30, 50), bth);
	    *isxp = 1;
	}
    }
}

int common_checks(dbref player, MECH * mech, int flag)
{
    MAP *mech_map;

    if (!mech)
	return 0;

    if ((flag & MECH_CONSISTENT) && !CheckData(player, mech))
	return 0;

//    if (!Wizard(player)) {
	/* ----------------------------- */
	/* INSERT UNSUPPORTED TYPES HERE */
	/* ----------------------------- */

//	if (MechType(mech) == CLASS_AERO)
//	    return 0;
	/* ----------------------------- */
//    }

    MechLastUse(mech) = 0;

    if (flag & MECH_STARTED) {
	DOCHECK0(Destroyed(mech), "You are destroyed!");
	DOCHECK0(!(MechStatus(mech) & STARTED), "Reactor is not online!");
    }

    if (flag & MECH_PILOT) {
	DOCHECK0(MechStatus(mech) & BLINDED,
	    "You are momentarily blinded!");
    }

    if (flag & MECH_PILOT_CON)
	DOCHECK0(Uncon(mech) && (!Started(mech) ||
		player == MechPilot(mech)),
	    "You are unconscious....zzzzzzz");

    if (flag & MECH_PILOTONLY)
    	DOCHECK0(!Wizard(player) && (IsDS(mech) || In_Character(mech->mynum)) && MechPilot(mech) != player,
    	    "Now now, only the pilot can push that button.");

    if (flag & MECH_MAP) {
	DOCHECK0(mech->mapindex < 0, "You are on no map!");
	mech_map = getMap(mech->mapindex);
	if (!mech_map) {
	    notify(player, "You are on an invalid map! Map index reset!");
	    mech_shutdown(player, (void *) mech, "");
	    mech->mapindex = -1;
	    return 0;
	}
    }
    return 1;
}

static int iter_prevent;

void recursive_commlink(int i, int dep)
{
    int j;

    if (iter_prevent++ >= 10000)
	return;
    if (dep >= comm_best)
	return;
    comm_path[dep] = i;
    for (j = 1; j < comm_num_to_conn; j++)
	if (comm_is[i][j] && !comm_done[j]) {
	    if (j == (comm_num_to_conn - 1)) {
		int k;

		comm_best = dep;
		for (k = 0; k < comm_best; k++)
		    comm_best_path[k] = comm_path[k];
	    } else {
		comm_done[j] = 1;
		recursive_commlink(j, dep + 1);
		comm_done[j] = 0;
	    }
	}
}

void nonrecursive_commlink(int i)
{
    int dep = 0, j;
    int comm_loop[MAX_MECHS_PER_MAP];
    int iter_c = 0;
    int maxdepth = 0;

    /* May _still_ contain fatal bug ; Ghod knows (I don't) */
    comm_loop[0] = 1;
    comm_path[0] = i;

    while (dep >= 0) {
	i = comm_path[dep];
	for (j = comm_loop[dep]; j < comm_num_to_conn; j++)
	    if (comm_is[i][j] && !comm_done[j]) {
		if (j == (comm_num_to_conn - 1)) {
		    int k;

		    comm_best = dep + 1;
		    for (k = 0; k < comm_best; k++)
			comm_best_path[k] = comm_path[k];
		    j = comm_num_to_conn;
		    break;
		} else if ((dep + 1) < comm_best) {
		    comm_done[j] = 1;
		    comm_loop[dep++] = j + 1;
		    comm_loop[dep] = 1;
		    comm_path[dep] = j;
		    if (dep > maxdepth)
			maxdepth = dep;
		    break;
		}
	    }
	if (j == comm_num_to_conn) {
	    if (dep > 0)
		comm_done[comm_loop[--dep] - 1] = 0;
	    else
		dep--;		/* We're finished! */
	}
	if (iter_c++ == 100000) {
	    SendError(tprintf
		("#%d: Infinite loop in relay code (?) ; using backup recursive code (num_mechs:%d, maxdepth:%d, nowdepth:%d)",
		    comm_mech[0]->mynum, comm_num_to_conn, maxdepth, dep));
	    comm_best = 9999;
	    for (i = 0; i < comm_num_to_conn; i++)
		comm_done[i] = 0;
	    iter_prevent = 0;
	    recursive_commlink(0, 0);
	    return;
	}
    }
}


int findCommLink(MAP * map, MECH * from, MECH * to, int freq)
{
    int i, j;
    MECH *t;

    comm_num_to_conn = 0;
    comm_mech[comm_num_to_conn++] = from;
    for (i = 0; i < map->first_free; i++) {
	if (!(t = FindObjectsData(map->mechsOnMap[i])))
	    continue;
	if (t == from || t == to)
	    continue;
/* Removed to allow relaying any message on a chan rather than just friendly. - DJ */
/*	if (MechTeam(from) != MechTeam(t)) */
/*	    continue; */
	if ((MechMove(t) != MOVE_NONE && !Started(t)) ||
	    (MechMove(t) == MOVE_NONE && Destroyed(t)))
	    continue;
	if (!(MechRadioInfo(t) & RADIO_RELAY))
	    continue;
	for (j = 0; j < MFreqs(t); j++)
	    if (t->freq[j] == freq)
		if (t->freqmodes[j] & FREQ_RELAY) {
		    comm_mech[comm_num_to_conn++] = t;
		    continue;
		}
    }
    comm_mech[comm_num_to_conn++] = to;
    if (comm_num_to_conn == 2)
	return 0;		/* Quickie kludge for the 'standard' case */
    for (i = 0; i < comm_num_to_conn; i++) {
	comm_done[i] = 0;
	comm_is[i][i] = 0;
	for (j = i + 1; j < comm_num_to_conn; j++) {
	    float range = FlMechRange(map, comm_mech[i], comm_mech[j]);

	    comm_is[i][j] = (range <= MechRadioRange(comm_mech[i]));
	    comm_is[j][i] = (range <= MechRadioRange(comm_mech[j]));
	}
    }
    comm_best = 9999;
    /*  recursive_commlink(0, 0);  */
    nonrecursive_commlink(0);	/* better _pray_ this works */
    return comm_best != 9999;
}

void sendchannelstuff(MECH * mech, int freq, char *msg)
{
    /* The _smart_ code :-) */
    int loop, range, bearing, i, isxp, realrange, obs = 0;
    MECH *tempMech;
    MAP *mech_map;
    char buf[LBUF_SIZE];
    char buf2[LBUF_SIZE];
    char buf3[LBUF_SIZE];

    if (!MechRadioRange(mech))
	return;

    mech_map = getMap(mech->mapindex);
 
    for (loop = 0; loop < mech_map->first_free; loop++) {
	if (mech_map->mechsOnMap[loop] != -1) {
	    if (!(tempMech = (MECH *)
		    FindObjectsData(mech_map->mechsOnMap[loop])))
		continue;
	    if (Destroyed(tempMech))
		continue;
	    obs = (MechCritStatus(tempMech) & OBSERVATORIC);
	    range = FaMechRange(mech, tempMech);
	    bearing =
		FindBearing(MechFX(tempMech), MechFY(tempMech),
		MechFX(mech), MechFY(mech));
	    for (i = 0; i < MFreqs(tempMech); i++) {
		if (tempMech->freq[i] == mech->freq[freq]) {
		    if (tempMech->freqmodes[i] & FREQ_MUTE)
			continue;
		    break;
		}
	    }
	    if (obs) {
		i = 0;
		realrange = range;
		range = 1;
		}
	    if (i >= MFreqs(tempMech)) {
		/* Possible scanner check */
		if (!(mech->freqmodes[freq] & FREQ_DIGITAL))
		    if ((MechRadioInfo(tempMech) & RADIO_SCAN) &&
			mech->freq[freq]) {
			int tnc = 0;

			for (i = 0; i < MFreqs(tempMech); i++)
			    if (tempMech->freqmodes[i] & FREQ_SCAN) {
				int l = strlen(msg), t;
				int mod, diff;
				int pr;

				/* Possible skill check here? Nah. */

				/* Chance of detection: 1 in MIN(80,l) out of 100 */
				if (Number(1, 100) > MIN(80, l))
				    continue;

				if (!tnc++)
				    mech_notify(tempMech, MECHALL,
					"You notice an unknown transmission on your scanner.. ");
				if (tempMech->freq[i] < mech->freq[freq]) {
				    diff =
					mech->freq[freq] -
					tempMech->freq[i];
				    mod = 1;
				} else {
				    diff =
					tempMech->freq[i] -
					mech->freq[freq];
				    mod = -1;
				}
				t =
				    MAX(1, Number(1, MIN(99,
					    l)) * diff / 100);
				pr = t * 100 / diff;
				mech_notify(tempMech, MECHALL,
				    tprintf
				    ("Your systems manage to zero on it %s on channel %c.",
				 pr < 30 ? "somewhat" : pr <
					60 ? "fairly well" : pr <
					95 ? "precisely" : "exactly",
					i + 'A'));
				t = (MIN(t, abs(tempMech->freq[i] - mech->freq[freq])));
				tempMech->freq[i] += mod * t;
				if (tempMech->freq[i] == mech->freq[freq] && mech->freq[freq] > 0)
				    SendFreqs(tprintf("ScanFreqHit of %d by #%d from source %d",
					mech->freq[freq], tempMech->mynum, mech->mynum));
			    }
		    }
		continue;
	    }
	    strncpy(buf2, msg, LBUF_SIZE);
	    if (MechAuto(tempMech) > 0 && tempMech->freq[i]) {
		AUTO *a = (AUTO *) FindObjectsData(MechAuto(tempMech));
		if (!a) {
		    MechAuto(tempMech) = -1;
		} else if (a && Location(a->mynum) != tempMech->mynum) {
		    SendAI(tprintf("Autopilot #%d reported on Mech #%d but not in proper location?", a->mynum, tempMech->mynum));
		} else if (a && !(MechStatus(tempMech) & ECM_DISTURBANCE)) {
		    strncpy(buf3, msg, LBUF_SIZE);
		    auto_parse_command(a, tempMech, i, buf3);
		}
	    }
	    if (!MechRadioRange(tempMech))
		continue;
	    if (mech->freqmodes[freq] & FREQ_DIGITAL) {
		if (range > MechRadioRange(mech)) {
		    if (!findCommLink(mech_map, mech, tempMech,
			    mech->freq[freq])) continue;
		} else
		    comm_best = 1;
/* Commented due to LoS ECM blocking. Want people to be aware they are blocked. - DJ */
/*		if (tempMech != mech) {
		    if (MechStatus(mech) & ECM_DISTURBANCE)
			continue;
		    else if (MechStatus(tempMech) & ECM_DISTURBANCE)
			continue;
		    else if (mech_map->LOSinfo[mech->mapnumber][tempMech->mapnumber] & MECHLOSFLAG_ECM)
			continue;
		} */
		ScrambleMessage(buf3, range, MechRadioRange(mech),
		    MechRadioRange(mech), mech->chantitle[freq], buf2,
		    MechComm(tempMech), &isxp, !obs && ((MechStatus(mech) & ECM_DISTURBANCE) ||
                        (MechStatus(tempMech) & ECM_DISTURBANCE) || (mech_map->LOSinfo[mech->mapnumber][tempMech->mapnumber] & MECHLOSFLAG_ECM) ||
			StealthAndECM(mech) || StealthAndECM(tempMech))
                        && mech != tempMech, (tempMech->freqmodes[i] & FREQ_INFO) ? 2 : 1);

		if (comm_best >= 2)
		    bearing =
			FindBearing(MechFX(tempMech), MechFY(tempMech),
			MechFX(comm_mech[comm_best_path[comm_best - 1]]),
			MechFY(comm_mech[comm_best_path[comm_best - 1]]));
		if (!obs)
		    snprintf(buf, LBUF_SIZE, "%s[%c:%d] %s%%c", ccode(tempMech, i, obs, MechTeam(mech)),
		        (char) ('A' + i), bearing, buf3);
		else {
/*			sprintf(buf, "%s[%c:%d] <%d> %s%%c", ccode(tempMech, i, obs, MechTeam(mech)),
			(char) ('A' + i), bearing, mech->freq[freq], buf3); */
		    snprintf(buf, LBUF_SIZE, "%s[%c:%d] <%s> %s%%c", ccode(tempMech, i, obs, MechTeam(mech)),
		        (char) ('A' + i), bearing, silly_atr_get(mech->mynum, A_FACTION), buf3);
		}
	    } else {
		ScrambleMessage(buf3, range, MechRadioRange(mech),
		    MechRadioRange(tempMech), mech->chantitle[freq], buf2,
		    MechComm(tempMech), &isxp,
		    !obs && ((MechStatus(mech) & ECM_DISTURBANCE) ||
			(MechStatus(tempMech) & ECM_DISTURBANCE) ||
		        (mech_map->LOSinfo[mech->mapnumber][tempMech->mapnumber] & MECHLOSFLAG_ECM))
			&& mech != tempMech, 0);
		if (!obs)
		    snprintf(buf, LBUF_SIZE, "%s(%c:%d) %s%%c", ccode(tempMech, i, obs, MechTeam(mech)),
		        (char) ('A' + i), bearing, buf3);
		else {
/*			sprintf(buf, "%s(%c:%d) <%d> %s%%c", ccode(tempMech, i, obs, MechTeam(mech)),
			(char) ('A' + i), bearing, mech->freq[freq], buf3); */
		    snprintf(buf, LBUF_SIZE, "%s(%c:%d) <%s> %s%%c", ccode(tempMech, i, obs, MechTeam(mech)),
		        (char) ('A' + i), bearing, silly_atr_get(mech->mynum, A_FACTION), buf3);
		}
	    }
	    mech_notify(tempMech, MECHALL, buf);
	    if (isxp && In_Character(tempMech->mynum))
		if ((MechCommLast(tempMech) + 60) < event_tick) {
		    AccumulateCommXP(MechPilot(tempMech), tempMech);
		    MechCommLast(tempMech) = event_tick;
		}
	}
    }
}

void mech_radio(dbref player, void *data, char *buffer)
{
    int argc;
    int fail = 0;
    char *args[3];
    MECH *mech = (MECH *) data;
    dbref target;
    MECH *tempMech;
    char radbuf[LBUF_SIZE];

    /* radio <id>=message */
    /* Quick clone :-) */
    /* This is silly, but who cares. */
    cch(MECH_USUAL);

    DOCHECK(MechCritStatus(mech) & OBSERVATORIC || MechNoRadio(mech),
	"You can't radio anyone.");
    if ((argc = silly_parseattributes(buffer, args, 3)) != 3)
	fail = 1;
    if (!fail && (!args[1] || args[1][0] != '=' || args[1][1] != 0))
	fail = 1;
    if (!fail && (!args[0] || args[0][0] == 0 || args[0][1] == 0 ||
	    args[0][2] != 0))
	fail = 1;

    snprintf(radbuf, LBUF_SIZE * sizeof(char), "%s%%cn", args[2]);
    if (!fail) {
	target = FindTargetDBREFFromMapNumber(mech, args[0]);
	tempMech = getMech(target);
	DOCHECK(!tempMech ||
	    !InLineOfSight(mech, tempMech, MechX(tempMech),
 MechY(tempMech), FlMechRange(map, mech, tempMech)),
	    "Target is not in line of sight!");
	if (!MechNoRadio(tempMech)) {
	    mech_notify(mech, MECHSTARTED, tprintf("You radio %s with, '%s'",
		GetMechToMechID(mech, tempMech), radbuf));
	    mech_notify(tempMech, MECHSTARTED,
	        tprintf("%s radios you with, '%s'", GetMechToMechID(tempMech,
		    mech), radbuf));
	    auto_replyA(tempMech, tprintf("%s radio'ed me '%s'",
		GetMechToMechID(tempMech, mech), radbuf));
	} else {
	    mech_notify(tempMech, MECHSTARTED, tprintf("%s tries to radio you but your comgear filters it out.", GetMechToMechID(tempMech, mech)));
	    mech_notify(mech, MECHSTARTED, "Your target filters out your comgear.");
	    auto_replyA(tempMech, tprintf("%s radio'ed me but I filtered it out", GetMechToMechID(tempMech, mech)));
	}
    }
    DOCHECK(fail,
	"Invalid format! Usage: radio <letter><letter>=<message>");
}

void MechBroadcast(MECH * mech, MECH * target, MAP * mech_map,
    char *buffer)
{
    int loop;
    MECH *tempMech;

    if (target) {
	for (loop = 0; loop < mech_map->first_free; loop++) {
	    if (mech_map->mechsOnMap[loop] != mech->mynum &&
		mech_map->mechsOnMap[loop] != -1 &&
		mech_map->mechsOnMap[loop] != target->mynum) {
		tempMech = (MECH *)
		    FindObjectsData(mech_map->mechsOnMap[loop]);
		if (tempMech)
		    mech_notify(tempMech, MECHSTARTED, buffer);
	    }
	}
    } else {
	for (loop = 0; loop < mech_map->first_free; loop++) {
	    if (mech_map->mechsOnMap[loop] != mech->mynum &&
		mech_map->mechsOnMap[loop] != -1) {
		tempMech = (MECH *)
		    FindObjectsData(mech_map->mechsOnMap[loop]);
		if (tempMech)
		    mech_notify(tempMech, MECHSTARTED, buffer);
	    }
	}
    }
}

void MechLOSBroadcast(MECH * mech, char *message)
{
    /* Sends msg to everyone except the mech */
    int i;
    MECH *tempMech;
    MAP *mech_map = getMap(mech->mapindex);
    char buf[LBUF_SIZE];

    possibly_see_mech(mech);
    if (!mech_map)
	return;
    for (i = 0; i < mech_map->first_free; i++)
	if (mech_map->mechsOnMap[i] != -1 &&
	    mech_map->mechsOnMap[i] != mech->mynum) if ((tempMech =
		    getMech(mech_map->mechsOnMap[i])))
		if (InLineOfSight(tempMech, mech, MechX(mech), MechY(mech),
			FlMechRange(mech_map, tempMech, mech))) {
		    sprintf(buf, "%s%s%s", GetMechToMechID(tempMech, mech),
			*message != '\'' ? " " : "", message);
		    mech_notify(tempMech, MECHSTARTED, buf);
		}
}

int MechSeesHexF(MECH * mech, MAP * map, float x, float y, int ix, int iy)
{
    return (InLineOfSight(mech, NULL, ix, iy, FindRange(MechFX(mech),
		MechFY(mech), MechFZ(mech), x, y, ZSCALE * Elevation(map,
		    ix, iy))));
}

int MechSeesHex(MECH * mech, MAP * map, int x, int y)
{
    float fx, fy;

    MapCoordToRealCoord(x, y, &fx, &fy);
    return MechSeesHexF(mech, map, fx, fy, x, y);
}

void HexLOSBroadcast(MAP * mech_map, int x, int y, char *message)
{
    int i;
    MECH *tempMech;
    float fx, fy;

    /* substitution:
       $h = !alarming ('your hex', '%d,%d')
       $H = alarming ('YOUR HEX', '%d,%d (%.2f away)')
     */
    if (!mech_map)
	return;
    MapCoordToRealCoord(x, y, &fx, &fy);
    for (i = 0; i < mech_map->first_free; i++)
	if (mech_map->mechsOnMap[i] != -1)
	    if ((tempMech = getMech(mech_map->mechsOnMap[i])))
		if (MechSeesHexF(tempMech, mech_map, fx, fy, x, y)) {
		    char tbuf[LBUF_SIZE];
		    char *c, *d = tbuf;
		    int done;

		    for (c = message; *c; c++) {
			done = 0;
			if (*c == '$') {
			    if (*(c + 1) == 'h' || *(c + 1) == 'H') {
				c++;
				if (*c == 'h') {
				    if (x == MechX(tempMech) &&
					y == MechY(tempMech)) strcpy(d,
					    "your hex");
				    else
					sprintf(d, "%d,%d", x, y);
				    while (*d)
					d++;
				} else {
				    /* Dangerous */
				    if (x == MechX(tempMech) &&
					y == MechY(tempMech)) strcpy(d,
					    "%ch%crYOUR HEX%cn");
				    else
					sprintf(d, "%%ch%%cy%d,%d%%cn", x,
					    y);
				    while (*d)
					d++;
				}
				done = 1;
			    } else
				c--;	/* To prevent trailing $ from munging game */
			}
			if (!done)
			    *(d++) = *c;
		    }
#if 0
		    *d = 0;
#else
		    if (*(d-1) == '$')
			d--;
		    *d = '\0'; 
#endif
		    mech_notify(tempMech, MECHSTARTED, tbuf);
		}
}

void MechLOSBroadcasti(MECH * mech, MECH * target, char *message)
{
    /* Sends msg to everyone except the mech */
    int i, a, b;
    char oddbuff[LBUF_SIZE];
    char oddbuff2[LBUF_SIZE];
    MECH *tempMech;
    MAP *mech_map = getMap(mech->mapindex);

    if (!mech_map)
	return;
    possibly_see_mech(mech);
    possibly_see_mech(target);
    for (i = 0; i < mech_map->first_free; i++)
	if (mech_map->mechsOnMap[i] != -1 &&
	    mech_map->mechsOnMap[i] != mech->mynum &&
	    mech_map->mechsOnMap[i] != target->mynum)
	    if ((tempMech = getMech(mech_map->mechsOnMap[i]))) {
		a =
		    InLineOfSight(tempMech, mech, MechX(mech), MechY(mech),
		    FlMechRange(mech_map, tempMech, mech));
		b =
		    InLineOfSight(tempMech, target, MechX(target),
		    MechY(target), FlMechRange(mech_map, tempMech,
			target));
		if (a || b) {
		    sprintf(oddbuff, message, b ? GetMechToMechID(tempMech,
			    target) : "someone");
		    sprintf(oddbuff2, "%s%s%s",
			a ? GetMechToMechID(tempMech, mech) : "Someone",
			*oddbuff != '\'' ? " " : "", oddbuff);
		    mech_notify(tempMech, MECHSTARTED, oddbuff2);
		}
	    }
}

void MapBroadcast(MAP * map, char *message)
{
    /* Sends msg to everyone except the mech */
    int i;
    MECH *tempMech;

    for (i = 0; i < map->first_free; i++)
	if (map->mechsOnMap[i] != -1)
	    if ((tempMech = getMech(map->mechsOnMap[i])))
		mech_notify(tempMech, MECHSTARTED, message);
}

void MechFireBroadcast(MECH * mech, MECH * target, int x, int y,
    MAP * mech_map, char *weapname, int IsHit)
{
    int loop, attacker, defender;
    float fx, fy, fz;
    int mapx, mapy;
    MECH *tempMech;
    char buff[50];

    possibly_see_mech(mech);
    if (target) {
	possibly_see_mech(target);
	mapx = MechX(target);
	mapy = MechY(target);
	fx = MechFX(target);
	fy = MechFY(target);
	fz = MechFZ(target);
	for (loop = 0; loop < mech_map->first_free; loop++)
	    if (mech_map->mechsOnMap[loop] != mech->mynum &&
		mech_map->mechsOnMap[loop] != -1 &&
		mech_map->mechsOnMap[loop] != target->mynum) {
		attacker = 0;
		defender = 0;
		tempMech = (MECH *)
		    FindObjectsData(mech_map->mechsOnMap[loop]);
		if (!tempMech)
		    continue;
		if (InLineOfSight(tempMech, mech, MechX(mech), MechY(mech),
			FlMechRange(mech_map, tempMech, mech)))
		    attacker = 1;
		if (target) {
		    if (InLineOfSight(tempMech, target, mapx, mapy,
			    FlMechRange(mech_map, tempMech, target)))
			defender = 1;
		} else if (InLineOfSight(tempMech, target, mapx, mapy,
			FindRange(MechFX(tempMech), MechFY(tempMech),
			    MechFZ(tempMech), fx, fy, fz)))
		    defender = 1;

		if (!attacker && !defender)
		    continue;
		if (defender)
		    sprintf(buff, "%s", GetMechToMechID(tempMech, target));
		if (attacker) {
		    if (defender)
			mech_notify(tempMech, MECHSTARTED,
			    tprintf("%s %s %s with a %s",
				GetMechToMechID(tempMech, mech),
				IsHit ? "hits" : "misses", buff,
				weapname));
		    else
			mech_notify(tempMech, MECHSTARTED,
			    tprintf("%s fires a %s at something!",
				GetMechToMechID(tempMech, mech),
				weapname));
		} else
		    mech_notify(tempMech, MECHSTARTED,
			tprintf("Something %s %s with a %s",
			    IsHit ? "hits" : "misses", buff, weapname));
	    }
    } else {
	mapx = x;
	mapy = y;
	MapCoordToRealCoord(x, y, &fx, &fy);
	fz = ZSCALE * Elevation(mech_map, x, y);
	sprintf(buff, "hex %d %d!", mapx, mapy);
	for (loop = 0; loop < mech_map->first_free; loop++)
	    if (mech_map->mechsOnMap[loop] != mech->mynum &&
		mech_map->mechsOnMap[loop] != -1) {
		attacker = 0;
		defender = 0;
		tempMech = (MECH *)
		    FindObjectsData(mech_map->mechsOnMap[loop]);
		if (!tempMech)
		    continue;
		if (InLineOfSight(tempMech, mech, MechX(mech), MechY(mech),
			FlMechRange(mech_map, tempMech, mech)))
		    attacker = 1;
		if (target) {
		    if (InLineOfSight(tempMech, target, mapx, mapy,
			    FlMechRange(mech_map, tempMech, target)))
			defender = 1;
		} else if (InLineOfSight(tempMech, target, mapx, mapy,
			FindRange(MechFX(tempMech), MechFY(tempMech),
			    MechFZ(tempMech), fx, fy, fz)))
		    defender = 1;
		if (!attacker && !defender)
		    continue;
		if (attacker) {
		    if (defender)	/* att + def */
			mech_notify(tempMech, MECHSTARTED,
			    tprintf("%s fires a %s at %s",
				GetMechToMechID(tempMech, mech), weapname,
				buff));
		    else	/* att */
			mech_notify(tempMech, MECHSTARTED,
			    tprintf("%s fires a %s at something!",
				GetMechToMechID(tempMech, mech),
				weapname));
		} else		/* def */
		    mech_notify(tempMech, MECHSTARTED,
			tprintf("Something fires a %s at %s", weapname,
			    buff));
	    }
    }
}

void mech_notify(MECH * mech, int type, char *buffer)
{
    int i;

    if (Uncon(mech))
	return;
    if (Blinded(mech))
	return;
    if (mech->mynum < 0)
	return;
    /* Let's do colorization too, just in case. */
    if (type == MECHPILOT) {
	if (GotPilot(mech))
	    notify(MechPilot(mech), buffer);
	else
	    mech_notify(mech, MECHALL, buffer);
    } else if ((type == MECHALL && !Destroyed(mech)) ||
	(type == MECHSTARTED && Started(mech))) {
	notify_except(mech->mynum, NOSLAVE, mech->mynum, buffer);
	for (i = 0; i < NUM_TURRETS; i++)
	    if (AeroTurret(mech, i) > 0)
		notify_except(AeroTurret(mech, i), NOSLAVE, AeroTurret(mech, i), buffer);
    }
}

#if 0

#undef notify_except
#undef notify
#define notify raw_notify

void mech_notify_noc(MECH * mech, int type, char *buffer)
{
    int i;

    if (Uncon(mech))
	return;
    if (Blinded(mech))
	return;
    if (mech->mynum < 0)
	return;
    /* Let's do colorization too, just in case. */
    if (type == MECHPILOT) {
	if (GotPilot(mech))
	    notify(MechPilot(mech), buffer);
	else
	    mech_notify_noc(mech, MECHALL, buffer);
    } else if ((type == MECHALL && !Destroyed(mech)) ||
	(type == MECHSTARTED && Started(mech))) {
	notify_except(mech->mynum, NOSLAVE, mech->mynum, buffer);
	for (i = 0; i < NUM_TURRETS; i++)
	    if (AeroTurret(mech, i) > 0)
		notify_except(AeroTurret(mech, i), NOSLAVE, AeroTurret(mech, i), buffer);
    }
}
#endif
