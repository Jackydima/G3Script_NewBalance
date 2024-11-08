#pragma once

#include "Script.h"
#include "FunctionHook.h"
#include "CallHook.h"
#include "CodePatch.h"
#include "ge_DamageReceiver_ext.h"
#include "Projectile_PS.h"
#include "ge_effectmap.h"
#include "MyGUI.h"
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"
#include "utility.h"
#include "SharedConfig.h"

gSScriptInit & GetScriptInit();

GEInt CanFreezeAddition ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs );

static GEU32 getPerfectBlockLastTime ( bCString iD);

GEInt StaminaUpdateOnTickHelper ( Entity& p_entity , GEInt p_staminaValue );

void AddNewEffect ( );