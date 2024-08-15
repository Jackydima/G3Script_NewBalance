
#include "FunctionHook.h"

GEFloat GetAnimationSpeedModifier ( Entity entity , GEU32 u32 ) {
	gESpecies species = entity.NPC.GetProperty<PSNpc::PropertySpecies> ( );
	gEAction action = entity.Routine.GetProperty<PSRoutine::PropertyAction> ( );
	GEInt staminaPoints = GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , &entity , &None , 0 );
	GEBool isArenaNPC = entity != Entity::GetPlayer ( ) && entity.NPC.GetProperty<PSNpc::PropertyAttackReason> ( ) == gEAttackReason_Arena;
	GEFloat retVal = 1.0;
	GEFloat multiPlier = 1.0;
	
	// New Perfect Block Vulnerability
	auto damageReceiver = static_cast< gCDamageReceiver_PS_Ext* >( entity.GetGameEntity ( )->GetPropertySet ( eEPropertySetType_DamageReceiver ) );
	if ( damageReceiver->GetVulnerableState () == 1 && entity.Routine.GetCurrentTask ( ).Contains ( "Stumble" ) ) {
		damageReceiver->AccessVulnerableState() = 2;
		return 0.2;
	}
	else if ( damageReceiver->GetVulnerableState ( ) == 2 ) {
		damageReceiver->AccessVulnerableState ( ) = 0;
	}

	if ( staminaPoints <= 20 ) {
		if ( isArenaNPC )
			multiPlier = 0.9;
		else 
			multiPlier = 0.8;
	}
	else if ( staminaPoints <= 50 ) {
		if ( isArenaNPC )
			multiPlier = 0.95;
		else 
			multiPlier = 0.9;
	}

	if ( action == gEAction_SprintAttack )
		return 1.5;
	if ( species == gESpecies_Troll ) {
		if ( action == gEAction_PowerAttack )
			return 1.3;
		return 1;
	}
	if ( entity.NPC.IsDiseased ( ) ) 
		multiPlier *= 0.9;

	if ( isArenaNPC )
		multiPlier *= 1.1; // default 1.25
	switch ( action ) {
	case gEAction_FinishingAttack:
	case gEAction_SitKnockDown:
	case gEAction_LieKnockDown:
	case gEAction_LieKnockOut:
	case gEAction_LieDead:
	case gEAction_LiePiercedKO:
	case gEAction_LiePiercedDead:
		return 1;
	case gEAction_Aim:
	case gEAction_Reload:
		if ( entity == Entity::GetPlayer ( ) ) {
			if ( entity.Inventory.IsSkillActive ( "Perk_Bow_3" ) ) {
				return animationSpeedBonusHigh;
			}
			if ( entity.Inventory.IsSkillActive ( "Perk_Bow_2" ) ) {
				return animationSpeedBonusMid;
			}
		}
		else {
			if ( getPowerLevel ( entity ) >= 40 )
				return animationSpeedBonusHigh;
			if ( getPowerLevel ( entity ) >= 30 )
				return animationSpeedBonusMid;
		}
		return 1;
	case gEAction_Cast:
	case gEAction_PowerCast:
		if ( entity.GetName ( ).Contains ( "Xardas" ) )
			return 1.3;
		return 1;
	case gEAction_Attack:
		if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_1H , entity ) )
			return 0.6 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_Shield , gEUseType_1H , entity ) )
			return 0.6 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_Torch , gEUseType_1H , entity ) )
			return 0.6 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , entity ) )
			return 0.6 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_2H , entity ) )
			return 0.7 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_Axe , entity ) )
			return 0.7 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_Staff , entity ) )
			return 0.7 * multiPlier;
		if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_Halberd , entity ) )
			return 0.7 * multiPlier;
		return 1 * multiPlier;
	case gEAction_PowerAttack:
		if ( species == gESpecies_Orc && u32 == 0 )
			return 1.3 * multiPlier; //orcs
		if ( u32 == 0 ) {
			return 1.5 * multiPlier;
		}
		if ( ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , entity ) ))
			return 0.9 * multiPlier;
	case gEAction_QuickAttackR:
	case gEAction_QuickAttackL:
	case gEAction_PierceAttack:
	case gEAction_HackAttack:
	case gEAction_PierceStumble:
		return 1 * multiPlier;
	case gEAction_WhirlAttack:
		return 0.9;
	case gEAction_SimpleWhirl:
		return 1.3 * multiPlier;
	case gEAction_Parade:
	case gEAction_ParadeR:
	case gEAction_ParadeL:
		return 2.5 * multiPlier; // default 2.3
	case gEAction_ParadeStumble:
	case gEAction_ParadeStumbleR:
	case gEAction_ParadeStumbleL:
		return 2.0 * multiPlier;
	case gEAction_HeavyParadeStumble:
		if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , entity ) )
			return 0.7 * multiPlier;
		return 1 * multiPlier;
	case gEAction_QuickStumble:
		return 2 * multiPlier;
	case gEAction_Stumble:
	case gEAction_StumbleR:
	case gEAction_StumbleL:
		return 1.3 * multiPlier;
	case gEAction_Summon:
		return 1.5 * multiPlier;
	case gEAction_FlameSword:
		return 0.7 * multiPlier;

	case gEAction_Cock:
		if ( entity == Entity::GetPlayer ( ) ) {
			if ( entity.Inventory.IsSkillActive ( "Perk_Crossbow_3" ) )
				return 2.0 * multiPlier;
			if ( entity.Inventory.IsSkillActive ( "Perk_Crossbow_2" ) )
				return 1.5 * multiPlier;
			return 1.0 * multiPlier;
		}

		if ( getPowerLevel ( entity ) >= 35 ) {
			return 2.0 * multiPlier;
		}
		if ( getPowerLevel ( entity ) > 20 ) {
			return 1.5 * multiPlier;
		}
		return 1 * multiPlier;

	default:
		return 1 * multiPlier;
	}

}

