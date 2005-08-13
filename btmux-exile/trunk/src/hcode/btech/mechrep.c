#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>
#include <dirent.h>
#include <sys/stat.h>

#define MECH_STAT_C		/* want to use the POSIX stat() call. */

#include "config.h"
#include "externs.h"
#include "mech.h"
#include "mechrep.h"
#include "create.h"
#include "p.mech.build.h"
#include "p.mech.status.h"
#include "p.template.h"
#include "p.mechrep.h"
#include "p.mech.restrict.h"
#include "p.mech.consistency.h"
#include "p.mech.utils.h"

/* Selectors */
#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

extern char *strtok(char *s, const char *ct);

/* EXTERNS THAT SHOULDN'T BE IN HERE! */
extern void *FindObjectsData(dbref key);
dbref match_thing(dbref player, char *name);
void event_remove_data(void *data);

#define MECHREP_COMMON(a) \
struct mechrep_data *rep = (struct mechrep_data *) data; \
MECH *mech; \
DOCHECK(!Template(player), "I'm sorry Dave, can't do that."); \
if (!CheckData(player, rep)) return; \
if (a) { DOCHECK(rep->current_target == -1, "You must set a target first!"); \
mech = getMech (rep->current_target); \
if (!CheckData(player, mech)) return; }

/*--------------------------------------------------------------------------*/

/* Code Begins                                                              */

/*--------------------------------------------------------------------------*/

/* Alloc free function */

/* Alloc/free routine */

void newfreemechrep(dbref key, void **data, int selector)
{
    struct mechrep_data *new = *data;

    switch (selector) {
    case SPECIAL_ALLOC:
	new->current_target = -1;
	break;
    }
}

/* With cap R means restricted command */

void mechrep_Rresetcrits(dbref player, void *data, char *buffer)
{
    int i;

    MECHREP_COMMON(1);
    notify(player, "Default criticals set!");
    for (i = 0; i < NUM_SECTIONS; i++)
	FillDefaultCriticals(mech, i);
}

void mechrep_Rdisplaysection(dbref player, void *data, char *buffer)
{
    char *args[1];
    int index;

    MECHREP_COMMON(1);
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"You must specify a section to list the criticals for!");
    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    DOCHECK(index == -1, "Invalid section!");
    CriticalStatus(player, mech, index);
}

#define MechComputersRadioRange(mech) \
(DEFAULT_RADIORANGE * generic_radio_multiplier(mech))

void mechrep_Rsetradio(dbref player, void *data, char *buffer)
{
    char *args[2];
    int i;

    MECHREP_COMMON(1);
    switch (mech_parseattributes(buffer, args, 2)) {
    case 0:
	notify(player,
	    "This remains to be done [showing of stuff when no args]");
	return;
    case 2:
	notify(player, "Too many args, unable to cope().");
	return;
    case 1:
	break;
    }
    i = BOUNDED(1, atoi(args[0]), 5);
    notify(player, tprintf("Radio level set to %d.", i));
    MechRadio(mech) = i;
    MechRadioType(mech) = generic_radio_type(MechRadio(mech), 0);
    notify(player, tprintf("Number of freqs: %d  Extra stuff: %d",
	    MechRadioType(mech) % 16, (MechRadioType(mech) / 16) * 16));
    MechRadioRange(mech) = MechComputersRadioRange(mech);
    notify(player, tprintf("Radio range set to %d.",
	    (int) MechRadioRange(mech)));
}

void mechrep_Rsettarget(dbref player, void *data, char *buffer)
{
    char *args[2];
    int newmech;

    MECHREP_COMMON(0);
    switch (mech_parseattributes(buffer, args, 2)) {
    case 1:
	newmech = match_thing(player, args[0]);
	DOCHECK(!(Good_obj(newmech) &&
		Hardcode(newmech)),
	    "That is not a BattleMech or Vehicle!");
	rep->current_target = newmech;
	notify(player, tprintf("Mech to repair changed to #%d", newmech));
	break;
    default:
	notify(player, "Too many arguments!");
    }
}

void mechrep_Rsettype(dbref player, void *data, char *buffer)
{
    char *args[1];

    MECHREP_COMMON(1);
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"Invalid number of arguments!");
    switch (toupper(args[0][0])) {
    case 'M':
	MechType(mech) = CLASS_MECH;
	MechMove(mech) = MOVE_BIPED;
	notify(player, "Type set to MECH");
	break;
    case 'Q':
	MechType(mech) = CLASS_MECH;
	MechMove(mech) = MOVE_QUAD;
	notify(player, "Type set to QUAD");
	break;
    case 'G':
	MechType(mech) = CLASS_VEH_GROUND;
	notify(player, "Type set to VEHICLE");
	break;
    case 'V':
	MechType(mech) = CLASS_VEH_VTOL;
	MechMove(mech) = MOVE_VTOL;
	notify(player, "Type set to VTOL");
	break;
    case 'N':
	MechType(mech) = CLASS_VEH_NAVAL;
	notify(player, "Type set to NAVAL");
	break;
    case 'A':
	MechType(mech) = CLASS_AERO;
	MechMove(mech) = MOVE_FLY;
	notify(player, "Type set to AeroSpace");
	break;
    case 'D':
	MechType(mech) = CLASS_DS;
	MechMove(mech) = MOVE_FLY;
	notify(player, "Type set to DropShip");
	break;
    case 'B':
	MechType(mech) = CLASS_BSUIT;
	MechMove(mech) = MOVE_BIPED;
	notify(player, "Type set to Battlesuit");
	break;
    default:
	notify(player,
	    "Types are: MECH, GROUND, VTOL, NAVAL, AERO and DROPSHIP");
	break;
    }
}

#define SETVALUE_FUNCTION_FLOAT(funcname,valname,valstring,modifier) \
void funcname (dbref player, void *data, char *buffer) \
{ char *args[1]; float f; MECHREP_COMMON(1); \
  DOCHECK(mech_parseattributes (buffer, args, 1) != 1, \
	  tprintf("Invalid number of arguments to Set%s!", valstring)); \
  f = atof(args[0]); \
  valname = f * modifier; \
  notify(player, tprintf("%s changed to %.2f.", valstring, valname)); \
}


