#pragma once

#include "Script.h"

gSScriptInit & GetScriptInit();

GEInt CanFreezeAddition ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );

static GEU32 getPerfectBlockLastTime ( bCString iD);

GEInt StaminaUpdateOnTickHelper ( Entity& p_entity , GEInt p_staminaValue );