// RetSegm.h : main header file for the RetSegm DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CRetSegmApp
// See RetSegm.cpp for the implementation of this class
//

class CRetSegmApp : public CWinApp
{
public:
	CRetSegmApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