#define SETVALUE_FUNCTION_INT(funcname,valname,valstring,modifier) \
void funcname (dbref player, void *data, char *buffer) \
{ char *args[1]; int f; MECHREP_COMMON(1); \
  DOCHECK(mech_parseattributes (buffer, args, 1) != 1, \
	  tprintf("Invalid number of arguments to Set%s!", valstring)); \
  f = atoi(args[0]); \
  valname = f * modifier; \
  notify(player, tprintf("%s changed to %d.", valstring, valname)); \
}

SETVALUE_FUNCTION_FLOAT(mechrep_Rsetspeed, MechMaxSpeed(mech), "Maxspeed",
    KPH_PER_MP);
SETVALUE_FUNCTION_FLOAT(mechrep_Rsetjumpspeed, MechJumpSpeed(mech),
    "Jumpspeed", KPH_PER_MP);
SETVALUE_FUNCTION_INT(mechrep_Rsetheatsinks, MechRealNumsinks(mech),
    "Heatsinks", 1);
SETVALUE_FUNCTION_INT(mechrep_Rsetlrsrange, MechLRSRange(mech), "LRSrange",
    1);
SETVALUE_FUNCTION_INT(mechrep_Rsettacrange, MechTacRange(mech), "TACrange",
    1);
SETVALUE_FUNCTION_INT(mechrep_Rsetscanrange, MechScanRange(mech),
    "SCANrange", 1);
SETVALUE_FUNCTION_INT(mechrep_Rsetradiorange, MechRadioRange(mech),
    "RADIOrange", 1);
SETVALUE_FUNCTION_INT(mechrep_Rsettons, MechTons(mech), "Tons", 1);


void mechrep_Rsetmove(dbref player, void *data, char *buffer)
{
    char *args[1];

    MECHREP_COMMON(1);
    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"Invalid number of arguments!");
    switch (toupper(args[0][0])) {
    case 'T':
	MechMove(mech) = MOVE_TRACK;
	notify(player, "Movement set to TRACKED");
	break;
    case 'W':
	MechMove(mech) = MOVE_WHEEL;
	notify(player, "Movement set to WHEELED");
	break;
    case 'H':
	switch (toupper(args[0][1])) {
	case 'O':
	    MechMove(mech) = MOVE_HOVER;
	    notify(player, "Movement set to HOVER");
	    break;
	case 'U':
	    MechMove(mech) = MOVE_HULL;
	    notify(player, "Movement set to HULL");
	    break;
	}
	break;
    case 'V':
	MechMove(mech) = MOVE_VTOL;
	notify(player, "Movement set to VTOL");
	break;
    case 'Q':
	MechMove(mech) = MOVE_QUAD;
	notify(player, "Movement set to QUAD");
	break;
    case 'S':
	MechMove(mech) = MOVE_SUB;
	notify(player, "Movement set to SUB");
	break;
    case 'F':
	switch (toupper(args[0][1])) {
	case 'O':
	    MechMove(mech) = MOVE_FOIL;
	    notify(player, "Movement set to FOIL");
	    break;
	case 'L':
	    MechMove(mech) = MOVE_FLY;
	    notify(player, "Movement set to FLY");
	    break;
	}
	break;
    default:
	notify(player,
	    "Types are: TRACK, WHEEL, VTOL, HOVER, HULL, FLY, SUB and FOIL");
	break;
    }
}

/*
 * Implement a name cache of a template names.  This allows differences
 * in case and characters past the 14th to be ignored in mech references
 * when loading templates.  Templates can also be stored in any subdirectory
 * of the main template directory instead of in just one of list of hard
 * coded subdirectories.  
 * 
 * CACHE_MAXNAME sets the limit on how long a template filename can be.
 * Any template with a filename longer than this is ignored and not stored
 * in the cache.
 *
 * MECH_MAXREF sets the number of signficant characters in a mechref when
 * searching the cache.  This should be equal to the length of the
 * 'mech_type' (minus one for the terminating '\0' character) field of
 * MECH structure.
 * 
 */

enum {
    CACHE_MAXNAME = 24,
    MECH_MAXREF = 14
};

struct tmpldirent {
    char name[CACHE_MAXNAME + 1];
    char const *dir;
};

struct tmpldir {
    char name[CACHE_MAXNAME + 1];
    struct tmpldir *next;
};

struct tmpldirent *tmpl_list = NULL;
int tmpl_pos = 0;
int tmpl_len = 0;

struct tmpldir *tmpldir_list = NULL;

/*
 * The ordering function for the template name cache.  Used to sort and
 * search the cache.
 */
static int tmplcmp(void const *v1, void const *v2)
{
    struct tmpldirent const *p1 = v1;
    struct tmpldirent const *p2 = v2;

    return strncasecmp(p1->name, p2->name, MECH_MAXREF);
}

/*
 * Add all the template names in a directory to the template cache.
 */
static int scan_template_dir(char const *dirname, char const *parent)
{
    char buf[1000];
    int dirnamelen = strlen(dirname);
    DIR *dir = opendir(dirname);

    if (dir == NULL) {
	return -1;
    }

    while (1) {
	struct stat sb;
	struct dirent *ent = readdir(dir);

	if (ent == NULL) {
	    break;
	}

	if (dirnamelen + 1 + strlen(ent->d_name) + 1 > sizeof buf) {
	    continue;
	}

	sprintf(buf, "%s/%s", dirname, ent->d_name);
	if (stat(buf, &sb) == -1) {
	    continue;
	}

	if (parent == NULL && S_ISDIR(sb.st_mode)
	    && ent->d_name[0] != '.' &&
	    strlen(ent->d_name) <= CACHE_MAXNAME) {
	    struct tmpldir *link;

	    Create(link, struct tmpldir, 1);

	    strcpy(link->name, ent->d_name);
	    link->next = tmpldir_list;
	    tmpldir_list = link;
	    continue;
	}

	if (!S_ISREG(sb.st_mode)) {
	    continue;
	}

	if (tmpl_pos == tmpl_len) {
	    if (tmpl_len == 0) {
		tmpl_len = 4;
		Create(tmpl_list, struct tmpldirent, tmpl_len);
	    } else {
		tmpl_len *= 2;
		ReCreate(tmpl_list, struct tmpldirent, tmpl_len);
	    }
	}

	strncpy(tmpl_list[tmpl_pos].name, ent->d_name, CACHE_MAXNAME);
	tmpl_list[tmpl_pos].name[CACHE_MAXNAME] = '\0';
	tmpl_list[tmpl_pos].dir = parent;
	tmpl_pos++;
    }

    closedir(dir);
    return 0;
}

