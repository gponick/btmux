#ifndef MECH_TECH_H
#define MECH_TECH_H

#include "mech.events.h"

/* In minutes */
#define MAX_TECHTIME 480
#if 1
#define TECH_TICK     60
#define TECH_UNIT "minute"
#else
#define TECH_TICK      1
#define TECH_UNIT "second"
#endif

/* Tech skill modifiers ; + = bad, - = good */
#define PARTTYPE_DIFFICULTY(a) (1)
#define WEAPTYPE_DIFFICULTY(a) ((int) (sqrt(MechWeapons[Weapon2I(a)].criticals)*1.5-1.5))
#define REPAIR_DIFFICULTY     0
#define REPLACE_DIFFICULTY     1
#define RELOAD_DIFFICULTY      0
#define FIXARMOR_DIFFICULTY    1
#define FIXINTERNAL_DIFFICULTY 2
#define REATTACH_DIFFICULTY    3
#define REMOVEG_DIFFICULTY     1
#define REMOVEP_DIFFICULTY     0
#define REMOVES_DIFFICULTY     2
#define RESEAL_DIFFICULTY      0

/* Times are in minutes */
#define MOUNT_BOMB_TIME  5
#define UMOUNT_BOMB_TIME 5
#define REPLACEGUN_TIME  60
#define REPLACEPART_TIME 20
#define REPAIRGUN_TIME   20
#define REPAIRPART_TIME  10
#define RELOAD_TIME      5
#define BSUIT_RELOAD_TIME 3
#define FIXARMOR_TIME    2
#define FIXINTERNAL_TIME 7
/* #define REATTACH_TIME    240 */
#define REATTACH_TIME(a) ((a / 5) * 10) 
#define REMOVEP_TIME     40
#define REMOVEG_TIME     40
/* #define REMOVES_TIME     120 */
#define REMOVES_TIME(a) ((a / 5) * 8)
#define RESEAL_TIME    60	//Added 8/4/99. Kipsta.

#define TECHCOMMANDH(a) \
   void a (dbref player, void * data, char * buffer)
#define TECHCOMMANDB \
 MECH *mech = (MECH *) data; \
 int loc, part, t, full, now, from, to, change, mod=2, isds=0, notic = 0; \
 char *c;


#define TECHCOMMANDCOMNI \
DOCHECK(!(Tech(player)) && In_Character(mech->mynum),"Insufficient clearance to access the command."); \
DOCHECK(!mech, "Error has occured in techcommand ; please contact a wiz"); \
isds = DropShip(MechType(mech)); \
DOCHECK(Starting(mech) && !Wiz(player), "The mech's starting up! Please stop the sequence first."); \
DOCHECK(Started(mech) && !Wiz(player), "The mech's started up ; please shut it down first."); \
DOCHECK(!isds && !MechStall(mech) && !Wiz(player) && In_Character(mech->mynum), "The 'mech isn't in a repair stall!"); 


#define TECHCOMMANDC \
DOCHECK(!(Tech(player)),"Insufficient clearance to access the command."); \
DOCHECK(!mech, "Error has occured in techcommand ; please contact a wiz"); \
isds = DropShip(MechType(mech)); \
DOCHECK(Starting(mech) && !Wiz(player), "The mech's starting up! Please stop the sequence first."); \
DOCHECK(Started(mech) && !Wiz(player), "The mech's started up ; please shut it down first."); \
DOCHECK(!isds && !MechStall(mech) && !Wiz(player), "The 'mech isn't in a repair stall!"); 

#define TECHCOMMANDD \
DOCHECK(!(Tech(player)),"Insufficient clearance to access the command."); \
DOCHECK(!mech, "Error has occured in techcommand ; please contact a wiz"); \
isds = DropShip(MechType(mech)); \
DOCHECK(Starting(mech) && !Wiz(player), "The mech's starting up! Please stop the sequence first."); \
DOCHECK(Started(mech) && !Wiz(player), "The mech's started up ; please shut it down first."); \
DOCHECK(mudconf.btech_limitedrepairs && !isds && !MechStall(mech) && !Wiz(player), "The 'mech isn't in a repair stall!");

