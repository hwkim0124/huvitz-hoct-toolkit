// NormData.h : main header file for the NormData DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CNormDataApp
// See NormData.cpp for the implementation of this class
//

class CNormDataApp : public CWinApp
{
public:
	CNormDataApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
