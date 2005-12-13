
/*
 * $Id: weapons.h,v 1.1.1.1 2005/01/11 21:18:33 kstevens Exp $
 *
 * Last modified: $Date: 2005/01/11 21:18:33 $
 *
 * Header file for weapons, includes specific featured lists.
 */

#include "weapons.vrt.h"

struct weapon_struct MechWeapons[] = {

    {"CL.AC/10", VRT_CL_AC10, TAMMO, 3, 10, 0, 5, 10, 15, 0, -1, -1, -1, 6,
	10, 1100, -1, CLAT | RFAC, 124},
    {"CL.AC/2", VRT_CL_AC2, TAMMO, 1, 2, 4, 8, 16, 24, 0, -1, -1, -1, 1,
	45, 500, -1, CLAT | RFAC, 37},
    {"CL.AC/20", VRT_CL_AC20, TAMMO, 7, 20, 0, 3, 6, 9, 0, -1, -1, -1, 9,
	5, 1300, -1, CLAT | RFAC, 178},
    {"CL.AC/5", VRT_CL_AC5, TAMMO, 1, 5, 3, 6, 12, 18, 0, -1, -1, -1, 3,
	20, 700, -1, CLAT | RFAC, 70},
    {"CL.A-Pod", VRT_CL_APOD, TBEAM, 0, 0, 0, 1, 1, 1, 0, -1, -1, -1, 1, 0,
	50, -1, CLAT | A_POD, 1},
    {"CL.Anti-MissileSystem", VRT_CL_AMS, TMISSILE, 1, 2, 0, 1, 1, 1, 0,
	-1, -1, -1, 1, 24, 50, -1, CLAT | AMS, 63},
    {"CL.ArrowIVSystem", VRT_CL_ARROWIV, TARTILLERY, 10, 20, 0, 0, 0, 6, 0,
	-1, -1, -1, 12, 5, 1200, -1, IDF | DAR | CLAT, 171},
    {"CL.GaussRifle", VRT_CL_GR, TAMMO, 1, 15, 2, 7, 15, 22, 0, -1, -1, -1,
	6, 8, 1200, 20, GAUSS | CLAT, 321},
    {"CL.LB10-XAC", VRT_CL_LBX10, TAMMO, 2, 10, 0, 6, 12, 18, 0, -1, -1,
	-1, 5, 10, 1000, -1, LBX | CLAT, 148},
    {"CL.LB2-XAC", VRT_CL_LBX2, TAMMO, 1, 2, 4, 10, 20, 30, 0, -1, -1, -1,
	3, 45, 500, -1, LBX | CLAT, 47},
    {"CL.LB20-XAC", VRT_CL_LBX20, TAMMO, 6, 20, 0, 4, 8, 12, 0, -1, -1, -1,
	9, 5, 1200, -1, LBX | CLAT, 237},
    {"CL.LB5-XAC", VRT_CL_LBX5, TAMMO, 1, 5, 3, 8, 15, 24, 0, -1, -1, -1,
	4, 20, 700, -1, LBX | CLAT, 93},
    {"CL.LRM-10", VRT_CL_LRM10, TMISSILE, 4, 1, 0, 7, 14, 21, 0, -1, -1,
	-1, 1, 12, 250, -1, IDF | CLAT, 109},
    {"CL.LRM-15", VRT_CL_LRM15, TMISSILE, 5, 1, 0, 7, 14, 21, 0, -1, -1,
	-1, 2, 8, 350, -1, IDF | CLAT, 164},
    {"CL.LRM-20", VRT_CL_LRM10, TMISSILE, 6, 1, 0, 7, 14, 21, 0, -1, -1,
	-1, 4, 6, 500, -1, IDF | CLAT, 220},
    {"CL.LRM-5", VRT_CL_LRM5, TMISSILE, 2, 1, 0, 7, 14, 21, 0, -1, -1, -1,
	1, 24, 100, -1, IDF | CLAT, 55},
    {"CL.MachineGun", VRT_CL_MG, TAMMO, 0, 2, 0, 1, 2, 3, 0, -1, -1, -1, 1,
	200, 25, -1, CLAT | GMG, 5},
    {"CL.NarcBeacon", VRT_CL_NARC, TMISSILE, 1, 4, 0, 4, 8, 12, 0, -1, -1,
	-1, 1, 6, 200, -1, NARC | CLAT, 30},
    {"CL.SRM-2", VRT_CL_SRM2, TMISSILE, 2, 2, 0, 3, 6, 9, 0, -1, -1, -1, 1,
	50, 50, -1, CLAT, 21},
    {"CL.SRM-4", VRT_CL_SRM4, TMISSILE, 3, 2, 0, 3, 6, 9, 0, -1, -1, -1, 1,
	25, 100, -1, CLAT, 39},
    {"CL.SRM-6", VRT_CL_SRM6, TMISSILE, 4, 2, 0, 3, 6, 9, 0, -1, -1, -1, 1,
	15, 150, -1, CLAT, 59},
    {"CL.StreakSRM-2", VRT_CL_SSRM2, TMISSILE, 2, 2, 0, 4, 8, 12, 0, -1,
	-1, -1, 1, 50, 100, -1, STREAK | CLAT | NOSPA, 40},
    {"CL.StreakSRM-4", VRT_CL_SSRM4, TMISSILE, 3, 2, 0, 4, 8, 12, 0, -1,
	-1, -1, 1, 25, 200, -1, STREAK | CLAT | NOSPA, 79},
    {"CL.StreakSRM-6", VRT_CL_SSRM6, TMISSILE, 4, 2, 0, 4, 8, 12, 0, -1,
	-1, -1, 2, 15, 300, -1, STREAK | CLAT | NOSPA, 119},
    {"CL.UltraAC/10", VRT_CL_UAC10, TAMMO, 3, 10, 0, 6, 12, 18, 0, -1, -1,
	-1, 4, 10, 1000, -1, ULTRA | CLAT, 211},
    {"CL.UltraAC/2", VRT_CL_UAC2, TAMMO, 1, 2, 2, 9, 18, 27, 0, -1, -1, -1,
	2, 45, 500, -1, ULTRA | CLAT, 62},
    {"CL.UltraAC/20", VRT_CL_UAC20, TAMMO, 7, 20, 0, 4, 8, 12, 0, -1, -1,
	-1, 8, 5, 1200, -1, ULTRA | CLAT, 337},
    {"CL.UltraAC/5", VRT_CL_UAC5, TAMMO, 1, 5, 0, 7, 14, 21, 0, -1, -1, -1,
	3, 20, 700, -1, ULTRA | CLAT, 123},
    {"IS.AC/10", VRT_IS_AC10, TAMMO, 3, 10, 0, 5, 10, 15, 0, -1, -1, -1, 7,
	10, 1200, -1, RFAC, 124},
    {"IS.AC/2", VRT_IS_AC2, TAMMO, 1, 2, 4, 8, 16, 24, 0, -1, -1, -1, 1,
	45, 600, -1, RFAC, 37},
    {"IS.AC/20", VRT_IS_AC20, TAMMO, 7, 20, 0, 3, 6, 9, 0, -1, -1, -1, 10,
	5, 1400, -1, RFAC, 178},
    {"IS.AC/5", VRT_IS_AC5, TAMMO, 1, 5, 3, 6, 12, 18, 0, -1, -1, -1, 4,
	20, 800, -1, RFAC, 70},
    {"IS.Anti-MissileSystem", VRT_IS_AMS, TMISSILE, 1, 2, 0, 1, 1, 1, 0,
	-1, -1, -1, 1, 12, 50, -1, AMS, 32},
    {"IS.ArrowIVSystem", VRT_IS_ARROWIV, TARTILLERY, 10, 20, 0, 0, 0, 5, 0,
	-1, -1, -1, 15, 5, 1500, -1, IDF | DAR, 171},
    {"IS.GaussRifle", VRT_IS_GR, TAMMO, 1, 15, 2, 7, 15, 22, 0, -1, -1, -1,
	7, 8, 1500, 20, GAUSS, 321},
    {"IS.LB10-XAC", VRT_IS_LBX10, TAMMO, 2, 10, 0, 6, 12, 18, 0, -1, -1,
	-1, 6, 10, 1100, -1, LBX, 148},
    {"IS.LRM-10", VRT_IS_LRM10, TMISSILE, 4, 1, 6, 7, 14, 21, 0, -1, -1,
	-1, 2, 12, 500, -1, IDF, 90},
    {"IS.LRM-15", VRT_IS_LRM15, TMISSILE, 5, 1, 6, 7, 14, 21, 0, -1, -1,
	-1, 3, 8, 700, -1, IDF, 136},
    {"IS.LRM-20", VRT_IS_LRM20, TMISSILE, 6, 1, 6, 7, 14, 21, 0, -1, -1,
	-1, 5, 6, 1000, -1, IDF, 181},
    {"IS.LRM-5", VRT_IS_LRM5, TMISSILE, 2, 1, 6, 7, 14, 21, 0, -1, -1, -1,
	1, 24, 200, -1, IDF, 45},
    {"IS.LongTom", VRT_IS_LONGTOM, TARTILLERY, 20, 20, 0, 0, 0, 20, 0, -1,
	-1, -1, 30, 5, 3000, -1, IDF | DAR, 171},
    {"IS.MachineGun", VRT_IS_MG, TAMMO, 0, 2, 0, 1, 2, 3, 0, -1, -1, -1, 1,
	200, 50, -1, GMG, 5},
    {"IS.NarcBeacon", VRT_IS_NARC, TMISSILE, 1, 4, 0, 3, 6, 9, 0, -1, -1,
	-1, 2, 6, 300, -1, NARC, 30},
    {"IS.SRM-2", VRT_IS_SRM2, TMISSILE, 2, 2, 0, 3, 6, 9, 0, -1, -1, -1, 1,
	50, 100, -1, NONE, 21},
    {"IS.SRM-4", VRT_IS_SRM4, TMISSILE, 3, 2, 0, 3, 6, 9, 0, -1, -1, -1, 1,
	25, 200, -1, NONE, 39},
    {"IS.SRM-6", VRT_IS_SRM6, TMISSILE, 4, 2, 0, 3, 6, 9, 0, -1, -1, -1, 2,
	15, 300, -1, NONE, 59},
    {"IS.Sniper", VRT_IS_SNIPER, TARTILLERY, 10, 10, 0, 0, 0, 12, 0, -1,
	-1, -1, 20, 10, 2000, -1, IDF | DAR, 86},
    {"IS.StreakSRM-2", VRT_IS_SSRM2, TMISSILE, 2, 2, 0, 3, 6, 9, 0, -1, -1,
	-1, 1, 50, 150, -1, STREAK | NOSPA, 30},
    {"IS.Thumper", VRT_IS_THUMPER, TARTILLERY, 6, 5, 0, 0, 0, 14, 0, -1,
	-1, -1, 15, 20, 1500, -1, IDF | DAR, 40},
    {"IS.UltraAC/5", VRT_IS_UAC5, TAMMO, 1, 5, 2, 6, 13, 20, 0, -1, -1, -1,
	5, 20, 900, -1, ULTRA, 113},
    {"PC.Blazer", VRT_PC_BLAZER, TAMMO, 0, 32, 0, 24, 48, 72, 0, -1, -1,
	-1, 1, 30, 0, -1, PC_HEAT, 1},
    {"PC.Crossbow", VRT_PC_BLAZER, TAMMO, 0, 9, 0, 9, 18, 27, 0, -1, -1,
	-1, 1, 20, 0, -1, PC_IMPA | NOBOOM, 1},
    {"PC.FederatedLongRifle", VRT_PC_FLRIFLE, TAMMO, 0, 9, 0, 22, 44, 66,
	0, -1, -1, -1, 1, 50, 0, -1, PC_IMPA, 1},
    {"PC.FlamerPistol", VRT_PC_FLAMER, TAMMO, 0, 7, 0, 4, 9, 13, 0, -1, -1,
	-1, 1, 50, 0, -1, PC_HEAT, 1},
    {"PC.GyroslugRifle", VRT_PC_GYROSLUG, TAMMO, 0, 14, 0, 30, 60, 90, 0,
	-1, -1, -1, 1, 15, 0, -1, PC_IMPA, 1},
    {"PC.HeavyGyrojetGun", VRT_PC_HGGUN, TAMMO, 0, 27, 0, 35, 70, 105, 0,
	-1, -1, -1, 1, 10, 0, -1, PC_IMPA, 1},
    {"PC.IntekLaserRifle", VRT_PC_ILRIFLE, TAMMO, 0, 9, 0, 40, 80, 120, 0,
	-1, -1, -1, 1, 45, 0, -1, PC_HEAT | NOBOOM, 1},
    {"PC.LaserRifle", VRT_PC_LRIFLE, TAMMO, 0, 17, 0, 24, 48, 72, 0, -1,
	-1, -1, 1, 30, 0, -1, PC_HEAT | NOBOOM, 1},
    {"PC.PulseLaserPistol", VRT_PC_PLPISTOL, TAMMO, 0, 11, 0, 5, 10, 15, 0,
	-1, -1, -1, 1, 50, 0, -1, PC_HEAT | NOBOOM, 1},
    {"PC.PulseLaserRifle", VRT_PC_PLRIFLE, TAMMO, 0, 13, 0, 22, 44, 66, 0,
	-1, -1, -1, 1, 30, 0, -1, PC_HEAT | NOBOOM, 1},
    {"PC.SMG", VRT_PC_SMG, TAMMO, 0, 10, 0, 7, 14, 20, 0, -1, -1, -1, 1,
	50, 0, -1, PC_IMPA, 1},
    {"PC.Shotgun", VRT_PC_SHOTGUN, TAMMO, 0, 12, 0, 6, 12, 18, 0, -1, -1,
	-1, 1, 10, 0, -1, PC_IMPA, 1},
    {"PC.SternsnachtPistol", VRT_PC_SPISTOL, TAMMO, 0, 16, 0, 7, 14, 20, 0,
	-1, -1, -1, 1, 50, 0, -1, PC_IMPA, 1},
    {"PC.SunbeamLaserPistol", VRT_PC_SLPISTOL, TAMMO, 0, 18, 0, 8, 16, 24,
	0, -1, -1, -1, 1, 50, 0, -1, PC_HEAT | NOBOOM, 1},
    {"PC.ZeusHeavyRifle", VRT_PC_ZHRIFLE, TAMMO, 0, 21, 0, 19, 38, 57, 0,
	-1, -1, -1, 1, 30, 0, -1, PC_IMPA, 1},

/* TacMunch stuff */

