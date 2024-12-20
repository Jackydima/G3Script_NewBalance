#include "SharedConfig.h"


//TODO 
GEFloat npcSTRMultiplier = 1.5;
GEInt npcSTRAddition = 15;

//
// 
// Configureable

GEU32 bossLevel = 65;
GEU32 uniqueLevel = 45;
GEU32 eliteLevel = 35;
GEU32 warriorLevel = 30;
GEU32 noviceLevel = 20;

GEBool useNewBalanceMagicWeapon = GEFalse;
GEBool useNewStaminaRecovery = GETrue;
GEBool useAlwaysMaxLevel = GETrue;
GEBool enablePerfectBlock = GETrue;
GEBool playerOnlyPerfectBlock = GEFalse;
GEBool useNewBalanceMeleeScaling = GETrue;
GEBool adjustXPReceive = GETrue;
GEBool useNewBowMechanics = GETrue;
GEBool enableNPCSprint = GETrue;
GEBool zombiesCanSprint = GEFalse;
GEBool enableNewTransformation = GETrue;
GEBool disableMonsterRage = GETrue;

GEFloat PerfectBlockDamageMult = 2.0f;
GEFloat PowerAttackArmorPen = 0.1f;
GEFloat QuickAttackArmorRes = 0.05f;
GEFloat SpecialAttackArmorPen = 0.125f;
GEFloat NPCStrengthMultiplicator = 1.8f;
GEFloat NPCStrengthCorrection = -50.0f;
GEFloat NPCDamageReductionMultiplicator = 0.5f;
GEInt poiseThreshold = -2;
GEFloat npcArenaSpeedMultiplier = 1.1;
GEFloat npcWeaponDamageMultiplier = 0.15;
GEU32 staminaRecoveryDelay = 20;
GEU32 staminaRecoveryPerTick = 13;
GEFloat fMonsterDamageMultiplicator = 0.5; // Default 0.5
GEDouble npcArmorMultiplier = 1.5;
GEDouble playerArmorMultiplier = 0.4;
GEU32 startSTR = 100;
GEU32 startDEX = 100;
GEDouble shootVelocity = 300;
GEDouble NPC_AIM_INACCURACY = 0.40;
GEDouble ATTACK_REACH_MULTIPLIER = 1.75;

GEU32 blessedBonus = 10;
GEU32 sharpBonus = 10;
GEBool useSharpPercentage = GETrue;
GEU32 forgedBonus = 20;
GEU32 wornPercentageMalus = 70;