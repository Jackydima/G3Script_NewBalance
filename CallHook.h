#pragma once

#include "Script.h"
#include "utility.h"
#include "SharedConfig.h"

static mCCallHook Hook_Shoot_Velocity;
void Shoot_Velocity ( gCScriptProcessingUnit* p_PSU , Entity* p_self , Entity* p_target, PSProjectile* p_projectile );

static mCCallHook Hook_CombatMoveScale;
void CombatMoveScale ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, bCVector* vec );

static mCCallHook Hook_PS_Ranged_PowerAim;
void PS_Ranged_PowerAim ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, void* esp );

static mCCallHook Hook_ZS_Ranged_PowerAim;
void ZS_Ranged_PowerAim ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, void* esp );

static mCCallHook Hook_AssureProjectiles;
void AssureProjectiles ( GEInt registerBaseStack );
