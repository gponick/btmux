#include "mech.h"
#include "mech.sensor.h"
#include "p.map.obj.h"
#include "p.mech.update.h"
#include "p.mech.utils.h"
#include "p.template.h"

/* Full chance of seeing, except if range > conditionrange */
SEEFUNC(vislight_see, r > (c * ((!l && t &&
		IsLit(t)) ? 3 : 1)) ? 0 : (100 - (r / 3)) / ((t &&
	    (MechType(t) == CLASS_BSUIT || MechType(t) == CLASS_MW))
	? 3 : 1));

/* In perfect darkness, 2x conditionrange. Otherwise conditionrange,
   always same chance of seeing */
SEEFUNC(liteamp_see, ((!l && r > (2 * c)) ? 0 : (l &&
	    r > c) ? 0 : (70 - r)) / ((t && (MechType(t) == CLASS_BSUIT ||
		MechType(t) == CLASS_MW))
	? 3 : 1));

/* Always same chance, if within range */
SEEFUNC(infrared_see, (100 - r ));
SEEFUNC(electrom_see, MAX(r < 24 ? 2 : 0, (120 - (r * 4))));
SEEFUNC(seismic_see, 60 - (r * 4));

SEEFUNC(radar_see, BOUNDED(20, (120 - r), 90));
SEEFUNC(bap_see, 101);

/* Prior requirement: the seechance > 0. We assume it so,
   and only examine the flag. */
#define wood_count(a)  (((a / MECHLOSFLAG_WOOD) % MECHLOSMAX_WOOD))
#define smoke_count(a) (((a / MECHLOSFLAG_SMOKE) % MECHLOSMAX_SMOKE))
#define water_count(a) (((a / MECHLOSFLAG_WATER) % MECHLOSMAX_WATER))
#define build_count(a) (((a / MECHLOSFLAG_BUILD) % MECHLOSMAX_BUILD))

extern float ActualElevation(MAP * map, int x, int y, MECH * mech);

/* Visual methods are hampered by excess woods / water */
CSEEFUNC(vislight_csee,
    !(f & (MECHLOSFLAG_BLOCK | MECHLOSFLAG_FIRE)) && (wood_count(f) + smoke_count(f)) < 3 && 
    (!t || MechZ(t) >= 0 || ActualElevation(getMap(t->mapindex), MechX(t), MechY(t), t) >= 0.0
	|| water_count(f) < 6));

/* Liteamp doesn't see into water, thanks to reflections etc */
CSEEFUNC(liteamp_csee,
    !(f & (MECHLOSFLAG_BLOCK | MECHLOSFLAG_FIRE)) &&
    (!t || !IsLit(t)) && (wood_count(f) + smoke_count(f)) < 2 && !(water_count(f)));

/* Not too good with woods, infra.. too much variation in temperature */
CSEEFUNC(infrared_csee, !(f & (MECHLOSFLAG_BLOCK | MECHLOSFLAG_FIRE)) &&
    (wood_count(f) + (smoke_count(f))) < 6 && 
    (!t || ((MechType(t) == CLASS_MECH) && (MechPlusHeat(t) > 0))
    || ((MechType(t) == CLASS_VEH_GROUND) && Started(t))  
    || ((MechType(t) != CLASS_BSUIT && MechType(t) != CLASS_MW))));

/* Mountains provide too much hindarance in terms of electromagnetic
   detection */
CSEEFUNC(electrom_csee, !(f & (MECHLOSFLAG_BLOCK | MECHLOSFLAG_MNTN)) &&
    ((wood_count(f) + (build_count(f) * 2)) < 10) && (!t || ((MechType(t) != CLASS_MW))));

/* Seismic sees. Period. */
CSEEFUNC(seismic_csee, t && (!Jumping(m) && abs(MechSpeed(t)) > MP1  && (MechType(t) != CLASS_BSUIT &&
	    MechType(t) != CLASS_MW) && !Jumping(t) && MechMove(t) != MOVE_HOVER &&
		((MechMove(t) != MOVE_VTOL && MechMove(t) != MOVE_FLY) || Landed(t)) && MechMove(t) != MOVE_NONE && Started(t)));

/* Radar does the same thing */
/* CSEEFUNC(radar_csee, t && MechZ(t) > 2 && !(f & MECHLOSFLAG_BLOCK) &&
    (MechZ(t) >= 10 || (r < (MechZ(t) * MechZ(t)))) && !(MechStatus(m)
    & ECM_DISTURBANCE) && MechZ(t) > (MechsElevation(t) + 3)); */

CSEEFUNC(radar_csee, t && MechZ(t) > 2 && !(f & MECHLOSFLAG_BLOCK) &&
    ((MechStatus2(m) & CS_ON_LAND) ? !Landed(m) : 1) &&
    (MechZ(m) < ATMO_Z ? MechZ(t) < ATMO_Z :
     MechZ(m) < ORBIT_Z ? (MechZ(t) < ORBIT_Z && MechZ(t) > ATMO_Z) : 
     MechZ(m) >= ORBIT_Z ? MechZ(t) >= ORBIT_Z : 0) && (MechsElevation(t) > 2));

CSEEFUNC(bap_csee, !(MechStatus(m) & ECM_DISTURBANCE) && !(MechStatus2(m) & SPRINTING));

/* Basically, mechs w/o heat are +2 tohit, mechs in utter overheat are
   -3 tohit */
