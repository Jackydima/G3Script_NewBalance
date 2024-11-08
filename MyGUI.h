#pragma once

#include "util/Memory.h"
#include "util/Logging.h"
#include "util/Hook.h"
#include <iostream>
#include "Script.h"

class MyView :
	public CFFGFCView
{
public:    GEBool OnInitDialog ( void ) override;
protected: virtual GEInt  WindowProc ( GEUInt , GEUInt , GEUInt );
public:
	void OnPaint ( ) override;
	void DoDataExchange ( CFFGFCDataExchange* ) override;
	~MyView ( void );

public:
	MyView ( MyView const& );
	MyView ( void );

public:
	Entity Focus;
	CFFGFCStatic focusStaminaBack;
	CFFGFCProgressBar staminaBar;
};

void InitGUI ( );
void DoIt ( CFFGFCWnd* a0 );



