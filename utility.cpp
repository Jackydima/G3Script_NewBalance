#include "utility.h"


// SDK Function
gEWeaponCategory GetHeldWeaponCategoryNB ( Entity const& a_Entity )
{
    typedef gEWeaponCategory ( GE_STDCALL* mFGetHeldWeaponCategory )( Entity );
    static mFGetHeldWeaponCategory s_fGetHeldWeaponCategory = force_cast< mFGetHeldWeaponCategory >( RVA_ScriptGame ( 0x3240 ) );

    return s_fGetHeldWeaponCategory ( a_Entity );
}

GEInt getPowerLevel ( Entity& p_entity ) {
    Entity player = Entity::GetPlayer ( );
    GEInt level = p_entity.NPC.GetProperty<PSNpc::PropertyLevel> ( ) + player.NPC.GetProperty<PSNpc::PropertyLevel> ( );
    if ( level > p_entity.NPC.GetProperty<PSNpc::PropertyLevelMax> ( ) )
        level = p_entity.NPC.GetProperty<PSNpc::PropertyLevelMax> ( );
    //std::cout << "PowerLevel of: " << p_entity.GetName ( ) << ":\t" << level << "\n";
    if ( useAlwaysMaxLevel )
        level = p_entity.NPC.GetProperty<PSNpc::PropertyLevelMax> ( );
    return level;
}

Template getProjectile ( Entity& p_entity ,gEUseType p_rangedWeaponType ) {
    GEInt powerLevel = getPowerLevel ( p_entity );
    gEPoliticalAlignment alignment = p_entity.NPC.GetProperty<PSNpc::PropertyPoliticalAlignment> ( );
    gESpecies targetSpecies = p_entity.NPC.GetCurrentTarget ( ).NPC.GetProperty<PSNpc::PropertySpecies> ( );
    GEInt random = Entity::GetRandomNumber ( 100 );
    Template projectile;

    if ( p_rangedWeaponType == gEUseType_CrossBow ) {
        if ( powerLevel >= 25 ) {
            projectile = Template("Bolt_Sharp");
            if ( projectile.IsValid ( ) ) {
                return projectile;
            }
        }
        return Template ("Bolt");
    }
    else if ( p_rangedWeaponType == gEUseType_Bow ) {
        if ( p_entity.GetName ( ) == "Jorn" ) {// And add after quest! 
            projectile = Template ( "ExplosiveArrow" );
        }
        else if ( targetSpecies == gESpecies_FireGolem || targetSpecies == gESpecies_Golem || targetSpecies == gESpecies_IceGolem || targetSpecies == gESpecies_Skeleton ) {
            projectile = Template ( "BluntArrow" );
        }
        else if ( alignment == gEPoliticalAlignment_Ass || alignment == gEPoliticalAlignment_Nom ) {
            projectile = Template ( "PoisonArrow" );
        }
        else if ( alignment == gEPoliticalAlignment_Nrd ) {
            projectile = Template ( "SharpArrow" );
        }
        else if ( powerLevel >= 30 ) {
            if ( alignment == gEPoliticalAlignment_Orc ) {
                if ( random <= 20 ) {
                    projectile = Template ( "SharpArrow" );
                }
                else {
                    projectile = Template ( "GoldArrow" );
                }
            }
            else if ( random <= 20 ) {
                projectile = Template ( "BluntArrow");
            }
            else {
                projectile = Template ( "FireArrow" );
            }
        }
        if ( projectile.IsValid() )
            return projectile;
        return Template ("Arrow");
    }
}

GEInt getMonsterHyperArmorPoints ( Entity& p_monster , gEAction p_monsterAction ) {
    if ( p_monsterAction != gEAction_PowerAttack && p_monsterAction != gEAction_SprintAttack )
        return 0;
    if ( !isBigMonster ( p_monster ) )
        return 2;
    switch ( p_monster.NPC.GetProperty<PSNpc::PropertySpecies> ( ) ) {
    case gESpecies_Demon:
    case gESpecies_Ogre:
        return 3;
    case gESpecies_Troll:
    case gESpecies_Trex:
    case gESpecies_Shadowbeast:
    case gESpecies_Dragon:
    case gESpecies_Gargoyle:
    case gESpecies_ScorpionKing:
        return 5;
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_Golem:
        return 4;
    default:
        return 0;
    }
}

