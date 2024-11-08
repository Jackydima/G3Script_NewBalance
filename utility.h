#pragma once
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"
#include <map>
#include "util/Util.h"
#include "SharedConfig.h"

enum VulnerabilityStatus {
	VulnerabilityStatus_IMMUNE = 0,
	VulnerabilityStatus_REGULAR = 1,
	VulnerabilityStatus_WEAK = 2,
	VulnerabilityStatus_STRONG = 3,
	VulnerabilityStatus_SLIGHTLYWEAK = 4,
	VulnerabilityStatus_SLIGHTLYSTRONG = 5,
};

enum WarriorType {
	WarriorType_None = 0,
	WarriorType_Novice = 1,
	WarriorType_Warrior = 2,
	WarriorType_Elite = 3,
};


std::vector<bCString> splitTobCStrings ( const std::string str , char delim );
GEInt getPowerLevel ( Entity& p_entity );
gEWeaponCategory GetHeldWeaponCategoryNB ( Entity const& a_Entity );
Template getProjectile ( Entity& p_entity , gEUseType p_rangedWeaponType );
GEBool isBigMonster ( Entity& p_monster );
GEInt getMonsterHyperArmorPoints ( Entity& p_monster , gEAction p_monsterAction );
GEInt IsEvil ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );
GEInt CanBurn ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );
GEInt CanFreeze ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );
GEBool IsNormalProjectileNB ( Entity& p_damager );
GEBool IsSpellContainerNB ( Entity& p_damager );
GEBool IsMagicProjectileNB ( Entity& p_damager );
GEBool CheckHandUseTypesNB ( gEUseType p_lHand , gEUseType p_rHand , Entity& entity );
GEInt GetSkillLevelsNB ( Entity& p_entity );
GEInt GetActionWeaponLevelNB ( Entity& p_damager , gEAction p_action );
GEInt GetShieldLevelBonusNB ( Entity& p_entity );
VulnerabilityStatus DamageTypeEntityTestNB ( Entity& p_victim , Entity& p_damager );
GEInt GetHyperActionBonus ( gEAction p_action );
GEU32 GetPoisonDamage ( Entity& attacker );
GEInt getWeaponLevelNB ( Entity& p_entity );
GEBool IsHoldingTwoHandedWeapon ( Entity& entity );
GEBool IsInSameParty ( Entity& p_self , Entity& p_other );
GEBool IsPlayerInCombat ( );
GEBool IsInRecovery ( Entity& p_entity );
GEInt speciesLeftHand ( Entity p_entity );
GEInt speciesRightHand ( Entity p_entity );
WarriorType GetWarriorType ( Entity& p_entity );
GEU32 getLastTimeFromMap ( bCString iD , std::map<bCString , GEU32>& map );