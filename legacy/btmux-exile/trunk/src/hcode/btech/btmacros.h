/* Descendant of the original macros.h that died of being too bloated :) */

#ifndef BTMACROS_H
#define BTMACROS_H

#include "macros.h"
#include "floatsim.h"


#define LOS_NB InLineOfSight_NB
#define MWalkingSpeed(maxspeed) ((float) 2.0 * (maxspeed) / 3.0 + 0.1)
/* #define WalkingSpeed(maxspeed) ((float) 2.0 * (maxspeed) / 3.0) */
#define WalkingSpeed(maxspeed) ((float) ((float) (maxspeed) / 3) * 2)
#define IsRunning(speed,maxspeed) (speed > MWalkingSpeed(maxspeed))
#define is_aero(mech) ((MechType(mech) == CLASS_AERO) || (IsDS(mech)))
#define IsForest(t) (t == LIGHT_FOREST || t == HEAVY_FOREST)
#define IsForestHex(map,x,y) (IsForest(GetRTerrain(map,x,y)))
#define BaseElev(terr,elev) ((terr) == WATER ? -(elev) : (terr) == ICE ? -(elev) : (elev))
#define Elevation(mech_map,x,y) \
  BaseElev(GetRTerrain(mech_map,x,y),GetElevation(mech_map,x,y))
#define MechElevation(mech) \
  BaseElev(MechRTerrain(mech),MechElev(mech))
/* #define MechEngineSizeC(mech) \
    (((int) ((float) ((float) MechMaxSpeed(mech) * MP_PER_KPH)) * 2 / 3) * MechTons(mech)) */
#define MechEngineSizeC(mech) \
    ((int) ((int) WalkingSpeed(MechMaxSpeed(mech)) / KPH_PER_MP) * (((MechTons(mech) + 4) / 5) * 5))
#define MechOMaxSpeed(mech) ((((float) MechEngineSizeV(mech) / MechTons(mech)) * 1.5) * KPH_PER_MP)
#define MechLowerElevation(mech) \
 ((MechRTerrain(mech) != BRIDGE || MechRTerrain(mech) != DBRIDGE) ? MechElevation(mech) : bridge_w_elevation(mech))
#define MechUpperElevation(mech) (MechRTerrain(mech) == ICE ? 0 : MechElevation(mech))
#define MechsElevation(mech) \
  (MechZ(mech) - ((MechUpperElevation(mech) <= MechZ(mech) ? MechUpperElevation(mech) : MechLowerElevation(mech))))

/* GotPilot checks if mech's pilot is valid and inside his machine */
#define GotPilot(mech) \
     (MechPilot(mech) > 0 && Location(MechPilot(mech)) == mech->mynum)

#define RGotPilot(mech) \
       ((GotPilot(mech)) && (Connected(MechPilot(mech)) || !isPlayer(MechPilot(mech))))

#define GotGPilot(mech) \
     ((pilot_override && GunPilot(mech) > 0) || \
      (!pilot_override && GotPilot(mech)))

#define RGotGPilot(mech) \
     ((pilot_override && GunPilot(mech) > 0 && (Connected(GunPilot(mech)) || \
						!isPlayer(GunPilot(mech)))) \
      || (!pilot_override && RGotPilot(mech)))

