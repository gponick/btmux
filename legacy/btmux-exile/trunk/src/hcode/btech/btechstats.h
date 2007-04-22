/* Function declarations / skill list for btechstats.c */

#ifndef BTECHSTATS_H
#define BTECHSTATS_H

#include "db.h"
#include "externs.h"
#include "interface.h"
#include "config.h"
#include "powers.h"
#include "btechstats_global.h"

#ifdef BTECHSTATS_C
char *btech_charvaluetype_names[] =
    { "Char_value", "Char_skill", "Char_advantage", "Char_attribute" };

char *btech_charskillflag_names[] =
    { "Athletic", "Mental", "Physical", "Social" };

#endif

#define EE_NUMBER 11

#ifdef BTECHSTATS

struct char_value {
    char *name;
    char type;
    int flag;
    int xpthreshold;
} char_values[] = {
    {
    "XP", CHAR_VALUE, 0, 0}, {
    "MaxXP", CHAR_VALUE, 0, 0}, {
    "Type", CHAR_VALUE, 0, 0}, {
    "Level", CHAR_VALUE, 0, 0}, {
    "Package", CHAR_VALUE, 0, 0}, {
    "Lives", CHAR_VALUE, 0, 0}, {
    "Bruise", CHAR_VALUE, 0, 0}, {
    "Lethal", CHAR_VALUE, 0, 0}, {
    "Unused1", CHAR_VALUE, 0, 0}, 
/* Advantages */
    {
    "Ambidextrous", CHAR_ADVANTAGE, CHAR_ADV_BOOL, 0}, {
    "Exceptional_Attribute", CHAR_ADVANTAGE, CHAR_ADV_EXCEPT, 0}, {
    "Extra_Edge", CHAR_ADVANTAGE, CHAR_ADV_VALUE, 0}, {
    "Sixth_Sense", CHAR_ADVANTAGE, CHAR_ADV_BOOL, 0}, {
    "Toughness", CHAR_ADVANTAGE, CHAR_ADV_BOOL, 0}, {
    "Wealth", CHAR_ADVANTAGE, CHAR_ADV_BOOL, 0}, 
/* Attributes */
    {
    "Build", CHAR_ATTRIBUTE, 0, 0}, {
    "Reflexes", CHAR_ATTRIBUTE, 0, 0}, {
    "Intuition", CHAR_ATTRIBUTE, 0, 0}, {
    "Learn", CHAR_ATTRIBUTE, 0, 0}, {
    "Charisma", CHAR_ATTRIBUTE, 0, 0}, 
/* Skills themselves */
    {
    "Acrobatics", CHAR_SKILL, SK_XP | CHAR_ATHLETIC, 50}, {
    "Administration", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
    "Alternate_Identity", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
    "Appraisal", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
    "Archery", CHAR_SKILL, SK_XP | CHAR_ATHLETIC, 50}, {
    "Blade", CHAR_SKILL, SK_XP | CHAR_ATHLETIC | CAREER_MISC, 50}, {
    "Bureaucracy", CHAR_SKILL, SK_XP | CHAR_SOCIAL | CAREER_MISC, 50}, {
    "Climbing", CHAR_SKILL, SK_XP | CHAR_ATHLETIC, 50}, {
    "Comm-Conventional", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 150}, {
    "Comm-Hyperpulse", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 50}, {
    "Computer", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 50}, {
    "Cryptography", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 50}, {
    "Demolitions", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
    "Disguise", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_RECON, 50}, {
#if NEW_STATS
    "Piloting-Tracked", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 3000}, {
#else
    "Drive", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 3000}, {
#endif
    "Engineering", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 50}, {
    "Escape_Artist", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_RECON, 50}, {
    "Forgery", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
    "Gambling", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
#if NEW_STATS
    "Gunnery-Laser", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_GUNNER, 7500}, {
    "Gunnery-Missile", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_GUNNER, 7500}, {
    "Gunnery-Autocannon", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_GUNNER, 7500}, {
    "Gunnery-Artillery", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_ARTILLERY , 5000}, {
    "Gunnery-Spotting", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 1500}, {
    "Piloting-Hover", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 3000}, {
    "Piloting-Wheeled", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 3000}, {
#else
    "Gunnery-Aerospace", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_AERO, 1000}, {
    "Gunnery-Artillery", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_ARTILLERY, 500}, {
    "Gunnery-Battlemech", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_BMECH, 1600}, {
    "Gunnery-BSuit", CHAR_SKILL, SK_XP | CHAR_PHYSICAL| CAREER_BSUIT, 500}, {
    "Gunnery-Conventional", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 1600}, {
    "Gunnery-Spacecraft", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_DROPSHIP, 50}, {
    "Gunnery-Spotting", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_ARTILLERY, 5}, { 
#endif
    "Impersonation", CHAR_SKILL, SK_XP | CHAR_SOCIAL, 50}, {
    "Interrogation", CHAR_SKILL, SK_XP | CHAR_SOCIAL | CAREER_RECON, 50}, {
    "Jump_Pack", CHAR_SKILL, SK_XP | CHAR_ATHLETIC, 50}, {
    "Leadership", CHAR_SKILL, SK_XP | CHAR_SOCIAL | CAREER_ACADMISC, 50}, {
    "Medtech", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_MISC, 300}, {
    "Navigation", CHAR_SKILL, SK_XP | CHAR_MENTAL, 25}, {
    "Negotiation", CHAR_SKILL, SK_XP | CHAR_SOCIAL, 25}, {
    "Perception", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_RECON, 150}, {
#if NEW_STATS
    "Piloting-Quad", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_BMECH, 3000}, {
    "Piloting-Biped", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_BMECH, 3000}, {
    "Piloting-BSuit", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_BSUIT, 3000}, {
    "Piloting-Spacecraft", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_DROPSHIP, 3000}, { 
#else
    "Piloting-Aerospace", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_AERO, 2500}, {
    "Piloting-Battlemech", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_BMECH, 3000}, {
    "Piloting-BSuit", CHAR_SKILL, SK_XP | CHAR_PHYSICAL, 3000}, {
    "Piloting-Spacecraft", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_DROPSHIP, 50}, {
#endif
    "Protocol", CHAR_SKILL, SK_XP | CHAR_SOCIAL, 50}, {
    "Quickdraw", CHAR_SKILL, SK_XP | CHAR_PHYSICAL, 50}, {
    "Research", CHAR_SKILL, SK_XP |CHAR_MENTAL | CAREER_TECH, 100}, {
    "Running", CHAR_SKILL, SK_XP | CHAR_ATHLETIC, 100}, {
    "Scrounge", CHAR_SKILL, SK_XP | CHAR_SOCIAL | CAREER_TECH, 50}, {
    "Security_Systems", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_RECON, 50}, {
    "Seduction", CHAR_SKILL, SK_XP | CHAR_SOCIAL, 50}, {
    "Small_Arms", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_MISC, 50}, {
    "Stealth", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_RECON, 50}, {
    "Strategy", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_ACADMISC, 50}, {
    "Streetwise", CHAR_SKILL, SK_XP | CHAR_SOCIAL, 50}, {
    "Support_Weapons", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_MISC, 50}, {
    "Survival", CHAR_SKILL, SK_XP | CHAR_MENTAL, 50}, {
    "Swimming", CHAR_SKILL, SK_XP | CHAR_ATHLETIC, 50}, {
    "Tactics", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_ACADMISC, 50}, {
    "Technician-Aerospace", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECHVEH, 50}, {
    "Technician-BSuit", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECHBS, 200}, {
    "Technician-Battlemech", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECHMECH, 600}, {
    "Technician-Electronics", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 50}, {
    "Technician-Mechanic", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECHVEH, 400}, {
    "Technician-Weapons", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 300}, {
    "Throwing_Weapons", CHAR_SKILL, SK_XP | CHAR_PHYSICAL, 50}, {
    "Tinker", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_TECH, 50}, {
    "Tracking", CHAR_SKILL, SK_XP | CHAR_MENTAL | CAREER_RECON, 50}, {
    "Training", CHAR_SKILL, SK_XP | CHAR_SOCIAL, 50}, {
    "Unarmed_Combat", CHAR_SKILL, SK_XP | CHAR_ATHLETIC | CAREER_MISC, 50}, {
    "Dodge_Maneuver", CHAR_ADVANTAGE, SK_XP | CHAR_ADV_BOOL, 0}, {
    "Maneuvering_Ace", CHAR_ADVANTAGE, SK_XP | CHAR_ADV_BOOL, 0}, {
    "Melee_Specialist", CHAR_ADVANTAGE, SK_XP | CHAR_ADV_BOOL, 0}, {
    "Pain_Resistance", CHAR_ADVANTAGE, SK_XP | CHAR_ADV_BOOL, 0}, {
    "Speed_Demon", CHAR_ADVANTAGE, SK_XP | CHAR_ADV_BOOL, 0}
#if NEW_STATS
    , {
    "Piloting-Aerospace", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_AERO, 3000}, {
    "Piloting-Naval", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_CAVALRY, 3000}, {
    "Gunnery-Flamer", CHAR_SKILL, SK_XP | CHAR_PHYSICAL | CAREER_GUNNER , 3000}, {
    "Tech_Aptitude", CHAR_ADVANTAGE, CHAR_ADV_BOOL, 0}
#endif
};