    {"IS.ELRM-5", VRT_IS_ELRM5, TMISSILE, 3, 1, 10, 12, 24, 36, 0, -1, -1,
	-1, 1, 18, 600, -1, IDF | ELRM | NOSPA, 1000},
    {"IS.ELRM-10", VRT_IS_ELRM10, TMISSILE, 6, 1, 10, 12, 24, 36, 0, -1,
	-1, -1, 4, 9, 800, -1, IDF | ELRM | NOSPA, 1000},
    {"IS.ELRM-15", VRT_IS_ELRM15, TMISSILE, 8, 1, 10, 12, 24, 36, 0, -1,
	-1, -1, 6, 6, 1200, -1, IDF | ELRM | NOSPA, 1000},
    {"IS.ELRM-20", VRT_IS_ELRM20, TMISSILE, 10, 1, 10, 12, 24, 36, 0, -1,
	-1, -1, 8, 4, 1800, -1, IDF | ELRM | NOSPA, 1000},
    {"IS.LR_DFM-5", VRT_IS_LR_DFM5, TMISSILE, 2, 2, 4, 6, 12, 18, 0, -1,
	-1, -1, 1, 24, 200, -1, IDF | DFM | NOSPA, 1000},
    {"IS.LR_DFM-10", VRT_IS_LR_DFM10, TMISSILE, 4, 2, 4, 6, 12, 18, 0, -1,
	-1, -1, 2, 12, 500, -1, IDF | DFM | NOSPA, 1000},
    {"IS.LR_DFM-15", VRT_IS_LR_DFM15, TMISSILE, 5, 2, 4, 6, 12, 18, 0, -1,
	-1, -1, 3, 8, 700, -1, IDF | DFM | NOSPA, 1000},
    {"IS.LR_DFM-20", VRT_IS_LR_DFM20, TMISSILE, 6, 2, 4, 6, 12, 18, 0, -1,
	-1, -1, 5, 6, 1000, -1, IDF | DFM | NOSPA, 1000},
    {"IS.SR_DFM-2", VRT_IS_SR_DFM2, TMISSILE, 2, 3, 0, 2, 4, 6, 0, -1, -1,
	-1, 1, 50, 100, -1, DFM | NOSPA, 1000},
    {"IS.SR_DFM-4", VRT_IS_SR_DFM4, TMISSILE, 3, 3, 0, 2, 4, 6, 0, -1, -1,
	-1, 1, 25, 200, -1, DFM | NOSPA, 1000},
    {"IS.SR_DFM-6", VRT_IS_SR_DFM6, TMISSILE, 4, 3, 0, 2, 4, 6, 0, -1, -1,
	-1, 2, 15, 300, -1, DFM | NOSPA, 1000},
    {"IS.StreakSRM-4", VRT_IS_SSRM4, TMISSILE, 3, 2, 0, 3, 6, 9, -1, -1,
	-1, 9, 1, 25, 300, -1, STREAK | NOSPA, 59},
    {"IS.StreakSRM-6", VRT_IS_SSRM6, TMISSILE, 4, 2, 0, 3, 6, 9, -1, -1,
	-1, 9, 2, 15, 450, -1, STREAK | NOSPA, 89},
    {"IS.Thunderbolt-5", VRT_IS_TBOLT5, TMISSILE, 3, 5, 5, 6, 12, 18, 0,
	-1, -1, -1, 1, 12, 300, -1, IDF | NOSPA, 1000},
    {"IS.Thunderbolt-10", VRT_IS_TBOLT10, TMISSILE, 5, 10, 5, 6, 12, 18, 0,
	-1, -1, -1, 2, 6, 600, -1, IDF | NOSPA, 1000},
    {"IS.Thunderbolt-15", VRT_IS_TBOLT15, TMISSILE, 7, 15, 5, 6, 12, 18, 0,
	-1, -1, -1, 3, 4, 800, -1, IDF | NOSPA, 1000},
    {"IS.Thunderbolt-20", VRT_IS_TBOLT20, TMISSILE, 8, 20, 5, 6, 12, 18, 0,
	-1, -1, -1, 5, 3, 1100, -1, IDF | NOSPA, 1000},
    {"IS.CaselessAC/2", VRT_IS_CAC2, TAMMO, 1, 2, 4, 8, 16, 24, 0, -1, -1,
	-1, 1, 67, 600, -1, NOSPA | CASELESS, 1000},
    {"IS.CaselessAC/5", VRT_IS_CAC5, TAMMO, 1, 5, 3, 6, 12, 18, 0, -1, -1,
	-1, 4, 30, 800, -1, NOSPA | CASELESS, 1000},
    {"IS.CaselessAC/10", VRT_IS_CAC10, TAMMO, 3, 10, 0, 5, 10, 15, 0, -1,
	-1, -1, 6, 15, 1200, -1, NOSPA | CASELESS, 1000},
    {"IS.CaselessAC/20", VRT_IS_CAC20, TAMMO, 7, 20, 0, 3, 6, 9, 9, 0, -1,
	-1, -1, 8, 1400, -1, NOSPA | CASELESS, 1000},
    {"IS.HyperAC/2", VRT_IS_HAC2, TAMMO, 1, 2, 3, 10, 20, 35, 0, -1, -1,
	-1, 4, 30, 800, -1, NOSPA | HYPER, 1000},
    {"IS.HyperAC/5", VRT_IS_HAC5, TAMMO, 3, 5, 0, 8, 16, 28, 0, -1, -1, -1,
	5, 15, 1200, -1, NOSPA | HYPER, 1000},
    {"IS.HyperAC/10", VRT_IS_HAC10, TAMMO, 7, 10, 0, 6, 12, 20, 0, -1, -1,
	-1, 6, 8, 1400, -1, NOSPA | HYPER, 1000},
    {"IS.LB2-XAC", VRT_IS_LBX2, TAMMO, 1, 2, 4, 9, 18, 27, 0, -1, -1, -1,
	4, 45, 600, -1, LBX, 42},
    {"IS.LB5-XAC", VRT_IS_LBX5, TAMMO, 1, 5, 3, 7, 14, 21, 0, -1, -1, -1,
	5, 20, 800, -1, LBX, 83},
    {"IS.LB20-XAC", VRT_IS_LBX20, TAMMO, 6, 20, 0, 4, 8, 12, 0, -1, -1, -1,
	11, 5, 1400, -1, LBX, 237},
    {"IS.UltraAC/2", VRT_IS_UAC2, TAMMO, 1, 2, 4, 8, 17, 25, 0, -1, -1, -1,
	3, 45, 700, -1, ULTRA, 56},
    {"IS.UltraAC/10", VRT_IS_UAC10, TAMMO, 4, 10, 0, 6, 12, 18, 0, -1, -1,
	-1, 7, 10, 1300, -1, ULTRA, 253},
    {"IS.UltraAC/20", VRT_IS_UAC20, TAMMO, 8, 20, 0, 3, 7, 10, 0, -1, -1,
	-1, 10, 5, 1500, -1, ULTRA, 282},

/* DCMS stuff */