#define ChangingHulldown(a)     event_count_type_data(EVENT_DIG,(void *) a)
#define Dugin(a)                MechStatus(a) & DUG_IN
#define AeroBay(a,b)		(a)->pd.bay[b]
#define AeroFuel(a)		(a)->ud.fuel
#define AeroFuelMax(a)		(a)->rd.maxfuel
#define AeroFuelOrig(a)	 	(a)->ud.fuel_orig
#define AeroFreeFuel(a)		0 /* Port from mudinfo. Might include a free-fuel-for-fusion config param someday tho */
#define AeroSI(a)		(a)->ud.si
#define AeroSIOrig(a)	 	(a)->ud.si_orig
#define AeroTurret(a,b)	 	(a)->pd.turret[b]
#define AeroUnusableArcs(a) 	(a)->pd.unusable_arcs
#define DSLastMsg(a)		(a)->rd.last_ds_msg
#define GunPilot(a)		(pilot_override>0?pilot_override:MechPilot(a))
#define MechRadioType(a)	((a)->ud.radioinfo)
#define MechRadioInfo(a)	(MechRadioType(a) / FREQS)
#define MechFreqs(a)		(MechRadioType(a) % FREQS)
#define MFreqs(a)		MechFreqs(a)
#define MechAim(a)		(a)->rd.aim
#define MechAimType(a)	 	(a)->rd.aim_type
#define MechAuto(a)		(a)->rd.autopilot_num
#define MechBTH(a)		(a)->rd.basetohit
#define MechBoomStart(a)	(a)->rd.boom_start
#define MechC3Master(a)	 	(a)->pd.master_c3_node
#define MechCarriedCargo(a) 	(a)->rd.cargo_weight
#define SetCWCheck(a)		MechCritStatus(a) &= ~LOAD_OK
#define SetWCheck(a)		MechCritStatus(a) &= ~OWEIGHT_OK
#define SetCarriedCargo(a,b) 	do { MechCarriedCargo(a) = (b) ; SetCWCheck(a); } while (0)
#define MechCarrying(a)	 	(a)->rd.carrying
#define SetCarrying(a,b)	do { MechCarrying(a) = (b) ; SetCWCheck(a) ; } while (0)
#define MechChargeTarget(a)	(a)->rd.chgtarget
#define MechChargeTimer(a)	(a)->rd.chargetimer
#define MechChargeDistance(a)	(a)->rd.chargedist
#define MechCocoon(a)		(a)->rd.cocoon
#define MechComm(a)		(a)->rd.commconv
#define MechCommLast(a)	 	(a)->rd.commconv_last
#define MechComputer(a)	 	(a)->ud.computer
#define MechCritStatus(a)	(a)->rd.critstatus
#define MechCritStatus2(a)	(a)->rd.critstatus2
#define AeroCritStatus(a)	(is_aero(a) ? (a)->rd.critstatus2 : 0)
#define MechDFATarget(a)	(a)->rd.dfatarget
#define MechDesiredAngle(a) 	(a)->rd.angle
#define MechAngle(a)		(a)->rd.realangle
#define MechVelocity(a)		(a)->rd.velocity
#define MechThrust(a)		(a)->rd.thrust
#define MechDesiredThrust(a)	(a)->rd.desired_speed
#define LastSpinRoll(a)		(a)->rd.last_spinroll
#define MechDesiredFacing(a) 	(a)->rd.desiredfacing
#define MechDesiredSpeed(a) 	(a)->rd.desired_speed
#define MechElev(a)		(a)->pd.elev
#define MechEndFZ(a)		(a)->rd.endfz
#define MechEngineHeat(a)	(a)->rd.engineheat
#define MechFX(a)		(a)->pd.fx
#define MechFY(a)		(a)->pd.fy
#define MechFZ(a)		(a)->pd.fz
#define MechFacing(a)		(FSIM2SHO((a)->pd.facing))
#define MechRFacing(a)	 	(a)->pd.facing
#define SetRFacing(a,b)	 	MechRFacing(a) = (b)
#define SetFacing(a,b)	 	SetRFacing(a,SHO2FSIM(b))
#define AddRFacing(a,b)	 	MechRFacing(a) += b
#define AddFacing(a,b)	 	AddRFacing(a,SHO2FSIM(b))
#define MechFireAdjustment(a) 	(a)->rd.fire_adjustment
#define MechGoingX(a)		(a)->rd.goingx
#define MechGoingY(a)		(a)->rd.goingy
#define MechHeat(a)		(a)->rd.heat
#define MechHeatLast(a)		(a)->rd.heatboom_last
#define MechStartupLast(a)	(a)->rd.startup_last
#define MechBVLast(a)		(a)->rd.bv_last
#define MechHexes(a)		(a)->pd.hexes_walked
#define MechID(a)		(a)->ID
#define MechJumpHeading(a) 	(a)->rd.jumpheading
#define MechJumpLength(a)	(a)->rd.jumplength
#define MechJumpSpeed(a)	(a)->rd.jumpspeed
#define MechJumpTop(a)	 	(a)->rd.jumptop
#define MechLRSRange(a)	 	(a)->ud.lrs_range
#define MechLWRT(a)		(a)->rd.last_weapon_recycle
#define MechLastRndU(a)		(a)->rd.lastrndu
#define MechLastUse(a)	 	(a)->rd.lastused
#define MechLastX(a)		(a)->pd.last_x
#define MechLastY(a)		(a)->pd.last_y
#define MechLateral(a)		(a)->rd.lateral
#define MechMASCCounter(a) 	(a)->rd.masc_value
#define MechSChargeCounter(a)	(a)->rd.scharge_value
#define MechEngineSizeV(a) 	(a)->rd.erat
#define MechEngineSize(a)	(MechEngineSizeV(a) > 0 ? MechEngineSizeV(a) : MechEngineSizeC(a))
#define MechMaxSpeed(a)	 	(a)->ud.maxspeed
#define SetMaxSpeed(a,b) 	do {MechMaxSpeed(a) = b;MechCritStatus(a) &= ~SPEED_OK;correct_speed(a);} while (0)
#define LowerMaxSpeed(a,b) 	SetMaxSpeed(a,MechMaxSpeed(a)-b)
#define DivideMaxSpeed(a,b) 	SetMaxSpeed(a,MechMaxSpeed(a)/b)
#define MechRMaxSpeed(a)	(a)->rd.rspd
#define MMaxSpeed(a)		((float) MechCargoMaxSpeed((a),(float) MechMaxSpeed((a))))
#define MechMinusHeat(a)	(a)->rd.minus_heat
#define MechMove(a)		(a)->ud.move
#define MechIsQuad(a)		(MechMove(a) == MOVE_QUAD)
#define MechIsBiped(a)          (MechMove(a) == MOVE_BIPED)
#define MechNumOsinks(a)	(a)->rd.onumsinks
#define MechNumSeen(a)	 	(a)->rd.num_seen
#define MechPNumSeen(a)		(a)->rd.can_see
#define MechRealNumsinks(a)	(a)->ud.numsinks
#define MechActiveNumsinks(a)	(MechRealNumsinks(a) - MechDisabledHS(a))
#define MechDisabledHS(a)	(a)->rd.disabled_hs
#define MechPer(a)		(a)->rd.per
#define MechPrefs(a)		(a)->rd.mech_prefs
#define MechTargComp(a)		(a)->ud.targcomp
#define MechOT(a)		(MechPrefs(a) & MECHPREF_OT)
#define SetMechOT(a)		(MechPrefs(a) |= MECHPREF_OT)
#define UnSetMechOT(a)		(MechPrefs(a) &= ~MECHPREF_OT)
#define MechCruise(a)		(MechPrefs(a) & MECHPREF_CRUISE)
#define SetMechCruise(a)	(MechPrefs(a) |= MECHPREF_CRUISE)
#define UnSetMechCruise(a)	(MechPrefs(a) &= ~MECHPREF_CRUISE)
#define MechPKiller(a)		(MechPrefs(a) & MECHPREF_PKILL)
#define SetMechPKiller(a)	(MechPrefs(a) |= MECHPREF_PKILL)
#define UnSetMechPKiller(a)	(MechPrefs(a) &= ~MECHPREF_PKILL)
#define MechSLWarn(a)		(MechPrefs(a) & MECHPREF_SLWARN)
#define SetMechSLWarn(a)	(MechPrefs(a) |= MECHPREF_SLWARN)
#define UnSetMechSLWarn(a)	(MechPrefs(a) &= ~MECHPREF_SLWARN)
#define MechNoRadio(a)		(MechPrefs(a) & MECHPREF_NORADIO)
#define SetMechNoRadio(a)	(MechPrefs(a) |= MECHPREF_NORADIO)
#define UnSetMechNoRadio(a)	(MechPrefs(a) &= ~MECHPREF_NORADIO) 
#define MechAutofall(a)         (MechPrefs(a) & MECHPREF_AUTOFALL)
#define SetMechAutofall(a)      (MechPrefs(a) |= MECHPREF_AUTOFALL)
#define UnSetMechAutofall(a)    (MechPrefs(a) &= ~MECHPREF_AUTOFALL)
#define MechWalkXPFactor(a) 	(a)->rd.wxf
#define MechPilot(a)		(a)->pd.pilot
#define MechPilotSkillBase(a) 	(a)->rd.pilotskillbase
#define MechPilotStatus(a) 	(a)->pd.pilotstatus
#define MechPlusHeat(a)	 	(a)->rd.plus_heat
#define MechRadio(a)		(a)->ud.radio
#define MechRadioRange(a) 	(a)->ud.radio_range
#define MechRnd(a)		(a)->rd.rnd
#define MechScanRange(a)	(a)->ud.scan_range
#define MechSections(a)	 	(a)->ud.sections
#define MechSensor(a)		(a)->rd.sensor
#define MechSpecials(a)	 	(a)->rd.specials
#define MechSpecials2(a)	(a)->rd.specials2
#define MechSpeed(a)		(a)->rd.speed
#define MechSpotter(a)	 	(a)->rd.spotter
#define MechStall(a)		(a)->pd.stall
#define MechStartFX(a)	 	(a)->rd.startfx
#define MechStartFY(a)	 	(a)->rd.startfy
#define MechStartFZ(a)	 	(a)->rd.startfz
/* Legacy
#define MechStartSpin(a)	(a)->rd.sspin
*/
#define MechStatus(a)		(a)->rd.status
#define MechStatus2(a) 		(a)->rd.status2
#define MechSwarmTarget(a) 	(a)->rd.swarming
#define MechTacRange(a)	 	(a)->ud.tac_range
#define MechTargX(a)		(a)->rd.targx
#define MechTargY(a)		(a)->rd.targy
#define MechTargZ(a)		(a)->rd.targz
#define MechTarget(a)		(a)->rd.target
#define MechTeam(a)		(a)->pd.team
#define MechTerrain(a)	 	(a)->pd.terrain
#define MechRTerrain(a) 	((MechTerrain(a) == FIRE || MechTerrain(a) == SMOKE || MechTerrain(a) == HSMOKE) ? mech_underlying_terrain(a) : MechTerrain(a))
#define MechTons(a)		(a)->ud.tons
#define MechRTons(a)		get_weight(a)
#define MechRTonsV(a)	 	(a)->rd.row
#define MechRCTonsV(a)	 	(a)->rd.rcw
#define MechTurnDamage(a) 	(a)->rd.turndamage
#if 0
#define MechTurnPercent(a)	(a)->rd.turnperc;
#define MechTurnTrack(a)	(a)->rd.turntrack;
#endif
#define MechTurretFacing(a) 	(a)->rd.turretfacing
#define MechType(a)		(a)->ud.type
#define MechType_Name(a)	(a)->ud.mech_name
#define MechType_Ref(a)	 	(a)->ud.mech_type
#define MechVFacing(a)	 	AcceptableDegree(MechFacing(a) + MechLateral(a))
#define MechVerticalSpeed(a) 	(a)->rd.verticalspeed
#define MechVisMod(a)	 	(a)->rd.vis_mod
#define MechWeapHeat(a)	 	(a)->rd.weapheat
#define MechX(a)		(a)->pd.x
#define MechY(a)		(a)->pd.y
#define MechZ(a)		(a)->pd.z
#define MechLX(a)		(a)->rd.lx
#define MechLY(a)		(a)->rd.ly

