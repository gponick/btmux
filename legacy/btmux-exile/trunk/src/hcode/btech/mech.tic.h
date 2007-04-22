#ifndef MECH_TIC_H
#define MECH_TIC_H

/* mech.tic.c */
void cleartic_sub(dbref player, MECH * mech, char *buffer);
void addtic_sub(dbref player, MECH * mech, char *buffer);
void deltic_sub(dbref player, MECH * mech, char *buffer);
void firetic_sub(dbref player, MECH * mech, char *buffer);
void listtic_sub(dbref player, MECH * mech, char *buffer);
void mech_cleartic(dbref player, void *data, char *buffer);
void mech_addtic(dbref player, void *data, char *buffer);
void mech_deltic(dbref player, void *data, char *buffer);
void mech_firetic(dbref player, void *data, char *buffer);
void mech_listtic(dbref player, void *data, char *buffer);
void heat_cutoff(dbref player, void *data, char *buffer);

#endif				/* MECH_TIC_H */
