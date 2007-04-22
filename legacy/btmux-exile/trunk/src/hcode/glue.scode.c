#include <stdio.h>
#include <string.h>

#include "mech.h"
#include "glue.h"
#include "extern.h"
#include "coolmenu.h"
#include "mycool.h"
#include "turret.h"
#include "mech.custom.h"
#include "scen.h"
#include "p.template.h"
#include "mech.tech.h"
#include "p.mech.tech.h"
#include "p.mech.utils.h"
#include "p.mech.partnames.h"
#include "p.econ.h"
#include "p.econ_cmds.h"
#include "p.map.obj.h"
#include "p.mech.tech.damages.h"
#include "p.mech.status.h"
#include "p.mech.sensor.h"
#include "p.aero.bomb.h"
#include "p.mech.tech.do.h"
#include "p.mech.build.h"
#include "p.functions.h"
#include "p.mechrep.h"
#include "p.map.h"
#include "p.mech.los.h"
#include "p.btechstats.h"
#include "p.mech.restrict.h"
#include "p.mech.combat.h"
#include "p.mech.consistency.h"
#include "p.mech.tech.commands.h"
#include "map.h"

extern int damage_last;
extern SpecialObjectStruct SpecialObjects[];
dbref match_thing(dbref player, char *name);
char *mechref_path(char *id);
char *setarmorstatus_func(MECH * mech, char *sectstr, char *typestr, char *valuestr);

typedef struct {
    int gtype;
    char *name;
    int rel_addr;
    int type;
    int size;
} GMV;

static MECH tmpm;
static MAP tmpmap;
static TURRET_T tmpturret;
static CUSTOM tmpcustom;
static SCEN tmpscen;
static SSIDE tmpsside;
static SSOBJ tmpssobj;

enum { TYPE_STRING, TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_FLOAT,
    TYPE_DBREF, TYPE_STRFUNC, TYPE_STRFUNC_S, TYPE_BV, TYPE_STRFUNC_BD, TYPE_USHORT,
    TYPE_CBV, TYPE_CHAR_RO, TYPE_SHORT_RO, TYPE_INT_RO, TYPE_FLOAT_RO, TYPE_DBREF_RO,
    TYPE_LAST_TYPE
};

static int scode_in_out[TYPE_LAST_TYPE] =
/*st ch sh in fl db sf sfs bv sfb us cb cr sr ir fr dr */
 { 3, 3, 3, 3, 3, 3, 1, 2,  3, 3,  3, 3, 1, 1, 1, 1, 1};

#define Uglie(dat) ((int) &dat(&tmpm)) - ((int) &tmpm)
#define UglieV(dat,val) ((int) &dat(&tmpm,val)) - ((int) &tmpm)

#define MeEntry(Name,Func,Type) \
{GTYPE_MECH,Name,Uglie(Func),Type,0}
#define MeEntryS(Name,Func,Type,Size) \
{GTYPE_MECH,Name,Uglie(Func),Type,Size}

#define MeVEntry(Name,Func,Val,Type) \
{GTYPE_MECH,Name,UglieV(Func,Val),Type,0}

#define UglieM(dat) ((int) &tmpmap.dat) - ((int) &tmpmap)
#define MaEntry(Name,Func,Type) \
{GTYPE_MAP,Name,UglieM(Func),Type,0}
#define MaEntryS(Name,Func,Type,Size) \
{GTYPE_MAP,Name,UglieM(Func),Type,Size}

#define UglieT(dat) ((int) &tmpturret.dat) - ((int) &tmpturret)
#define TuEntry(Name,Func,Type) \
{GTYPE_TURRET,Name,UglieT(Func),Type,0}
#define TuEntryS(Name,Func,Type,Size) \
{GTYPE_TURRET,Name,UglieT(Func),Type,Size}

#define UglieC(dat) ((int) &tmpcustom.dat) - ((int) &tmpcustom)
#define CuEntry(Name,Func,Type) \
{GTYPE_CUSTOM,Name,UglieC(Func),Type,0}
#define CuEntryS(Name,Func,Type,Size) \
{GTYPE_CUSTOM,Name,UglieC(Func),Type,Size}

#define UglieScen(dat) ((int) &tmpscen.dat) - ((int) &tmpscen)
#define SEntry(Name,Func,Type) \
{GTYPE_SCEN,Name,UglieScen(Func),Type,0}
#define SEntryS(Name,Func,Type,Size) \
{GTYPE_SCEN,Name,UglieScen(Func),Type,Size}

#define UglieSside(dat) ((int) &tmpsside.dat) - ((int) &tmpsside)
#define SSEntry(Name,Func,Type) \
{GTYPE_SSIDE,Name,UglieSside(Func),Type,0}
#define SSEntryS(Name,Func,Type,Size) \
{GTYPE_SSIDE,Name,UglieSside(Func),Type,Size}

#define UglieSsobj(dat) ((int) &tmpssobj.dat) - ((int) &tmpssobj)
#define SSOEntry(Name,Func,Type) \
{GTYPE_SSOBJ,Name,UglieSsobj(Func),Type,0}
#define SSOEntryS(Name,Func,Type,Size) \
{GTYPE_SSOBJ,Name,UglieSsobj(Func),Type,Size}

char *mechIDfunc(int mode, MECH * mech)
{
    static char buf[3];

    buf[0] = MechID(mech)[0];
    buf[1] = MechID(mech)[1];
    buf[2] = 0;
    return buf;
}

static char *mech_getset_ref(int mode, MECH * mech, char *data)
{
    if (mode) {
	strncpy(MechType_Ref(mech), data, MECHREF_SIZE - 1);
	MechType_Ref(mech)[MECHREF_SIZE - 1] = '\0';
	return NULL;
    } else
	return MechType_Ref(mech);
}


#ifdef BT_ENABLED

extern char *mech_types[];
extern char *move_types[];

char *mechTypefunc(int mode, MECH * mech, char *arg)
{
    int i;

    if (!mode)
	return mech_types[(short) MechType(mech)];
    /* Should _alter_ mechtype.. weeeel. */
    if ((i = compare_array(mech_types, arg)) >= 0)
	MechType(mech) = i;
    return NULL;
}

char *mechMovefunc(int mode, MECH * mech, char *arg)
{
    int i;

    if (!mode)
	return move_types[(short) MechMove(mech)];
    if ((i = compare_array(move_types, arg)) >= 0)
	MechMove(mech) = i;
    return NULL;
}

char *mechTechTimefunc(int mode, MECH * mech)
{
    static char buf[MBUF_SIZE];
    int n = figure_latest_tech_event(mech);

    sprintf(buf, "%d", n);
    return buf;
}

void apply_mechDamage(MECH * omech, char *buf)
{
    MECH mek;
    MECH *mech = &mek;
    int i, j, i1, i2, i3;
    char *s;
    int do_mag = 0;

    memcpy(mech, omech, sizeof(MECH));
    for (i = 0; i < NUM_SECTIONS; i++) {
	SetSectInt(mech, i, GetSectOInt(mech, i));
	SetSectArmor(mech, i, GetSectOArmor(mech, i));
	SetSectRArmor(mech, i, GetSectORArmor(mech, i));
	for (j = 0; j < NUM_CRITICALS; j++)
	    if (GetPartType(mech, i, j) && !IsCrap(GetPartType(mech, i, j))) {
		if (PartIsDestroyed(mech, i, j))
		    UnDestroyPart(mech, i, j);
		if (IsAmmo(GetPartType(mech, i, j)))
		    SetPartData(mech, i, j, FullAmmo(mech, i, j));
		else
		    SetPartDamage(mech, i, j, 0);
	    }
    }
    s = buf;
    while (*s) {
	while (*s && (*s == ' ' || *s == ','))
	    s++;
	if (!(*s))
	    break;
	/* Parse the keyword ; it's one of the many known types */
	if (sscanf(s, "A:%d/%d", &i1, &i2) == 2) {
	    /* Ordinary armor damage */
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		SetSectArmor(mech, i1, GetSectOArmor(mech, i1) - i2);
	} else if (sscanf(s, "A(R):%d/%d", &i1, &i2) == 2) {
	    /* Ordinary rear armor damage */
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		SetSectRArmor(mech, i1, GetSectORArmor(mech, i1) - i2);
	} else if (sscanf(s, "I:%d/%d", &i1, &i2) == 2) {
	    /* Ordinary int damage */
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		SetSectInt(mech, i1, GetSectOInt(mech, i1) - i2);
	} else if (sscanf(s, "C:%d/%d", &i1, &i2) == 2) {
	    /* Dest'ed crit */
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		DestroyPart(mech, i1, i2);
	} else if (sscanf(s, "G:%d/%d(%d)", &i1, &i2, &i3) == 3) {
	    /* Glitch */
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		if (i2 >= 0 && i2 < NUM_CRITICALS)
		    SetPartDamage(mech, i1, i2, i3);
	} else if (sscanf(s, "R:%d/%d(%d)", &i1, &i2, &i3) == 3) {
	    /* Reload */
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		if (i2 >= 0 && i2 < NUM_CRITICALS)
		    SetPartData(mech, i1, i2, FullAmmo(mech, i1, i2) - i3);
	} else if (sscanf(s, "M:%d/%d(%d)", &i1, &i2, &i3) == 3) {
	    if (i1 >= 0 && i1 < NUM_SECTIONS)
		if (i2 >= 0 && i2 < NUM_CRITICALS)
		    SetPartMode(mech, i1, i2, i3);
	}
	while (*s && (*s != ' ' && *s != ','))
	    s++;
    }
    for (i = 0; i < NUM_SECTIONS; i++) {
	if (GetSectInt(mech, i) != GetSectInt(omech, i))
	    SetSectInt(omech, i, GetSectInt(mech, i));
	if (GetSectArmor(mech, i) != GetSectArmor(omech, i))
	    SetSectArmor(omech, i, GetSectArmor(mech, i));
	if (GetSectRArmor(mech, i) != GetSectRArmor(omech, i))
	    SetSectRArmor(omech, i, GetSectRArmor(mech, i));
	for (j = 0; j < NUM_CRITICALS; j++)
	    if (GetPartType(mech, i, j) && !IsCrap(GetPartType(mech, i, j))) {
		if (PartIsDestroyed(mech, i, j) && !PartIsDestroyed(omech, i, j)) {
		    /* Blast a part */
		    DestroyPart(omech, i, j);
		    do_mag = 1;
		} else if (!PartIsDestroyed(mech, i, j) && PartIsDestroyed(omech, i, j)) {
		    mech_RepairPart(omech, i, j);
		    do_mag = 1;
		}
		if (IsAmmo(GetPartType(mech, i, j))) {
		    if (GetPartData(mech, i, j) != GetPartData(omech, i, j))
			SetPartData(omech, i, j, GetPartData(mech, i, j));
		}
		if (GetPartDamage(mech, i, j) != GetPartDamage(omech, i, j))
		    SetPartDamage(omech, i, j, GetPartDamage(mech, i, j)); 
		if (GetPartMode(mech, i, j) != GetPartMode(omech, i, j))
		    SetPartMode(omech, i, j, GetPartMode(mech, i, j));
	    }
    }
    if (do_mag && MechType(omech) == CLASS_MECH)
	do_magic(omech);
}

