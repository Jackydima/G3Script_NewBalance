#include "Script_NewBalance.h"
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"
#include "utility.h"


gSScriptInit& GetScriptInit ( )
{
    static gSScriptInit s_ScriptInit;
    return s_ScriptInit;
}

enum gEHitForce
{
    gEHitForce_Minimal = 0 ,
    gEHitForce_Normal = 1 ,
    gEHitForce_Heavy = 2
};

GEFloat fMonsterDamageMultiplicator = 0.5;
void LoadSettings ( ) {
    eCConfigFile config = eCConfigFile ( );
    if ( config.ReadFile ( bCString ( "monsterdamage.ini" ) ) ) {
        fMonsterDamageMultiplicator = config.GetFloat ( bCString ( "Game" ) , bCString ( "Game.MonsterDamageMultiplicator" ) , GEFalse );
    }
}

// wird aufgerufen von DoLogicalDamage
gEAction GE_STDCALL AssessHit ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs )
{
    INIT_SCRIPT_EXT ( Victim , Damager );

    gCScriptAdmin& ScriptAdmin = GetScriptAdmin ( );
    //std::cout << "\n--------------------------------------------------------------\n";

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
    gEAction VictimAction = Victim.Routine.GetProperty<PSRoutine::PropertyAction> ( );

    /**
    * Workaround: Changed in the FixResetAll() the Trigger of Weapons when functions like all Stumble + PipiStumble get called it would still register Weaponhits 
    * after Trading with no stun
    * After the fix, when Attacker are Stunning Victims the Weapon of the Victim has Attackframes even thought he got stunned before!
    * So here it should ignore Hits, when Attacker is Stunned before but not if he got the PipiStumble.
    */
    switch ( DamagerOwnerAction ) {
    case gEAction_QuickParadeStumble:
    case gEAction_ParadeStumble:
    case gEAction_ParadeStumbleR:
    case gEAction_ParadeStumbleL:
    case gEAction_HeavyParadeStumble:
    case gEAction_QuickStumble:
    case gEAction_Stumble:
    case gEAction_StumbleR:
    case gEAction_StumbleL:
    case gEAction_AbortAttack:
        return DamagerOwnerAction;
    }

    //std::cout << "Victim gEAction: " << Victim.Routine.GetProperty<PSRoutine::PropertyAction> ( )
       // << "\tDamager gEAction: " << DamagerOwnerAction << "\n";
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
    bCString VictimItemTemplateName = Victim.Inventory.GetTemplateItem(Victim.Inventory.FindStackIndex ( gESlot_RightHand )).GetName();
    //std::cout << "Damager: " << DamagerOwner.GetName() << "\tVictim: " << Victim.GetName() << std::endl;
    //std::cout << "TemplateDamage: " << iDamageAmount << "\tMultiplier: " << fDamageMultiplier << std::endl;
    //std::cout << "Victim Weapon: " << VictimItemTemplateName << std::endl;

    // Handle a few special cases if the player is the damager.
    if ( Player == DamagerOwner )
    {
        // Player is behind (not in his FOV) an unsuspicious (following his daily routine) NPC.
        if ( Victim.Routine.GetProperty<PSRoutine::PropertyAIMode> ( ) == gEAIMode_Routine && !Victim.IsInFOV ( DamagerOwner ) )
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
        //std::cout << "Went in DamagerOwner Logic for getting Skills\tWeaponLevel: " << ActionWeaponLevel << "\tHitforce: "
           // << HitForce << "\tShieldLevel Bonus: " << GetShieldLevelBonus(Victim) << std::endl;
        if ( isBigMonster ( Victim ) && (VictimAction == gEAction_PowerAttack || VictimAction == gEAction_SprintAttack) )
            HitForce = static_cast< gEHitForce >(HitForce - 2);
        //std::cout << "HitForce after Monster: " << HitForce << "\n";
        if ((GEInt)HitForce <= -2)
        {
            HitForce = gEHitForce_Minimal;
        }
        else if ((GEInt)HitForce <= gEHitForce_Normal)
        {
            HitForce = gEHitForce_Normal;
        }
    }



    GEBool isHeadshot = GEFalse;


    // PSCollisionShape.GetType() -> gCCollisionShape_PS.GetTouchType()
    if ( Damager.CollisionShape.GetType ( ) == eEPropertySetType_Animation )
    {
        bCString BoneName = Damager.CollisionShape.GetTouchingBone ( );
        if ( BoneName.Contains ( "_Head" , 0 ) ) // Kopfschuﬂ
        {
            isHeadshot = GETrue;
        }
    }
    // New Hyperarmor for New Balancing Spells
    if ( DamagerOwnerAction != gEAction_PierceAttack && DamagerOwnerAction != gEAction_HackAttack && DamagerOwnerAction != gEAction_Summon 
       && !VictimItemTemplateName.Contains("Heal") && !isHeadshot ) {
        HitForce = static_cast< gEHitForce >( HitForce - GetHyperActionBonus ( VictimAction ) );
    }
    
    //std::cout << "Hitforce after Special Hyperarmor: " << HitForce << "\n";
    GEInt FinalDamage = iWeaponDamage;

    // Headshot? -> Double damage
    // ??? Nahkampf-Treffer am Kopf? => Schaden = Schaden * 2
    if ( isHeadshot ) {
        FinalDamage *= 2;
    }
    //std::cout << "Finaldamage after Head: " << FinalDamage << "\n";

    // Player attacks, while not being transformed
    if ( Player == DamagerOwner && !Player.NPC.IsTransformed ( ) )
    {
        // PC-ATTR-BONUS
        GEInt iAttributeBonusDamage;

        // Magic damage
        if ( IsSpellContainer ( Damager ) )
        {
            // Magic projectiles do double damage, !but are capped to 200.
            if ( IsMagicProjectile ( Damager ) )
            {
                FinalDamage *= 2;

            }
            //std::cout << "Finaldamage after Magictest: " << FinalDamage << "\n";
            
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
            GEInt dexterity = ScriptAdmin.CallScriptFromScript ( "GetDexterity" , &DamagerOwner , &None , 0 );
            GEInt strength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
            GEInt intelligence = ScriptAdmin.CallScriptFromScript ( "GetIntelligence" , &DamagerOwner , &None , 0 );
            // Ranged damage
            if ( IsNormalProjectile ( Damager ) == GETrue )
            {
                iAttributeBonusDamage = dexterity / 2;
            }
            // Melee damage
            else {
                GEInt rightWeaponStackIndex = Player.Inventory.FindStackIndex ( gESlot_RightHand );
                GEInt leftWeaponStackIndex = Player.Inventory.FindStackIndex ( gESlot_LeftHand );
                gEUseType playerRightWeaponType = Player.Inventory.GetUseType ( rightWeaponStackIndex );

                GEChar* arr = ( GEChar* )*( DWORD* )( *( DWORD* )&Player.GetWeapon ( GETrue ).Item + 0x74 ); // A bit Unsafe ...
                //if (arr != nullptr) //std::cout << "ReqAttribute1 Tag: " << arr << "\n";
                bCString reqAttributeTag = "";
                if ( arr != nullptr ) reqAttributeTag = bCString ( arr );
                if ( playerRightWeaponType == gEUseType_1H && Player.Inventory.GetUseType ( leftWeaponStackIndex ) == gEUseType_1H  ) {
                    iAttributeBonusDamage = static_cast<GEInt>(strength * 0.3 + dexterity * 0.35);
                    //std::cout << "DEX Scaling\tBonus: " << iAttributeBonusDamage << "\n";
                }
                else if ( reqAttributeTag.Contains ( "DEX" ) ) {
                    iAttributeBonusDamage = static_cast< GEInt >( strength * 0.2 + dexterity * 0.4 );
                }
                else if ( playerRightWeaponType == gEUseType_2H || playerRightWeaponType == gEUseType::gEUseType_Axe || playerRightWeaponType == gEUseType::gEUseType_Pickaxe ) {
                    iAttributeBonusDamage = static_cast< GEInt >(strength * 0.6);
                }
                else if ( playerRightWeaponType == gEUseType_Staff || reqAttributeTag.Contains ( "INT" )
                     || DamagerOwner.Inventory.GetItemFromSlot ( gESlot_RightHand ).Item.GetQuality ( ) & ( 8 + 16 ) ) {
                    iAttributeBonusDamage = static_cast< GEInt >( strength * 0.2 + intelligence * 0.4 + 15 ); // Because you start with low Int, (Assume 60 INT)
                    //std::cout << "INT Scaling\tBonus: " << iAttributeBonusDamage << "\n";
                }
                else {
                    iAttributeBonusDamage = strength / 2;
                    //std::cout << "Normal Scaling\tBonus: " << iAttributeBonusDamage << "\n";
                }
            }
        }
        if ( iAttributeBonusDamage < 0 )
        {
            iAttributeBonusDamage = 0;
        }
        FinalDamage += ScriptAdmin.CallScriptFromScript ( "GetQualityBonus" , &Damager , &Victim , FinalDamage );
        //std::cout << "Finaldamage after Adding QualityBonus: " << FinalDamage << "\n";
        //std::cout << "GetPlayerSkillDamageBonus: " << ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , FinalDamage ) << "\n";
        FinalDamage += iAttributeBonusDamage + ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , FinalDamage );
        //std::cout << "Finaldamage after Adding Bonuses for PC_Hero: " << FinalDamage << "\tAttributBonus was: "<< iAttributeBonusDamage << "\n";
    }
    // Damager is transformed player or NPC
    else if ( DamagerOwner.Navigation.IsValid ( ) )
    {
        // Player is under level 2 and AB is not activated
        if ( Player.NPC.GetProperty<PSNpc::PropertyLevel> ( ) < 2
            || !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing
            //  Besitzt der Angreifer keine Waffe?
            || ( Damager.Interaction.GetOwner ( ) == None && Damager.Navigation.IsValid ( ) ) )
            //  Benutzt der Angreifer Fernkampfwaffen?
        {
            FinalDamage = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
            //std::cout << "Finaldamage after GetStrengh for NPCs: " << FinalDamage << "\n";
        }
        // NPC attacks with a weapon
        else 
        {
            GEInt iStrength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 );
            //std::cout << "NPC Strengh: " << iStrength << "\n"; 
            if ( Damager.GetName ( ) == "Fist" && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 ) )
            {
                // Greift ein Mensch oder Ork mit F‰usten an?
                FinalDamage = static_cast< GEInt >( iStrength / 2 );
                //std::cout << "Finaldamage after Fists: " << FinalDamage << "\n";
            }
            else
            {
                if ( IsNormalProjectile ( Damager ) || IsMagicProjectile ( Damager ) || IsSpellContainer ( Damager ) ) {
                    FinalDamage = static_cast< GEInt >( iStrength * 0.9f + FinalDamage / 7.0f );
                    //std::cout << "Finaldamage after Spell/Projectile: " << FinalDamage << "\n";
                }
                // Greift ein Ork mit einer Nahkampfwaffe an?
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Orc )
                {
                    FinalDamage = static_cast< GEInt >( iStrength * 0.75f + FinalDamage / 7.0f );
                    //std::cout << "Finaldamage after GetStrengh for Orcs: " << FinalDamage << "\n";
                }
                // Greift ein Mensch mit einer Nahkampfwaffe an?
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Human )
                {
                    FinalDamage = static_cast< GEInt >( iStrength * 0.9f + FinalDamage / 9.0f );
                    //std::cout << "Finaldamage after GetStrengh for Humans: " << FinalDamage << "\n";
                }
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Demon )
                {
                    FinalDamage = static_cast< GEInt >( iStrength * 0.9f + FinalDamage / 12.0f );
                    //std::cout << "Finaldamage after GetStrengh for Demons: " << FinalDamage << "\n";
                }
                else {
                    // Probably if No Weapon is equipped (not even Fist)
                    if ( FinalDamage == 0 ) {
                        FinalDamage = iStrength;
                    }
                    else {
                        FinalDamage = static_cast< GEInt >( iStrength * 0.9f + FinalDamage / 9.0f );
                    }
                    //std::cout << "Finaldamage after GetStrengh Restshit: " << FinalDamage << "\n";
                }
            }
        }

        // Monster attacks Orc or Human (NPC)
        if ( ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 )
            && ( !ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 ) )
            && ( Player != DamagerOwner )
            && ( Player != Victim ) )
        {
            if ( DamagerOwner.Party.GetProperty<PSParty::PropertyPartyMemberType> ( ) == gEPartyMemberType_Summoned )
            {
                //FinalDamage /= 2;
            }
            else
            {
                FinalDamage *= fMonsterDamageMultiplicator;
            }
        }

        // Magic spells, casted by NPCs, do double damage
        if ( IsMagicProjectile ( Damager ) || IsSpellContainer ( Damager ) )
        {
            FinalDamage *= 2;
            //std::cout << "Finaldamage after Magicspells for NPCs: " << FinalDamage << "\n";
        }
    }

    // Vulnerabilities (hooked by Damage.cpp)
    /*if ( DamageTypeEntityTest ( Victim , Damager ) == VulnerabilityStatus_Weak )
    {
        FinalDamage *= 2;
    }*/
    switch ( DamageTypeEntityTest ( Victim , Damager ) ) {
    case VulnerabilityStatus_WEAK:
        FinalDamage *= 2;
        break;
    case VulnerabilityStatus_STRONG:
        FinalDamage /= 2;
    }
    //std::cout << "Finaldamage after Vulnerabilities: " << FinalDamage << "\n";

    // Handelt es sich um einen Powercast? (Player and NPCs)
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainer ( Damager ) )
    {
        // Powercast
        if ( Damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle> ( ) == gEProjectilePath_Missile )
        {
            FinalDamage *= 2;
            //std::cout << "Finaldamage after Powercast: " << FinalDamage << "\n";
        }
    }

    //
    // Schritt 2: R¸stung
    //
    GEInt FinalDamage2;
    GEInt iProtection = ScriptAdmin.CallScriptFromScript ( "GetProtection" , &Victim , &Damager , 0 );
    //std::cout << "Protection of Victim: " << iProtection << "\n";
    // If AB Active, it reduces the Bonus of Armor with 0.2, because it used another system here
    if ( eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing && Victim == Player )
        iProtection *= 2; 

    FinalDamage2 = FinalDamage - static_cast< GEInt >( FinalDamage * ( iProtection / 100.0f ) );

    /*
    * Default Protection!
    if ( Victim != Player || Player.NPC.IsTransformed ( )
        || !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing
        || IsNormalProjectile ( Damager )
        || IsMagicProjectile ( Damager ) )
    {
        FinalDamage2 = FinalDamage - static_cast< GEInt >( FinalDamage * ( iProtection / 100.0f ) );
        //std::cout << "Finaldamage2 after Armor Protection for NonePlayerEnity and Projectiles: " << FinalDamage2 << "\n";
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
        //std::cout << "Finaldamage2 after Armor Protect for PC_Hero: " << FinalDamage2 << "\n";
    }*/

    //
    // Schritt 3: Angriffsart
    //
    //std::cout << "Action: " << DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) << std::endl;
    if ( DamageTypeEntityTest ( Victim , Damager ) == VulnerabilityStatus_IMMUNE && FinalDamage2 > 5) 
        FinalDamage2 = 5;

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
    //std::cout << "Finaldamage2 after AttackType: " << FinalDamage2 << "\n";

    //
    // Schritt 4: Parade
    //
    /*
       Voraussetzungen f¸r erfolgreiche Paraden:
         1. Das Opfer befindet sich im Parade-Modus.
         2. Der Angriff kommt von vorne.
         3. Bei Verteidigung mit F‰usten kann man nur parieren, wenn der Angreifer auch F‰uste benutzt (Boxkampf).
         4. F¸r Nahkampf-Angriffe gilt:
           - Hack-Attacken kˆnnen nie pariert werden.
           - Sprint-Attacken kˆnnen nie pariert werden. (Sprint-Attacken tauchen nur mit AB auf.)
           - Stich-Attacken kann man nur mit einem Schild abwehren.
         5. Magie-Angriffe kann nur der Held abwehren. Voraussetzung: Er beherrscht "Magische St‰be" und h‰lt in seinen H‰nden einen Stab und einen Zauberspruch.
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
    else if ( ScriptAdmin.CallScriptFromScript ( "CanParade" , &Victim , &DamagerOwner , 0 ) || 
        ( Victim.Routine.GetProperty<PSRoutine::PropertyAniState>() == gEAniState_SitKnockDown && Victim.IsInFOV ( DamagerOwner ) ))
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
        /*
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

            gCScriptProcessingUnit::sAICombatMoveInstr_Args InstrArgs (
                DamagerOwner.GetInstance () , Victim.GetInstance () , DamagerOwnerAction , bCString("Recover") , 1.5f );//g_pstrPhaseString[gEPhase_Recover]
            if ( !gCScriptProcessingUnit::sAICombatMoveInstr (&InstrArgs , a_pSPU , GEFalse ))
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
        }*/

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

    if ( IsNormalProjectile ( Damager ) && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
    {
        // Projectile is a Headbutt
        if ( Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( ) == gEDamageType_Impact )
        {
            if ( ( ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 ) - FinalDamage2 ) <= 0 )
            {
                if ( FinalDamage2 > 0 )
                {
                    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = FinalDamage2;
                    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageType> ( ) = Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
                }
                //std::cout << "DamagerOwner: " << DamagerOwner.GetName ( ) << "\nVictim: " << Victim.GetName ( ) << "\n";
                //std::cout << "Attitude D->V: " << ScriptAdmin.CallScriptFromScript ( "GetAttitude" , &DamagerOwner , &Victim , 0 ) << "\n";
                //std::cout << "Attitude V->D: " << ScriptAdmin.CallScriptFromScript ( "GetAttitude" , &Victim , &DamagerOwner , 0 ) << "\n";
                if ( ScriptAdmin.CallScriptFromScript ( "GetAttitude" , &Victim , &DamagerOwner , 0 ) == gEAttitude::gEAttitude_Hostile ||
                    ScriptAdmin.CallScriptFromScript ( "GetAttitude" , &Victim , &DamagerOwner , 0 ) == gEAttitude::gEAttitude_Panic ) {
                    Victim.Routine.FullStop ( );
                    Victim.Routine.SetTask ( "ZS_RagDollDead" );
                    return gEAction_LieDead;
                }

                Victim.Routine.FullStop ( );
                Victim.Routine.SetTask ( "ZS_Unconscious" );
                return gEAction_LieKnockDown;
            }

            if ( ( Damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( ) ) >= 0.7f )
            {
                if ( FinalDamage2 > 0 )
                {
                    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = FinalDamage2;
                    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageType> ( ) = Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
                }
                Victim.Routine.FullStop ( );
                Victim.Routine.SetTask ( "ZS_SitKnockDown" );
                return gEAction_SitKnockDown;
            }
        }
    }

    // Receive Damage
    if ( FinalDamage2 > 0 )
    {
        Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = FinalDamage2;
        Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageType> ( ) = Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
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

    /*
    * Nostun Of animals when they are Mad
    if ( Victim.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_SprintAttack )
    {
        return gEAction_None;
    }
    */

    if ( ScriptAdmin.CallScriptFromScript ( "CanBePoisoned" , &Victim , &Damager , DamagerOwnerAction == gEAction_PierceAttack || DamagerOwnerAction == gEAction_HackAttack ) )
    {
        Victim.NPC.EnableStatusEffects ( gEStatusEffect_Poisoned , GETrue );
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
        if ( Damager.GetName ( ) == "Mis_IceBlock" )
            iFreezeTime = 60;
        Victim.Routine.FullStop ( );
        Victim.Routine.SetTask ( "ZS_Freeze" );
        Victim.Routine.AccessProperty<PSRoutine::PropertyTaskPosition> ( ) = 12 * iFreezeTime;
        return gEAction_None;
    }

    //Stun Protection
    if ( DamageTypeEntityTest ( Victim , Damager ) == VulnerabilityStatus_IMMUNE )
        return gEAction_None;

    if ( VictimAction == gEAction_SitKnockDown ) {
        ScriptAdmin.CallScriptFromScript ( "PipiStumble" , &Victim , &None , 0 );
        return gEAction_SitKnockDown;
    }

    if ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_PierceAttack
        && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
    {
        Victim.Routine.FullStop ( );
        Victim.Routine.SetTask ( "ZS_PierceStumble" );
        return gEAction_PierceStumble;
    }

    // Scream or make HitEffect, but no Stumble also processes logic when you hit someone, like setting up combat mode
    if ( HitForce <= gEHitForce_Minimal )
    {
        ScriptAdmin.CallScriptFromScript ( "PipiStumble" , &Victim , &None , 0 );
        return DamagerOwnerAction;
    }

    if ( /*GetHeldWeaponCategory ( Victim ) != gEWeaponCategory_None && */ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 )) //Ge‰ndert
    {
        if ( HitForce >= 4 /* && GetHeldWeaponCategory ( Victim ) == gEWeaponCategory_Melee */) //Remove
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

static mCCallHook Hook_AssureProjectiles;
void AssureProjectiles (GEInt registerBaseStack) {
    Entity* self = (Entity*) ( registerBaseStack - 0x2A0 );
    //std::cout << "Self: " << self->GetName() << "\n";
    if ( *self == None ) {
        //std::cout << "Unlucky" << "\n";
        return;
    }
    GEInt random = Entity::GetRandomNumber ( 10 );
    GEInt leftHandWeaponIndex = self->Inventory.FindStackIndex ( gESlot_LeftHand );
    Hook_AssureProjectiles.SetImmEbx<GEInt> ( leftHandWeaponIndex );
    gEUseType leftHandUseType = self->Inventory.GetUseType ( leftHandWeaponIndex );
    Template projectile = Template ( getProjectile ( *self , leftHandUseType ) );
    GEInt stack = self->Inventory.AssureItems ( projectile , 0 , random + 10 );
    *(GEInt*)( registerBaseStack - 0x2C4 ) = stack;
}



void ResetAllFix() {
    //const BYTE args[] = { 0x6A, 0x07 };
    //const size_t argsSize = sizeof ( args ) / sizeof ( BYTE );
    DWORD currProt , newProt;
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x24af4 ) , 0x24aff -0x24af4 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x24af4 ),0x90, 0x24aff - 0x24af4 );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x24af4 ) , 0x24aff - 0x24af4 , currProt , &newProt );

    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x24b2a ) , 0x24b35 - 0x24b2a , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x24b2a ) , 0x90 , 0x24b35 - 0x24b2a );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x24b2a ) , 0x24b35 - 0x24b2a , currProt , &newProt );
}