    {"IS.MRM-10", VRT_IS_MRM10, TMISSILE, 4, 1, 0, 3, 8, 15, 0, -1, -1, -1,
	2, 24, 300, -1, MRM, 56},
    {"IS.MRM-20", VRT_IS_MRM20, TMISSILE, 6, 1, 0, 3, 8, 15, 0, -1, -1, -1,
	3, 12, 700, -1, MRM, 112},
    {"IS.MRM-30", VRT_IS_MRM30, TMISSILE, 10, 1, 0, 3, 8, 15, 0, -1, -1,
	-1, 5, 8, 1000, -1, MRM, 168},
    {"IS.MRM-40", VRT_IS_MRM40, TMISSILE, 12, 1, 0, 3, 8, 15, 0, -1, -1,
	-1, 7, 6, 1200, -1, MRM, 224},

/* .. normal cont'd ; energy weapons .. */

    {"CL.ERLargeLaser", VRT_CL_ERLL, TBEAM, 12, 10, 0, 8, 15, 25, 0, 5, 10,
	15, 1, 0, 400, -1, CLAT, 249},
    {"CL.ERMediumLaser", VRT_CL_ERML, TBEAM, 5, 7, 0, 5, 10, 15, 0, 3, 7,
	10, 1, 0, 100, -1, CLAT, 108},
    {"CL.ERPPC", VRT_CL_ERPPC, TBEAM, 15, 15, 0, 7, 14, 23, 0, 4, 10, 16,
	2, 0, 600, -1, CLAT, 412},
    {"CL.ERSmallLaser", VRT_CL_ERSL, TBEAM, 2, 5, 0, 2, 4, 6, 0, 1, 2, 4,
	1, 0, 50, -1, CLAT, 31},
    {"CL.Flamer", VRT_CL_FLAMER, TBEAM, 3, 2, 0, 1, 2, 3, 0, -1, -1, -1, 1,
	0, 50, -1, CLAT | CHEAT, 6},
    {"CL.LargeLaser", VRT_CL_LL, TBEAM, 8, 8, 0, 5, 10, 15, 0, 3, 6, 9, 1,
	0, 400, -1, CLAT, 124},
    {"CL.LargePulseLaser", VRT_CL_LPL, TBEAM, 10, 10, 0, 6, 14, 20, 0, 4,
	10, 14, 2, 0, 600, -1, PULSE | CLAT, 265},
    {"CL.MediumLaser", VRT_CL_ML, TBEAM, 3, 5, 0, 3, 6, 9, 0, 2, 4, 6, 1,
	0, 100, -1, CLAT, 46},
    {"CL.MediumPulseLaser", VRT_CL_MPL, TBEAM, 4, 7, 0, 4, 8, 12, 0, 3, 5,
	8, 1, 0, 200, -1, PULSE | CLAT, 111},
    {"CL.PPC", VRT_CL_PPC, TBEAM, 10, 10, 3, 6, 12, 18, 3, 4, 7, 10, 2, 0,
	600, -1, CLAT, 176},
    {"CL.SmallLaser", VRT_CL_SL, TBEAM, 1, 3, 0, 1, 2, 3, 0, 1, 2, -1, 1,
	0, 50, -1, CLAT, 9},
    {"CL.SmallPulseLaser", VRT_CL_SPL, TBEAM, 2, 3, 0, 2, 4, 6, 0, 1, 2, 4,
	1, 0, 100, -1, PULSE | CLAT, 24},
    {"IS.ERLargeLaser", VRT_IS_ERLL, TBEAM, 12, 8, 0, 7, 14, 19, 0, 3, 5,
	12, 2, 0, 500, -1, NONE, 163},
    {"IS.ERPPC", VRT_IS_ERPPC, TBEAM, 15, 10, 0, 7, 14, 23, 0, 4, 10, 16,
	3, 0, 700, -1, NONE, 229},
    {"IS.Flamer", VRT_IS_FLAMER, TBEAM, 3, 2, 0, 1, 2, 3, 0, -1, -1, -1, 1,
	0, 100, -1, CHEAT, 6},
    {"IS.LargeLaser", VRT_IS_LL, TBEAM, 8, 8, 0, 5, 10, 15, 0, 3, 6, 9, 2,
	0, 500, -1, NONE, 124},
    {"IS.LargePulseLaser", VRT_IS_LPL, TBEAM, 10, 9, 0, 3, 7, 10, 0, 2, 5,
	7, 2, 0, 700, -1, PULSE, 119},
    {"IS.MediumLaser", VRT_IS_ML, TBEAM, 3, 5, 0, 3, 6, 9, 0, 2, 4, 6, 1,
	0, 100, -1, NONE, 46},
    {"IS.MediumPulseLaser", VRT_IS_MPL, TBEAM, 4, 6, 0, 2, 4, 6, 0, 2, 3,
	4, 1, 0, 200, -1, PULSE, 48},
    {"IS.PPC", VRT_IS_PPC, TBEAM, 10, 10, 3, 6, 12, 18, 3, 4, 7, 10, 3, 0,
	700, -1, NONE, 176},
    {"IS.SmallLaser", VRT_IS_SL, TBEAM, 1, 3, 0, 1, 2, 3, 0, 1, 2, -1, 1,
	0, 50, -1, NONE, 9},
    {"IS.SmallPulseLaser", VRT_IS_SPL, TBEAM, 2, 3, 0, 1, 2, 3, 0, 1, 2,
	-1, 1, 0, 100, -1, PULSE, 12},

/* pc weapons without ammo */

