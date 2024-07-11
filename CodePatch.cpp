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
}