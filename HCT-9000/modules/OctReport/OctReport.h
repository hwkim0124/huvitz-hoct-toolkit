// OctReport.h : main header file for the OctReport DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COctReportApp
// See OctReport.cpp for the implementation of this class
//

class COctReportApp : public CWinApp
{
public:
	COctReportApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
