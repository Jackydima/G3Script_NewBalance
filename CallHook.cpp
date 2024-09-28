
#include "CallHook.h"

/**
* Changed the Default Projectile Velocity for NPCs and give them Better Aiming
*/
void Shoot_Velocity ( gCScriptProcessingUnit* p_PSU , Entity* p_self , Entity* p_target, PSProjectile* p_projectile ) {

	if ( *p_self == Entity::GetPlayer ( ) )
		return;

	Entity projectileItem = p_self->Inventory.GetItemFromSlot ( gESlot_RightHand );
	p_projectile->AccessProperty<PSProjectile::PropertyShootVelocity> ( ) = shootVelocity;

	if ( false ) // Config later
		return;

	GEDouble time = projectileItem.GetDistanceTo(*p_target) / (shootVelocity * PROJECTILEMULTIPLICATOR );
	eCVisualAnimation_PS* targetAnimation = ( eCVisualAnimation_PS* )(p_target->Animation.m_pEngineEntityPropertySet);
	bCString actor = targetAnimation->GetActor ( )->GetActorName ( );
	actor.GetWord ( 1 , "_" , actor , GETrue , GETrue );
	if ( actor.CompareFast ( "Boar" ) )
		actor = bCString ( "Keiler" );
	if ( actor.CompareFast ( "Alligator" ) )
		actor = bCString ( "Aligator" );
	bCString spineName = actor + "_Spine_Spine_3";
	bCMatrix targetBoneMatrix;
	GEBool gotBoneMatrix = targetAnimation->GetBoneMatrix ( spineName , targetBoneMatrix );
	if ( !gotBoneMatrix ) {
		spineName = actor + "_Spine_Spine_1";
		gotBoneMatrix = targetAnimation->GetBoneMatrix ( spineName , targetBoneMatrix );
	}

	if ( !gotBoneMatrix )
		return;

	bCVector targetVecPos = targetBoneMatrix.AccessTranslation ( );
	bCVector targetVec = targetVecPos - projectileItem.GetPosition ( );
	
	// Absolute Randomness of shots
	targetVecPos.AccessX ( ) += ( Entity::GetRandomNumber ( 200*NPC_AIM_INACCURACY ) - 100* NPC_AIM_INACCURACY );
	//targetVec.AccessY ( ) += ( Entity::GetRandomNumber ( 50 ) - 25 );
	targetVecPos.AccessZ ( ) += ( Entity::GetRandomNumber ( 200*NPC_AIM_INACCURACY ) - 100 * NPC_AIM_INACCURACY );

	bCVector newTargetDirectionVec = ( targetVecPos + ( p_target->GetGameEntity ( )->GetLinearVelocity ( ) * time ) ) - projectileItem.GetPosition ( );

	// Speed relative Randomness of shots
	GEFloat xDiff = newTargetDirectionVec.GetX ( ) - targetVec.GetX();
	GEFloat zDiff = newTargetDirectionVec.GetZ ( ) - targetVec.GetZ ( );
	newTargetDirectionVec.AccessX ( ) += ( Entity::GetRandomNumber ( static_cast< GEInt >( xDiff * NPC_AIM_INACCURACY * 2 ) ) - xDiff * NPC_AIM_INACCURACY );
	newTargetDirectionVec.AccessZ ( ) += ( Entity::GetRandomNumber ( static_cast< GEInt >( zDiff * NPC_AIM_INACCURACY * 2 ) ) - zDiff * NPC_AIM_INACCURACY );

	p_projectile->AccessProperty < PSProjectile::PropertyPathStyle> ( ) = gEProjectilePath::gEProjectilePath_Line;
	p_projectile->SetTarget ( None );
	p_projectile->AccessProperty<PSProjectile::PropertyTargetDirection> ( ) = newTargetDirectionVec.GetNormalized();
}

