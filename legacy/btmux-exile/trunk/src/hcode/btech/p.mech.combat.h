
/*
   p.mech.combat.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Mon Mar 22 10:40:19 CET 1999 from mech.combat.c */

#ifndef _P_MECH_COMBAT_H
#define _P_MECH_COMBAT_H

/* mech.combat.c */
void mech_safety(dbref player, void *data, char *buffer);
void mech_cruise(dbref player, void *data, char *buffer);
void mech_overthrust(dbref player, void *data, char *buffer);
int IsArtyMech(MECH * mech);
void mech_slwarn(dbref player, void *data, char *buffer);
void mech_noradio(dbref player, void *data, char *buffer);
void mech_turnmode(dbref player, void *data, char *buffer);
void mech_autofall(dbref player, void *data, char *buffer);
void ClearFireAdjustments(MAP * map, dbref mech);
void mech_spot(dbref player, void *data, char *buffer);
void mech_target(dbref player, void *data, char *buffer);
void sixth_sense_check(MECH * mech, MECH * target);
void mech_settarget(dbref player, void *data, char *buffer);
void mech_fireweapon(dbref player, void *data, char *buffer);
MECH *find_mech_in_hex(MECH * mech, MAP * mech_map, int x, int y,

    int needlos);
int FireSpot(dbref player, MECH * mech, MAP * mech_map, int weaponnum,
    int weapontype, int sight, int section, int critical);
int FireWeaponNumber(dbref player, MECH * mech, MAP * mech_map,
    int weapnum, int argc, char **args, int sight);
int Dump_Decrease(MECH * mech, int loc, int pos, int *hm);
void mech_dump(dbref player, void *data, char *buffer);
int AttackMovementMods(MECH * mech);
int TargetMovementMods(MECH * mech, MECH * target, float range);
void ammo_expedinture_check(MECH * mech, int weapindx, int ns);
int FindAndCheckAmmo(MECH * mech, int weapindx, int section, int critical,
    int *ammoLoc, int *ammoCrit);
void Missile_Hit(MECH * mech, MECH * target, int isrear, int iscritical,
    int weapindx, int num_missiles_hit, int damage, int salvo_size,
    int LOS, int bth, int mode, int real, int realroll);
void mech_damage(dbref player, MECH * mech, char *buffer);
void heat_effect(MECH * mech, MECH * tempMech, int heatdam);
void Inferno_Hit(MECH * mech, MECH * hitMech, int missiles, int LOS);
int AMSMissiles(MECH * mech, MECH * hitMech, int incoming, int type,
    int ammoLoc, int ammoCrit, int LOS);
int LocateAMSDefenses(MECH * target, int *AMStype, int *ammoLoc,

    int *ammoCrit);
int MissileHitIndex(MECH * mech, MECH * hitMech, int weapindx, int plus2, int mode, int glance);
int MissileHitTarget(MECH * mech, int weapindx, MECH * hitMech, int LOS,
    int baseToHit, int roll, int plus2, int reallynarc, int inferno,
    int incoming, int mode, int realroll);
int FindNormalBTH(MECH * mech, MAP * mech_map, int section, int critical,
    int weapindx, float range, MECH * target, int indirectFire, int sight);
int FindArtilleryBTH(MECH * mech, int weapindx, int indirect, float range);
char *hex_target_id(MECH * mech);
int possibly_ignite(MECH * mech, MAP * map, int weapindx, int x, int y,
    int bth1, int bth2, int r, int firemod);
int possibly_clear(MECH * mech, int weapindx, int x, int y, int bth1,
    int bth2, int r);
void possibly_ignite_or_clear(MECH * mech, int weapindx, int x, int y,
    int bth1, int bth2);
void hex_hit(MECH * mech, int x, int y, int weapindx, int ishit, int mode);
void decrement_ammunition(MECH * mech, int weapindx, int section,
    int critical, int ammoLoc, int ammoCrit);
void SwarmHitTarget(MECH * mech, int weapindx, MECH * hitMech, int LOS,
    int baseToHit, int roll, int plus2, int reallynarc, int inferno,
    int incoming, int fof, int mode, int section, int critical, int realroll);
void FireWeapon(MECH * mech, MAP * mech_map, MECH * target, int LOS,
    int weapindx, int weapnum, int section, int critical, float enemyX,
    float enemyY, int mapx, int mapy, float range, int indirectFire,
    int sight, int ishex, int swarmhit);
void HitTarget(MECH * mech, int weapindx, MECH * hitMech, int LOS,
    int ultra_mode, int on_tc, int lbx_mode, int plus2,
    int modifier, int reallyhit, int bth, int rfmmg, int mode, int section, int critical, int realroll,
    int swarmhit);
int FindAreaHitGroup(MECH * mech, MECH * target);
int FindTargetHitLoc(MECH * mech, MECH * target, int *isrear,

    int *iscritical);
int FindTCHitLoc(MECH * mech, MECH * target, int *isrear, int *iscritical);
int FindAimHitLoc(MECH * mech, MECH * target, int *isrear,

    int *iscritical);
void KillMechContentsIfIC(dbref aRef, MECH * mech);
void DestroyMech(MECH * target, MECH * mech, int bc);
int cause_armordamage(MECH * wounded, MECH * attacker, int LOS,
    int attackPilot, int isrear, int iscritical, int hitloc, int damage,
    int *crits, int glance, int swarmhit);
int cause_internaldamage(MECH * wounded, MECH * attacker, int LOS,
    int attackPilot, int isrear, int hitloc, int intDamage, int weapindx,
    int *crits, int glance, int swarmhit);
void DamageMech(MECH * wounded, MECH * attacker, int LOS, int attackPilot,
    int hitloc, int isrear, int iscritical, int damage, int intDamage,
    int cause, int bth, int glance);
void DestroyWeapon(MECH * wounded, int hitloc, int type, char numcrits, int crit);
int CountWeaponsInLoc(MECH * mech, int loc);
int FindWeaponTypeNumInLoc(MECH * mech, int loc, int num);
void LoseWeapon(MECH * mech, int hitloc);
int FindObj(MECH * mech, int loc, int type);
void DestroyHeatSink(MECH * mech, int hitloc);
void StartAcidEffect(MECH * mech, MECH * hitmech, int hitloc, int notifatt, int notifdef, int bc);
void DestroySection(MECH * wounded, MECH * attacker, int LOS, int hitloc);
char *setarmorstatus_func(MECH * mech, char *sectstr, char *typestr,
    char *valuestr);
int dodamage_func(dbref player, MECH * mech, int totaldam, int clustersize, int direction, int iscritical, char *mechmsg, char *mechbroadcast);

#endif				/* _P_MECH_COMBAT_H */
