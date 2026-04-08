// SemtRetina.h : main header file for the SemtRetina DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSemtRetinaApp
// See SemtRetina.cpp for the implementation of this class
//

class CSemtRetinaApp : public CWinApp
{
public:
	CSemtRetinaApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
