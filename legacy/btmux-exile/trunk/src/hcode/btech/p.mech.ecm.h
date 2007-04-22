
/*
   p.mech.ecm.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Fri Jan 15 15:32:48 CET 1999 from mech.ecm.c */

#ifndef _P_MECH_ECM_H
#define _P_MECH_ECM_H

/* mech.ecm.c */
void cause_ecm(MECH * from, MECH * to);
void end_ecm_check(MECH * mech); 
int ecm_count(MECH * mech, int mode);
int eccm_count(MECH * mech, int mode); 
int ecm_affect_hex(MECH * mech,int x, int y, int z, int *IsAngel);

#endif				/* _P_MECH_ECM_H */
