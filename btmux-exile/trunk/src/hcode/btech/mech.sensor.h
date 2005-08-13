#ifndef MECH_SENSOR_H
#define MECH_SENSOR_H

/* 
   For all scanners chance of seeing a foe is modified by:
   - Side arcs are 70% chance
   - Rear arc is 40% chance
 */

typedef struct {
    char *sensorname;

    /* Is the sensor 360 degree with just one of them? */
    int fullvision;

    /* Longest vis this sensor brand sees */
    int maxvis;

    /* Variable factor in maxvis ; it changes by +- 1 every 30 seconds */
    int maxvvar;

    /* Function for retrieving generic chance of spotting foe with
       this scanner at the range */
    /* first int = sensor type #, second = maxrange by conditions,
       third = lightning level */
    int (*seechance_func) (MECH *, int, float, int, int);

    /* Do we really see 'em? Mainly checks for various things that
       vary between diff. sensors (and also seechancefunc > 0) */
    int (*cansee_func) (MECH *, MECH *, float, int);

    /* Chance of actually hitting someone */
    int (*tohitbonus_func) (MECH * mech, MECH * target, int, int);

    /* If <0, not used */
    int min_light;
    int max_light;

    int required_special;

    char *range_desc;
    char *block_desc;
    char *special_desc;
} SensorStruct;

#define ESEEFUNC(a) extern int a (MECH *,int, float, int, int);
#define SEEFUNC(a,b) \
int a (MECH *t, int num, float r, int c, int l) { return (int) (b); }
ESEEFUNC(vislight_see);
ESEEFUNC(liteamp_see);
ESEEFUNC(infrared_see);
ESEEFUNC(electrom_see);
ESEEFUNC(seismic_see);
ESEEFUNC(radar_see);
ESEEFUNC(bap_see);

#define ECSEEFUNC(a) extern int a (MECH *, MECH *, float, int);
#define CSEEFUNC(a,b) \
int a (MECH *m, MECH *t, float r, int f) { return (int) (b); }
ECSEEFUNC(vislight_csee);
ECSEEFUNC(liteamp_csee);
ECSEEFUNC(infrared_csee);
ECSEEFUNC(electrom_csee);
ECSEEFUNC(seismic_csee);
ECSEEFUNC(radar_csee);
ECSEEFUNC(bap_csee);

#define ETOHITFUNC(a) extern int a (MECH *, MECH *,int,int);
#define TOHITFUNC(a,b) \
int a (MECH *m, MECH *t, int f, int l) { return (int) (b); }

ETOHITFUNC(vislight_tohit);
ETOHITFUNC(liteamp_tohit);
ETOHITFUNC(infrared_tohit);
ETOHITFUNC(electrom_tohit);
ETOHITFUNC(seismic_tohit);
ETOHITFUNC(radar_tohit);
ETOHITFUNC(bap_tohit);

#define SENSOR_VIS 0
#define SENSOR_LA  1
#define SENSOR_IR  2
#define SENSOR_EM  3
#define SENSOR_SE  4
#define SENSOR_RA  5
#define SENSOR_BP  6
#define SENSOR_BL  7

#ifdef _MECH_SENSOR_C
SensorStruct sensors[] = {
    {"Vislight", 0, 60, 0, vislight_see, vislight_csee, vislight_tohit,
	    -1, -1, 0,
	    "Visual",
	    "Fire/Obstacles, 3 pt woods, 3 pt smoke, 5 underwater hexes",
	"Bad in night-fighting (BTH)"},
    {"Light-amplification", 0, 60, 0, liteamp_see, liteamp_csee,
	    liteamp_tohit,
	    0, 1, 0 - NS_TECH,
	    "Visual (Dawn/Dusk), 2x Visual (Night)",
	    "Fire/Obstacles, 2 pt woods, 2 pt smoke, any water",
	"Somewhat harder enemy detection (than vislight), bad in forests (BTH/range)"},
    {"Infrared", 1, 15, 0, infrared_see, infrared_csee, infrared_tohit,
	    -1, -1, 0 - NS_TECH,
	    "15",
	    "Fire/Obstacles, 6 pt woods, 6 pt smoke",
	"Easy to hit 'hot' targets, not very efficient in forests (BTH) Better than vis in smoke."},
    {"Electromagnetic", 1, 20, 10, electrom_see, electrom_csee,
	    electrom_tohit,
	    -1, -1, 0 - NS_TECH,
	    "10-20",
	    "Mountains/Obstacles, 10 pt woods",
	"Easy to hit heavies, good in forests/smoke (BTH), overall unreliable (chances of detection/BTH)"},
    {"Seismic", 1, 12, 8, seismic_see, seismic_csee, seismic_tohit, -1, -1,
	    0 - NS_TECH,
	    "4-12",
	    "Nothing",
	"Easier heavy and/or moving object detection (although overall hard to detect with), somewhat unreliable(BTH)"},
    {"Radar", 1, 100, 0, radar_see, radar_csee, radar_tohit, -1, -1,
	    AA_TECH,
	    "<=100",
	    "Obstacles, enemy elevation (Enemy Z >= 10, range: 100, Enemy Z < 10, range: varies)",
	"Premier anti-aircraft sensor, partially negates partial cover(BTH), doesn't see targets that are too low for detection"},

    {"Beagle ActiveProbe", 1, 4, 0, bap_see, bap_csee, bap_tohit, -1,
	    -1, BEAGLE_PROBE_TECH, "<=4", "Nothing (except range)",
	"Ultimate sensor in close-range detection (very slightly varying BTH, but ignores partial/woods/water)"},

    {"Bloodhound ActiveProbe", 1, 8, 0, bap_see, bap_csee, bap_tohit, -2,
	    -2, BLOODHOUND_PROBE_TECH, "<=8", "Nothing (except range)",
	"Ultimate sensor in close-range detection (very slightly varying BTH, but ignores partial/woods/water/hidden)"},

    {"ARadar", 1, 5000, 0, radar_see, radar_csee, radar_tohit, -2, -2,
	    ADV_AA_TECH,
	    "<=5000",
	    "Obstacles, enemy elevation (Enemy Z >= 10, range: 100, Enemy Z < 10, range: varies)",
	"Premier anti-aircraft sensor, partially negates partial cover(BTH), doesn't see targets that are too low for detection"}
};

#define NUM_SENSORS (sizeof (sensors) / sizeof(sensors[0]))
#else
extern SensorStruct sensors[];
#endif
#endif				/* MECH_SENSOR_H */
