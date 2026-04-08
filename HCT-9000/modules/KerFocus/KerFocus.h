// KerFocus.h : main header file for the KerFocus DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CKerFocusApp
// See KerFocus.cpp for the implementation of this class
//

class CKerFocusApp : public CWinApp
{
public:
	CKerFocusApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
