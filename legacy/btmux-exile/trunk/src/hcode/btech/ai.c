/* Point of the excercise : move from point a,b to point c,d while
   eliminating opponents and stuff, avoiding enemies in rear/side arc
   and generally having fun */

#include <math.h>

#include "mech.h"
#include "autopilot.h"
#include "p.autopilot_command.h"
#include "p.mech.utils.h"
#include "p.map.obj.h"
#include "spath.h"
#include "p.mech.combat.h"

#define SCORE_MOD 100
#define SAFE_SCORE SCORE_MOD * 1000
#define MIN_SAFE 8
#define NORM_SAFE 32
#define MAX_SIM_PATHS 40

typedef struct {
    int e, t;
    float s, ds;
    float fx, fy;
    short x, y, lx, ly;
    int h;
    int dh;
} LOC;

enum {
    SP_OPT_NORM, SP_OPT_FASTER, SP_OPT_SLOWER, SP_OPT_C
};
int sp_opt[SP_OPT_C] = { 0, 1, -1 };	/* We prefer to keep at same speed, however */

#define MNORM_COUNT 37

int move_norm_opt[MNORM_COUNT][2] = {
    {0, SP_OPT_FASTER},
    {0, SP_OPT_NORM},
    {0, SP_OPT_SLOWER},
    {-1, SP_OPT_NORM},
    {1, SP_OPT_NORM},
    {-2, SP_OPT_NORM},
    {2, SP_OPT_NORM},
    {-3, SP_OPT_NORM},
    {3, SP_OPT_NORM},
    {-5, SP_OPT_NORM},
    {5, SP_OPT_NORM},
    {-10, SP_OPT_NORM},
    {10, SP_OPT_NORM},
    {-15, SP_OPT_NORM},
    {15, SP_OPT_NORM},
    {-20, SP_OPT_NORM},
    {20, SP_OPT_NORM},
    {-30, SP_OPT_NORM},
    {30, SP_OPT_NORM},
    {-40, SP_OPT_NORM},
    {40, SP_OPT_NORM},
    {-60, SP_OPT_NORM},
    {60, SP_OPT_NORM},
    {-60, SP_OPT_SLOWER},
    {60, SP_OPT_SLOWER},
    {-60, SP_OPT_FASTER},
    {60, SP_OPT_FASTER},
    {-80, SP_OPT_NORM},
    {80, SP_OPT_NORM},
    {-120, SP_OPT_NORM},
    {120, SP_OPT_NORM},
    {-160, SP_OPT_NORM},
    {160, SP_OPT_NORM},
    {-160, SP_OPT_SLOWER},
    {160, SP_OPT_SLOWER},
    {-160, SP_OPT_FASTER},
    {160, SP_OPT_FASTER}
};

#define CFAST_COUNT 9

/* Update: Just do subset if we're in silly mood */
int combat_fast_opt[CFAST_COUNT][2] = {
    {0, SP_OPT_FASTER},
    {0, SP_OPT_NORM},
    {0, SP_OPT_SLOWER},
    {-10, SP_OPT_NORM},
    {10, SP_OPT_NORM},
    {-30, SP_OPT_NORM},
    {30, SP_OPT_NORM},
    {-60, SP_OPT_NORM},
    {60, SP_OPT_NORM}
};

void sendAIM(AUTO * a, MECH * m, char *msg)
{
    auto_replyA(m, msg);
    SendAI(msg);
}

char *AI_Info(MECH * m, AUTO * a)
{
    static char buf[MBUF_SIZE];

    sprintf(buf, "Unit#%d on #%d [A#%d]:", m->mynum, m->mapindex,
	a->mynum);
    return buf;
}

extern float terrain_speed(MECH * mech, float tempspeed, float maxspeed,
    int terrain, int elev);