GEInt OnPowerAim_Loop ( gCScriptProcessingUnit* p_PSU ) {
	Entity Self = p_PSU->GetSelfEntity ( );
	Entity Other = p_PSU->GetOtherEntity ( );

	Entity focusEntity = Self.Focus.GetFocusEntity ( (gECombatMode)3,gEDirection_None );
	Self.Focus.SetFocusEntity ( focusEntity );

	Entity item = Self.Inventory.GetItemFromSlot ( gESlot_RightHand ); 

	GEFloat stateTime = Self.Routine.GetStateTime ( );

	/**
	* Here the regular Bow Animation has 50 Frames: The Statetime for 100% is 1.95 Seconds
	*/
	GEFloat hitMultiplier = stateTime / 1.95;
	if ( Self == Entity::GetPlayer ( ) ) {
		if ( Self.Inventory.IsSkillActive ( "Perk_Bow_3" ) )
			hitMultiplier *= animationSpeedBonusHigh;
		else if ( Self.Inventory.IsSkillActive ( "Perk_Bow_2" ) )
			hitMultiplier *= animationSpeedBonusMid;
	}
	else {
		if ( getPowerLevel ( Self ) >= 40 )
			hitMultiplier *= animationSpeedBonusHigh;
		else if ( getPowerLevel ( Self ) >= 30 )
			hitMultiplier *= animationSpeedBonusMid;
	}
	if ( hitMultiplier < 0.7 )
		hitMultiplier /= 2;
	if ( hitMultiplier > 1 )
		hitMultiplier = 1;

	item.Damage.AccessProperty<PSDamage::PropertyDamageHitMultiplier> ( ) = hitMultiplier;

	GEInt* flagForPuttingWeaponBack = (GEInt*)(RVA_ScriptGame ( 0x118d58 ));
	
	if ( *flagForPuttingWeaponBack ) {
		gEAction action = (gEAction)**( gEAction** )( RVA_ScriptGame ( 0x118d54 ) );
		typedef void (*Func1)( Entity );
		typedef void (*Func2)( GEInt, BYTE, Entity);
		Func1 func1 = ( Func1 )RVA_ScriptGame ( 0x79a0 );
		Func2 func2 = ( Func2 )RVA_ScriptGame ( 0x2e50 );
		if ( action == gEAction_AbortAttack ) {
			func1 ( Self );
			func2 ( -1 , -1 , Self );
			Self.Routine.FullStop ( );
			Self.Routine.SetTask ( "PS_Ranged_Reload" );
			Self.Routine.SetState ( "PS_Ranged_Reload" );
			return 1;
		}
		
		if ( action == gEAction_Shoot ) {
			func1 ( Self );
			Self.Routine.FullStop ( );
			Self.Routine.SetState ( "PS_Ranged_Shoot" );
		}
	}
	return 1;
}