/*
#define HEAT_MODIFIER(a) ((a) <= 7 ? 2 : (a) > 28 ? -3 : (a) > 21 ? -2 : (a) > 14 ? 0 : 1)
#define HEAT_MODIFIER(a) ((a) <= 4 ? 2 : (a) <= 8 ? 1 : (a) <= 12 ? 0 : (a) <= 16 ? -1 : (a) <= 20 ? -2 : -3)
#define HEAT_MODIFIER(a) ((a) <= .75 ? 2 : (a) <= 1.25 ? 1 : (a) <= 1.75 ? 0 : (a) <= 2.25 ? -1 : (a) <= 2.75 ? -2 : (a) > 2.75 ? -3 : 2)
*/

#define HEAT_MODIFIER(a) ((a) < 0 ? 2 : (a) > 50 ? -2 : (a) > 35 ? -1 : (a) > 20 ? 0 : 1) 
/* Heavy/assault -1 tohit, medium 0, light +1 */
#define WEIGHT_MODIFIER(a) (a >= 60  ? -1 : a >= 30 ? 0 : 1)

/* If target's moving, +1 tohit */
#define MOVE_MODIFIER(a) (abs(a) > MP1 ? 1 : 0)

#define nsmoke_count(mech,a) (((a / MECHLOSFLAG_SMOKE) % MECHLOSMAX_SMOKE) + \
			     ((MechElevation(mech) + 4) < MechZ(mech) ? 0 : \
			      MechRTerrain(mech) == SMOKE ? 1 : \
			      MechRTerrain(mech) == HSMOKE ? 2 : 0))
#define nwood_count(mech,a)  (((a / MECHLOSFLAG_WOOD) % MECHLOSMAX_WOOD) + \
                             ((MechElevation(mech) + 2) < MechZ(mech) ? 0 : \
			      MechRTerrain(mech) == LIGHT_FOREST ? 1 : \
			      MechRTerrain(mech) == HEAVY_FOREST ? 2 : 0))
#define nbuild_count(mech,a) (((a / MECHLOSFLAG_BUILD) % MECHLOSMAX_BUILD) + \
			     ((MechElevation(mech) + 2) < MechZ(mech) ? 0 : \
			      MechRTerrain(mech) == BUILDING ? 1 : 0))
/* To-hit functions */
TOHITFUNC(vislight_tohit, ((!t || !IsLit(t)) ? (2 - l) : 0) + nwood_count(t, f) + nsmoke_count(t, f) + 
	    ((f & MECHLOSFLAG_PARTIAL) ? ((MechCritStatus(t) & DUG_IN && MechMove(t) == MOVE_QUAD) ? 5 : 3) : 0));
TOHITFUNC(liteamp_tohit, (2 - l) / 2 + (((nwood_count(t, f) + nsmoke_count(t, f)) * 3) / 2) +  
	    ((f & MECHLOSFLAG_PARTIAL) ? ((MechCritStatus(t) & DUG_IN && MechMove(t) == MOVE_QUAD) ? 5 : 3) : 0));
TOHITFUNC(infrared_tohit, ((((nwood_count(t, f) + ((nsmoke_count(t, f) + 1) / 2)) * 4) / 3) + 
	 ((f & MECHLOSFLAG_PARTIAL) ? ((MechCritStatus(t) & DUG_IN && MechMove(t) == MOVE_QUAD) ? 5 : 3) : 0) +
         HEAT_MODIFIER(HeatFactor(t))));
TOHITFUNC(electrom_tohit, (((nwood_count(t, f) + ((nsmoke_count(t, f) + 1) / 2))  * 2) / 3) + 
    ((f & MECHLOSFLAG_PARTIAL) ? ((MechCritStatus(t) & DUG_IN && MechMove(t) == MOVE_QUAD) ? 4 : 2) : 0) +
    WEIGHT_MODIFIER(MechRTons(t) / 1024) + (MOVE_MODIFIER(MechSpeed(t))) + (nbuild_count(t, f)) +
    (MechEngineHeat(t) > 0 ? -1 : 0) + ((MechStatus2(t) & BEAGLE_ACTIVE) ? -2 : 0) + (((MechStatus2(t) & ECM_PROTECTED) || 
    (f & MECHLOSFLAG_ECM) || (MechStatus(m) & ECM_DISTURBANCE)) ? 1 : 0) + (MechStatus(t) & FIRED ? -1 : 0));

/* Seismic's hurt by enemy staying in place / light mechs */
TOHITFUNC(seismic_tohit,
    2 + ((f & MECHLOSFLAG_PARTIAL) ? ((MechCritStatus(t) & DUG_IN && MechMove(t) == MOVE_QUAD) ? 3 : 1) : 0) +
    WEIGHT_MODIFIER(MechRTons(t) / 1024) - MOVE_MODIFIER(MechSpeed(t)) +
    MNumber(m, 0, 1));
TOHITFUNC(bap_tohit, (-1 + MNumber(m, 0, 2)));	/* Very evil */ /* But not anymore - DJ */

TOHITFUNC(radar_tohit, (((FlyingT(t) || OODing(t) || Jumping(t)) && !Landed(t)) ? -1 : 0) + 
		((f & MECHLOSFLAG_PARTIAL) ? ((MechCritStatus(t) & DUG_IN && MechMove(t) == MOVE_QUAD) ? 4 : 2) : 0) +
		(nwood_count(t, f) + nsmoke_count(t, f)) + 
		(((MechStatus2(t) & ECM_PROTECTED) || (f & MECHLOSFLAG_ECM) || (MechStatus(m) & ECM_DISTURBANCE)) ? 1 : 0));