static int ai_crash(MAP * map, MECH * m, LOC * l)
{
    float newx = 0.0, newy = 0.0;
    float tempspeed, maxspeed, acc;
    int offset;
    int normangle;
    int mw_mod = 1;
    int ch = 0;

    if (!map)
	return 0;

    maxspeed = MMaxSpeed(m);

    /* UpdateHeading */
    if (l->h != l->dh && !(MechCritStatus(m) & GYRO_DESTROYED)) {
	ch = 1;
	if (is_aero(m))
	    maxspeed = maxspeed * ACCEL_MOD;
	normangle = l->h - l->dh;
	if (MechType(m) == CLASS_MW || MechType(m) == CLASS_BSUIT)
	    mw_mod = 5;
	/* XXX Jumping */
	if (fabs(l->s) < 1.0)
	    offset = 3 * maxspeed * MP_PER_KPH * mw_mod;
	else {
	    offset = 2 * maxspeed * MP_PER_KPH * mw_mod;
	    if ((abs(normangle) > offset) && (l->s > (maxspeed * 2 / 3)))
		offset -= offset / 2 * (3.0 * l->s / maxspeed - 2.0);
	}
	offset = offset * MOVE_MOD;
	if (normangle < 0)
	    normangle += 360;
	if (IsDS(m) && offset >= 10)
	    offset = 10;
	if (normangle > 180) {
	    l->h += offset;
	    if (l->h >= 360)
		l->h -= 360;
	    normangle += offset;
	    if (normangle >= 360)
		l->h = l->dh;
	} else {
	    l->h -= offset;
	    if (l->h < 0)
		l->h += 360;
	    normangle -= offset;
	    if (normangle < 0)
		l->h = l->dh;
	}

    }
    /* UpdateSpeed */
    /* XXX MASC */
    /* XXX heat (_hard_ to track) */
    tempspeed = l->ds;
    if (MechType(m) != CLASS_MW && MechMove(m) != MOVE_VTOL &&
	(MechMove(m) != MOVE_FLY || Landed(m)))
	tempspeed = terrain_speed(m, tempspeed, maxspeed, l->t, l->e);
    if (ch) {
	if (mudconf.btech_slowdown == 2) {
	    int dif = MechFacing(m) - MechDesiredFacing(m);
	    if (dif < 0)
		dif = -dif;
	    if (dif > 180)
		dif = 360 - dif;
	    if (dif) {
		dif = (dif - 1) / 30 + 2;
		tempspeed = tempspeed * (10 - dif) / 10;
	    }
	} else if (mudconf.btech_slowdown == 1) {
	    if (l->h != l->dh)
		tempspeed = tempspeed * 2.0 / 3.0;
	    else
		tempspeed = tempspeed * 3.0 / 4.0;
	}
    }
    if (MechMove(m) == MOVE_QUAD && MechLateral(m))
	tempspeed -= MP1;
    if (tempspeed <= 0.0) {
	tempspeed = 0.0;
    }
    if (l->ds < 0.)
	tempspeed = -tempspeed;
    if (tempspeed != l->s) {
	if (MechMove(m) == MOVE_QUAD)
	    acc = maxspeed / 10.;
	else
	    acc = maxspeed / 20.;
	if (tempspeed < l->s) {
	    /* Decelerating */
	    l->s -= acc;
	    if (tempspeed > l->s)
		l->s = tempspeed;
	} else {
	    /* Accelerating */
	    l->s += acc;
	    if (tempspeed < l->s)
		l->s = tempspeed;
	}
    }
    /* move_mech + NewHexEntered */
    /* XXX Jumping mechs [aeros,VTOLs] */
    /* XXX Non-mechs */
    /* XXX Quads */
    FindComponents(l->s * MOVE_MOD, l->h, &newx, &newy);
    l->fx += newx;
    l->fy += newy;
    l->lx = l->x;
    l->ly = l->y;
    RealCoordToMapCoord(&l->x, &l->y, l->fx, l->fy);
    /* Ensure we don't run off map */
    if (BOUNDED(1, l->x, map->map_width - 1) != l->x)
	return 1;
    if (BOUNDED(1, l->y, map->map_height - 1) != l->y)
	return 1;
    if (l->lx != l->x || l->ly != l->y) {
	int oz = l->e;

	if (mechs_in_hex(map, l->x, l->y, -1, 0) >= 2)
	    return 1;

	/* Ensure if mech m can do transition auto_lx,auto_ly => auto_x,auto_y */

	/* XXX Decent handling of terrain choosing and stuff */
	switch (GetRTerrain(map, l->x, l->y)) {
	case HEAVY_FOREST:
	    if (MechType(m) != CLASS_MECH && MechMove(m) != MOVE_TRACK)
		return 1;
	    break;
	case LIGHT_FOREST:
	    if (MechType(m) != CLASS_MECH && MechMove(m) != MOVE_TRACK)
		return 1;
	    break;
	case WATER:
	case HIGHWATER:
	    if (MechMove(m) == MOVE_TRACK || MechMove(m) == MOVE_WHEEL)
		return 1;
	    else {
		/* Check floodability for mechs */
		if (MechType(m) == CLASS_MECH) {
		    int t = Elevation(map, l->x, l->y);

		    if (t < 0) {
			if (t == -1) {
			    /* Check feet */
#define Floodable(loc) (!GetSectArmor(m,loc) || (!GetSectRArmor(m,loc) && GetSectORArmor(m,loc)))
#define FloodCheck(loc) if (GetSectInt(m,loc)) if (Floodable(loc)) return 1;
			    FloodCheck(LLEG);
			    FloodCheck(RLEG);
			    if (MechMove(m) == MOVE_QUAD) {
				FloodCheck(LARM);
				FloodCheck(RARM);
			    }
			} else {
			    int i;

			    for (i = 0; i < NUM_SECTIONS; i++)
				FloodCheck(i);
			}
		    }
		}
	    }
	    break;
/*	case BRIDGE: */
	case ICE:
	case SNOW:
	    return 1;
	    break;
	}
	l->e = Elevation(map, l->x, l->y);
	if (MechMove(m) == MOVE_HOVER)
	    l->e = MAX(0, l->e);
	l->t = GetRTerrain(map, l->x, l->y);
	if (MechType(m) == CLASS_MECH)
	    if ((l->e - oz) > 2 || (oz - l->e) > 2)
		return 1;
	/* XXX Check flooding etc, should avoid _that_ */
	if (MechType(m) == CLASS_VEH_GROUND)
	    if ((l->e - oz) > 1 || (oz - l->e) > 1)
		return 1;

    }
    return 0;
}


