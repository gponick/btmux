#define GMOD 1			/* Acceleration / second, in Z hexes */

#include "mech.h"
#include "aero.bomb.h"
#include "coolmenu.h"
#include "mycool.h"
#include "mech.events.h"
#include "math.h"
#include "create.h"
#include "p.mech.utils.h"
#include "p.artillery.h"
#include "p.mech.combat.h"
#include "p.econ_cmds.h"

BOMBINFO bombs[] = {
    {"5_AntiInfantry", 5, 5, 256, 15}
    ,
    {"5_Cluster", 5, 2, 256, 10}
    ,
    {"5_Standard", 5, 0, 256, 10}
    ,
    {"10_Inferno", 10, 1, 512, 5}
    ,
    {"10_Cluster", 10, 2, 5124, 5}
    ,
    {"10_Standard", 10, 0, 512, 5}
    ,
    {"20_Concussion", 20, 3, 1024, 2}
    ,
    {"20_Neutron", 20, 4, 2056, 1} 
    ,
    {"20_Standard", 20, 0, 1024, 2}
    ,
    {NULL, 0, 0, 0}
};

void DestroyBomb(MECH * mech, int loc)
{

}

int BombWeight(int i)
{
    return bombs[i].weight;
}

char *bomb_name(int i)
{
    return bombs[i].name;
}

int BombStrength(int i)
{
    return bombs[i].aff;
}

int BombCap(int i)
{
    return bombs[i].cap;
}

void bomb_list(MECH * mech, int player)
{
    int bc = 0, fb;
    int i, j, k;
    char location[20];
    static char *types[] = {
	"Standard",
	"Inferno",
	"Cluster",
	"Concussion",
	"Neutron",
	"AntiInfantry",
	NULL
    };
    coolmenu *c = NULL;

    addline();
    cent(tprintf("Bomb payload for %s:", GetMechID(mech)));
    addline();
    for (i = 0; i < NUM_SECTIONS; i++) {
	fb = 1;
	for (j = 0; j < NUM_CRITICALS; j++)
	    if (IsBomb((k = GetPartType(mech, i, j)))) {
		k = Bomb2I(k);
		if (fb) {
		    ArmorStringFromIndex(i, location, MechType(mech),
			MechMove(mech));
		    fb = 0;
		}
		if (!bc) {
		    vsi(tprintf("#  %-20s %-7s %-5s %s", "Location", "Amount",
			    "Power", "Type"));
		}
		vsi(tprintf("%-2d %-20s %-7d %5d %s", bc + 1, location,
			GetPartData(mech, i, j), bombs[k].aff,
			types[(bombs[k].type)]));
		bc++;
	    }
    }
    if (!bc)
	cent("No bombs installed.");
    addline();
    ShowCoolMenu(player, c);
    KillCoolMenu(c);
}

float calc_dest(MECH * mech, short *x, short *y)
{
    /* Present location */
    float fx = MechFX(mech);
    float fy = MechFY(mech);
    float fz = MechFZ(mech) / ZSCALE;
    float zspd = MechStartFZ(mech) / ZSCALE;
    float t, ot;

    ot = t = (zspd + sqrt(zspd * zspd + 2 * GMOD * fz)) / GMOD;
    t = (float) t / MOVE_TICK;
    fx = fx + MechStartFX(mech) * t;
    fy = fy + MechStartFY(mech) * t;
    RealCoordToMapCoord(x, y, fx, fy);
    return ot;
}

void bomb_aim(MECH * mech, dbref player)
{
    float t;			/* The time of impact */
    char toi[LBUF_SIZE];
    short x, y;

    t = calc_dest(mech, &x, &y);
    sprintf(toi, "%.1f second%s", t, (t >= 2.0 || t < 1.0) ? "" : "s");
    mech_notify(mech, MECHALL,
	tprintf
	("Estimated bomb flight time %s, estimated landing hex %d,%d.",
     toi, x, y));
}

