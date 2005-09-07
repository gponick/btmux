
/*
 * $Id: autopilot.h,v 1.4 2005/08/03 21:40:54 av1-op Exp $
 *
 * Author: Markus Stenberg <fingon@iki.fi>
 *
 *  Copyright (c) 1996 Markus Stenberg
 *  Copyright (c) 1998-2002 Thomas Wouters
 *  Copyright (c) 2000-2002 Cord Awtry
 *       All rights reserved
 *
 * Created: Wed Oct 30 20:43:42 1996 fingon
 * Last modified: Sat Jun  6 19:56:42 1998 fingon
 *
 */

#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include "mech.events.h"
#include "dllist.h"

#define AUTOPILOT_MEMORY        100     /* Number of command slots available to AI */
#define AUTOPILOT_MAX_ARGS      5       /* Max number of arguments for a given AI Command
                                           Includes the command as the first argument */
#define AUTOPILOT_NC_DELAY      1       /* Generic command wait time before executing */

/* Delay for next command */
#define AUTOPILOT_GOTO_TICK     3       /* How often to check any GOTO event */
#define AUTOPILOT_LEAVE_TICK    5       /* How often to check if we've left the bay/hangar */
#define AUTOPILOT_WAITFOE_TICK  4
#define AUTOPILOT_PURSUE_TICK   3
#define AUTOPILOT_FOLLOW_TICK   3

/* The autopilot structure */
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
    /*
    unsigned char program_counter;
    unsigned char first_free;
    unsigned short commands[AUTOPILOT_MEMORY];
    */

    /* The autopilot's command list */
    dllist *commands;

    /* AI A* pathfinding stuff */
    dllist *astar_path;

    /* Temporary AI-pathfind data variables */
    int ahead_ok;
    int auto_cmode;     /* 0 = Flee ; 1 = Close ; 2 = Maintain distances if possible */
    int auto_cdist;     /* Distance we're trying to maintain */
    int auto_goweight;
    int auto_fweight;
    int auto_nervous;

    int b_msc, w_msc, b_bsc, w_bsc, b_dan, w_dan, last_upd;
} AUTO;

#define AUTO_GOET               15
#define AUTO_GOTT               240

#define AUTOPILOT_AUTOGUN       1       /* Is autogun enabled, ie: shoot what AI wants to */
#define AUTOPILOT_GUNZOMBIE     2
#define AUTOPILOT_PILZOMBIE     4
#define AUTOPILOT_ROAMMODE      8       /* Are we roaming around */
#define AUTOPILOT_LSENS         16      /* Should change sensors or user set them */
#define AUTOPILOT_CHASETARG     32      /* Should chase the target */
#define AUTOPILOT_SWARMCHARGE   64

#define Gunning(a)              ((a)->flags & AUTOPILOT_AUTOGUN)
#define StartGun(a)             (a)->flags |= AUTOPILOT_AUTOGUN
#define StopGun(a)              (a)->flags &= ~(AUTOPILOT_AUTOGUN|AUTOPILOT_GUNZOMBIE)

#define UpdateAutoSensor(a) \
    AUTOEVENT(a, EVENT_AUTOGS, auto_gun_sensor_event, 1, 1)

#define TrulyStartGun(a) \
    do { AUTOEVENT(a, EVENT_AUTOGUN, auto_gun_event, 1, 0); \
        AUTOEVENT(a, EVENT_AUTOGS, auto_gun_sensor_event, 1, 0); } while (0)

#define DoStartGun(a)       \
    do { StartGun(a) ; TrulyStartGun(a); \
        a->flags &= ~AUTOPILOT_GUNZOMBIE; } while (0)

#define TrulyStopGun(a) \
    do { muxevent_remove_type_data(EVENT_AUTOGUN, a); \
        muxevent_remove_type_data(EVENT_AUTOGS, a); } while (0)

#define DoStopGun(a)        \
    do { StopGun(a); TrulyStopGun(a); } while (0)

#define Zombify(a) \
    do { a->flags |= AUTOPILOT_GUNZOMBIE; TrulyStopGun(a); } while (0)

#define PilZombify(a) \
    do { a->flags |= AUTOPILOT_PILZOMBIE; } while (0)

#define UnZombify(a) \
    do { if (a->flags & AUTOPILOT_GUNZOMBIE) { TrulyStartGun(a);\
        a->flags &= ~AUTOPILOT_GUNZOMBIE; } } while (0)

#define UnZombifyAuto(a) \
    do { if (Gunning(a)) UnZombify(a); if (a->flags & AUTOPILOT_PILZOMBIE) { \
        a->flags &= ~AUTOPILOT_PILZOMBIE ;\
        AUTOEVENT(a, EVENT_AUTOCOM, auto_com_event, 1, 0); } } while (0)

