#pragma once
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"

enum VulnerabilityStatus {
	VulnerabilityStatus_IMMUNE ,
	VulnerabilityStatus_REGULAR ,
	VulnerabilityStatus_WEAK ,
	VulnerabilityStatus_STRONG ,
};

GEBool isBigMonster ( Entity& p_monster );
GEBool CanBurn ( Entity& p_Self , Entity& p_other );
GEBool CanFreeze ( Entity& p_victim , Entity& p_damager );
GEBool IsNormalProjectile ( Entity& p_damager );
GEBool IsSpellContainer ( Entity& p_damager );
GEBool IsMagicProjectile ( Entity& p_damager );
GEBool CheckHandUseTypes ( gEUseType p_lHand , gEUseType p_rHand , Entity& entity );
GEInt GetSkillLevels ( Entity& p_entity );
GEInt GetActionWeaponLevel ( Entity& p_damager , gEAction p_action );
GEInt GetShieldLevelBonus ( Entity& p_entity );
VulnerabilityStatus DamageTypeEntityTest ( Entity& p_victim , Entity& p_damager );
GEInt GetHyperActionBonus ( gEAction p_action );