#define StealthAndECM(a)	((MechStatus(a) & ECM_ACTIVE) && (MechStatus2(a) & STEALTHARM_ACTIVE) \
    && !(MechCritStatus(a) & ECM_DESTROYED) && !(MechCritStatus2(a) & STEALTHARM_DESTROYED))

#define ECMRange(a) (MechSpecials2(a) & PERSONAL_ECM ? 1 : ECM_RANGE)

#define MechBV(a)		(a)->ud.mechbv
#define CargoSpace(a)		(a)->ud.cargospace
#define PodSpace(a)		(a)->ud.podspace
#define CarMaxTon(a)		(a)->ud.carmaxton
#define Heatcutoff(a)		(MechCritStatus(mech) & HEATCUTOFF)

#define DSSpam(mek,msg)		do { if (DropShip(MechType(mek)) && DSOkToNotify(mek)) MechLOSBroadcast(mek,msg); } while (0)
#define DSSpam_O(mek,msg)	do { if (DropShip(MechType(mek))) MechLOSBroadcast(mek,msg); } while (0)

#define MechHasTurret(a)        ((MechType(a) == CLASS_VEH_GROUND || \
                                  MechType(a) == CLASS_VEH_NAVAL || \
                                  MechType(a) == CLASS_VTOL) && \
                                 GetSectOInt(a, TURRET))

#define MechSeemsFriend(a, b)   (MechTeam(a) == MechTeam(b) && \
                                 InLineOfSight_NB(a, b, 0, 0, 0)) 

#define SetTurnMode(a,b) do { if (b) MechPrefs(a) |= MECHPREF_TURNMODE; else MechPrefs(a) &= ~MECHPREF_TURNMODE; } while (0)
#define GetTurnMode(a)	(MechPrefs(a) & MECHPREF_TURNMODE)

#define MECHEVENT(mech,type,func,time,data) \
  do { if (mech->mynum > 0) \
     event_add(time, 0, type, func, (void *) (mech), (void *) (data)); } while (0)

#define AUTOEVENT(auto,type,func,time,data) \
  event_add(time, 0, type, func, (void *) (auto), (void *) (data))

#define MAPEVENT(map,type,func,time,data) \
  event_add(time, 0, type, func, (void *) (map), (void *) (data))