/*
 * Scan all the template names in the mech template directory.  Only looks
 * in the mech template directory and it immediate subdirectories. 
 * It doesn't recursively look any further down the tree.
 */

static int scan_templates(char const *dir)
{
    char buf[1000];
    struct tmpldir *p;

    if (scan_template_dir(dir, NULL) == -1) {
	return -1;
    }

    p = tmpldir_list;
    while (p != NULL) {
	sprintf(buf, "%s/%s", dir, p->name);
	scan_template_dir(buf, p->name);
	p = p->next;
    }

    qsort(tmpl_list, tmpl_pos, sizeof tmpl_list[0], tmplcmp);

    return 0;
}

/*
 * Free all the memory used by the template cache.  Sets the cache to
 * the empty state.
 */
static void free_template_list()
{
    struct tmpldir *p;

    free(tmpl_list);

    p = tmpldir_list;
    while (p != NULL) {
	struct tmpldir *np = p->next;

	free(p);
	p = np;
    }

    tmpl_list = NULL;
    tmpldir_list = NULL;
    tmpl_pos = 0;
    tmpl_len = 0;

    return;
}

char *subdirs[] = {
    "3025",
    "3050",
    "3055",
    "3058",
    "3060",
    "2750",
    "Aero",
    "MISC",
    "Clan",
    "ClanVehicles",
    "Clan2nd",
    "ClanAero",
    "Custom",
    "Solaris",
    "Vehicles",
    "MFNA",
    "Infantry",
    "AeroBase",
    "Base",
    "ClanShort",
    "DropJump",
    "ISAero",
    "ISOmni", 
    "Exile",
    "3062",
    "3062IS",
    "3062Clan",
    "3062Vehicles",
    NULL
};

void mechrep_Rloadnew(dbref player, void *data, char *buffer)
{
    char *args[1];

    MECHREP_COMMON(1);
    if (mech_parseattributes(buffer, args, 1) == 1)
	if (mech_loadnew(player, mech, args[0]) == 1) {
	    event_remove_data((void *) mech);
	    clear_mech_from_LOS(mech);
	    notify(player, "Template loaded.");
	    return;
	}
    notify(player, "Unable to read that template.");
}

void clear_mech(MECH * mech, int flag)
{
    int i, j;

    mech->brief = 1;

    bzero(&mech->rd, sizeof(mech_rd));
    bzero(&mech->ud, sizeof(mech_ud));

    MechSpotter(mech) = -1;
    MechTarget(mech) = -1;
    MechChargeTarget(mech) = -1;
    MechChargeTimer(mech) = 0;
    MechChargeDistance(mech) = 0;
    MechSwarmTarget(mech) = -1;
    MechDFATarget(mech) = -1;
    MechTargX(mech) = -1;
    MechStatus(mech) = 0;
    MechTargY(mech) = -1;
    MechPilot(mech) = -1;
    MechAim(mech) = NUM_SECTIONS;
    if (flag) {
	for (i = 0; i < NUM_TICS; i++)
	    for (j = 0; j < TICLONGS; j++)
		mech->tic[i][j] = 0;
	for (i = 0; i < FREQS; i++) {
	    mech->freq[i] = 0;
	    mech->freqmodes[i] = 0;
	    mech->chantitle[i][0] = 0;
	}
    }
}

char *mechref_path(char *id)
{
    static char openfile[1024];
    FILE *fp;
    int i;

    /*
     * If the template name doesn't have slash search for it in the
     * template name cache.
     */
    if (strchr(id, '/') == NULL && (tmpl_list != NULL ||
	    scan_templates(MECH_PATH) != -1)) {
	struct tmpldirent *ent;
	struct tmpldirent key;

	strncpy(key.name, id, CACHE_MAXNAME);
	key.name[CACHE_MAXNAME] = '\0';

	ent =
	    bsearch(&key, tmpl_list, tmpl_pos, sizeof tmpl_list[0],
	    tmplcmp);
	if (ent == NULL) {
	    return NULL;
	}
	if (ent->dir == NULL) {
	    sprintf(openfile, "%s/%s", MECH_PATH, ent->name);
	} else {
	    sprintf(openfile, "%s/%s/%s", MECH_PATH, ent->dir, ent->name);
	}
	return openfile;
    }

    /*
     * Look up a template name the old way...
     */
    sprintf(openfile, "%s/%s", MECH_PATH, id);
    fp = fopen(openfile, "r");
    for (i = 0; !fp && subdirs[i]; i++) {
	sprintf(openfile, "%s/%s/%s", MECH_PATH, subdirs[i], id);
	fp = fopen(openfile, "r");
    }
    if (fp) {
	fclose(fp);
	return openfile;
    }
    return NULL;
}

int load_mechdata2(dbref player, MECH * mech, char *id)
{
    FILE *fp = NULL;
    char *filename;

    filename = mechref_path(id);

    if (!filename)
	return 0;
    if (!(fp = fopen(filename, "r")))
	return 0;
    fclose(fp);
    return load_template(player, mech, filename) >= 0 ? 1 : 0;
}

extern int num_def_weapons;

int unable_to_find_proper_type(int i)
{
    if (!i)
	return 0;
    if (IsWeapon(i)) {
	if (i > (num_def_weapons))
	    return 1;
    }
    if (IsAmmo(i)) {
	if ((Ammo2Weapon(i) + 1) > (num_def_weapons))
	    return 1;
    }
    if (IsSpecial(i))
	if (Special2I(i) >= count_special_items())
	    return 1;
    return 0;
}

