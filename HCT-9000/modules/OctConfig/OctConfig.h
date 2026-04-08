// OctConfig.h : main header file for the OctConfig DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctConfigApp
// See OctConfig.cpp for the implementation of this class
//

class COctConfigApp : public CWinApp
{
public:
	COctConfigApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
