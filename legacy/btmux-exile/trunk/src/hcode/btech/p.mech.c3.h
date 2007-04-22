
/*
   p.mech.c3.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Fri Jan 15 15:32:45 CET 1999 from mech.c3.c */

#ifndef _P_MECH_C3_H
#define _P_MECH_C3_H

/* mech.c3.c */
void mech_c3_targets(dbref player, void *data, char *buffer);
void c3_send(MECH * mech, char *msg);
void mech_c3_message(dbref player, void *data, char *buffer);
void mech_c3_set_master(dbref player, void *data, char *buffer);
float FindC3Range(MECH * mech, MECH * target, float range);

#endif				/* _P_MECH_C3_H */