#define ADD(foo...) { if (count++) strcat(buf,","); sprintf(buf+strlen(buf), foo); }

char *mechDamagefunc(int mode, MECH * mech, char *arg)
{
    /* Lists damage in form:
       A:LOC/num[,LOC/num[,LOC(R)/num]],I:LOC/num
       C:LOC/num,R:LOC/num(num),G:LOC/num(num) */
    int i, j;
    static char buf[LBUF_SIZE];
    int count = 0;

    if (mode) {
	apply_mechDamage(mech, arg);
	return "?";
    };
    buf[0] = 0;
    for (i = 0; i < NUM_SECTIONS; i++)
	if (GetSectOInt(mech, i)) {
	    if (GetSectArmor(mech, i) != GetSectOArmor(mech, i))
		ADD("A:%d/%d", i, GetSectOArmor(mech, i) - GetSectArmor(mech, i));
	    if (GetSectRArmor(mech, i) != GetSectORArmor(mech, i))
		ADD("A(R):%d/%d", i, GetSectORArmor(mech, i) - GetSectRArmor(mech, i));
	}
    for (i = 0; i < NUM_SECTIONS; i++)
	if (GetSectOInt(mech, i))
	    if (GetSectInt(mech, i) != GetSectOInt(mech, i))
		ADD("I:%d/%d", i, GetSectOInt(mech, i) - GetSectInt(mech, i));
    for (i = 0; i < NUM_SECTIONS; i++)
	for (j = 0; j < CritsInLoc(mech, i); j++) {
	    if (GetPartType(mech, i, j) && !IsCrap(GetPartType(mech, i, j))) {
		if (PartIsDestroyed(mech, i, j)) {
		    ADD("C:%d/%d", i, j);
		} else {
		    if (IsAmmo(GetPartType(mech, i, j))) {
			if (GetPartData(mech, i, j) != FullAmmo(mech, i, j))
			    ADD("R:%d/%d(%d)", i, j, FullAmmo(mech, i, j) - GetPartData(mech, i, j));
		    }
		    if (GetPartDamage(mech, i, j))
			ADD("G:%d/%d(%d)", i, j, GetPartDamage(mech, i, j));
		    if (GetPartMode(mech, i, j))
			ADD("M:%d/%d(%d)", i, j, GetPartMode(mech, i, j));
		}
	    }
	}
    return buf;
}

char *mechCentBearingfunc(int mode, MECH * mech, char *arg)
{
    int x = MechX(mech);
    int y = MechY(mech);
    float fx, fy;
    static char buf[SBUF_SIZE];

    MapCoordToRealCoord(x, y, &fx, &fy);
    sprintf(buf, "%d", FindBearing(MechFX(mech), MechFY(mech), fx, fy));
    return buf;
}

char *mechCentDistfunc(int mode, MECH * mech, char *arg)
{
    int x = MechX(mech);
    int y = MechY(mech);
    float fx, fy;
    static char buf[SBUF_SIZE];

    MapCoordToRealCoord(x, y, &fx, &fy);
    sprintf(buf, "%.2f", FindHexRange(fx, fy, MechFX(mech), MechFY(mech)));
    return buf;
}

#endif

/* Mode:
   0 = char -> bit field
   1 = bit field -> char
   */


static int bv_val(int in, int mode)
{
    int p = 0;

    if (mode == 0) {
	if (in >= 'a' && in <= 'z')
	    return 1 << (in - 'a');
	return 1 << ('z' - 'a' + 1 + (in - 'A'));
    }
    while (in > 0) {
	p++;
	in >>= 1;
    }
    /* Hmm. */
    p--;
    if (p < 0)
 	p = 31;
    if (p > ('z' - 'a'))
	return 'A' + (p - ('z' - 'a' + 1));
    return 'a' + p;
}

static int text2bv(char *text)
{
    char *c;
    int j = 0;
    int mode_not = 0;

    if (!Readnum(j, text))
	return j;		/* Allow 'old style' as well */

    /* Valid bitvector letters are: a-z (=27), A-Z (=27 more) */
    for (c = text; *c; c++) {
	if (*c == '!') {
	    mode_not = 1;
	    c++;
	};
	if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')) {
	    int k = bv_val(*c, 0);

	    if (k) {
		if (mode_not)
		    j &= ~k;
		else
		    j |= k;
	    }
	}
	mode_not = 0;
    }
    return j;
}

static char *bv2text(int i)
{
    static char buf[SBUF_SIZE];
    int p = 1;
    char *c = buf;

    while (i > 0) {
	if (i & 1)
	    *(c++) = bv_val(p, 1);
	i >>= 1;
	p <<= 1;
    }
    if (c == buf)
	*(c++) = '-';
    *c = 0;
    return buf;
}