GEBool isBigMonster ( Entity& p_monster ) {
    gCScriptAdmin& ScriptAdmin = GetScriptAdmin ( );
    if ( p_monster == None ) {
        return GEFalse;
    }
    if ( ScriptAdmin.CallScriptFromScript ( "IsHumanoid" , &p_monster , &None , 0 ) ) {
        return GEFalse;
    }
    switch ( p_monster.NPC.GetProperty<PSNpc::PropertySpecies> ( ) ) {
    case gESpecies_Demon:
    case gESpecies_Ogre:
    case gESpecies_Troll:
    case gESpecies_Trex:
    case gESpecies_Shadowbeast:
        // Dragon maybe needs to check if Action is PowerCast is active
    case gESpecies_Dragon:
    case gESpecies_Gargoyle:
    case gESpecies_ScorpionKing:
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_Golem:
        return GETrue;
    default:
        return GEFalse;
    }


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

GEInt CanBurn ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {

    INIT_SCRIPT_EXT ( p_victim , p_damager );
    if ( p_damager == None ) return GEFalse;
    gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies> ( );
    switch ( victimSpecies ) {
    case gESpecies_Golem:
    case gESpecies_Demon:
    case gESpecies_Troll:
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_Dragon:
        return GEFalse;
    }
    GEInt random = Entity::GetRandomNumber ( 100 );
    gEDamageType damageType = p_damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
    GEU32 itemQuality = p_damager.Item.GetQuality ( );
    Entity DamagerOwner = p_damager.Interaction.GetOwner ( );
    if ( DamagerOwner == None && p_damager.Navigation.IsValid ( ) )
    {
        DamagerOwner = p_damager;
    }
    if ( ( p_victim == Entity::GetPlayer ( ) && p_victim.Inventory.IsSkillActive ( Template ( "Perk_ResistHeat" ) ) )
        || ( p_victim != Entity::GetPlayer ( ) && getPowerLevel ( p_victim ) >= 40 ) )
        random = static_cast< GEInt >( random * 2 );
    // Special Resistance :O
    if ( random >= 100 ) {
        return GEFalse;
    }

    //std::cout << "In CanBurn:\tDamageType: " << damageType << "\trandom: " << random
        //<< "\nProjectile.IsValid: " << p_damager.Projectile.IsValid () << "\titemQuality: " << itemQuality << "\n";
    if ( !p_damager.Projectile.IsValid ( ) ) {
        if ( damageType != gEDamageType_None ) {
            if ( damageType == gEDamageType_Fire ) {
                return GETrue;
            }
            if ( DamagerOwner != None && DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction_PowerAttack )
                random = static_cast< GEInt >( random * 0.66 );
            if ( ( ( BYTE )itemQuality & gEItemQuality_Burning ) == gEItemQuality_Burning && random < 26 ) {
                return GETrue;
            }
        }
    }
    // Missile Here
    else {
        if ( damageType == gEDamageType_Fire
            && p_damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle> ( ) == gEProjectilePath::gEProjectilePath_Missile )
            return GETrue;
        if ( ( ( BYTE )itemQuality & gEItemQuality_Burning ) == gEItemQuality_Burning
            && p_damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( ) >= 0.5f ) // Bow Tension...
            return GETrue;

    }
    return GEFalse;
}

GEInt CanFreeze ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEU32 a_iArgs ) {

    INIT_SCRIPT_EXT ( p_victim , p_damager );
    if ( p_damager == None ) return GEFalse;
    if ( p_damager.GetName ( ) == "Mis_IceBlock" )
        return GETrue;
    gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies> ( );
    //std::cout << "Projectilename: " << p_damager.GetName ( ) << std::endl;
    switch ( victimSpecies ) {
    case gESpecies_Golem:
    case gESpecies_Demon:
    case gESpecies_Troll:
    case gESpecies_FireGolem:
    case gESpecies_IceGolem:
    case gESpecies_Dragon:
        return GEFalse;
    }
    GEInt random = Entity::GetRandomNumber ( 100 );
    Entity DamagerOwner = p_damager.Interaction.GetOwner ( );
    gEDamageType damageType = p_damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
    GEU32 itemQuality = p_damager.Item.GetQuality ( );
    if ( DamagerOwner == None && p_damager.Navigation.IsValid ( ) )
    {
        DamagerOwner = p_damager;
    }
    if ( (p_victim == Entity::GetPlayer ( ) && p_victim.Inventory.IsSkillActive ( Template ( "Perk_ResistCold" ) ) )
        || (p_victim != Entity::GetPlayer() && getPowerLevel(p_victim) >= 40 ) )
        random = static_cast<GEInt>(random * 2.0);
    // Special Resistance :O
    if ( random >= 100 ) {
        return GEFalse;
    }

    if ( !p_damager.Projectile.IsValid ( ) ) {
        if ( damageType != gEDamageType_None ) {
            if ( damageType == gEDamageType_Ice ) {
                return GETrue;
            }
            if ( DamagerOwner !=None && DamagerOwner.Routine.GetProperty<PSRoutine::PropertyAction>() == gEAction_PowerAttack )
                random = static_cast< GEInt >( random * 0.66 );
            if ( ( ( BYTE )itemQuality & gEItemQuality_Frozen ) == gEItemQuality_Frozen && random < 26 ) {
                return GETrue;
            }
        }
    }
    // Missile Here
    else {
        if ( damageType == gEDamageType_Ice
            && p_damager.Projectile.GetProperty<PSProjectile::PropertyPathStyle> ( ) == gEProjectilePath::gEProjectilePath_Missile )
            return GETrue;
        if ( ( ( BYTE )itemQuality & gEItemQuality_Frozen ) == gEItemQuality_Frozen
            && p_damager.Damage.GetProperty<PSDamage::PropertyDamageHitMultiplier> ( ) >= 0.5f ) // Bow Tension...
            return GETrue;

    }
    return GEFalse;
}

