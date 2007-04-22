
/*
   p.glue.scode.h

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at Mon Feb 22 14:59:38 CET 1999 from glue.scode.c */

#ifndef _P_GLUE_SCODE_H
#define _P_GLUE_SCODE_H

#include "functions.h"

/* glue.scode.c */
char *mechIDfunc(int mode, MECH * mech);
char *mechTypefunc(int mode, MECH * mech, char *arg);
char *mechMovefunc(int mode, MECH * mech, char *arg);
char *mechTechTimefunc(int mode, MECH * mech);
void apply_mechDamage(MECH * omech, char *buf);
char *mechDamagefunc(int mode, MECH * mech, char *arg);
char *mechCentBearingfunc(int mode, MECH * mech, char *arg);
char *mechCentDistfunc(int mode, MECH * mech, char *arg);
void set_xcodestuff(dbref player, void *data, char *buffer);
void list_xcodestuff(dbref player, void *data, char *buffer);
void list_xcodevalues(dbref player);

XFUNCTION(fun_btaddparts);
XFUNCTION(fun_btarmorstatus);
XFUNCTION(fun_btcritstatus);
XFUNCTION(fun_btdamagemech);
XFUNCTION(fun_btdamages);
XFUNCTION(fun_btdesignex);
XFUNCTION(fun_btgetcharvalue);
XFUNCTION(fun_btgetxcodevalue);
XFUNCTION(fun_btloadmap);
XFUNCTION(fun_btloadmech);
XFUNCTION(fun_btmakemechs);
XFUNCTION(fun_btmakepilotroll);
XFUNCTION(fun_btmapelev);
XFUNCTION(fun_btmapterr);
XFUNCTION(fun_btmechfreqs);
XFUNCTION(fun_btpartmatch);
XFUNCTION(fun_btpartname);
XFUNCTION(fun_btsetarmorstatus);
XFUNCTION(fun_btsetcharvalue);
XFUNCTION(fun_btsetxcodevalue);
XFUNCTION(fun_btstores);
XFUNCTION(fun_bttechstatus);
XFUNCTION(fun_btthreshold);
XFUNCTION(fun_btunderrepair);
XFUNCTION(fun_btweaponstatus);
XFUNCTION(fun_btaddstores);
XFUNCTION(fun_btarmorstatus_ref);
XFUNCTION(fun_btcharlist);
XFUNCTION(fun_btcritslot);
XFUNCTION(fun_btcritslot_ref);
XFUNCTION(fun_btcritstatus_ref);
XFUNCTION(fun_btengrate);
XFUNCTION(fun_btengrate_ref);
XFUNCTION(fun_btfasabasecost_ref);
XFUNCTION(fun_btgetbv);
XFUNCTION(fun_btgetbv_ref);
XFUNCTION(fun_btgetpartcost);
XFUNCTION(fun_btgetrange);
XFUNCTION(fun_btgetrealmaxspeed);
XFUNCTION(fun_btgetreftech_ref);
XFUNCTION(fun_btgetweight);
XFUNCTION(fun_btgetxcodevalue_ref);
XFUNCTION(fun_bthexemit);
XFUNCTION(fun_bthexinblz);
XFUNCTION(fun_bthexlos);
XFUNCTION(fun_btid2db);
XFUNCTION(fun_btlistblz);
XFUNCTION(fun_btlosm2m);
XFUNCTION(fun_btmapemit);
XFUNCTION(fun_btnumrepjobs);
XFUNCTION(fun_btparttype);
XFUNCTION(fun_btgetweight);
XFUNCTION(fun_btpayload_ref);
XFUNCTION(fun_btremovestores);
XFUNCTION(fun_btsetmaxspeed);
XFUNCTION(fun_btsetpartcost);
XFUNCTION(fun_btsetxy);
XFUNCTION(fun_btshowcritstatus_ref);
XFUNCTION(fun_btshowstatus_ref);
XFUNCTION(fun_btshowwspecs_ref);
XFUNCTION(fun_bttech_ref);
XFUNCTION(fun_bttechlist);
XFUNCTION(fun_bttechlist_ref);
XFUNCTION(fun_bttechtime);
XFUNCTION(fun_btunitfixable);
XFUNCTION(fun_btweaponstatus_ref);
XFUNCTION(fun_btweapstat);
                                                                            

#endif				/* _P_GLUE_SCODE_H */
