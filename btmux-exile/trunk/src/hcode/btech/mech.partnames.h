#ifndef MECH_PARTNAMES_H
#define MECH_PARTNAMES_H

typedef struct {
    char *shorty;
    char *longy;
    char *vlongy;
    int index;
} PN;

extern PN **short_sorted;
extern PN **long_sorted;
extern PN **vlong_sorted;
extern int object_count;

#define PACK_PART(to,id) to = id
#define UNPACK_PART(from,id) \
id = from

char *get_parts_short_name(int);
char *get_parts_long_name(int);
char *get_parts_vlong_name(int);

#include "p.mech.partnames.h"

#endif				/* MECH_PARTNAMES_H */
