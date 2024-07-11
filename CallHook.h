#pragma once

#include "Script.h"
#include "utility.h"

static const GEDouble shootVelocity = 400;
static const GEDouble* shootVelocityPtr = &shootVelocity;

static mCCallHook Hook_Shoot_Velocity;
void Shoot_Velocity ( gCScriptProcessingUnit* p_PSU , Entity* p_self , Entity* p_target, PSProjectile* p_projectile );

static mCCallHook Hook_CombatMoveScale;
void CombatMoveScale ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, bCVector* vec );