static std::map<bCString , GEU32> LastHealthDamageMap = {};
static GEInt healthRecoveryDelay = 60;

static GEU32 getLastHealthDamageTime ( bCString iD ) {
	GEU32 worldTime = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
	GEU32 retVal = 0;
	for ( auto it = LastHealthDamageMap.cbegin ( ); it != LastHealthDamageMap.cend ( ); ) {
		if ( worldTime - it->second > 400 )
			LastHealthDamageMap.erase ( it++ );
		else
			++it;
	}
	try {
		retVal = worldTime - LastHealthDamageMap.at ( iD );
	}
	catch ( std::exception e ) {
		retVal = ULONG_MAX;
	}
	return retVal;
}

GEInt HealthUpdateOnTickHelper ( Entity& p_entity , GEInt p_healthValue ) {
	//std::cout << "HealthInc: " << p_healthValue << "\t Last Time: " << getLastHealthDamageTime ( p_entity.GetGameEntity ( )->GetID ( ).GetText ( ) ) << "\n";
	if ( p_healthValue > 0 && getLastHealthDamageTime ( p_entity.GetGameEntity ( )->GetID ( ).GetText ( ) ) < healthRecoveryDelay )
		return 0;
	return p_healthValue;
}

void ResetHitPointsRegen ( Entity& p_entity ) {
	LastHealthDamageMap[p_entity.GetGameEntity ( )->GetID ( ).GetText ( )] = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
}

static mCFunctionHook Hook_AddHitPoints;
GEInt GE_STDCALL AddHitPoints ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs ) {
	INIT_SCRIPT_EXT ( Self , Other );
	if ( a_iArgs < 0 ) {
		LastHealthDamageMap[Self.GetGameEntity ( )->GetID ( ).GetText ( )] = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
	}
	return Hook_AddHitPoints.GetOriginalFunction ( &AddHitPoints )( a_pSPU , a_pSelfEntity , a_pOtherEntity , a_iArgs );
}

GEInt UpdateHitPointsOnTick ( Entity p_entity ) {
	GEInt retVal = 0;
	gEAIMode aiMode = p_entity.Routine.GetProperty<PSRoutine::PropertyAIMode> ( );
	GEBool aBActive = eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing;
	if ( aiMode == gEAIMode_Dead ) {
		return 0;
	}

	//std::cout << "Entity: "<< p_entity.GetName() << " --- CombatState: " << p_entity.NPC.GetProperty<PSNpc::PropertyCombatState> ( ) << "\n";

	if ( p_entity.NPC.IsBurning ( ) )
		retVal -= 5;

	if ( p_entity.NPC.IsPoisoned ( ) ) {
		gCDamageReceiver_PS_Ext* damageReceiver = static_cast< gCDamageReceiver_PS_Ext* >( p_entity.GetGameEntity ( )->GetPropertySet ( eEPropertySetType_DamageReceiver ) );
		GEU32 poisonDamage = damageReceiver->GetPoisonDamage ( );
		if ( poisonDamage <= 0 )
			poisonDamage = 5;
		retVal -= poisonDamage;
	}

	if ( p_entity.NPC.IsInMagicBarrier ( ) )
		retVal -= 10;

	if (retVal < 0 )
		return HealthUpdateOnTickHelper ( p_entity , retVal );

	if ( p_entity.IsPlayer ( ) && p_entity.Inventory.IsSkillActive ( "Perk_MasterGladiator" )) {
		GEInt maxHealth = p_entity.PlayerMemory.GetHitPointsMax ( );
		retVal += static_cast< GEInt >( maxHealth * 0.01 );
	}
	else if ( !p_entity.IsPlayer ( ) && getPowerLevel ( p_entity ) >= 50 ) {
		GEInt maxHealth = p_entity.DamageReceiver.GetProperty<PSDamageReceiver::PropertyHitPointsMax> ( );
		retVal += static_cast< GEInt >( maxHealth * 0.01 );
	}
	
	if ( retVal > 0 ) {
		if ( aiMode == gEAIMode_Combat )
			retVal = static_cast< GEInt >( retVal * 0.5 );
		if ( retVal < 3 ) {
			retVal = 3;
		}
	}

	gEAniState aniState = p_entity.Routine.GetProperty<PSRoutine::PropertyAniState> ( );
	switch ( aniState ) {
	case gEAniState_SitThrone:
	case gEAniState_SitGround:
	case gEAniState_SitStool:
	case gEAniState_SitBench:
	case gEAniState_SleepGround:
	case gEAniState_SleepBed:
		retVal += 5;
		break;
	case gEAniState_SitKnockDown:
	case gEAniState_LieKnockDown:
		return 0;
	}
	return HealthUpdateOnTickHelper(p_entity, retVal);
}