static GMV xcode_data[] = {

    {GTYPE_MECH, "mapindex", ((int) &tmpm.mapindex) - ((int) &tmpm), TYPE_DBREF_RO}, 
    {GTYPE_MECH, "id", (int) mechIDfunc, TYPE_STRFUNC},
    MeEntryS("mechname", MechType_Name, TYPE_STRING, MECHNAME_SIZE),
    MeEntry("maxspeed", MechMaxSpeed, TYPE_FLOAT_RO),
    MeEntry("jumpspeed", MechJumpSpeed, TYPE_FLOAT_RO),
    MeEntry("jumpheading", MechJumpHeading, TYPE_SHORT_RO),
    MeEntry("jumplength", MechJumpLength, TYPE_SHORT_RO),
    MeEntry("pilotnum", MechPilot, TYPE_DBREF),
    MeEntry("pilotdam", MechPilotStatus, TYPE_CHAR),
    MeEntry("si", AeroSI, TYPE_CHAR),
    MeEntry("si_orig", AeroSIOrig, TYPE_CHAR),
    MeEntry("speed", MechSpeed, TYPE_FLOAT),
    MeEntry("heading", MechRFacing, TYPE_SHORT),
    MeEntry("stall", MechStall, TYPE_INT),
    MeEntry("status", MechStatus, TYPE_BV),
    MeEntry("status2", MechStatus2, TYPE_BV),
    MeEntry("critstatus", MechCritStatus, TYPE_BV),
    MeEntry("target", MechTarget, TYPE_DBREF_RO),
    MeEntry("team", MechTeam, TYPE_USHORT),
    MeEntry("tons", MechTons, TYPE_INT_RO),
    MeEntry("towing", MechCarrying, TYPE_INT_RO),
    MeEntry("heat", MechPlusHeat, TYPE_FLOAT_RO),
    MeEntry("disabled_hs", MechDisabledHS, TYPE_INT_RO),
    MeEntry("overheat", MechHeat, TYPE_FLOAT_RO),
    MeEntry("dissheat", MechMinusHeat, TYPE_FLOAT_RO),
    MeEntry("heatsinks", MechRealNumsinks, TYPE_CHAR_RO), 
    MeEntry("targcomp", MechTargComp, TYPE_CHAR),
    MeEntry("carmaxton", CarMaxTon, TYPE_CHAR_RO),
    MeEntry("startup_last", MechStartupLast, TYPE_INT_RO),
    MeEntry("mechprefs", MechPrefs, TYPE_BV),
    MeEntry("staggerdamage", MechTurnDamage, TYPE_INT_RO),
 
#ifdef BT_ENABLED
    {GTYPE_MECH, "mechtype", (int) mechTypefunc, TYPE_STRFUNC_BD},
    {GTYPE_MECH, "mechmovetype", (int) mechMovefunc, TYPE_STRFUNC_BD},
    {GTYPE_MECH, "mechdamage", (int) mechDamagefunc, TYPE_STRFUNC_BD},
    {GTYPE_MECH, "techtime", (int) mechTechTimefunc, TYPE_STRFUNC},
    {GTYPE_MECH, "centdist", (int) mechCentDistfunc, TYPE_STRFUNC},
    {GTYPE_MECH, "centbearing", (int) mechCentBearingfunc, TYPE_STRFUNC},
    {GTYPE_MECH, "sensors", (int) mechSensorInfo, TYPE_STRFUNC},
    {GTYPE_MECH, "mechref", (int) mech_getset_ref, TYPE_STRFUNC_BD},
#endif

    MeEntry("fuel", AeroFuel, TYPE_INT),
    MeEntry("fuel_orig", AeroFuelOrig, TYPE_INT),
    MeEntry("cocoon", MechCocoon, TYPE_INT_RO),
    MeEntry("numseen", MechNumSeen, TYPE_SHORT_RO),

    MeEntry("fx", MechFX, TYPE_FLOAT),
    MeEntry("fy", MechFY, TYPE_FLOAT),
    MeEntry("fz", MechFZ, TYPE_FLOAT),
    MeEntry("x", MechX, TYPE_SHORT),
    MeEntry("y", MechY, TYPE_SHORT),
    MeEntry("z", MechZ, TYPE_SHORT),

    MeEntry("lrsrange", MechLRSRange, TYPE_CHAR),
    MeEntry("radiorange", MechRadioRange, TYPE_SHORT),
    MeEntry("scanrange", MechScanRange, TYPE_CHAR),
    MeEntry("tacrange", MechTacRange, TYPE_CHAR),
    MeEntry("radiotype", MechRadioType, TYPE_CHAR),
    MeEntry("bv", MechBV, TYPE_INT_RO),
    MeEntry("cargospace", CargoSpace, TYPE_INT_RO),
    MeEntry("podspace", PodSpace, TYPE_INT_RO),

    MeVEntry("bay0", AeroBay, 0, TYPE_DBREF),
    MeVEntry("bay1", AeroBay, 1, TYPE_DBREF),
    MeVEntry("bay2", AeroBay, 2, TYPE_DBREF),
    MeVEntry("bay3", AeroBay, 3, TYPE_DBREF),

    MeVEntry("turret0", AeroTurret, 0, TYPE_DBREF),
    MeVEntry("turret1", AeroTurret, 1, TYPE_DBREF),
    MeVEntry("turret2", AeroTurret, 2, TYPE_DBREF),

    MeEntry("unusablearcs", AeroUnusableArcs, TYPE_INT_RO),

    MaEntry("buildflag", buildflag, TYPE_CHAR),
    MaEntry("buildonmap", onmap, TYPE_DBREF_RO),
    MaEntry("cf", cf, TYPE_SHORT),
    MaEntry("cfmax", cfmax, TYPE_SHORT),
    MaEntry("gravity", grav, TYPE_CHAR),
    MaEntry("maxcf", cfmax, TYPE_SHORT),
    MaEntry("firstfree", first_free, TYPE_CHAR_RO),
    MaEntry("mapheight", map_height, TYPE_SHORT_RO),
    MaEntry("maplight", maplight, TYPE_CHAR),
    MaEntryS("mapname", mapname, TYPE_STRING, MAP_NAME_SIZE),
    MaEntry("mapvis", mapvis, TYPE_CHAR),
    MaEntry("mapwidth", map_width, TYPE_SHORT_RO),
    MaEntry("maxvis", maxvis, TYPE_SHORT),
    MaEntry("temperature", temp, TYPE_CHAR),
    MaEntry("winddir", winddir, TYPE_SHORT),
    MaEntry("windspeed", windspeed, TYPE_SHORT),
    MaEntry("cloudbase", cloudbase, TYPE_SHORT),
    MaEntry("flags", flags, TYPE_CBV),

    TuEntry("arcs", arcs, TYPE_INT),
    TuEntry("parent", parent, TYPE_DBREF),
    TuEntry("gunner", gunner, TYPE_DBREF),
    TuEntry("target", target, TYPE_DBREF),
    TuEntry("targx", target, TYPE_SHORT),
    TuEntry("targy", target, TYPE_SHORT),
    TuEntry("targz", target, TYPE_SHORT),
    TuEntry("lockmode", lockmode, TYPE_INT_RO),

    CuEntry("state", state, TYPE_INT),
    CuEntry("user", user, TYPE_DBREF),
    CuEntry("submit", submit, TYPE_DBREF),
    CuEntry("allow", allow, TYPE_INT),

    SEntry("state", state, TYPE_INT),
    SEntry("start", start_t, TYPE_INT),
    SEntry("end", end_t, TYPE_INT),

    SSEntryS("slet", slet, TYPE_STRING, 10),
    SSOEntry("state", state, TYPE_INT),

    MeEntry("radio", MechRadio, TYPE_CHAR),
    MeEntry("computer", MechComputer, TYPE_CHAR),
    MeEntry("perception", MechPer, TYPE_INT_RO),

    {-1, NULL, 0, TYPE_STRING}
};

FUNCTION(fun_btsetxcodevalue)
{
    /* fargs[0] = id of the mech
       fargs[1] = name of the value
       fargs[2] = what the value's to be set as
     */
    dbref it;
    int i, spec;
    void *foo;
    void *bar;
    void *(*tempfun) ();

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    for (i = 0; xcode_data[i].name; i++)
	if (!strcasecmp(fargs[1], xcode_data[i].name) &&
	    xcode_data[i].gtype == spec &&
	    (scode_in_out[xcode_data[i].type] & 2)) {
	    bar = (void *) ((int) foo + xcode_data[i].rel_addr);
	    switch (xcode_data[i].type) {
	    case TYPE_STRFUNC_BD:
	    case TYPE_STRFUNC_S:
		tempfun = (void *) xcode_data[i].rel_addr;
		tempfun(1, (MECH *) foo, (char *) fargs[2]);
		break;
	    case TYPE_STRING:
		strncpy((char *) bar, fargs[2], xcode_data[i].size - 1);
		((char *)bar)[xcode_data[i].size - 1] = '\0';
		break;
	    case TYPE_DBREF:
		*((dbref *) bar) = atoi(fargs[2]);
		break;
	    case TYPE_CHAR:
		*((char *) bar) = atoi(fargs[2]);
		break;
	    case TYPE_SHORT:
		*((short *) bar) = atoi(fargs[2]);
		break;
	    case TYPE_USHORT:
		*((unsigned short *) bar) = atoi(fargs[2]);
		break;
	    case TYPE_INT:
		*((int *) bar) = atoi(fargs[2]);
		break;
	    case TYPE_FLOAT:
		*((float *) bar) = atof(fargs[2]);
		break;
	    case TYPE_BV:
		*((int *) bar) = text2bv(fargs[2]);
		break;
	    case TYPE_CBV:
		*((byte *) bar) = (byte) text2bv(fargs[2]);
		break;
	    }
	    safe_tprintf_str(buff, bufc, "1"); 
	    return;
	}
    safe_tprintf_str(buff, bufc, "#-1");
    return;
}

static char *RetrieveValue(void *data, int i)
{
    void *bar = (void *) ((int) data + xcode_data[i].rel_addr);
    static char buf[LBUF_SIZE];
    char *(*tempfun) ();

    switch (xcode_data[i].type) {
    case TYPE_STRFUNC_BD:
    case TYPE_STRFUNC:
	tempfun = (void *) xcode_data[i].rel_addr;
	sprintf(buf, "%s", (char *) tempfun(0, (MECH *) data));
	break;
    case TYPE_STRING:
	sprintf(buf, "%s", (char *) bar);
	break;
    case TYPE_DBREF:
    case TYPE_DBREF_RO:
	sprintf(buf, "%d", (dbref) * ((dbref *) bar));
	break;
    case TYPE_CHAR:
    case TYPE_CHAR_RO:
	sprintf(buf, "%d", (char) *((char *) bar));
	break;
    case TYPE_SHORT:
    case TYPE_SHORT_RO:
	sprintf(buf, "%d", (short) *((short *) bar));
	break;
    case TYPE_USHORT:
	sprintf(buf, "%d", (unsigned short) *((unsigned short *) bar));
	break;
    case TYPE_INT:
    case TYPE_INT_RO:
	sprintf(buf, "%d", (int) *((int *) bar));
	break;
    case TYPE_FLOAT:
    case TYPE_FLOAT_RO:
	sprintf(buf, "%.2f", (float) *((float *) bar));
	break;
    case TYPE_BV:
	strcpy(buf, bv2text((int) *((int *) bar)));
	break;
    case TYPE_CBV:
	strcpy(buf, bv2text((int) *((char *) bar)));
	break;
    }
    return buf;
} 

