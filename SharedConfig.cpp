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

GEInt KnockDownThreshold = 5;
GEInt MonsterRageModus = 0;

GEFloat summoningLevelMultiplier = 7.5f;

GEBool newSummoning = GETrue;
GEBool vanishSummons = GETrue;
GEBool onlyHeaveAttackKnockDown = GEFalse;
GEBool useHardCoreAttacks = GETrue;
GEBool useExtendedBlocking = GEFalse;
GEBool useDamagingInnosLight = GETrue;
GEBool useNewBalanceMagicWeapon = GEFalse;
GEBool useNewStaminaRecovery = GETrue;
GEBool useAlwaysMaxLevel = GETrue;
GEBool enablePerfectBlock = GETrue;
GEBool playerOnlyPerfectBlock = GEFalse;
GEBool useNewBalanceMeleeScaling = GETrue;
GEBool adjustXPReceive = GETrue;
GEBool useStaticBlocks = GEFalse;
GEBool useStrengthForCrossbows = GEFalse;
GEBool alternativeProtection = GEFalse;
GEBool useNewBowMechanics = GETrue;
GEBool enableNPCSprint = GETrue;
GEBool zombiesCanSprint = GEFalse;
GEBool enableNewTransformation = GETrue;
GEBool disableMonsterRage = GETrue;
GEBool enableNewMagicAiming = GETrue;
GEBool enableAOEDamage = GETrue;
std::vector<bCString> AOENames = {};

GEFloat PerfectBlockDamageMult = 1.6f;
GEFloat PowerAttackArmorPen = 0.1f;
GEFloat QuickAttackArmorRes = 0.05f;
GEFloat SpecialAttackArmorPen = 0.125f;
GEFloat NPCStrengthMultiplicator = 1.8f;
GEFloat NPCStrengthCorrection = -50.0f;
GEFloat NPCDamageReductionMultiplicator = 0.5f;
GEInt poiseThreshold = -2;
GEFloat npcArenaSpeedMultiplier = 1.1;
GEFloat npcWeaponDamageMultiplier = 0.15;
GEInt staminaRecoveryDelay = 20;
GEInt staminaRecoveryPerTick = 13;
GEFloat fMonsterDamageMultiplicator = 0.5; // Default 0.5
GEDouble npcArmorMultiplier = 1.5;
GEDouble playerArmorMultiplier = 0.4;
GEInt startSTR = 0;
GEInt startDEX = 0;
GEDouble attackRangeAI = 3000.0;
GEDouble telekinesisRange = 4000.0;
GEDouble shootVelocity = 300.0;
GEDouble NPC_AIM_INACCURACY = 0.40;
GEDouble ATTACK_REACH_MULTIPLIER = 1.75;
GEInt elementalPerkBonusResistance = 35;
GEFloat animationSpeedBonusMid = 1.5f;
GEFloat animationSpeedBonusHigh = 2.0f;

GEInt blessedBonus = 10;
GEInt sharpBonus = 10;
GEBool useSharpPercentage = GETrue;
GEInt forgedBonus = 20;
GEInt wornPercentageMalus = 70;