int load_mechdata(MECH * mech, char *id)
{
    FILE *fp = NULL;
    int i, j, k, t;
    int i1, i2, i3, i4, i5, i6;
    char *filename;

    filename = mechref_path(id);
    TEMPLATE_GERR(filename == NULL, tprintf("No matching file for '%s'.",
	    id));
    if (filename)
	fp = fopen(filename, "r");
    TEMPLATE_GERR(!fp, tprintf("Unable to open file %s (%s)!", filename,
	    id));
    strncpy(MechType_Ref(mech), id, MECHREF_SIZE);
    MechType_Ref(mech)[MECHREF_SIZE - 1] = '\0';
    TEMPLATE_GERR(fscanf(fp, "%d %d %d %d %d %f %f %d\n", &i1, &i2, &i3,
	    &i4, &i5, &MechMaxSpeed(mech), &MechJumpSpeed(mech), &i6) < 8,
	"Old template loading system: %s is invalid template file.", id);
    MechTons(mech) = i1;
    MechTacRange(mech) = i2;
    MechLRSRange(mech) = i3;
    MechScanRange(mech) = i4;
    MechRealNumsinks(mech) = i5;
#define DROP(a) \
  if (i6 & a) i6 &= ~a
    DROP(32768);		/* Quad */
    DROP(16384);		/* Salvagetech */
    DROP(8192);			/* Cargotech */
    DROP(4196);			/* Watergun */
    MechSpecials(mech) = i6;
    for (k = 0; k < NUM_SECTIONS; k++) {
	i = k;
	if (MechType(mech) == 4) {
	    switch (k) {
	    case 3:
		i = 4;
		break;
	    case 4:
		i = 5;
		break;
	    case 5:
		i = 3;
		break;
	    }
	}
	TEMPLATE_GERR(fscanf(fp, "%d %d %d %d\n", &i1, &i2, &i3, &i4) < 4,
	    "Insufficient data reading section %d!", i);
	MechSections(mech)[i].recycle = 0;
	SetSectArmor(mech, i, i1);
	SetSectOArmor(mech, i, i1);
	SetSectInt(mech, i, i2);
	SetSectOInt(mech, i, i2);
	SetSectRArmor(mech, i, i3);
	SetSectORArmor(mech, i, i3);
	/* Remove all rampant AXEs from the arms themselves, we do
	   things differently here */
	if (i4 & 4)
	    i4 &= ~4;
	MechSections(mech)[i].config = i4;
	for (j = 0; j < NUM_CRITICALS; j++) {
	    TEMPLATE_GERR(fscanf(fp, "%d %d %d\n", &i1, &i2, &i3) < 3,
		"Insufficient data reading critical %d/%d!", i, j);
	    MechSections(mech)[i].criticals[j].type = i1;
	    TEMPLATE_GERR(unable_to_find_proper_type(GetPartType(mech, i,
			j)), "Invalid datatype at %d/%d!", i, j);
	    if (IsSpecial(i1))
		i1 += SPECIAL_BASE_INDEX - OSPECIAL_BASE_INDEX;
	    if (IsWeapon(GetPartType(mech, i, j)) &&
		IsAMS((t = Weapon2I(GetPartType(mech, i, j))))) {
		if (MechWeapons[t].special & CLAT)
		    MechSpecials(mech) |= CL_ANTI_MISSILE_TECH;
		else
		    MechSpecials(mech) |= IS_ANTI_MISSILE_TECH;
	    }
	    MechSections(mech)[i].criticals[j].data = i2;
	    MechSections(mech)[i].criticals[j].mode = i3;
	}
    }
    if (fscanf(fp, "%d %d\n", &i1, &i2) == 2) {
	MechType(mech) = i1;
	TEMPLATE_GERR(MechType(mech) > CLASS_LAST, "Invalid 'mech type!");
	MechMove(mech) = i2;
	TEMPLATE_GERR(MechMove(mech) > MOVENEMENT_LAST,
	    "Invalid movenement type!");
    }
    if (fscanf(fp, "%d\n", &i1) != 1)
	MechRadioRange(mech) = DEFAULT_RADIORANGE;
    else
	MechRadioRange(mech) = i1;
    fclose(fp);
    return 1;
}

#undef  LOADNEW_LOADS_OLD_IF_FAIL
#define LOADNEW_LOADS_MUSE_FORMAT

int mech_loadnew(dbref player, MECH * mech, char *id)
{
    char mech_origid[100];

    strncpy(mech_origid, MechType_Ref(mech), 100);

    if (!strcmp(mech_origid, id)) {
	clear_mech(mech, 0);
	if (load_mechdata2(player, mech, id) <= 0)
	    return load_mechdata(mech, id) > 0;
	return 1;
    } else {
	clear_mech(mech, 1);
	if (load_mechdata2(player, mech, id) < 1)
#ifdef LOADNEW_LOADS_MUSE_FORMAT
	    if (load_mechdata(mech, id) < 1)
#endif
#ifdef LOADNEW_LOADS_OLD_IF_FAIL
		if (load_mechdata2(player, mech, mech_origid) < 1)
#ifdef LOADNEW_LOADS_MUSE_FORMAT
		    if (load_mechdata(mech, mech_origid) < 1)
#endif
#endif
			return 0;
    }
    return 1;
}

void mechrep_Rrestore(dbref player, void *data, char *buffer)
{
    char *c;

    MECHREP_COMMON(1);
    c = silly_atr_get(mech->mynum, A_MECHREF);
    DOCHECK(!c || !*c, "Sorry, I don't know what type of mech this is");
    DOCHECK(mech_loadnew(player, mech, c) == 1, "Restoration complete!");
    notify(player, "Unable to restore this mech!.");
}

