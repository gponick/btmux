
/*
   p.bsuit.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Mon Mar 22 08:51:11 CET 1999 from bsuit.c */

#ifndef _P_BSUIT_H
#define _P_BSUIT_H

#define BSUIT_RECYCLE_TIME	20

/* bsuit.c */
char *bsuit_name(MECH * mech);
void bsuit_start_recycling(MECH * mech);
void bsuit_stopswarmer(MECH * mech, MECH * t, int intentional);
int bsuit_countswarmers(MECH * mech);
void bsuit_stopswarmers(MAP * map, MECH * mech, int intentional);
void bsuit_mirrorswarmers(MAP * map, MECH * mech);
int bsuit_attack_common_checks(MECH * mech, dbref player);
int bsuit_members(MECH * mech);
int bsuit_findtarget(dbref player, MECH * mech, MECH ** target,

    char *buffer);
void bsuit_swarm(dbref player, void *data, char *buffer);
void bsuit_attackleg(dbref player, void *data, char *buffer);
void bsuit_hide(dbref player, void *data, char *buffer);
void mech_thrash(dbref player, void *data, char *buffer);

#endif				/* _P_BSUIT_H */
