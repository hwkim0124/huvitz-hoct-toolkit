// SemtSegm.h : main header file for the SemtSegm DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSemtSegmApp
// See SemtSegm.cpp for the implementation of this class
//

class CSemtSegmApp : public CWinApp
{
public:
	CSemtSegmApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
