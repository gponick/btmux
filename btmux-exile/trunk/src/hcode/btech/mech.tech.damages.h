#ifndef MECH_TECH_DAMAGES_H
#define MECH_TECH_DAMAGES_H

/* Added RESEAL to repair flooded sections
 * -Kipsta
 * 8/4/99
 */

enum damage_type {
    REATTACH, REPAIRP, REPAIRP_T, REPAIRG, RELOAD, FIXARMOR, FIXARMOR_R,
    FIXINTERNAL, DETACH, SCRAPP, SCRAPG, UNLOAD, RESEAL,
    NUM_DAMAGE_TYPES
};

/* Reattachs / fixints / fixarmors, repair / reload */
#define MAX_DAMAGES (3 * NUM_SECTIONS + 2 * NUM_SECTIONS * NUM_CRITICALS)

#endif				/* MECH_TECH_DAMAGES_H */
