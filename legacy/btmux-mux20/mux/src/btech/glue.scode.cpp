
/*
 * $Id: glue.scode.c,v 1.5 2005/08/08 09:43:09 murrayma Exp $
 *
 * Author: Markus Stenberg <fingon@iki.fi>
 *
 *  Copyright (c) 1996 Markus Stenberg
 *  Copyright (c) 1998-2002 Thomas Wouters 
 *  Copyright (c) 2000-2002 Cord Awtry 
 *  Copyright (c) 1999-2005 Kevin Stevens
 *       All rights reserved
 *
 * Created: Wed Oct  9 19:13:52 1996 fingon
 * Last modified: Tue Sep  8 10:00:29 1998 fingon
 *
 */

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "db.h"
#include "stringutil.h"
#include "alloc.h"
#include "functions.h"

#include <stdio.h>
#include <string.h>

#include "btmacros.h"
#include "mech.h"
#include "mech.events.h"
#include "glue.h"
#include "extern.h"
#include "coolmenu.h"
#include "mycool.h"
#include "turret.h"
#include "mech.custom.h"
#include "scen.h"
#include "p.template.h"
#include "p.mech.tech.h"
#include "p.mech.utils.h"
#include "p.mech.partnames.h"
#include "p.econ.h"
#include "p.map.obj.h"
#include "p.mech.tech.damages.h"
#include "p.mech.status.h"
#include "p.mech.sensor.h"
#include "p.btechstats.h"
#include "p.mech.combat.h"
#include "p.mech.damage.h"
#include "p.mechrep.h"
#include "p.mech.move.h"
#include "p.mech.los.h"
#include "p.event.h"
#include "p.mech.restrict.h"
#include "mech.partnames.h"
#include "p.mech.tech.commands.h"
#include "p.mech.consistency.h"
#include "p.glue.h"
#include "p.mech.notify.h"

extern SpecialObjectStruct SpecialObjects[];
dbref match_thing(dbref executor, char *name);
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
    TYPE_DBREF, TYPE_STRFUNC, TYPE_STRFUNC_S, TYPE_BV,
    TYPE_STRFUNC_BD, TYPE_CBV, TYPE_CHAR_RO, TYPE_SHORT_RO, TYPE_INT_RO,
    TYPE_FLOAT_RO, TYPE_DBREF_RO, TYPE_LAST_TYPE
};

/* INDENT OFF */
static int scode_in_out[TYPE_LAST_TYPE] =

/* st ch sh in fl db sf sfs bv sfb cbv ro-ch ro-sh ro-in ro-fl ro-db*/
{ 3, 3, 3, 3, 3, 3, 1, 2, 3, 3, 3, 1, 1, 1, 1, 1 };

/* INDENT ON */


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
        strncpy(MechType_Ref(mech), data, 14);
        MechType_Ref(mech)[14] = '\0';
        return NULL;
    } else
        return MechType_Ref(mech);
}

extern char *mech_types[];
extern char *move_types[];

char *mechTypefunc(int mode, MECH * mech, char *arg)
{
    int i;

    if (!mode)
        return mech_types[static_cast <short>(MechType(mech))];
    
    /* Should _alter_ mechtype.. weeeel. */
    if ((i = compare_array(mech_types, arg)) >= 0)
        MechType(mech) = i;
    
    return NULL;
}