static void LOCInit(LOC * l, MECH * m)
{
    l->fx = MechFX(m);
    l->fy = MechFY(m);
    l->e = MechElevation(m);
    l->h = MechFacing(m);
    l->dh = MechDesiredFacing(m);
    l->s = MechSpeed(m);
    l->t = MechRTerrain(m);
    l->ds = MechDesiredSpeed(m);
    l->x = MechX(m);
    l->y = MechY(m);
    l->lx = MechX(m);
    l->ly = MechY(m);
}

static LOC enemy_l[MAX_MECHS_PER_MAP];
static MECH *enemy_m[MAX_MECHS_PER_MAP];	/* Hopefully no more mechs on map */
static int enemy_o[MAX_MECHS_PER_MAP];	/* 'out' = not moving anymore (crashed?) */
static int enemy_i[MAX_MECHS_PER_MAP];	/* dbref */
static int enemy_c = 0;		/* Number of enemies */

static LOC friend_l[MAX_MECHS_PER_MAP];
static MECH *friend_m[MAX_MECHS_PER_MAP];	/* Hopefully no more mechs on map */
static int friend_o[MAX_MECHS_PER_MAP];	/* 'out' = not moving anymore (crashed?) */
static int friend_i[MAX_MECHS_PER_MAP];	/* dbref */
static int friend_c = 0;	/* Number of enemies */

int getEnemies(MECH * mech, MAP * map, int reset)
{
    MECH *tempMech;
    int i;

    if (reset) {
	for (i = 0; i < enemy_c; i++) {
	    LOCInit(&enemy_l[i], enemy_m[i]);	/* Just reset location */
	    enemy_o[i] = 0;
	}
	return 0;
    }
    enemy_c = 0;
    for (i = 0; i < map->first_free; i++) {
	tempMech = FindObjectsData(map->mechsOnMap[i]);
	if (!tempMech)
	    continue;
	if (Destroyed(tempMech))
	    continue;
	if (MechStatus(tempMech) & COMBAT_SAFE)
	    continue;
	if (MechTeam(tempMech) == MechTeam(mech))
	    continue;
	if (FlMechRange(map, mech, tempMech) > 50.0)
	    continue;		/* Inconsequential */
	/* Something that is _possibly_ unnerving */
	LOCInit(&enemy_l[enemy_c], tempMech);	/* Location */
	enemy_m[enemy_c] = tempMech;	/* Mech data */
	enemy_i[enemy_c++] = i;
    }
    return enemy_c;
}

int getFriends(MECH * mech, MAP * map, int reset)
{
    MECH *tempMech;
    int i;

    if (reset) {
	for (i = 0; i < friend_c; i++) {
	    LOCInit(&friend_l[i], friend_m[i]);	/* Just reset location */
	    friend_o[i] = 0;
	}
	return 0;
    }
    friend_c = 0;
    for (i = 0; i < map->first_free; i++) {
	tempMech = FindObjectsData(map->mechsOnMap[i]);
	if (!tempMech)
	    continue;
	if (Destroyed(tempMech))
	    continue;
	if (MechTeam(tempMech) != MechTeam(mech))
	    continue;
	if (MechType(tempMech) != CLASS_MECH)
	    continue;
	if (FlMechRange(map, mech, tempMech) > 50.0)
	    continue;		/* Inconsequential */
	LOCInit(&friend_l[friend_c], tempMech);	/* Location */
	friend_m[friend_c] = tempMech;	/* Mech data */
	friend_i[friend_c++] = i;
    }
    return friend_c;
}

