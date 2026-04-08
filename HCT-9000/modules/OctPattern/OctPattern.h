// OctPattern.h : main header file for the OctPattern DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctPatternApp
// See OctPattern.cpp for the implementation of this class
//

class COctPatternApp : public CWinApp
{
public:
	COctPatternApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