GEInt GE_STDCALL CanParade ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs )
{
	INIT_SCRIPT_EXT ( Victim , Damager );
	Entity DamagerOwner = Damager.Interaction.GetOwner ( );
	if ( DamagerOwner == None && Damager.Navigation.IsValid ( ) )
	{
		DamagerOwner = Damager;
	}

	GEBool canParadeMoveOf = GetScriptAdmin ( ).CallScriptFromScript ( "CanParadeMoveOf" , &Victim , &Damager , 0 );
	GEBool isMonsterDamager = !GetScriptAdmin ( ).CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 );
	GEBool victimInParade = GetScriptAdmin ( ).CallScriptFromScript ( "IsInParadeMode" , &Victim , &None , 0 );
	gEAction victimAction = Victim.Routine.GetProperty<PSRoutine::PropertyAction> ( );
	gEAction damagerAction = Damager.Routine.GetProperty<PSRoutine::PropertyAction> ( );

	/*
		Special Request Change
	*/

	gEUseType rightWeaponUseType = Victim.Inventory.GetItemFromSlot ( gESlot_RightHand ).Interaction.GetUseType ( );

	// Special return true for Blocking Hackattacks with a 2H Weapon, Axe, Halbert, Staff
	GEBool victimHolding2HWeap = IsHoldingTwoHandedWeapon ( Victim );
	if ( victimHolding2HWeap && victimInParade && damagerAction == gEAction_HackAttack
		 && victimAction != gEAction_HackAttack && victimAction != gEAction_WhirlAttack && victimAction != gEAction_FinishingAttack ) {
		if ( Victim.IsInFOV ( DamagerOwner ) ) {
			return 1;
		}
	}
	// Special return for Blocking Monsterdamage with a 2H Weapon, Axe, Halbert, Staff and 1H When skilled up
	GEInt weaponLevel = getWeaponLevelNB ( Victim );
	if ( isBigMonster ( DamagerOwner ) ) {
		weaponLevel -= 1;
	}
	if ( victimInParade && victimAction != gEAction_HackAttack && victimAction != gEAction_PierceAttack
		&& victimAction != gEAction_WhirlAttack && victimAction != gEAction_FinishingAttack
		&& isMonsterDamager && (weaponLevel >= 2 || (!isBigMonster(DamagerOwner) && !Damager.GetName().Contains("Fist")) )
		&& ( damagerAction != gEAction_PierceAttack || CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , Victim ) )
		&& ( damagerAction != gEAction_HackAttack || victimHolding2HWeap ) ) {
		if ( Victim.IsInFOV ( DamagerOwner ) ) {
			return 1;
		}
	}

	// 1H1H can block Pierceattacks
	if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , Victim ) && victimInParade
		 && victimAction != gEAction_PierceAttack && victimAction != gEAction_FinishingAttack
		&& damagerAction == gEAction_PierceAttack ) {
		if ( Victim.IsInFOV ( DamagerOwner ) ) {
			return 1;
		}
	}

	// Human Fists are blockable
	if ( victimInParade && GetScriptAdmin ( ).CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 ) && GetScriptAdmin ( ).CallScriptFromScript ( "IsInFistMode" , &DamagerOwner , &None , 0 ) ) {
		if ( Victim.IsInFOV ( DamagerOwner ) ) {
			return 1;
		}
	}

	/*
		Special Request Change End
	*/

	if ( canParadeMoveOf && victimInParade && victimAction != gEAction::gEAction_HackAttack ) {
		if ( !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeAI ||
			( victimAction != gEAction_PierceAttack &&
				victimAction != gEAction_WhirlAttack &&
				victimAction != gEAction_FinishingAttack &&
				damagerAction != gEAction_SprintAttack ) ) {

			if ( Victim.IsInFOV ( DamagerOwner ) ) {
				return 1;
			}
		}
	}
	return 0;
}