/* This has been made .. slightly more complicated.
   Basic idea (now): Calculate all the [n] states _simultaneously_ ;
   our movement isn't affecting enemy/friend movement after all, just
   our own, therefore this is _almost_ [n] times faster than the old code
   (approx. 1/30 P60 per 'sheep outside combat, perhaps 1/10 P60 inside) */

#define MAGIC_NUM -123456

#define U1(a,b) if (a < b || a == MAGIC_NUM) a = b
#define U2(a,b) if (a > b || a == MAGIC_NUM) a = b
#define FU(a,b,c,s) ((b) == (c) ? 0 : (s) * ((a) - (b)) / ((c) - (b)))

void ai_path_score(MECH * m, MAP * map, AUTO * a, int opts[][2], int num_o,
    int gotenemy, float dx, float dy, float delx, float dely, int *rl,
    int *bd, int *bscore)
{
    int i, j, k, l, bearing;
    int sd, sc;
    LOC lo[MAX_SIM_PATHS];
    int dan[MAX_SIM_PATHS], tdan[MAX_SIM_PATHS], msc[MAX_SIM_PATHS], bsc[MAX_SIM_PATHS];
    int out[MAX_SIM_PATHS], stack[MAX_SIM_PATHS], br[MAX_SIM_PATHS];


    for (i = 0; i < num_o; i++) {
	msc[i] = 0;
	bsc[i] = 0;
	dan[i] = 0;
	tdan[i] = 0;
	out[i] = 0;
	stack[i] = 0;
	br[i] = 9999;
	LOCInit(&lo[i], m);
	lo[i].dh = AcceptableDegree(lo[i].dh + opts[i][0]);
	sd = sp_opt[opts[i][1]];
	if (sd) {
	    if (sd < 0) {
		lo[i].ds = lo[i].ds * 2.0 / 3.0;
	    } else if (sd == 1) {
		float ms = MMaxSpeed(m);

		lo[i].ds = (lo[i].ds < MP1 ? MP1 : lo[i].ds) * 4.0 / 3.0;
		if (lo[i].ds > ms)
		    lo[i].ds = ms;
	    } else {
		float ms = MMaxSpeed(m);

		lo[i].ds = ms;
	    }
	}
    }
    for (i = 0; i < NORM_SAFE; i++) {
	dx += delx;
	dy += dely;
	for (k = 0; k < num_o; k++) {
	    if (out[k])
		continue;
	    if (ai_crash(map, m, &lo[k])) {	/* Simulate _one_ step */
		out[k] = i + 1;
		continue;
	    }
	    /* Base target-acquisition stuff */
	    if ((l = FindXYRange(lo[k].fx, lo[k].fy, dx, dy)) < br[k])
		br[k] = l;

	    /* Generally speaking we're going to the point spesified */
	    msc[k] += 4 * (2 * (50 - br[k]) + (100 - l));

	    /* Heading change's inherently [slightly] evil */
	    if (lo[k].h != lo[k].dh)
		msc[k] -= 1;

	    /* Moving is a good thing */
	    if (lo[k].x != lo[k].lx || lo[k].y != lo[k].ly) {
/*		if (lo[k].t == WATER)
		    msc[k] -= 5; */
		switch (lo[k].t) {
		    case WATER:
		    case HIGHWATER:
			msc[k] -= 10;
			break;
		    case LIGHT_FOREST:
		    case DESERT:
			msc[k] -= 4;
			break;
		    case HEAVY_FOREST:
			msc[k] -= 6;
			break;
		    case MOUNTAINS:
			msc[k] -= 8;
			break;
		    }
		msc[k] += 10;
	    }
	    /* Punish for not utilizing full speed (this is .. hm, flaky) */
	    if (opts[k][1] != SP_OPT_FASTER && MMaxSpeed(m) > 0.1) {
		sc = BOUNDED(0, 100 * lo[k].ds / MMaxSpeed(m), 100); 
		msc[k] -= (100 - sc) / 30;
	    }
	}
	if (MechType(m) == CLASS_MECH) {
	    /* Simulate friends */
	    for (j = 0; j < friend_c; j++) {
		if (friend_o[j])
		    continue;
		if (ai_crash(map, friend_m[j], &friend_l[j]))
		    friend_o[j] = 1;
	    }
	    for (k = 0; k < num_o; k++) {
		int sc = 0;

		if (out[k] || stack[k])
		    continue;
		/* Meaning of stack: Someone moves _into_ the hex */
		for (j = 0; j < friend_c; j++)
		    if (!friend_o[j])
			if (lo[k].x == friend_l[j].x &&
			    lo[k].y == friend_l[j].y)
			    sc++;
		if (sc > 1) {	/* Possible stackage */
		    int osc = sc;

		    for (j = 0; j < friend_c; j++)
			if (!friend_o[j])
			    if (lo[k].x == friend_l[j].x &&
				lo[k].y == friend_l[j].y)
				if ((lo[k].lx != lo[k].x ||
					lo[k].ly != lo[k].y) ||
				    (friend_l[j].lx != friend_l[j].x ||
					friend_l[j].ly != friend_l[j].y))
				    osc--;
		    if (osc != sc)
			stack[k] = i + 1;
		}
		if (gotenemy)
		    tdan[k] = 0;
	    }
	}
	if (gotenemy) {
	    /* Update enemy locations as well */
	    for (j = 0; j < enemy_c; j++) {
		if (enemy_o[j])
		    continue;
		if (ai_crash(map, enemy_m[j], &enemy_l[j]))
		    enemy_o[j] = 1;
		for (k = 0; k < num_o; k++) {
		    if (out[k])
			continue;
		    if ((l =
			    MyHexDist(lo[k].x, lo[k].y, enemy_l[j].x,
			       enemy_l[j].y, 0)) >= 100)
			continue;
		    switch (a->auto_cmode) {
		    case 0:	/* Withdraw */
			if (l > a->auto_cdist)
			    bsc[k] +=
				5 * a->auto_cdist + l - a->auto_cdist;
			else
			    bsc[k] += 5 * l;
			break;
		    case 1:	/* Score  = fulfilling goal (=> distance from cdist) */
			if (l < a->auto_cdist)
			    bsc[k] -= 10 * (a->auto_cdist - l);	/* Not too close */
			else
			    bsc[k] -= 2 * (l - a->auto_cdist);
			break;
		    case 2:
			if (l < a->auto_cdist)
			    bsc[k] -= 2 * (a->auto_cdist - l);
			else
			    bsc[k] -= 10 * (l - a->auto_cdist);
		    }
		    if (l > 28)
			continue;
		    /* Danger modifier ; it's _always_ dangerous to be close */
		    tdan[k] += (40 - MIN(40, l));
		    /* Arcs can be .. dangerous */
		    if (MechType(m) == CLASS_MECH) {
			bearing =
			    FindBearing(lo[k].fx, lo[k].fy, enemy_l[j].fx,
			    enemy_l[j].fy);
			bearing = lo[k].h - bearing;
			if (bearing < 0)
			    bearing += 360;
			if (bearing >= 90 && bearing <= 270) {
			    /* Sides are moderately dangerous [potential rear arcs] */
			    tdan[k] += 5 * (29 - MIN(29, l));
			    if (bearing >= 120 && bearing <= 240) {
				/* Rear arc is VERY dangerous */
				tdan[k] += 20 * (29 - MIN(29, l));
			    }
			}
		    } else if (MechType(m) == CLASS_VEH_GROUND) {
			bearing =
			    FindBearing(lo[k].fx, lo[k].fy, enemy_l[j].fx,
			    enemy_l[j].fy);
			bearing = lo[k].h - bearing;
			if (bearing < 0)
			    bearing += 360;
			if (bearing >= 45 && bearing <= 315) {
			    if (bearing >= 135 && bearing <= 225) {
				/* Rear arc is VERY dangerous */
				tdan[k] +=
				    10 * (29 - MIN(29,
				 l)) * (100 - 100 * GetSectArmor(m,
					BSIDE) / MAX(1, GetSectOArmor(m,
					    BSIDE))) / 100;
			    } else if (bearing < 135) {
				/* right side */
				tdan[k] +=
				    7 * (29 - MIN(29,
				 l)) * (100 - 100 * GetSectArmor(m,
					RSIDE) / MAX(1, GetSectOArmor(m,
					    RSIDE))) / 100;
			    } else {
				tdan[k] +=
				    7 * (29 - MIN(29,
				 l)) * (100 - 100 * GetSectArmor(m,
					LSIDE) / MAX(1, GetSectOArmor(m,
					    LSIDE))) / 100;
			    }
			} else
			    tdan[k] +=
				5 * (29 - MIN(29,
			     l)) * (100 - 100 * GetSectArmor(m,
	       FSIDE) / MAX(1, GetSectOArmor(m, FSIDE))) / 100;
		    }
		}
		for (k = 0; k < num_o; k++) {
		    if (out[k])
			continue;
		    /* Dangerous to be far from buddy in fight */
		    l = FindXYRange(lo[k].fx, lo[k].fy, dx, dy);
		    if (gotenemy && (delx != 0.0 || dely != 0.0))
			tdan[k] += MIN(100, l * l);
		    if (enemy_c)
			tdan[k] = tdan[k] / enemy_c;
		    /* It's inherently dangerous to move slowly: */
		    if (lo[k].s <= MP2)
			tdan[k] += 400;
		    else if (lo[k].s <= MP4)
			tdan[k] += 300;
		    else if (lo[k].s <= MP6)
			tdan[k] += 200;
		    else if (lo[k].s <= MP9)
			tdan[k] += 100;
		    dan[k] += tdan[k] * (NORM_SAFE - i) / (NORM_SAFE / 2);
		}
	    }
	}
    }
    for (i = 0; i < num_o; i++) {
	U2(a->w_msc, msc[i]);
	U1(a->b_msc, msc[i]);
	if (gotenemy) {
	    U2(a->w_bsc, bsc[i]);
	    U1(a->b_bsc, bsc[i]);
	    U2(a->w_dan, dan[i]);
	    U1(a->b_dan, dan[i]);
	}
    }
    /* Now we have been.. calibrated */
    *bscore = 0;
    *bd = -1;
#if 0
    k = 1;
#endif
    /* Find best overall score */
    for (i = 0; i < num_o; i++) {
	if (!out[i])
	    out[i] = NORM_SAFE + 1;
	if (!stack[i])
	    stack[i] = out[i];
	sc =
	    (out[i] - (out[i] - stack[i]) / 2) * SAFE_SCORE + FU(msc[i],
	    a->w_msc, a->b_msc, SCORE_MOD * a->auto_goweight);
	if (gotenemy)
	    sc +=
		FU(bsc[i], a->w_bsc, a->b_bsc,
		SCORE_MOD * a->auto_fweight) - FU(dan[i], a->w_dan,
		a->b_dan,
		SCORE_MOD * (a->auto_fweight + a->auto_goweight));
	if (sc > *bscore || (*bd > 0 && sc == *bscore &&
		sp_opt[opts[i][1]] >= sp_opt[opts[*bd][1]])) {
	    *bscore = sc;
	    *bd = i;
	    *rl = out[i] - 1;
	}
#if 0
	if (*bd > 0 && sc != *bscore)
	    k = 0;
    SendDebug(tprintf("Path %d = out - %d stack - %d msc - %d a.w_msc - %d a.b_msc - %d gotenemy - %d bsc - %d a.w_bsc - %d a.b_bsc - %d dan - %d a.w_dan - %d a.b_dan - %d sc - %d bscore - %d bd - %d rl - %d", i, out[i], stack[i], msc[i], a->w_msc, a->b_msc, gotenemy, bsc[i], a->w_bsc, a->b_bsc, dan[i], a->w_dan, a->b_dan, sc, *bscore, *bd, *rl));
#endif
    }
#if 0
SendDebug(tprintf("Chosen Path = %d", *bd));
if (k)
    *bd = 13;
#endif
}

