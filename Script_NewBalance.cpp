#include "Script_NewBalance.h"
#include <iostream>
#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
//#include "util/ScriptUtil.h"
#include "Script.h"
#include "utility.h"

// eSSetupEngine[1ea] = AB; eSSetupEngine[1eb] alternative AI
static std::map<bCString , GEU32> PerfektBlockTimeStampMap = {};
static std::map<bCString , GEU32> LastStaminaUsageMap = {};
static GEBool useNewBalanceMagicWeapon = GEFalse;
static GEBool useNewStaminaRecovery = GETrue;
static GEU32 staminaRecoveryDelay = 20;
static GEU32 staminaRecoveryPerTick = 13;
static GEFloat fMonsterDamageMultiplicator = 0.5;
static GEDouble npcArmorMultiplier = 1.2;
static GEDouble* npcArmorMultiplierPtr = &npcArmorMultiplier;
static GEU32 startSTR = 100;
static GEU32 startDEX = 100;

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
        staminaRecoveryDelay = config.GetU32 ( "Script" , "StaminaRecoveryDelay" , staminaRecoveryDelay );
        staminaRecoveryPerTick = config.GetU32 ( "Script" , "StaminaRecoveryPerTick" , staminaRecoveryPerTick );
        npcArmorMultiplier = config.GetFloat( "Script" , "NPCProtectionMultiplier" , npcArmorMultiplier );
        startSTR = config.GetU32 ( "Script" , "StartSTR" , startSTR );
        startDEX = config.GetU32( "Script" , "StartDEX" , startDEX );
    }
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