#define StopDec(a)    event_remove_type_data2(EVENT_DECORATION, (void *) a)

#define OBJEVENT(obj,type,func,time,data) \
  event_add(time, 0, type, func, (void *) obj, (void *) (data))

#define GetPartType(a,b,c)   MechSections(a)[b].criticals[c].type
#define SetPartType(a,b,c,d) GetPartType(a,b,c)=d

#define GetPartDamage(a,b,c)   MechSections(a)[b].criticals[c].damagemode
#define SetPartDamage(a,b,c,d) GetPartDamage(a,b,c)=d

#define GetPartMode(a,b,c)   MechSections(a)[b].criticals[c].mode
#define SetPartMode(a,b,c,d) GetPartMode(a,b,c)=d

#define GetPartData(a,b,c)   MechSections(a)[b].criticals[c].data
#define SetPartData(a,b,c,d) GetPartData(a,b,c)=d

#define PartIsNonfunctional(a,b,c)  \
	(PartIsDisabled(a,b,c) || PartIsDestroyed(a,b,c))
#define PartIsDisabled(a,b,c)  (GetPartMode(a,b,c) & DISABLED_MODE)
#define DisablePart(a,b,c)     (GetPartMode(a,b,c) |= DISABLED_MODE)
#define UnDisablePart(a,b,c)   \
	do { GetPartMode(a,b,c) &= ~DISABLED_MODE ; } while (0)
#define PartIsDestroyed(a,b,c) (GetPartMode(a,b,c) & DESTROYED_MODE)
#define DestroyPart(a,b,c)     do { (GetPartMode(a,b,c) |= DESTROYED_MODE); UnDisablePart(a,b,c); } while (0)
#define UnDestroyPart(a,b,c)   \
	do { GetPartMode(a,b,c) &= ~(DESTROYED_MODE|HOTLOAD_MODE|DISABLED_MODE) ; SetPartDamage(a,b,c,0); } while (0)

#define WpnIsRecycling(a,b,c)  (GetPartData(a,b,c) > 0 && \
				IsWeapon(GetPartType(a,b,c)) && \
				!PartIsNonfunctional(a,b,c) && \
				!SectIsDestroyed(a,b))
#define SectArmorRepair(a,b)  SomeoneFixingA(a,b)
#define SectRArmorRepair(a,b) SomeoneFixingA(a,b+8)
#define SectIntsRepair(a,b)   SomeoneFixingI(a,b)

#define SectIsDestroyed(a,b)  (!GetSectArmor(a,b) && !GetSectInt(a,b) && !is_aero(a))
#define SetSectDestroyed(a,b)
#define UnSetSectDestroyed(a,b)
#define SectIsBreached(a,b)  ((a)->ud.sections[b].config & SECTION_BREACHED)
#define SetSectBreached(a,b) \
do { MechSections(a)[b].config |= SECTION_BREACHED ; SetWCheck(a); } while (0)
#define UnSetSectBreached(a,b) \
do { MechSections(a)[b].config &= ~SECTION_BREACHED ; SetWCheck(a); } while (0)

/*
 * Added 8/4/99 by Kipsta for new flooding code
 */

#define SectIsFlooded(a,b)  ((a)->ud.sections[b].config & SECTION_FLOODED)
#define SetSectFlooded(a,b) do { MechSections(a)[b].config |= SECTION_FLOODED ; SetWCheck(a); } while (0)
#define UnSetSectFlooded(a,b) do { MechSections(a)[b].config &= ~SECTION_FLOODED ; SetWCheck(a); } while (0)

#define GetSectArmor(a,b)    ((a)->ud.sections[b].armor)
#define GetSectRArmor(a,b)   ((a)->ud.sections[b].rear)
#define GetSectInt(a,b)      (is_aero(a) ? (b < NumSections(MechType(a)) ? AeroSI(a) : 0) : ((a)->ud.sections[b].internal))

#define SetSectArmor(a,b,c)  do { (a)->ud.sections[b].armor=c;SetWCheck(a); } while (0)
#define SetSectRArmor(a,b,c) do { (a)->ud.sections[b].rear=c;SetWCheck(a); } while (0)
#define SetSectInt(a,b,c)    do { (a)->ud.sections[b].internal=c;SetWCheck(a); } while (0)

#define GetSectOArmor(a,b)   (a)->ud.sections[b].armor_orig
#define GetSectORArmor(a,b)  (a)->ud.sections[b].rear_orig
#define GetSectOInt(a,b)     (is_aero(a) ? (b < NumSections(MechType(a)) ? AeroSIOrig(a) : 0) : (a)->ud.sections[b].internal_orig)

#define SetSectOArmor(a,b,c) (a)->ud.sections[b].armor_orig=c
#define SetSectORArmor(a,b,c) (a)->ud.sections[b].rear_orig=c
#define SetSectOInt(a,b,c)   (a)->ud.sections[b].internal_orig=c

#define CanJump(a) (!(Stabilizing(a)) && !(Jumping(a)))