char *mechMovefunc(int mode, MECH * mech, char *arg)
{
    int i;

    if (!mode)
        return move_types[static_cast <short>(MechMove(mech))];
    
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
                    SetPartTempNuke(mech, i, j, 0);
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
        } else 
            if (sscanf(s, "A(R):%d/%d", &i1, &i2) == 2) {
                /* Ordinary rear armor damage */
                if (i1 >= 0 && i1 < NUM_SECTIONS)
                    SetSectRArmor(mech, i1, GetSectORArmor(mech, i1) - i2);
            } else 
                if (sscanf(s, "I:%d/%d", &i1, &i2) == 2) {
                    /* Ordinary int damage */
                    if (i1 >= 0 && i1 < NUM_SECTIONS)
                        SetSectInt(mech, i1, GetSectOInt(mech, i1) - i2);
                } else 
                    if (sscanf(s, "C:%d/%d", &i1, &i2) == 2) {
                        /* Dest'ed crit */
                        if (i1 >= 0 && i1 < NUM_SECTIONS)
                            DestroyPart(mech, i1, i2);
                    } else 
                        if (sscanf(s, "G:%d/%d(%d)", &i1, &i2, &i3) == 3) {
                            /* Glitch */
                            if (i1 >= 0 && i1 < NUM_SECTIONS)
                                if (i2 >= 0 && i2 < NUM_CRITICALS)
                                    SetPartTempNuke(mech, i1, i2, i3);
                        } else 
                            if (sscanf(s, "R:%d/%d(%d)", &i1, &i2, &i3) == 3) {
                                /* Reload */
                                if (i1 >= 0 && i1 < NUM_SECTIONS)
                                    if (i2 >= 0 && i2 < NUM_CRITICALS)
                                        SetPartData(mech, i1, i2, FullAmmo(mech, i1, i2) - i3);
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
                } else 
                    if (!PartIsDestroyed(mech, i, j) && PartIsDestroyed(omech, i, j)) {
                        mech_RepairPart(omech, i, j);
                        SetPartTempNuke(omech, i, j, 0);
                        do_mag = 1;
                    }
        
                if (IsAmmo(GetPartType(mech, i, j))) {
                    if (GetPartData(mech, i, j) != GetPartData(omech, i, j))
                        SetPartData(omech, i, j, GetPartData(mech, i, j));
                } else {
                    if (PartTempNuke(mech, i, j) != PartTempNuke(omech, i, j))
                        SetPartTempNuke(omech, i, j, PartTempNuke(mech, i, j));
                }
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
    }
    
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
        for (j = 0; j < CritsInLoc(mech, i); j++) 
            if (GetPartType(mech, i, j) && !IsCrap(GetPartType(mech, i, j))) 
                if (PartIsDestroyed(mech, i, j)) {
                    ADD("C:%d/%d", i, j);
                } else 
                    if (IsAmmo(GetPartType(mech, i, j))) {
                        if (GetPartData(mech, i, j) != FullAmmo(mech, i, j)) {
                            ADD("R:%d/%d(%d)", i, j, FullAmmo(mech, i, j) - GetPartData(mech, i, j));
                        }
                    } else 
                        if (PartTempNuke(mech, i, j)) {
                            ADD("G:%d/%d(%d)", i, j, PartTempNuke(mech, i, j));
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

dbref IsValidMech(dbref executor, char *name, char *buff, char **bufc)
{
    dbref it = match_thing(executor, name);

    if(it == NOTHING || !Examinable(executor, it)) {
        safe_tprintf_str(buff, bufc, "#-1 NOT A MECH");
        return 0;
    }

    if(!IsMech(it)) {
        safe_tprintf_str(buff, bufc, "#-1 NOT A MECH");
        return 0;
    }

    return it;
}

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
        return j;        /* Allow 'old style' as well */

    /* Valid bitvector letters are: a-z (=27), A-Z (=27 more) */
    for (c = text; *c; c++) {
        if (*c == '!') {
            mode_not = 1;
            c++;
        }
        
        if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z')) {
            int k = bv_val(*c, 0);

            if (k) 
                if (mode_not)
                    j &= ~k;
                else
                    j |= k;
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

    MeEntryS("mechname", MechType_Name, TYPE_STRING, 31),
    MeEntry("maxspeed", MechMaxSpeed, TYPE_FLOAT),
    MeEntry("pilotnum", MechPilot, TYPE_DBREF),
    MeEntry("pilotdam", MechPilotStatus, TYPE_CHAR),
    MeEntry("si", AeroSI, TYPE_CHAR),
    MeEntry("si_orig", AeroSIOrig, TYPE_CHAR),
    MeEntry("speed", MechSpeed, TYPE_FLOAT),
    MeEntry("basewalkspeed", MechBaseWalk, TYPE_INT),
    MeEntry("baserunspeed", MechBaseRun, TYPE_INT),
    MeEntry("heading", MechRFacing, TYPE_SHORT),
    MeEntry("stall", MechStall, TYPE_INT),
    MeEntry("status", MechStatus, TYPE_BV),
    MeEntry("status2", MechStatus2, TYPE_BV),
    MeEntry("critstatus", MechCritStatus, TYPE_BV),
    MeEntry("tankcritstatus", MechTankCritStatus, TYPE_BV),
    MeEntry("target", MechTarget, TYPE_DBREF),
    MeEntry("team", MechTeam, TYPE_INT),
    MeEntry("tons", MechTons, TYPE_INT),
    MeEntry("towing", MechCarrying, TYPE_INT_RO),
    MeEntry("heat", MechPlusHeat, TYPE_FLOAT),
    MeEntry("disabled_hs", MechDisabledHS, TYPE_INT_RO),
    MeEntry("overheat", MechHeat, TYPE_FLOAT),
    MeEntry("dissheat", MechMinusHeat, TYPE_FLOAT),
    MeEntry("heatsinks", MechRealNumsinks, TYPE_CHAR_RO),
    MeEntry("last_startup", MechLastStartup, TYPE_INT),
    MeEntry("C3iNetworkSize", MechC3iNetworkSize, TYPE_INT_RO),
    MeEntry("MaxSuits", MechMaxSuits, TYPE_INT),
    MeEntry("realweight", MechRTonsV, TYPE_INT),
    MeEntry("StaggerDamage", StaggerDamage, TYPE_INT_RO),
    MeEntry("MechPrefs", MechPrefs, TYPE_BV),

    {GTYPE_MECH, "mechtype", (int) mechTypefunc, TYPE_STRFUNC_BD},
    {GTYPE_MECH, "mechmovetype", (int) mechMovefunc, TYPE_STRFUNC_BD},
    {GTYPE_MECH, "mechdamage", (int) mechDamagefunc, TYPE_STRFUNC_BD},
    {GTYPE_MECH, "techtime", (int) mechTechTimefunc, TYPE_STRFUNC},
    {GTYPE_MECH, "centdist", (int) mechCentDistfunc, TYPE_STRFUNC},
    {GTYPE_MECH, "centbearing", (int) mechCentBearingfunc, TYPE_STRFUNC},
    {GTYPE_MECH, "sensors", (int) mechSensorInfo, TYPE_STRFUNC},
    {GTYPE_MECH, "mechref", (int) mech_getset_ref, TYPE_STRFUNC_BD},

    MeEntry("fuel", AeroFuel, TYPE_INT),
    MeEntry("fuel_orig", AeroFuelOrig, TYPE_INT),
    MeEntry("cocoon", MechCocoon, TYPE_INT_RO),
    MeEntry("numseen", MechNumSeen, TYPE_SHORT),

    MeEntry("fx", MechFX, TYPE_FLOAT),
    MeEntry("fy", MechFY, TYPE_FLOAT),
    MeEntry("fz", MechFZ, TYPE_FLOAT),
    MeEntry("x", MechX, TYPE_SHORT),
    MeEntry("y", MechY, TYPE_SHORT),
    MeEntry("z", MechZ, TYPE_SHORT),

    MeEntry("targcomp", MechTargComp, TYPE_CHAR),
    MeEntry("lrsrange", MechLRSRange, TYPE_CHAR),
    MeEntry("radiorange", MechRadioRange, TYPE_SHORT),
    MeEntry("scanrange", MechScanRange, TYPE_CHAR),
    MeEntry("tacrange", MechTacRange, TYPE_CHAR),
    MeEntry("radiotype", MechRadioType, TYPE_CHAR),
    MeEntry("bv", MechBV, TYPE_INT),
    MeEntry("cargospace", CargoSpace, TYPE_INT),
    MeEntry("carmaxton", CarMaxTon, TYPE_CHAR_RO),

    MeVEntry("bay0", AeroBay, 0, TYPE_DBREF),
    MeVEntry("bay1", AeroBay, 1, TYPE_DBREF),
    MeVEntry("bay2", AeroBay, 2, TYPE_DBREF),
    MeVEntry("bay3", AeroBay, 3, TYPE_DBREF),

    MeVEntry("turret0", AeroTurret, 0, TYPE_DBREF),
    MeVEntry("turret1", AeroTurret, 1, TYPE_DBREF),
    MeVEntry("turret2", AeroTurret, 2, TYPE_DBREF),

    MeEntry("unusablearcs", AeroUnusableArcs, TYPE_INT_RO),
    MeEntry("maxjumpspeed", MechJumpSpeed, TYPE_FLOAT),
    MeEntry("jumpheading", MechJumpHeading, TYPE_SHORT),
    MeEntry("jumplength", MechJumpLength, TYPE_SHORT),

    MaEntry("buildflag", buildflag, TYPE_CHAR),
    MaEntry("buildonmap", onmap, TYPE_DBREF_RO),
    MaEntry("cf", cf, TYPE_SHORT),
    MaEntry("cfmax", cfmax, TYPE_SHORT),
    MaEntry("gravity", grav, TYPE_CHAR),
    MaEntry("maxcf", cfmax, TYPE_SHORT),
    MaEntry("firstfree", first_free, TYPE_CHAR_RO),
    MaEntry("mapheight", map_height, TYPE_SHORT_RO),
    MaEntry("maplight", maplight, TYPE_CHAR),
    MaEntryS("mapname", mapname, TYPE_STRING, 30),
    MaEntry("mapvis", mapvis, TYPE_CHAR),
    MaEntry("mapwidth", map_width, TYPE_SHORT_RO),
    MaEntry("maxvis", maxvis, TYPE_SHORT),
    MaEntry("temperature", temp, TYPE_CHAR),
    MaEntry("winddir", winddir, TYPE_SHORT),
    MaEntry("windspeed", windspeed, TYPE_SHORT),
    MaEntry("cloudbase", cloudbase, TYPE_SHORT),
    MaEntry("flags", flags, TYPE_CBV),
    MaEntry("sensorflags", sensorflags, TYPE_BV),

    TuEntry("arcs", arcs, TYPE_INT),
    TuEntry("parent", parent, TYPE_DBREF),
    TuEntry("gunner", gunner, TYPE_DBREF),
    TuEntry("target", target, TYPE_DBREF),
    TuEntry("targx", target, TYPE_SHORT),
    TuEntry("targy", target, TYPE_SHORT),
    TuEntry("targz", target, TYPE_SHORT),
    TuEntry("lockmode", lockmode, TYPE_INT),

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
    MeEntry("perception", MechPer, TYPE_INT),

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
    void *(*tempfun) (int, MECH *, char *);

    it = match_thing(executor, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    for (i = 0; xcode_data[i].name; i++)
        if (!strcasecmp(fargs[1], xcode_data[i].name) && xcode_data[i].gtype == spec && (scode_in_out[xcode_data[i].type] & 2)) {
            bar = reinterpret_cast <void *>((reinterpret_cast <int>(foo) + xcode_data[i].rel_addr));
        
            switch (xcode_data[i].type) {
                case TYPE_STRFUNC_BD:
                case TYPE_STRFUNC_S:
                    tempfun = (void *(*)(int, MECH *, char *)) xcode_data[i].rel_addr;
                    tempfun(1, static_cast <MECH *>(foo), static_cast <char *>(fargs[2]));
                    break;
                case TYPE_STRING:
                    strncpy(static_cast <char *>(bar), fargs[2], xcode_data[i].size - 1);
                    (static_cast <char *>(bar))[xcode_data[i].size - 1] = '\0';
                    break;
                case TYPE_DBREF:
                    *(static_cast <dbref *>(bar)) = atoi(fargs[2]);
                    break;
                case TYPE_CHAR:
                    *(static_cast <char *>(bar)) = atoi(fargs[2]);
                    break;
                case TYPE_SHORT:
                    *(static_cast <short *>(bar)) = atoi(fargs[2]);
                    break;
                case TYPE_INT:
                    *(static_cast <int *>(bar)) = atoi(fargs[2]);
                    break;
                case TYPE_FLOAT:
                    *(static_cast <float *>(bar)) = atof(fargs[2]);
                    break;
                case TYPE_BV:
                    *(static_cast <int *>(bar)) = text2bv(fargs[2]);
                    break;
                case TYPE_CBV:
                    *(static_cast <byte * >(bar)) = (byte) text2bv(fargs[2]);
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
    void *bar = reinterpret_cast <void *>((reinterpret_cast <int>(data) + xcode_data[i].rel_addr));
    static char buf[LBUF_SIZE];
    char *(*tempfun) (int, MECH *);

    switch (xcode_data[i].type) {
        case TYPE_STRFUNC_BD:
        case TYPE_STRFUNC:
            tempfun = (char *(*)(int, MECH *)) xcode_data[i].rel_addr;
            sprintf(buf, "%s", static_cast <char *>(tempfun(0, static_cast <MECH *>(data))));
            break;
        case TYPE_STRING:
            sprintf(buf, "%s", static_cast <char *>(bar));
            break;
        case TYPE_DBREF:
        case TYPE_DBREF_RO:
            sprintf(buf, "%d", static_cast <dbref>(*(static_cast <dbref *>(bar))));
            break;
        case TYPE_CHAR:
        case TYPE_CHAR_RO:
            sprintf(buf, "%d", static_cast <char>(*(static_cast <char *>(bar))));
            break;
        case TYPE_SHORT:
        case TYPE_SHORT_RO:
            sprintf(buf, "%d", static_cast <short>(*(static_cast <short *>(bar))));
            break;
        case TYPE_INT:
        case TYPE_INT_RO:
            sprintf(buf, "%d", static_cast <int>(*(static_cast <int *>(bar))));
            break;
        case TYPE_FLOAT:
        case TYPE_FLOAT_RO:
            sprintf(buf, "%.2f", static_cast <float>(*(static_cast <float *>(bar))));
            break;
        case TYPE_BV:
            strcpy(buf, bv2text(static_cast <int>(*(static_cast <int *>(bar)))));
            break;
        case TYPE_CBV:
            strcpy(buf, bv2text(static_cast <int>(*(static_cast <char *>(bar)))));
            break;
    }
    
    return buf;
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

    it = match_thing(executor, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    
    for (i = 0; xcode_data[i].name; i++)
        if (!strcasecmp(fargs[1], xcode_data[i].name) && xcode_data[i].gtype == spec && (scode_in_out[xcode_data[i].type] & 1)) {
            safe_tprintf_str(buff, bufc, "%s", RetrieveValue(foo, i));
            return;
        }
    
    safe_tprintf_str(buff, bufc, "#-1");
}

FUNCTION(fun_btgetxcodevalue_ref)
{
    /* fargs[0] = mech ref
       fargs[1] = name of the value
     */
    int i;
    MECH *foo;
    int spec;

    FUNCHECK((foo = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    spec = GTYPE_MECH;
    
    for (i = 0; xcode_data[i].name; i++)
        if (!strcasecmp(fargs[1], xcode_data[i].name) && xcode_data[i].gtype == spec && (scode_in_out[xcode_data[i].type] & 1)) {
            safe_tprintf_str(buff, bufc, "%s", RetrieveValue(foo, i));
            return;
        }
    
    safe_tprintf_str(buff, bufc, "#-1");
}

void set_xcodestuff(dbref player, void *data, char *buffer)
{
    char *args[2];
    int t, i;
    void *bar;
    void *(*tempfun) (int, MECH *, char *);

    memset(args, 0, sizeof(char *) * 2);

    DOCHECK(silly_parseattributes(buffer, args, 2) != 2, "Invalid arguments!");
    t = WhichSpecial(Location(player));
    
    for (i = 0; xcode_data[i].name; i++)
        if (xcode_data[i].gtype == t)
            break;
    
    DOCHECK(!xcode_data[i].name, "Error: No xcode values for this type of object found.");

    for (i = 0; xcode_data[i].name; i++)
        if (!strcasecmp(args[0], xcode_data[i].name) && xcode_data[i].gtype == t && (scode_in_out[xcode_data[i].type] & 2))
            break;
    
    DOCHECK(!xcode_data[i].name, "Error: No matching xcode value for this type of object found.");

    bar = reinterpret_cast <void *>((reinterpret_cast <int>(FindObjectsData(Location(player))) + xcode_data[i].rel_addr));

    switch (xcode_data[i].type) {
        case TYPE_STRFUNC_BD:
        case TYPE_STRFUNC_S:
            tempfun = (void *(*)(int, MECH *, char *)) xcode_data[i].rel_addr;
            tempfun(1, getMech(Location(player)), static_cast <char *>(args[1]));
            break;
        case TYPE_STRING:
            strncpy(static_cast < char *>(bar), args[1], xcode_data[i].size - 1);
            (static_cast <char *>(bar))[xcode_data[i].size - 1] = '\0';
            break;
        case TYPE_DBREF:
            *(static_cast <dbref *>(bar)) = atoi(args[1]);
            break;
        case TYPE_CHAR:
            *(static_cast <char *>(bar)) = atoi(args[1]);
            break;
        case TYPE_SHORT:
            *(static_cast <short *>(bar)) = atoi(args[1]);
            break;
        case TYPE_INT:
            *(static_cast <int *>(bar)) = atoi(args[1]);
            break;
        case TYPE_FLOAT:
            *(static_cast <float *>(bar)) = atof(args[1]);
            break;
        case TYPE_BV:
            *(static_cast <int *>(bar)) = text2bv(args[1]);
            break;
        case TYPE_CBV:
            *(static_cast <byte *>(bar)) = static_cast <byte>(text2bv(args[1]));
    }
}

void list_xcodestuff(dbref player, void *data, char *buffer)
{
    int t, i, flag = CM_TWO, se_len = 37;
    coolmenu *c = NULL;

    t = WhichSpecial(Location(player));

    for (i = 0; xcode_data[i].name; i++)
        if (xcode_data[i].gtype == t && (scode_in_out[xcode_data[i].type] & 1))
            break;
    
    DOCHECK(!xcode_data[i].name, "Error: No xcode values for this type of object found.");

    addline();
    cent(tprintf("Data for %s (%s)", Name(Location(player)), SpecialObjects[t].type));
    addline();

    if (*buffer == '1') {
        flag = CM_ONE;
        se_len = se_len * 2;
    }
    
    if (*buffer == '4') {
        flag = CM_FOUR;
        se_len = se_len / 2;
    }
    
    if (*buffer == '1' || *buffer == '4')
        buffer++;
    
    for (i = 0; xcode_data[i].name; i++) {
        if (xcode_data[i].gtype == t && (scode_in_out[xcode_data[i].type] & 1)) {
            /* 1/3(left) = name, 2/3(right)=value */
            char mask[SBUF_SIZE];
            char lab[SBUF_SIZE];

            if (*buffer)
                if (strncasecmp(xcode_data[i].name, buffer, strlen(buffer)))
                    continue;
        
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


FUNCTION(fun_btunderrepair)
{
    /* fargs[0] = ref of the mech to be checked */
    int n;
    MECH *mech;
    dbref it;

    it = match_thing(executor, fargs[0]);
    
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");
    FUNCHECK(!IsMech(it), "#-2");
    
    mech = static_cast <MECH *>(FindObjectsData(it));
    n = figure_latest_tech_event(mech);
    safe_tprintf_str(buff, bufc, "%d", n > 0);
}


FUNCTION(fun_btstores)
{
    /* fargs[0] = id of the bay/mech */
    /* fargs[1] = (optional) name of the part */
    dbref it;
    int i = -1, x = 0;
    int p, b;
    int pile[BRANDCOUNT + 1][NUM_ITEMS];
    char *t;

    it = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(it), "#-1 INVALID TARGET");

    if (nfargs > 1) {
       i = -1;
        if (!find_matching_long_part(fargs[1], &i, &p, &b)) {
            i = -1;
            FUNCHECK(!find_matching_vlong_part(fargs[1], &i, &p, &b), "#-1 INVALID PART NAME");
        }
    
        safe_tprintf_str(buff, bufc, "%d", econ_find_items(it, p, b));
    } else {
        memset(pile, 0, sizeof(pile));

        dbref aowner;
        int aflags;
        ATTR *pattr;

        if ((pattr = atr_str("ECONPARTS")) && pattr->number)
            t = atr_get(it, pattr->number, &aowner, &aflags);
        else {
            safe_tprintf_str(buff, bufc, "#-1 NO ECONPARTS");
            return;
        }

        while (*t) {
            if (*t == '[')
                if ((sscanf(t, "[%d,%d,%d]", &i, &p, &b)) == 3)
                    pile[p][i] += b;
            t++;
        }
    
        for (i = 0; i < object_count; i++) {
            UNPACK_PART(short_sorted[i]->index, p, b);
            if (pile[b][p]) {
                if (x)
                    safe_str("|", buff, bufc);
        
                x = pile[b][p];
                safe_tprintf_str(buff, bufc, "%s:%d", part_name_long(p, b), x);
            }
        }
    }
}

FUNCTION(fun_btmapterr)
{
    /* fargs[0] = reference of map
       fargs[1] = x
       fargs[2] = y */
    dbref it;
    MAP *map;
    int x, y;
    int spec;
    char terr;

    it = match_thing(executor, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");

    spec = WhichSpecial(it);
    
    FUNCHECK(spec != GTYPE_MAP, "#-1");
    FUNCHECK(!(map = static_cast < MAP * >(FindObjectsData(it))), "#-1");
    FUNCHECK(Readnum(x, fargs[1]), "#-2");
    FUNCHECK(Readnum(y, fargs[2]), "#-2");
    FUNCHECK(x < 0 || y < 0 || x >= map->map_width || y >= map->map_height, "?");
    
    terr = GetTerrain(map, x, y);
    if (terr == GRASSLAND)
        terr = '.';

    safe_tprintf_str(buff, bufc, "%c", terr);
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

    it = match_thing(executor, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");
    
    spec = WhichSpecial(it);
    
    FUNCHECK(spec != GTYPE_MAP, "#-1");
    FUNCHECK(!(map = static_cast < MAP * >(FindObjectsData(it))), "#-1");
    FUNCHECK(Readnum(x, fargs[1]), "#-2");
    FUNCHECK(Readnum(y, fargs[2]), "#-2");
    FUNCHECK(x < 0 || y < 0 || x >= map->map_width || y >= map->map_height, "?");

    i = Elevation(map, x, y);
    
    if (i < 0)
        safe_tprintf_str(buff, bufc, "-%c", '0' + -i);
    else
        safe_tprintf_str(buff, bufc, "%c", '0' + i);
}

void list_xcodevalues(dbref executor)
{
    int i;

    notify(executor, "Xcode attributes accessible thru get/setxcodevalue:");

    for (i = 0; xcode_data[i].name; i++)
        notify(executor, tprintf("\t%d\t%s", xcode_data[i].gtype, xcode_data[i].name));
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

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");
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

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");
    
    critstr = critstatus_func(mech, fargs[1]);    /* fargs[1] unguaranteed ! */
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

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");

    infostr = armorstatus_func(mech, fargs[1]);    /* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_btweaponstatus)
{
    /* fargs[0] = id of the mech
     * fargs[1] = location to show
     */
    dbref it;
    char *infostr;
    MECH *mech;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");
    
    infostr = weaponstatus_func(mech, nfargs == 2 ? fargs[1] : NULL);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_btcritstatus_ref)
{
    /* fargs[0] = ref of the mech
     * fargs[1] = location to show
     */
    char *critstr;
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    critstr = critstatus_func(mech, fargs[1]);    /* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, critstr ? critstr : "#-1 ERROR");
}

FUNCTION(fun_btarmorstatus_ref)
{
    /* fargs[0] = ref of the mech
     * fargs[1] = location to show
     */
    char *infostr;
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    infostr = armorstatus_func(mech, fargs[1]);    /* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_btweaponstatus_ref)
{
    /* fargs[0] = ref of the mech
     * fargs[1] = location to show
     */
    char *infostr;
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    infostr = weaponstatus_func(mech, nfargs == 2 ? fargs[1] : NULL);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
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

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");
    
    infostr = setarmorstatus_func(mech, fargs[1], fargs[2], fargs[3]);    /* fargs[1] unguaranteed ! */
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_btthreshold)
{
    /*
     * fargs[0] = skill to query
     */
    int xpth;

    xpth = btthreshold_func(fargs[0]);
    safe_tprintf_str(buff, bufc, xpth < 0 ? "#%d ERROR" : "%d", xpth);
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

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1 UNABLE TO GET MECHDATA");
    FUNCHECK(Readnum(totaldam, fargs[1]) || totaldam < 1, "#-1 INVALID 2ND ARG");
    FUNCHECK(Readnum(clustersize, fargs[2]) || clustersize < 1, "#-1 INVALID 3RD ARG");
    FUNCHECK(Readnum(direction, fargs[3]), "#-1 INVALID 4TH ARG");
    FUNCHECK(Readnum(iscrit, fargs[4]), "#-1 INVALID 5TH ARG");

    safe_tprintf_str(buff, bufc, "%d", dodamage_func(executor, mech, totaldam, clustersize, direction, iscrit, fargs[5], fargs[6]));
}

FUNCTION(fun_bttechstatus)
{
    /*
     * fargs[0] = dbref of MECH object
     */

    dbref it;
    MECH *mech;
    char *infostr;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1 UNABLE TO GET MECHDATA");
    infostr = techstatus_func(mech);
    safe_tprintf_str(buff, bufc, "%s", infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_bttech_ref)
{
    /* fargs[0] = ref of the mech
     */
    char *infostr;
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    infostr = techstatus_func(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_bthexemit)
{
    /* fargs[0] = mapref
       fargs[1] = x coordinate
       fargs[2] = y coordinate
       fargs[3] = message
     */
    MAP *map;
    int x = -1, y = -1;
    char *msg = fargs[3];
    dbref mapnum;

    while (msg && *msg && mux_isspace(*msg))
    msg++;
    FUNCHECK(!msg || !*msg, "#-1 INVALID MESSAGE");

    mapnum = match_thing(executor, fargs[0]);
    FUNCHECK(mapnum < 0, "#-1 INVALID MAP");
    map = getMap(mapnum);
    FUNCHECK(!map, "#-1 INVALID MAP");

    x = atoi(fargs[1]);
    y = atoi(fargs[2]);
    FUNCHECK(x < 0 || x > map->map_width || y < 0 || y > map->map_height,
    "#-1 INVALID COORDINATES");
    HexLOSBroadcast(map, x, y, msg);
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btmakepilotroll)
{
    /* fargs[0] = mechref
       fargs[1] = roll modifier
       fargs[2] = damage modifier
     */

    MECH *mech;
    int rollmod = 0, dammod = 0;
    dbref mechnum;

    if(!(mechnum = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech =
        static_cast <MECH *>(FindObjectsData(mechnum))),
    "#-1 INVALID MECH");

    /* No checking on rollmod/dammod, they're assumed to be 0 if invalid. */
    rollmod = atoi(fargs[1]);
    dammod = atoi(fargs[2]);

    if (MadePilotSkillRoll(mech, rollmod)) {
    safe_tprintf_str(buff, bufc, "1");
    } else {
    MechFalls(mech, dammod, 1);
    safe_tprintf_str(buff, bufc, "0");
    }
}

FUNCTION(fun_btgetreftech_ref)
{
    /* fargs[0] = mech reference */
    char *infostr;
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    infostr = mechrep_gettechstring(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1 ERROR");
}

FUNCTION(fun_btid2db)
{
    /* fargs[0] = mech 
       fargs[1] = target ID */
    MECH *target;
    MECH *mech = NULL;
    dbref mechnum;

    mechnum = match_thing(executor, fargs[0]);
    FUNCHECK(mechnum == NOTHING || !Examinable(executor, mechnum), "#-1 INVALID MECH/MAP");
    FUNCHECK(strlen(fargs[1]) != 2, "#-1 INVALID TARGETID");
    if (IsMech(mechnum)) {
        FUNCHECK(!(mech = getMech(mechnum)), "#-1 INVALID MECH");
        mechnum = FindTargetDBREFFromMapNumber(mech, fargs[1]);
    } else 
        if (IsMap(mechnum)) {
            MAP *map;

            FUNCHECK(!(map = getMap(mechnum)), "#-1 INVALID MAP");
            mechnum = FindMechOnMap(map, fargs[1]);
        } else {
            safe_str("#-1 INVALID MECH/MAP", buff, bufc);
            return;
        }
    
    FUNCHECK(mechnum < 0, "#-1 INVALID TARGETID");
    
    if (mech) {
        FUNCHECK(!(target = getMech(mechnum)), "#-1 INVALID TARGETID");
        FUNCHECK(!InLineOfSight_NB(mech, target, MechX(target), MechY(target), FlMechRange(getMap(mech->mapindex), mech, target)), "#-1 INVALID TARGETID");
    }

    safe_tprintf_str(buff, bufc, "#%d", static_cast < int >(mechnum));
}

FUNCTION(fun_bthexlos)
{
    /* fargs[0] = mech 
       fargs[1] = x
       fargs[2] = y
     */

    MECH *mech;
    MAP *map;
    int x = -1, y = -1, mechnum;
    float fx, fy;

    if(!(mechnum = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = getMech(mechnum)), "#-1 INVALID MECH");
    FUNCHECK(!(map = getMap(mech->mapindex)), "#-1 INTERNAL ERROR");

    x = atoi(fargs[1]);
    y = atoi(fargs[2]);
    FUNCHECK(x < 0 || x > map->map_width || y < 0 || y > map->map_height, "#-1 INVALID COORDINATES");
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

    int mechnum;
    MECH *mech, *target;

    if(!(mechnum = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = getMech(mechnum)), "#-1 INVALID MECH");

    mechnum = match_thing(executor, fargs[1]);
    FUNCHECK(mechnum == NOTHING || !Examinable(executor, mechnum), "#-1 INVALID MECH");
    FUNCHECK(!IsMech(mechnum), "#-1 INVALID MECH");
    FUNCHECK(!(target = getMech(mechnum)), "#-1 INVALID MECH");

    if (InLineOfSight_NB(mech, target, MechX(mech), MechY(mech), FlMechRange(getmap(mech->mapindex), mech, target)))
        safe_tprintf_str(buff, bufc, "1");
    else
        safe_tprintf_str(buff, bufc, "0");
}

FUNCTION(fun_btaddparts)
{
    /* fargs[0] = mech/map
       fargs[1] = partname
       fargs[2] = quantity
     */
    int loc;
    int index = -1, id = 0, brand = 0, count;

    loc = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(loc), "#-1 INVALID TARGET");
    FUNCHECK(strlen(fargs[1]) >= MBUF_SIZE, "#-1 PARTNAME TOO LONG");
    FUNCHECK(!fargs[1], "#-1 NEED PARTNAME");
    count = atoi(fargs[2]);
    FUNCHECK(!count, "1");
    FUNCHECK(!find_matching_short_part(fargs[1], &index, &id, &brand) && !find_matching_vlong_part(fargs[1], &index, &id, &brand) && !find_matching_long_part(fargs[1], &index, &id, &brand), "0");
    econ_change_items(loc, id, brand, count);
    SendEcon(tprintf("#%d added %d %s to #%d", executor, count, get_parts_vlong_name(id, brand), loc));
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btloadmap)
{
    /* fargs[0] = mapobject
       fargs[1] = mapname
       fargs[2] = clear or not to clear
     */
    int mapdbref;
    MAP *map;

    notify(executor, tprintf("map dir: %s", MAP_PATH));
    mapdbref = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(mapdbref), "#-1 INVALID TARGET");
    map = getMap(mapdbref);
    FUNCHECK(!map, "#-1 INVALID TARGET");

    switch (map_load(map, fargs[1])) {
        case -1:
            safe_str("#-1 MAP NOT FOUND", buff, bufc);
            return;
        case -2:
            safe_str("#-1 INVALID MAP", buff, bufc);
            return;
        case 0:
            break;
    }
    
    if (nfargs > 2 && xlate(fargs[2]))
        map_clearmechs(executor, static_cast <void *>(map), "");

    safe_str("1", buff, bufc);
}

FUNCTION(fun_btloadmech)
{
    /* fargs[0] = mechobject
       fargs[1] = mechref
     */
    int mechdbref;
    MECH *mech;

    mechdbref = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(mechdbref), "#-1 INVALID TARGET");

    mech = getMech(mechdbref);
    FUNCHECK(!mech, "#-1 INVALID TARGET");
    
    if (mech_loadnew(executor, mech, fargs[1]) == 1) {
        muxevent_remove_data(static_cast < void *>(mech));
        clear_mech_from_LOS(mech);
        safe_str("1", buff, bufc);
    } else {
        safe_str("#-1 UNABLE TO LOAD TEMPLATE", buff, bufc);
    }
}

const char radio_colorstr[] = "xrgybmcwXRGYBMCW";

FUNCTION(fun_btmechfreqs)
{
    /* fargs[0] = mechobject
     */
    int mechdbref;
    MECH *mech;
    int i;

    mechdbref = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(mechdbref), "#-1 INVALID TARGET");
    mech = getMech(mechdbref);
    FUNCHECK(!mech, "#-1 INVALID TARGET");

    for (i = 0; i < MFreqs(mech); i++) {
        if (i)
            safe_str(",", buff, bufc);
    
        safe_tprintf_str(buff, bufc, "%d|%d|%s", i + 1, mech->freq[i], bv2text(mech->freqmodes[i] % FREQ_REST));

        if (mech->freqmodes[i] / FREQ_REST) {
            safe_tprintf_str(buff, bufc, "|%c",
            radio_colorstr[mech->freqmodes[i] / FREQ_REST - 1]);
        } else {
            safe_str("|-", buff, bufc);
        }
    }
}

FUNCTION(fun_btgetweight)
{
    /*
       fargs[0] = stringname of part
     */
    float sw = 0;
    int i = -1, p, b;


    if (!find_matching_long_part(fargs[0], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p, &b), "#-1 INVALID PART NAME");
    }
    
    sw = GetPartWeight(p);
    if (sw <= 0)
        sw = (1024 * 100);
    
    safe_tprintf_str(buff, bufc, tprintf("%.3f", (float) sw / 1024));
}

FUNCTION(fun_btaddstores)
{
    /* fargs[0] = id of the bay */
    /* fargs[1] = name of the part */
    /* fargs[2] = amount */
    dbref it;
    int i = -1, spec, num;
    void *foo;
    int p, b;

    it = match_thing(executor, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(Readnum(num, fargs[2]), "#-2 Illegal Value");
    
    if (!find_matching_long_part(fargs[1], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[1], &i, &p, &b), "#-1 INVALID PART NAME");
    }

    econ_change_items(it, p, b, num);
    safe_tprintf_str(buff, bufc, "%d", econ_find_items(it, p, b));
}

FUNCTION(fun_btremovestores)
{
    /* fargs[0] = id of the bay */
    /* fargs[1] = name of the part */
    /* fargs[2] = amount */
    dbref it;
    int i = -1, spec;
    int num = 0;
    void *foo;
    int p, b;

    it = match_thing(executor, fargs[0]);
    FUNCHECK(it == NOTHING || !Examinable(executor, it), "#-1");
    spec = WhichSpecial(it);
    FUNCHECK(!(foo = FindObjectsData(it)), "#-1");
    FUNCHECK(Readnum(num, fargs[2]), "#-2 Illegal Value");
    
    if (!find_matching_long_part(fargs[1], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[1], &i, &p, &b), "#-1 INVALID PART NAME");
    }
    
    econ_change_items(it, p, b, 0 - num);
    safe_tprintf_str(buff, bufc, "%d", econ_find_items(it, p, b));
}

FUNCTION(fun_bttechtime)
{
    CLinearTimeAbsolute old, ltaNow;
    CLinearTimeDelta ltd;

    dbref aowner;
    int aflags;
    ATTR *pattr;
    char *olds;

    if ((pattr = atr_str("TECHTIME")) && pattr->number)
        olds = atr_get(executor, pattr->number, &aowner, &aflags);
    else
        olds = "";

    char buf[MBUF_SIZE];

    ltaNow.GetUTC();

    if (olds) {
        old.SetString(olds);
        if (old < ltaNow) 
            strcpy(buf, "00:00.00");
        else {
            ltd = old - ltaNow;
            sprintf(buf, "%02ld:%02d.%02d", (ltd.ReturnSeconds() / 3600), ((ltd.ReturnSeconds() / 60) % 60), (ltd.ReturnSeconds() % 60));
        }
    } else 
        strcpy(buf, "00:00.00");

    notify(executor, buf);
}

FUNCTION(fun_btcritslot)
{
    /* fargs[0] = id of the mech
       fargs[1] = location name
       fargs[2] = critslot
       fargs[3] = partname type flag, 0 template name, 1 repair part name (differentiate Ammo types basically) */
    dbref it;
    MECH *mech;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1 INVALID MECH");

    safe_tprintf_str(buff, bufc, "%s", critslot_func(mech, fargs[1], fargs[2], fargs[3]));
}

FUNCTION(fun_btcritslot_ref)
{
    /* fargs[0] = ref
       fargs[1] = location name
       fargs[2] = critslot
       fargs[3] = partname type flag, 0 template name, 1 repair part name (differentiate Ammo types basically)
     */
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");

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

    mapdb = match_thing(executor, fargs[0]);
    FUNCHECK(mapdb == NOTHING || !Examinable(executor, mapdb), "#-1 INVALID MAPDB");
    FUNCHECK(!IsMap(mapdb), "#-1 OBJECT NOT HCODE");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");

    if (nfargs == 3 && fargs[1][0] == '#' && fargs[2][0] == '#') {
        mechAdb = match_thing(executor, fargs[1]);
        FUNCHECK(mechAdb == NOTHING || !Examinable(executor, mechAdb), "#-1 INVALID MECHDBREF");
        mechBdb = match_thing(executor, fargs[2]);
        FUNCHECK(mechBdb == NOTHING || !Examinable(executor, mechBdb), "#-1 INVALID MECHDBREF");
        FUNCHECK(!IsMech(mechAdb) || !IsMech(mechBdb), "#-1 INVALID MECH");
        FUNCHECK(!(mechA = getMech(mechAdb)) || !(mechB = getMech(mechBdb)), "#-1 INVALID MECH");
        FUNCHECK(mechA->mapindex != mapdb || mechB->mapindex != mapdb, "#-1 MECH NOT ON MAP");
        safe_tprintf_str(buff, bufc, "%f", FaMechRange(mechA, mechB));
        return;
    } else 
        if (nfargs == 4 && fargs[1][0] == '#') {
            mechAdb = match_thing(executor, fargs[1]);
            FUNCHECK(mechAdb == NOTHING || !Examinable(executor, mechAdb), "#-1 INVALID MECHDBREF");
            FUNCHECK(!IsMech(mechAdb), "#-1 INVALID MECH");
            FUNCHECK(!(mechA = getMech(mechAdb)), "#-1 INVALID MECH");
            FUNCHECK(mechA->mapindex != mapdb, "#-1 MECH NOT ON MAP");
            xA = atoi(fargs[2]);
            yA = atoi(fargs[3]);
            FUNCHECK(xA < 0 || yA < 0 || xA > map->map_width || yA > map->map_height, "#-1 INVALID COORDS");
            MapCoordToRealCoord(xA, yA, &fxA, &fyA);
            safe_tprintf_str(buff, bufc, "%f", FindRange(MechFX(mechA), MechFY(mechA), MechFZ(mechA), fxA, fyA, Elevation(map, xA, yA) * ZSCALE));
            return;
        } else 
            if (nfargs == 5) {
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

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");

    MechMaxSpeed(mech) = newmaxspeed;
    correct_speed(mech);

    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btgetrealmaxspeed)
{
    dbref it;
    MECH *mech;
    float speed;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");

    speed = MechCargoMaxSpeed(mech, MechMaxSpeed(mech));

    safe_tprintf_str(buff, bufc, tprintf("%f", speed));
}

FUNCTION(fun_btgetbv)
{
    dbref it;
    MECH *mech;
    int bv;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");

    bv = CalculateBV(mech, 100, 100);
    MechBV(mech) = bv;
    safe_tprintf_str(buff, bufc, tprintf("%d", bv));
}

FUNCTION(fun_btgetbv_ref)
{
    MECH *mech;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");

    MechBV(mech) = CalculateBV(mech, 4, 5);
    safe_tprintf_str(buff, bufc, "%d", MechBV(mech));
}

FUNCTION(fun_bttechlist)
{
    dbref it;
    MECH *mech;
    int i, ii, part = 0, axe = 0, sword = 0, mace = 0, hascase = 0;
    char *infostr;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    FUNCHECK(!(mech = static_cast <MECH *>(FindObjectsData(it))), "#-1");
    infostr = techlist_func(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : " ");
}

FUNCTION(fun_bttechlist_ref)
{
    dbref it;
    MECH *mech;
    int i, ii, part = 0, axe = 0, sword = 0, mace = 0, hascase = 0;
    char *infostr;


    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    infostr = techlist_func(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1");
}

/* Function to return the 'payload' of a unit
 * ie: the Guns and Ammo
 * in a list format like <item_1> <# of 1>|...|<item_n> <# of n>
 * Dany - 06/2005 */
FUNCTION(fun_btpayload_ref)
{
    MECH *mech;
    char *infostr;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    infostr = payloadlist_func(mech);
    safe_tprintf_str(buff, bufc, infostr ? infostr : "#-1");
}

FUNCTION(fun_btshowstatus_ref)
{
    dbref outexecutor;
    MECH *mech;
    char *infostr;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    outexecutor = match_thing(executor, fargs[1]);
    FUNCHECK(outexecutor == NOTHING || !Examinable(executor, outexecutor) || !isPlayer(outexecutor), "#-1");

    mech_status(outexecutor, static_cast <void *>(mech), "R");
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btshowwspecs_ref)
{
    dbref outexecutor;
    MECH *mech;
    char *infostr;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    outexecutor = match_thing(executor, fargs[1]);
    FUNCHECK(outexecutor == NOTHING || !Examinable(executor, outexecutor) || !isPlayer(outexecutor), "#-1");

    mech_weaponspecs(outexecutor, static_cast <void *>(mech), "");
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btshowcritstatus_ref)
{
    dbref outexecutor;
    MECH *mech;
    char *infostr;

    FUNCHECK((mech = load_refmech(fargs[0])) == NULL, "#-1 NO SUCH MECH");
    outexecutor = match_thing(executor, fargs[1]);
    FUNCHECK(outexecutor == NOTHING || !Examinable(executor, outexecutor) || !isPlayer(outexecutor), "#-1");

    mech_critstatus(outexecutor, static_cast < void *>(mech), fargs[2]);
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btengrate)
{
    dbref mechdb;
    MECH *mech;

    if(!(mechdb = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    mechdb = match_thing(executor, fargs[0]);
    FUNCHECK(mechdb == NOTHING || !Examinable(executor, mechdb), "#-1 NOT A MECH");
    FUNCHECK(!IsMech(mechdb), "#-1 NOT A MECH");
    FUNCHECK(!(mech = getMech(mechdb)), "#-1");

    safe_tprintf_str(buff, bufc, "%d %d", MechEngineSize(mech),
    susp_factor(mech));
}

FUNCTION(fun_btengrate_ref)
{
    MECH *mech;

    FUNCHECK(!(mech = load_refmech(fargs[0])), "#-1 INVALID REF");
    safe_tprintf_str(buff, bufc, "%d %d", MechEngineSize(mech),
    susp_factor(mech));
}

FUNCTION(fun_btfasabasecost_ref)
{
    MECH *mech;

    FUNCHECK(!(mech = load_refmech(fargs[0])), "#-1 INVALID REF");
    safe_tprintf_str(buff, bufc, "%lld", CalcFasaCost(mech));
}

FUNCTION(fun_btweapstat)
{
    /* fargs[0] = weapon name
     * fargs[1] = stat type */

    int i = -1, p, weapindx, val = -1, b;

    if (!find_matching_long_part(fargs[0], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p, &b), "#-1 INVALID PART NAME");
    }
    
    if (!IsWeapon(p)) {
        safe_tprintf_str(buff, bufc, "#-1 NOT A WEAPON");
        return;
    }
    
    weapindx = Weapon2I(p);
    if (strcasecmp("VRT", fargs[1]) == 0)
        val = MechWeapons[weapindx].vrt;
    else 
        if (strcasecmp("TYPE", fargs[1]) == 0)
            val = MechWeapons[weapindx].type;
        else 
            if (strcasecmp("HEAT", fargs[1]) == 0)
                val = MechWeapons[weapindx].heat;
            else 
                if (strcasecmp("DAMAGE", fargs[1]) == 0)
                    val = MechWeapons[weapindx].damage;
                else 
                    if (strcasecmp("MIN", fargs[1]) == 0)
                        val = MechWeapons[weapindx].min;
                    else 
                        if (strcasecmp("SR", fargs[1]) == 0)
                            val = MechWeapons[weapindx].shortrange;
                        else 
                            if (strcasecmp("MR", fargs[1]) == 0)
                                val = MechWeapons[weapindx].medrange;
                            else 
                                if (strcasecmp("LR", fargs[1]) == 0)
                                    val = MechWeapons[weapindx].longrange;
                                else 
                                    if (strcasecmp("CRIT", fargs[1]) == 0)
                                        val = MechWeapons[weapindx].criticals;
                                    else 
                                        if (strcasecmp("AMMO", fargs[1]) == 0)
                                            val = MechWeapons[weapindx].ammoperton;
                                        else 
                                            if (strcasecmp("WEIGHT", fargs[1]) == 0)
                                                val = MechWeapons[weapindx].weight;
                                            else 
                                                if (strcasecmp("BV", fargs[1]) == 0)
                                                    val = MechWeapons[weapindx].battlevalue;
    if (val == -1)
        safe_tprintf_str(buff, bufc, "#-1");
    
    safe_tprintf_str(buff, bufc, "%d", val);
}

FUNCTION(fun_btnumrepjobs)
{
    extern int damage_last;
    MECH *mech;
    dbref it;

    if(!(it = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    mech = static_cast <MECH *>(FindObjectsData(it));

    if (unit_is_fixable(mech))
        make_damage_table(mech);
    else
        make_scrap_table(mech);

    safe_tprintf_str(buff, bufc, "%d", damage_last);
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


    if(!(mechdb = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    mechdb = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(mechdb), "#-1 INVALID TARGET");
    mech = getMech(mechdb);
    FUNCHECK(!mech, "#-1 INVALID TARGET");

    mapdb = match_thing(executor, fargs[1]);
    FUNCHECK(mapdb == NOTHING || !Examinable(executor, mapdb), "#-1 INVALID MAP");
    FUNCHECK(!IsMap(mapdb), "#-1 INVALID MAP");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");

    x = atoi(fargs[2]);
    FUNCHECK(x < 0 || x > map->map_width, "#-1 X COORD");
    y = atoi(fargs[3]);
    FUNCHECK(y < 0 || y > map->map_height, "#-1 Y COORD");
    z = atoi(fargs[4]);
    FUNCHECK(z < 0 || z > 10000, "#-1 Z COORD");

    mech_Rsetmapindex(GOD, static_cast <void *>(mech), tprintf("%d", mapdb));
    mech_Rsetxy(GOD, static_cast <void *>(mech), tprintf("%d %d %d", x, y, z));
    safe_tprintf_str(buff, bufc, "1");
}

FUNCTION(fun_btmapemit)
{
    /* fargs[0] = mapref
       fargs[1] = message
     */
    MAP *map;
    dbref mapnum;

    FUNCHECK(!fargs[1] || !*fargs[1], "#-1 INVALID MESSAGE");

    mapnum = match_thing(executor, fargs[0]);
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
    int i = -1, p, b;

    if (!find_matching_long_part(fargs[0], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p, &b), "#-1 INVALID PART NAME");
    }
    
    if (strstr(fargs[0], "Sword") && !strstr(fargs[0], "PC."))
        p = I2Special(SWORD);
    
    if (IsWeapon(p)) {
        safe_tprintf_str(buff, bufc, "WEAP");
        return;
    } else 
        if (IsAmmo(p) || strstr(fargs[0], "Ammo_")) {
            safe_tprintf_str(buff, bufc, "AMMO");
            return;
        } else 
            if (IsBomb(p)) {
                safe_tprintf_str(buff, bufc, "BOMB");
                return;
            } else 
                if (IsSpecial(p)) {
                    safe_tprintf_str(buff, bufc, "PART");
                    return;
                } else 
                    if (btechconf.btech_complexrepair && IsCargo(p) && Cargo2I(p) >= TON_SENSORS_FIRST && Cargo2I(p) <= TON_ENGINE_COMP_LAST) {
                        safe_tprintf_str(buff, bufc, "PART");
                        return;
                    } else 
                        if (IsCargo(p)) {
                            safe_tprintf_str(buff, bufc, "CARG");
                            return;
                        } else {
                            safe_tprintf_str(buff, bufc, "OTHER");
                            return;
                        }
}

FUNCTION(fun_btgetpartcost)
{
    int i = -1, p, index, b;

    if (!find_matching_long_part(fargs[0], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p, &b), "#-1 INVALID PART NAME");
    }
    
    if (strstr(fargs[0], "Sword") && !strstr(fargs[0], "PC."))
        p = I2Special(SWORD);

    safe_tprintf_str(buff, bufc, "%lld", GetPartCost(p));
}

FUNCTION(fun_btsetpartcost)
{
    int i = -1, p, index, b;
    unsigned long long int cost;

    if (!find_matching_long_part(fargs[0], &i, &p, &b)) {
        i = -1;
        FUNCHECK(!find_matching_vlong_part(fargs[0], &i, &p, &b), "#-1 INVALID PART NAME");
    }
    
    if (strstr(fargs[0], "Sword") && !strstr(fargs[0], "PC."))
        p = I2Special(SWORD);
    
    cost = atoll(fargs[1]);
    if (cost < 0) {
        safe_tprintf_str(buff, bufc, "#-1 COST ERROR");
        return;
    }
    
    SetPartCost(p, cost);
    safe_tprintf_str(buff, bufc, "%lld", cost);
}

FUNCTION(fun_btunitfixable)
{
    MECH *mech;
    dbref mechdb;

    if(!(mechdb = IsValidMech(executor, fargs[0], buff, bufc)))
        return;

    mechdb = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(mechdb), "#-1 INVALID TARGET");
    mech = getMech(mechdb);
    FUNCHECK(!mech, "#-1 INVALID TARGET");

    safe_tprintf_str(buff, bufc, "%d", unit_is_fixable(mech));
}

FUNCTION(fun_btlistblz)
{
    char buf[MBUF_SIZE] = { '\0' };
    dbref mapdb;
    MAP *map;
    mapobjt *tmp;
    int i, count = 0, strcount = 0;


    mapdb = match_thing(executor, fargs[0]);
    FUNCHECK(!Good_obj(mapdb), "#-1 INVALID MAP");
    FUNCHECK(!(map = getMap(mapdb)), "#-1 INVALID MAP");

    for (i = 0; i < NUM_MAPOBJTYPES; i++)
        for (tmp = first_mapobj(map, i); tmp; tmp = next_mapobj(tmp))
            if (i == TYPE_B_LZ) {
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
    mapobjt *o;
    int x, y, bl = 0;
    float fx, fy, tx, ty;

    mapdb = match_thing(executor, fargs[0]);
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