static GEU32 getLastStaminaUsageTime ( bCString iD ) {
    GEU32 worldTime = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
    GEU32 retVal = 0;
    for ( auto it = LastStaminaUsageMap.cbegin ( ); it != LastStaminaUsageMap.cend ( ); ) {
        if ( worldTime - it->second > 400 )
            LastStaminaUsageMap.erase ( it++ );
        else
            ++it;
    }
    try {
        retVal = worldTime - LastStaminaUsageMap.at ( iD );
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
        //std::cout << "BoneName: " << BoneName << "\n";
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
        if ( IsSpellContainerNB ( Damager ) )
        {
            // Magic projectiles do double damage, !but are capped to 200.
            if ( IsMagicProjectileNB ( Damager ) )
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
                else if ( playerRightWeaponType == gEUseType_2H ) {
                    iAttributeBonusDamage = static_cast< GEInt >( strength * 0.55 );
                }
                else if ( playerRightWeaponType == gEUseType::gEUseType_Axe || playerRightWeaponType == gEUseType::gEUseType_Pickaxe ) {
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
        FinalDamage += iAttributeBonusDamage;
        FinalDamage += ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , FinalDamage );
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
                // Greift ein Mensch oder Ork mit Fäusten an?
                FinalDamage = static_cast< GEInt >( iStrength / 2 );
                //std::cout << "Finaldamage after Fists: " << FinalDamage << "\n";
            }
            else
            {
                if ( IsNormalProjectileNB ( Damager ) || IsMagicProjectileNB ( Damager ) || IsSpellContainerNB ( Damager ) ) {
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
        if ( IsMagicProjectileNB ( Damager ) || IsSpellContainerNB ( Damager ) )
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

    //
    // Schritt 2: Rüstung
    //
    GEInt FinalDamage2;
    GEInt iProtection = ScriptAdmin.CallScriptFromScript ( "GetProtection" , &Victim , &Damager , 0 );
    //std::cout << "Protection of Victim: " << iProtection << "\n";
    // If AB Active, it reduces the Bonus of Armor with 0.2, because it used another system here
    // Only a hotfix for now!
    if ( eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing && Victim == Player && !Victim.NPC.IsTransformed()) {
        iProtection *= 2;
    }
    if ( iProtection > 90 )
        iProtection = 90;

    FinalDamage2 = static_cast< GEInt >( ( FinalDamage - FinalDamage * ( iProtection / 100.0f ) ) );
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
    if ( DamageTypeEntityTestNB ( Victim , Damager ) == VulnerabilityStatus_IMMUNE && FinalDamage2 > 5)
        FinalDamage2 = 5;

    switch ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) )
    {
        // Angreifer benutzt Quick-Attacke
        //   => Schaden = Schaden / 2
    case gEAction_QuickAttack:
    case gEAction_QuickAttackR:
    case gEAction_QuickAttackL:
        //Quickattacken sind weniger effektiv gegen Starke NPC oder hohe Rüstung (5%)
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 0.55f - FinalDamage * 0.05f );
        break;

        // Angreifer benutzt Powerattacke
        //   => Schaden = Schaden * 2 (bei Kampf mit 2 Waffen nicht durchgehend!)
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
        if ( !CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , DamagerOwner )
            || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyStatePosition> ( ) == 2 )
        {
            //Starke Attacken ignorieren 7.5 % Rüstung
            FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 1.85f + FinalDamage * 0.15f);
        }
        break;

        // Angreifer benutzt Hack-Attacke
        //   => Schaden = Schaden * 2
    case gEAction_HackAttack:
        // Hackattacken ignorieren 10 % Rüstung
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 1.80f + FinalDamage * 0.20f );
        break;
    }
    if ( victimDamageReceiver->GetVulnerableState ( ) == 2 ) {
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * 2 );
        victimDamageReceiver->AccessVulnerableState ( ) = 0;
    }
    //std::cout << "Finaldamage2 after AttackType: " << FinalDamage2 << "\n";

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
    else if ( IsNormalProjectileNB ( Damager ) )
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
    else if ( !Victim.NPC.IsFrozen ( ) && ( ScriptAdmin.CallScriptFromScript ( "CanParade" , &Victim , &DamagerOwner , 0 )
        || ( Victim.Routine.GetProperty<PSRoutine::PropertyAniState>() == gEAniState_SitKnockDown && GetHeldWeaponCategoryNB (Victim) == gEWeaponCategory_Melee
            && Victim.IsInFOV ( DamagerOwner ) && !IsNormalProjectileNB ( Damager ) && !IsSpellContainerNB ( Damager ) )
        || ( Victim.Routine.GetProperty<PSRoutine::PropertyAniState> ( ) == gEAniState_Parade && Victim.Routine.GetStateTime ( ) < 0.05 ) ))
    {
        /*
            TODO: Get here the check for lastHit and ignore last
        */
        GEU32 lastHit = getPerfectBlockLastTime ( Victim.GetGameEntity ( )->GetID ( ).GetText ( ) );
        PerfektBlockTimeStampMap[Victim.GetGameEntity ( )->GetID ( ).GetText ( )] = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
        // Changed to Damage Numbers after Defenses
        GEInt FinalDamage3 = FinalDamage / -2;
        // Reduce damage if parading melee with shield
        if ( CheckHandUseTypesNB ( gEUseType_Shield , gEUseType_1H , Victim ) )
        {
            if ( Victim != Player || !Victim.Inventory.IsSkillActive ( "Perk_Shield_2" ) )
            {
                // Weicht von "Detaillierte Schadenberechnung" ab, dort wird ein Faktor von 2/3 anstatt 0.5 beschrieben.
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

        //std::cout << "StateTime: " << Victim.Routine.GetStateTime ( ) << "\t AniState: " << Victim.Routine.GetProperty<PSRoutine::PropertyAniState>() <<"\n";
        //std::cout << "LastHit: " << lastHit << "\tVictimID: " << Victim.GetGameEntity()->GetID().GetText() << "\tName: " << Victim.GetName() << "\tSize Map: " << PerfektBlockTimeStampMap.size() << "\n";
        if ( lastHit > 12 && (Victim.Routine.GetStateTime ( ) < 0.05 
            || (DamagerOwnerAction != gEAction_PowerAttack && DamagerOwnerAction!=gEAction_HackAttack && DamagerOwnerAction != gEAction_SprintAttack && Victim.Routine.GetStateTime ( ) < 0.1) ) 
            && Victim.Routine.GetProperty<PSRoutine::PropertyAniState> ( ) == gEAniState_Parade ) {
            PerfektBlockTimeStampMap[Victim.GetGameEntity ( )->GetID ( ).GetText ( )] = 0;
            Victim.Routine.SetStateTime ( 0.0 );
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
            if ( !Damager.GetName ( ).Contains ( "Fist" ) ) {
                DamagerOwner.NPC.SetCurrentAttacker ( Victim );
                DamagerOwner.Routine.FullStop ( );
                DamagerOwner.Routine.SetTask ( "ZS_HeavyParadeStumble" );
            }
            else {
                DamagerOwner.Routine.SetTask ( "ZS_Stumble" );
            } 
            damagerOwnerDamageReceiver->AccessVulnerableState ( ) = 1;
            return VictimAction;
        }

        GEInt iStaminaRemaining = FinalDamage3 + ScriptAdmin.CallScriptFromScript ( "GetStaminaPoints" , &Victim , &None , 0 );
        if ( iStaminaRemaining > 0 )
            iStaminaRemaining = 0;
        ScriptAdmin.CallScriptFromScript ( "AddStaminaPoints" , &Victim , &None , FinalDamage3 );
        //Changed back the remaining raw Damage after Def. Reductuion and Stamina consumption
        GEInt healthDamage = iStaminaRemaining * 2 * FinalDamage2 / FinalDamage;
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

static mCFunctionHook Hook_AddStaminaPoints;
GEInt AddStaminaPoints ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEI32 a_iArgs ) {
    INIT_SCRIPT_EXT ( Self , Other );
    if ( a_iArgs < 0 ) {
        LastStaminaUsageMap[Self.GetGameEntity ( )->GetID ( ).GetText ( )] = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
    }

    /*if ( a_iArgs > 0 && getLastStaminaUsageTime ( Self.GetGameEntity ( )->GetID ( ).GetText ( ) ) < 20 )
        return 0;*/
    //std::cout << "Name: " << Self.GetName ( ) << ":\tLastUsage: " << getLastStaminaUsageTime ( Self.GetGameEntity ( )->GetID ( ).GetText ( ) ) << "\n";
    return Hook_AddStaminaPoints.GetOriginalFunction ( &AddStaminaPoints )( a_pSPU , a_pSelfEntity , a_pOtherEntity , a_iArgs );
}

static mCFunctionHook Hook_StaminaUpdateOnTick;
GEInt StaminaUpdateOnTick ( Entity p_entity ) {
    //std::cout << "Lengh of List: " << LastStaminaUsageMap.size ( ) << "\n";
    const GEInt standardStaminaRecovery = staminaRecoveryPerTick;
    GEInt retStaminaDelta = 0;

    if ( p_entity.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction::gEAction_Aim ) {
        if ( GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , &p_entity , &None , 0 ) <= 7 ) {
            p_entity.Routine.FullStop ( );
            p_entity.Routine.SetState ( "PS_Normal" );
            bCString aniname = p_entity.GetAni ( gEAction_AbortAttack , gEPhase::gEPhase_Begin );
            p_entity.StartPlayAni( aniname,0,GETrue,0,GEFalse);
        }
        return StaminaUpdateOnTickHelper ( p_entity , -7 );
    }

    if ( p_entity.IsSprinting ( ) ||  p_entity == Entity::GetPlayer ( ) && ( p_entity.IsSwimming ( ) && *( BYTE* )RVA_Executable ( 0x27FD2 ) ) ) {
        if ( p_entity.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Bloodfly ) {
            return StaminaUpdateOnTickHelper (p_entity, -1 );
        }

        if ( eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing ) {
            if ( p_entity.Inventory.IsSkillActive(Template("Perk_Sprinter") ))
                return StaminaUpdateOnTickHelper ( p_entity , -4 );
            return StaminaUpdateOnTickHelper ( p_entity , -8 );
        }
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Sprinter" ) ) )
            return StaminaUpdateOnTickHelper ( p_entity , -5 );
        return StaminaUpdateOnTickHelper ( p_entity , -10 );
    }

    if (p_entity.IsJumping() )
        return StaminaUpdateOnTickHelper ( p_entity , 0 );

    if (p_entity.NPC.IsDiseased( ) )
        return StaminaUpdateOnTickHelper ( p_entity , 1 );

    // HoldingBlockFlag 0x118ab0
    if ( *(BYTE*)RVA_ScriptGame(0x118ab0) && eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing )
        return StaminaUpdateOnTickHelper ( p_entity , 1 );
    typedef GEU32(GetWeatherAdmin)( void );
    // Get eCWeatherAdmin *! also available at RVA_ScriptGame(0x11a210)
    GetWeatherAdmin* getWeatherAdminFunction = ( GetWeatherAdmin*)RVA_ScriptGame ( 0x12e0 );

    GEU32 weatherAdmin = getWeatherAdminFunction ( );
    // Or Temperatur
    GEFloat weatherCondition = *( GEFloat* )( weatherAdmin + 0xd0 );

    //Maybe Add more complex logic for Npcs aswell bro
    if ( weatherCondition >= 40.0 ) {
        if (p_entity.IsPlayer() && !p_entity.Inventory.IsSkillActive(Template("Perk_ResistHeat") ) )
            return StaminaUpdateOnTickHelper ( p_entity , 2 );
        return StaminaUpdateOnTickHelper ( p_entity , standardStaminaRecovery );
    }

    if ( weatherCondition <= -40.0 ) {
        if ( p_entity.IsPlayer ( ) && !p_entity.Inventory.IsSkillActive ( Template ( "Perk_ResistCold" ) ) )
            return StaminaUpdateOnTickHelper ( p_entity , 2 );
        return StaminaUpdateOnTickHelper ( p_entity , standardStaminaRecovery );
    }

    return StaminaUpdateOnTickHelper ( p_entity , standardStaminaRecovery );
}

GEInt StaminaUpdateOnTickHelper (Entity& p_entity, GEInt p_staminaValue) {
    if ( p_staminaValue > 0 && getLastStaminaUsageTime ( p_entity.GetGameEntity ( )->GetID ( ).GetText ( ) ) < staminaRecoveryDelay )
        return 0;
    return p_staminaValue;
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
    Template projectile = getProjectile ( *self , leftHandUseType );
    GEInt stack = self->Inventory.AssureItems ( projectile , 0 , random + 10 );
    *(GEInt*)( registerBaseStack - 0x2C4 ) = stack;
}

static mCFunctionHook Hook_GetAttituteSummons;

GEInt GetAttitudeSummons ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {
    INIT_SCRIPT_EXT ( Self , Other );
    gCScriptAdmin& ScriptAdmin = GetScriptAdmin ( );

    if ( Self.Party.GetPartyLeader() != None && Self.Party.GetPartyLeader ( ) == Other.Party.GetPartyLeader ( ) )
        return 1;

    if ( Self.Party.GetProperty<PSParty::PropertyPartyMemberType> ( ) == gEPartyMemberType_Summoned && Self.Party.GetPartyLeader() != Other && Self.Party.GetPartyLeader().NPC.GetCurrentTarget ( ) != Other ) {
        return ScriptAdmin.CallScriptFromScript ( "GetAttitude" , &Self.Party.GetPartyLeader () , &Other , a_iArgs );
    }
    if ( Other.Party.GetProperty<PSParty::PropertyPartyMemberType> ( ) == gEPartyMemberType_Summoned && Other.Party.GetPartyLeader ( ) != Self && Other.Party.GetPartyLeader ( ).NPC.GetCurrentTarget ( ) != Self ) {
        return ScriptAdmin.CallScriptFromScript ( "GetAttitude" , &Self , &Other.Party.GetPartyLeader () , a_iArgs );
    }
    return Hook_GetAttituteSummons.GetOriginalFunction(&GetAttitudeSummons)( a_pSPU , a_pSelfEntity , a_pOtherEntity , a_iArgs );
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

/**
* PatchCode is for small Codefixes
* These should be changed -- Use the G3 SDK Hookfunctions to edit ASM Code.
*/
void PatchCode () {
    DWORD currProt , newProt;
    /**
    * 0xb5045 - 0xb503d 
    * Call for DamageEntityTest in DoLogicalDamage removed ( not ingnorig the Immune Status anymore )
    */
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb503d ) , 0xb5045 - 0xb503d , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb503d ) , 0x90 , 0xb5045 - 0xb503d );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb503d ) , 0xb5045 - 0xb503d , currProt , &newProt );

    /**
    * Remove the Limiter on Block for the Player via simple Bytejmp patch
    */
    BYTE patchcode[] = {0xE9,0x2B,0x02,0x00,0x00};
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x63359 ) , 0x63365 - 0x63359 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x63359 ) , 0x90 , 0x63365 - 0x63359 );
    memcpy ( ( LPVOID )RVA_ScriptGame ( 0x6335f ) , patchcode, sizeof(patchcode)/sizeof(BYTE));
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x63359 ) , 0x63365 - 0x63359 , currProt , &newProt );

    /**
    * Change the Protection Multiplier for NPCs to npcArmorMultiplier (1.2)
    * 
    */
    //std::cout << "Adr adress: " << npcArmorMultiplierPtr << "\tFloat: " << npcArmorMultiplier << "\n";
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x34656 ) , 0x3465a - 0x34656 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x34656 ) , 0x90 , 0x3465a - 0x34656 );
    memcpy ( ( LPVOID )RVA_ScriptGame ( 0x34656 ) , &npcArmorMultiplierPtr , sizeof ( &npcArmorMultiplierPtr ) );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x34656 ) , 0x3465a - 0x34656 , currProt , &newProt );

    /** Remove HitProt when Entity is SitDowned
    * 0xb51c0 - 0xb51b1
    * Is gEAnimationState Knockdown? 
    */ 
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb51b1 ) , 0xb51c0 - 0xb51b1 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb51b1 ) , 0x90 , 0xb51c0 - 0xb51b1 );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb51b1 ) , 0xb51c0 - 0xb51b1 , currProt , &newProt );

    /**
    * Remove the Targetlimitation of Pierce- and Hack-Attacks on the Currentarget
    * 0xb51db - 0xb51cd
    */
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb51cd ) , 0xb51dc - 0xb51cd , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb51cd ) , 0x90 , 0xb51d0 - 0xb51cd ); // Remove Compare for Hackattack
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb51d6 ) , 0x90 , 0xb51db - 0xb51d6 ); // Remove Jump for Hackattack and PierceAttack Compare
    memset ( ( LPVOID )RVA_ScriptGame ( 0xb51db ) , 0xEB , 1 ); // JMP always
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xb51cd ) , 0xb51dc - 0xb51cd , currProt , &newProt );
    /*
    //0xac2e6
    // Test Quality Worn
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xac2e6 ) , 0xac2ec - 0xac2e6 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xac2e6 ) , 0x90 , 0xac2ec - 0xac2e6 );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xac2e6 ) , 0xac2ec - 0xac2e6 , currProt , &newProt );*/
}

