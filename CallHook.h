#pragma once

#include "Script.h"
#include "utility.h"
#include "SharedConfig.h"

void Shoot_Velocity ( gCScriptProcessingUnit* p_PSU , Entity* p_self , Entity* p_target, PSProjectile* p_projectile );

void CombatMoveScale ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, bCVector* vec );

void PS_Ranged_PowerAim ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, void* esp );

void ZS_Ranged_PowerAim ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, void* esp );

void AssureProjectiles ( GEInt registerBaseStack );

void GiveXPPowerlevel ( gCNPC_PS* p_npc );

void HookCallHooks ( );
