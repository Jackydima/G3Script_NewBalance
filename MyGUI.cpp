#include "MyGUI.h"

static MyView myView;
static CFFGFCStatic crossHair;

void MyView::DoDataExchange ( CFFGFCDataExchange* exchange ) {
    //FF_DDX_Control ( exchange , 99381 , bar );
    CFFGFCView::DoDataExchange ( exchange );
}

GEInt MyView::WindowProc ( GEUInt , GEUInt , GEUInt ) {
    return 1;
}

GEBool MyView::OnInitDialog ( ) {
    //CFFGFCDataExchange data = CFFGFCDataExchange ( this , GEFalse );
    //DoDataExchange ( &data );
    std::cout << "OnInit!\n";
    focusStaminaBack.Create ( "" , 0x10 , bCRect ( 0 , 0 , 201 , 20 ) , this , 1 );
    CFFGFCBitmap barBack;
    barBack.Create ( "g3_hud_statusbar_back.tga" );
    focusStaminaBack.SetBitmap ( barBack.GetHandle ( ) );
    focusStaminaBack.ShowWindow ( GETrue );

    staminaBar.Create ( "" , 0x10 , bCRect ( 12 , 4 , 188 , 16 ) , this , 2 );
    staminaBar.SetBarColor( *( bCByteAlphaColor* )RVA_Game ( 0x3f2aac )); // SP Regular Colour
    staminaBar.SetRange ( 0 , 100 );
    staminaBar.SetPos ( 100 );
    staminaBar.ShowWindow ( GETrue );

    CFFGFCView::OnInitDialog ( );
    return GETrue; 
}

void MyView::OnPaint ( ) {
    //std::cout << "Go Paint!\n";
    Entity Player = Entity::GetPlayer ( );
    Entity FocusEntity = Player.Focus.GetFocusEntity ( );
    if ( FocusEntity != None && FocusEntity.IsNPC ( ) ) {
        this->Focus = FocusEntity;
        //myView.ShowWindow ( GETrue );
        myView.AnimateWindow ( 500 , 0xa0000 );
        //focusStaminaBack.SetWindowTextA ( Focus.GetFocusName ( ) );
        GEInt staminaMax = FocusEntity.DamageReceiver.GetProperty<PSDamageReceiver::PropertyStaminaPointsMax> ( );
        GEInt stamina = FocusEntity.DamageReceiver.GetProperty<PSDamageReceiver::PropertyStaminaPoints> ( );
        if ( staminaMax != 0 ) {
            GEFloat percentage = (static_cast<GEFloat>(stamina) / staminaMax) * 100;
            //std::cout << "Stamina now: " << stamina << "\tPercentage: " << percentage << "\n";
            staminaBar.SetPos ( static_cast<GEInt>(percentage));
        }
    }
    else {
        //myView.ShowWindow ( GEFalse );A
        myView.AnimateWindow ( 500 , 0x90000 );
    }

    staminaBar.BringWindowToTop ( );
    staminaBar.OnPaint ( );
    focusStaminaBack.OnPaint ( );
    //CFFGFCView::OnPaint ( );
}
MyView::~MyView ( void ) {}
MyView::MyView ( MyView const& p_myView ) : CFFGFCView ( p_myView ) {}
MyView::MyView ( ) : CFFGFCView ( ) {}


static mCFunctionHook Hook_CFFGFCWnd_OnPaint;
void CFFGFCWnd_OnPaint ( void ) {
    CFFGFCWnd* This = Hook_CFFGFCWnd_OnPaint.GetSelf<CFFGFCWnd*> ( );
    if ( This->GetDlgCtrlID ( ) == 20533 )
    {
        DoIt ( This );
    }

    Hook_CFFGFCWnd_OnPaint.GetOriginalFunction ( &CFFGFCWnd_OnPaint )( );
}

