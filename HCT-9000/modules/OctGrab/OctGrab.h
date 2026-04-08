// OctGrab.h : main header file for the OctGrab DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctGrabApp
// See OctGrab.cpp for the implementation of this class
//

class COctGrabApp : public CWinApp
{
public:
	COctGrabApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
