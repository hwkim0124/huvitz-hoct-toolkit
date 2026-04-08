// OctData.h : main header file for the OctData DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctDataApp
// See OctData.cpp for the implementation of this class
//

class COctDataApp : public CWinApp
{
public:
	COctDataApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
