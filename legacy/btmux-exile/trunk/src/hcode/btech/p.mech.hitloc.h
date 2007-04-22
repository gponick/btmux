
/*
   p.mech.hitloc.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Tue Feb  9 14:31:33 CET 1999 from mech.hitloc.c */

#ifndef _P_MECH_HITLOC_H
#define _P_MECH_HITLOC_H

/* mech.hitloc.c */
int ModifyHeadHit(int hitGroup, MECH * mech);
int FindPunchLocation(int hitGroup, MECH * mech);
int FindKickLocation(int hitGroup, MECH * mech);
int get_bsuit_hitloc(MECH * mech);
int TransferTarget(MECH * mech, int hitloc);
int FindSwarmHitLocation(int *iscritical, int *isrear);
int crittable(MECH * m, MECH * att, int loc, int tres);
void MotiveCritRoll(MECH * mech, MECH * attacker);
int FindHitLocation(MECH * mech, int hitGroup, int *iscritical, 
    int *isrear, MECH * att);
int FindFasaHitLocation(MECH * mech, int hitGroup, int *iscritical, 
    int *isrear, MECH * att);

#endif				/* _P_MECH_HITLOC_H */
