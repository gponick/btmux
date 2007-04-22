#ifndef ARTILLERY_H
#define ARTILLERY_H

typedef struct artillery_shot_type {
    int from_x, from_y;		/* hex this is shot from */
    int to_x, to_y;		/* hex this lands in */
    int type;			/* weapon index in MechWeapons */
    int mode;			/* weapon mode */
    int ishit;			/* did we hit target hex? */
    dbref shooter;		/* nice to know type of information */
    dbref map;			/* map we're on */
    struct artillery_shot_type *next;
    /* next in stack of unused things */
} artillery_shot;

/* Weapon values for artillery guns */
#define IS_LTOM       30
#define IS_THUMPER    31
#define IS_SNIPER     32
#define IS_ARROW      27

#define CL_ARROW      71

#endif				/* ARTILLERY_H */
