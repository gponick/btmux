#ifndef TURRET_H
#define TURRET_H

#include "mech.h"

typedef struct {
    dbref mynum;

    int arcs;			/* arc_override */
    dbref parent;		/* ship whose stats we use for this */
    dbref gunner;		/* who's da gunner? */
    dbref target;		/* what do we have locked? */
    short targx, targy, targz;	/* in map coords, target squares */
    int lockmode;		/* lock modes (hex, etc) */
} TURRET_T;

#endif				/* TURRET_H */
