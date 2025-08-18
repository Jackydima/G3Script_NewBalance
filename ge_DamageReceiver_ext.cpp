#include "ge_DamageReceiver_ext.h"
#include "me_propertymacros.h"

ME_DEFINE_PROPERTY_TYPE ( gCDamageReceiver_PS_Ext , gCDamageReceiver_PS , PoisonDamage , m_PoisonDamage )
ME_DEFINE_PROPERTY_TYPE ( gCDamageReceiver_PS_Ext , gCDamageReceiver_PS , VulnerableState , m_VulnerableState )

gCDamageReceiver_PS_Ext::gCDamageReceiver_PS_Ext ( void ) : m_PoisonDamage(0), m_VulnerableState(0) {}

void gCDamageReceiver_PS_Ext::Invalidate ( ) {
	gCDamageReceiver_PS::Invalidate ( );
	this->m_PoisonDamage = 0;
	this->m_VulnerableState = 0;
}

ME_MODULE( gCDamageReceiver_PS_Ext )
{
	ME_PATCH_PROPERTY_SET ( "Game.dll" , "Script_NewBalance.dll" , gCDamageReceiver_PS , gCDamageReceiver_PS_Ext , RVA_Game ( 0x3b96C ) , GEU32 )
}