FUNCTION(fun_btgetweight)
{
    /* 
      fargs[0] = stringname of part
    */
    float sw = 0;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
#if 0
    i = -1;
    if (!find_matching_long_part(fargs[0], &i, &p)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p),
            "#-1 INVALID PART NAME");
    }
    if (strstr(fargs[0], "Sword") && !strstr(fargs[0], PC.))
	p = I2Special(SWORD);
    if (IsWeapon(p))
        sw = ((float) ((float) ((float) 1024 / 10) * (float) MechWeapons[Weapon2I(p)].weight) / (float) MechWeapons[Weapon2I(p)].criticals);
    else if (IsAmmo(p))
        sw = ((float) 1024 / (float) MechWeapons[Ammo2I(p)].ammoperton);
    else if (IsBomb(p))
        sw = ((float) ((float) 1024 / 10) * (float) BombWeight(Bomb2I(p)));
    else if (IsSpecial(p)) /* && p <= I2Special(LAMEQUIP) */
        sw = ((float) specialweight[Special2I(p)]);
    else if (IsCargo(p))
        sw = ((float) cargoweight[Cargo2I(p)]);
    else
        sw = ((float) 1024 / 10);
#endif
    sw = EconPartWeight(0, 0, fargs[0]);

    if (sw <= 0)
	sw = (1024 * 100);

    safe_tprintf_str(buff, bufc, tprintf("%.3f", (float) sw / 1024));
    return;
} 

FUNCTION(fun_btgetxcodevalue)
{
    /* fargs[0] = id of the mech
       fargs[1] = name of the value
     */
    dbref it;
    int i;
    void *foo;
    int spec;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    for (i = 0; xcode_data[i].name; i++)
	if (!strcasecmp(fargs[1], xcode_data[i].name) &&
	    xcode_data[i].gtype == spec &&
	    (scode_in_out[xcode_data[i].type] & 1)) {
	    safe_tprintf_str(buff, bufc, "%s", RetrieveValue(foo, i));
	    return;
	}
    safe_tprintf_str(buff, bufc, "#-1");
    return;
}

void set_xcodestuff(dbref player, void *data, char *buffer)
{
    char *args[2];
    int t, i;
    void *bar;
    void *(*tempfun) ();

    memset(args, 0, sizeof(char *) * 2);

    DOCHECK(silly_parseattributes(buffer, args, 2) != 2,
	"Invalid arguments!");
    t = WhichSpecial(Location(player));
    for (i = 0; xcode_data[i].name; i++)
	if (xcode_data[i].gtype == t)
	    break;
    DOCHECK(!xcode_data[i].name,
	"Error: No xcode values for this type of object found.");
    for (i = 0; xcode_data[i].name; i++)
	if (!strcasecmp(args[0], xcode_data[i].name) &&
	    xcode_data[i].gtype == t &&
	    (scode_in_out[xcode_data[i].type] & 2)) break;
    DOCHECK(!xcode_data[i].name,
	"Error: No matching xcode value for this type of object found.");
    bar =
	(void *) ((int) FindObjectsData(Location(player)) +
	xcode_data[i].rel_addr);
    switch (xcode_data[i].type) {
    case TYPE_STRFUNC_BD:
    case TYPE_STRFUNC_S:
	tempfun = (void *) xcode_data[i].rel_addr;
	tempfun(1, getMech(Location(player)), (char *) args[1]);
	break;
    case TYPE_STRING:
	strncpy((char *) bar, args[1], xcode_data[i].size - 1);
	((char *)bar)[xcode_data[i].size - 1] = '\0';
	break;
    case TYPE_DBREF:
	*((dbref *) bar) = atoi(args[1]);
	break;
    case TYPE_CHAR:
	*((char *) bar) = atoi(args[1]);
	break;
    case TYPE_SHORT:
	*((short *) bar) = atoi(args[1]);
	break;
    case TYPE_USHORT:
	*((unsigned short *) bar) = atoi(args[1]);
	break;
    case TYPE_INT:
	*((int *) bar) = atoi(args[1]);
	break;
    case TYPE_FLOAT:
	*((float *) bar) = atof(args[1]);
	break;
    case TYPE_BV:
	*((int *) bar) = text2bv(args[1]);
	break;
    case TYPE_CBV:
	*((byte *) bar) = (byte) text2bv(args[1]);
	break;
    }
}

void list_xcodestuff(dbref player, void *data, char *buffer)
{
    int t, i, flag = CM_TWO, se_len = 37;
    coolmenu *c = NULL;

    t = WhichSpecial(Location(player));
    for (i = 0; xcode_data[i].name; i++)
	if (xcode_data[i].gtype == t &&
	    (scode_in_out[xcode_data[i].type] & 1)) break;
    DOCHECK(!xcode_data[i].name,
	"Error: No xcode values for this type of object found.");
    addline();
    cent(tprintf("Data for %s (%s)", Name(Location(player)),
	    SpecialObjects[t].type));
    addline();
    if (*buffer == '1') {
	flag = CM_ONE;
	se_len = se_len * 2;
    };
    if (*buffer == '4') {
	flag = CM_FOUR;
	se_len = se_len / 2;
    };
    if (*buffer == '1' || *buffer == '4')
	buffer++;
    for (i = 0; xcode_data[i].name; i++) {
	if (xcode_data[i].gtype == t &&
	    (scode_in_out[xcode_data[i].type] & 1)) {
	    /* 1/3(left) = name, 2/3(right)=value */
	    char mask[SBUF_SIZE];
	    char lab[SBUF_SIZE];

	    if (*buffer)
		if (strncasecmp(xcode_data[i].name, buffer,
			strlen(buffer))) continue;
	    strcpy(lab, xcode_data[i].name);
	    lab[se_len / 3] = 0;
	    sprintf(mask, "%%-%ds%%%ds", se_len / 3, se_len * 2 / 3);
	    sim(tprintf(mask, lab, RetrieveValue(data, i)), flag);
	}
    }
    addline();
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}


#ifdef BT_ENABLED
FUNCTION(fun_btunderrepair)
{
    /* fargs[0] = ref of the mech to be checked */
    int n;
    MECH *mech;
    dbref it;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    FUNCHECK(!IsMech(it), "#-2");
    mech = FindObjectsData(it);
    n = figure_latest_tech_event(mech);
    safe_tprintf_str(buff, bufc, "%d", n > 0);
}

FUNCTION(fun_btnumrepjobs)
{
    MECH *mech;
    dbref it;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    FUNCHECK(!IsMech(it), "#-2");
    mech = FindObjectsData(it);

    if (unit_is_fixable(mech))
        make_damage_table(mech);
    else
        make_scrap_table(mech);

    safe_tprintf_str(buff, bufc, "%d", damage_last);
}

FUNCTION(fun_btstores)
{
    /* fargs[0] = id of the bay */
    /* fargs[1] = name of the part */
    dbref it;
    int i, spec;
    void *foo;
    int p;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    i = -1;
    if (!find_matching_long_part(fargs[1], &i, &p)) {
	i = -1;
	FUNCHECK(!find_matching_vlong_part(fargs[1], &i, &p),
	    "#-1 INVALID PART NAME");
    }
    safe_tprintf_str(buff, bufc, "%d", econ_find_items(it, p));
    return;
}

FUNCTION(fun_btmapterr)
{
    /* fargs[0] = reference of map
       fargs[1] = x
       fargs[2] = y
     */
    dbref it;
    MAP *map;
    int x, y;
    int spec;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(spec != GTYPE_MAP, "#-1");
    FUNCHECK(!(map = FindObjectsData(it)), "#-1");
    FUNCHECK(Readnum(x, fargs[1]), "#-2");
    FUNCHECK(Readnum(y, fargs[2]), "#-2");
    FUNCHECK(x < 0 || y < 0 || x >= map->map_width ||
	y >= map->map_height, "?");
    safe_tprintf_str(buff, bufc, "%c", GetTerrain(map, x, y));
}

