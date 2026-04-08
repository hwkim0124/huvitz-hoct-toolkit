// SigChain.h : main header file for the SigChain DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSigChainApp
// See SigChain.cpp for the implementation of this class
//

class CSigChainApp : public CWinApp
{
public:
	CSigChainApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