void bomb_hit_hexes(MAP * map, int x, int y, int hitnb, int iscluster,
    int aff_d, int aff_h, char *tomsg, char *otmsg, char *tomsg1,
    char *otmsg1)
{
    blast_hit_hexes(map, aff_d, iscluster ? 2 : 10, aff_h, x, y, tomsg,
	otmsg, tomsg1, otmsg1, 0, 4, 1, 1, hitnb, NULL, NULL);
}

static void bomb_hit(bomb_shot * s)
{
    int loop, isrear = 0, iscritical = 1, tx, ty, lx, ly;
    float range;
    MECH * tempmech;
    switch (bombs[s->type].type) {
    case 0:
	HexLOSBroadcast(s->map, s->x, s->y,
	    "A blast rocks the area around $H!");
	bomb_hit_hexes(s->map, s->x, s->y, 1, 0,
	    bombs[s->type].type ==
	    1 ? bombs[s->type].aff / 2 : bombs[s->type].aff,
	    bombs[s->type].type == 1 ? bombs[s->type].aff : 0,
	    "You receive a direct hit!", "receives a direct hit!",
	    "You are hit by shrapnel!", "is hit by shrapnel!");
	break;
    case 1:
	HexLOSBroadcast(s->map, s->x, s->y,
	    "A fiery blast occurs in $H, spraying flaming gel everywhere!");
	bomb_hit_hexes(s->map, s->x, s->y, 1, 0,
	    bombs[s->type].type ==
	    1 ? bombs[s->type].aff / 2 : bombs[s->type].aff,
	    bombs[s->type].type == 1 ? bombs[s->type].aff : 0,
	    "You receive a direct hit!", "receives a direct hit!",
	    "You are hit by the globs of flaming gel!",
	    "is hit by the globs!");
	break;
    case 2:
	HexLOSBroadcast(s->map, s->x, s->y,
	    "A bomb drops rain of small bomblets in $H's surroundings!");
	bomb_hit_hexes(s->map, s->x, s->y, 1, 1,
	    bombs[s->type].type ==
	    1 ? bombs[s->type].aff / 2 : bombs[s->type].aff,
	    bombs[s->type].type == 1 ? bombs[s->type].aff : 0,
	    "You are hit by ton of small munitions!",
	    "is hit by many small munitions!",
	    "You are hit by some of the small munitions!",
	    "is hit by some small munitions!");
	break;
    case 3:
	HexLOSBroadcast(s->map, s->x, s->y,
	    "A concussion bomb thumps into the ground at $H!");
	for (loop = 0; loop < s->map->first_free; loop++) {
	    tempmech = getMech(s->map->mechsOnMap[loop]);
	    if (!tempmech)
		continue;
	    if (MechStatus(tempmech) & COMBAT_SAFE)
		continue;
	    if (MechType(tempmech) != CLASS_MECH)
		continue;
	    if (Jumping(tempmech) || OODing(tempmech))
		continue;
	    if (FindHexRange(MechX(tempmech), MechY(tempmech), s->x, s->y) > 4)
		continue;
	    mech_notify(tempmech, MECHALL, "You are shocked by the concussion blast!");
            if (!MadePilotSkillRoll(tempmech, 2)) {
		mech_notify(tempmech, MECHALL, "You fall over from the concussion!!");
		MechLOSBroadcast(tempmech, "falls down, staggered by the concussion!");
		MechFalls(tempmech, 2, 0);
		if (MiscEventing(tempmech))
		    StopMiscEvent(tempmech);
	        } 
	    }
	break;
    case 4:
       HexLOSBroadcast(s->map, s->x, s->y,
            "A small neutron bomb smashes into the ground at $H!");
        for (loop = 0; loop < s->map->first_free; loop++) {
            tempmech = getMech(s->map->mechsOnMap[loop]);
            if (!tempmech)
                continue;
            if (MechStatus(tempmech) & COMBAT_SAFE)
                continue;
            if ((range = FindHexRange(MechX(tempmech), MechY(tempmech), s->x, s->y)) > 30)
                continue;
	    if (MechZ(tempmech) > ATMO_Z)
		continue;
            mech_notify(tempmech, MECHALL, "You are blasted by an extreme neutron wave!");
    	    MechLOSBroadcast(tempmech, "is blasted by the neutron wave!");
	    isrear = InWeaponArc(tempmech, s->x, s->y) & REARARC ? 1 : 0; 
	    Missile_Hit(s->aero, tempmech, isrear, iscritical, 0, 1, 60, 5, 1, 0, 0, 1, (range > 15 ? 0 : 1));
            }
	lx = MIN(s->map->map_width - 1, s->x + 40);
	ly = MIN(s->map->map_height - 1, s->y + 40);
	for (tx = MAX(1, s->x - 40) ; tx < lx; tx++)
 	    for (ty = MAX(1, s->y - 40); ty < ly; ty++)
		if (FindHexRange(tx, ty, s->x, s->y) < 30) {
		    if (GetTerrain(s->map, tx, ty) == BRIDGE && Number(1,6) <= 3)
			SetTerrain(s->map, tx, ty, DBRIDGE);
		    else if (GetTerrain(s->map, tx, ty) != BUILDING && GetTerrain(s->map, tx, ty) != WATER && Number(1,6) <= 1)
		        SetTerrain(s->map, tx, ty, ROUGH);
		    else if (Number(1,6) <= 2)
		    	SetElevation(s->map, tx, ty, Number(0,3));
		}
	break;
    case 5:
        HexLOSBroadcast(s->map, s->x, s->y,
            "A concussion bomb thumps into the ground at $H!");
        for (loop = 0; loop < s->map->first_free; loop++) {
            tempmech = getMech(s->map->mechsOnMap[loop]);
            if (!tempmech)
                continue;
            if (MechStatus(tempmech) & COMBAT_SAFE)
                continue;
            if (MechType(tempmech) != CLASS_BSUIT)
                continue;
            if (Jumping(tempmech) || OODing(tempmech))
                continue;
            if (FindHexRange(MechX(tempmech), MechY(tempmech), s->x, s->y) > 4)
                continue;
            mech_notify(tempmech, MECHALL, "You are slammed by the concussion and flak!");
    	    MechLOSBroadcast(tempmech, "is rocked by anti-infantry munitions!");
            isrear = InWeaponArc(tempmech, s->x, s->y) & REARARC ? 1 : 0;
            Missile_Hit(s->aero, tempmech, isrear, iscritical, 0, 1, bombs[s->type].aff, 1, 1, 0, 0, 1, 1); 
	    }
        break; 
    }
}

