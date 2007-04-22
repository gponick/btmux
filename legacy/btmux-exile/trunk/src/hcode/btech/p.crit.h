
/*
   p.crit.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Wed Feb 17 23:36:30 CET 1999 from crit.c */

#ifndef _P_CRIT_H
#define _P_CRIT_H

/* crit.c */
void correct_speed(MECH * mech);
void explode_unit(MECH * wounded, MECH * attacker);
void HandleVTOLCrit(MECH * wounded, MECH * attacker, int LOS, int hitloc,

    int num);
void DestroyMainWeapon(MECH * mech);
void JamMainWeapon(MECH * mech);
void HandleFasaVehicleCrit(MECH * wounded, MECH * attacker, int LOS,
    int hitloc, int num);
void HandleMaxTechVehicleCrit(MECH * wounded, MECH * attacker, int LOS,
    int hitloc, int num);
void HandleVehicleCrit(MECH * wounded, MECH * attacker, int LOS,
    int hitloc, int num);
int HandleMechCrit(MECH * wounded, MECH * attacker, int LOS, int hitloc,
    int critHit, int critType, int critData, int realcrit, int notif);
void HandleCritical(MECH * wounded, MECH * attacker, int LOS, int hitloc, 
    int num, int realcrit, int tac);

#endif				/* _P_CRIT_H */