FUNCTION(fun_btmapelev)
{
    /* fargs[0] = reference of map
       fargs[1] = x
       fargs[2] = y
     */
    dbref it;
    int i;
    MAP *map;
    int x, y;
    int spec;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(spec != GTYPE_MAP, "#-1");
    FUNCHECK(!(map = FindObjectsData(it)), "#-1");
    FUNCHECK(Readnum(x, fargs[1]), "#-2");
    FUNCHECK(Readnum(y, fargs[2]), "#-2");
    FUNCHECK(x < 0 || y < 0 || x >= map->map_width ||
	y >= map->map_height, "?");
    i = Elevation(map, x, y);
    if (i < 0)
	safe_tprintf_str(buff, bufc, "-%c", '0' + -i);
    else
	safe_tprintf_str(buff, bufc, "%c", '0' + i);
}

void list_xcodevalues(dbref player)
{
    int i;

    notify(player, "Xcode attributes accessible thru get/setxcodevalue:");
    for (i = 0; xcode_data[i].name; i++)
	notify(player, tprintf("\t%d\t%s", xcode_data[i].gtype,
		xcode_data[i].name));
}

/* Glue functions for easy scode interface to ton of hcode stuff */

FUNCTION(fun_btdesignex)
{
    char *id = fargs[0];

    if (mechref_path(id)) {
	safe_tprintf_str(buff, bufc, "1");
    } else
	safe_tprintf_str(buff, bufc, "0");
}

FUNCTION(fun_btdamages)
{
    /* fargs[0] = id of the mech
     */
    dbref it;
    char *damstr;
    MECH *mech;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    damstr = damages_func(mech);
    safe_tprintf_str(buff, bufc, damstr ? damstr : "#-1 ERROR");
}

FUNCTION(fun_btcritstatus)
{
    /* fargs[0] = id of the mech
     * fargs[1] = location to show
     */
    dbref it;
    char *critstr;
    MECH *mech;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    critstr = critstatus_func(mech, fargs[1]);	/* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, critstr ? critstr : "#-1 ERROR");
}

FUNCTION(fun_btarmorstatus)
{
    /* fargs[0] = id of the mech
     * fargs[1] = location to show
     */
    dbref it;
    char *infostr;
    MECH *mech;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    infostr = armorstatus_func(mech, fargs[1]);	/* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_bttechlist)
{
    dbref it;
    MECH *mech;
    int i, ii, part = 0, axe = 0, sword = 0, mace = 0, hascase = 0;
    char *infostr;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    infostr = techlist_func(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : " ");
}

FUNCTION(fun_btsetarmorstatus)
{
    /* fargs[0] = id of the mech
     * fargs[1] = location to set
     * fargs[2] = what to change
     * fargs[3] = value to change to.
     */
    dbref it;
    char *infostr;
    MECH *mech;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    infostr = setarmorstatus_func(mech, fargs[1], fargs[2], fargs[3]);	/* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
	STARTLOG(LOG_ALWAYS, "WIZ", "CHANGE") {
		log_text(tprintf("ARMOR on %d, changed to %d, by ",
			fargs[0], fargs[3]));
		log_name(player);
	ENDLOG;
	}
}

FUNCTION(fun_btthreshhold)
{
    /*
     * fargs[0] = skill to query
     */
    int xpth;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    xpth = btthreshhold_func(fargs[0]);
    safe_tprintf_str(buff, bufc, xpth < 0 ? "#%d ERROR" : "%d", xpth);
}

extern void correct_speed(MECH *);

FUNCTION(fun_btsetmaxspeed)
{
    /* fargs[0] = id of the mech
       fargs[1] = what the new maxspeed should be set too
     */
    dbref it;
    MECH *mech;
    float newmaxspeed = atof(fargs[1]);

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    MechMaxSpeed(mech) = newmaxspeed;
    correct_speed(mech);

    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btgetrealmaxspeed)
{
    dbref it;
    MECH *mech;
    float speed;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    speed = MechCargoMaxSpeed(mech, MechMaxSpeed(mech));

    safe_tprintf_str(buff, bufc, tprintf("%f", speed));
}

FUNCTION(fun_btgetbv)
{
    dbref it;
    MECH *mech;
    int bv;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    bv = CalculateBV(mech, 100, 100);
    MechBV(mech) = bv;
    safe_tprintf_str(buff, bufc, tprintf("%d", bv));
}

FUNCTION(fun_btcritslot)
{
    /* fargs[0] = id of the mech
       fargs[1] = location name
       fargs[2] = critslot
       fargs[3] = partname type flag, 0 template name, 1 repair part name (differentiate Ammo types basically)
     */
    dbref it;
    MECH *mech;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    if (!fn_range_check("BTCRITSLOT", nfargs, 3, 4, buff, bufc))
	return;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1 INVALID MECH");

    safe_tprintf_str(buff, bufc, "%s", critslot_func(mech, fargs[1], fargs[2], fargs[3]));
}

FUNCTION(fun_btgetrange)
{
/* fargs[0] - [4] Combos of XY or DBref */
    dbref mechAdb, mechBdb, mapdb;
    MECH *mechA, *mechB;
    MAP *map;
    float fxA, fyA, fxB, fyB;
    int xA, yA, xB, yB;

    FUNCHECK(!WizR(player), "#=1 PERMISSION DENIED");

    if (!fn_range_check("BTGETRANGE", nfargs, 3, 5, buff, bufc))
	return;

    mapdb = match_thing(player, fargs[0]);
    FUNCHECK(mapdb == NOTHING || !Examinable(player, mapdb), "#-1 INVALID MAPDB");
    FUNCHECK(!IsMap(mapdb), "#-1 OBJECT NOT HCODE");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");

    if (nfargs == 3 && fargs[1][0] == '#' && fargs[2][0] == '#') {
	mechAdb = match_thing(player, fargs[1]);
	FUNCHECK(mechAdb == NOTHING || !Examinable(player, mechAdb), "#-1 INVALID MECHDBREF");
	mechBdb = match_thing(player, fargs[2]);
	FUNCHECK(mechBdb == NOTHING || !Examinable(player, mechBdb), "#-1 INVALID MECHDBREF");
	FUNCHECK(!IsMech(mechAdb) || !IsMech(mechBdb), "#-1 INVALID MECH");
	FUNCHECK(!(mechA = getMech(mechAdb)) || !(mechB = getMech(mechBdb)), "#-1 INVALID MECH");
	FUNCHECK(mechA->mapindex != mapdb || mechB->mapindex != mapdb, "#-1 MECH NOT ON MAP");
	safe_tprintf_str(buff, bufc, "%f", FaMechRange(mechA, mechB));
	return; 
    } else if (nfargs == 4 && fargs[1][0] == '#') {
	mechAdb = match_thing(player, fargs[1]);
	FUNCHECK(mechAdb == NOTHING || !Examinable(player, mechAdb), "#-1 INVALID MECHDBREF");
	FUNCHECK(!IsMech(mechAdb), "#-1 INVALID MECH");
	FUNCHECK(!(mechA = getMech(mechAdb)), "#-1 INVALID MECH");
	FUNCHECK(mechA->mapindex != mapdb, "#-1 MECH NOT ON MAP");
	xA = atoi(fargs[2]);
	yA = atoi(fargs[3]);
	FUNCHECK(xA < 0 || yA < 0 || xA > map->map_width || yA > map->map_height, "#-1 INVALID COORDS");
	MapCoordToRealCoord(xA, yA, &fxA, &fyA);
	safe_tprintf_str(buff, bufc, "%f", FindRange(MechFX(mechA), MechFY(mechA), MechFZ(mechA), fxA, fyA, Elevation(map, xA, yA) * ZSCALE));
	return;
    } else if (nfargs == 5) {
	xA = atoi(fargs[1]);
	yA = atoi(fargs[2]);
	FUNCHECK(xA < 0 || yA < 0 || xA > map->map_width || yA > map->map_height, "#-1 INVALID COORDS"); 
	xB = atoi(fargs[3]);
	yB = atoi(fargs[4]);
	FUNCHECK(xB < 0 || yB < 0 || xB > map->map_width || yB > map->map_height, "#-1 INVALID COORDS");
	MapCoordToRealCoord(xA, yA, &fxA, &fyA);
	MapCoordToRealCoord(xB, yB, &fxB, &fyB);
	safe_tprintf_str(buff, bufc, "%f", FindRange(fxA, fyA, Elevation(map, xA, yA) * ZSCALE, fxB, fyB, Elevation(map, xB, yB)));
	return; 
    }
safe_tprintf_str(buff, bufc, "#-1 GENERAL ERROR");
return;
}

