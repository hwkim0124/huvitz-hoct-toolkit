// OctResult.h : main header file for the OctResult DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctResultApp
// See OctResult.cpp for the implementation of this class
//

class COctResultApp : public CWinApp
{
public:
	COctResultApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