#if 0
int ai_max_speed(MECH * m, AUTO * a)
{
    float ms;

    if (MechDesiredSpeed(m) != (ms = MMaxSpeed(m)))
	return 0;
    return 1;
}
#endif

int ai_opponents(AUTO * a, MECH * m)
{
    if (a->auto_nervous) {
	a->auto_nervous--;
	return 1;
    }
    if (MechNumSeen(m))
	a->auto_nervous = 120;	/* We'll stay frisky for awhile even if cons are lost for one reason or another */
    return MechNumSeen(m);
}

#if 0
void ai_run_speed(MECH * mech, AUTO * a)
{
    /* Warp speed, cap'n! */
    char buf[128];

    if (!ai_max_speed(mech, a)) {
	strcpy(buf, "run");
	mech_speed(a->mynum, mech, buf);
    }
}

void ai_stop(MECH * mech, AUTO * a)
{
    char buf[128];

    if (MechDesiredSpeed(mech) > 0.1) {
	strcpy(buf, "stop");
	mech_speed(a->mynum, mech, buf);
    }
}

void ai_set_speed(MECH * mech, AUTO * a, int s)
{
    float ms;
    char buf[SBUF_SIZE];

    s = BOUNDED(0, s, 100);

    /* Send in percentage-based speed */
    ms = MMaxSpeed(mech);
    ms = ms * s / 100.0;
    if (MechDesiredSpeed(mech) != ms) {
	sprintf(buf, "%f", ms);
	mech_speed(a->mynum, mech, buf);
    }
}

