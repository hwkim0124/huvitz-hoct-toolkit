// SigProc.h : main header file for the SigProc DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSigProcApp
// See SigProc.cpp for the implementation of this class
//

class CSigProcApp : public CWinApp
{
public:
	CSigProcApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