#define ETECHCOMMAND(a) \
 void a (dbref player, void *data, char *buffer)

#define LOCMAX 16
#define POSMAX 16
#define EXTMAX 256
#define PLAYERPOS (LOCMAX*POSMAX*EXTMAX)

#define TECHEVENT(a) \
   void a (EVENT *e) \
     { MECH *mech = (MECH *) e->data;  \
       int earg = (int) (e->data2) % PLAYERPOS;

#define ETECHEVENT(a) \
   extern void a (EVENT *e)

#define START(a) notify(player, a)
#define FIXEVENT(time,d1,d2,fu,type) \
     event_add((mudconf.btech_freetechtime ? 2 : MAX(2, time)), 0, type, fu, (void *) d1, (void *) ((d2) + player * PLAYERPOS))
#define REPAIREVENT(time,d1,d2,fu,type) \
     FIXEVENT((time)*TECH_TICK,d1,d2,fu,type)
#define STARTREPAIR(time,d1,d2,fu,type) \
     FIXEVENT(tech_addtechtime(player, (time * mod) / 2),d1,d2,fu,type)
#define STARTIREPAIR(time,d1,d2,fu,type,amount) \
     FIXEVENT((tech_addtechtime(player, (time * mod) / 2) - (amount > 0 ? TECH_TICK * (time * (amount - 1) / (amount)) : 0)), d1, d2, fu, type)
#define FAKEREPAIR(time,type,d1,d2) \
     FIXEVENT(tech_addtechtime(player, (time * mod) / 2),d1,d2,very_fake_func,type)
#define CHECKREPAIRTIME(player,time) DOCHECK(tech_playertime(player, time) >= MAX_TECHTIME, "You're to exhausted for THAT much more tech work.")


/* replace gun/part, repair gun/part (loc/pos) */
#define DOTECH_LOCPOS(diff,flunkfunc,succfunc,resourcefunc,time,d1,d2,fu,type,msg,isgun)\
   if (resourcefunc(player,mech,loc,part)>=0) { START(msg); \
   if ((!isgun && tech_roll(player, mech, diff) < 0) || \
       (isgun && tech_weapon_roll(player, mech, diff) < 0)) { mod = 3;  \
   if (flunkfunc(player,mech,loc,part)<0) { FAKEREPAIR(time,type,d1,d2); return;}} \
    else \
     { if (succfunc(player,mech,loc,part)<0) return; } \
     STARTREPAIR(time,d1,d2,fu,type); }

/* reload (loc/pos/amount) */
#define DOTECH_LOCPOS_VAL(diff,flunkfunc,succfunc,resourcefunc,amo,time,d1,d2,fu,type,msg)\
   if (resourcefunc(player,mech,loc,part,amo)<0) return; \
   START(msg); \
   if (tech_roll(player, mech, diff) < 0) { mod = 3; \
   if (flunkfunc(player,mech,loc,part,amo)<0) {FAKEREPAIR(time,type,d1,d2);return;}}\
     else \
   { if (succfunc(player,mech,loc,part,amo)<0) return; } \
   STARTREPAIR(time,d1,d2,fu,type)


/* fixarmor/internal (loc/amount) */
#define DOTECH_LOC_VAL_S(diff,flunkfunc,succfunc,resourcefunc,amo,time,type,d1,d2,msg) \
   if (resourcefunc(player,mech,loc,amo)<0) return; \
   START(msg); \
   if (tech_roll(player, mech, diff) < 0) { mod = 3; \
   if (flunkfunc(player,mech,loc,amo)<0) { FAKEREPAIR(time,type,d1,d2); return; }} \
   else \
     { if (succfunc(player,mech,loc,amo)<0) return; }

#define DOTECH_LOC_VAL(diff,flunkfunc,succfunc,resourcefunc,amo,time,d1,d2,fu,type,msg) \
   if (resourcefunc(player,mech,loc,amo)<0) return; \
   START(msg); \
   if (tech_roll(player, mech, diff) < 0) { mod = 3; \
   if (flunkfunc(player,mech,loc,amo)<0) { FAKEREPAIR(time,type,d1,d2); return; }} \
   else \
     { if (succfunc(player,mech,loc,amo)<0) return; } \
   STARTREPAIR(time,d1,d2,fu,type)

/* reattach and reseal (loc) */
#define DOTECH_LOC(diff,flunkfunc,succfunc,resourcefunc,time,d1,d2,fu,type,msg) \
   if (resourcefunc(player,mech,loc)<0) return; \
   START(msg); \
   if (tech_roll(player, mech, diff) < 0) { mod = 3; \
   if (flunkfunc(player,mech,loc)<0) { FAKEREPAIR(time,type,d1,d2);return; }} \
    else \
   { if (succfunc(player,mech,loc)<0) return; } \
   STARTREPAIR(time,d1,d2,fu,type)

#define TFUNC_LOCPOS_VAL(name) \
int name (dbref player,MECH *mech,int loc,int part, int * val)
#define TFUNC_LOC_VAL(name) \
int name (dbref player, MECH *mech, int loc, int * val)
#define TFUNC_LOCPOS(name) \
int name (dbref player, MECH *mech, int loc, int part)
#define TFUNC_LOC(name) \
int name (dbref player, MECH *mech, int loc)
#define TFUNC_LOC_RESEAL(name) int name (dbref player, MECH *mech, int loc)
#define NFUNC(a) a { return 0; }



ETECHCOMMAND(tech_removegun);
ETECHCOMMAND(tech_removepart);
ETECHCOMMAND(tech_removesection);
ETECHCOMMAND(tech_replacegun);
ETECHCOMMAND(tech_repairgun);
ETECHCOMMAND(tech_oaddgun);
ETECHCOMMAND(tech_oremgun);
ETECHCOMMAND(tech_omodammo); 
ETECHCOMMAND(tech_oaddsp);
ETECHCOMMAND(tech_replacepart);
ETECHCOMMAND(tech_repairpart);
ETECHCOMMAND(tech_reload);
ETECHCOMMAND(tech_unload);
ETECHCOMMAND(tech_fixarmor);
ETECHCOMMAND(tech_fixinternal);
ETECHCOMMAND(tech_reattach);
ETECHCOMMAND(tech_checkstatus);
ETECHCOMMAND(tech_reseal);
ECMD(show_mechs_damage);
ECMD(tech_fix);

#define PACK_LOCPOS(loc,pos)          ((loc) + (pos)*LOCMAX)
#define PACK_LOCPOS_E(loc,pos,extra)  ((loc) + (pos)*LOCMAX + (extra)*LOCMAX*POSMAX)

#define UNPACK_LOCPOS(var,loc,pos)  loc = (var % LOCMAX);pos = (var / LOCMAX) % POSMAX
#define UNPACK_LOCPOS_E(var,loc,pos,extra) UNPACK_LOCPOS(var,loc,pos);extra = var / (LOCMAX * POSMAX)

#if 0
#define GrabParts(a,b,c)  econ_change_items(Location(Location(player)),a,b,0-c)
#define PartAvail(a,b,c)  (econ_find_items(Location(Location(player)),a,b)>=c)
#define AddParts(a,b,c)   econ_change_items(Location(Location(player)),a,b,c)
#define AVCHECK(a,b,c)    DOCHECK1(!PartAvail(a,b,c), tprintf("Not enough %ss in store!",part_name(a,b)));
#endif

#if 0 /* Let's use the functions overriding macros */
#define ProperArmor(mech) \
(Cargo(\
       (MechSpecials(mech) & FF_TECH) ? FF_ARMOR : \
       (MechSpecials(mech) & HARDA_TECH) ? HD_ARMOR : \
       (MechSpecials2(mech) & (LFF_TECH|HFF_TECH)) ? FF_ARMOR : \
       S_ARMOR))

#define ProperInternal(mech) \
(Cargo(\
       (MechSpecials(mech) & ES_TECH) ? ES_INTERNAL : \
       (MechSpecials(mech) & REINFI_TECH) ? RE_INTERNAL : \
       (MechSpecials(mech) & COMPI_TECH) ? CO_INTERNAL : \
       S_INTERNAL))

#define alias_part(m,t) \
  (IsActuator(t) ? Cargo(S_ACTUATOR) : \
   (t == Special(ENGINE) && MechSpecials(m) & XL_TECH) ? Cargo(XL_ENGINE) : \
   (t == Special(ENGINE) && MechSpecials(m) & ICE_TECH) ? Cargo(IC_ENGINE) : \
   (t == Special(ENGINE) && MechSpecials(m) & CE_TECH) ? Cargo(COMP_ENGINE) : \
   (t == Special(ENGINE) && MechSpecials(m) & XXL_TECH) ? Cargo(XXL_ENGINE) : \
   (t == Special(ENGINE) && MechSpecials2(m) & LENG_TECH) ? Cargo(LIGHT_ENGINE) : \
   (t == Special(HEAT_SINK) && MechSpecials(m) & (DOUBLE_HEAT_TECH|CLAN_TECH)) ? Cargo(DOUBLE_HEAT_SINK) : \
   (t == Special(HEAT_SINK) && MechSpecials2(m) & (COMPACT_HEAT_TECH)) ? Cargo(COMPACT_HEAT_SINK) : \
   (t == Special(GYRO) && MechSpecials2(m) & (XLGYRO_TECH)) ? Cargo(XL_GYRO) : \
   (t == Special(GYRO) && MechSpecials2(m) & (HDGYRO_TECH)) ? Cargo(HD_GYRO) : \
   (t == Special(GYRO) && MechSpecials2(m) & (CGYRO_TECH)) ? Cargo(COMP_GYRO) : \
   t)
#endif
/* mod for dez. if the mech/vehicle has a bay 0 then take parts from it. */
#if 0
#define GrabPartsM(m,a,b,c) econ_change_items(IsDS(m) ? AeroBay(m,0) : Location(m->mynum),a,b,0-c)
#define PartAvailM(m,a,b,c) (econ_find_items(IsDS(m) ? AeroBay(m,0) : Location(m->mynum),a,b)>=c)
#define AddPartsM(m,a,b,c) econ_change_items(IsDS(m) ? AeroBay(m,0) : Location(m->mynum), alias_part(m, a) , b, c)
#endif

#define GrabPartsM(m,a,c) econ_change_items((AeroBay(m,0) > 0) ? AeroBay(m,0) : Location(m->mynum),a,0-c)
#define PartAvailM(m,a,c) (econ_find_items((AeroBay(m,0) > 0) ? AeroBay(m,0) : Location(m->mynum),a)>=c)
#define AddPartsM(m,l,a,c) econ_change_items((AeroBay(m,0) > 0) ? AeroBay(m,0) : Location(m->mynum), alias_part(m, a, l) , c)
#define AVCHECKM(m,a,c)    DOCHECK1(!PartAvailM(m,a,c), tprintf("Not enough %ss in store!",part_name(a)));
#define AVCHECKM2(m,a,c)    DOCHECK(!PartAvailM(m,a,c), tprintf("Not enough %ss in store!",part_name(a)))

ETECHEVENT(event_mech_reattach);
ETECHEVENT(event_mech_reseal);
ETECHEVENT(event_mech_reload);
ETECHEVENT(event_mech_removegun);
ETECHEVENT(event_mech_removepart);
ETECHEVENT(event_mech_removesection);
ETECHEVENT(event_mech_repairarmor);
ETECHEVENT(event_mech_repairgun);
ETECHEVENT(event_mech_repairinternal);
ETECHEVENT(event_mech_repairpart);
ETECHEVENT(event_mech_replacegun);
ETECHEVENT(event_mech_mountbomb);
ETECHEVENT(event_mech_umountbomb);
ETECHEVENT(very_fake_func);

void loadrepairs(FILE * f);
void saverepairs(FILE * f);
int valid_ammo_mode(MECH * mech, int loc, int part, int let);

#endif				/* MECH_TECH_H */
