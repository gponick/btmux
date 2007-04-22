#ifndef MECH_ICE_H
#define MECH_ICE_H

/* mech.ice.c */
void drop_thru_ice(MECH * mech);
void break_thru_ice(MECH * mech);
int possibly_drop_thru_ice(MECH * mech);
void possibly_blow_bridge(MECH * mech, int weapindx, int x, int y);
void possibly_blow_ice(MECH * mech, int weapindx, int x, int y);

#endif				/* MECH_ICE_H */
