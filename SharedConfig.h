#pragma once
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"


//TODO 
extern GEFloat npcSTRMultiplier;
extern GEInt npcSTRAddition;
//
// 
// Configureable

//NPC Level for Powergroup
extern GEU32 bossLevel;
extern GEU32 uniqueLevel;
extern GEU32 eliteLevel;
extern GEU32 warriorLevel;
extern GEU32 noviceLevel;

extern GEBool useNewBalanceMagicWeapon;
extern GEBool useNewStaminaRecovery;
extern GEBool useAlwaysMaxLevel;
extern GEBool enablePerfectBlock;
extern GEBool playerOnlyPerfectBlock;
extern GEBool useNewBalanceMeleeScaling;
extern GEBool adjustXPReceive;
extern GEBool useNewBowMechanics;
extern GEBool enableNPCSprint;
extern GEBool zombiesCanSprint;
extern GEBool enableNewTransformation;
extern GEBool disableMonsterRage;

extern GEInt poiseThreshold;
extern GEFloat npcArenaSpeedMultiplier;
extern GEFloat npcWeaponDamageMultiplier;
extern GEU32 staminaRecoveryDelay;
extern GEU32 staminaRecoveryPerTick;
extern GEFloat fMonsterDamageMultiplicator; // Default 0.5
extern GEDouble npcArmorMultiplier;
extern GEDouble playerArmorMultiplier;
extern GEU32 startSTR;
extern GEU32 startDEX;
extern GEDouble shootVelocity;
extern GEDouble NPC_AIM_INACCURACY;
extern GEDouble ATTACK_REACH_MULTIPLIER;

extern GEU32 blessedBonus;
extern GEU32 sharpBonus;
extern GEBool useSharpPercentage;
extern GEU32 forgedBonus;
extern GEU32 wornPercentageMalus;

//
// 
//Const Data for Usage:
static const GEDouble PROJECTILEMULTIPLICATOR = 27.777778;
static const GEDouble* npcArmorMultiplierPtr = &npcArmorMultiplier;
static GEU32 elementalPerkBonusResistance = 35;
static const GEDouble* shootVelocityPtr = &shootVelocity;
static GEFloat animationSpeedBonusMid = 1.3f;
static GEFloat animationSpeedBonusHigh = 1.6f;

static char* BONETARGET = "Spine_Spine_3";
static char* powerAimEffectName = "eff_ani_fight_bow_raise_01";
static char* powerAimWarriorEffectName = "eff_ani_fight_bow_raise_warrior_01";
static char* powerAimEliteEffectName = "eff_ani_fight_bow_raise_elite_01";

static bCString blessedBonusString;
static bCString sharpBonusString;
static bCString forgedBonusString;
static bCString wornMalusString;
//