GEBool IsNormalProjectileNB ( Entity& p_damager ) {
    //std::cout << "Projectilename: " << p_damager.GetName ( ) << std::endl;
    //std::cout << "Projectile?: " << p_damager.Projectile.IsValid() << std::endl;
    return p_damager.Projectile.IsValid ( ) &&
        p_damager.Interaction.GetSpell ( ) == None;
}

GEBool IsSpellContainerNB ( Entity& p_damager ) {
    //std::cout << "Spell?: " << p_damager.Interaction.GetSpell ( ).GetName ( )
    //    << "\n" << p_damager.Interaction.GetOwner ( ).GetName() << std::endl;
    return p_damager.Interaction.GetSpell ( ) != None;
}

GEBool IsMagicProjectileNB ( Entity& p_damager ) {
    //std::cout << "Projectilename: " << p_damager.GetName ( ) << std::endl;
    //std::cout << "Projectile?: " << p_damager.Projectile.IsValid ( ) << std::endl;
    return p_damager.Projectile.IsValid ( ) &&
        p_damager.Interaction.GetSpell ( ) != None;
}

GEBool CheckHandUseTypesNB ( gEUseType p_lHand , gEUseType p_rHand , Entity& entity ) {
    //std::cout << "Left: " << p_lHand << "Right: " << p_rHand
       // << "\nItem Left: " << entity.Inventory.GetItemFromSlot ( gESlot_LeftHand ).GetName ( )
       // << "\nItem Right" << entity.Inventory.GetItemFromSlot ( gESlot_RightHand ).GetName ( ) << std::endl;
    return ( entity.Inventory.GetItemFromSlot ( gESlot_LeftHand )
        .Interaction.GetUseType ( ) == p_lHand &&
        entity.Inventory.GetItemFromSlot ( gESlot_RightHand )
        .Interaction.GetUseType ( ) == p_rHand );
}