    {"PC.Sword", VRT_PC_SWORD, THAND, 0, 5, 0, 1, 1, 1, 0, -1, -1, -1, 1,
	0, 0, -1, PC_SHAR, 1},
    {"PC.Vibroblade", VRT_PC_VIBROBLADE, THAND, 0, 7, 0, 1, 1, 1, 0, -1,
	-1, -1, 1, 0, 0, -1, PC_SHAR, 1},

/* FWL FM stuff */

    {"IS.ERMediumLaser", VRT_IS_ERML, TBEAM, 5, 5, 0, 4, 8, 12, 0, 3, 5, 8,
	1, 0, 100, -1, NONE, 62},
    {"IS.ERSmallLaser", VRT_IS_ERSL, TBEAM, 2, 3, 0, 2, 4, 5, 0, 1, 2, 3,
	1, 0, 50, -1, NONE, 17},
    {"IS.LightGaussRifle", VRT_IS_LGR, TAMMO, 1, 8, 3, 8, 17, 25, 0, -1,
	-1, -1, 5, 16, 1200, 16, GAUSS, 159},

/* MaxMunch stuff */

    {"IS.X-LargePulseLaser", VRT_IS_XLPL, TBEAM, 14, 9, 0, 5, 10, 15, 0, 3,
	6, 9, 2, 0, 700, -1, PULSE, 178},
    {"IS.X-MediumPulseLaser", VRT_IS_XMPL, TBEAM, 6, 6, 0, 3, 6, 9, 0, 2,
	4, 6, 1, 0, 200, -1, PULSE, 71},
    {"IS.X-SmallPulseLaser", VRT_IS_XSPL, TBEAM, 3, 3, 0, 2, 4, 5, 0, -1,
	-1, -1, 1, 0, 100, -1, PULSE, 21},
    {"IS.HeavyFlamer", VRT_IS_HFLAMER, TBEAM, 5, 4, 0, 2, 4, 6, 0, -1, -1,
	-1, 1, 0, 100, -1, CHEAT, 20},
    {"IS.HeavyMachineGun", VRT_IS_HMG, TAMMO, 0, 2, 0, 2, 4, 6, 0, -1, -1,
	-1, 1, 100, 100, -1, GMG, 6},
    {"IS.LightAC/2", VRT_IS_LAC2, TAMMO, 1, 2, 0, 6, 12, 18, 0, -1, -1, -1,
	1, 45, 400, -1, RFAC, 30},
    {"IS.LightAC/5", VRT_IS_LAC5, TAMMO, 1, 5, 0, 5, 10, 15, 0, -1, -1, -1,
	1, 20, 400, -1, RFAC, 62},
    {"CL.ERLargePulseLaser", VRT_CL_ERLPL, TBEAM, 13, 10, 0, 7, 15, 23, 0,
	4, 10, 16, 3, 0, 600, -1, PULSE | CLAT, 271},
    {"CL.ERMediumPulseLaser", VRT_CL_ERMPL, TBEAM, 6, 7, 0, 5, 9, 14, 0, 3,
	6, 8, 2, 0, 200, -1, PULSE | CLAT, 116},
    {"CL.ERSmallPulseLaser", VRT_CL_ERSPL, TBEAM, 3, 5, 0, 2, 4, 6, 0, 2,
	3, 4, 1, 0, 150, -1, PULSE | CLAT, 36},
    {"CL.StreakLRM-5", VRT_CL_SLRM5, TMISSILE, 2, 1, 6, 7, 14, 21, 0, -1,
	-1, -1, 1, 24, 200, -1, STREAK | CLAT | NOSPA, 87},
    {"CL.StreakLRM-10", VRT_CL_SLRM10, TMISSILE, 4, 1, 6, 7, 14, 21, 0, -1,
	-1, -1, 2, 12, 500, -1, STREAK | CLAT | NOSPA, 173},
    {"CL.StreakLRM-15", VRT_CL_SLRM15, TMISSILE, 5, 1, 6, 7, 14, 21, 0, -1,
	-1, -1, 3, 8, 700, -1, STREAK | CLAT | NOSPA, 260},
    {"CL.StreakLRM-20", VRT_CL_SLRM20, TMISSILE, 6, 1, 6, 7, 14, 21, 0, -1,
	-1, -1, 5, 6, 1000, -1, STREAK | CLAT | NOSPA, 346},
    {"IS.A-Pod", VRT_IS_APOD, TBEAM, 0, 0, 0, 1, 1, 1, 0, -1, -1, -1, 1, 0,
	50, -1, A_POD, 1},
    {"CL.HeavyLargeLaser", VRT_CL_HLL, TBEAM, 18, 16, 0, 5, 10, 15, 0, 3,
	6, 9, 3, 0, 400, -1, CLAT | HVYW, 243},
    {"CL.HeavyMediumLaser", VRT_CL_HML, TBEAM, 7, 10, 0, 3, 6, 9, 0, 2, 4,
	6, 2, 0, 100, -1, CLAT | HVYW, 76},
    {"CL.HeavySmallLaser", VRT_CL_HSL, TBEAM, 3, 6, 0, 1, 2, 3, 0, 1, 2,
	-1, 1, 0, 50, -1, CLAT | HVYW, 15},