FUNCTION(fun_btsetcritslot)
{
    /* fargs[0] = id of the mech
       fargs[1] = location name
       fargs[2] = critslot
       fargs[3] = partname
       fargs[4] = partname type flag, 0 template name, 1 repair part name (differentiate Ammo types basically)
     */
    dbref it;
    char *partname;
    MECH *mech;
    int critslot;
    int section;
    int ind;
    int type, mode, data;
    char *tsection;
    int numsections;
    int partflag = 0;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    if (!fn_range_check("BTSETCRITSLOT", nfargs, 4, 5, buff, bufc))
	return;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1");
    FUNCHECK(Readnum(critslot, fargs[2]), "#-2");

    switch (MechType(mech)) {
    case CLASS_MECH:
    case CLASS_MW:
	numsections = NUM_SECTIONS;
	break;
    case CLASS_VEH_GROUND:
    case CLASS_VEH_NAVAL:
	numsections = (NUM_VEH_SECTIONS - 1);	/* Rotor is included in vehicle section names */
	break;
    case CLASS_VEH_VTOL:
	numsections = NUM_VEH_SECTIONS;
	break;
    case CLASS_AERO:
	numsections = NUM_AERO_SECTIONS;
	break;
    case CLASS_SPHEROID_DS:
	numsections = NUM_DS_SECTIONS;
	break;
    case CLASS_DS:
	numsections = NUM_DS_SECTIONS;
	break;
    case CLASS_BSUIT:
	numsections = NUM_BSUIT_MEMBERS;
	break;
    default:
	safe_tprintf_str(buff, bufc, "Invalid Mech Type");
	return;
    }

    for (ind = 0; ind < numsections; ++ind) {
	if ((tsection =
		ShortArmorSectionString(MechType(mech), MechMove(mech),
		    ind)) != NULL) {
	    if (!strcasecmp(ShortArmorSectionString(MechType(mech),
			MechMove(mech), ind), fargs[1])) {
		break;
	    }
	}
    }
    section = ind;

    /* get part flag, default to template scheme of partnames */
    if (fargs[4] && *fargs[4]) {
	partflag = (int) (*fargs[4] - '0');
    }

    FUNCHECK(((partflag < 0) ||
	    (partflag >= 4)),
	"#-1 Invalid Partflag! valid values are: 0,1,2,3");
    FUNCHECK(((section < 0) ||
	    (section >= numsections)), "#-1 Invalid section!");
    FUNCHECK(((critslot < 1) ||
	    (critslot > NUM_CRITICALS)), "#-1 Invalid Critical Slot");
    critslot--;

    type = GetPartType(mech, section, critslot);
    mode = GetPartMode(mech, section, critslot);
    data = GetPartData(mech, section, critslot);
    FUNCHECK(type == 0, "Empty");

    /* EndoSteel and FerroFibrous are non-parts parts */
    FUNCHECK(IsCrap(type), "Empty");

    /* handle XL_ENGINE, DOUBLE_HEAT_SINK, S_ACTUATOR */
    type = alias_part(mech, type, section);

    /* convert template ammo partnames to tech partnames */
    if (IsAmmo(type)) {
	type = FindAmmoType(mech, section, critslot);
    }

    partname = get_parts_vlong_name(type);
    safe_tprintf_str(buff, bufc, partname ? partname : "#-1 ERROR");
}

FUNCTION(fun_bttechtime)
{
    time_t old;
    char *olds = silly_atr_get(player, A_TECHTIME);
    char buf[MBUF_SIZE];

    if (olds) {
	old = (time_t) atoi(olds);
	if (old < mudstate.now) {
	    strcpy(buf, "00:00.00");
	} else {
	    old -= mudstate.now;
	    sprintf(buf, "%02ld:%02d.%02d", (long) (old / 3600),
		(int) ((old / 60) % 60), (int) (old % 60));
	}
    } else {
	strcpy(buf, "00:00.00");
    }

    notify(player, buf);
}

FUNCTION(fun_btaddstores)
{
    /* fargs[0] = id of the bay */
    /* fargs[1] = name of the part */
    /* fargs[2] = amount */
    dbref it;
    int i, spec, num;
    void *foo;
    int p;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK(Readnum(num, fargs[2]), "#-2 Illegal Value");
    i = -1;
    if (!find_matching_long_part(fargs[1], &i, &p)) {
	i = -1;
	FUNCHECK(!find_matching_vlong_part(fargs[1], &i, &p),
	    "#-1 INVALID PART NAME");
    }
    econ_change_items(it, p, num);
    safe_tprintf_str(buff, bufc, "%d", econ_find_items(it, p));
    return;
}

FUNCTION(fun_btremovestores)
{
    /* fargs[0] = id of the bay */
    /* fargs[1] = name of the part */
    /* fargs[2] = amount */
    dbref it;
    int i, spec;
    int num = 0;
    void *foo;
    int p;

    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK(Readnum(num, fargs[2]), "#-2 Illegal Value");
    i = -1;
    if (!find_matching_long_part(fargs[1], &i, &p)) {
	i = -1;
	FUNCHECK(!find_matching_vlong_part(fargs[1], &i, &p),
	    "#-1 INVALID PART NAME");
    }
    econ_change_items(it, p, 0 - num);
    safe_tprintf_str(buff, bufc, "%d", econ_find_items(it, p));
    return;
}

FUNCTION(fun_btid2db)
{
    /* fargs[0] = map 
       fargs[1] = target ID */
    MAP *map;
    MECH *tempmech;
    dbref mapnum;
    int loop;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mapnum = match_thing(player, fargs[0]);
    FUNCHECK(mapnum == NOTHING || !Examinable(player, mapnum),
        "#-1 INVALID MAP");
    FUNCHECK(!IsMap(mapnum), "#-1 INVALID MECH");
    FUNCHECK(!(map = getMap(mapnum)), "#-1 INVALID MAP");
    FUNCHECK(strlen(fargs[1]) != 2, "#-1 INVALID ID");

    tempmech = FindTargetDBREFFromID(map, fargs[1]);
    FUNCHECK(!tempmech, "#-1 INVALID ID");
    safe_tprintf_str(buff, bufc, "#%d", (int) tempmech->mynum); 
}

FUNCTION(fun_bthexlos)
{
    /* fargs[0] = mech
       fargs[1] = x
       fargs[2] = y
     */
                                                                                                                                                                                      
    MECH *mech;
    MAP *map;
    int x = -1, y = -1;
    float fx, fy;
    dbref mechdb;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mechdb = match_thing(player, fargs[0]);
    FUNCHECK(mechdb == NOTHING || !Examinable(player, mechdb),
        "#-1 INVALID MECH");
    FUNCHECK(!IsMech(mechdb), "#-1 INVALID MECH");
    FUNCHECK(!(mech = getMech(mechdb)), "#-1 INVALID MECH");
    FUNCHECK(!(map = getMap(mech->mapindex)), "#-1 INVALID MAP"); 

    x = atoi(fargs[1]);
    y = atoi(fargs[2]);
    FUNCHECK(x < 0 || x > map->map_width, "#-1 INVALID X COORD");
    FUNCHECK(y < 0 || y > map->map_height, "#-1 INVALID Y COORD");
    MapCoordToRealCoord(x, y, &fx, &fy);

    if (InLineOfSight_NB(mech, NULL, x, y, FindHexRange(MechFX(mech), MechFY(mech), fx, fy)))
        safe_tprintf_str(buff, bufc, "1");
    else
        safe_tprintf_str(buff, bufc, "0"); 
}

FUNCTION(fun_btlosm2m)
{
    /* fargs[0] = mech
       fargs[1] = target
     */
                                                                                                                                                                                      
    int mechdb;
    MECH *mech, *target;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mechdb = match_thing(player, fargs[0]);
    FUNCHECK(mechdb == NOTHING || !Examinable(player, mechdb), "#-1 INVALID MECH");
    FUNCHECK(!IsMech(mechdb), "#-1 INVALID MECH");
    FUNCHECK(!(mech = getMech(mechdb)), "#-1 INVALID MECH");
                                                                                                                                                                                      
    mechdb = match_thing(player, fargs[1]);
    FUNCHECK(mechdb == NOTHING || !Examinable(player, mechdb),
        "#-1 INVALID MECH");
    FUNCHECK(!IsMech(mechdb), "#-1 INVALID MECH");
    FUNCHECK(!(target = getMech(mechdb)), "#-1 INVALID MECH");
                                                                                                                                                                                      
    if (InLineOfSight_NB(mech, target, MechX(mech), MechY(mech),
            FlMechRange(getmap(mech->mapindex), mech, target)))
        safe_tprintf_str(buff, bufc, "1");
    else
        safe_tprintf_str(buff, bufc, "0");
}

FUNCTION(fun_btloadmap)
{
    /* fargs[0] = mapdb
       fargs[1] = mapname
    */
    int mapdb;
    MAP * map;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mapdb = match_thing(player, fargs[0]);
    FUNCHECK(!Good_obj(mapdb), "#-1 INVALID MAP");
    map = getMap(mapdb);
    FUNCHECK(!map, "#-1 INVALID MAP");
    switch (do_loadmap(player, map, fargs[1])) {
    case -1:
        safe_str("#-1 MAPFILE NOT FOUND", buff, bufc);
        return;
    case -2:
	safe_str("#-1 INVALID MAP", buff, bufc);
	return;
    case -3:
	safe_str("#-1 ERROR PARTIAL LOAD", buff, bufc);
	return;
    case 0:
        break;
    default:
        safe_str("#-1 WIERD ERROR", buff, bufc);
        return;
    }
    map_clearmechs(player, map, "");
    safe_str("1", buff, bufc);
}

