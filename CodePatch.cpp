#include "CodePatch.h"

void PatchCode1 ( ) {
    /**
    * New Velocity for bows!
    */
    DWORD currProt , newProt;
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x86705 ) , sizeof ( &shootVelocityPtr ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_ScriptGame ( 0x86705 ) , 0x90 , sizeof ( &shootVelocityPtr ) );
    memcpy ( ( LPVOID )RVA_ScriptGame ( 0x86705 ) , &shootVelocityPtr , sizeof ( &shootVelocityPtr ) );
    VirtualProtect ( ( LPVOID )RVA_ScriptGame ( 0x386705 ) , sizeof ( &shootVelocityPtr ) , currProt , &newProt );

    /**
    * Changed the Automatic TagetBone
    */
    VirtualProtect ( ( LPVOID )RVA_Game ( 0x151f12 ) , sizeof ( &BONETARGET ) , PAGE_EXECUTE_READWRITE , &currProt );
    memset ( ( LPVOID )RVA_Game ( 0x151f12 ) , 0x90 , sizeof ( &BONETARGET ) );
    memcpy ( ( LPVOID )RVA_Game ( 0x151f12 ) , &BONETARGET , sizeof ( &BONETARGET ) );
    VirtualProtect ( ( LPVOID )RVA_Game ( 0x151f12 ) , sizeof ( &BONETARGET ) , currProt , &newProt );
}