    /* new FM stuff */
    {"IS.iNarcBeacon", VRT_IS_INARC, TMISSILE, 1, 6, 0, 4, 9, 15, 0, -1,
	-1, -1, 3, 4, 500, -1, INARC, 75},
    {"IS.RotaryAC/2", VRT_IS_RAC2, TAMMO, 1, 2, 0, 6, 12, 18, 0, -1, -1,
	-1, 3, 45, 800, -1, RAC, 118},
    {"IS.RotaryAC/5", VRT_IS_RAC5, TAMMO, 1, 5, 0, 5, 10, 15, 0, -1, -1,
	-1, 6, 20, 1000, -1, RAC, 247},
    {"IS.HeavyGaussRifle", VRT_IS_HGR, TAMMO, 2, 25, 4, 6, 13, 20, 0, -1,
	-1, -1, 11, 4, 1800, 25, GAUSS | HVYGAUSS, 346},
    {"IS.MagshotGaussRifle", VRT_IS_MGR, TAMMO, 0, 2, 0, 3, 6, 9, 0, -1,
	-1, -1, 1, 20, 50, 5, GAUSS, 10},

    {"CL.MicroPulseLaser", VRT_CL_MICROPL, TBEAM, 1, 3, 0, 1, 2, 3, 0, 1,
	2, 2, 1, 0, 50, -1, PULSE | CLAT, 12},

