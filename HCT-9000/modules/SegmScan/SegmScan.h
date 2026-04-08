// SegmScan.h : main header file for the SegmScan DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSegmScanApp
// See SegmScan.cpp for the implementation of this class
//

class CSegmScanApp : public CWinApp
{
public:
	CSegmScanApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
