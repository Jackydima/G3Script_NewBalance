#include "util/ScriptUtil.h"
#include "util/Util.h"

enum gEHitForce
{
    gEHitForce_Minimal = 0 ,
    gEHitForce_Normal = 1 ,
    gEHitForce_Heavy = 2
};

// wird aufgerufen von DoLogicalDamage
gEAction GE_STDCALL AssessHit ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs )
{
    INIT_SCRIPT_EXT ( Victim , Damager );

    gCScriptAdmin& ScriptAdmin = GetScriptAdmin ( );

    //
    // Schritt 1: Basisschaden
    //

    // Damager would be the weapon / projectile and DamagerOwner the attacking player / NPC.
    Entity DamagerOwner = Damager.Interaction.GetOwner ( );
    if ( DamagerOwner == None && Damager.Navigation.IsValid ( ) )
    {
        DamagerOwner = Damager;
    }
    gEAction DamagerOwnerAction = DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( );

    // Update damage and attacker of victim
    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = 0;
    Victim.NPC.SetLastAttacker ( Victim.NPC.GetCurrentAttacker ( ) );
    Victim.NPC.SetCurrentAttacker ( DamagerOwner );

    Entity Player = Entity::GetPlayer ( );

    if ( DamagerOwnerAction == gEAction_SprintAttack || DamagerOwnerAction == gEAction_PowerAttack )
    {
        if ( Victim == Player )
        {
            Victim.NPC.SetLastHitTimestamp ( 0 );
            DamagerOwner.Routine.AccessProperty<PSRoutine::PropertyAction> ( ) = gEAction_PowerAttack;
        }
        else
        {
            DamagerOwnerAction = gEAction_PowerAttack;
        }
    }

    // Calc weapon damage (WAF-SCHD)
    GEI32 iDamageAmount = Damager.Damage.GetProperty<PSDamage::PropertyDamageAmount> ( );
    GEFloat fDamageMultiplier = Damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( );
    const GEInt iWeaponDamage = static_cast< GEInt >( fDamageMultiplier * iDamageAmount );

    // Handle a few special cases if the player is the damager.
    if ( Player == DamagerOwner )
    {
        // Player uses a normal projectile and the victim is humanoid (prerequisite for the Headbutt special case)
        if ( IsNormalProjectile ( Damager ) && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
        {
            // Projectile is a Headbutt
            if ( Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( ) == gEDamageType_Impact )
            {
                GEI32 iPlayerDamage = ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , iWeaponDamage ) + iWeaponDamage;

                if ( ( ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 ) - iPlayerDamage ) <= 0 )
                {
                    if ( iPlayerDamage > 0 )
                    {
                        Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = iPlayerDamage;
                    }
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_Unconscious" );
                    return gEAction_LieKnockDown;
                }

                if ( ( Damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( ) ) >= 0.7f )
                {
                    if ( iPlayerDamage > 0 )
                    {
                        Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = iPlayerDamage;
                    }
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_SitKnockDown" );
                    return gEAction_SitKnockDown;
                }
            }
        }
        // Player is behind (not in his FOV) an unsuspicious (following his daily routine) NPC.
        else if ( Victim.Routine.GetProperty<PSRoutine::PropertyAIMode> ( ) == gEAIMode_Routine && !Victim.IsInFOV ( DamagerOwner ) )
        {
            // PierceAttack is prerequisite for use of Perk_Assassin.
            if ( DamagerOwnerAction == gEAction_PierceAttack )
            {
                if ( Player.Inventory.IsSkillActive ( "Perk_Assassin" ) )
                {
                    // Victim loses all its HP
                    ScriptAdmin.CallScriptFromScript ( "SetHitPoints" , &Victim , &None , 0 );
                    Victim.Routine.FullStop ( );

                    if ( ScriptAdmin.CallScriptFromScript ( "CanBeKilled" , &Victim , &None , 0 ) )
                    {
                        if ( ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
                        {
                            Victim.Routine.SetTask ( "ZS_PiercedDead" );
                            return gEAction_LiePiercedDead;
                        }
                        Victim.Routine.SetTask ( "ZS_RagDollDead" );
                        return gEAction_LieDead;
                    }
                    else
                    {
                        Victim.Routine.SetTask ( "ZS_LieKnockDown" );
                        return gEAction_LieKnockDown;
                    }
                }
            }
            // PowerAttack with a impact Weapon is prerequisite for Perk_KnockOut.
            else if ( ( DamagerOwnerAction == gEAction_PowerAttack || DamagerOwnerAction == gEAction_SprintAttack )
                && ( Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( ) == gEDamageType_Impact ) )
            {
                if ( Player.Inventory.IsSkillActive ( "Perk_KnockOut" ) )
                {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_LieKnockDown" );
                    return gEAction_LieKnockDown;
                }
            }
        }
    }

    // HitForce Berechnung

    // - Magisches Projektil: HitForce = 2

    // - Normales Projektil:
    //     - Wenn Spannung >= 0.6: HitForce = 2
    //     - Sonst: HitForce = 0

    // - Wenn DamagerOwner == None: HitForce = 0

    // - HitForce = max(GetActionWeaponLevel(DamagerOwner) - GetShieldLevelBonus(Victim), 1)

    gEHitForce HitForce = gEHitForce_Minimal;
    if ( IsSpellContainer ( Damager ) )
    {
        HitForce = gEHitForce_Heavy;
    }
    else if ( IsNormalProjectile ( Damager ) )
    {
        // Bow tension, for crossbows always 1.0
        HitForce = Damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( ) >= 0.6f ? gEHitForce_Heavy : gEHitForce_Minimal;
    }
    else if ( DamagerOwner != None )
    {
        GEInt ActionWeaponLevel = GetActionWeaponLevel ( DamagerOwner , DamagerOwnerAction );
        HitForce = static_cast< gEHitForce >( ActionWeaponLevel - GetShieldLevelBonus ( Victim ) );
        if ( HitForce < gEHitForce_Normal )
        {
            HitForce = gEHitForce_Normal;
        }
        else if ( ( GEInt )HitForce < 0 )
        {
            HitForce = gEHitForce_Minimal;
        }
    }

    GEInt FinalDamage = iWeaponDamage;

    // Vulnerabilities (hooked by Damage.cpp)
    if ( DamageTypeEntityTest ( Victim , Damager ) == 2 )
    {
        FinalDamage *= 2;
    }

    // Headshot? -> Double damage
    // ??? Nahkampf-Treffer am Kopf? => Schaden = Schaden * 2
    // PSCollisionShape.GetType() -> gCCollisionShape_PS.GetTouchType()
    if ( Damager.CollisionShape.GetType ( ) == eEPropertySetType_Animation )
    {
        bCString BoneName = Damager.CollisionShape.GetTouchingBone ( );
        if ( BoneName.Contains ( "_Head" , 0 ) ) // Kopfschuß
        {
            FinalDamage *= 2;
        }
    }

    // Player attacks, while not being transformed
    if ( Player == DamagerOwner && !Player.NPC.IsTransformed ( ) )
    {
        // PC-ATTR-BONUS
        GEInt iAttributeBonusDamage;

        // Magic damage
        if ( IsSpellContainer ( Damager ) )
        {
            // Magic projectiles do double damage, but are capped to 200.
            if ( IsMagicProjectile ( Damager ) )
            {
                if ( FinalDamage > 100 )
                {
                    FinalDamage = 100;
                }
                FinalDamage *= 2;
            }

            // PropertyManaUsed depends on cast phase
            if ( Damager.Damage.GetProperty<PSDamage::PropertyManaUsed> ( ) )
            {
                // Bonus = ( Schaden * Altes Wissen / 100 )
                GEFloat fIntelligenceModifier = ScriptAdmin.CallScriptFromScript ( "GetIntelligence" , &DamagerOwner , &None , 0 ) / 100.0f;
                iAttributeBonusDamage = static_cast< GEInt >( FinalDamage * fIntelligenceModifier );
            }
            else
            {
                iAttributeBonusDamage = FinalDamage * 2;
            }
        }
        else
        {
            GEInt attributeValue;
            // Ranged damage
            if ( IsNormalProjectile ( Damager ) == GETrue )
            {
                attributeValue = ScriptAdmin.CallScriptFromScript ( "GetDexterity" , &DamagerOwner , &None , 0 );
            }
            // Melee damage
            else
            {
                attributeValue = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
            }
            iAttributeBonusDamage = attributeValue / 2;
        }
        if ( iAttributeBonusDamage < 0 )
        {
            iAttributeBonusDamage = 0;
        }
        FinalDamage += ScriptAdmin.CallScriptFromScript ( "GetQualityBonus" , &Damager , &Victim , FinalDamage );
        FinalDamage = iAttributeBonusDamage + ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , FinalDamage );
    }
    // Damager is transformed player or NPC
    else if ( DamagerOwner.Navigation.IsValid ( ) )
    {
        // Player is under level 2 and AB is not activated
        if ( Player.NPC.GetProperty<PSNpc::PropertyLevel> ( ) < 2
            || !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing
            //  Ist der Angreifer ein Tier oder Monster?
            || !ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 )
            //  Benutzt der Angreifer Fernkampfwaffen?
            || IsNormalProjectile ( Damager )
            //  Benutzt der Angreifer Magie?
            || IsMagicProjectile ( Damager ) )
        {
            FinalDamage = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
        }
        // Human or Orc attacks the player with a melee weapon
        else if ( Player == Victim )
        {
            GEInt iStrength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
            if ( Damager.GetName ( ) == "Fist" )
            {
                // Greift ein Mensch oder Ork mit Fäusten an?
                FinalDamage = static_cast< GEInt >( iStrength * 0.75f );
            }
            else
            {
                // Greift ein Ork mit einer Nahkampfwaffe an?
                if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Orc )
                {
                    FinalDamage = static_cast< GEInt >( iStrength * 0.75f + iWeaponDamage / 7.0f );
                }
                // Greift ein Mensch mit einer Nahkampfwaffe an?
                else
                {
                    FinalDamage = static_cast< GEInt >( iStrength * 0.9f + iWeaponDamage / 9.0f );
                }
            }
        }
        // Human or Orc attacks a NPC with his fists
        else if ( Damager.GetName ( ) == "Fist" )
        {
            FinalDamage = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 ) / 2;
        }
        else
        {
            FinalDamage = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
        }

        // Monster attacks Orc or Human (NPC)
        if ( ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 )
            && ( !ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 ) )
            && ( Player != DamagerOwner )
            && ( Player != Victim ) )
        {
            if ( DamagerOwner.Party.GetProperty<PSParty::PropertyPartyMemberType> ( ) == gEPartyMemberType_Summoned )
            {
                FinalDamage /= 2;
            }
            else
            {
                FinalDamage /= 10;
            }
        }

        // Magic spells, casted by NPCs, do double damage
        if ( IsMagicProjectile ( Damager ) )
        {
            FinalDamage *= 2;
        }
    }

    // Handelt es sich um einen Powercast? (Player and NPCs)
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainer ( Damager ) )
    {
        // Powercast
        if ( Damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle> ( ) == gEProjectilePath_Missile )
        {
            FinalDamage *= 2;
        }
    }

    //
    // Schritt 2: Rüstung
    //
    GEInt FinalDamage2;
    GEInt iProtection = ScriptAdmin.CallScriptFromScript ( "GetProtection" , &Victim , &Damager , 0 );
    if ( Victim != Player || Player.NPC.IsTransformed ( )
        || !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing
        || IsNormalProjectile ( Damager )
        || IsMagicProjectile ( Damager ) )
    {
        FinalDamage2 = FinalDamage - static_cast< GEInt >( FinalDamage * ( iProtection / 100.0f ) );
    }
    else
    {
        // Held wird mit Nahkampfwaffe angegriffen (AB aktiviert)
        // Schaden = Schaden - PC-PROT
        FinalDamage2 = FinalDamage - iProtection;
        // Wenn der Schaden < 1 ist und vor Schritt 2 > 39 war:
        //   => Schaden = 1-3 % des Schadens vor Abzug der PC-PROT.
        if ( FinalDamage2 < 1
            && ( FinalDamage > 39
                || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_HackAttack
                || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_PowerAttack
                || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_SprintAttack ) )
        {
            FinalDamage2 = static_cast< GEInt >( FinalDamage * ( ( Entity::GetRandomNumber ( 2 ) + 1 ) / 100.0f ) );
        }
    }

    //
    // Schritt 3: Angriffsart
    //
    switch ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) )
    {
        // Angreifer benutzt Quick-Attacke
        //   => Schaden = Schaden / 2
    case gEAction_QuickAttack:
    case gEAction_QuickAttackR:
    case gEAction_QuickAttackL:
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 0.5f );
        break;

        // Angreifer benutzt Powerattacke
        //   => Schaden = Schaden * 2 (bei Kampf mit 2 Waffen nicht durchgehend!)
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
        if ( !CheckHandUseTypes ( gEUseType_1H , gEUseType_1H , DamagerOwner )
            || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyStatePosition> ( ) == 2 )
        {
            FinalDamage2 *= 2;
        }
        break;

        // Angreifer benutzt Hack-Attacke
        //   => Schaden = Schaden * 2
    case gEAction_HackAttack:
        FinalDamage2 *= 2;
        break;
    }

    //
    // Schritt 4: Parade
    //
    /*
       Voraussetzungen für erfolgreiche Paraden:
         1. Das Opfer befindet sich im Parade-Modus.
         2. Der Angriff kommt von vorne.
         3. Bei Verteidigung mit Fäusten kann man nur parieren, wenn der Angreifer auch Fäuste benutzt (Boxkampf).
         4. Für Nahkampf-Angriffe gilt:
           - Hack-Attacken können nie pariert werden.
           - Sprint-Attacken können nie pariert werden. (Sprint-Attacken tauchen nur mit AB auf.)
           - Stich-Attacken kann man nur mit einem Schild abwehren.
         5. Magie-Angriffe kann nur der Held abwehren. Voraussetzung: Er beherrscht "Magische Stäbe" und hält in seinen Händen einen Stab und einen Zauberspruch.
         6. Fernkampf-Angriffe kann man nur mit einem Schild abwehren.
    */
    if ( FinalDamage2 < 0 )
        FinalDamage2 = 0;

    if ( Victim == Player )
        Victim.Effect.StopEffect ( GETrue );

    // Parade Magic
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainer ( Damager ) )
    {
        if ( ScriptAdmin.CallScriptFromScript ( "CanParadeMagic" , &Victim , &Damager , 0 ) )
        {
            GEInt iManaPenalty = -FinalDamage;
            if ( eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing )
            {
                GEFloat fPenaltyModifier = 1.0f - ( ( ScriptAdmin.CallScriptFromScript ( "GetIntelligence" , &Victim , &None , 0 ) - 200 ) / 500.0f );
                if ( fPenaltyModifier > 0 )
                {
                    if ( fPenaltyModifier < 1 )
                        iManaPenalty = static_cast< GEInt >( fPenaltyModifier * iManaPenalty );
                }
                else
                {
                    iManaPenalty = 0;
                }
            }
            GEInt iManaRemaining = iManaPenalty + ScriptAdmin.CallScriptFromScript ( "GetManaPoints" , &Victim , &None , 0 );
            if ( iManaRemaining > 0 )
                iManaRemaining = 0;

            ScriptAdmin.CallScriptFromScript ( "AddManaPoints" , &Victim , &None , iManaPenalty );
            ScriptAdmin.CallScriptFromScript ( "AddHitPoints" , &Victim , &None , iManaRemaining );
            if ( ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 ) > 0 )
            {
                Victim.Routine.FullStop ( );
                Victim.Routine.SetTask ( "ZS_ParadeStumble" );
                return gEAction_MagicParade;
            }
        }
    }
    // Parade Missile
    else if ( IsNormalProjectile ( Damager ) )
    {
        if ( ScriptAdmin.CallScriptFromScript ( "CanParadeMissile" , &Victim , &Damager , 0 ) )
        {
            GEInt iStaminaPenalty = -FinalDamage;
            if ( Victim == Player && Victim.Inventory.IsSkillActive ( "Perk_Shield_2" ) )
                iStaminaPenalty /= 2;

            GEInt iStaminaRemaining = iStaminaPenalty + ScriptAdmin.CallScriptFromScript ( "GetStaminaPoints" , &Victim , &None , 0 );
            if ( iStaminaRemaining > 0 )
                iStaminaRemaining = 0;

            ScriptAdmin.CallScriptFromScript ( "AddStaminaPoints" , &Victim , &None , iStaminaPenalty );
            ScriptAdmin.CallScriptFromScript ( "AddHitPoints" , &Victim , &None , iStaminaRemaining );
            if ( ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 ) > 0 )
            {
                if ( HitForce > gEHitForce_Minimal )
                {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_ParadeStumble" );
                    return gEAction_ParadeStumble;
                }
                return gEAction_QuickParadeStumble;
            }
        }
    }
    // Can parade meele?
    else if ( ScriptAdmin.CallScriptFromScript ( "CanParade" , &Victim , &DamagerOwner , 0 ) )
    {
        GEInt FinalDamage3 = FinalDamage / -2;
        // Reduce damage if parading melee with shield
        if ( CheckHandUseTypes ( gEUseType_Shield , gEUseType_1H , Victim ) )
        {
            if ( Victim != Player || !Victim.Inventory.IsSkillActive ( "Perk_Shield_2" ) )
            {
                // Weicht von "Detaillierte Schadenberechnung" ab, dort wird ein faktor von 2/3 anstatt 0.5 beschrieben.
                FinalDamage3 /= 2;
            }
            else
            {
                FinalDamage3 -= FinalDamage3 / 4;
            }
        }

        // AlternativeAI parade (es werden keine Lebenspunkte angezogen)
        if ( eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeAI )
        {
            // Ausdauer abziehen
            ScriptAdmin.CallScriptFromScript ( "AddStaminaPoints" , &Victim , &None , FinalDamage3 );

            if ( !ScriptAdmin.CallScriptFromScript ( "IsInFistMode" , &Victim , &None , 0 ) )
            {
                if ( Damager.CollisionShape.GetPhysicMaterial ( ) != eEShapeMaterial_Metal
                || ( Victim.Inventory.GetItemFromSlot ( gESlot_RightHand ).CollisionShape.GetPhysicMaterial ( ) != eEShapeMaterial_Metal ) )
                {
                    EffectSystem::StartEffect ( "eff_col_weaponhitslevel_metal_wood_01" , Victim );
                }
                else
                {
                    EffectSystem::StartEffect ( "eff_col_wh_01_me_me" , Victim );
                }
            }

            gCScriptProcessingUnit::gSArgsFor__sAICombatMoveInstr InstrArgs (
                DamagerOwner.GetInstance ( ) , Victim.GetInstance ( ) , DamagerOwnerAction , g_pstrPhaseString[gEPhase_Recover] , 1.5f );
            if ( !gCScriptProcessingUnit::sAICombatMoveInstr ( &InstrArgs , a_pSPU , GEFalse ) )
                return gEAction_None;

            if ( HitForce > gEHitForce_Normal )
            {
                if ( HitForce > gEHitForce_Heavy )
                {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_HeavyParadeStumble" );
                    return gEAction_HeavyParadeStumble;
                }
                else
                {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_ParadeStumble" );
                    return gEAction_ParadeStumble;
                }
            }

            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_ParadeStumble" );
            return gEAction_QuickParadeStumble;
        }

        // Ausdauer und ggf. Lebenspunkte abziehen
        GEInt iStaminaRemaining = FinalDamage3 + ScriptAdmin.CallScriptFromScript ( "GetStaminaPoints" , &Victim , &None , 0 );
        if ( iStaminaRemaining > 0 )
            iStaminaRemaining = 0;
        ScriptAdmin.CallScriptFromScript ( "AddStaminaPoints" , &Victim , &None , FinalDamage3 );
        ScriptAdmin.CallScriptFromScript ( "AddHitPoints" , &Victim , &None , iStaminaRemaining );

        // Wenn der bei der Parade erhaltene Schaden das Opfer nicht unter 0 HP bringt
        if ( ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 ) > 0 )
        {
            if ( HitForce > gEHitForce_Normal )
            {
                if ( HitForce > gEHitForce_Heavy )
                {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_HeavyParadeStumble" );
                    return gEAction_HeavyParadeStumble;
                }
                else
                {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_ParadeStumble" );
                    return gEAction_ParadeStumble;
                }
            }
            else
            {
                Victim.Routine.FullStop ( );
                Victim.Routine.SetTask ( "ZS_ParadeStumble" );
                return gEAction_QuickParadeStumble;
            }
        }
    }

    // Opfer konnte nicht parieren, oder der beim Parieren erhalten Schaden hat es auf <= 0 LP gebracht.

    // Reduce Damage for unkillable NPCs (obsolete)
    GEInt iVictimHitPoints = ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 );
    if ( Victim != Player && !ScriptAdmin.CallScriptFromScript ( "CanBeKilled" , &Victim , &None , 0 ) )
    {
        if ( FinalDamage2 > 30 )
        {
            FinalDamage2 = 30;
        }
        if ( FinalDamage2 > iVictimHitPoints )
        {
            FinalDamage2 = 0;
        }
    }

    // Modify damage in some situations, if AB is activated
    if ( eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing )
    {
        // Wenn es sich um einen Arenakampf handelt
        //  => Schaden = Schaden / 2
        if ( ( Victim != Player ) && ( Victim.NPC.GetProperty<PSNpc::PropertyAttackReason> ( ) == gEAttackReason_Arena ) )
        {
            FinalDamage2 /= 2;
        }

        // Wenn der Held bei Schwierigkeitsgrad "hoch" eine Fernkampfwaffe benutzt
        //  => Schaden = Schaden * 1,2
        if ( Victim != Player && IsNormalProjectile ( Damager ) && Entity::GetCurrentDifficulty ( ) == EDifficulty_Hard )
        {
            FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 1.2f );
        }
    }

    // Receive Damage
    if ( FinalDamage2 > 0 )
    {
        Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = FinalDamage2;
    }

    // Process StatusEffects and Animations:
    // CanBurn
    if ( ScriptAdmin.CallScriptFromScript ( "CanBurn" , &Victim , &Damager , FinalDamage2 ) )
    {
        Victim.NPC.EnableStatusEffects ( gEStatusEffect_Burning , GETrue );
        Victim.Effect.StartRuntimeEffect ( "eff_magic_firespell_target_01" );
    }
    // Dead, KnockDown or KnockedOut
    if ( iVictimHitPoints - FinalDamage2 <= 0 )
    {
        if ( ScriptAdmin.CallScriptFromScript ( "IsDeadlyDamage" , &Victim , &Damager , 0 ) )
        {
            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_RagDollDead" );
            return gEAction_LieDead;
        }

        if ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_PierceAttack
            && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
        {
            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_PiercedKO" );
            return gEAction_LiePiercedKO;
        }

        Victim.Routine.FullStop ( );
        if ( HitForce >= 3 )
        {
            Victim.Routine.SetTask ( "ZS_Unconscious" );
            return gEAction_LieKnockOut;
        }
        Victim.Routine.SetTask ( "ZS_LieKnockDown" );
        return gEAction_LieKnockDown;
    }

    if ( Victim.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_SprintAttack )
    {
        return gEAction_None;
    }

    if ( HitForce <= gEHitForce_Minimal )
    {
        ScriptAdmin.CallScriptFromScript ( "PipiStumble" , &Victim , &None , 0 );
        return gEAction_QuickStumble;
    }

    if ( ScriptAdmin.CallScriptFromScript ( "CanBePoisoned" , &Victim , &Damager , DamagerOwnerAction == gEAction_PierceAttack || DamagerOwnerAction == gEAction_HackAttack ) )
    {
        Victim.NPC.EnableStatusEffects ( gEStatusEffect_Poisoned , GETrue );
    }

    if ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_PierceAttack
        && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
    {
        Victim.Routine.FullStop ( );
        Victim.Routine.SetTask ( "ZS_PierceStumble" );
        return gEAction_PierceStumble;
    }

    if ( ScriptAdmin.CallScriptFromScript ( "CanBeDiseased" , &Victim , &Damager , 0 ) )
    {
        Victim.NPC.EnableStatusEffects ( gEStatusEffect_Diseased , GETrue );
    }

    if ( ScriptAdmin.CallScriptFromScript ( "CanFreeze" , &Victim , &Damager , FinalDamage2 ) )
    {
        GEInt iFreezeTime = FinalDamage2 / 20;
        if ( iFreezeTime < 5 )
            iFreezeTime = 5;

        Victim.Routine.FullStop ( );
        Victim.Routine.SetTask ( "ZS_Freeze" );
        Victim.Routine.AccessProperty<PSRoutine::PropertyTaskPosition> ( ) = 12 * iFreezeTime;
        return gEAction_None;
    }

    if ( GetHeldWeaponCategory ( Victim ) != gEWeaponCategory_None && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
    {
        if ( HitForce >= 4 && GetHeldWeaponCategory ( Victim ) == gEWeaponCategory_Melee )
        {
            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_SitKnockDown" );
            return gEAction_SitKnockDown;
        }

        if ( HitForce < gEHitForce_Heavy )
        {
            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_QuickStumble" );
            return gEAction_QuickStumble;
        }
    }

    Victim.Routine.FullStop ( );
    Victim.Routine.SetTask ( "ZS_Stumble" );
    return gEAction_Stumble;
}

void Hook ( )
{
    static mCFunctionHook Hook_AssessHit;
    Hook_AssessHit.Hook ( GetScriptAdminExt ( ).GetScript ( "AssessHit" )->m_funcScript , &AssessHit , mCBaseHook::mEHookType_OnlyStack );
}
