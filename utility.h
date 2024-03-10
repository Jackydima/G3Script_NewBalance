#pragma once
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"
#include <map>

enum VulnerabilityStatus {
	VulnerabilityStatus_IMMUNE = 0,
	VulnerabilityStatus_REGULAR = 1,
	VulnerabilityStatus_WEAK = 2,
	VulnerabilityStatus_STRONG = 3,
};
Template getProjectile ( Entity& p_entity , gEUseType p_rangedWeaponType );
GEBool isBigMonster ( Entity& p_monster );
GEInt CanBurn ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );
GEInt CanFreeze ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );
GEBool IsNormalProjectile ( Entity& p_damager );
GEBool IsSpellContainer ( Entity& p_damager );
GEBool IsMagicProjectile ( Entity& p_damager );
GEBool CheckHandUseTypes ( gEUseType p_lHand , gEUseType p_rHand , Entity& entity );
GEInt GetSkillLevels ( Entity& p_entity );
GEInt GetActionWeaponLevel ( Entity& p_damager , gEAction p_action );
GEInt GetShieldLevelBonus ( Entity& p_entity );
VulnerabilityStatus DamageTypeEntityTest ( Entity& p_victim , Entity& p_damager );
GEInt GetHyperActionBonus ( gEAction p_action );

