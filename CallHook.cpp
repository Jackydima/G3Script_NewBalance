
#include "CallHook.h"

void Shoot_Velocity ( gCScriptProcessingUnit* p_PSU , Entity* p_self , Entity* p_target, PSProjectile* p_projectile ) {
	Entity projectile = p_self->Inventory.GetItemFromSlot ( gESlot_RightHand );
	//std::cout << "Name: " << projectile.GetName ( ) << "\n";

	/**
	* Changed the Default Projectile Velocity for NPCs
	*/
	p_projectile->AccessProperty<PSProjectile::PropertyShootVelocity> ( ) = shootVelocity;

	//std::cout << "Velocity: " << projectile.Projectile.GetProperty<PSProjectile::PropertyShootVelocity> ( ) << "\n";
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
	case gEAction_LieKnockDown:
	case gEAction_LieKnockOut:
	case gEAction_ParadeStumble:
	case gEAction_ParadeStumbleL:
	case gEAction_ParadeStumbleR:
	case gEAction_PierceStumble:
		vec->Scale ( 0.5 );
		break;
	case gEAction_FinishingAttack:
		break;
	case gEAction_HackAttack:
		if ( Self.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Orc )
			vec->Scale ( 3.8 );
		else 
			vec->Scale ( 1.7 );
		break;
	default:
		vec->Scale ( 1.7 );
	}
}