void mechrep_Rsavetemp(dbref player, void *data, char *buffer)
{
    char *args[1];
    FILE *fp;
    char openfile[512];
    int i, j;

    MECHREP_COMMON(1);

    free_template_list();

    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"You must specify a template name!");
    DOCHECK(strstr(args[0], "/"), "Invalid file name!");
    notify(player, tprintf("Saving %s", args[0]));
    sprintf(openfile, "%s/", MECH_PATH);
    strcat(openfile, args[0]);
    DOCHECK(!(fp =
	    fopen(openfile, "w")),
	"Unable to open/create mech file! Sorry.");
    fprintf(fp, "%d %d %d %d %d %.2f %.2f %d\n", MechTons(mech),
	MechTacRange(mech), MechLRSRange(mech), MechScanRange(mech),
	MechRealNumsinks(mech), MechMaxSpeed(mech), MechJumpSpeed(mech),
	MechSpecials(mech));
    for (i = 0; i < NUM_SECTIONS; i++) {
	fprintf(fp, "%d %d %d %d\n", GetSectArmor(mech, i),
	    GetSectInt(mech, i), GetSectRArmor(mech, i),
	    MechSections(mech)[i].config);
	for (j = 0; j < NUM_CRITICALS; j++) {
	    fprintf(fp, "%d %d %d\n",
		MechSections(mech)[i].criticals[j].type,
		MechSections(mech)[i].criticals[j].data,
		MechSections(mech)[i].criticals[j].mode);
	}
    }
    fprintf(fp, "%d %d\n", MechType(mech), MechMove(mech));
    fprintf(fp, "%d\n", MechRadioRange(mech));
    fclose(fp);
    notify(player, "Saving complete!");
}

void mechrep_Rsavetemp2(dbref player, void *data, char *buffer)
{
    char *args[1];
    char openfile[512];

    MECHREP_COMMON(1);

    free_template_list();

    DOCHECK(mech_parseattributes(buffer, args, 1) != 1,
	"You must specify a template name!");
    DOCHECK(strstr(args[0], "/"), "Invalid file name!");
    notify(player, tprintf("Saving %s", args[0]));
    sprintf(openfile, "%s/", MECH_PATH);
    strcat(openfile, args[0]);
/*  DOCHECK(mech_weight_sub(GOD, mech, -1) > (MechTons(mech) * 1024),	"Error saving template: Too heavy."); */
    DOCHECK(save_template(player, mech, args[0], openfile) < 0,
	"Error saving the template file!");
    notify(player, "Saving complete!");
}

void mechrep_Rsetarmor(dbref player, void *data, char *buffer)
{
    char *args[4];
    int argc;
    int index;
    int temp;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(!argc, "Invalid number of arguments!");
    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    if (index == -1) {
	notify(player, "Not a legal area. Must be HEAD, CTORSO");
	notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	return;
    }
    argc--;
    if (argc) {
	temp = atoi(args[1]);
	if (temp < 0)
	    notify(player, "Invalid armor value!");
	else {
	    notify(player, "Front armor set!");
	    SetSectArmor(mech, index, temp);
	    SetSectOArmor(mech, index, temp);
	}
	argc--;
    }
    if (argc) {
	temp = atoi(args[2]);
	if (temp < 0)
	    notify(player, "Invalid Internal armor value!");
	else {
	    notify(player, "Internal armor set!");
	    SetSectInt(mech, index, temp);
	    SetSectOInt(mech, index, temp);
	}
	argc--;
    }
    if (argc) {
	temp = atoi(args[3]);
	if (index == CTORSO || index == RTORSO || index == LTORSO) {
	    if (temp < 0)
		notify(player, "Invalid Rear armor value!");
	    else {
		notify(player, "Rear armor set!");
		SetSectRArmor(mech, index, temp);
		SetSectORArmor(mech, index, temp);
	    }
	} else
	    notify(player, "Only the torso can have rear armor.");
    }
}

void mechrep_Raddweap(dbref player, void *data, char *buffer)
{
    char *args[20];
    int argc;
    int index;
    int weapindex;
    int loop, temp;
    int isrear = 0;
    int istc = 0;
    int isos = 0;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 20);
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
    if (args[argc - 1][0] == 'T' || args[argc - 1][0] == 't' ||
	args[argc - 1][1] == 'T' || args[argc - 1][1] == 't')
	istc = 1;

    if (args[argc - 1][0] == 'R' || args[argc - 1][0] == 'r' ||
	args[argc - 1][1] == 'R' || args[argc - 1][1] == 'r')
	isrear = 1;

    if (args[argc - 1][0] == 'O' || args[argc - 1][0] == 'o' ||
        args[argc - 1][1] == 'O' || args[argc - 1][1] == 'o')
        isos = 1;

    if (isrear || istc || isos)
	argc--;

    if (MechWeapons[weapindex].special & OS_WEAP)
	isos = 1;

    /* Subtract off our two arguments */
    argc -= 2;
    if (argc < GetWeaponCrits(mech, weapindex))
	notify(player, "Not enough critical slots specified!");
    else {
	for (loop = 0; loop < GetWeaponCrits(mech, weapindex); loop++) {
	    temp = atoi(args[2 + loop]);
	    temp--;		/* From 1 based to 0 based */
	    DOCHECK(temp < 0 ||
		temp > NUM_CRITICALS, "Bad critical location!");
	    MechSections(mech)[index].criticals[temp].type =
		(I2Weapon(weapindex));
	    MechSections(mech)[index].criticals[temp].mode = 0;
            if (isos)
		MechSections(mech)[index].criticals[temp].mode |=
		    OS_MODE;
	    if (isrear)
		MechSections(mech)[index].criticals[temp].mode |=
		    REAR_MOUNT;
	    if (istc)
		MechSections(mech)[index].criticals[temp].mode |= ON_TC;
	}
	if (IsAMS(weapindex)) {
	    if (MechWeapons[weapindex].special & CLAT)
		MechSpecials(mech) |= CL_ANTI_MISSILE_TECH;
	    else
		MechSpecials(mech) |= IS_ANTI_MISSILE_TECH;
	}
	notify(player, "Weapon added.");
    }
}
void mechrep_Rsethp(dbref player, void *data, char *buffer)
{
    char *args[3];
    int argc;
    int index;
    int loop, temp;
    int isAmmo = 0;
    int isLaser = 0;
    int isNone = 0;
    int isMissile = 0;
    int isSpecial = 0;
    int isGauss = 0;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 3);
    DOCHECK(argc < 3, "Invalid number of arguments!")
        index = ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    if (index == -1) {
        notify(player, "Not a legal area. Must be HEAD, CTORSO");
        notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
        notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
        return;
    }
    args[2][0] = (toupper(args[2][0]));
    if (args[2][0] == 'N')
        isNone = 1; 
    else if (args[2][0] == 'E')
        isLaser = 1; 
    else if (args[2][0] == 'M')
	isMissile = 1;
    else if (args[2][0] == 'S')
	isSpecial = 1; 
    else if (args[2][0] == 'B')
        isAmmo = 1;
    else if (args[2][0] == 'G')
	isGauss = 1;
    DOCHECK(!(isLaser || isAmmo || isNone || isMissile || isSpecial || isGauss), "No hardpoint mode selected. Try E M S B G or N.");

    temp = (atoi(args[1]) - 1);
    DOCHECK(temp < 0 || temp > CritsInLoc(mech, index), "Bad critical location!"); 
    if (isAmmo)
	MechSections(mech)[index].criticals[temp].mode |= BALL_HPOINT;   
    else if (isLaser)
        MechSections(mech)[index].criticals[temp].mode |= ENERGY_HPOINT; 
    else if (isMissile)
        MechSections(mech)[index].criticals[temp].mode |= MISSILE_HPOINT;
    else if (isSpecial)
        MechSections(mech)[index].criticals[temp].mode |= SPECIAL_HPOINT;
    else if (isGauss)
	MechSections(mech)[index].criticals[temp].mode |= GAUSS_HPOINT;

    if (isNone)
        MechSections(mech)[index].criticals[temp].mode &= ~(HPOINTS);

    if (!isNone)
        MechSections(mech)[index].criticals[temp].type = I2Special(HARDPOINT);
    else
	MechSections(mech)[index].criticals[temp].type = 0;

    notify(player, tprintf("%s Hardpoint set.", isLaser ? "Laser" :
	 isAmmo ? "Ammo" : isMissile ? "Missile" : isSpecial ? "Special" :
	 isGauss ? "Gauss" : isNone ? "None" : "ERROR"));
}

