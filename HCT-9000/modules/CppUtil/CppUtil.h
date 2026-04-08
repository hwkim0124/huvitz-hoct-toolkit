// CppUtil.h : main header file for the CppUtil DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCppUtilApp
// See CppUtil.cpp for the implementation of this class
//

class CCppUtilApp : public CWinApp
{
public:
	CCppUtilApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