static mCFunctionHook Hook_CanBurn;
static mCFunctionHook Hook_CanFreeze;
/**
* Addition to the CanFreeze for new Spell! 
* Always return true on IceBlock spell
*/ 
GEInt CanFreezeAddition ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {
    // Fix for new Spell
    if ( a_pOtherEntity->GetName ( ) == "Mis_IceBlock" )
        return GETrue;
    return Hook_CanFreeze.GetOriginalFunction ( &CanFreezeAddition )( a_pSPU , a_pSelfEntity , a_pOtherEntity , a_iArgs );
}

GEInt IsEvil ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {
    INIT_SCRIPT_EXT ( Self , Other );
    if ( GetScriptAdmin ( ).CallScriptFromScript ( "IsUndead" , &Self , &None , 0 ) )
        return 1;
    switch ( Self.NPC.GetProperty<PSNpc::PropertySpecies> ( ) ) {
    case gESpecies_Golem:
    case gESpecies_Demon:
    case gESpecies_Gargoyle:
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_ScorpionKing:
    // New Check for Dragon!
    case gESpecies_Dragon:
        return 1;
    default:
        return 0;
    }
    //return 0;
}

static mCCallHook Hook_GiveXPPowerlevel;

void GiveXPPowerlevel ( gCNPC_PS* p_npc ) {
    Entity entity = p_npc->GetEntity ( );
    GEInt powerLevel = getPowerLevel ( entity );
    Hook_GiveXPPowerlevel.SetImmEax ( powerLevel );
}

