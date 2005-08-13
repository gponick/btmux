#ifndef MECH_ECM_H
#define MECH_ECM_H

/* mech.ecm.c */
/* void cause_ecm(MECH * from, MECH * to); */
/* void end_ecm_check(MECH * mech); */

void cause_ecm(MECH * from, MECH * to);
void end_ecm_check(MECH * mech); 
int ecm_count(MECH * mech, int mode);
int eccm_count(MECH * mech, int mode); 
int ecm_affect_hex(MECH * mech,int x, int y, int z, int *IsAngel);

#endif				/* MECH.ECM_H */