// TODO: Adjusted Skill level, maybe change back :)
GEInt GetSkillLevelsNB ( Entity& p_entity ) {
    if ( p_entity != Entity::GetPlayer ( ) ) {
        GEU32 npcLevel = getPowerLevel(p_entity);
        //std::cout << "Entity: " << p_entity.GetName ( ) << "\tLevel: " << npcLevel << std::endl;
        if ( npcLevel > 69 )
            return 5;
        if ( npcLevel > 49 )
            return 4;
        if ( npcLevel > 34 )
            return 2;
        if ( npcLevel > 20 )
            return 1;
        return 0;
    }

    GEInt level = 0;
    GEInt playerRightHandStack = p_entity.Inventory.FindStackIndex ( gESlot_RightHand );
    gEUseType playerUseType = p_entity.Inventory.GetUseType ( playerRightHandStack );

    //std::cout << "Player in GetSkillLevels" << std::endl;
    // maybe even use a switch case here...

    switch ( playerUseType ) {
    case gEUseType_1H:
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_3" ) ) )
            level = 3;
        else if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_2" ) ) )
            level = 1;
        if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , p_entity )
            && p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H1H_2" ) ) )
            level += 1;
        break;
    case gEUseType_2H:
    case gEUseType_Axe:
    case gEUseType_Halberd:
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_3" ) ) )
            level = 3;
        else if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_2" ) ) )
            level = 1;
        break;
    case gEUseType_Staff:
        if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_3" ) ) )
            level = 3;
        else if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_2" ) ) )
            level = 1;
        break;
    case gEUseType_Cast:
        GEInt playerInt = p_entity.PlayerMemory.GetIntelligence ( );
        if ( playerInt > 199 ) 
            level = 3;
        else if ( playerInt > 99 ) 
            level = 1;
        break;
    }
    if ( GetScriptAdmin ( ).CallScriptFromScript ( "GetStrength" , &p_entity , &None , 0 ) >= 250 ) {
        level += 1;
    }
    if ( p_entity.NPC.GetProperty<PSNpc::PropertyLevel> ( ) > 49 )
        level += 1;
    //std::cout << "Returned PCHERO Level: " << level << "\n";
    return level; // or level
}

GEInt GetActionWeaponLevelNB ( Entity& p_damager , gEAction p_action ) {
    GEInt level = 0;
    gEUseType damagerWeaponType = p_damager.Inventory.GetUseType ( p_damager.Inventory.FindStackIndex ( gESlot_RightHand ) );
    switch ( p_action ) {
    case gEAction_Attack:
        if ( damagerWeaponType == gEUseType_2H || damagerWeaponType == gEUseType_Axe || 
            (damagerWeaponType == gEUseType_Fist && !GetScriptAdmin().CallScriptFromScript("IsHumanoid",&p_damager, &None) ) ) {
            level = 2;
            break;
        }
        level = 1;
        break;
    case gEAction_SimpleWhirl:
    case gEAction_GetUpAttack:
    case gEAction_WhirlAttack:
        level = 2;
        break;
    case gEAction_PierceAttack:
        level = 3;
        break;
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
        if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , p_damager ) ) {
            level = 3 - ( GEU32 )p_damager.Routine.GetProperty<PSRoutine::PropertyStatePosition> ( );
            break;
        }
        level = 4;
        break;
    case gEAction_HackAttack:
        level = 5;
        break;
    case gEAction_QuickAttack:
    case gEAction_QuickAttackR:
    case gEAction_QuickAttackL:
        level = 0;
        break;
    }
    return level + GetSkillLevelsNB ( p_damager );
}

