// OctGlobal.h : main header file for the OctGlobal DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctGlobalApp
// See OctGlobal.cpp for the implementation of this class
//

class COctGlobalApp : public CWinApp
{
public:
	COctGlobalApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
