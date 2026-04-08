// OctDevice.h : main header file for the OctDevice DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctDeviceApp
// See OctDevice.cpp for the implementation of this class
//

class COctDeviceApp : public CWinApp
{
public:
	COctDeviceApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
