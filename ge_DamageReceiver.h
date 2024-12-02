#pragma once
#include "Script.h"

class GE_DLLIMPORT gCDamageReceiver_PS :
    public eCEntityPropertySet
{
public: virtual unsigned short GetVersion ( void )const;
public: virtual bEResult Write ( bCOStream& );
public: virtual bEResult Read ( bCIStream& );
public: virtual void Destroy ( void );
public: virtual bEResult Create ( void );
public: virtual bEResult CopyFrom ( bCObjectBase const& );
public: virtual ~gCDamageReceiver_PS ( void );
public: virtual bEResult PostInitializeProperties ( void );
public: virtual eEPropertySetType GetPropertySetType ( void )const;
public: virtual bEResult ReadSaveGame ( bCIStream& );
public: virtual bEResult WriteSaveGame ( bCOStream& );
public: virtual eCEntityPropertySet::eEPSSaveGameRelevance GetSaveGameRelevance ( void )const;
public: virtual void OnProcess ( void );

public:
    static bTPropertyType<gCDamageReceiver_PS , eCEntityProxy> ms_PropertyMember_m_LastInflictor;
    static bTPropertyType<gCDamageReceiver_PS , bTPropertyContainer<gEDamageType> > ms_PropertyMember_m_enuDamageType;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_i32DamageAmount;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_iHitPoints;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_iHitPointsMax;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_iManaPoints;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_iManaPointsMax;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_iStaminaPoints;
    static bTPropertyType<gCDamageReceiver_PS , long> ms_PropertyMember_m_iStaminaPointsMax;

private:
    static bTPropertyObject<gCDamageReceiver_PS , eCEntityPropertySet> ms_PropertyObjectInstance_gCDamageReceiver_PS;

public:
    static bCPropertyObjectBase const* GE_STDCALL GetRootObject ( void );

public:
    gCDamageReceiver_PS ( gCDamageReceiver_PS const& );
    gCDamageReceiver_PS ( void );

public:
    gCDamageReceiver_PS const& operator=( gCDamageReceiver_PS const& );

public:
    long& AccessDamageAmount ( void );
    bTPropertyContainer<gEDamageType>& AccessDamageType ( void );
    long& AccessHitPoints ( void );
    long& AccessHitPointsMax ( void );
    eCEntityProxy& AccessLastInflictor ( void );
    long& AccessManaPoints ( void );
    long& AccessManaPointsMax ( void );
    long& AccessStaminaPoints ( void );
    long& AccessStaminaPointsMax ( void );
    long const& GetDamageAmount ( void )const;
    bTPropertyContainer<gEDamageType> const& GetDamageType ( void )const;
    long const& GetHitPoints ( void )const;
    long const& GetHitPointsMax ( void )const;
    eCEntityProxy const& GetLastInflictor ( void )const;
    long const& GetManaPoints ( void )const;
    long const& GetManaPointsMax ( void )const;
    long const& GetStaminaPoints ( void )const;
    long const& GetStaminaPointsMax ( void )const;
    void SetDamageAmount ( long const& );
    void SetDamageType ( bTPropertyContainer<gEDamageType> const& );
    void SetHitPoints ( long const& );
    void SetHitPointsMax ( long const& );
    void SetLastInflictor ( eCEntityProxy const& );
    void SetManaPoints ( long const& );
    void SetManaPointsMax ( long const& );
    void SetStaminaPoints ( long const& );
    void SetStaminaPointsMax ( long const& );

protected:
    void Invalidate ( void );

};