GEInt GetShieldLevelBonusNB ( Entity& p_entity ) {
    //std::cout << "Name in GetShieldLevelBonus: " << p_entity.GetName ( ) << std::endl;
    if ( p_entity.Routine.GetProperty<PSRoutine::PropertyAction> ( ) == gEAction::gEAction_GetUpParade ) {
        return 2;
    }
    GEInt level = GetSkillLevelsNB ( p_entity );
    GEInt stackIndex = p_entity.Inventory.FindStackIndex ( gESlot::gESlot_LeftHand );
    gEUseType useType = p_entity.Inventory.GetUseType ( stackIndex );

    if ( useType == gEUseType_Shield ) {
        if ( p_entity == Entity::GetPlayer ( ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Shield_2" ) ) )
                level += 1;
        }
        else if ( getPowerLevel ( p_entity ) > 34 )
            level += 1;
    }
    return level;
}

// return an Enum of vulnaribility
VulnerabilityStatus DamageTypeEntityTestNB ( Entity& p_victim , Entity& p_damager ) {  // 0: Immunity, 1: Regular Damage, 2: Double Damage 3 Half Damage
    if ( p_victim == None || p_damager == None )
        return VulnerabilityStatus::VulnerabilityStatus_IMMUNE;
    gEDamageType damageType = p_damager.Damage.GetProperty<PSDamage::PropertyDamageType> ( );
    gESpecies victimSpecies = p_victim.NPC.GetProperty<PSNpc::PropertySpecies> ( );
    //std::cout << "Damager in DamageTypeEntityTest: " << p_damager.GetName ( ) << "\tvictimspecies: " << victimSpecies 
        //<< "\tDamageType: " << damageType << std::endl;
    switch ( damageType ) {
    case gEDamageType_None:
        break;
    case gEDamageType_Impact:
        if ( victimSpecies == gESpecies::gESpecies_Skeleton || victimSpecies == gESpecies::gESpecies_Golem )
            return VulnerabilityStatus_WEAK;
        return VulnerabilityStatus_REGULAR;
    case gEDamageType_Blade:
        if ( victimSpecies == gESpecies::gESpecies_Golem )
            return VulnerabilityStatus_STRONG;
        return VulnerabilityStatus_REGULAR;
    case gEDamageType_Missile:
        switch ( victimSpecies ) {
        case gESpecies_Skeleton:
        case gESpecies_IceGolem:
        case gESpecies_Troll:
            if ( p_damager.GetName ( ).Contains ( "Fire" , 0 ) || p_damager.GetName ( ).Contains ( "Explosive" , 0 ) )
                return VulnerabilityStatus_REGULAR;
            if ( victimSpecies == gESpecies_Troll ) {
                return VulnerabilityStatus_STRONG;
            }
            return VulnerabilityStatus_IMMUNE;
        case gESpecies_Golem:
        case gESpecies_FireGolem:
            return VulnerabilityStatus_IMMUNE;
        default:
            return VulnerabilityStatus_REGULAR;
        }
    case gEDamageType_Fire:
        if ( p_victim.NPC.GetProperty<PSNpc::PropertyClass> ( ) == gEClass_Mage )
            return VulnerabilityStatus_SLIGHTLYSTRONG;
        switch ( victimSpecies ) {
        case gESpecies_FireVaran:
        case gESpecies_FireGolem:
        case gESpecies_Dragon:
            if ( p_victim.GetName ( ).Contains ( "Ice" ) ) {
                return VulnerabilityStatus_WEAK;
            }
            if ( IsSpellContainerNB ( p_damager ) )
                return VulnerabilityStatus_IMMUNE;
            return VulnerabilityStatus_STRONG;
        case gESpecies_IceGolem:
        case gESpecies_Zombie:
            return VulnerabilityStatus_WEAK;
        case gESpecies_Demon:
            return VulnerabilityStatus_STRONG;
        default:
            return VulnerabilityStatus_REGULAR;
        }
    case gEDamageType_Ice:
        if ( p_victim.NPC.GetProperty<PSNpc::PropertyClass> ( ) == gEClass_Mage )
            return VulnerabilityStatus_SLIGHTLYSTRONG;
        if ( p_victim.NPC.HasStatusEffects ( gEStatusEffect::gEStatusEffect_Frozen ) ) {
            if ( IsSpellContainerNB ( p_damager ) )
                return VulnerabilityStatus_IMMUNE;
            return VulnerabilityStatus_STRONG;
        }
        switch ( victimSpecies ) {
        case gESpecies_FireGolem:
            return VulnerabilityStatus_WEAK;
        case gESpecies_Zombie:
            return VulnerabilityStatus_STRONG;
        case gESpecies_IceGolem:
            if (IsSpellContainerNB ( p_damager ) )
                return VulnerabilityStatus_IMMUNE;
            return VulnerabilityStatus_STRONG;
        case gESpecies_Dragon:
            if ( p_victim.GetName ( ).Contains ( "Fire" ) ) {
                if ( IsSpellContainerNB ( p_damager ) )
                    return VulnerabilityStatus_WEAK;
                return VulnerabilityStatus_STRONG;
            }
            if ( p_victim.GetName ( ).Contains ( "Ice" ) ) {
                if ( IsSpellContainerNB ( p_damager ) )
                    return VulnerabilityStatus_IMMUNE;
                return VulnerabilityStatus_STRONG;
            }
            return VulnerabilityStatus_REGULAR;
            
        default:
            return VulnerabilityStatus_REGULAR;
        }
    case gEDamageType_Lightning:
        if ( p_victim.NPC.GetProperty<PSNpc::PropertyClass> ( ) == gEClass_Mage )
            return VulnerabilityStatus_SLIGHTLYSTRONG;
        if ( victimSpecies == gESpecies_Golem )
            return VulnerabilityStatus_WEAK;
        if (victimSpecies == gESpecies_Dragon && p_victim.GetName().Contains("Stone"))
            return VulnerabilityStatus_WEAK;
        return VulnerabilityStatus_REGULAR;
    default:
        return VulnerabilityStatus_REGULAR;
    }

    return VulnerabilityStatus_IMMUNE;
}

GEInt GetHyperActionBonus ( gEAction p_action )
{
    switch ( p_action ) {
    case gEAction_Summon:
    case gEAction_FlameSword:
        return 4;
    case gEAction_PowerAttack:
    case gEAction_SprintAttack:
    case gEAction_HackAttack:
        return 0; // Change someday
    case gEAction_Heal:
        return 0;
    default:
        return 0;
    }

}

GEU32 GetPoisonDamage ( Entity& attacker ) {
    GEInt poisonDamage = 3;
    if ( attacker.IsPlayer ( ) ) {
        GEInt thf = attacker.PlayerMemory.GetTheft ( );
        poisonDamage = static_cast<GEInt>(thf * 0.1) + 3;
        if ( poisonDamage < 3 )
            poisonDamage = 3;
        return poisonDamage;
    }

    GEInt level = getPowerLevel ( attacker );
    poisonDamage = static_cast< GEInt >( level * 0.1 ) + 2;
    return poisonDamage;
}

GEInt getWeaponLevelNB ( Entity& p_entity ) {
    if ( p_entity.IsPlayer() && !p_entity.NPC.IsTransformed ( ) ) {
        if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_2H , p_entity ) || CheckHandUseTypesNB ( gEUseType_None , gEUseType_Axe , p_entity ) ||
            CheckHandUseTypesNB ( gEUseType_None , gEUseType_Pickaxe , p_entity ) || CheckHandUseTypesNB ( gEUseType_None , gEUseType_Halberd , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_3" ) ) )
                return 3;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_2" ) ) )
                return 2;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_1" ) ) )
                return 1;
        }
        if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_1H , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_3" ) ) )
                return 3;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_2" ) ) )
                return 2;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_1" ) ) )
                return 1;
        }
        if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_1H_2" ) ) )
                return 3;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_1H_1" ) ) )
                return 2;
        }
        if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_Staff , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_3" ) ) )
                return 3;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_2" ) ) )
                return 2;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_1" ) ) )
                return 1;
        }
        return 0;
    }
    //NPC here
    GEInt powerLevel = getPowerLevel ( p_entity );
    if ( powerLevel > 49 )
        return 3;
    if ( powerLevel > 34 )
        return 2;
    if ( powerLevel > 20 )
        return 1;
    return 0;
}

