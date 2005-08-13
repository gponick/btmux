#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "mech.events.h"

#define AUTOPILOT_MEMORY       100
#define AUTOPILOT_NC_DELAY     1

/* Delay for next command */
#define AUTOPILOT_GOTO_TICK    3
#define AUTOPILOT_LEAVE_TICK   5
#define AUTOPILOT_WAITFOE_TICK 4
#define AUTOPILOT_PURSUE_TICK  3
#define AUTOPILOT_FOLLOW_TICK  3

/* We needn't be _so_ careful about when we leave */

typedef struct {
    dbref mynum;
    MECH *mymech;
    int mapindex;
    dbref mymechnum;
    unsigned short speed;
    int ofsx, ofsy;
    int targ;
    /* Where are we going on in the memory? */
    unsigned short flags;
    unsigned char program_counter;
    unsigned char first_free;
    unsigned short commands[AUTOPILOT_MEMORY];

    /* Temporary AI-pathfind data variables */
    int ahead_ok;
    int auto_cmode;		/* 0 = Flee ; 1 = Close ; 2 = Maintain distances if possible */
    int auto_cdist;		/* Distance we're trying to maintain */
    int auto_goweight;
    int auto_fweight;
    int auto_nervous;

    int b_msc, w_msc, b_bsc, w_bsc, b_dan, w_dan, last_upd;
} AUTO;

#define AUTO_GOET           15
#define AUTO_GOTT           240

#define AUTOPILOT_AUTOGUN   	1
#define AUTOPILOT_GUNZOMBIE 	2
#define AUTOPILOT_PILZOMBIE 	4
#define AUTOPILOT_ROAMMODE     	8
#define AUTOPILOT_LSENS     	16
#define AUTOPILOT_CHASETARG 	32
#define AUTOPILOT_SWARMCHARGE	64

#define Gunning(a)          ((a)->flags & AUTOPILOT_AUTOGUN)
#define StartGun(a)         (a)->flags |= AUTOPILOT_AUTOGUN
#define StopGun(a)          (a)->flags &= ~(AUTOPILOT_AUTOGUN|AUTOPILOT_GUNZOMBIE)

#define UpdateAutoSensor(a) \
AUTOEVENT(a, EVENT_AUTOGS, auto_gun_sensor_event, 1, 1)

#define TrulyStartGun(a) \
do { AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, 1, 0); \
  AUTOEVENT(a, EVENT_AUTOGS, auto_gun_sensor_event, 1, 0); } while (0)
#define DoStartGun(a)       \
do { StartGun(a) ; TrulyStartGun(a); a->flags &= ~AUTOPILOT_GUNZOMBIE; } while (0)
#define TrulyStopGun(a) \
do { event_remove_type_data(EVENT_AUTOGUN, a); \
event_remove_type_data(EVENT_AUTOGS, a); } while (0)
#define DoStopGun(a)        \
do { StopGun(a); TrulyStopGun(a); } while (0)

#define Zombify(a) \
do { a->flags |= AUTOPILOT_GUNZOMBIE; TrulyStopGun(a); } while (0)

#define PilZombify(a) \
do { a->flags |= AUTOPILOT_PILZOMBIE; } while (0)

#define UnZombify(a) \
do { if (a->flags & AUTOPILOT_GUNZOMBIE) { TrulyStartGun(a); a->flags &= ~AUTOPILOT_GUNZOMBIE; } } while (0)

#define UnZombifyAuto(a) \
do { if (Gunning(a)) UnZombify(a); if (a->flags & AUTOPILOT_PILZOMBIE) { a->flags &= ~AUTOPILOT_PILZOMBIE ; AUTOEVENT(a, EVENT_AUTOCOM, auto_com_event, 1, 0); } } while (0)

#define UnZombifyMech(mech) \
do { AUTO *au; if (MechAuto(mech) > 0 && (au=FindObjectsData(MechAuto(mech)))) \
  UnZombifyAuto(au); } while (0)

#define GVAL(a,b) \
(((a->program_counter + (b)) < a->first_free) ? a->commands[(a->program_counter+(b))] : -1)

#define CCLEN(a) \
       (1 + acom[a->commands[a->program_counter]].argcount)
#define PG(a) \
       a->program_counter

enum {
    GOAL_DUMBGOTO,
    GOAL_GOTO,
    GOAL_DUMBFOLLOW,
    GOAL_FOLLOW,
    GOAL_LEAVEBASE,
    GOAL_WAIT,

    COMMAND_SPEED,
    COMMAND_JUMP,
    COMMAND_STARTUP,
    COMMAND_SHUTDOWN,
    COMMAND_AUTOGUN,
    COMMAND_STOPGUN,
    COMMAND_ENTERBASE,
    COMMAND_LOAD,
    COMMAND_UNLOAD,
    COMMAND_REPORT,
    COMMAND_PICKUP,
    COMMAND_DROPOFF,
    COMMAND_EMBARK,
    COMMAND_UDISEMBARK,
    COMMAND_CMODE,
    COMMAND_SWARM,
    COMMAND_ATTACKLEG,
    COMMAND_CHASEMODE,
    COMMAND_SWARMMODE,
    COMMAND_ROAMMODE,
    GOAL_ROAM,
    NUM_COMMANDS
};

typedef struct {
    char *name;
    int argcount;
} ACOM;

#include "p.autopilot.h"
#include "p.ai.h"
#include "p.autopilot_command.h"
#include "p.autopilot_commands.h"
#include "p.autogun.h"

#endif				/* AUTOPILOT_H */
