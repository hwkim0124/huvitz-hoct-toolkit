// OctAngio.h : main header file for the OctAngio DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctAngioApp
// See OctAngio.cpp for the implementation of this class
//

class COctAngioApp : public CWinApp
{
public:
	COctAngioApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
