
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

	bCVector targetVec = targetBoneMatrix.AccessTranslation ( );
	// AI-Randomness
	targetVec.AccessX ( ) += ( Entity::GetRandomNumber ( 150 ) - 75 );
	//targetVec.AccessY ( ) += ( Entity::GetRandomNumber ( 50 ) - 25 );
	targetVec.AccessZ ( ) += ( Entity::GetRandomNumber ( 150 ) - 75 );
	bCVector newTargetDirectionVec = ( targetVec + ( p_target->GetGameEntity ( )->GetLinearVelocity ( ) * time ) ) - projectileItem.GetPosition ( );
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
			vec->Scale ( 3.8 );
		else 
			vec->Scale ( 1.75 );
		break;
	default:
		vec->Scale ( 1.75 );
	}
}