void mechrep_setcargospace(dbref player, void *data, char *buffer)
{
    char *args[2];
    int argc;
    int cargo;
    int max;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 2);
    DOCHECK(argc != 2, "Invalid number of arguements!");

    cargo = (atoi(args[0]) * 50);
    DOCHECK(cargo < 0 || cargo > 100000, "Doesn't that seem excessive?");
    CargoSpace(mech) = cargo;

    max = (atoi(args[1]));
    max = (BOUNDED(10,max,100)); 
    CarMaxTon(mech) = (char) max;

    notify(player, tprintf("%3.2f cargospace and %d tons of maxton space set.", (float) ((float) cargo / 100), (int) max));

}

void mechrep_setpodspace(dbref player, void *data, char *buffer)
{
    char *args[1];
    int argc;
    int pod;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 1);
    DOCHECK(argc != 1, "Invalid number of arguements!");
    pod = (atoi(args[0]) * 50);
    DOCHECK(pod < 0 || pod > 10000, "Doesn't that seem excessive?");
    PodSpace(mech) = pod;
    notify(player, tprintf("%3.2f tons of podspace set.", (float) ((float) pod / 100)));
}

void mechrep_Rreload(dbref player, void *data, char *buffer)
{
    char *args[4];
    int argc;
    int index;
    int weapindex;
    int subsect;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc <= 2, "Invalid number of arguments!");
    weapindex = WeaponIndexFromString(args[0]);
    if (weapindex == -1) {
	notify(player, "That is not a valid weapon!");
	DumpWeapons(player);
	return;
    }
    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), args[1]);
    if (index == -1) {
	notify(player, "Not a legal area. Must be HEAD, CTORSO");
	notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	return;
    }
    subsect = atoi(args[2]);
    subsect--;			/* from 1 based to 0 based */
    DOCHECK(subsect < 0 ||
	subsect > NUM_CRITICALS, "Subsection out of range!");
    if (MechWeapons[weapindex].ammoperton == 0)
	notify(player, "That weapon doesn't require ammo!");
    else {
	MechSections(mech)[index].criticals[subsect].type =
	    I2Ammo(weapindex);
	MechSections(mech)[index].criticals[subsect].mode = 0;
/*	MechSections(mech)[index].criticals[subsect].data =	*/
/*	    FullAmmo(mech, index, subsect);			*/
/* Moved to bottom of function for HALFTON_MODE support		*/
	if (argc > 3)
	    switch (toupper(args[3][0])) {
	    case '+':
		MechSections(mech)[index].criticals[subsect].mode |=
		    HALFTON_MODE;
		break;
	    case 'W':
		MechSections(mech)[index].criticals[subsect].mode |=
		    SWARM_MODE;
		break;
	    case '1':
		MechSections(mech)[index].criticals[subsect].mode |=
		    SWARM1_MODE;
		break;
	    case 'I':
		MechSections(mech)[index].criticals[subsect].mode |=
		    INFERNO_MODE;
		MechCritStatus(mech) |= INFERNO_AMMO;
		break;
	    case 'L':
		MechSections(mech)[index].criticals[subsect].mode |=
		    LBX_MODE;
		break;
            case 'T':
                MechSections(mech)[index].criticals[subsect].mode |=
                    TRACER_MODE;
                break;
            case 'G':
                MechSections(mech)[index].criticals[subsect].mode |=
                    SGUIDED_MODE;
                break;
	    case 'Z':
		MechSections(mech)[index].criticals[subsect].mode |=
		    STINGER_MODE;
		break;
	    case 'X':
		MechSections(mech)[index].criticals[subsect].mode |=
		    CASELESS_MODE;
		break;
	    case 'R':
                MechSections(mech)[index].criticals[subsect].mode |=
                    PRECISION_MODE; 
		break;
	    case 'P':
		MechSections(mech)[index].criticals[subsect].mode |=
		    PIERCE_MODE;
		break;
	    case 'D':
                MechSections(mech)[index].criticals[subsect].mode |=
                    DEADFIRE_MODE;
                break;
            case 'H':
                MechSections(mech)[index].criticals[subsect].mode |=
                    ATMHE_MODE;
                break; 
            case 'E':
                MechSections(mech)[index].criticals[subsect].mode |=
                    ATMER_MODE;
                break; 
	    case 'A':
		MechSections(mech)[index].criticals[subsect].mode |=
		    ARTEMIS_MODE;
		break;
	    case 'N':
		MechSections(mech)[index].criticals[subsect].mode |=
		    NARC_MODE;
		break;
	    case 'C':
		MechSections(mech)[index].criticals[subsect].mode |=
		    CLUSTER_MODE;
		break;
	    case 'M':
		MechSections(mech)[index].criticals[subsect].mode |=
		    MINE_MODE;
		break;
	    case 'S':
		MechSections(mech)[index].criticals[subsect].mode |=
		    SMOKE_MODE;
		break;
	    case 'F':
		MechSections(mech)[index].criticals[subsect].mode |=
		    INCEND_MODE;
		break;
	    } 
        MechSections(mech)[index].criticals[subsect].data =
            FullAmmo(mech, index, subsect);
	notify(player, "Weapon loaded!");
    }
}