FUNCTION(fun_btloadmech)
{
    /*
	fargs[0] = mechobject
	fargs[1] = mechref
    */
    int mechdb;
    MECH *mech;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mechdb = match_thing(player, fargs[0]);
    FUNCHECK(!Good_obj(mechdb), "#-1 INVALID TARGET");
    mech = getMech(mechdb);
    FUNCHECK(!mech, "#-1 INVALID TARGET");
    if (mech_loadnew(player, mech, fargs[1]) == 1) {
        event_remove_data((void *)mech);
        clear_mech_from_LOS(mech);
        safe_str("1", buff, bufc);
    } else {
        safe_str("#-1 UNABLE TO LOAD TEMPLATE", buff, bufc);
    }
}

FUNCTION(fun_bthexemit)
{
    /* fargs[0] = map
       fargs[1] = x
       fargs[2] = y
       fargs[3] = message
     */
    MAP *map;
    int x = -1, y = -1;
    dbref mapnum;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
                                                                                                                                                                                      
    FUNCHECK(!fargs[3] || !*fargs[3], "#-1 INVALID MESSAGE");
                                                                                                                                                                                      
    mapnum = match_thing(player, fargs[0]);
    FUNCHECK(mapnum < 0, "#-1 INVALID MAP");
    map = getMap(mapnum);
    FUNCHECK(!map, "#-1 INVALID MAP");
                                                                                                                                                                                      
    x = atoi(fargs[1]);
    y = atoi(fargs[2]);
    FUNCHECK(x < 0 || x > map->map_width, "#-1 X COORD");
    FUNCHECK(y < 0 || y > map->map_height, "#-1 Y COORD");
    HexLOSBroadcast(map, x, y, fargs[3]);
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btmapemit)
{
    /* fargs[0] = mapref
       fargs[1] = message
     */
    MAP *map;
    dbref mapnum;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED"); 
    FUNCHECK(!fargs[1] || !*fargs[1], "#-1 INVALID MESSAGE");
                                                                                                                                                                                      
    mapnum = match_thing(player, fargs[0]);
    FUNCHECK(mapnum < 0, "#-1 INVALID MAP");
    map = getMap(mapnum);
    FUNCHECK(!map, "#-1 INVALID MAP");
                                                                                                                                                                                      
    MapBroadcast(map, fargs[1]);
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btparttype)
{
    /*
      fargs[0] = stringname of part
    */
    int i, p;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    if (!find_matching_long_part(fargs[0], &i, &p)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p),
            "#-1 INVALID PART NAME");
    }
    if (strstr(fargs[0], "Sword") && !strstr(fargs[0], "PC."))
        p = I2Special(SWORD);
    if (IsWeapon(p)) {
	safe_tprintf_str(buff, bufc, "WEAP");
	return;
    } else if (IsAmmo(p) || strstr(fargs[0], "Ammo_")) {
	safe_tprintf_str(buff, bufc, "AMMO");
	return;
    } else if (IsBomb(p)) {
	safe_tprintf_str(buff, bufc, "BOMB");
	return;
    } else if (IsSpecial(p)) {
	safe_tprintf_str(buff, bufc, "PART");
	return;
    } else if (mudconf.btech_complexrepair && IsCargo(p) && Cargo2I(p) >= TON_SENSORS_FIRST && Cargo2I(p) <= TON_ENGINE_COMP_LAST) {
	safe_tprintf_str(buff, bufc, "PART");
	return; 
    } else if (IsCargo(p)) {
	safe_tprintf_str(buff, bufc, "CARG");
	return; 
    } else { 
	safe_tprintf_str(buff, bufc, "OTHER");
	return; 
    }
}

FUNCTION(fun_btgetpartcost)
{
int i, p, index;

FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
if (!find_matching_long_part(fargs[0], &i, &p)) {
    i = -1;
    FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p), "#-1 INVALID PART NAME");
    }
if (strstr(fargs[0], "Sword") && !strstr(fargs[0], "PC."))
    p = I2Special(SWORD);

safe_tprintf_str(buff, bufc, "%lld", GetPartCost(p));
return; 
}

FUNCTION(fun_btsetpartcost)
{
extern unsigned long long int specialcost[SPECIALCOST_SIZE];
extern unsigned long long int ammocost[AMMOCOST_SIZE];
extern unsigned long long int weapcost[WEAPCOST_SIZE];
extern unsigned long long int cargocost[CARGOCOST_SIZE];
extern unsigned long long int bombcost[BOMBCOST_SIZE];
int i, p, index;
unsigned long long int cost;                                                                                                                                                                                      
FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
if (!find_matching_long_part(fargs[0], &i, &p)) {
    i = -1;
    FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p), "#-1 INVALID PART NAME");
    }
if (strstr(fargs[0], "Sword") && !strstr(fargs[0], "PC."))
    p = I2Special(SWORD);
cost = atoll(fargs[1]);
if (cost < 0) {
    safe_tprintf_str(buff, bufc, "#-1 COST ERROR");
    return;
    }

if (IsWeapon(p))
    weapcost[Weapon2I(p)] = cost;
else if (IsAmmo(p))
    ammocost[Ammo2I(p)] = cost;
else if (IsSpecial(p))
    specialcost[Special2I(p)] = cost;
else if (IsBomb(p))
    bombcost[Bomb2I(p)] = cost;
else if (IsCargo(p))
    cargocost[Cargo2I(p)] = cost;
else
    safe_tprintf_str(buff, bufc, "#-1 ERROR");

safe_tprintf_str(buff, bufc, "%lld", cost);
return; 
}

FUNCTION(fun_btgetxcodevalue_ref)
{
    /* fargs[0] = mech ref
       fargs[1] = name of the value
     */
    int i;
    MECH *tempmech;
    int spec;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((tempmech = loadfuncref(fargs[0])) == NULL, "#-1 INVALID REF");
    spec = GTYPE_MECH;
    for (i = 0; xcode_data[i].name; i++)
        if (!strcasecmp(fargs[1], xcode_data[i].name) && xcode_data[i].gtype == spec && (scode_in_out[xcode_data[i].type] & 1)) {
            safe_tprintf_str(buff, bufc, "%s", RetrieveValue(tempmech, i));
            return;
        }
    safe_tprintf_str(buff, bufc, "#-1 INVALID XCODE");
    return;
}

FUNCTION(fun_btarmorstatus_ref)
{
    /* fargs[0] = ref of the mech
     * fargs[1] = location to show
     */
    char *str;
    MECH *mech;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    str = armorstatus_func(mech, fargs[1]); 
    safe_tprintf_str(buff, bufc, str ? str : "#-1 ERROR");
}

FUNCTION(fun_btcritstatus_ref)
{
    /* fargs[0] = ref of the mech
     * fargs[1] = location to show
     */
    char *str;
    MECH *mech;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    str = critstatus_func(mech, fargs[1]);
    safe_tprintf_str(buff, bufc, str ? str : "#-1 ERROR");
}

FUNCTION(fun_btcritslot_ref)
{
    /* fargs[0] = ref
       fargs[1] = location name
       fargs[2] = critslot
       fargs[3] = partname type flag, 0 template name, 1 repair part name (differentiate Ammo types basically)
     */
    MECH *mech;
 
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
 
    if (!fn_range_check("BTCRITSLOT_REF", nfargs, 3, 4, buff, bufc))
        return;
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");
                                                                                                                                                                                      
    safe_tprintf_str(buff, bufc, "%s", critslot_func(mech, fargs[1], fargs[2], fargs[3]));
}

