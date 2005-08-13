#ifndef MACROS_H
#define MACROS_H

#ifdef notify
#undef notify
#endif
#ifdef notify_except
#undef notify_except
#endif
#define notify(a,b) mecha_notify(a,b)
#define notify_except(a,b,c,d) mecha_notify_except(a,b,c,d)

/* DOCHECK: Macros for the lazy
   DOCHEK(a,b) basically replaces if (a) { <somewaytopassmessage b>, return }

   Macros ending with 0 return 0, and N NULL. Default behaviour is to
   return nothing.

   Default behavior to pass message is to use notify to notify the
   player who's executing the function. MA = MECHALL, sends to
   everyone within the mech executing the command, MP = MECHPILOT,
   sends to pilot of the mech
   */
#ifndef DOCHECK
#define DOCHECK(a,b) if (a) { notify(player, b); return; }
#define DOCHECKMA(a,b) if (a) { mech_notify(mech, MECHALL, b); return; }
#define DOCHECKMA0(a,b) if (a) { mech_notify(mech, MECHALL, b); return 0; }
#define DOCHECKMP(a,b) if (a) { mech_notify(mech, MECHPILOT, b); return; }
#define DOCHECKMP0(a,b) if (a) { mech_notify(mech, MECHPILOT, b); return 0; }
#define DOCHECKMP1(a,b) if (a) { mech_notify(mech, MECHPILOT, b); return 1; }
#define DOCHECK0(a,b) if (a) { notify(player, b); return 0; }
#define DOCHECK1(a,b) if (a) { notify(player, b); return -1; }
#define DOCHECKN(a,b) if (a) { notify(player, b); return NULL; }
#define FUNCHECK(a,b) \
if (a) { safe_tprintf_str(buff, bufc, b); return; }
#endif

/* Dice-rolling function used everywhere converted to a macro */
#define Number(a,b) ((a) + random()%((b)-(a)+1))


#define skipws(name)     while (name && *name && isspace(*name)) name++
#define readint(to,from) \
(!from || !*from || (!(to = atoi(from)) && strcmp(from, "0")))

#define Readnum(tovar,fromvar) \
        (!(tovar = atoi(fromvar)) && strcmp(fromvar, "0"))

#define SetBit(val,bit)   (val |= bit)
#define UnSetBit(val,bit) (val &= ~(bit))
#define EvalBit(val,bit,state) \
        do {if (state) SetBit(val,bit); else UnSetBit(val,bit);} while (0)
#define ToggleBit(val,bit) \
do { if (!(val & bit)) SetBit(val,bit);else UnSetBit(val,bit); } while (0)

#define WizPo(p,fun) (fun(Owner(p)) && Inherits(p))

#define Wiz(p)  WizPo(p,Wizard)
#define WizR(p) WizPo(p,WizRoy)
#define WizP(p) WizPo(p,Security)

#define hush_teleport(p,t) move_via_teleport(p,t,1,7)
#define loud_teleport(p,t) move_via_teleport(p,t,1,0)

#define can_pass_lock(guy,lockobj,lockname) could_doit(guy,lockobj,lockname)


#endif				/* MACROS_H */