void PatchCode () {
    // b5045-b503d Call for DamageEntityTest in DoLogicalDamage removed (not ingnorig the Immune Status anymore)
    DWORD currProt , newProt;
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb503d ) , 0xb5045 - 0xb503d , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb503d ) , 0x90 , 0xb5045 - 0xb503d );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb503d ) , 0xb5045 - 0xb503d , currProt , &newProt );

    // Remove HitProt when Entity is SitDowned
    // 0xb51c0 - 0xb51b1
    // Is gEAnimationState Knockdown? 
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb51b1 ) , 0xb51c0 - 0xb51b1 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb51b1 ) , 0x90 , 0xb51c0 - 0xb51b1 );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb51b1 ) , 0xb51c0 - 0xb51b1 , currProt , &newProt );
}

static mCFunctionHook Hook_CanBurn;
static mCFunctionHook Hook_CanFreeze;
GEInt CanFreezeAddition ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {
    // Fix for new Spell
    if ( a_pOtherEntity->GetName ( ) == "Mis_IceBlock" )
        return GETrue;
    return Hook_CanFreeze.GetOriginalFunction ( &CanFreezeAddition )( a_pSPU , a_pSelfEntity , a_pOtherEntity , a_iArgs );
}

extern "C" __declspec( dllexport )
gSScriptInit const * GE_STDCALL ScriptInit( void )
{
    // Ensure that that Script_Game.dll is loaded.
    GetScriptAdmin().LoadScriptDLL("Script_Game.dll");
    
    GetScriptAdmin().LoadScriptDLL("Script_G3Fixes.dll");
    if ( !GetScriptAdmin ( ).IsScriptDLLLoaded ( "Script_G3Fixes.dll" ) ) {
        Hook_CanBurn.Hook ( GetScriptAdminExt ( ).GetScript ( "CanBurn" )->m_funcScript , &CanBurn , mCBaseHook::mEHookType_OnlyStack );
        Hook_CanFreeze.Hook ( GetScriptAdminExt ( ).GetScript ( "CanFreeze" )->m_funcScript, &CanFreeze , mCBaseHook::mEHookType_OnlyStack );
    }
    else {
        Hook_CanFreeze.Hook ( GetScriptAdminExt ( ).GetScript ( "CanFreeze" )->m_funcScript , &CanFreezeAddition , mCBaseHook::mEHookType_OnlyStack );
    }


    LoadSettings();

    ResetAllFix();
    PatchCode();
    static mCFunctionHook Hook_Assesshit;
    /*Hook_Assesshit
        .Prepare(RVA_ScriptGame(0x2b580), &AssessHit, mCBaseHook::mEHookType_OnlyStack)
        .Hook();*/

    Hook_Assesshit.Hook ( GetScriptAdminExt ( ).GetScript ( "AssessHit" )->m_funcScript , &AssessHit , mCBaseHook::mEHookType_OnlyStack );

    Hook_AssureProjectiles
        .Prepare ( RVA_ScriptGame ( 0x192a2 ) , &AssureProjectiles , mCBaseHook::mEHookType_Mixed , mCRegisterBase::mERegisterType_Ebx )
        .InsertCall ( )
        .AddRegArg(mCRegisterBase::mERegisterType_Ebp)
        .ReplaceSize ( 0x1932b - 0x192a2 )
        .RestoreRegister()
        .Hook ( );

    return &GetScriptInit();
}

//
// Entry Point
//

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID )
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        //AllocConsole ( );
        //freopen_s ( ( FILE** )stdout , "CONOUT$" , "w" , stdout );
        ::DisableThreadLibraryCalls( hModule );
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

