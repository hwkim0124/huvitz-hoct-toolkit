#pragma once
#include "GuiCtrlDef.h"

// EditBoxComment

class GUICTRL_DLL_API EditBoxComment : public CEdit
{
	DECLARE_DYNAMIC(EditBoxComment)

	const COLORREF kColorEditBorderN = RGB(0x4a, 0x4d, 0x50);
	const COLORREF kColorEditBorderP = RGB(0x2b, 0x84, 0xf0);

public:
	EditBoxComment();
	virtual ~EditBoxComment();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnNcPaint();
};