    {"IS.RL-10", VRT_IS_RL10, TMISSILE, 3, 1, 0, 5, 11, 18, 0, -1, -1, -1,
	1, 0, 50, -1, ROCKET | IDF, 18},
    {"IS.RL-15", VRT_IS_RL15, TMISSILE, 4, 1, 0, 4, 9, 15, 0, -1, -1, -1,
	2, 0, 100, -1, ROCKET | IDF, 23},
    {"IS.RL-20", VRT_IS_RL20, TMISSILE, 5, 1, 0, 3, 7, 12, 0, -1, -1, -1,
	3, 0, 150, -1, ROCKET | IDF, 24},

    {NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, NONE, 1}

};

struct missile_hit_table_struct MissileHitTable[] = {
    {"CL.LB10-XAC", 0, {3, 3, 4, 6, 6, 6, 6, 8, 8, 10, 10}},
    {"CL.LB20-XAC", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"CL.LB2-XAC", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.LB5-XAC", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"CL.LRM-10", 0, {3, 3, 4, 6, 6, 6, 6, 8, 8, 10, 10}},
    {"CL.LRM-15", 0, {5, 5, 6, 9, 9, 9, 9, 12, 12, 15, 15}},
    {"CL.LRM-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"CL.StreakLRM-5", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"CL.StreakLRM-10", 0, {3, 3, 4, 6, 6, 6, 6, 8, 8, 10, 10}},
    {"CL.StreakLRM-15", 0, {5, 5, 6, 9, 9, 9, 9, 12, 12, 15, 15}},
    {"CL.StreakLRM-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"CL.LRM-5", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"CL.SRM-2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.SRM-4", 0, {1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4}},
    {"CL.SRM-6", 0, {2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6}},
    {"CL.StreakSRM-2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.StreakSRM-4", 0, {1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4}},
    {"CL.StreakSRM-6", 0, {2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6}},
    {"CL.UltraAC/10", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.UltraAC/20", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.UltraAC/2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.UltraAC/5", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.AC/2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.AC/5", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.AC/10", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"CL.AC/20", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.LB10-XAC", 0, {3, 3, 4, 6, 6, 6, 6, 8, 8, 10, 10}},
    {"IS.LRM-5", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"IS.LRM-10", 0, {3, 4, 4, 5, 6, 6, 6, 8, 8, 10, 10}},
    {"IS.LRM-15", 0, {5, 5, 9, 9, 9, 9, 9, 12, 12, 15, 15}},
    {"IS.LRM-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"IS.SRM-2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.SRM-4", 0, {1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4}},
    {"IS.SRM-6", 0, {2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6}},
    {"IS.StreakSRM-2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.StreakSRM-4", 0, {1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4}},
    {"IS.StreakSRM-6", 0, {2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6}},
    {"IS.UltraAC/5", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.LB20-XAC", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"IS.LB2-XAC", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.LB5-XAC", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"IS.UltraAC/10", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.UltraAC/20", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.UltraAC/2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.AC/2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.AC/5", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.AC/10", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.AC/20", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.LightAC/2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.LightAC/5", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.Thunderbolt-5", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"IS.Thunderbolt-10", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"IS.Thunderbolt-15", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"IS.Thunderbolt-20", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"IS.ELRM-10", 0, {3, 4, 4, 5, 6, 6, 6, 7, 7, 10, 10}},
    {"IS.ELRM-5", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"IS.ELRM-15", 0, {5, 5, 9, 9, 9, 9, 12, 12, 12, 15, 15}},
    {"IS.ELRM-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"IS.LR_DFM-10", 0, {3, 4, 4, 5, 6, 6, 6, 7, 7, 10, 10}},
    {"IS.LR_DFM-5", 0, {1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5}},
    {"IS.LR_DFM-15", 0, {5, 5, 9, 9, 9, 9, 12, 12, 12, 15, 15}},
    {"IS.LR_DFM-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"IS.SR_DFM-2", 0, {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2}},
    {"IS.SR_DFM-4", 0, {1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4}},
    {"IS.SR_DFM-6", 0, {2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6}},
    {"IS.NarcBeacon", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"CL.NarcBeacon", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"IS.MRM-10", 0, {2, 3, 4, 5, 6, 6, 6, 8, 8, 10, 10}},
    {"IS.MRM-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"IS.MRM-30", 0, {10, 14, 16, 18, 18, 18, 20, 24, 28, 30, 30}},
    {"IS.MRM-40", 0, {12, 12, 18, 24, 24, 24, 24, 32, 32, 40, 40}},
    {"IS.iNarcBeacon", 0, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {"IS.RL-10", 0, {3, 4, 4, 5, 6, 6, 6, 8, 8, 10, 10}},
    {"IS.RL-15", 0, {5, 5, 9, 9, 9, 9, 9, 12, 12, 15, 15}},
    {"IS.RL-20", 0, {6, 6, 9, 12, 12, 12, 12, 16, 16, 20, 20}},
    {"NoWeapon", -1, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}
};

#define NUM_DEF_WEAPONS (((sizeof(MechWeapons))/ \
			 (sizeof(struct weapon_struct)))-1)
