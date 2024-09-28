#pragma once
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"


//TODO 
static GEFloat npcSTRMultiplier = 1.5;
static GEInt npcSTRAddition = 15;
//
// 
// Configureable
static GEBool useNewBalanceMagicWeapon = GEFalse;
static GEBool useNewStaminaRecovery = GETrue;
static GEBool useAlwaysMaxLevel = GETrue;
static GEBool enablePerfectBlock = GETrue;
static GEBool playerOnlyPerfectBlock = GEFalse;
static GEBool useNewBalanceMeleeScaling = GETrue;
static GEBool adjustXPReceive = GETrue;
static GEBool useNewBowMechanics = GETrue;

static GEFloat npcWeaponDamageMultiplier = 0.15;
static GEU32 staminaRecoveryDelay = 20;
static GEU32 staminaRecoveryPerTick = 13;
static GEFloat fMonsterDamageMultiplicator = 0.5; // Default 0.5
static GEDouble npcArmorMultiplier = 1.5;
static GEDouble playerArmorMultiplier = 0.4;
static GEU32 startSTR = 100;
static GEU32 startDEX = 100;
static GEDouble shootVelocity = 300;
static GEDouble NPC_AIM_INACCURACY = 0.40;
static GEDouble ATTACK_REACH_MULTIPLIER = 1.75;

static GEU32 blessedBonus = 10;
static GEU32 sharpBonus = 10;
static GEBool useSharpPercentage = GETrue;
static GEU32 forgedBonus = 20;
static GEU32 wornPercentageMalus = 70;

//
// 
//Const Data for Usage:
static const GEDouble PROJECTILEMULTIPLICATOR = 27.777778;
static const GEDouble* npcArmorMultiplierPtr = &npcArmorMultiplier;
static GEU32 elementalPerkBonusResistance = 35;
static const GEDouble* shootVelocityPtr = &shootVelocity;
static GEFloat animationSpeedBonusMid = 1.3;
static GEFloat animationSpeedBonusHigh = 1.6;

static char* BONETARGET = "Spine_Spine_3";
static char* powerAimEffectName = "eff_ani_fight_bow_raise_01";
static char* powerAimWarriorEffectName = "eff_ani_fight_bow_raise_warrior_01";
static char* powerAimEliteEffectName = "eff_ani_fight_bow_raise_elite_01";

static bCString blessedBonusString;
static bCString sharpBonusString;
static bCString forgedBonusString;
static bCString wornMalusString;
//