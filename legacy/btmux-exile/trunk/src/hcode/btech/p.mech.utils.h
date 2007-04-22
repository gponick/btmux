
/*
   p.mech.utils.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Fri Jan 15 15:33:03 CET 1999 from mech.utils.c */

#ifndef _P_MECH_UTILS_H
#define _P_MECH_UTILS_H

/* mech.utils.c */
const char *mechtypename(MECH * foo);
int MNumber(MECH * mech, int low, int high);
char *MechIDS(MECH * mech, int islower);
char *MyToUpper(char *string);
int CritsInLoc(MECH * mech, int index);
int SectHasBusyWeap(MECH * mech, int sect);
MAP *ValidMap(dbref player, dbref map);
dbref FindTargetDBREFFromMapNumber(MECH * mech, char *mapnum);
void FindComponents(float magnitude, int degrees, float *x, float *y);
void CheckEdgeOfMap(MECH * mech);
int FindZBearing(float x0, float y0, float z0, float x1, float y1, float z1);
int FindBearing(float x0, float y0, float x1, float y1);
int InWeaponArc(MECH * mech, float x, float y);
int FindBaseToHitByRange(int weapindx, float frange, int mode, int nomin, char targ);
int FindBaseToHitByC3Range(int weapindx, float frange, int mode, char targ);
char *FindGunnerySkillName(MECH * mech, int weapindx);
char *FindPilotingSkillName(MECH * mech);
int FindPilotPiloting(MECH * mech);
int FindSPilotPiloting(MECH * mech);
int FindPilotSpotting(MECH * mech);
int FindPilotArtyGun(MECH * mech);
int FindAverageGunnery(MECH * mech);
int FindPilotGunnery(MECH * mech, int weapindx);
char *FindTechSkillName(MECH * mech);
int FindTechSkill(dbref player, MECH * mech);
int MadePilotSkillRoll(MECH * mech, int mods);
int MoFMadePilotSkillRoll(MECH * mech, int mods);
void FindXY(float x0, float y0, int bearing, float range, float *x1, float *y1);
float FindRange(float x0, float y0, float z0, float x1, float y1, float z1);
float FindXYRange(float x0, float y0, float x1, float y1);
float FindHexRange(float x0, float y0, float x1, float y1);
void RealCoordToMapCoord(short *hex_x, short *hex_y, float cart_x, float cart_y);
void MapCoordToRealCoord(int hex_x, int hex_y, float *cart_x, float *cart_y);
void navigate_sketch_mechs(MECH * mech, MAP * map, int x, int y, char buff[NAVIGATE_LINES][MBUF_SIZE]);
int FindTargetXY(MECH * mech, float *x, float *y, float *z);
int FindWeapons_Advanced(MECH * mech, int index, unsigned char *weaparray, unsigned char *weapdataarray, int *critical, int whine);
int FindAmmunition(MECH * mech, unsigned char *weaparray, unsigned short *ammoarray, unsigned short *ammomaxarray, unsigned int *modearray);
int FindLegHeatSinks(MECH * mech);
int FindLegJumpJets(MECH * mech);
int FindWeaponNumberOnMech_Advanced(MECH * mech, int number, int *section, int *crit, int sight);
int FindWeaponNumberOnMech(MECH * mech, int number, int *section, int *crit);
int FindWeaponFromIndex(MECH * mech, int weapindx, int *section, int *crit);
int FindWeaponIndex(MECH * mech, int number);
int FindAmmoForWeapon_sub(MECH * mech, int weapindx, int start, int *section, int *critical, int nogof, int gof);
int FindAmmoForWeapon(MECH * mech, int weapindx, int start, int *section, int *critical);
int FindArtemisForWeapon(MECH * mech, int section, int critical);
int FindCapacitorForWeapon(MECH * mech, int section, int critical);
int FindDestructiveAmmo(MECH * mech, int *section, int *critical);
int FindRoundsForWeapon(MECH * mech, int weapindx);
const char **ProperSectionStringFromType(int type, int mtype);
void ArmorStringFromIndex(int index, char *buffer, char type, char mtype);
int NumSections(int type);
int IsInWeaponArc(MECH * mech, float x, float y, float z, int section, int critical);
int GetWeaponCrits(MECH * mech, int weapindx);
int listmatch(char **foo, char *mat);
void do_sub_magic(MECH * mech, int loud);
void do_magic(MECH * mech);
void mech_RepairPart(MECH * mech, int loc, int pos);
int no_locations_destroyed(MECH * mech);
void mech_ReAttach(MECH * mech, int loc);
void mech_ReSeal(MECH * mech, int loc);
void mech_Detach(MECH * mech, int loc);
void mech_FillPartAmmo(MECH * mech, int loc, int pos);
int AcceptableDegree(int d);
void MarkForLOSUpdate(MECH * mech);
void multi_weap_sel(MECH * mech, dbref player, char *buffer, int bitbybit, int (*foo) (MECH *, dbref, int, int));
int Roll(void);
int MyHexDist(int x1, int y1, int x2, int y2, int tc);
int WeaponIsNonfunctional(MECH * mech, int section, int crit, int numcrits);
int WeaponFirstCrit(MECH * mech, int section, int crit);
int LocHasJointCrit(MECH * mech, int section);
int MechNumLegs(MECH * mech);
int MechNumJointCrits(MECH * mech, int loctype);
void CalcLegCritSpeed(MECH * mech);
int HeatFactor(MECH * mech);
int HasFuncTAG(MECH * mech);
int MechIsSwarmed(MECH * mech);
int GunStat(int weapindx, int mode, int num);
int GetAeroRange(int weapindx);
int IsDataSpecial(int index);
int NumCapacitorsOn(MECH * mech);
float EconPartWeight(int part, int index, char *name);
int IsLocDumping(MECH *mech, int loc, int *realloc);
void NarcClear(MECH *mech, int hitloc);
void AcidClear(MECH *mech, int hitloc, int bc);
int CalculateBV(MECH *mech, int gunstat, int pilstat);
int MechFullNoRecycle(MECH * mech, int num);
void SlagWeapon(MECH * wounded, MECH * attacker, int LOS, int hitloc, int critHit, int critType);
void DumpCarrier(MECH * mech);
float BombPoints(MECH * mech);
float BombThrustLoss(MECH * mech);
float MaxBombPoints(MECH * mech);
void DamageAeroSI(MECH * mech, int dmg, MECH * attacker);
MECH* FindTargetDBREFFromID(MAP * map, char *id);
int GetPartMod(MECH * mech, int t);
int ProperArmor(MECH* mech);
int ProperInternal(MECH * mech);
int alias_part(MECH * mech, int t, int loc); 
int ProperMyomer(MECH * mech);
int ReloadTime(MECH * mech, int loc, int crit, int dir);
unsigned long long int GetPartCost(int p);
unsigned long long int CalcFasaCost(MECH * mech);
/* int match_faction(dbref it, char *fact); */

#endif				/* _P_MECH_UTILS_H */