/* #define Jumping(a)           event_count_type_data(EVENT_JUMP,(void *) a) */
#define MoveModeChange(a)	event_count_type_data(EVENT_MOVEMODE,(void *) a)
#define MoveModeLock(a)		(MechStatus2(a) & MOVE_MODES_LOCK || (MoveModeChange(a) && !(MechStatus2(a) & DODGING)))
#define MoveModeData(a)	     event_count_type_data_firstev(EVENT_MOVEMODE, (void *) a)
#define DodgeOff(a)		MECHEVENT(a,EVENT_MOVEMODE, mech_movemode_event, 1, (MODE_DODGE|MODE_OFF|MODE_DG_USED)); 
#define MiscEventing(a)		event_count_type_data(EVENT_MISC,(void *) a)
#define Exploding(a)         event_count_type_data(EVENT_EXPLODE, (void *) a)
#define Dumping(a)           event_count_type_data(EVENT_DUMP, (void *) a)
#define SideSlipping(a)	     event_count_type_data(EVENT_SIDESLIP, (void *) a)
/* New search for data type used.  This to get an events (first) data for Ammo checks */
#define DumpData(a)	     event_count_type_data_firstev(EVENT_DUMP, (void *) a)
#define Dumping_Type(a,type) (event_count_type_data_data(EVENT_DUMP, (void *) a, (void *) type) || event_count_type_data_data(EVENT_DUMP, (void *) a, (void *) 0))
#define ChangingLateral(a)   event_count_type_data(EVENT_LATERAL,(void *) a)
#define Seeing(a)            event_count_type_data(EVENT_PLOS,(void *) a)
#define NullSigChanging(a)   event_count_type_data(EVENT_NULLSIG,(void *) a)
#define StealthArmChanging(a) event_count_type_data(EVENT_STEALTHARM,(void *) a)
#define Locking(a)           event_count_type_data(EVENT_LOCK,(void *) a)
#define Hiding(a)            event_count_type_data(EVENT_HIDE,(void *) a)
#define Digging(a)           MechCritStatus(a) & DIGGING_IN
#define CrewStunning(a)	     event_count_type_data(EVENT_CREWSTUN,(void *) a)
#define Moving(a)            event_count_type_data(EVENT_MOVE,(void *) a)
#define SensorChange(a)      event_count_type_data(EVENT_SCHANGE,(void *) a)
#define Stabilizing(a)       event_count_type_data(EVENT_JUMPSTABIL,(void *) a)
#define Standrecovering(a)   event_count_type_data(EVENT_STANDFAIL, (void *) a)
#define Standing(a)          event_count_type_data(EVENT_STAND,(void *) a)
#define Starting(a)          event_count_type_data(EVENT_STARTUP,(void *) a)
#define Recovering(a)        event_count_type_data(EVENT_RECOVERY,(void *) a)
#define TakingOff(a)         event_count_type_data(EVENT_TAKEOFF,(void *) a)
#define Landing(a)	     event_count_type_data(EVENT_LANDING,(void *) a)
#define NextRecycle(a)       event_first_type_data(EVENT_RECYCLE,(void *) a)
#define Recycling(a)         (NextRecycle(a) >= 0 ? 1 : 0)
#define FlyingT(a)           (is_aero(a) || MechMove(a) == MOVE_VTOL)
#define RollingT(a)          ((MechType(a) == CLASS_AERO) || (MechType(a) == CLASS_DS))
#define MaybeMove(a) \
do { if (!Moving(a) && Started(a) && (!Fallen(mech) || MechType(a) == CLASS_MECH)) \
	MECHEVENT(a,EVENT_MOVE,is_aero(a) ? aero_move_event : mech_move_event,\
	MOVE_TICK,0); } while (0)
#define SetRecyclePart(a,b,c,d) \
do { MaybeRecycle(a,d) ; SetPartData(a,b,c,d); } while (0)
#define SetRecycleLimb(a,b,c) \
do { MaybeRecycle(a,c) ; (a)->ud.sections[b].recycle=c; } while (0)
#define UpdateRecycling(a) \
do { if (Started(a) && !Destroyed(a) && a->rd.last_weapon_recycle != event_tick) \
    recycle_weaponry(a); } while (0)
#define StopSideslip(a)	    event_remove_type_data(EVENT_SIDESLIP, (void *) a)
#define StopMoveMode(a)	    event_remove_type_data(EVENT_MOVEMODE, (void *) a)
#define StopExploding(a)    event_remove_type_data(EVENT_EXPLODE, (void *) a)
#define StopLateral(a)      event_remove_type_data(EVENT_LATERAL,(void *) a)
#define StopMasc(a)         event_remove_type_data(EVENT_MASC_FAIL,(void *) a)
#define StopSCharge(a)	    event_remove_type_data(EVENT_SCHARGE_FAIL,(void *) a)
#define StopMascR(a)        event_remove_type_data(EVENT_MASC_REGEN,(void *) a)
#define StopSChargeR(a)	    event_remove_type_data(EVENT_SCHARGE_REGEN,(void *) a)
#define StopDump(a)          event_remove_type_data(EVENT_DUMP, (void *) a)
#define StopJump(a)          event_remove_type_data(EVENT_JUMP, (void *) a)
#define StopOOD(a)          event_remove_type_data(EVENT_OOD, (void *) a)
#define StopMoving(a)        event_remove_type_data(EVENT_MOVE, (void *) a)
#define StopStand(a)         event_remove_type_data(EVENT_STAND, (void *) a)
#define StopStabilization(a) event_remove_type_data(EVENT_JUMPSTABIL, (void *) a)
#define StopMiscEvent(a)	event_remove_type_data(EVENT_MISC, (void *) a)
#define StopSensorChange(a)  event_remove_type_data(EVENT_SCHANGE,(void *) a)
#define StopStartup(a)       event_remove_type_data(EVENT_STARTUP, (void *) a)
#define StopHiding(a)          event_remove_type_data(EVENT_HIDE, (void *) a)
#define StopDigging(a)          event_remove_type_data(EVENT_DIG, (void *) a);MechCritStatus(a) &= ~DIGGING_IN
#define StopUnjamming(a)	event_remove_type_data(EVENT_UNJAM, (void *) a);MechStatus2(a) &= ~UNJAMMING
#define StopCrewStunning(a)	event_remove_type_data(EVENT_CREWSTUN, (void *) a)
#define StopTakeOff(a)       event_remove_type_data(EVENT_TAKEOFF, (void *) a)
#define OODing(a)            MechCocoon(a)
#define C_OODing(a)          MechCocoon(a) > 0
#define InSpecial(a)         (MechStatus(a) & UNDERSPECIAL)
#define InGravity(a)         (MechStatus(a) & UNDERGRAVITY)
#define InVacuum(a)          (MechStatus(a) & UNDERVACUUM)
#define Running(a)	     (IsRunning(MechSpeed(a), MMaxSpeed(a)))
#define Jumping(a)           (MechStatus(a) & JUMPING)
#define Started(a)           (MechStatus(a) & STARTED)
#define HiddenMech(a)	     (MechCritStatus(a) & HIDDEN)
#define Destroyed(a)         (MechStatus(a) & DESTROYED)
#define Fallen(a)            (MechStatus(a) & FALLEN)
#define Landed(a)            (MechStatus(a) & LANDED)
#define Towed(a)             (MechStatus(a) & TOWED)
#define MakeMechFall(a)      MechStatus(a) |= FALLEN;FallCentersTorso(a);MarkForLOSUpdate(a);MechFloods(a);StopStand(a)
#define FallCentersTorso(a)  MechStatus(a) &= ~(TORSO_RIGHT|TORSO_LEFT|FLIPPED_ARMS)
#define MakeMechStand(a)     MechStatus(a) &= ~FALLEN;MarkForLOSUpdate(a)
#define StandMechTime(a)     (30 / (MechMaxSpeed(a)/MP2))
#define StopLock(a)          event_remove_type_data(EVENT_LOCK, (void *) a);\
MechStatus(a) &= ~LOCK_MODES
#define SearchlightChanging(a)	event_count_type_data(EVENT_SLITECHANGING, (void *) a)	// Added by Kipsta. July 26, 1999
#define HeatcutoffChanging(a)	event_count_type_data(EVENT_HEATCUTOFFCHANGING, (void *) a)

