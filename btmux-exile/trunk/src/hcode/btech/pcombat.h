#ifndef PCOMBAT_H
#define PCOMBAT_H

/* pcombat.c */
int pc_to_dam_conversion(MECH * target, int weapindx, int dam);
int dam_to_pc_conversion(MECH * target, int weapindx, int dam);
int armor_effect(MECH * wounded, int cause, int hitloc, int intDamage,

    int id);

#endif				/* PCOMBAT_H */
