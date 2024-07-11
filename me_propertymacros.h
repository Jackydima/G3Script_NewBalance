#pragma once

#define __ME_DEFINE_PROPERTY_TYPE( EXTOBJECTCLASS, OBJECTCLASS, PROPERTYNAME, MEMBERNAME, READ_ONLY ) \
    decltype(EXTOBJECTCLASS::ms_PropertyMember_ ## MEMBERNAME) EXTOBJECTCLASS::ms_PropertyMember_ ## MEMBERNAME ( *bCPropertyObjectSingleton::GetInstance().FindTemplate( bTClassName<OBJECTCLASS>::GetUnmangled()), offsetof( EXTOBJECTCLASS, MEMBERNAME ), #PROPERTYNAME, bCString(), bEPropertyType_Normal, READ_ONLY );

#define ME_DEFINE_PROPERTY_TYPE( EXTOBJECTCLASS, OBJECTCLASS, PROPERTYNAME, MEMBERNAME ) \
    __ME_DEFINE_PROPERTY_TYPE( EXTOBJECTCLASS, OBJECTCLASS, PROPERTYNAME, MEMBERNAME, GEFalse )

#define ME_DEFINE_PROPERTY_TYPE_READ_ONLY( EXTOBJECTCLASS, OBJECTCLASS, PROPERTYNAME, MEMBERNAME ) \
    __ME_DEFINE_PROPERTY_TYPE( EXTOBJECTCLASS, OBJECTCLASS, PROPERTYNAME, MEMBERNAME, GETrue )

#define ME_DEFINE_ENUMPROP_TYPE( EXTOBJECTCLASS, OBJECTCLASS, PROPERTYNAME, MEMBERNAME ) \
    decltype(EXTOBJECTCLASS::ms_PropertyMember_ ## MEMBERNAME) EXTOBJECTCLASS::ms_PropertyMember_ ## MEMBERNAME ( *bCPropertyObjectSingleton::GetInstance().FindTemplate( bTClassName<OBJECTCLASS>::GetUnmangled()), offsetof( EXTOBJECTCLASS, MEMBERNAME ), #PROPERTYNAME, bEPropertyType_PropertyContainer );

#define ME_REWRITE_MEMBER_FUNCTION( HOOKNAME, SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, PREFIX, SUFFIX ) \
    static mCFunctionHook Hook_ ## HOOKNAME;                                                         \
    Hook_ ## HOOKNAME.Prepare(                                                                       \
        GetProcAddress( SOURCE_MODULE, PREFIX + bTClassName<SOURCE_TYPE>::GetUnmangled() + SUFFIX ), \
        GetProcAddress( TARGET_MODULE, PREFIX + bTClassName<TARGET_TYPE>::GetUnmangled() + SUFFIX ), \
        mCBaseHook::mEHookType_ThisCall ).Transparent().Hook();

#define ME_REWRITE_CONSTRUCTOR( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE ) \
    ME_REWRITE_MEMBER_FUNCTION( Constructor, SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, "??0",          "@@QAE@XZ" )

#define ME_REWRITE_DESTRUCTOR( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE ) \
    ME_REWRITE_MEMBER_FUNCTION( Destructor,  SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, "??1",          "@@UAE@XZ" )

#define ME_REWRITE_INVALIDATE( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE ) \
    ME_REWRITE_MEMBER_FUNCTION( Invalidate,  SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, "?Invalidate@", "@@IAEXXZ" )

#define ME_PATCH_PROPERTY_SET_SIZE( TYPE, SIZE_OFFSET, SIZE_TYPE ) \
    static mCDataPatch Patch_CreatePropertySetSize( SIZE_OFFSET, static_cast<SIZE_TYPE>(sizeof(TYPE)) );

#define __ME_PATCH_PROPERTY_SET( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, SIZE_OFFSET, SIZE_TYPE, REWRITE_INVALIDATE ) \
    __pragma( warning ( push ))                                                              \
    __pragma( warning( disable: 4127 ))                                                      \
    do {                                                                                     \
        ME_PATCH_PROPERTY_SET_SIZE( TARGET_TYPE, SIZE_OFFSET, SIZE_TYPE )                    \
        ME_REWRITE_CONSTRUCTOR( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE )     \
        ME_REWRITE_DESTRUCTOR( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE )      \
        if ( REWRITE_INVALIDATE )                                                            \
        { ME_REWRITE_INVALIDATE( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE ) }  \
    } while (0);                                                                             \
    __pragma( warning ( pop ))

#define ME_PATCH_PROPERTY_SET( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, SIZE_OFFSET, SIZE_TYPE ) \
    __ME_PATCH_PROPERTY_SET( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, SIZE_OFFSET, SIZE_TYPE, GETrue )

#define ME_PATCH_PROPERTY_SET_NO_INVALIDATE( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, SIZE_OFFSET, SIZE_TYPE ) \
    __ME_PATCH_PROPERTY_SET( SOURCE_MODULE, TARGET_MODULE, SOURCE_TYPE, TARGET_TYPE, SIZE_OFFSET, SIZE_TYPE, GEFalse )