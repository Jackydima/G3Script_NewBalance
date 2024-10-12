#include "Script_NewBalance.h"
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"
#include "utility.h"
#include "SharedConfig.h"

// eSSetupEngine[1ea] = AB; eSSetupEngine[1eb] alternative AI
static std::map<bCString , GEU32> PerfektBlockTimeStampMap = {};

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

void LoadSettings ( ) {
    eCConfigFile config = eCConfigFile ( );
    if ( config.ReadFile ( bCString ( "monsterdamage.ini" ) ) ) {
        fMonsterDamageMultiplicator = config.GetFloat ( bCString ( "Game" ) , bCString ( "Game.MonsterDamageMultiplicator" ) , fMonsterDamageMultiplicator );
    }
    if ( config.ReadFile ( "newbalance.ini" ) ) {
        useNewBalanceMagicWeapon = config.GetBool ( "Script" , "UseNewBalanceMagicWeapon" , useNewBalanceMagicWeapon );
        useNewStaminaRecovery = config.GetBool ( "Script" , "UseNewStaminaRecovery" , useNewStaminaRecovery );
        useAlwaysMaxLevel = config.GetBool ( "Script" , "DisableNPCLeveling" , useAlwaysMaxLevel );
        enablePerfectBlock = config.GetBool ( "Script" , "EnablePerfectBlock" , enablePerfectBlock );
        playerOnlyPerfectBlock = config.GetBool ( "Script" , "PlayerOnlyPerfectBlock" , playerOnlyPerfectBlock );
        useNewBalanceMeleeScaling = config.GetBool ( "Script" , "NewMeleeScaling" , useNewBalanceMeleeScaling );
        adjustXPReceive = config.GetBool ( "Script" , "AdjustXPReceive" , adjustXPReceive );
        poiseThreshold = config.GetInt ( "Script" , "PoiseThreshold" , poiseThreshold );
        staminaRecoveryDelay = config.GetU32 ( "Script" , "StaminaRecoveryDelay" , staminaRecoveryDelay );
        staminaRecoveryPerTick = config.GetU32 ( "Script" , "StaminaRecoveryPerTick" , staminaRecoveryPerTick );
        npcArmorMultiplier = config.GetFloat( "Script" , "NPCProtectionMultiplier" , npcArmorMultiplier );
        playerArmorMultiplier = config.GetFloat( "Script" , "PlayerProtectionMultiplier" , playerArmorMultiplier );
        npcWeaponDamageMultiplier = config.GetFloat( "Script" , "NPCWeaponDamageMultiplier" , npcWeaponDamageMultiplier );
        useNewBowMechanics = config.GetBool ( "Script" , "NewBowMechanics" , useNewBowMechanics );
        shootVelocity = config.GetFloat ( "Script" , "ProjectileVelocity" , shootVelocity );
        NPC_AIM_INACCURACY = config.GetFloat ( "Script" , "NPCAimInaccuracy" , NPC_AIM_INACCURACY );
        ATTACK_REACH_MULTIPLIER = config.GetFloat ( "Script" , "AttackReachMultiplier" , ATTACK_REACH_MULTIPLIER );
        startSTR = config.GetU32 ( "Script" , "StartSTR" , startSTR );
        startDEX = config.GetU32( "Script" , "StartDEX" , startDEX );
        blessedBonus = config.GetU32 ( "Script" , "BlessedBonus" , blessedBonus );
        sharpBonus = config.GetU32 ( "Script" , "SharpBonus" , sharpBonus );
        useSharpPercentage = config.GetBool ( "Script" , "UseSharpPercentage" , useSharpPercentage );
        forgedBonus = config.GetU32 ( "Script" , "ForgedBonus" , forgedBonus );
        wornPercentageMalus = config.GetU32 ( "Script" , "WornMalus" , wornPercentageMalus );
        npcArenaSpeedMultiplier = config.GetFloat ( "Script" , "NPCArenaSpeedMultiplier" , npcArenaSpeedMultiplier );
        enableNPCSprint = config.GetBool ( "Script" , "EnableNPCSprint" , enableNPCSprint );
        zombiesCanSprint = config.GetBool ( "Script" , "ZombiesCanSprint" , zombiesCanSprint );
        enableNewTransformation = config.GetBool ( "Script" , "EnableNewTransformation" , enableNewTransformation );
        disableMonsterRage = config.GetBool ( "Script" , "DisableMonsterRage" , disableMonsterRage );
        bossLevel = config.GetU32 ( "Script" , "BossLevelCap" , bossLevel );
        uniqueLevel = config.GetU32 ( "Script" , "UniqueLevelCap" , uniqueLevel );
        eliteLevel = config.GetU32 ( "Script" , "EliteLevelCap" , eliteLevel );
        warriorLevel = config.GetU32 ( "Script" , "WarriorLevelCap" , warriorLevel );
        noviceLevel = config.GetU32 ( "Script" , "NoviceLevelCap" , noviceLevel );
    }

    CFFGFCWnd* test = CFFGFCWnd ( ).GetDesktopWindow();
    CFFGFCView view = CFFGFCView ( 20386 , test );
}

