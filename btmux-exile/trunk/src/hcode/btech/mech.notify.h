#ifndef MECH_NOTIFY_H
#define MECH_NOTIFY_H

#include "mech.h"
#include "db.h"

#define MECHPILOT 0
#define MECHSTARTED 1
#define MECHALL 2

#define cch(c) ccheck(player, mech, (c))
#define ccheck(a,b,c) if (!common_checks((a), (b), (c))) return

#include "p.mech.notify.h"

#endif				/* MECH_NOTIFY_H */