GEBool IsHoldingTwoHandedWeapon ( Entity& entity ) {

    gEUseType weaponUseType = entity.Inventory.GetItemFromSlot ( gESlot_RightHand ).Interaction.GetUseType ( );
    if ( weaponUseType == gEUseType_2H || weaponUseType == gEUseType_Staff || weaponUseType == gEUseType_Axe || weaponUseType == gEUseType_Halberd || weaponUseType == gEUseType_Pickaxe ) {
        return GETrue;
    }
    return GEFalse;
}

GEBool IsInSameParty ( Entity& p_self , Entity& p_other ) {
    Entity partyLeader = p_self.Party.GetPartyLeader ( );
    if ( partyLeader == None )
        return GEFalse;
    return partyLeader == p_other.Party.GetPartyLeader ( );
}

GEBool IsPlayerInCombat ( ) {
    Entity Player = Entity::GetPlayer ( );
    bTObjArray<Entity> entities = Entity::GetNPCs ( );
    if ( !entities.IsEmpty ( ) ) {
        GEInt i = 0;
        while ( i < entities.GetCount ( ) ) {
            Entity e = entities.AccessAt ( i++ );
            if ( e == None || e.IsDead ( ) || e.IsDown ( ) || e.IsPlayer ( ) || e.GetDistanceTo ( Player ) > 90000.0f )
                continue;
            if ( e.Routine.GetProperty<PSRoutine::PropertyAIMode> ( ) == gEAIMode_Combat 
                && e.NPC.GetCurrentTarget ( ) == Player ) {
                return GETrue;
            }
        }
    }
    return GEFalse;
}