#endif

void ai_set_speed(MECH * mech, AUTO *a, float spd)
{
char buf[SBUF_SIZE];
float newspeed;

if (!mech || !a)
    return;

newspeed = FBOUNDED(0, spd, ((MMaxSpeed(mech) * a->speed) / 100.0));

if (MechDesiredSpeed(mech) != newspeed) {
    sprintf(buf, "%f", newspeed);
    mech_speed(a->mynum, mech, buf);
    }
}

void ai_set_heading(MECH * mech, AUTO * a, int dir)
{
    char buf[128];

    if (dir == MechDesiredFacing(mech))
	return;
    sprintf(buf, "%d", dir);
    mech_heading(a->mynum, mech, buf);
}

#define UNREF(a,b,mod) if (a != MAGIC_NUM && b != MAGIC_NUM) { int c = a, d = b; a = (c * (mod - 1) + d) / mod; b = (c + d * (mod - 1)) / mod; }

void ai_adjust_move(AUTO * a, MECH * m, char *text, int hmod, int smod, int b_score)
{
    float ms;

    ai_set_heading(m, a, AcceptableDegree(MechDesiredFacing(m) + hmod));
    switch (smod) {
    default:
	SendAI("%s state: %s (hmod:%d) sc:%d", AI_Info(m, a), text, hmod, b_score);
	break;
    case SP_OPT_FASTER:
#ifdef AUTOP_DEBUG
	SendAI("%s state: %s+accelerating (hmod:%d) sc:%d", AI_Info(m, a), text, hmod, b_score);
#endif
	ms = MMaxSpeed(m);
	ai_set_speed(m, a, (float) ((MechDesiredSpeed(m) < MP1 ? MP1 : MechDesiredSpeed(m)) * 4.0 / 3.0)); 
	break;
    case SP_OPT_SLOWER:
#ifdef AUTOP_DEBUG
	SendAI("%s state: %s+decelerating (hmod:%d) sc:%d", AI_Info(m, a), text, hmod, b_score);
#endif
	ms = MMaxSpeed(m);
	ai_set_speed(m, a, (float) (MechDesiredSpeed(m) * 2.0 / 3.0));
	break;
    }
}

