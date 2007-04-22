
/*
   p.mech.advanced.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Wed Feb 17 23:36:31 CET 1999 from mech.advanced.c */

#ifndef _P_MECH_ADVANCED_H
#define _P_MECH_ADVANCED_H

/* mech.advanced.c */
void mech_nullsig(dbref player, MECH * mech, char *buffer);
void mech_stealtharm(dbref player, MECH * mech, char *buffer);
void mech_ecm(dbref player, MECH * mech, char *buffer);
void mech_eccm(dbref player, MECH * mech, char *buffer);
void mech_slite(dbref player, MECH * mech, char *buffer);
void mech_ams(dbref player, void *data, char *buffer);
void mech_fliparms(dbref player, void *data, char *buffer);
void mech_ultra(dbref player, void *data, char *buffer);
void mech_rottwo(dbref player, void *data, char *buffer);
void mech_rotfour(dbref player, void *data, char *buffer);
void mech_rotsix(dbref player, void *data, char *buffer);
void mech_heat(dbref player, void *data, char *buffer);
void mech_explosive(dbref player, void *data, char *buffer);
void mech_lbx(dbref player, void *data, char *buffer);
void mech_caseless(dbref player, void *data, char *buffer);
void mech_pierce(dbref player, void *data, char *buffer);
void mech_precision(dbref player, void *data, char *buffer);
void mech_tracer(dbref player, void *data, char *buffer);
void mech_sguided(dbref player, void *data, char *buffer);
void mech_stinger(dbref player, void *data, char *buffer);
void mech_deadfire(dbref player, void *data, char *buffer);
void mech_atmer(dbref player, void *data, char *buffer);
void mech_atmhe(dbref player, void *data, char *buffer); 
void mech_artemis(dbref player, void *data, char *buffer);
void mech_narc(dbref player, void *data, char *buffer);
void mech_swarm(dbref player, void *data, char *buffer);
void mech_swarm1(dbref player, void *data, char *buffer);
void mech_inferno(dbref player, void *data, char *buffer);
void mech_hotload(dbref player, void *data, char *buffer);
void mech_cluster(dbref player, void *data, char *buffer);
void mech_smoke(dbref player, void *data, char *buffer);
void mech_mine(dbref player, void *data, char *buffer);
void mech_incendarty(dbref player, void *data, char *buffer);
void mech_masc(dbref player, void *data, char *buffer);
void mech_scharge(dbref player, void *data, char *buffer);
void mech_explode(dbref player, void *data, char *buffer);
void mech_dig(dbref player, void *data, char *buffer);
void mech_disableweap(dbref player, void *data, char *buffer);
void mech_capacitate(dbref player, void *data, char *buffer);
void mech_unjamweap(dbref player, void *data, char *buffer);

int FindMainWeapon(MECH * mech, int (*callback) (MECH *, int, int, int,
	int)); 

#endif				/* _P_MECH_ADVANCED_H */