#define LoseLock(a)         StopLock(a);MechTarget(a)=-1;MechTargX(a)=-1;MechTargY(a)=-1;if (MechAim(a) != NUM_SECTIONS) { mech_notify(a, MECHALL, "Location-specific targeting powers down."); MechAim(a) = NUM_SECTIONS; }
#ifdef ADVANCED_LOS
#define StartSeeing(a) \
MECHEVENT(a,EVENT_PLOS,mech_plos_event,INITIAL_PLOS_TICK,0)
#else
#define StartSeeing(a)
#endif

#define Startup(a)           \
    do { MechStatus(a) |= STARTED;MechTurnDamage(a) = 0;MaybeRecycle(a,1); \
    MechNumSeen(a)=0; StartSeeing(a); } while (0)

#define AIDestCheck(m) \
    if (MechAuto(m) && FindObjectsData(MechAuto(m))) { StopAutoPilot(FindObjectsData(MechAuto(m))); \
        SendAI(tprintf("Stopped AUTO events on #%d (Mech #%d)", MechAuto(m), m->mynum)); MechAuto(m) = -1; } else { \
        MechAuto(m) = -1; }

#define Shutdown(a)          \
    do { if (!Destroyed(a)) { UpdateRecycling(a);MechStatus(a) &= ~(STARTED|ECM_ACTIVE|SLITE_ON|DUG_IN); \
    MechStatus2(a) &=~ (SPRINTING|DODGING|EVADING|ECCM_ACTIVE|STEALTHARM_ACTIVE|NULLSIG_ACTIVE|UNJAMMING); StopMoveMode(a); MechDesiredFacing(a) = MechFacing(a); \
    MechSpeed(a) = 0.0; MechVelocity(a) = 0.0; MechCritStatus(a) &= ~(HEATCUTOFF); MechDisabledHS(a) = 0; MechDesiredSpeed(a) = 0.0; } ; MechPilot(a) = -1 ; \
    MechTarget(a) = -1; StopStartup(a) ; StopJump(a) ; StopMoving(a) ; StopHiding(a); StopDump(a);\
    StopStand(a) ; StopStabilization(a); StopTakeOff(a); StopHiding(a); StopDigging(a) ; \
    MechChargeTarget(a) = -1; MechSwarmTarget(a) = -1; } while (0)

#define Destroy(a)           \
    do { if (Uncon(a)) \
    {  MechStatus(a) &= ~(BLINDED|UNCONSCIOUS); \
      mech_notify(a, MECHALL, "The mech was destroyed while pilot was unconscious!"); \
    } \
    Shutdown(a) ; MechStatus(a) |= DESTROYED; MechStatus(a) &= ~(JELLIED|SLITE_ON); MechCritStatus(a) &= ~CREW_STUNNED; \
    bsuit_stopswarmers(FindObjectsData(a->mapindex),a,1); \
    event_remove_data((void *) a); \
  if ((MechType(a) == CLASS_MECH && Jumping(a)) || \
      (MechType(a) != CLASS_MECH && MechZ(a) > MechUpperElevation(a) && MechZ(a) < ORBIT_Z) || \
      (MechMove(a) == MOVE_HOVER && MechRTerrain(a) == WATER))\
      MECHEVENT(a, EVENT_FALL, mech_fall_event, FALL_TICK, -1); \
      AIDestCheck(a) \
    } while (0)
#define DestroyAndDump(a)           \
    do { Destroy(a); MechVerticalSpeed(a) = 0.0; \
    if (MechRTerrain(a) == WATER || MechRTerrain(a) == ICE) \
      MechZ(a) = -MechElev(a); \
    else \
       if ((MechRTerrain(a) == BRIDGE || MechRTerrain(a) == DBRIDGE) && MechZ(a) >= MechUpperElevation(a)) MechZ(a) = MechUpperElevation(a) ; else \
     MechZ(a) = MechElev(a); \
      MechFZ(a) = ZSCALE * MechZ(a); } while (0)

#define TowDump(a) \
    do { MechVerticalSpeed(a) = 0.0; \
    if (MechRTerrain(a) == WATER) MechZ(a) = -MechElev(a); \
    else if ((MechRTerrain(a) == BRIDGE || MechRTerrain(a) == DBRIDGE) && MechZ(a) >= MechUpperElevation(a)) MechZ(a) = MechUpperElevation(a) ; else \
    MechZ(a) = MechElev(a); \
    MechFZ(a) = ZSCALE * MechZ(a); } while (0)
    