static mCFunctionHook Hook_OnTick;
GEInt GE_STDCALL OnTick ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs ) {
	INIT_SCRIPT_EXT ( Self , Other );
	if ( !Self.IsPlayer ( ) && /*Self.NPC.GetProperty<PSNpc::PropertySpecies> ( ) != gESpecies::gESpecies_Zombie
		&&*/ Self.Routine.GetProperty<PSRoutine::PropertyAIMode> ( ) == gEAIMode_Combat ) {
		GEFloat staminaPercantage = Self.DamageReceiver.GetProperty<PSDamageReceiver::PropertyStaminaPoints> ( ) / Self.DamageReceiver.GetProperty<PSDamageReceiver::PropertyStaminaPointsMax> ( );
		if (  Self.GetDistanceTo ( Self.NPC.GetCurrentTarget ( ) ) > 450.0 /* && staminaPercantage >= 0.2*/ )
			Self.SetMovementMode ( gECharMovementMode_Sprint );
		else
			Self.SetMovementMode ( gECharMovementMode_Run );
	}

	if ( Self == Entity::GetPlayer ( ) && IsPlayerInCombat ( ) ) {
		Self.NPC.AccessProperty<PSNpc::PropertyCombatState> ( ) = 1;
		//std::cout << "IsInCombat Suka\n";
	}
	else if ( Self == Entity::GetPlayer ( ) ) {
		Self.NPC.AccessProperty<PSNpc::PropertyCombatState> ( ) = 0;
		//std::cout << "Unluck\n";
	}

	return Hook_OnTick.GetOriginalFunction ( &OnTick )( a_pSPU , a_pSelfEntity , a_pOtherEntity , a_iArgs );
}

static mCFunctionHook Hook_MagicTransform;
GEInt GE_STDCALL MagicTransform ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs ) {
	INIT_SCRIPT_EXT ( Self , Other );

	// TODO: Check for NPCs Transformation
	if ( !Self.IsPlayer ( ) )
		return 1;

	Entity transformedSpawn = Self.Interaction.GetSpell ( ).Magic.GetSpawn ( );
	Template spawn = transformedSpawn.GetTemplate ( );
	bCMatrix pos = Self.GetPose ( );

	Entity spawnedEntity = Entity::Spawn ( spawn , pos );
	if ( !spawnedEntity.FindSpawnPose ( pos , spawnedEntity , GETrue , 0 ) ) {
		EffectSystem::StartEffect ( "CAST_FAIL" , pos , spawnedEntity );
		spawnedEntity.Kill ( );
		return 0;
	}

	spawnedEntity.MoveTo ( pos );

	GEInt transformDuration = GetScriptAdmin ( ).CallScriptFromScript ( "GetTransformDuration" , a_pSelfEntity , a_pOtherEntity , 0 );
	if ( Self.Inventory.GetCategory ( a_iArgs ) == gEWeaponCategory::gEWeaponCategory_Magic ) {
		transformDuration = -1;
	}
	GetScriptAdmin ( ).CallScriptFromScript ( "SendPerceptionTransform" , &Self , &None, 0 );

	Self.StartTransform ( spawnedEntity , transformDuration , GEFalse );
	GEInt leftHand = speciesLeftHand ( spawnedEntity );
	GEInt rightHand = speciesRightHand ( spawnedEntity );

	//spawnedEntity.Inventory.HoldStacks ( leftHand , rightHand );

	//TODO extra Functions
	typedef void ( *Func1 )( GEInt, GEInt, Entity );
	typedef void ( *Func2 )( Entity );
	Func1 func1 = ( Func1 )RVA_ScriptGame ( 0x2e50 );
	Func2 func2 = ( Func2 )RVA_ScriptGame ( 0x7a00 );
	//func1 ( leftHand , rightHand , spawnedEntity );
	func2 ( spawnedEntity );

	//spawnedEntity.Routine.SetTask ( "PS_Melee" );

	return 1;
}


