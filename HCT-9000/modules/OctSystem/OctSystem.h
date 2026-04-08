// OctSystem.h : main header file for the OctSystem DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctSystemApp
// See OctSystem.cpp for the implementation of this class
//

class COctSystemApp : public CWinApp
{
public:
	COctSystemApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