void mechrep_Rrepair(dbref player, void *data, char *buffer)
{
    char *args[4];
    int argc;
    int index;
    int temp;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc <= 2, "Invalid number of arguments!");
    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), args[0]);
    if (index == -1) {
	notify(player, "Not a legal area. Must be HEAD, CTORSO");
	notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	return;
    }
    temp = atoi(args[2]);
    DOCHECK(temp < 0, "Illegal value for armor!");
    switch (args[1][0]) {
    case 'A':
    case 'a':
	/* armor */
	SetSectArmor(mech, index, temp);
	notify(player, "Armor repaired!");
	break;
    case 'I':
    case 'i':
	/* internal */
	SetSectInt(mech, index, temp);
	notify(player, "Internal structure repaired!");
	break;
    case 'C':
    case 'c':
	/* criticals */
	temp--;
	if (temp >= 0 && temp < NUM_CRITICALS) {
	    MechSections(mech)[index].criticals[temp].data = 0;
	    notify(player, "Critical location repaired!");
	} else {
	    notify(player, "Critical Location out of range!");
	}
	break;
    case 'R':
    case 'r':
	/* rear */
	if (index == CTORSO || index == LTORSO || index == RTORSO) {
	    SetSectRArmor(mech, index, temp);
	    notify(player, "Rear armor repaired!");
	} else {
	    notify(player,
		"Only the center, rear and left torso have rear armor!");
	}
	break;
    default:
	notify(player,
	    "Illegal Type-> must be ARMOR, INTERNAL, CRIT, REAR");
	return;
    }
}

/*
   ADDSP <ITEM> <LOCATION> <SUBSECT> [<DATA>]
 */
void mechrep_Raddspecial(dbref player, void *data, char *buffer)
{
    char *args[4];
    int argc;
    int index;
    int itemcode;
    int subsect;
    int newdata;
    int max;

    MECHREP_COMMON(1);
    argc = mech_parseattributes(buffer, args, 4);
    DOCHECK(argc <= 2, "Invalid number of arguments!");
    itemcode = FindSpecialItemCodeFromString(args[0]);
    if (itemcode == -1)
	if (strcasecmp(args[0], "empty")) {
	    notify(player, "That is not a valid special object!");
	    DumpMechSpecialObjects(player);
	    return;
	}
    index =
	ArmorSectionFromString(MechType(mech), MechMove(mech), args[1]);
    if (index == -1) {
	notify(player, "Not a legal area. Must be HEAD, CTORSO");
	notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	return;
    }
    subsect = atoi(args[2]);
    subsect--;
    max = NUM_CRITICALS;
    if (index == LLEG || index == RLEG || index == HEAD)
	max = 6;
    DOCHECK(subsect < 0 || subsect >= max, "Subsection out of range!");
    if (argc == 4)
	newdata = atoi(args[3]);
    else
	newdata = 0;
    MechSections(mech)[index].criticals[subsect].type =
	itemcode < 0 ? 0 : I2Special(itemcode);
    MechSections(mech)[index].criticals[subsect].data = newdata;
    switch (itemcode) {
    case CASE:
	MechSections(mech)[(MechType(mech) ==
		CLASS_VEH_GROUND) ? BSIDE : index].config |= CASE_TECH;
	notify(player, "CASE Technology added to section.");
	break;
    case TRIPLE_STRENGTH_MYOMER:
	MechSpecials(mech) |= TRIPLE_MYOMER_TECH;
	notify(player,
	    "Triple Strength Myomer Technology added to 'Mech.");
	break;
    case NULLSIG:
	MechSpecials(mech) |= NULLSIG_TECH;
	notify(player,
	    "Null Signature Technology added to 'Mech.");
	break;
    case STEALTHARM:
	MechSpecials2(mech) |= STEALTHARMOR_TECH;
	notify(player,
	    "Stealth Armor Technology added to 'Mech.");
	break;
    case MASC:
	MechSpecials(mech) |= MASC_TECH;
	notify(player,
	    "Myomer Accelerator Signal Circuitry added to 'Mech.");
	break;
    case SUPERCHARGER:
	MechSpecials2(mech) |= SCHARGE_TECH;
	notify(player, "SuperCharger added to 'Mech.");
	break;
    case C3_MASTER:
	MechSpecials(mech) |= C3_MASTER_TECH;
	notify(player, "C3 Command Unit added to 'Mech.");
	break;
    case C3_SLAVE:
	MechSpecials(mech) |= C3_SLAVE_TECH;
	notify(player, "C3 Slave Unit added to 'Mech.");
	break;
    case C3I:
	MechSpecials(mech) |= C3_MASTER_TECH;
	MechSpecials(mech) |= C3_SLAVE_TECH;
	notify(player, "C3I Unit added to 'Mech.");
	break;
    case ARTEMIS_IV:
	MechSections(mech)[index].criticals[subsect].data--;
	MechSpecials(mech) |= ARTEMIS_IV_TECH;
	notify(player, "Artemis IV Fire-Control System added to 'Mech.");
	notify(player,
	    tprintf
	    ("System will control the weapon which starts at slot %d.",
	 newdata));
	break;
    case CAPACITOR:
        MechSections(mech)[index].criticals[subsect].data--; 
        notify(player, "PPC Capacitor added to 'Mech.");
        notify(player,
            tprintf
            ("Capacitor will control the weapon which starts at slot %d.",
         newdata));
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
    }
    notify(player, "Critical slot filled.");
}

extern char *specials[];
extern char *specials2[];