GEInt speciesLeftHand ( Entity p_entity ) {
    gESpecies species = p_entity.NPC.GetProperty<PSNpc::PropertySpecies> ( );

    if ( species == gESpecies_Troll ) {
        GEInt retVal = p_entity.Inventory.AssureItems ( "TrollFist" , gEItemQuality::gEItemQuality_Diseased , 1 );
        return retVal;
    }
    return -1;
}

GEInt speciesRightHand ( Entity p_entity ) {
    gESpecies species = p_entity.NPC.GetProperty<PSNpc::PropertySpecies> ( );

    switch ( species ) {
    case gESpecies_Zombie:
    case gESpecies_Varan:
    case gESpecies_Ripper:
        return p_entity.Inventory.AssureItems ( "Fist" , gEItemQuality::gEItemQuality_Diseased , 1 );
    case gESpecies_Demon:
        return p_entity.Inventory.AssureItems ( "It_2H_DemonSword_01" , gEItemQuality::gEItemQuality_Burning , 1 );
    case gESpecies_Goblin:
        return p_entity.Inventory.AssureItems ( "It_1H_Club_01" , gEItemQuality::gEItemQuality_Worn , 1 );
    case gESpecies_Troll:
        return p_entity.Inventory.AssureItems ( "TrollFist" , gEItemQuality::gEItemQuality_Diseased , 1 );
    case gESpecies_FireVaran:
    case gESpecies_FireGolem:
        return p_entity.Inventory.AssureItems ( "Fist" , gEItemQuality::gEItemQuality_Burning , 1 );
    case gESpecies_Bloodfly:
    case gESpecies_SwampLurker:
    case gESpecies_ScorpionKing:
        return p_entity.Inventory.AssureItems ( "Fist" , gEItemQuality::gEItemQuality_Poisoned , 1 );
    case gESpecies_Ogre:
        return p_entity.Inventory.AssureItems ( "It_Axe_OgreMorningStar_01" , /*gEItemQuality::gEItemQuality_Worn*/0 , 1 );
    case gESpecies_IceGolem:
        return p_entity.Inventory.AssureItems ( "Fist" , gEItemQuality::gEItemQuality_Frozen , 1 );
    case gESpecies_Stalker:
        return p_entity.Inventory.AssureItems ( "It_Axe_SpikedClub_01" , gEItemQuality::gEItemQuality_Worn , 1 );
    case gESpecies_Dragon:
        return p_entity.Inventory.AssureItems ( "It_Spell_Fireball" , 0 , 1 );
    default: 
        return p_entity.Inventory.AssureItems ( "Fist" , 0 , 1 );
    }
    
    return -1;
}