void CombatMoveScale ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, bCVector* vec) {
	Entity Self = p_PSU->GetSelfEntity ( );
	//std::cout << "Name: " << Self.GetName ( ) << "\nRoutine: " << Self.Routine.GetProperty<PSRoutine::PropertyAction> ( ) << "\n";
	switch ( Self.Routine.GetProperty<PSRoutine::PropertyAction> ( ) ) {
	case gEAction_Stumble:
	case gEAction_StumbleL:
	case gEAction_StumbleR:
	case gEAction_QuickParadeStumble:
	case gEAction_HeavyParadeStumble:
	case gEAction_QuickStumble:
	case gEAction_SitKnockDown:
	case gEAction_ParadeStumble:
	case gEAction_ParadeStumbleL:
	case gEAction_ParadeStumbleR:
	case gEAction_PierceStumble:
		vec->Scale ( 0.5 );
		break;
	case gEAction_LieKnockDown:
	case gEAction_LieKnockOut:
	case gEAction_FinishingAttack:
		break;
	case gEAction_HackAttack:
		if ( Self.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Orc )
			vec->Scale ( 2.17 * ATTACK_REACH_MULTIPLIER );
		else 
			vec->Scale ( 1.2 * ATTACK_REACH_MULTIPLIER );
		break;
	case gEAction_QuickAttack:
	case gEAction_QuickAttackR:
	case gEAction_QuickAttackL:
		vec->Scale ( 0.85 * ATTACK_REACH_MULTIPLIER );
		break;
	case gEAction_PowerAttack:
		vec->Scale ( 1.2 * ATTACK_REACH_MULTIPLIER );
		break;
	default:
		vec->Scale ( ATTACK_REACH_MULTIPLIER );
	}
}

/**
* Adjusted Bow Effect on Aiming for Player
*/
void PS_Ranged_PowerAim ( void* p_Ptr , gCScriptProcessingUnit* p_PSU, void* esp ) {
	Entity player = p_PSU->GetSelfEntity ( );
	void* address = &powerAimEffectName;
	if ( player.Inventory.IsSkillActive ( "Perk_Bow_3" ) ) {
		address = &powerAimEliteEffectName;
	}
	else if ( player.Inventory.IsSkillActive ( "Perk_Bow_2" ) ) {
		address = &powerAimWarriorEffectName;
	}
	DWORD currProt , newProt;
	VirtualProtect ( ( LPVOID )esp , sizeof ( void* ) , PAGE_EXECUTE_READWRITE , &currProt );
	memset ( ( LPVOID )esp , 0x90 , sizeof ( void* ) );
	memcpy ( ( LPVOID )esp , address , sizeof ( void* ) );
	VirtualProtect ( ( LPVOID )esp , sizeof ( void* ) , currProt , &newProt );
}

void ZS_Ranged_PowerAim ( void* p_Ptr , gCScriptProcessingUnit* p_PSU , void* esp ) {
	Entity Self = p_PSU->GetSelfEntity ( );
	void* address = &powerAimEffectName;

	if ( getPowerLevel ( Self ) >= 40 ) {
		address = &powerAimEliteEffectName;
	}
	else if ( getPowerLevel ( Self ) >= 30 ) {
		address = &powerAimWarriorEffectName;
	}
	DWORD currProt , newProt;
	VirtualProtect ( ( LPVOID )esp , sizeof ( void* ) , PAGE_EXECUTE_READWRITE , &currProt );
	memset ( ( LPVOID )esp , 0x90 , sizeof ( void* ) );
	memcpy ( ( LPVOID )esp , address , sizeof ( void* ) );
	VirtualProtect ( ( LPVOID )esp , sizeof ( void* ) , currProt , &newProt );
}


void AssureProjectiles ( GEInt registerBaseStack ) {
	Entity* self = ( Entity* )( registerBaseStack - 0x2A0 );
	//std::cout << "Self: " << self->GetName() << "\n";
	if ( *self == None ) {
		//std::cout << "Unlucky" << "\n";
		return;
	}
	GEInt random = Entity::GetRandomNumber ( 10 );
	GEInt leftHandWeaponIndex = self->Inventory.FindStackIndex ( gESlot_LeftHand );
	Hook_AssureProjectiles.SetImmEbx<GEInt> ( leftHandWeaponIndex );
	gEUseType leftHandUseType = self->Inventory.GetUseType ( leftHandWeaponIndex );
	Template projectile = getProjectile ( *self , leftHandUseType );
	GEInt stack = self->Inventory.AssureItems ( projectile , 0 , random + 10 );
	*( GEInt* )( registerBaseStack - 0x2C4 ) = stack;
}