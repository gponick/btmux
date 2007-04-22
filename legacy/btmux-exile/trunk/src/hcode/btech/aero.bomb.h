#ifndef AERO_BOMB_H
#define AERO_BOMB_H

typedef struct {
    char *name;
    int aff;
    int type;			/* 0 = standard, 1 = inferno, 2 = cluster */
    int weight;
    int cap;
} BOMBINFO;

typedef struct {
    int x, y, type;
    MAP *map;
    MECH *aero;
} bomb_shot;

#endif				/* AERO_BOMB_H */