static mCFunctionHook Hook_Shoot;

GEInt Shoot ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {
    INIT_SCRIPT_EXT ( Self , Other );
    if ( GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , &Self , &None , 0 ) <= 0 ) {
        Self.Routine.FullStop ( );
        Self.Routine.SetState ( "PS_Normal" );
        //bCString aniname = p_entity.GetAni ( gEAction_AbortAttack , gEPhase::gEPhase_Begin );
        //p_entity.StartPlayAni ( aniname , 0 , GETrue , 0 , GEFalse );
        return 0;
    }
    return Hook_Shoot.GetOriginalFunction ( &Shoot )( a_pSPU , a_pSelfEntity, a_pOtherEntity, a_iArgs );
}


/*static mCCallHook Hook_NPCAnimationSpeed;
void NPCAnimationSpeed ( Entity* p_entity, GEFloat * p_speedMult ) {
    GEInt staminapoints = GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , p_entity , &None , 0 );
    if ( staminapoints <= 20 )
        *p_speedMult *= 0.75;
    if ( staminapoints <= 50 )
        *p_speedMult *= 0.875;
}*/


extern "C" __declspec( dllexport )
gSScriptInit const * GE_STDCALL ScriptInit( void )
{
    // Ensure that that Script_Game.dll is loaded.
    GetScriptAdmin().LoadScriptDLL("Script_Game.dll");

    LoadSettings ( );
    ResetAllFix ( );
    PatchCode ( );
    PatchCode1 ( );
 
    // If G3Fixes is installed use them
    GetScriptAdmin().LoadScriptDLL("Script_G3Fixes.dll");
    if ( !GetScriptAdmin ( ).IsScriptDLLLoaded ( "Script_G3Fixes.dll" ) || useNewBalanceMagicWeapon ) {
        Hook_CanBurn.Hook ( GetScriptAdminExt ( ).GetScript ( "CanBurn" )->m_funcScript , &CanBurn , mCBaseHook::mEHookType_OnlyStack );
        Hook_CanFreeze.Hook ( GetScriptAdminExt ( ).GetScript ( "CanFreeze" )->m_funcScript, &CanFreeze , mCBaseHook::mEHookType_OnlyStack );
    }
    else {
        Hook_CanFreeze.Hook ( GetScriptAdminExt ( ).GetScript ( "CanFreeze" )->m_funcScript , &CanFreezeAddition , mCBaseHook::mEHookType_OnlyStack );
    }

    if ( useNewStaminaRecovery ) {
        Hook_AddStaminaPoints.Hook ( GetScriptAdminExt ( ).GetScript ( "AddStaminaPoints" )->m_funcScript , &AddStaminaPoints );

        Hook_StaminaUpdateOnTick
            .Prepare ( RVA_ScriptGame ( 0xb0520 ) , &StaminaUpdateOnTick , mCBaseHook::mEHookType_OnlyStack )
            .Hook ( );
    }

    /*Hook_NPCAnimationSpeed
        .Prepare ( RVA_ScriptGame ( 0x4d9a ) , &NPCAnimationSpeed , mCBaseHook::mEHookType_OnlyStack , mCRegisterBase::mERegisterType_Ebp )
        .InsertCall ( )
        .AddStackArgEbp ( 0x8 )
        .AddPtrStackArg ( 0x0C )
        .ReplaceSize( 0x4daa - 0x4d9a )
        .RestoreRegister ( )
        .Hook ( );*/

    static mCFunctionHook Hook_Assesshit;
    static mCFunctionHook Hook_IsEvil;
    static mCFunctionHook Hook_GetAnimationSpeedModifier;
    static mCFunctionHook Hook_OnPowerAim_Loop;
    static mCFunctionHook Hook_CanParade;
    static mCFunctionHook Hook_UpdateHitPointsOnTick;

    HookFunctions ( );

    Hook_UpdateHitPointsOnTick
        .Prepare ( RVA_ScriptGame ( 0xb0360 ) , &UpdateHitPointsOnTick )
        .Hook ( );

    GetScriptAdmin ( ).LoadScriptDLL ( "Script_OptionalGuard.dll" );
    if ( !GetScriptAdmin ( ).IsScriptDLLLoaded ( "Script_OptionalGuard.dll" ) ) {
        Hook_CanParade
            .Prepare ( RVA_ScriptGame ( 0xd480 ) , &CanParade , mCBaseHook::mEHookType_OnlyStack )
            .Hook ( );
    }

    Hook_GetAnimationSpeedModifier
        .Prepare ( RVA_ScriptGame ( 0x42a0 ) , &GetAnimationSpeedModifier )
        .Hook ( );

    Hook_OnPowerAim_Loop
        .Prepare ( RVA_ScriptGame ( 0x84b90 ), &OnPowerAim_Loop )
        .Hook ( );

    Hook_CombatMoveScale
        .Prepare ( RVA_Game ( 0x16b8a3 ) , &CombatMoveScale, mCBaseHook::mEHookType_Mixed, mCRegisterBase::mERegisterType_Ecx )
        .InsertCall ( )
        .AddPtrStackArgEbp ( 0x8 )
        .AddPtrStackArgEbp ( 0xC )
        .AddRegArg ( mCRegisterBase::mERegisterType_Ecx )
        .RestoreRegister ( )
        .Hook ( );

    Hook_Shoot  
        .Prepare ( RVA_ScriptGame ( 0x86450 ) , &Shoot )
        .Hook ( );

    Hook_Shoot_Velocity
        .Prepare ( RVA_ScriptGame ( 0x8680a ) , &Shoot_Velocity )
        .InsertCall ( )
        .AddPtrStackArgEbp ( 0x8 )
        .AddPtrStackArgEbp ( 0xC )
        .AddPtrStackArgEbp ( 0x10 )
        .AddStackArg(0xB8)
        .RestoreRegister ( )
        .Hook ( );

    Hook_GiveXPPowerlevel
        .Prepare ( RVA_ScriptGame ( 0x4e451 ) , &GiveXPPowerlevel , mCBaseHook::mEHookType_Mixed , mCRegisterBase::mERegisterType_Eax )
        .InsertCall ( )
        .AddPtrStackArg( 0x11c )
        .ReplaceSize ( 0x4e45a - 0x4e451 )
        .RestoreRegister ( )
        .Hook ( );

    Hook_GetAttituteSummons.Hook( GetScriptAdminExt ( ).GetScript ( "GetAttitude" )->m_funcScript , &GetAttitudeSummons );

    Hook_IsEvil.Hook ( GetScriptAdminExt ( ).GetScript ( "IsEvil" )->m_funcScript , &IsEvil );

    Hook_Assesshit.Hook ( GetScriptAdminExt ( ).GetScript ( "AssessHit" )->m_funcScript , &AssessHit , mCBaseHook::mEHookType_OnlyStack );

    Hook_AssureProjectiles
        .Prepare ( RVA_ScriptGame ( 0x192a2 ) , &AssureProjectiles , mCBaseHook::mEHookType_Mixed , mCRegisterBase::mERegisterType_Ebx )
        .InsertCall ( )
        .AddRegArg ( mCRegisterBase::mERegisterType_Ebp )
        .ReplaceSize ( 0x1932b - 0x192a2 )
        .RestoreRegister ( )
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

