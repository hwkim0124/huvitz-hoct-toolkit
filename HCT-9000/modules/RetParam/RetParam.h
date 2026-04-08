// RetParam.h : main header file for the RetParam DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CRetParamApp
// See RetParam.cpp for the implementation of this class
//

class CRetParamApp : public CWinApp
{
public:
	CRetParamApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
