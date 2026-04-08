// ImgProc.h : main header file for the ImgProc DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CImgProcApp
// See ImgProc.cpp for the implementation of this class
//

class CImgProcApp : public CWinApp
{
public:
	CImgProcApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
