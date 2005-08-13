#ifndef MECH_STAT_H
#define MECH_STAT_H

typedef struct {
    int rolls[11];
    int hitrolls[11];
    int critrolls[11];
    int totrolls;
    int tothrolls;
    int totcrolls;
} stat_type;

#ifndef MECH_STAT_C
extern stat_type stat;
#endif
#endif				/* MECH.STAT_H */