FUNCTION(fun_bttechlist_ref)
{
    dbref it;
    MECH *mech;
    int i, ii, part = 0, axe = 0, sword = 0, mace = 0, hascase = 0;
    char *infostr;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");

    infostr = techlist_func(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1");
}

FUNCTION(fun_btshowstatus_ref)
{
    dbref outplayer;
    MECH *mech;
    char *infostr;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    outplayer = match_thing(player, fargs[1]);
    FUNCHECK(outplayer == NOTHING || !Examinable(player, outplayer) || !isPlayer(outplayer), "#-1");

    mech_status(outplayer, (void *) mech, "R");
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btshowwspecs_ref)
{
    dbref outplayer;
    MECH *mech;
    char *infostr;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    outplayer = match_thing(player, fargs[1]);
    FUNCHECK(outplayer == NOTHING || !Examinable(player, outplayer) || !isPlayer(outplayer), "#-1");
                                                                                                                                                                                      
    mech_weaponspecs(outplayer, (void *) mech, "");
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btshowcritstatus_ref)
{
    dbref outplayer;
    MECH *mech;
    char *infostr;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    outplayer = match_thing(player, fargs[1]);
    FUNCHECK(outplayer == NOTHING || !Examinable(player, outplayer) || !isPlayer(outplayer), "#-1");

    mech_critstatus(outplayer, (void *) mech, fargs[2]);
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btdamagemech)
{
    /*
     * fargs[0] = dbref of MECH object
     * fargs[1] = total amount of damage
     * fargs[2] = clustersize
     * fargs[3] = direction of 'attack'
     * fargs[4] = (try to) force crit
     * fargs[5] = message to send to damaged 'mech
     * fargs[6] = message to MechLOSBroadcast, prepended by mech name
     */
                                                                                                                                                                                      
    int totaldam, clustersize, direction, iscrit;
    MECH *mech;
    dbref it;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    it = match_thing(player, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(player, it), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(it), "#-1 NOT A MECH");
    FUNCHECK(!(mech = FindObjectsData(it)), "#-1 UNABLE TO GET MECHDATA");
    FUNCHECK(Readnum(totaldam, fargs[1]), "#-1 INVALID 2ND ARG");
    FUNCHECK(Readnum(clustersize, fargs[2]), "#-1 INVALID 3RD ARG");
    FUNCHECK(Readnum(direction, fargs[3]), "#-1 INVALID 4TH ARG");
    FUNCHECK(Readnum(iscrit, fargs[4]), "#-1 INVALID 5TH ARG");
    safe_tprintf_str(buff, bufc, "%d", dodamage_func(player, mech,
            totaldam, clustersize, direction, iscrit, fargs[5], fargs[6]));
}

FUNCTION(fun_btweapstat)
{
    /* fargs[0] = weapon name
     * fargs[1] = stat type 
     */

    int i = -1, p, weapindx, val = -1;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
                                                                                                                                                                                      
    if (!find_matching_long_part(fargs[0], &i, &p)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p),
            "#-1 INVALID PART NAME");
    }
    if (!IsWeapon(p)) {
	safe_tprintf_str(buff, bufc, "#-1 NOT A WEAPON");
	return;
	}
    weapindx = Weapon2I(p);
    if (strcasecmp("VRT", fargs[1]) == 0)
	val = MechWeapons[weapindx].vrt;
    else if (strcasecmp("TYPE", fargs[1]) == 0)
	val = MechWeapons[weapindx].type;
    else if (strcasecmp("HEAT", fargs[1]) == 0)
	val = MechWeapons[weapindx].heat;
    else if (strcasecmp("DAMAGE", fargs[1]) == 0)
	val = MechWeapons[weapindx].damage;
    else if (strcasecmp("MIN", fargs[1]) == 0)
	val = MechWeapons[weapindx].min;
    else if (strcasecmp("SR", fargs[1]) == 0)
	val = MechWeapons[weapindx].shortrange;
    else if (strcasecmp("MR", fargs[1]) == 0)
	val = MechWeapons[weapindx].medrange;
    else if (strcasecmp("LR", fargs[1]) == 0)
	val = MechWeapons[weapindx].longrange;
    else if (strcasecmp("CRIT", fargs[1]) == 0)
	val = MechWeapons[weapindx].criticals;
    else if (strcasecmp("AMMO", fargs[1]) == 0)
	val = MechWeapons[weapindx].ammoperton;
    else if (strcasecmp("WEIGHT", fargs[1]) == 0)
	val = MechWeapons[weapindx].weight;
    else if (strcasecmp("BV", fargs[1]) == 0)
	val = MechWeapons[weapindx].battlevalue;
    else if (strcasecmp("ABV", fargs[1]) == 0)
	val = MechWeapons[weapindx].abattlevalue;
    else if (strcasecmp("REP", fargs[1]) == 0)
	val = MechWeapons[weapindx].reptime;
    else if (strcasecmp("WCLASS", fargs[1]) == 0)
	val = MechWeapons[weapindx].class;
    if (val == -1)
 	safe_tprintf_str(buff, bufc, "#-1"); 
    safe_tprintf_str(buff, bufc, "%d", val); 
}

FUNCTION(fun_btsetxy)
{
    /*
        fargs[0] = mech
        fargs[1] = map
        fargs[2] = x
        fargs[3] = y
        fargs[4] = z

    */
    dbref mechdb, mapdb;
    int x, y, z;
    MECH *mech;
    MAP *map;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mechdb = match_thing(player, fargs[0]);
    FUNCHECK(!Good_obj(mechdb), "#-1 INVALID TARGET");
    mech = getMech(mechdb);
    FUNCHECK(!mech, "#-1 INVALID TARGET"); 

    mapdb = match_thing(player, fargs[1]);
    FUNCHECK(mapdb == NOTHING || !Examinable(player, mapdb), "#-1 INVALID MAP");
    FUNCHECK(!IsMap(mapdb), "#-1 INVALID MAP");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");


    x = atoi(fargs[2]);
    y = atoi(fargs[3]);
    z = atoi(fargs[4]);
    FUNCHECK(x < 0 || x > map->map_width, "#-1 X COORD");
    FUNCHECK(y < 0 || y > map->map_height, "#-1 Y COORD");
    FUNCHECK(z < 0 || z > 10000, "#-1 Z COORD");

    mech_Rsetmapindex(GOD, (void *) mech, tprintf("%d", mapdb));
    mech_Rsetxy(GOD, (void *) mech, tprintf("%d %d %d", x, y, z));
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btgetbv_ref)
{
    MECH *mech;
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK((mech = loadfuncref(fargs[0])) == NULL, "#-1 NO SUCH MECH");

    MechBV(mech) = CalculateBV(mech, 4, 5);
    safe_tprintf_str(buff, bufc, "%d", MechBV(mech));
}

FUNCTION(fun_btengrate)
{
    dbref mechdb;
    MECH *mech;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mechdb = match_thing(player, fargs[0]);
    FUNCHECK(mechdb == NOTHING || !Examinable(player, mechdb), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(mechdb), "#-1 NOT A MECH");
    FUNCHECK(!(mech = getMech(mechdb)), "#-1");

    safe_tprintf_str(buff, bufc, "%d %d", MechEngineSize(mech), susp_factor(mech));
    return;
}

FUNCTION(fun_btengrate_ref)
{
    MECH *mech;
                                                                                                                                                                                      
    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK(!(mech = loadfuncref(fargs[0])), "#-1 INVALID REF");

    safe_tprintf_str(buff, bufc, "%d %d", MechEngineSize(mech), susp_factor(mech));
    return;
}

FUNCTION(fun_btlistblz)
{
    char buf[MBUF_SIZE] = { '\0' };
    dbref mapdb;
    MAP *map;
    mapobj *tmp;
    int i, count = 0, strcount = 0;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    mapdb = match_thing(player, fargs[0]);
    FUNCHECK(!Good_obj(mapdb), "#-1 INVALID MAP");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");

    for (i = 0; i < NUM_MAPOBJTYPES; i++)
	for (tmp = first_mapobj(map, i); tmp; tmp = next_mapobj(tmp))
	    if (i ==  TYPE_B_LZ) {
		count++;
		if (count == 1)
		    strcount += snprintf(buf + strcount, MBUF_SIZE - strcount, "%d %d %d", tmp->x, tmp->y, tmp->datai);
		else
		    strcount += snprintf(buf + strcount, MBUF_SIZE - strcount, "|%d %d %d", tmp->x, tmp->y, tmp->datai);
		}
    safe_tprintf_str(buff, bufc, buf);
}

FUNCTION(fun_bthexinblz)
{
    dbref mapdb;
    MAP *map;
    mapobj *o;
    int x, y, bl = 0;
    float fx, fy, tx, ty;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");

    mapdb = match_thing(player, fargs[0]);
    FUNCHECK(!Good_obj(mapdb), "#-1 INVALID MAP");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");
    x = atoi(fargs[1]);
    y = atoi(fargs[2]);
    FUNCHECK(x < 0 || y < 0 || x > map->map_width || y > map->map_height, "#-1 INVALID COORDS");
    MapCoordToRealCoord(x, y, &fx, &fy);

    for (o = first_mapobj(map, TYPE_B_LZ); o; o = next_mapobj(o)) {
        if (abs(x - o->x) > o->datai || abs(y - o->y) > o->datai)
            continue;
        MapCoordToRealCoord(o->x, o->y, &tx, &ty);
        if (FindHexRange(fx, fy, tx, ty) <= o->datai) {
	    bl = 1;
	    break;
	    }
	}
    safe_tprintf_str(buff, bufc, "%d", bl);
}

FUNCTION(fun_btfasabasecost_ref)
{
    MECH *mech;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    FUNCHECK(!(mech = loadfuncref(fargs[0])), "#-1 INVALID REF");

    safe_tprintf_str(buff, bufc, "%lld", CalcFasaCost(mech));
    return;

}

FUNCTION(fun_btunitfixable)
{
    MECH *mech;
    dbref mechdb;

    FUNCHECK(!WizR(player), "#-1 PERMISSION DENIED");
    mechdb = match_thing(player, fargs[0]);
    FUNCHECK(!Good_obj(mechdb), "#-1 INVALID TARGET");
    mech = getMech(mechdb);
    FUNCHECK(!mech, "#-1 INVALID TARGET"); 

    safe_tprintf_str(buff, bufc, "%d", unit_is_fixable(mech));
    return;
}
#endif
