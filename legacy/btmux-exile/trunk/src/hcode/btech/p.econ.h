
/*
   p.econ.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Fri Jan 15 15:32:39 CET 1999 from econ.c */

#ifndef _P_ECON_H
#define _P_ECON_H

/* econ.c */
void econ_change_items(dbref d, int id, int num);
int econ_find_items(dbref d, int id);
void econ_set_items(dbref d, int id, int num);

#endif				/* _P_ECON_H */
