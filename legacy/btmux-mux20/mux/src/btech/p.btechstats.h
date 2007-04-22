
/*
   p.btechstats.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Mon Mar 22 10:40:18 CET 1999 from btechstats.c */

#include "config.h"

#ifndef _P_BTECHSTATS_H
#define _P_BTECHSTATS_H

/* btechstats.c */
char *silly_get_uptime_to_string(int i);
void list_charvaluestuff(dbref player, int flag);
int char_getvaluecode(char *name);
int char_rollsaving(void);
int char_rollunskilled(void);
int char_rollskilled(void);
int char_rolld6(int num);
int char_getvalue(dbref player, char *name);
void char_setvalue(dbref player, char *name, int value);
int char_getskilltargetbycode(dbref player, int code, int modifier);
int char_getskilltarget(dbref player, char *name, int modifier);
int char_getxpbycode(dbref player, int code);
int char_gainxpbycode(dbref player, int code, int amount);
int char_gainxp(dbref player, char *skill, int amount);
int char_getskillsuccess(dbref player, char *name, int modifier, int loud);
int char_getskillmargsucc(dbref player, char *name, int modifier);
int char_getopposedskill(dbref first, char *skill1, dbref second,
    char *skill2);
int char_getattrsave(dbref player, char *name);
int char_getattrsavesucc(dbref player, char *name);
void zap_unneccessary_stats(void);
void init_btechstats(void);
void do_charstatus(dbref player, dbref cause, int key, char *arg1);
void do_charclear(dbref player, dbref cause, int key, char *arg1);
dbref char_lookupplayer(dbref player, dbref cause, int key, char *arg1);
void initialize_pc(dbref player, MECH * mech);
void fix_pilotdamage(MECH * mech, dbref player);
int mw_ic_bth(MECH * mech);
int handlemwconc(MECH * mech, int initial);
void headhitmwdamage(MECH * mech, int dam);
void mwlethaldam(MECH * mech, int dam);
void lower_xp(dbref player, int promillage);
void AccumulateTechXP(dbref pilot, MECH * mech, int reason);
void AccumulateTechWeaponsXP(dbref pilot, MECH * mech, int reason);
void AccumulateCommXP(dbref pilot, MECH * mech);
void AccumulatePilXP(dbref pilot, MECH * mech, int reason, int addanyway);
void AccumulateSpotXP(dbref pilot, MECH * attacker, MECH * wounded);
int MadePerceptionRoll(MECH * mech, int modifier);
void AccumulateArtyXP(dbref pilot, MECH * attacker, MECH * wounded);
void AccumulateComputerXP(dbref pilot, MECH * mech, int reason);
int HasBoolAdvantage(dbref player, const char *name);
void AccumulateGunXP(dbref pilot, MECH * attacker, MECH * wounded,
    int numOccurences, int multiplier, int weapindx, int bth);
void AccumulateGunXPold(dbref pilot, MECH * attacker, MECH * wounded,
    int numOccurences, int multiplier, int weapindx, int bth);
void fun_btgetcharvalue(char *buff, char **bufc, dbref player, dbref cause,
    char *fargs[], int nfargs, char *cargs[], int ncargs);
void fun_btsetcharvalue(char *buff, char **bufc, dbref player, dbref cause,
    char *fargs[], int nfargs, char *cargs[], int ncargs);
void fun_btcharlist(char *buff, char **bufc, dbref player, dbref cause,
    char *fargs[], int nfargs, char *cargs[], int ncargs);
void debug_xptop(dbref player, void *data, char *buffer);
void debug_setxplevel(dbref player, void *data, char *buffer);
int btthreshold_func(char *skillname);
struct chargen_struct *retrieve_chargen_struct(dbref player);
int lowest_bit(int num);
int recursive_add(int lev);
int can_proceed(dbref player, struct chargen_struct *st);
int can_advance_state(struct chargen_struct *st);
int can_go_back_state(struct chargen_struct *st);
void recalculate_skillpoints(struct chargen_struct *st);
void go_back_state(dbref player, struct chargen_struct *st);

#endif				/* _P_BTECHSTATS_H */