#define NUM_CHARVALUES sizeof(char_values)/sizeof(struct char_value)

char *char_values_short[NUM_CHARVALUES];

/*************************************************************************/

char *char_levels[] = { "Green", "Regular", "Veteran",
    "Elite", "Historical"
};

#define NUM_CHARLEVELS 5

char *char_types[] = { "Inner_Sphere", "Clan_MechWarrior",
    "Clan_Aerospace", "Clan_Elemental",
    "Clan_Freebirth", "Clan_Other"
};

#define NUM_CHARTYPES 6

char *char_packages[] = { "None",
    "Primary_Clan_Warrior", "Secondary_Clan_Warrior",
    "Secondar_Clan_Pilot", "Clan_Elemental",
    "Basic_Academy", "Advanced_Academy",
    "Basic_University", "Advanced_University"
};

#define NUM_CHARPACKAGES 9

/* 
   Okay.. XP is gained as follows:
   - If last xp-gain use is >= 60 sec away, add xp.
   - Must be online
 */

typedef struct {
    dbref dbref;
    unsigned char values[NUM_CHARVALUES];
    time_t last_use[NUM_CHARVALUES];
    int xp[NUM_CHARVALUES];
} PSTATS;

#endif

#include "p.btechstats.h"

#endif				/* BTECHSTATS_H */
