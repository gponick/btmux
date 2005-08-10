
/* mguests.h */

/* $Id: mguests.h,v 1.2 2005/06/24 04:39:05 av1-op Exp $ */

#ifndef  __MGUESTS_H
#define __MGUESTS_H

#include "copyright.h"
#include "config.h"
#include "interface.h"

extern char *FDECL(make_guest, (DESC *));
extern dbref FDECL(create_guest, (char *, char *));
extern void FDECL(destroy_guest, (dbref));
#endif