GEBool IsInRecovery ( Entity& p_entity ) {
    eCVisualAnimation_PS* va = ( eCVisualAnimation_PS* )p_entity.Animation.m_pEngineEntityPropertySet;
    if ( ( GEU32 )va == 0 )
        return GEFalse;

    bCString* ptrCurrentMotionDescription = ( bCString* )( *( GEU32* )( ( GEU32 )va + 0xE8 ) + 0x4 );
    //std::cout << "String Ani: " << ptrCurrentMotionDescription << "\n";
    GEInt firstP = ptrCurrentMotionDescription->Find ( "_" , 4 );
    GEInt secondP = ptrCurrentMotionDescription->Find ( "_" , 12 );
    bCString test = "";
    ptrCurrentMotionDescription->GetWord ( 4 , "_" , test , GETrue , GETrue );
    //std::cout << "Test: " << test.GetText ( ) << "\n";
    if ( test.Contains ( "P0" ) && ptrCurrentMotionDescription->Contains ( "Recover" ) ) {
        //std::cout << "String Ani: " << ptrCurrentMotionDescription->GetText ( ) << "\n";
        return GETrue;
    }
    return GEFalse;
}

WarriorType GetWarriorType ( Entity& p_entity ) {
    if ( p_entity.IsPlayer ( ) && !p_entity.NPC.IsTransformed ( ) ) {
        if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_2H , p_entity ) || CheckHandUseTypesNB ( gEUseType_None , gEUseType_Axe , p_entity ) ||
            CheckHandUseTypesNB ( gEUseType_None , gEUseType_Pickaxe , p_entity ) || CheckHandUseTypesNB ( gEUseType_None , gEUseType_Halberd , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_3" ) ) )
                return WarriorType_Elite;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_2" ) ) )
                return WarriorType_Warrior;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Axe_1" ) ) )
                return WarriorType_Novice;
        }
        if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_1H , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_3" ) ) )
                return WarriorType_Elite;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_2" ) ) )
                return WarriorType_Warrior;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_1" ) ) )
                return WarriorType_Novice;
        }
        if ( CheckHandUseTypesNB ( gEUseType_1H , gEUseType_1H , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_1H_2" ) ) )
                return WarriorType_Elite;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_1H_1H_1" ) ) )
                return WarriorType_Warrior;
        }
        if ( CheckHandUseTypesNB ( gEUseType_None , gEUseType_Staff , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_3" ) ) )
                return WarriorType_Elite;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_2" ) ) )
                return WarriorType_Warrior;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Staff_1" ) ) )
                return WarriorType_Novice;
        }
        if ( CheckHandUseTypesNB ( gEUseType_Bow , gEUseType_Arrow, p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Bow_3" ) ) )
                return WarriorType_Elite;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Bow_2" ) ) )
                return WarriorType_Warrior;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Bow_1" ) ) )
                return WarriorType_Novice;
        }
        if ( CheckHandUseTypesNB ( gEUseType_CrossBow , gEUseType_Bolt , p_entity ) ) {
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Bow_3" ) ) )
                return WarriorType_Elite;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Bow_2" ) ) )
                return WarriorType_Warrior;
            if ( p_entity.Inventory.IsSkillActive ( Template ( "Perk_Bow_1" ) ) )
                return WarriorType_Novice;
        }
        return WarriorType_None;
    }
    GEInt powerLevel = getPowerLevel ( p_entity );
    if ( powerLevel >= 40 )
        return WarriorType_Elite;
    if ( powerLevel >= 30 )
        return WarriorType_Warrior;
    return WarriorType_Novice;
}

GEU32 getLastTimeFromMap ( bCString iD, std::map<bCString , GEU32>& map ) {
    GEU32 worldTime = Entity::GetWorldEntity ( ).Clock.GetTimeStampInSeconds ( );
    GEU32 retVal = 0;
    for ( auto it = map.cbegin ( ); it != map.cend ( ); ) {
        if ( worldTime - it->second > 400 )
            map.erase ( it++ );
        else
            ++it;
    }
    try {
        retVal = worldTime - map.at ( iD );
    }
    catch ( std::exception e ) {
        retVal = ULONG_MAX;
    }
    return retVal;
}