int ai_check_path(MECH * m, AUTO * a, float dx, float dy, float delx,
    float dely)
{
    int o;
    int b_len, bl, b, b_score;
    MAP *map;

    if (!m)
	return 0;

    map = getMap(m->mapindex);

    if (!map || !a)
	return 0;

    o = ai_opponents(a, m);
    if (a->last_upd > event_tick || (event_tick - a->last_upd) > AUTO_GOET) {
	if ((event_tick - a->last_upd) > AUTO_GOTT) {
	    a->b_msc = MAGIC_NUM;
	    a->w_msc = MAGIC_NUM;
	    a->b_bsc = MAGIC_NUM;
	    a->w_bsc = MAGIC_NUM;
	    a->b_dan = MAGIC_NUM;
	    a->b_dan = (40 + 20 * 29 + 100) * 30;	/* To stay focused */
	} else {
	    /* Slight update ; Un-refine the goals somewhat */
	    UNREF(a->w_msc, a->b_msc, 3);
	    UNREF(a->w_bsc, a->b_bsc, 5);
	    UNREF(a->w_dan, a->b_dan, 8);
	    a->b_dan = MAX(a->b_dan, (40 + 20 * 29 + 100) * 30);	/* To stay focused */
	}
	a->last_upd = event_tick;
    }
    /* Got either opponents (nasty) or [possibly] blocked path (slightly nasty), i.e. 12sec */
    if (MechType(m) == CLASS_MECH)
	getFriends(m, map, 0);
    if (o) {
	getEnemies(m, map, 0);
	if (!((event_tick / AUTOPILOT_GOTO_TICK) % 4)) {	/* Just every fourth tick, i.e. 12sec */
	    /* Thorough check */
	    ai_path_score(m, map, a, combat_fast_opt, CFAST_COUNT, 1, dx, dy,
		delx, dely, &bl, &b, &b_score);
	    b_len = b_score / SAFE_SCORE;
	    if (b_len >= MIN_SAFE)
		ai_adjust_move(a, m, "combat(/twitchy)",
		    combat_fast_opt[b][0], combat_fast_opt[b][1], b_score);
	} else {
		/* Changed these to use norm_opt */
	    ai_path_score(m, map, a, combat_fast_opt, CFAST_COUNT, 1, dx,
		dy, delx, dely, &bl, &b, &b_score);
	    b_len = b_score / SAFE_SCORE;
	    if (b_len >= MIN_SAFE)
		ai_adjust_move(a, m, "[f]combat(/twitchy)",
		    combat_fast_opt[b][0], combat_fast_opt[b][1], b_score);
	}
	return 1;		/* We want to keep fighting near foes */
    }
    if (!((event_tick / AUTOPILOT_GOTO_TICK) % 4)) {	/* Just every fourth tick, i.e. 12sec */
	/* Thorough check */
	ai_path_score(m, map, a, move_norm_opt, MNORM_COUNT, 0, dx, dy,
	    delx, dely, &bl, &b, &b_score);
	b_len = b_score / SAFE_SCORE;
	if (b_len >= MIN_SAFE)
	    ai_adjust_move(a, m, "moving", move_norm_opt[b][0],
		move_norm_opt[b][1], b_score);
    } else {
		/* Changed these to use norm_opt */
	ai_path_score(m, map, a, move_norm_opt, MNORM_COUNT, 0, dx, dy,
	    delx, dely, &bl, &b, &b_score);
	b_len = b_score / SAFE_SCORE;
	if (b_len >= MIN_SAFE)
	    ai_adjust_move(a, m, "[f]moving", move_norm_opt[b][0],
		move_norm_opt[b][1], b_score);
    }

    if (b_len >= MIN_SAFE)
	return 1;

    /* Slow down + stop - no sense in dying needlessly */
    ai_set_speed(m, a, 0);
#ifdef AUTOP_DEBUG
    SendAI("%s state: panic", AI_Info(m, a));
#endif
    sendAIM(a, m, "PANIC! Unable to comply with order.");
    return 0;
}