static mCFunctionHook Hook_MagicAimDeactivate;
//OnTick()
GEInt MagicAimDeactivate ( gCScriptProcessingUnit* a_pSPU , Entity* a_pSelfEntity , Entity* a_pOtherEntity , GEInt p_args ) {
    INIT_SCRIPT_EXT ( Self , Other );
    if ( Self.IsPlayer ( )
        && !Self.Routine.GetCurrentTask ( ).Contains ( "PS_Magic" ) /* && !Self.Routine.GetCurrentTask ( ).Contains ( "PS_Ranged" )*/ ) {
        crossHair.AnimateWindow ( 500 , 0x90000 );
    }
    return Hook_MagicAimDeactivate.GetOriginalFunction ( &MagicAimDeactivate )( a_pSPU , a_pSelfEntity , a_pOtherEntity , p_args );
}

static mCFunctionHook Hook_MagicAim;
//PS_Magic_...()
GEInt MagicAim ( GEInt* p_ptr , gCScriptProcessingUnit* p_spu ) {
    //Player.EnableAutoAiming ( GETrue , None , "" ); // The variable CrosshairSize by Distance of Point
    crossHair.AnimateWindow ( 600 , 0xa0000 );
    return Hook_MagicAim.GetOriginalFunction ( &MagicAim )( p_ptr , p_spu );
}

void InitGUI ( ) {
    CFFGFCWnd* dW = ( ( CFFGFCWnd* )0 )->GetDesktopWindow ( );
#ifdef DEBUG
    bCRect test = bCRect ( 411 , 30 , 411 + 200 , 30 + 19 );
    myView.Create ( NULL , 0x10 , test , dW , UINT_MAX );
    myView.OnInitDialog ( );
#endif

    bCRect ClientRect;
    crossHair.GetDesktopWindow ( )->GetClientRect ( ClientRect );
    GEI32 iX = ( ClientRect.GetWidth ( ) - 40 ) / 2;
    GEI32 iY = ( ClientRect.GetHeight ( ) - 40 ) / 2;
    bCRect DrawBox ( iX , iY , iX + 40 , iY + 40 );
    crossHair.Create ( NULL , 0x10 , DrawBox , dW , UINT_MAX );
    CFFGFCBitmap bitMap;
    bitMap.Create ( "G3_HUD_CrossHair.tga" );
    auto handle = bitMap.GetHandle ( );
    crossHair.SetBitmap ( handle );
    crossHair.CenterWindow ( );
    crossHair.SetWindowTextA ( "" );
    crossHair.EnableWindow ( GETrue );

    Hook_CFFGFCWnd_OnPaint
        .Prepare ( RVA_GFC ( 0x4680 ) , &CFFGFCWnd_OnPaint , mCBaseHook::mEHookType_ThisCall )
        .Hook ( );
    Hook_MagicAimDeactivate
        .Prepare ( RVA_ScriptGame ( 0xb0ef0 ) , &MagicAimDeactivate )
        .Hook ( );
    Hook_MagicAim
        .Prepare ( RVA_ScriptGame ( 0x79e20 ) , &MagicAim )
        .Hook ( );
}

void DoIt ( CFFGFCWnd* a0 ) {
    if ( !a0 || !gCSession::GetSession ( ).IsValid ( ) || gCSession::GetSession ( ).IsPaused ( ) || !gCSession::GetSession ( ).GetGUIManager ( ) )
    {
        return;
    }

    if ( gCSession::GetInstance ( ).GetGUIManager ( )->IsMenuOpen ( ) || gCSession::GetInstance ( ).GetGUIManager ( )->IsAnyPageOpen ( ) || gCInfoManager_PS::GetInstance ( )->IsRunning )
    {
        return;
    }

    if ( gCInfoManager_PS::GetInstance ( )->IsRunning )
    {
        return;
    }
    crossHair.CenterWindow ( );
    crossHair.OnPaint ( );
#ifdef DEBUG
    myView.OnPaint ( );
#endif
}