#define GetTerrain(mapn,x,y)     Coding_GetTerrain(mapn->map[y][x])
#define GetRTerrain(map,x,y)      ((GetTerrain(map,x,y)==FIRE || GetTerrain(map,x,y)==SMOKE || GetTerrain(map,x,y)==HSMOKE) ? \
					 map_underlying_terrain(map,x,y) : GetTerrain(map,x,y))
#define GetElevation(mapn,x,y)   Coding_GetElevation(mapn->map[y][x])
#define GetElev(mapn,x,y)        GetElevation(mapn,x,y)
#define SetMap(mapn,x,y,t,e)     mapn->map[y][x] = Coding_GetIndex(t,e)
#define SetMapB(mapn,x,y,t,e)    mapn[y][x] = Coding_GetIndex(t,e)
#define SetTerrain(mapn,x,y,t)   do {SetMap(mapn,x,y,t,GetElevation(mapn,x,y));UpdateMechsTerrain(mapn,x,y,t); } while (0)
#define SetTerrainBase(mapn,x,y,t) SetMap(mapn,x,y,t,GetElevation(mapn,x,y))
#define SetElevation(mapn,x,y,e) SetMap(mapn,x,y,GetTerrain(mapn,x,y),e)

/* For now I don't care about allocations */
#define ScenError(msg...)        send_channel("ScenErrors",tprintf(msg))
#define ScenStatus(msg...)       send_channel("ScenStatus",tprintf(msg))
#define SendAI(msg...)           send_channel("MechAI",tprintf(msg))
#define SendAlloc(msg)
#define SendLoc(msg)
#define SendCustom(msg)          send_channel("MechCustom",msg)
#define SendDB(msg)              send_channel("DBInfo",msg)
#define SendDebug(msg)           send_channel("MechDebugInfo",msg)
#define SendEcon(msg)            send_channel("MechEconInfo",msg)
#define SendError(msg)           send_channel("MechErrors",msg)
#define SendEvent(msg)           send_channel("EventInfo",msg)
#define SendSensor(msg)          send_channel("MechSensor",msg)
#define SendTrigger(msg)         send_channel("MineTriggers",msg)
#define SendXP(msg)              send_channel("MechXP",msg)
#define SendAttackXP(msg)	 send_channel("MechAttackXP",msg)
#define SendPilotXP(msg)	 send_channel("MechPilotXP",msg)
#define SendTechXP(msg)		 send_channel("MechTechXP", msg)
#define SendAttacks(msg)	 send_channel("MechAttacks", msg)
#define SendBTHDebug(msg)	 send_channel("MechBTHDebug", msg)
#define SendFreqs(msg)		 send_channel("MechFreqs",msg)
#define SendAttackEmits(msg)	 send_channel("MechAttackEmits",msg)

/*
 * This is the prototype for functions
 */

#ifdef TEMPLATE_VERBOSE_ERRORS

#define TEMPLATE_ERR(a,b...) \
if (a) { \
notify(player, tprintf(b)); \
if (fp) fclose(fp); return -1; }

#define TEMPLATE_GERR(a,b...) \
if (a) { \
char foobarbuf[512]; \
sprintf(foobarbuf, b); \
SendError(foobarbuf); \
if (fp) fclose(fp); return -1; }
#else

#define TEMPLATE_ERR(a,b...) \
if (a) { \
if (fp) fclose(fp); return -1; }
#define TEMPLATE_GERR TEMPLATE_ERR

#endif

#define HotLoading(weapindx,mode) \
((mode & HOTLOAD_MODE) && (MechWeapons[weapindx].type == TMISSILE ? \
   MechWeapons[weapindx].special & IDF : 1) && !(MechWeapons[weapindx].special & DAR))

#define MirrorPosition(from,to) \
do {	  MechFX(to) = MechFX(from); \
	  MechFY(to) = MechFY(from); \
	  MechFZ(to) = MechFZ(from); \
	  MechX(to) = MechX(from); \
	  MechY(to) = MechY(from); \
	  MechZ(to) = MechZ(from); \
          MechLastX(to) = MechLastX(from); \
          MechLastY(to) = MechLastY(from); \
	  MechTerrain(to) = MechTerrain(from); \
	  MechElev(to) = MechElev(from); MarkForLOSUpdate(to); } while (0)


#define FindPunchLoc(mech,hitloc,arc,iscritical,isrear) \
 do { if (MechType(mech) != CLASS_MECH) \
   hitloc = FindHitLocation(mech, arc, &iscritical, &isrear, NULL); \
 else \
	hitloc = FindPunchLocation(arc, mech); } while (0)

#define FindKickLoc(mech,hitloc,arc,iscritical,isrear) \
 do { if (MechType(mech) != CLASS_MECH) \
   hitloc = FindHitLocation(mech, arc, &iscritical, &isrear, NULL); \
 else \
   hitloc = FindKickLocation(arc, mech); } while (0)



#define ValidCoordA(mech_map,newx,newy,msg) \
DOCHECK(newx < 0 || newx >= mech_map->map_width || \
	newy < 0 || newy >= mech_map->map_height, \
	msg)
#define ValidCoord(mech_map,newx,newy) \
        ValidCoordA(mech_map,newx, newy, "Illegal coordinates!")
#define FlMechRange(map,m1,m2) \
        FaMechRange(m1,m2)

#define Readnum(tovar,fromvar) \
        (!(tovar = atoi(fromvar)) && strcmp(fromvar, "0"))

#define SetBit(val,bit)   (val |= bit)
#define UnSetBit(val,bit) (val &= ~(bit))
#define EvalBit(val,bit,state) \
        do {if (state) SetBit(val,bit); else UnSetBit(val,bit);} while (0)
#define ToggleBit(val,bit) \
do { if (!(val & bit)) SetBit(val,bit);else UnSetBit(val,bit); } while (0)
#define Sees360(mech) (MechMove(mech)==MOVE_NONE||MechType(mech)==CLASS_MW||MechType(mech)==CLASS_MECH||is_aero(mech))
#define FindWeapons(m,i,wa,wda,cr) FindWeapons_Advanced(m,i,wa,wda,cr,0)