static GEU32 getPerfectBlockLastTime ( bCString iD ) {
    GEU32 worldTime = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
    GEU32 retVal = 0;
    for ( auto it = PerfektBlockTimeStampMap.cbegin ( ); it != PerfektBlockTimeStampMap.cend ( );  ) {
        if ( worldTime - it->second > 400 )
            PerfektBlockTimeStampMap.erase ( it++ );
        else
            ++it;
    }
    try {
        retVal = worldTime - PerfektBlockTimeStampMap.at ( iD );
    }
    catch ( std::exception e ) {
        retVal = ULONG_MAX;
    }
    return retVal;
}
// wird aufgerufen von DoLogicalDamage
gEAction GE_STDCALL AssessHit ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs )
{
    INIT_SCRIPT_EXT ( Victim , Damager );
    UNREFERENCED_PARAMETER ( a_iArgs );

    gCScriptAdmin& ScriptAdmin = GetScriptAdmin ( );
    GEU32 lastHit = getPerfectBlockLastTime ( Victim.GetGameEntity ( )->GetID ( ).GetText ( ) );
    PerfektBlockTimeStampMap[Victim.GetGameEntity ( )->GetID ( ).GetText ( )] = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
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
    auto damagerOwnerDamageReceiver = static_cast< gCDamageReceiver_PS_Ext* >( DamagerOwner.GetGameEntity ( )->GetPropertySet ( eEPropertySetType_DamageReceiver ) );
    auto victimDamageReceiver = static_cast< gCDamageReceiver_PS_Ext* >( Victim.GetGameEntity ( )->GetPropertySet ( eEPropertySetType_DamageReceiver ) );

    /**
    * Workaround: Changed in the FixResetAll() the Trigger of Weapons when functions like all Stumble + PipiStumble get called it would still register Weaponhits 
    * after Trading with no stun
    * After the fix, when Attacker are Stunning Victims the Weapon of the Victim has Attackframes even thought he got stunned before!
    * So here it should ignore Hits, when Attacker is Stunned before but not if he got the PipiStumble.
    */
    switch ( DamagerOwnerAction ) {
    case gEAction_QuickParadeStumble:
    case gEAction_PierceStumble:
    case gEAction_ParadeStumble:
    case gEAction_ParadeStumbleR:
    case gEAction_ParadeStumbleL:
    case gEAction_HeavyParadeStumble:
    case gEAction_QuickStumble:
    case gEAction_Stumble:
    case gEAction_StumbleR:
    case gEAction_StumbleL:
    case gEAction_AbortAttack:
    case gEAction_SitKnockDown:
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
    if ( IsSpellContainerNB ( Damager ) )
    {
        HitForce = gEHitForce_Heavy;
    }
    else if ( IsNormalProjectileNB ( Damager ) )
    {
        // Bow tension, for crossbows always 1.0
        HitForce = Damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( ) >= 0.6f ? gEHitForce_Heavy : gEHitForce_Minimal;
    }
    else if ( DamagerOwner != None )
    {
        GEInt ActionWeaponLevel = GetActionWeaponLevelNB ( DamagerOwner , DamagerOwnerAction );
        HitForce = static_cast< gEHitForce >( ActionWeaponLevel - GetShieldLevelBonusNB ( Victim ) );
        //std::cout << "Went in DamagerOwner Logic for getting Skills\tWeaponLevel: " << ActionWeaponLevel << "\tHitforce: "
           // << HitForce << "\tShieldLevel Bonus: " << GetShieldLevelBonus(Victim) << std::endl;
        HitForce = static_cast< gEHitForce >(HitForce - getMonsterHyperArmorPoints(Victim, VictimAction));
        //std::cout << "HitForce after Monster: " << HitForce << "\n";

        if ((GEInt)HitForce <= poiseThreshold )
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
        if ( BoneName.Contains ( "_Head" , 0 ) ) // Kopfschuß
        {
            isHeadshot = GETrue;
        }
    }
    // New Hyperarmor for New Balancing Spells
    if ( DamagerOwnerAction != gEAction_PierceAttack && DamagerOwnerAction != gEAction_HackAttack && DamagerOwnerAction != gEAction_Summon 
       && !VictimItemTemplateName.Contains("Heal") && !isHeadshot ) {
        HitForce = static_cast< gEHitForce >( HitForce - GetHyperActionBonus ( VictimAction ) );
    }
    GEInt FinalDamage = iWeaponDamage;

    // Headshot? -> Double damage
    // ??? Nahkampf-Treffer am Kopf? => Schaden = Schaden * 2
    if ( isHeadshot ) {
        FinalDamage *= 2;
    }

    // Player attacks, while not being transformed
    if ( Player == DamagerOwner && !Player.NPC.IsTransformed ( ) )
    {
        // PC-ATTR-BONUS
        GEInt iAttributeBonusDamage;

        // Magic damage
        if ( IsSpellContainerNB ( Damager ) )
        {
            // Magic projectiles do double damage, !but are capped to 200.
            if ( IsMagicProjectileNB ( Damager ) )
            {
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
            // setze STR und DEX 100 als default wert an und rechne ohne Bonusschaden am Anfang
            // dafür haben NPC, weniger Rüstung (1.2*NPCMAXLEVEL)
            GEInt dexterity = ScriptAdmin.CallScriptFromScript ( "GetDexterity" , &DamagerOwner , &None , 0 )- startDEX;
            GEInt strength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 )- startSTR;
            GEInt intelligence = ScriptAdmin.CallScriptFromScript ( "GetIntelligence" , &DamagerOwner , &None , 0 );
            // Ranged damage
            if ( IsNormalProjectileNB ( Damager ) == GETrue )
            {
                iAttributeBonusDamage = dexterity / 2;
            }
            // Melee damage
            else {
                GEInt rightWeaponStackIndex = Player.Inventory.FindStackIndex ( gESlot_RightHand );
                GEInt leftWeaponStackIndex = Player.Inventory.FindStackIndex ( gESlot_LeftHand );
                gEUseType playerRightWeaponType = Player.Inventory.GetUseType ( rightWeaponStackIndex );

                //New Scaling
                if ( useNewBalanceMeleeScaling ) {
                    GEChar* arr = ( GEChar* )*( DWORD* )( *( DWORD* )&Player.GetWeapon ( GETrue ).Item + 0x74 ); // A bit Unsafe ...
                    bCString reqAttributeTag = "";
                    if ( arr != nullptr ) reqAttributeTag = bCString ( arr );
                    if ( playerRightWeaponType == gEUseType_1H && Player.Inventory.GetUseType ( leftWeaponStackIndex ) == gEUseType_1H ) {
                        iAttributeBonusDamage = static_cast< GEInt >( strength * 0.3 + dexterity * 0.35 );
                    }
                    else if ( reqAttributeTag.Contains ( "DEX" ) ) {
                        iAttributeBonusDamage = static_cast< GEInt >( strength * 0.2 + dexterity * 0.4 );
                    }
                    else if ( playerRightWeaponType == gEUseType_2H ) {
                        iAttributeBonusDamage = static_cast< GEInt >( strength * 0.55 );
                    }
                    else if ( playerRightWeaponType == gEUseType::gEUseType_Axe || playerRightWeaponType == gEUseType::gEUseType_Pickaxe ) {
                        iAttributeBonusDamage = static_cast< GEInt >( strength * 0.6 );
                    }
                    else if ( playerRightWeaponType == gEUseType_Staff || reqAttributeTag.Contains ( "INT" )
                         || DamagerOwner.Inventory.GetItemFromSlot ( gESlot_RightHand ).Item.GetQuality ( ) & ( 8 + 16 ) ) {
                        iAttributeBonusDamage = static_cast< GEInt >( strength * 0.2 + intelligence * 0.4 + 15 ); // Because you start with low Int, (Assume 60 INT)
                    }
                    else {
                        iAttributeBonusDamage = strength / 2;
                    }
                }
                else {
                    iAttributeBonusDamage = strength / 2;
                }
            }
        }
        if ( iAttributeBonusDamage < 0 )
        {
            iAttributeBonusDamage = 0;
        }
        FinalDamage += ScriptAdmin.CallScriptFromScript ( "GetQualityBonus" , &Damager , &Victim , FinalDamage );
        FinalDamage += iAttributeBonusDamage;
        FinalDamage += ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , FinalDamage );
    }
    // Damager is transformed player or NPC
    else if ( DamagerOwner.Navigation.IsValid ( ) )
    {
        GEInt iStrength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 ) * 2.0f - startSTR * 0.5f; //STR Bonus Real
        if ( iStrength < 10 )
            iStrength = 10;
        //std::cout << "STR NPC: " << iStrength << "\tDamager: " << Damager.GetName() << "\tOwner: " << DamagerOwner.GetName() << "\n";

        // Player is under level 2 and AB is not activated
        /*if ( Player.NPC.GetProperty<PSNpc::PropertyLevel> ( ) < 2
            || !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing
            //  Besitzt der Angreifer keine Waffe?
            || ( Damager.Interaction.GetOwner ( ) == None && Damager.Navigation.IsValid ( ) ) )
            //  Benutzt der Angreifer Fernkampfwaffen?
        {
            FinalDamage = iStrength;
        }
        // NPC attacks with a weapon
        else 
        {*/
            //std::cout << "NPC Strengh: " << iStrength << "\n"; 
            if ( Damager.GetName ( ) == "Fist" && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &DamagerOwner , &None , 0 ) )
            {
                // Greift ein Mensch oder Ork mit Fäusten an?
                FinalDamage = static_cast< GEInt >( iStrength / 2 );
            }
            else
            {
                if ( IsNormalProjectileNB ( Damager ) || IsMagicProjectileNB ( Damager ) || IsSpellContainerNB ( Damager ) ) {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier );
                }
                // Greift ein Ork mit einer Nahkampfwaffe an?
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Orc )
                {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier / 2.0f );
                }
                // Greift ein Mensch mit einer Nahkampfwaffe an?
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Human )
                {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier );
                }
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Demon
                    || DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Ogre )
                {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier / 2.0f );
                }
                else if ( FinalDamage == 0 ) {
                    FinalDamage = iStrength;
                }
                // MonsterAttack
                else if ( Damager.GetName ( ) == "Fist") {
                    FinalDamage = static_cast< GEInt >( iStrength * 1.3f + FinalDamage );
                }
                else {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier );
                }
           // }
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
        if ( IsMagicProjectileNB ( Damager ) || IsSpellContainerNB ( Damager ) )
        {
            FinalDamage *= 2;
        }
    }

    // Vulnerabilities (hooked by Damage.cpp)
    /*if ( DamageTypeEntityTest ( Victim , Damager ) == VulnerabilityStatus_Weak )
    {
        FinalDamage *= 2;
    }*/
    switch ( DamageTypeEntityTestNB ( Victim , Damager ) ) {
    case VulnerabilityStatus_WEAK:
        FinalDamage *= 1.6;
        break;
    case VulnerabilityStatus_STRONG:
        FinalDamage /= 2;
        break;
    case VulnerabilityStatus_SLIGHTLYWEAK:
        FinalDamage *= 1.2;
        break;
    case VulnerabilityStatus_SLIGHTLYSTRONG:
        FinalDamage *= 0.8;
        break;
    }

    if ( ( Damager.IsItem() 
        && ( Damager.Item.GetQuality ( ) & gEItemQuality_Blessed ) == gEItemQuality_Blessed && ScriptAdmin.CallScriptFromScript ( "IsEvil" , &Victim , NULL , 0 )) )
        FinalDamage *= 1.2; 

    if ( GetScriptAdmin().CallScriptFromScript ( "GetStaminaPoints" , &DamagerOwner , &None , 0 ) <= 50 )
        FinalDamage *= 0.7;
    else if ( GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , &DamagerOwner , &None , 0 ) <= 20 )
        FinalDamage *= 0.5;
    //std::cout << "Finaldamage after Vulnerabilities: " << FinalDamage << "\n";

    // Handelt es sich um einen Powercast? (Player and NPCs)
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainerNB ( Damager ) )
    {
        // Powercast
        if ( Damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle> ( ) == gEProjectilePath_Missile )
        {
            FinalDamage *= 2;
            //std::cout << "Finaldamage after Powercast: " << FinalDamage << "\n";
        }
    }
    //std::cout << "FinalDamage: " << FinalDamage << "\n";
    //
    // Schritt 2: Rüstung
    //
    GEInt FinalDamage2;
    GEInt iProtection = ScriptAdmin.CallScriptFromScript ( "GetProtection" , &Victim , &Damager , 0 );
    //std::cout << "Protection of Victim: " << iProtection << "\n";
    GEInt pProtection = iProtection;
    GEInt aProtection = iProtection * 0.25;

    if ( pProtection > 80 ) {
        pProtection = 80;
    }
    
    FinalDamage2 = FinalDamage - aProtection;
    FinalDamage2 = static_cast< GEInt >( ( FinalDamage2 - FinalDamage2 * ( pProtection / 100.0f ) ) );
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

    // New Feature for Percantage Based damage 
    /*
    GEU32 random = Entity::GetRandomNumber ( 1000 );
    if ( !IsMagicProjectileNB ( Damager ) && !IsSpellContainerNB ( Damager ) && !IsNormalProjectileNB(Damager) ) {
        switch ( GetWarriorType ( DamagerOwner ) ) {
        case WarriorType_Novice:
            if ( random > 200 )
                FinalDamage2 *= 0.40;
            break;
        case WarriorType_Warrior:
            if ( random > 400 )
                FinalDamage2 *= 0.40;
            break;
        case WarriorType_Elite:
            if ( random > 600 )
                FinalDamage2 *= 0.40;
            break;
        default:
            if ( random > 100 )
                FinalDamage2 *= 0.40;
        }
    }*/

    //
    // Schritt 3: Angriffsart
    //
    //std::cout << "Action: " << DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) << std::endl;
    if ( DamageTypeEntityTestNB ( Victim , Damager ) == VulnerabilityStatus_IMMUNE && FinalDamage2 > 5)
        FinalDamage2 = 5;

    switch ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) )
    {
        // Angreifer benutzt Quick-Attacke
        //   => Schaden = Schaden / 2
    case gEAction_QuickAttack:
    case gEAction_QuickAttackR:
    case gEAction_QuickAttackL:
        //Quickattacken sind weniger effektiv gegen Starke NPC oder hohe Rüstung (7.5%)
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 0.575f - FinalDamage * 0.075f );
        break;

        // Angreifer benutzt Powerattacke
        //   => Schaden = Schaden * 2 (bei Kampf mit 2 Waffen nicht durchgehend!)
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
        if ( !CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , DamagerOwner )
            || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyStatePosition> ( ) == 2 )
        {
            //Starke Attacken ignorieren 10 % Rüstung
            FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 1.80f + FinalDamage * 0.20f);
        }
        break;

        // Angreifer benutzt Hack-Attacke
        //   => Schaden = Schaden * 2
    case gEAction_HackAttack:
        // Hackattacken ignorieren 15 % Rüstung
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 1.70f + FinalDamage * 0.30f );
        break;
    }
    if ( victimDamageReceiver->GetVulnerableState ( ) == 2 ) {
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 2 );
        if ( HitForce >= 3 ) {
            HitForce = static_cast< gEHitForce >( 4 );
        }
        else {
            HitForce = gEHitForce_Normal;
        }
        victimDamageReceiver->AccessVulnerableState ( ) = 0;
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
    if ( FinalDamage2 < 5 )
        FinalDamage2 = 5;

    if ( Victim == Player )
        Victim.Effect.StopEffect ( GETrue );

    // Parade Magic
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainerNB ( Damager ) )
    {
        if ( ScriptAdmin.CallScriptFromScript ( "CanParadeMagic" , &Victim , &Damager , 0 ) && !Victim.NPC.IsFrozen() )
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
    else if ( IsNormalProjectileNB ( Damager ) )
    {
        if ( ScriptAdmin.CallScriptFromScript ( "CanParadeMissile" , &Victim , &Damager , 0 ) && !Victim.NPC.IsFrozen() )
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
    else if ( !Victim.NPC.IsFrozen ( ) 
        && ( ScriptAdmin.CallScriptFromScript ( "CanParade" , &Victim , &DamagerOwner , 0 )
        || ( Victim.Routine.GetProperty<PSRoutine::PropertyAniState>() == gEAniState_SitKnockDown && GetHeldWeaponCategoryNB (Victim) == gEWeaponCategory_Melee
            && Victim.IsInFOV ( DamagerOwner ) && !IsNormalProjectileNB ( Damager ) && !IsSpellContainerNB ( Damager )) ))
    {
        // Changed to Damage Numbers after Defenses
        GEInt FinalDamage3 = FinalDamage / -2;
        // Reduce damage if parading melee with shield
        if ( CheckHandUseTypesNB ( gEUseType_Shield , gEUseType_1H , Victim ) )
        {
            if ( Victim != Player || !Victim.Inventory.IsSkillActive ( "Perk_Shield_2" ) )
            {
                // Weicht von "Detaillierte Schadenberechnung" ab, dort wird ein Faktor von 2/3 anstatt 0.5 beschrieben.
                FinalDamage3 *= 0.5f;
            }
            else
            {
                FinalDamage3 *= 0.3f;
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

        
        if ( enablePerfectBlock && ( !playerOnlyPerfectBlock || Victim.IsPlayer() ) ) {

            if ( lastHit > 12 && ( Victim.Routine.GetStateTime ( ) < 0.05
                || ( DamagerOwnerAction != gEAction_PowerAttack && DamagerOwnerAction != gEAction_HackAttack && DamagerOwnerAction != gEAction_SprintAttack && Victim.Routine.GetStateTime ( ) < 0.1f ) )
                && Victim.Routine.GetProperty<PSRoutine::PropertyAniState> ( ) == gEAniState_Parade ) {
                PerfektBlockTimeStampMap[Victim.GetGameEntity ( )->GetID ( ).GetText ( )] = 0;
                Victim.Routine.SetStateTime ( 0.0f );
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
                EffectSystem::StartEffect ( "parry_sound_01" , Victim );
                if ( !Damager.GetName ( ).Contains ( "Fist" ) ) {
                    DamagerOwner.NPC.SetCurrentAttacker ( Victim );
                    DamagerOwner.Routine.FullStop ( );
                    DamagerOwner.Routine.SetTask ( "ZS_HeavyParadeStumble" );
                }
                else {
                    DamagerOwner.Routine.SetTask ( "ZS_Stumble" );
                }
                damagerOwnerDamageReceiver->AccessVulnerableState ( ) = 1;
                return gEAction_PierceStumble;
            }
        }

        // Ausdauer und ggf. Lebenspunkte abziehen
        GEInt iStaminaRemaining = FinalDamage3 + ScriptAdmin.CallScriptFromScript ( "GetStaminaPoints" , &Victim , &None , 0 );
        if ( iStaminaRemaining > 0 )
            iStaminaRemaining = 0;
        ScriptAdmin.CallScriptFromScript ( "AddStaminaPoints" , &Victim , &None , FinalDamage3 );
        //Changed back the remaining raw Damage after Def. Reductuion and Stamina consumption#
        GEInt healthDamage = iStaminaRemaining * 2;
        if ( FinalDamage != 0 )
            healthDamage = iStaminaRemaining * 2 * FinalDamage2 / FinalDamage;
        ScriptAdmin.CallScriptFromScript ( "AddHitPoints" , &Victim , &None , healthDamage );

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
        if ( FinalDamage2 >= iVictimHitPoints )
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
        if ( Victim != Player && IsNormalProjectileNB ( Damager ) && Entity::GetCurrentDifficulty ( ) == EDifficulty_Hard )
        {
            FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 1.2f );
        }
    }

    if ( IsNormalProjectileNB ( Damager ) && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ) )
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
        ResetHitPointsRegen ( Victim );
    }

    // Process StatusEffects and Animations:
    // CanBurn
    if ( !Victim.NPC.IsFrozen() && ScriptAdmin.CallScriptFromScript ( "CanBurn" , &Victim , &Damager , FinalDamage2 ) )
    {
        Victim.NPC.EnableStatusEffects ( gEStatusEffect_Burning , GETrue );
        Victim.Effect.StartRuntimeEffect ( "eff_magic_firespell_target_01" );
    }
    // Dead, KnockDown or KnockedOut
    if ( (iVictimHitPoints - FinalDamage2) <= 0 )
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

    if ( ScriptAdmin.CallScriptFromScript ( "CanBePoisoned" , &Victim , &Damager , DamagerOwnerAction == gEAction_PierceAttack || DamagerOwnerAction == gEAction_HackAttack ) )
    {
        Victim.NPC.EnableStatusEffects ( gEStatusEffect_Poisoned , GETrue );
        auto damageReceiver = static_cast< gCDamageReceiver_PS_Ext* >( Victim.GetGameEntity ( )->GetPropertySet ( eEPropertySetType_DamageReceiver ));
        damageReceiver->AccessPoisonDamage ( ) = GetPoisonDamage(DamagerOwner);
    }

    if ( ScriptAdmin.CallScriptFromScript ( "CanBeDiseased" , &Victim , &Damager , 0 ) )
    {
        if ( Damager.GetName ( ).Contains ( "Fist" ) 
            || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction>() == gEAction_HackAttack 
            || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_PierceAttack ) {
            Victim.NPC.EnableStatusEffects ( gEStatusEffect_Diseased , GETrue );
        }
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

    // Nostun Of animals when they are Mad
    if ( !disableMonsterRage && Victim.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_SprintAttack )
    {
        return gEAction_None;
    }

    //Stun Protection
    if ( DamageTypeEntityTestNB ( Victim , Damager ) == VulnerabilityStatus_IMMUNE )
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
    // Freeze Reduced Timer on Hit

    // Scream or make HitEffect, but no Stumble also processes logic when you hit someone, like setting up combat mode
    if ( HitForce <= gEHitForce_Minimal )
    {
        if ( VictimAction == gEAction_PierceStumble ) {
            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_QuickStumble" );
            return gEAction_QuickStumble;
        }
        ScriptAdmin.CallScriptFromScript ( "PipiStumble" , &Victim , &None , 0 );
        return DamagerOwnerAction;
    }

    if ( /*GetHeldWeaponCategory ( Victim ) != gEWeaponCategory_None && */ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 )) //Geändert
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

void AddNewEffect ( ) {
    //EffectModulePtr
    DWORD EffectModulePtr = ( ( DWORD ( * )( void ) )( RVA_Game ( 0x601f0 ) ) ) ( );
    //std::cout << "EffectModule Pointer: " << EffectModulePtr << "\n";
    if ( EffectModulePtr == 0 )
        return;
    DWORD gCEffectSystemPtr = EffectModulePtr + 0x14;
    if ( gCEffectSystemPtr == 0 )
        return;
    gCEffectMap* EffectMap = ( gCEffectMap* )( *( DWORD* )gCEffectSystemPtr + 0x4 );
    if ( EffectMap == nullptr )
        return;
    gCEffectMap NewEM;
    NewEM.Load ( "g3-new.efm" );
    for ( auto iter = NewEM.Begin ( ); iter != NewEM.End ( ); iter++ ) {
        //std::cout << "EffectName: " << iter.GetKey ( ) << "\n";
        EffectMap->RemoveAt ( iter.GetKey ( ) );
        gCEffectCommandSequence* effectCommand = EffectMap->InsertNewAt ( iter.GetKey ( ) );
        *effectCommand = iter.GetNode ( )->m_Element;
    }
    //std::cout << "EffectMap Merged " << "\n";
}

extern "C" __declspec( dllexport )
gSScriptInit const * GE_STDCALL ScriptInit( void )
{
    // Ensure that that Script_Game.dll is loaded.
    GetScriptAdmin ( ).LoadScriptDLL ( "Script_Game.dll" );
    LoadSettings ( );
    PatchCode ( );
    AddNewEffect ( );

    HookFunctions ( );
    HookCallHooks ( ); 

    //Hook_Shoot  
    //    .Prepare ( RVA_ScriptGame ( 0x86450 ) , &Shoot )
    //    .Hook ( );

    static mCFunctionHook Hook_Assesshit;
    Hook_Assesshit.Hook ( GetScriptAdminExt ( ).GetScript ( "AssessHit" )->m_funcScript , &AssessHit , mCBaseHook::mEHookType_OnlyStack );

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

