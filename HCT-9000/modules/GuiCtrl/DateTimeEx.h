#pragma once
#include "GuiCtrlDef.h"

// DateTimeEx

class GUICTRL_DLL_API DateTimeEx : public CDateTimeCtrl
{
	DECLARE_DYNAMIC(DateTimeEx)

public:
	DateTimeEx();
	virtual ~DateTimeEx();

private:
	CFont font;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDtnDropdown(NMHDR *pNMHDR, LRESULT *pResult);
};