static void bomb_hit_event(EVENT * e)
{
    bomb_shot *s = (bomb_shot *) e->data;

    bomb_hit(s);
    free((void *) s);
}

void simulate_flight(MECH * mech, MAP * map, short *x, short *y, float t)
{
    float fx = MechFX(mech);
    float fy = MechFY(mech);
    float fz = MechFZ(mech);

/*   float fs = MechStartFZ(mech); */
    float delx, dely;
    float dx, dy;
    int i;
    short tx = *x, ty = *y;

    if (t < 1.0)
	return;

    MapCoordToRealCoord(*x, *y, &dx, &dy);
    delx = (dx - fx) / t;
    dely = (dy - fy) / t;
    for (i = 1; i < t; i++) {
	fx = fx + delx;
	fy = fy + dely;
	fz = (float) fz - GMOD;
	{
	float tfx = fx, tfy = fy;
	FindXY(tfx, tfy, map->winddir, (float) map->windspeed / 30, &fx, &fy); 
	}
	RealCoordToMapCoord(&tx, &ty, fx, fy);
	if (tx < 0 || ty < 0 || tx >= map->map_width || ty >= map->map_height)
	    continue;
	if (Elevation(map, tx, ty) > (fz / ZSCALE)) {
	    *x = tx;
	    *y = ty;
	}
    }
*x = tx; *y = ty;
}

