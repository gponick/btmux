#ifndef STRHANDLER_H
#define STRHANDLER_H

#include "db.h"
#include "externs.h"

#define MAX_NUM_FIFOS 4
#define MAX_NUM_STRS  62

char *strarray_get_obj_atr(dbref obj, int atr, int qnum, int num);
void strarray_add_obj_atr(dbref obj, int atr, int qnum, int max,

    char *val);
void strarray_add_obj_atr_n(dbref obj, int atr, int qnum, int n,

    char *val);

#endif				/* STRHANDLER_H */