void ai_init(AUTO * a, MECH * m)
{
    /* XXX Analyze our unit type ; set basic combat tactic */
    a->auto_cmode = 1;		/* CHARGE! */
    a->auto_cdist = 2;		/* Attempt to avoid kicking distance */
    a->auto_nervous = 0;
    a->auto_goweight = 44;	/* We're mainly concentrating on fighting */
    a->auto_fweight = 55;
    a->speed = 100;
    a->flags = 0;
    a->targ = -1;
}

static MECH *target_mech;

int artillery_round_flight_time(float fx, float fy, float tx, float ty);

static int mech_snipe_func(MECH * mech, dbref player, int index, int high)
{
    /* Simulate mech movements until flight_time <= now */
    int now = 0, crashed = 0;
    int flt_time;
    LOC t;
    MAP *map = getMap(mech->mapindex);

    LOCInit(&t, target_mech);
    while ((flt_time =
	    artillery_round_flight_time(MechFX(mech), MechFY(mech), t.fx,
		t.fy)) > now) {
	if (!crashed)
	    if (ai_crash(map, target_mech, &t))
		crashed = 1;
	now++;
    }
    /* Fire at t.x, t.y */
    if (MechTargX(mech) != t.x || MechTargY(mech) != t.y)
	mech_settarget(player, mech, tprintf("%d %d", t.x, t.y));
    mech_fireweapon(player, mech, tprintf("%d", index));
    return 0;
}

void mech_snipe(dbref player, MECH * mech, char *buffer)
{
    char *args[3];
    dbref d;

    DOCHECK(!WizRoy(player), "Permission denied.");
    DOCHECK(mech_parseattributes(buffer, args, 3) != 2,
	"Please supply target ID _and_ weapon(s) to use");
    DOCHECK((d =
	    FindTargetDBREFFromMapNumber(mech, args[0])) <= 0,
	"Invalid target!");
    target_mech = getMech(d);
    multi_weap_sel(mech, player, args[1], 1, mech_snipe_func);

}