void bomb_drop(MECH * mech, int player, int bn)
{
    int bc = 0;
    int i, j, k;
    int lloc = 0, lpos = 0;
    float t;
    short x, y;
    int mof;
    bomb_shot *s;
    MAP *map;

    DOCHECK(bn < 0, "Negative bomb number? Gimme a break.");
    bn--;
    for (i = 0; i < NUM_SECTIONS; i++)
	for (j = 0; j < NUM_CRITICALS; j++)
	    if (IsBomb((k = GetPartType(mech, i, j))) &&
		!PartIsDestroyed(mech, i, j)) {
		if (bc == bn) {
		    lloc = i;
		    lpos = j;
		}
		bc++;
	    }
    DOCHECK(Landed(mech), "No dropping while landed.");
    DOCHECK(MechZ(mech) > ATMO_Z, "No dropping bombs from the upper atmosphere.");
    DOCHECK(!bc, "No bombs installed.");
    DOCHECK(!(map = getMap(mech->mapindex)), "You're on invalid map!");
    DOCHECK(bn < 0 ||
	bn >= bc, "No bomb with such number installed! (See BOMB LIST)");
    MechLOSBroadcast(mech,
	"detaches a small object that starts falling down..");
    k = Bomb2I(GetPartType(mech, lloc, lpos));
    mech_notify(mech, MECHALL, "The ship trembles as you detach a bomb..");
    t = calc_dest(mech, &x, &y);
    if ((mof = MoFMadePilotSkillRoll(mech, (MechVelocity(mech) / MP2) + t / 2)) >= 0)
	mech_notify(mech, MECHALL,
	    "Despite the slight problems, you keep the craft stable enough to drop the bomb right on target..");
    else {
	float tx, ty, nx, ny;
	mech_notify(mech, MECHALL,
	    "The ship's lurches slightly, dropping the bomb off target!");
	MapCoordToRealCoord(x, y, &tx, &ty);
	FindXY(tx, ty,  Number(0,359), abs(mof) * Number(2,5), &nx, &ny);
	RealCoordToMapCoord(&x, &y, nx, ny); 
    }
    simulate_flight(mech, map, &x, &y, t);
    if (x < 0 || y < 0 || x >= map->map_width || y >= map->map_height)
	return;
    if (GetPartData(mech, lloc, lpos) <= 1) {
	SetPartData(mech, lloc, lpos, 0);
	SetPartType(mech, lloc, lpos, 0);
    } else {
	GetPartData(mech, lloc, lpos)--;
    }

    Create(s, bomb_shot, 1);
    s->x = x;
    s->y = y;
    s->type = k;
    s->map = map;
    s->aero = mech;
    SetCargoWeight(mech);
    event_add(MAX(1, t), 0, EVENT_DHIT, bomb_hit_event, (void *) s, NULL);
}

void mech_bomb(dbref player, void *data, char *buffer)
{
    MECH *mech = (MECH *) data;
    char *args[3];
    int argc;
    int bn;

    cch(MECH_USUALSO);
    DOCHECK(!(argc =
	    mech_parseattributes(buffer, args, 3)),
	"(At least) one option required.");
    DOCHECK(argc > 2, "Too many arguments!");
    if (!strcasecmp(args[0], "list")) {
	bomb_list(mech, player);
	return;
    }
    DOCHECK(Landed(mech), "The craft is landed!");
    if (!strcasecmp(args[0], "aim")) {
	bomb_aim(mech, player);
	return;
    }
    DOCHECK(strcasecmp(args[0], "drop"), "Invalid argument to BOMB!");
    DOCHECK(argc < 2,
	"The BOMB commands needs to know WHICH bomb to drop!");
    DOCHECK(Readnum(bn, args[1]), "Invalid bomb number!");
    bomb_drop(mech, player, bn);
}