void mechrep_Rshowtech(dbref player, void *data, char *buffer)
{
    int i;
    char location[20];

    MECHREP_COMMON(1);
    notify(player, "--------Advanced Technology--------");
    if (MechSpecials(mech) & TRIPLE_MYOMER_TECH)
	notify(player, "Triple Strength Myomer");
    if (MechSpecials(mech) & NULLSIG_TECH)
	notify(player, "Null Signature Device");
    if (MechSpecials2(mech) & STEALTHARMOR_TECH)
	notify(player, "Stealth Armor");
    if (MechSpecials(mech) & MASC_TECH)
	notify(player, "Myomer Accelerator Signal Circuitry");
    if (MechSpecials2(mech) & SCHARGE_TECH)
	notify(player, "SuperCharger");
    for (i = 0; i < NUM_SECTIONS; i++)
	if (MechSections(mech)[i].config & CASE_TECH) {
	    ArmorStringFromIndex(i, location, MechType(mech),
		MechMove(mech));
	    notify(player,
		tprintf("Cellular Ammunition Storage Equipment in %s",
		    location));
	}
    if (MechSpecials(mech) & CLAN_TECH) {
	notify(player, "Mech is set to Clan Tech.  This means:");
	notify(player, "    Mech automatically has Double Heat Sink Tech");
	notify(player, "    Mech automatically has CASE in all sections");
    }
    if (MechSpecials(mech) & DOUBLE_HEAT_TECH)
	notify(player, "Mech uses Double Heat Sinks");
    if (MechSpecials2(mech) & COMPACT_HEAT_TECH)
        notify(player, "Mech uses Compact Heat Sinks");
    if (MechSpecials(mech) & CL_ANTI_MISSILE_TECH)
	notify(player, "Clan style Anti-Missile System");
    if (MechSpecials(mech) & IS_ANTI_MISSILE_TECH)
	notify(player, "Inner Sphere style Anti-Missile System");
    if (MechSpecials(mech) & FLIPABLE_ARMS)
	notify(player, "The arms may be flipped into the rear firing arc");
    if (MechSpecials(mech) & (C3_MASTER_TECH|C3_SLAVE_TECH)) {
	if ((MechSpecials(mech) & C3_MASTER_TECH) && (MechSpecials(mech) & C3_SLAVE_TECH))
	    notify(player, "C3 Improved Module");
        else if (MechSpecials(mech) & C3_MASTER_TECH)
    	    notify(player, "C3 Command Computer");
        else if (MechSpecials(mech) & C3_SLAVE_TECH)
	    notify(player, "C3 Slave Computer"); 
    }
    if (MechSpecials(mech) & ARTEMIS_IV_TECH)
	notify(player, "Artemis IV Fire-Control System");
    if (MechSpecials(mech) & ECM_TECH)
	notify(player, "Guardian ECM Suite");
    if (MechSpecials2(mech) & ANGEL_ECM_TECH)
        notify(player, "Angel ECM Suite");
    if (MechSpecials(mech) & BEAGLE_PROBE_TECH)
	notify(player, "Beagle Active Probe");
    if (MechSpecials2(mech) & BLOODHOUND_PROBE_TECH)
        notify(player, "Bloodhound Active Probe");
    if (MechSpecials(mech) & ICE_TECH)
	notify(player, "It has ICE engine");
    notify(player, "Brief version (May have something previous hadn't):");
    notify(player, MechSpecials(mech) ? BuildBitString(specials,
	    MechSpecials(mech)) : "-");
    notify(player, MechSpecials2(mech) ? BuildBitString(specials2,
	    MechSpecials2(mech)) : "-");
}

void mechrep_Rdeltech(dbref player, void *data, char *buffer)
{
    int i, j;
    int Type;

    MECHREP_COMMON(1);
    for (i = 0; i < NUM_SECTIONS; i++)
	if ((MechSections(mech)[i].config & CASE_TECH)
	    || (MechSpecials(mech) & TRIPLE_MYOMER_TECH)
	    || (MechSpecials(mech) & MASC_TECH)
	    || (MechSpecials(mech) & NULLSIG_TECH) 
	    || (MechSpecials2(mech) & SCHARGE_TECH)
	    || (MechSpecials2(mech) & STEALTHARMOR_TECH)) {
	    for (j = 0; j < NUM_CRITICALS; j++) {
		Type = MechSections(mech)[i].criticals[j].type;
		if (Type == I2Special((CASE))
		    || Type == I2Special((TRIPLE_STRENGTH_MYOMER))
		    || Type == I2Special((NULLSIG))
		    || Type == I2Special((STEALTHARM))
		    || Type == I2Special((MASC))
		    || Type == I2Special(SUPERCHARGER))
		    MechSections(mech)[i].criticals[j].type = EMPTY;
	    }
	    MechSections(mech)[i].config &= ~CASE_TECH;
	}
    MechSpecials(mech) = 0;
    MechSpecials2(mech) = 0;
    notify(player, "Advanced Technology Deleted");
}

void mechrep_Raddtech(dbref player, void *data, char *buffer)
{
    int nv, nv2;

    MECHREP_COMMON(1);
    nv = BuildBitVector(specials, buffer);
    nv2 = BuildBitVector(specials2, buffer);
    if (nv < 0 && nv2 < 0) {
	notify(player, "Invalid tech: Available techs:");
	for (nv = 0; specials[nv]; nv++)
	    notify(player, tprintf("\t%s", specials[nv]));
	for (nv2 = 0; specials2[nv2]; nv2++)
	    notify(player, tprintf("\t%s", specials2[nv2]));
	return;
    }
    if (!nv && !nv2) {
	notify(player, "Nothing set!");
	return;
    }
    if (nv > 0)
        MechSpecials(mech) |= nv;
    if (nv2 > 0)
	MechSpecials2(mech) |= nv2;
    notify(player, tprintf("Set: %s", BuildBitString((nv2 > 0 ? specials2 : specials), (nv2 > 0 ? nv2 : nv))));
}

MECH *loadfuncref(char *ref)
{
    static MECH funcmech;
    static char funcref[128];
                                                                                                                                                                                      
    if (!strcmp(funcref, ref))
        return &funcmech;
    if (mech_loadnew(GOD, &funcmech, ref) < 1) {
        funcref[0] = '\0';
        return NULL;
    }
    strncpy(funcref, ref, 1023);
    funcref[127] = '\0';
    return &funcmech;
}