#define ContinueFlying(mech) \
if (FlyingT(mech)) { \
  MechStatus(mech) &= ~LANDED; \
  MechZ(mech) += 1; \
  MechFZ(mech) = ZSCALE * MechZ(mech); \
  StopMoving(mech); }

#define Overwater(mech) \
(MechMove(mech) == MOVE_HOVER || MechType(mech) == CLASS_MW || \
 MechMove(mech) == MOVE_FOIL)

#define MoveMod(mech) \
(MechType(mech) == CLASS_MW ? 3 : \
 (MechMove(mech) == MOVE_BIPED || MechMove(mech) == MOVE_QUAD || \
   MechMove(mech) == MOVE_TRACK) ? 2 : 1)

#define IsWater(t)    ((t) == ICE  || (t) == WATER || (t) == BRIDGE || (t) == DBRIDGE)
#define InWater(mech) (IsWater(MechRTerrain((mech))) && MechZ(mech)<0)
#define OnWater(mech) (IsWater(MechRTerrain((mech))) && MechZ(mech)<=0)

#define IsC3(mech) \
((MechSpecials(mech) & (C3_MASTER_TECH|C3_SLAVE_TECH)) && !C3Destroyed(mech))
#define IsC3i(mech) \
((MechSpecials(mech) & (C3_MASTER_TECH)) && \
 (MechSpecials(mech) & (C3_SLAVE_TECH)) && \
 !C3Destroyed(mech))

#define IsAMS(weapindx) \
(MechWeapons[weapindx].special & AMS)

/* Macro for figuring out the truly ugly stuff - whether ammo
   crit is in fact 1 or 2 'half-tons' of ammo */
#define AmmoMod(mech,loc,pos) \
  ((!IsAmmo(GetPartType(mech,loc,pos)) || \
    GetPartMode(mech,loc,pos) & HALFTON_MODE || GetPartMode(mech,loc,pos) & (LARGE_AMMO)) ? 1 : 2)

/* #define FullAmmo(mech,loc,pos) \ */

/*   ( \ */

/*    ( \ */

/*     ((MechWeapons[Ammo2I(GetPartType(mech,loc,pos))].special & NARC) && \ */

/*      (GetPartMode(mech,loc,pos) & NARC_MODE))  ? 6 : \ */

/*      MechWeapons[Ammo2I(GetPartType(mech,loc,pos))].ammoperton \ */

/*     ) \ */

/*    / \ */

/*    (3 - AmmoMod(mech, loc, pos))) */

#define FullAmmo(mech,loc,pos) \
  ((GetPartMode(mech,loc,pos) & OS_MODE) ? 1 : \
    (MechType(mech) == CLASS_BSUIT && MechWeapons[Ammo2I(GetPartType(mech,loc,pos))].class != WCLASS_PC) ? \
      ((MechWeapons[Ammo2I(GetPartType(mech,loc,pos))].type == TAMMO) ? 25 : 1) : \
  ((MechWeapons[Ammo2I(GetPartType(mech,loc,pos))].ammoperton / (3 - AmmoMod(mech, loc, pos))) *  \
  ((GetPartMode(mech, loc, pos) & (SMALL_AMMO)) ? 2 : 1)))

#define JumpSpeed(mech,map) \
  ((InGravity(mech) && map) ? (MechJumpSpeed(mech) * 100 / ((MAX(50, MapGravity(map))))) : MechJumpSpeed(mech))
#define JumpSpeedMP(mech,map) \
  ((int) (JumpSpeed(mech,map) * MP_PER_KPH))

#define NotInWater(mech) (!(OnWater(mech)))
#define WaterBeast(mech) (MechMove(mech)==MOVE_HULL || MechMove(mech)==MOVE_FOIL)

#define IsCrap(val) \
  (((val) == Special(ENDO_STEEL)) || ((val) == Special(FERRO_FIBROUS)) \
  || ((val) == Special(HARDPOINT)) || ((val) == Special(STEALTHARM)))

#define Spinning(mech)	event_count_type_data(EVENT_SPINNING, (void *) mech)
#define StopSpinning(mech) event_remove_type_data(EVENT_SPINNING, (void *) mech)
#define StartSpinning(mech) MECHEVENT(mech, EVENT_SPINNING, aero_spinning_event, 30, 0)

/*
#define Spinning(mech)      (MechCritStatus(mech) & SPINNING)
#define StopSpinning(mech)  (MechCritStatus(mech) &= ~SPINNING)
#define StartSpinning(mech) (MechCritStatus(mech) |= SPINNING)
*/

#define IsLit2(mech) (MechCritStatus(mech) & SLITE_LIT)
#define IsLit3(mech) (MechStatus(mech) & SLITE_ON)
#define IsLit(mech) (IsLit2(mech) || IsLit3(mech))

#define OkayCritSect(sect,num,ok) \
  (GetPartType(mech,sect,num)==(ok) && !PartIsNonfunctional(mech,sect,num))
#define OkayCritSectS(sect,num,ok) OkayCritSect(sect,num,I2Special(ok))
#define MAPMOVEMOD(map) ((map)->movemod > 0 ? (float) (map)->movemod / 100.0 : 1.0)

#define RCache_Remove(n)
#define RCache_Flush()

/* Ancient remnant ; of pre-06061998 rangecache-code */
#define FaMechRange(mech,target) \
    FindRange(MechFX(mech), MechFY(mech), MechFZ(mech), \
	      MechFX(target), MechFY(target), MechFZ(target))

#define DSBearMod(ds) \
  ((MechFacing(ds) +30) / 60) % 6

#define MechCalcAngle(mech) \
    ((int) (MechVerticalSpeed(mech) == 0 ? 0 : \
	(MechVerticalSpeed(mech) < 0 ? -90 : 90) - \
	((fatan((sqrt((MechSpeed(mech) * abs(MechSpeed(mech))) + (0 * 0)) / MechVerticalSpeed(mech))) * 180) / PI)))


#endif				/* BTMACROS_H */