static mCFunctionHook Hook_StartTransform;
void GE_STDCALL StartTransform ( Entity* p_targetEntity , GEFloat p_duration , GEBool p_bool ) {
	Entity Self = *Hook_StartTransform.GetSelf<Entity*> ( );
	std::cout << "Start: " << Self.GetName() << "\n";
	//TODO Check for NPCs
	if ( !Self.IsPlayer ( ) )
		return;
	if ( !p_bool ) {
		Self.GetGameEntity ( )->Enable ( GEFalse );
		Self.EnableCollision ( GEFalse );
		gCEntity* entity = Self.GetGameEntity ( );
		gCDynamicCollisionCircle_PS* dcc = ( gCDynamicCollisionCircle_PS* )entity->GetPropertySet ( eEPropertySetType_DynamicCollisionCircle );
		if ( dcc != 0 ) {
			Self.Navigation.SetDCCEnabled ( GEFalse );
		}
	}

	if ( Self.PlayerMemory.IsValid ( ) ) {
		Self.PlayerMemory.AccessProperty<PSPlayerMemory::PropertySecondsTransformRemain> ( ) = p_duration;
	}

	GEInt playerLevel = Self.NPC.GetProperty<PSNpc::PropertyLevel>();
	GEBool isDruid = Self.Inventory.IsSkillActive ( "Perk_Druid" );
	GEBool isWaterMage = Self.Inventory.IsSkillActive ( "Perk_WaterMage" );
	GEBool hasManaRegen = Self.Inventory.IsSkillActive ( "Perk_MasterMage" );

	/**
	* Áfter TakeOver, The Player Takes over the targetEntity
	* The TargetEntity had no PlayerMemory and after the TakeOver, it gets the PlayerMem
	* with every Attribute of the Player on 100
	* Also the Self Entity (PC_Hero) is no longer the Player in checks
	* The Player can be taken from the Entity::GetPlayer() function, which also 
	* has most of the Entities properties. 
	* PlayerMemory and CharacterControll got added to the new Player Entity
	* Changes to the new Entity will not affect the PC_Hero Entity (default Player)
	*/
	p_targetEntity->StartTakeOver ( );

	// needs to be this. Cant be targetEntity (or Self)
	Entity player = Entity::GetPlayer ( );

	if ( player.Routine.IsValid ( ) )
		player.Routine.ContinueRoutine ( );

	player.NPC.EnableStatusEffects ( gEStatusEffect::gEStatusEffect_Transformed, GETrue );

	GEInt targetLevel = p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( );
	if ( isDruid )
		targetLevel *= 1.5;

	if ( isWaterMage )
		targetLevel += 10;

	GEInt newLevel = ( targetLevel + playerLevel) / 2;

	p_targetEntity->NPC.AccessProperty<PSNpc::PropertyLevel> ( ) = newLevel;
	p_targetEntity->NPC.AccessProperty<PSNpc::PropertyLevelMax> ( ) = targetLevel;
		
	// Stats must be in PlayerMem for some reason ... 
	// Either do it here, or adjust all the Other things
	player.PlayerMemory.SetHitPointsMax(p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) * 20);
	player.PlayerMemory.SetHitPoints(p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) * 20);
	player.PlayerMemory.SetManaPointsMax(p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) * 10);
	player.PlayerMemory.SetManaPoints (p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) * 10);
	player.PlayerMemory.SetStaminaPointsMax(p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) * 10);
	player.PlayerMemory.SetStaminaPoints (p_targetEntity->NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) * 10);

	if ( hasManaRegen ) {
		p_targetEntity->Inventory.AssureItemsEx ( "It_Perk_MasterMage" , 0 , 1 , -1 , GETrue );
	}
}

static mCFunctionHook Hook_PS_Melee_Attack;
GEInt PS_Melee_Attack ( int* p_ptr , gCScriptProcessingUnit* p_PSU ) {
	Entity Self = p_PSU->GetSelfEntity ( );
	if ( IsInRecovery ( Self ) ) {
		return 0;
	}
	return Hook_PS_Melee_Attack.GetOriginalFunction ( &PS_Melee_Attack )( p_ptr , p_PSU );
}

static mCFunctionHook Hook_PS_Melee_PowerAttack;
GEInt PS_Melee_PowerAttack ( int* p_ptr , gCScriptProcessingUnit* p_PSU ) {
	Entity Self = p_PSU->GetSelfEntity ( );
	if ( IsInRecovery ( Self ) ) {
		return 0;
	}
	return Hook_PS_Melee_PowerAttack.GetOriginalFunction ( &PS_Melee_PowerAttack )( p_ptr , p_PSU );
}

