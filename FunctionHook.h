#pragma once
#include "Script.h"
#include "utility.h"
//#include "util/ScriptUtil.h"
#include "ge_DamageReceiver_ext.h"
#include "SharedConfig.h"

GEFloat GetAnimationSpeedModifier ( Entity entity, GEU32 u32 );

GEInt OnPowerAim_Loop ( gCScriptProcessingUnit* p_PSU );

GEInt UpdateHitPointsOnTick ( Entity p_entity );

GEInt GE_STDCALL CanParade ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );

GEInt GE_STDCALL AddHitPoints ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

GEInt GE_STDCALL OnTick ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

GEInt GE_STDCALL MagicTransform ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

GEInt GE_STDCALL MagicSleep ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

GEInt GE_STDCALL GetProtection ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

GEInt GE_STDCALL GetProtectionHUD ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

GEInt GE_STDCALL GetCurrentLevel ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs );

void ResetHitPointsRegen (Entity& p_entity );

void HookFunctions ( );