#pragma once
#include "Script.h"

class GE_DLLIMPORT gCProjectile_PS {

public:
    GE_PADDING ( 0x94 );
public:
    gCProjectile_PS ( void );
    gCProjectile_PS ( class gCProjectile_PS const& );
    virtual~gCProjectile_PS ( void );
    gCProjectile_PS const& operator=( class gCProjectile_PS const& );
    float& AccessDecayDuration ( void );
    bCString& AccessEffectPointImpact ( void );
    bCString& AccessEffectTargetHit ( void );

public: BOOL& __thiscall gCProjectile_PS::AccessFadeOnDecay ( void );
public: class bCString& __thiscall gCProjectile_PS::AccessFuncOnTargetHit ( void );
public: BOOL& __thiscall gCProjectile_PS::AccessLinkToBones ( void );
public: class bTPropertyContainer<enum gEProjectilePath>& __thiscall gCProjectile_PS::AccessPathStyle ( void );
public: float& __thiscall gCProjectile_PS::AccessShootVelocity ( void );
public: class bCVector& __thiscall gCProjectile_PS::AccessTargetDirection ( void );
public: class eCEntityProxy& __thiscall gCProjectile_PS::AccessTargetEntity ( void );
public: class bCVector& __thiscall gCProjectile_PS::AccessTargetPosition ( void );
public: unsigned long& __thiscall gCProjectile_PS::AccessTargetUpdateMSec ( void );
public: float& __thiscall gCProjectile_PS::AccessTouchAngleTreshold ( void );
public: class bTPropertyContainer<enum gEProjectileTouchBehavior>& __thiscall gCProjectile_PS::AccessTouchBehavior ( void );
public: virtual enum bEResult __thiscall gCProjectile_PS::CopyFrom ( class bCObjectBase const& );
public: virtual enum bEResult __thiscall gCProjectile_PS::Create ( void );
public: virtual void Decay ( void );
public: virtual void __thiscall gCProjectile_PS::Destroy ( void );
public: float const& __thiscall gCProjectile_PS::GetDecayDuration ( void )const;
protected: class bCVector __thiscall gCProjectile_PS::GetDirection ( void )const;
public: class bCString const& __thiscall gCProjectile_PS::GetEffectPointImpact ( void )const;
public: class bCString const& __thiscall gCProjectile_PS::GetEffectTargetHit ( void )const;
public: BOOL const& __thiscall gCProjectile_PS::GetFadeOnDecay ( void )const;
public: class bCString const& __thiscall gCProjectile_PS::GetFuncOnTargetHit ( void )const;
public: BOOL const& __thiscall gCProjectile_PS::GetLinkToBones ( void )const;
public: class bTPropertyContainer<enum gEProjectilePath> const& __thiscall gCProjectile_PS::GetPathStyle ( void )const;
public: virtual enum eEPropertySetType __thiscall gCProjectile_PS::GetPropertySetType ( void )const;
public: static class bCPropertyObjectBase const* __stdcall gCProjectile_PS::GetRootObject ( void );
public: float const& __thiscall gCProjectile_PS::GetShootVelocity ( void )const;
public: class bCVector const& __thiscall gCProjectile_PS::GetTargetDirection ( void )const;
public: class eCEntityProxy const& __thiscall gCProjectile_PS::GetTargetEntity ( void )const;
public: class bCVector const& __thiscall gCProjectile_PS::GetTargetPosition ( void )const;
public: unsigned long const& __thiscall gCProjectile_PS::GetTargetUpdateMSec ( void )const;
public: float const& __thiscall gCProjectile_PS::GetTouchAngleTreshold ( void )const;
public: bTPropertyContainer<gEProjectileTouchBehavior> const&GetTouchBehavior ( void )const;
public: virtual unsigned short __thiscall gCProjectile_PS::GetVersion ( void )const;
public: GEBool HasCollided ( void )const;
protected: void __thiscall gCProjectile_PS::Invalidate ( void );
public: GEBool IsFlying ( void )const;
protected: virtual BOOL __thiscall gCProjectile_PS::IsProcessable ( void )const;
public: virtual BOOL __thiscall gCProjectile_PS::IsReferencedByTemplate ( void )const;
protected: virtual void __thiscall gCProjectile_PS::OnExitProcessingRange ( void );
protected: virtual void __thiscall gCProjectile_PS::OnProcess ( void );
protected: virtual void __thiscall gCProjectile_PS::OnTouch ( class eCEntity* , class eCContactIterator& );
protected: virtual void __thiscall gCProjectile_PS::OnTrigger ( class eCEntity* , class eCEntity* );
public: virtual enum bEResult __thiscall gCProjectile_PS::PostInitializeProperties ( void );
public: virtual enum bEResult __thiscall gCProjectile_PS::Read ( class bCIStream& );
public: void __thiscall gCProjectile_PS::SetDecayDuration ( float const& );
public: void __thiscall gCProjectile_PS::SetEffectPointImpact ( class bCString const& );
public: void __thiscall gCProjectile_PS::SetEffectTargetHit ( class bCString const& );
public: void __thiscall gCProjectile_PS::SetFadeOnDecay ( BOOL const& );
public: void __thiscall gCProjectile_PS::SetFuncOnTargetHit ( class bCString const& );
public: void __thiscall gCProjectile_PS::SetLinkToBones ( BOOL const& );
public: void __thiscall gCProjectile_PS::SetPathStyle ( class bTPropertyContainer<enum gEProjectilePath> const& );
public: void __thiscall gCProjectile_PS::SetShootVelocity ( float const& );
public: void __thiscall gCProjectile_PS::SetTargetDirection ( class bCVector const& );
public: void __thiscall gCProjectile_PS::SetTargetEntity ( class eCEntityProxy const& );
public: void __thiscall gCProjectile_PS::SetTargetPosition ( class bCVector const& );
public: void __thiscall gCProjectile_PS::SetTargetUpdateMSec ( unsigned long const& );
public: void __thiscall gCProjectile_PS::SetTouchAngleTreshold ( float const& );
public: void SetTouchBehavior ( bTPropertyContainer<gEProjectileTouchBehavior> const& );
public: void __thiscall gCProjectile_PS::Shoot ( void );
protected: void __thiscall gCProjectile_PS::ShootIteration ( void );
public: virtual enum bEResult __thiscall gCProjectile_PS::Write ( class bCOStream& );
};