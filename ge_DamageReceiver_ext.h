#pragma once

#include "util/Memory.h"
#include "util/Hook.h"
#include "util/Util.h"
#include "util/Module.h"

#include "Script.h"
#include "Game.h"

class gCDamageReceiver_PS_Ext :
    public gCDamageReceiver_PS {
public:
    GE_PADDING ( 0x54 - 0x14 )
    GE_DEFINE_PROPERTY ( gCDamageReceiver_PS , GEU32 , m_PoisonDamage , PoisonDamage )
    GE_DEFINE_PROPERTY ( gCDamageReceiver_PS , GEU32 , m_VulnerableState , VulnerableState )


public:
    gCDamageReceiver_PS_Ext ( void );

protected:
    void Invalidate ( void );
};

GE_ASSERT_SIZEOF ( gCDamageReceiver_PS_Ext , 0x54 + 0x8 )