#define UnZombifyMech(mech) \
    do { AUTO *au; if (MechAuto(mech) > 0 && \
        (au=FindObjectsData(MechAuto(mech)))) UnZombifyAuto(au); } while (0)

#define GVAL(a,b) \
    (((a->program_counter + (b)) < a->first_free) ? \
    a->commands[(a->program_counter+(b))] : -1)

#define CCLEN(a) \
       (1 + acom[a->commands[a->program_counter]].argcount)

#define PG(a) \
       a->program_counter

/* defines for quick access to bitfield code */
#define HexOffSet(x, y) (x * MAPY + y)
#define HexOffSetNode(node) (HexOffSet(node->x, node->y))

#define WhichByte(hex_offset) ((hex_offset) >> 3)
#define WhichBit(hex_offset) ((hex_offset) & 7)

#define CheckHexBit(array, offset) (array[WhichByte(offset)] & (1 << WhichBit(offset)) ? 1 : 0)
#define SetHexBit(array, offset) (array[offset >> 3] = \
        array[offset >> 3] | (1 << (offset & 7)))
#define ClearHexBit(array, offset) (array[offset >> 3] = \
        array[offset >> 3] & ~(1 << (offset & 7)))

/* Quick flags for use with the various autopilot
 * commands.  Check the ACOM array in autopilot_commands.c */
enum {
    GOAL_DUMBFOLLOW,
    GOAL_DUMBGOTO,
    GOAL_FOLLOW,        /* unimplemented */
    GOAL_GOTO,          /* unimplemented */
    GOAL_LEAVEBASE,     /* unimplemented */
    GOAL_ROAM,          /* unimplemented */
    GOAL_WAIT,          /* unimplemented */

    COMMAND_ATTACKLEG,  /* unimplemented */
    COMMAND_AUTOGUN,    /* unimplemented */
    COMMAND_CHASEMODE,  /* unimplemented */
    COMMAND_CMODE,      /* unimplemented */
    COMMAND_DROPOFF,    /* unimplemented */
    COMMAND_EMBARK,     /* unimplemented */
    COMMAND_ENTERBASE,  /* unimplemented */
    COMMAND_ENTERBAY,   /* unimplemented */
    COMMAND_JUMP,       /* unimplemented */
    COMMAND_LOAD,       /* unimplemented */
    COMMAND_PICKUP,     /* unimplemented */
    COMMAND_REPORT,     /* unimplemented */
    COMMAND_ROAMMODE,   /* unimplemented */
    COMMAND_SHUTDOWN,
    COMMAND_SPEED,      /* unimplemented */
    COMMAND_STARTUP,
    COMMAND_STOPGUN,    /* unimplemented */
    COMMAND_SWARM,      /* unimplemented */
    COMMAND_SWARMMODE,  /* unimplemented */
    COMMAND_UDISEMBARK, /* unimplemented */
    COMMAND_UNLOAD,     /* unimplemented */
    AUTO_NUM_COMMANDS
};

/* command_node struct to store AI 
 * commands for the AI command list */
typedef struct command_node_t {
    char *args[5];                          /* Store arguements - At most will ever have 5 */
    unsigned char argcount;                 /* Number of arguments */
    int command_enum;                       /* The ENUM value for the command */
    int (*ai_command_function)(AUTO *);     /* Function Pointer */
} command_node;

/* A structure to store info about the various AI commands */
typedef struct {
    char *name;
    int argcount;
    int command_enum;
    int (*ai_command_function)(AUTO *);
} ACOM;

/* astar node Structure for the A star pathfinding */
typedef struct astar_node_t {
    short x;
    short y;
    short x_parent;
    short y_parent;
    int g_score;
    int h_score;
    int f_score;
    int hexoffset;
} astar_node;

/* Function Prototypes will go here */

/* From autopilot_core.c */
void auto_load_commands(FILE *f, AUTO *a);
void auto_save_commands(FILE *f, AUTO *a);
void auto_destroy_command_node(command_node *node);
void auto_set_comtitle(AUTO * a, MECH * mech);
void auto_init(AUTO * a, MECH * m);
char *auto_get_command_arg(AUTO *a, int command_number, int arg_number);
int auto_get_command_enum(AUTO *a, int command_number);
void auto_goto_next_command(AUTO *a);

/* From autopilot_commands.c */
void auto_cal_mapindex(MECH * mech);
void auto_command_pickup(AUTO *a);

/* From autopilot_ai.c */
int auto_astar_generate_path(AUTO * a, MECH * mech, short end_x, short end_y);

#include "p.autopilot.h"
#include "p.ai.h"
#include "p.autopilot_command.h"
#include "p.autopilot_commands.h"
#include "p.autogun.h"

#endif                  /* AUTOPILOT_H */
