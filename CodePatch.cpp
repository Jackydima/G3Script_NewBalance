#include "CodePatch.h"

void PatchCode ( ) {
    /**
    * New Velocity for bows!
    */
    DWORD currProt , newProt;
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x86705 ) , sizeof ( &shootVelocityPtr ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x86705 ) , 0x90 , sizeof ( &shootVelocityPtr ) );
    memcpy ( ( LPVOID )RVA_ScriptGame ( 0x86705 ) , &shootVelocityPtr , sizeof ( &shootVelocityPtr ) );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x386705 ) , sizeof ( &shootVelocityPtr ) , currProt , &newProt );

    /**
    * Reset All Fix for PipiStumble (for HyperArmor working correctly)
    */
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x23036 ) , 0x23049 - 0x23036 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x23036 ) , 0x90 , 0x23049 - 0x23036 );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x23036 ) , 0x23049 - 0x23036 , currProt , &newProt );

    /**
    * Changed the Automatic TagetBone
    */
    VirtualProtect ( ( LPVOID )RVA_Game ( 0x151f12 ) , sizeof ( &BONETARGET ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_Game ( 0x151f12 ) , 0x90 , sizeof ( &BONETARGET ) );
    memcpy ( ( LPVOID )RVA_Game ( 0x151f12 ) , &BONETARGET , sizeof ( &BONETARGET ) );
    VirtualProtect ( ( LPVOID )RVA_Game ( 0x151f12 ) , sizeof ( &BONETARGET ) , currProt , &newProt );

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
    BYTE patchcode[] = { 0xE9,0x2B,0x02,0x00,0x00 };
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x63359 ) , 0x63365 - 0x63359 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x63359 ) , 0x90 , 0x63365 - 0x63359 );
    memcpy ( ( LPVOID )RVA_ScriptGame ( 0x6335f ) , patchcode , sizeof ( patchcode ) / sizeof ( BYTE ) );
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

    /**
    * Remove the limitation to target friendly NPCs via quick-patch
    * This is important for Attacks with Fist (especially for Transformed PC_Hero) 
    * Fists had no CollisionShapes, and therefore could not get registered
    * They only hit the "Current Target"
    */
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xaa5e6 ) , 0xaa5eb - 0xaa5e6 , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0xaa5e6 ) , 0x90 , 3 ); // Remove cmp Instr.
    memset ( ( LPVOID )RVA_ScriptGame ( 0xaa5e9 ) , 0xEB , 1 ); // Change jne (0x75) to jmp (0xEB)
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0xaa5e6 ) , 0xaa5eb - 0xaa5e6 , currProt , &newProt );
    
    /**
    * Adjust the QualityBonuses
    */
    if (useSharpPercentage )
        sharpBonusString = bCString::GetFormattedString ( "+(%d%%)" , sharpBonus );
    else 
        sharpBonusString = bCString::GetFormattedString ( "+(%d)" , sharpBonus );

    blessedBonusString = bCString::GetFormattedString ( "+(%d)" , blessedBonus );
    forgedBonusString = bCString::GetFormattedString ( "+(%d)" , forgedBonus );
    wornMalusString = bCString::GetFormattedString ( "-(%d%%)" , 100-wornPercentageMalus );
    
    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3e91 ) , sizeof(&sharpBonusString) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_Game ( 0xa3e91 ) , 0x00 , sizeof ( &sharpBonusString ) );
    memcpy ( ( LPVOID )RVA_Game ( 0xa3e91 ) , &sharpBonusString , sizeof ( &sharpBonusString ) );
    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3e91 ) , sizeof ( &sharpBonusString ) , currProt , &newProt );

    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3ef5 ) , sizeof ( &blessedBonusString ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_Game ( 0xa3ef5 ) , 0x00 , sizeof ( &blessedBonusString ) );
    memcpy ( ( LPVOID )RVA_Game ( 0xa3ef5 ) , &blessedBonusString , sizeof ( &blessedBonusString ) );
    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3ef5 ) , sizeof ( &blessedBonusString ) , currProt , &newProt );

    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3e2b ) , sizeof ( &forgedBonusString ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_Game ( 0xa3e2b ) , 0x00 , sizeof ( &forgedBonusString ) );
    memcpy ( ( LPVOID )RVA_Game ( 0xa3e2b ) , &forgedBonusString , sizeof ( &forgedBonusString ) );
    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3e2b ) , sizeof ( &forgedBonusString ) , currProt , &newProt );

    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3fba ) , sizeof ( &wornMalusString ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_Game ( 0xa3fba ) , 0x00 , sizeof ( &wornMalusString ) );
    memcpy ( ( LPVOID )RVA_Game ( 0xa3fba ) , &wornMalusString , sizeof ( &wornMalusString ) );
    VirtualProtect ( ( LPVOID )RVA_Game ( 0xa3fba ) , sizeof ( &wornMalusString ) , currProt , &newProt );
}