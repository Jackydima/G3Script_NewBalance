#include "Script_NewBalance.h"


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
        useHardCoreAttacks = config.GetBool ( "Script" , "UseHardCoreAttacks" , useHardCoreAttacks );
        useDamagingInnosLight = config.GetBool ( "Script" , "UseDamagingInnosLight" , useDamagingInnosLight );
        useNewStaminaRecovery = config.GetBool ( "Script" , "UseNewStaminaRecovery" , useNewStaminaRecovery );
        useAlwaysMaxLevel = config.GetBool ( "Script" , "DisableNPCLeveling" , useAlwaysMaxLevel );
        enablePerfectBlock = config.GetBool ( "Script" , "EnablePerfectBlock" , enablePerfectBlock );
        playerOnlyPerfectBlock = config.GetBool ( "Script" , "PlayerOnlyPerfectBlock" , playerOnlyPerfectBlock );
        useNewBalanceMeleeScaling = config.GetBool ( "Script" , "NewMeleeScaling" , useNewBalanceMeleeScaling );
        adjustXPReceive = config.GetBool ( "Script" , "AdjustXPReceive" , adjustXPReceive );
        useStaticBlocks = config.GetBool ( "Script" , "UseStaticBlocks" , useStaticBlocks );
        useStrengthForCrossbows = config.GetBool ( "Script" , "UseStrengthForCrossbows" , useStrengthForCrossbows );

        PerfectBlockDamageMult = config.GetFloat ( "Script" , "PerfectBlockDamageMult" , PerfectBlockDamageMult );
        PowerAttackArmorPen = config.GetFloat ( "Script" , "PowerAttackArmorPen" , PowerAttackArmorPen );
        QuickAttackArmorRes = config.GetFloat ( "Script" , "QuickAttackArmorRes" , QuickAttackArmorRes );
        SpecialAttackArmorPen = config.GetFloat ( "Script" , "SpecialAttackArmorPen" , SpecialAttackArmorPen );
        NPCStrengthMultiplicator = config.GetFloat ( "Script" , "NPCStrengthMultiplicator" , NPCStrengthMultiplicator );
        NPCStrengthCorrection = config.GetFloat ( "Script" , "NPCStrengthCorrection" , NPCStrengthCorrection );
        elementalPerkBonusResistance = config.GetInt ( "Script" , "ElementalPerkBonusResistance" , elementalPerkBonusResistance );
        animationSpeedBonusMid = config.GetFloat ( "Script" , "BowAnimationSpeedBonusMid" , animationSpeedBonusMid );
        animationSpeedBonusHigh = config.GetFloat ( "Script" , "BowAnimationSpeedBonusHigh" , animationSpeedBonusHigh );

        NPCDamageReductionMultiplicator = config.GetFloat ( "Script" , "NPCDamageReductionMultiplicator" , NPCDamageReductionMultiplicator );
        poiseThreshold = config.GetInt ( "Script" , "PoiseThreshold" , poiseThreshold );
        staminaRecoveryDelay = config.GetInt ( "Script" , "StaminaRecoveryDelay" , staminaRecoveryDelay );
        staminaRecoveryPerTick = config.GetInt ( "Script" , "StaminaRecoveryPerTick" , staminaRecoveryPerTick );
        npcArmorMultiplier = config.GetFloat( "Script" , "NPCProtectionMultiplier" , npcArmorMultiplier );
        playerArmorMultiplier = config.GetFloat( "Script" , "PlayerProtectionMultiplier" , playerArmorMultiplier );
        npcWeaponDamageMultiplier = config.GetFloat( "Script" , "NPCWeaponDamageMultiplier" , npcWeaponDamageMultiplier );
        useNewBowMechanics = config.GetBool ( "Script" , "NewBowMechanics" , useNewBowMechanics );
        attackRangeAI = config.GetFloat ( "Script" , "AttackRangeAI" , attackRangeAI );
        telekinesisRange = config.GetFloat ( "Script" , "TelekinesisRange" , telekinesisRange );
        shootVelocity = config.GetFloat ( "Script" , "ProjectileVelocity" , shootVelocity );
        NPC_AIM_INACCURACY = config.GetFloat ( "Script" , "NPCAimInaccuracy" , NPC_AIM_INACCURACY );
        ATTACK_REACH_MULTIPLIER = config.GetFloat ( "Script" , "AttackReachMultiplier" , ATTACK_REACH_MULTIPLIER );
        startSTR = config.GetInt ( "Script" , "StartSTR" , startSTR );
        startDEX = config.GetInt ( "Script" , "StartDEX" , startDEX );
        blessedBonus = config.GetInt ( "Script" , "BlessedBonus" , blessedBonus );
        sharpBonus = config.GetInt ( "Script" , "SharpBonus" , sharpBonus );
        useSharpPercentage = config.GetBool ( "Script" , "UseSharpPercentage" , useSharpPercentage );
        forgedBonus = config.GetInt ( "Script" , "ForgedBonus" , forgedBonus );
        wornPercentageMalus = config.GetInt ( "Script" , "WornMalus" , wornPercentageMalus );
        npcArenaSpeedMultiplier = config.GetFloat ( "Script" , "NPCArenaSpeedMultiplier" , npcArenaSpeedMultiplier );
        enableNPCSprint = config.GetBool ( "Script" , "EnableNPCSprint" , enableNPCSprint );
        zombiesCanSprint = config.GetBool ( "Script" , "ZombiesCanSprint" , zombiesCanSprint );
        enableNewTransformation = config.GetBool ( "Script" , "EnableNewTransformation" , enableNewTransformation );
        disableMonsterRage = config.GetBool ( "Script" , "DisableMonsterRage" , disableMonsterRage );
        enableNewMagicAiming = config.GetBool ( "Script" , "EnableNewMagicAiming" , enableNewMagicAiming );
        enableAOEDamage = config.GetBool ( "Script" , "EnableAOEDamage" , enableAOEDamage );
        bCString AOENamesString = config.GetString ( "Script" , "AOENames" , "" );
        AOENames = splitTobCStrings ( AOENamesString.GetText ( ) , ',' );
        bossLevel = config.GetU32 ( "Script" , "BossLevelCap" , bossLevel );
        uniqueLevel = config.GetU32 ( "Script" , "UniqueLevelCap" , uniqueLevel );
        eliteLevel = config.GetU32 ( "Script" , "EliteLevelCap" , eliteLevel );
        warriorLevel = config.GetU32 ( "Script" , "WarriorLevelCap" , warriorLevel );
        noviceLevel = config.GetU32 ( "Script" , "NoviceLevelCap" , noviceLevel );
        KnockDownThreshold = config.GetInt ( "Script" , "KnockDownThreshold" , KnockDownThreshold );
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
// wird aufgerufen von DoLogicalDamage
gEAction GE_STDCALL AssessHit ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs )
{
    INIT_SCRIPT_EXT ( Victim , Damager );
    UNREFERENCED_PARAMETER ( a_iArgs );

    if ( Victim == None || Damager == None )
        return gEAction_None;

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
    const GEI32 iDamageAmount = Damager.Damage.GetProperty<PSDamage::PropertyDamageAmount> ( );
    const GEFloat fDamageMultiplier = Damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( );
    const GEInt iWeaponDamage = static_cast< GEInt >( fDamageMultiplier * iDamageAmount );
    const bCString VictimItemTemplateName = Victim.Inventory.GetTemplateItem(Victim.Inventory.FindStackIndex ( gESlot_RightHand )).GetName();

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
        HitForce = static_cast< gEHitForce >(HitForce - getHyperArmorPoints(Victim, VictimAction));

        print ( "ActionWeaponLevel: %d\tShieldLevelBonus: %d\tgetHyperArmorPoints: %d\n" , ActionWeaponLevel , GetShieldLevelBonusNB ( Victim ) , getHyperArmorPoints ( Victim , VictimAction ) );

        if (HitForce <= poiseThreshold )
        {
            HitForce = gEHitForce_Minimal;
        }
        else if (HitForce <= gEHitForce_Normal)
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
            // TODO: Remove doubling damage for projectile (Rather Add new more potent Spell)
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
            GEInt dexterity = ScriptAdmin.CallScriptFromScript ( "GetDexterity" , &DamagerOwner , &None , 0 ) - startDEX;
            GEInt strength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 )- startSTR;
            GEInt intelligence = ScriptAdmin.CallScriptFromScript ( "GetIntelligence" , &DamagerOwner , &None , 0 );
            // Ranged damage
            if ( IsNormalProjectileNB ( Damager ) == GETrue )
            {
                //print ( "UseType Left : %d" , DamagerOwner.Inventory.GetItemFromSlot(gESlot_LeftHand).Interaction.UseType );
                if ( useStrengthForCrossbows && DamagerOwner.Inventory.GetItemFromSlot ( gESlot_LeftHand ).Interaction.UseType == gEUseType_CrossBow ) {
                    iAttributeBonusDamage = strength / 2;
                }
                else {
                    iAttributeBonusDamage = dexterity / 2;
                }
            }
            // Melee damage
            else {
                GEInt rightWeaponStackIndex = Player.Inventory.FindStackIndex ( gESlot_RightHand );
                GEInt leftWeaponStackIndex = Player.Inventory.FindStackIndex ( gESlot_LeftHand );
                gEUseType playerRightWeaponType = Player.Inventory.GetUseType ( rightWeaponStackIndex );

                //New Scaling
                if ( useNewBalanceMeleeScaling ) {
                    // TODO Use another Function for this bloaded if else statements (returns better)
                    //GEChar* arr = nullptr; 
                    bCString reqAttributeTag = "";
                    Entity Weapon = Player.GetWeapon ( GETrue );
                    if ( Weapon != None && Weapon.Item.IsValid() ) {
                        //arr = ( GEChar* )*( DWORD* )( *( DWORD* )&Weapon.Item + 0x74 ); // A bit Unsafe ... AccessReqAttrib1Tag()
                        gCItem_PS* item = ( gCItem_PS* )Weapon.Item.m_pEngineEntityPropertySet;
                        bCString reqAttributeTag = item->AccessReqAttrib1Tag ( );
                    }
                    //if ( arr != nullptr ) reqAttributeTag = bCString ( arr );
                    if ( playerRightWeaponType == gEUseType_Staff || reqAttributeTag.Contains ( "INT" )
                         || ( DamagerOwner.Inventory.GetItemFromSlot ( gESlot_RightHand ) != None
                             && DamagerOwner.Inventory.GetItemFromSlot ( gESlot_RightHand ).IsItem ( )
                             && DamagerOwner.Inventory.GetItemFromSlot ( gESlot_RightHand ).Item.GetQuality ( ) & ( gEItemQuality_Burning | gEItemQuality_Frozen ) ) ) {

                        if ( ( playerRightWeaponType == gEUseType_1H && Player.Inventory.GetUseType ( leftWeaponStackIndex ) == gEUseType_1H )
                            || reqAttributeTag.Contains ( "DEX" ) ) {
                            iAttributeBonusDamage = static_cast< GEInt >( dexterity * 0.3 + intelligence * 0.35 + 15 ); // Because you start with low Int, (Assume 60 INT)
                        }
                        else if ( playerRightWeaponType == gEUseType_Staff ) {
                            iAttributeBonusDamage = static_cast< GEInt >( strength * 0.2 + intelligence * 0.4 + 15 ); // Because you start with low Int, (Assume 60 INT)
                        }
                        else if ( playerRightWeaponType == gEUseType_2H || playerRightWeaponType == gEUseType::gEUseType_Axe || playerRightWeaponType == gEUseType::gEUseType_Pickaxe ) {
                            iAttributeBonusDamage = static_cast< GEInt >( strength * 0.3 + intelligence * 0.35 + 15 ); // Because you start with low Int, (Assume 60 INT)
                        }
                        else {
                            iAttributeBonusDamage = static_cast< GEInt >( strength * 0.2 + intelligence * 0.4 + 15 ); // Because you start with low Int, (Assume 60 INT)
                        }
                    }
                    else if ( playerRightWeaponType == gEUseType_1H && Player.Inventory.GetUseType ( leftWeaponStackIndex ) == gEUseType_1H ) {
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
        //Adjust GetPlayerSkillDamageBonus
        FinalDamage += ScriptAdmin.CallScriptFromScript ( "GetPlayerSkillDamageBonus" , &Damager , &Victim , FinalDamage );
    }
    // Damager is transformed player or NPC
    else if ( DamagerOwner.Navigation.IsValid ( ) )
    {
        GEInt iStrength = ScriptAdmin.CallScriptFromScript ( "GetStrength" , &DamagerOwner , &None , 0 ) * NPCStrengthMultiplicator + NPCStrengthCorrection; //STR Bonus Real
        if ( iStrength < 10 )
            iStrength = 10;

        // Player is under level 2 or AB is not activated
        if ( Player.NPC.GetProperty<PSNpc::PropertyLevel> ( ) < 2
            // ignore for newbalance || !eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeBalancing
            //  Besitzt der Angreifer keine Waffe?
            || ( Damager.Interaction.GetOwner ( ) == None && Damager.Navigation.IsValid ( ) ) )
            //  Benutzt der Angreifer Fernkampfwaffen?
        {
            FinalDamage = iStrength;
        }
        // NPC attacks with a weapon
        else 
        {
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
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier / 2.0f ); //TODO: Should 2H Weapon in 1H only do half weapon damage?
                }
                // Greift ein Mensch mit einer Nahkampfwaffe an?
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Human )
                {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier );
                }
                else if ( DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Demon
                    || DamagerOwner.NPC.GetProperty<PSNpc::PropertySpecies> ( ) == gESpecies_Ogre )
                {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier / 2.0f ); //TODO: Should 2H Weapon in 1H only do half weapon damage?
                }
                // MonsterAttack
                else if ( Damager.GetName ( ) == "Fist" ) {
                    FinalDamage = static_cast< GEInt >( iStrength * 1.3f + FinalDamage ); // Change and simplify after Adding Unique "Fist"-Weapons for Monsters
                }
                else if ( FinalDamage == 0 ) {
                    FinalDamage = iStrength;
                }
                else {
                    FinalDamage = static_cast< GEInt >( iStrength + FinalDamage * npcWeaponDamageMultiplier );
                }
            }
        }

        // Magic Projectiles, casted by NPCs (against NPCs), do double damage (Modified)
        if ( IsMagicProjectileNB ( Damager ) && !Victim.IsPlayer() )
        {
            FinalDamage *= 2;
        }
    }

    if ( ( Damager.IsItem() 
        && ( Damager.Item.GetQuality ( ) & gEItemQuality_Blessed ) == gEItemQuality_Blessed && ScriptAdmin.CallScriptFromScript ( "IsEvil" , &Victim , NULL , 0 )) )
        FinalDamage *= 1.2;

    if ( GetHeldWeaponCategoryNB ( DamagerOwner ) == gEWeaponCategory_Melee ) {
        if ( GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , &DamagerOwner , &None , 0 ) <= 50 )
            FinalDamage *= 0.7;
        else if ( GetScriptAdmin ( ).CallScriptFromScript ( "GetStaminaPoints" , &DamagerOwner , &None , 0 ) <= 20 )
            FinalDamage *= 0.5;
    }

    // Handelt es sich um einen Powercast? (Player and NPCs)
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainerNB ( Damager ) )
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
    GEInt FinalDamage2 = 0;
    GEInt iProtection = ScriptAdmin.CallScriptFromScript ( "GetProtection" , &Victim , &Damager , 0 );
    //print ( "Protection Integer %d\n" , iProtection );

    // Vulnerabilities

    if ( alternativeProtection ) {
        GEFloat fProtection = static_cast< GEFloat >( iProtection );
        switch ( DamageTypeEntityTestNB ( Victim , Damager ) ) {
        case VulnerabilityStatus_WEAK:
            fProtection *= 0.2f;
            break;
        case VulnerabilityStatus_STRONG:
            fProtection *= 2.0f;
            break;
        case VulnerabilityStatus_SLIGHTLYWEAK:
            fProtection *= 0.6f;
            break;
        case VulnerabilityStatus_SLIGHTLYSTRONG:
            fProtection *= 1.4f;
            break;
        }
        FinalDamage2 = FinalDamage - static_cast< GEInt >( fProtection );
    }
    else {
        switch ( DamageTypeEntityTestNB ( Victim , Damager ) ) {
        case VulnerabilityStatus_WEAK:
            FinalDamage *= 1.6f;
            break;
        case VulnerabilityStatus_STRONG:
            FinalDamage *= 0.5f;
            break;
        case VulnerabilityStatus_SLIGHTLYWEAK:
            FinalDamage *= 1.2f;
            break;
        case VulnerabilityStatus_SLIGHTLYSTRONG:
            FinalDamage *= 0.8f;
            break;
        }
        if ( iProtection > 90 )
            iProtection = 90;
        FinalDamage2 = FinalDamage - static_cast< GEInt >( FinalDamage * ( iProtection / 100.0f ) );
    }

    //print ( "Protection After Vulnerabilities %f\n ", fProtection );
    if ( FinalDamage2 < 5 )
        FinalDamage2 = 5;
    /*
    * Default Protection!
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
    }*/

    //
    // Schritt 3: Angriffsart
    //
    if ( DamageTypeEntityTestNB ( Victim , Damager ) == VulnerabilityStatus_IMMUNE && FinalDamage2 > 5)
        FinalDamage2 = 5;

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

    switch ( DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) )
    {
        // Angreifer benutzt Quick-Attacke
        //   => Schaden = Schaden / 2
    case gEAction_QuickAttack:
    case gEAction_QuickAttackR:
    case gEAction_QuickAttackL:
        //Quickattacken sind weniger effektiv gegen Starke NPC oder hohe Rüstung (5%)
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * (1.0f + QuickAttackArmorRes) - FinalDamage * QuickAttackArmorRes ) / 2;
        break;

        // Angreifer benutzt Powerattacke
        //   => Schaden = Schaden * 2 (bei Kampf mit 2 Waffen nicht durchgehend!)
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
        if ( !CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , DamagerOwner )
            || DamagerOwner.Routine.GetProperty<PSRoutine::PropertyStatePosition> ( ) == 2 )
        {
            //Starke Attacken ignorieren 10 % Rüstung
            FinalDamage2 = static_cast< GEInt >( FinalDamage2 * (1.0f - PowerAttackArmorPen) + FinalDamage * PowerAttackArmorPen ) * 2;
        }
        break;

        // Angreifer benutzt Hack-Attacke
        //   => Schaden = Schaden * 2
    case gEAction_HackAttack:
        // Hackattacken ignorieren 12.5 % Rüstung
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * ( 1.0f - SpecialAttackArmorPen ) + FinalDamage * SpecialAttackArmorPen ) * 2;
        break;
    }

    if ( victimDamageReceiver->GetVulnerableState ( ) == 2 ) {
        FinalDamage2 = static_cast< GEInt >( FinalDamage2 * PerfectBlockDamageMult );
        if ( static_cast<GEInt>(HitForce) >= 3 ) {
            HitForce = static_cast< gEHitForce >( KnockDownThreshold );
        }
        else {
            HitForce = gEHitForce_Normal;
        }
        victimDamageReceiver->AccessVulnerableState ( ) = 0;
    }

    // New Multiplier for NPC vs NPC Damage
    if ( !DamagerOwner.IsPlayer ( ) && !Victim.IsPlayer ( ) ) {
        FinalDamage2 *= NPCDamageReductionMultiplicator;
    }

    if ( FinalDamage2 < 5 )
        FinalDamage2 = 5;

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

    if ( Victim == Player )
        Victim.Effect.StopEffect ( GETrue );
    // Parade Magic
    if ( Damager.Projectile.IsValid ( ) && IsSpellContainerNB ( Damager ) )
    {
        if ( !Victim.NPC.IsFrozen ( ) && ScriptAdmin.CallScriptFromScript ( "CanParadeMagic" , &Victim , &Damager , 0 ) )
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
        if ( !Victim.NPC.IsFrozen ( ) && ScriptAdmin.CallScriptFromScript ( "CanParadeMissile" , &Victim , &Damager , 0 ) )
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
        || ( Victim.Routine.GetStateTime ( ) < 0.1f && Victim.Routine.GetProperty<PSRoutine::PropertyAniState> ( ) == gEAniState_Parade )
            //This can maybe a good feature , when registering attacks right in the beginning
        || ( Victim.Routine.GetProperty<PSRoutine::PropertyAniState>() == gEAniState_SitKnockDown && GetHeldWeaponCategoryNB (Victim) == gEWeaponCategory_Melee
            && Victim.IsInFOV ( DamagerOwner ) && !IsNormalProjectileNB ( Damager ) && !IsSpellContainerNB ( Damager )) )) 
    {
        GEFloat staminaDamageMultiplier = -0.5f;
        // Reduce damage if parading melee with shield
        if ( Victim == Player && CheckHandUseTypesNB ( gEUseType_Shield , gEUseType_1H , Victim ) )
        {
            if ( !Victim.Inventory.IsSkillActive ( "Perk_Shield_2" ) )
            {
                // Weicht von "Detaillierte Schadenberechnung" ab, dort wird ein Faktor von 2/3 anstatt 0.5 beschrieben.
                staminaDamageMultiplier *= 0.6f;
            }
            else
            {
                staminaDamageMultiplier *= 0.4f;
            }
        }
        else {
            // NPC now always get less Stamina consumption (they really block often and the pc hero does much more damage than an npc still)
            // , right now they also don't use consumables, Hopefully soon :)
            staminaDamageMultiplier *= 0.5f; // 0.25
            if ( CheckHandUseTypesNB ( gEUseType_Shield , gEUseType_1H , Victim ) ) {
                staminaDamageMultiplier *= 0.5f; // 0.125
            }
        }
        print ( "StaminaDamageMultiplier: %f\n" , staminaDamageMultiplier );
        GEInt FinalDamage3 = FinalDamage * staminaDamageMultiplier;
        print ( "FinalDamage3: %d\n" , FinalDamage3 );

        if ( enablePerfectBlock && GetHeldWeaponCategoryNB( DamagerOwner ) == gEWeaponCategory_Melee && ( !playerOnlyPerfectBlock || Victim.IsPlayer ( ) ) ) {

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
                EffectSystem::StartEffect ( "parry_sound_01" , DamagerOwner );
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

        // AlternativeAI parade (es werden keine Lebenspunkte angezogen)
        if ( useStaticBlocks && eCApplication::GetInstance ( ).GetEngineSetup ( ).AlternativeAI )
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
                DamagerOwner.GetInstance ( ) , Victim.GetInstance ( ) , DamagerOwnerAction , bCString ( "Recover" ) , 1.5f );//g_pstrPhaseString[gEPhase_Recover]
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
        //Changed back the remaining raw Damage after Def. Reduction and Stamina consumption
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
        // TODO: Invincible NPCs cannot be Killed atleast
        // They receive still some damage through blocks before
        // Atleast the engine does not let them get killed
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
            if ( ScriptAdmin.CallScriptFromScript ( "CanBeKilled" , &Victim , &None , 0 ) 
                && ( ScriptAdmin.CallScriptFromScript ( "GetHitPoints" , &Victim , &None , 0 ) - FinalDamage2 ) <= 0 )
            {
                if ( FinalDamage2 > 0 )
                {
                    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageAmount> ( ) = FinalDamage2;
                    Victim.DamageReceiver.AccessProperty<PSDamageReceiver::PropertyDamageType> ( ) = Damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
                }
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

    // Receive Damage (later outside of this function)
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
        auto damageReceiver = static_cast< gCDamageReceiver_PS_Ext* >( Victim.GetGameEntity ( )->GetPropertySet ( eEPropertySetType_DamageReceiver ) );
        damageReceiver->AccessPoisonDamage ( ) = GetPoisonDamage ( DamagerOwner );
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
            iFreezeTime = 30;
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
    if ( DamageTypeEntityTestNB ( Victim , Damager ) == VulnerabilityStatus_IMMUNE ) {
        ScriptAdmin.CallScriptFromScript ( "AssessAgressor" , &Victim , &DamagerOwner , 0 );
        return gEAction_None;
    }

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

    // Magic Casting Protection!
    if ( gEAction_Summon == Victim.Routine.GetProperty<PSRoutine::PropertyAction>()
        && 0.5f < Victim.Routine.GetStateTime ( ) // Delay the protection a bit
        && !Victim.Interaction.GetSpell ( ).GetName ( ).Contains ( "Heal" )
        && !Victim.Interaction.GetSpell ( ).GetName ( ).Contains ( "Cure" ) )
    {
        // If a special attack, or any strong attack-force is executed against the caster
        if ( gEAction_HackAttack != DamagerOwnerAction && 4 > HitForce && !isHeadshot ) // PierceAttack is already filtered out above
        {
            ScriptAdmin.CallScriptFromScript ( "PipiStumble" , &Victim , &None , 0 ); // Make Noice without Stumbles
            return gEAction_Stumble;
        }
    }

    // Scream or make HitEffect, but no Stumble also processes logic when you hit someone, like setting up combat mode
    if ( ( GEInt )HitForce <= gEHitForce_Minimal
        && ( GetHeldWeaponCategoryNB ( DamagerOwner ) == gEWeaponCategory_Ranged || IsInActiveAttack( Victim ) ) )
    {
        if ( VictimAction == gEAction_PierceStumble ) {
            Victim.Routine.FullStop ( );
            Victim.Routine.SetTask ( "ZS_QuickStumble" );
            return gEAction_QuickStumble;
        }
        ScriptAdmin.CallScriptFromScript ( "PipiStumble" , &Victim , &None , 0 );
        return DamagerOwnerAction;
    }

    if ( GetHeldWeaponCategoryNB ( Victim ) != gEWeaponCategory_None && ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &Victim , &None , 0 ))
    {
        if ( (GEInt)HitForce >= KnockDownThreshold /* && GetHeldWeaponCategoryNB ( Victim ) == gEWeaponCategory_Melee */) //Remove
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
    if ( enableNewMagicAiming ) {
        InitGUI ( );
    }

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
#ifdef DEBUG
        AllocConsole ( );
        freopen_s ( ( FILE** )stdout , "CONOUT$" , "w" , stdout );
#endif
        ::DisableThreadLibraryCalls( hModule );
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