static mCFunctionHook Hook_GetAniName; 
void GetAniName ( bCString* p_retString , eCEntity* p_entity , gEAction p_action , bCString p_actionString , bCString* p_directionString , GEBool p_bool ) {
	gCScriptProcessingUnit* selfPSU = Hook_GetAniName.GetSelf< gCScriptProcessingUnit*> ( );
	Hook_GetAniName.GetOriginalFunction ( &GetAniName )( p_retString , p_entity , p_action , p_actionString , p_directionString , p_bool );
	/*
	std::cout << "\nEntity: " << p_entity->GetName ( )
		<< "\nAction: " << p_action
		<< "\nActionString: " << p_actionString.GetText ( )
		<< "\nDirection: " << p_directionString->GetText ( )
		<< "\nBool: " << p_bool
		<< "\nAniName: " << p_retString->GetText ( )
		<< "\n";*/
	GEInt index = p_retString->Find ( "2H" );
	if ( index != -1 ) {
		p_retString->Replace ( "2H" , "1H" );
		if ( p_retString->Contains ( "FinishingAttack" ) && p_action != gEAction_FinishingAttack ) 
			p_retString->Replace ( "FinishingAttack" , "PierceAttack" );
	}
	
}

void HookFunctions ( ) {
	Hook_AddHitPoints
		.Prepare ( RVA_ScriptGame ( 0x35b50 ) , &AddHitPoints )
		.Hook ( );
	Hook_OnTick
		.Prepare ( RVA_ScriptGame ( 0xb0ef0 ), &OnTick )
		.Hook ( );

	Hook_MagicTransform.Hook ( GetScriptAdminExt ( ).GetScript ( "MagicTransform" )->m_funcScript, &MagicTransform , mCBaseHook::mEHookType_OnlyStack );

	static mCFunctionHook Hook_SpeciesRightHand;
	Hook_SpeciesRightHand
		.Prepare ( RVA_ScriptGame ( 0xb200 ) , &speciesRightHand )
		.Hook ( );

	Hook_StartTransform
		.Prepare ( RVA_Script ( 0x1afe0 ) , &StartTransform , mCBaseHook::mEHookType_ThisCall )
		.Hook ( );

	//TODO: Bring that shit on!
	//Hook_PS_Melee_Attack
	//	.Prepare ( RVA_ScriptGame ( 0x7eee0 ) , &PS_Melee_Attack )
	//	.Hook ( );

	//Hook_PS_Melee_PowerAttack
	//	.Prepare ( RVA_ScriptGame ( 0x7f420 ) , &PS_Melee_PowerAttack )
	//	.Hook ( );
	//Hook_GetAniName
	//	.Prepare ( RVA_Game ( 0x16f840 ) , &GetAniName , mCBaseHook::mEHookType_ThisCall )
	//	.Hook ( );
}

/*ME_DEFINE_AND_REGISTER_SCRIPT ( MagicSummonWolfPack )
{
	UNREFERENCED_PARAMETER ( a_iArgs );
	INIT_SCRIPT_EXT ( SelfEntity , Target );

	bCMatrix Pos = Target.GetPose ( );

	auto Spell = Target.Interaction.GetSpell ( );

	for ( GEInt x = 0; x < 5; x++ )
	{
		auto Spawned = Entity::Spawn ( Template ( "IceWolf" ) , Pos );
		bCMatrix NewPos;
		if ( Spawned.FindSpawnPose ( NewPos , Target , true , x + 1 ) )
		{
			Spawned.MoveTo ( NewPos );
			Spawned.Effect.StartEffect ( Spell.Magic.EffectTargetCast , GEFalse );
			Target.Party.Add ( Spawned );
			Spawned.Party.Waiting = GEFalse;
			Spawned.Party.PartyMemberType = gEPartyMemberType::gEPartyMemberType_Summoned;
			Spawned.Routine.FullStop ( );
			Spawned.Routine.SetTask ( "ZS_FollowPlayer" );
			Spawned.Dialog.PartyEnabled = GETrue;
		}
		else
		{
			Spawned.Kill ( );
			Spawned.Decay ( );
		}
	}

	return 0;
}*/