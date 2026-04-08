#pragma once
#include "GuiCtrlDef.h"

// EditBox

class GUICTRL_DLL_API EditBox : public CEdit
{
	DECLARE_DYNAMIC(EditBox)

public:
	EditBox();
	virtual ~EditBox();

	void SetMargins(int left, int top, int right, int bottom);
	void SetBorderColor(COLORREF clr);
	void SetGuidText(CString text, COLORREF clr);
	void SetTextColor(COLORREF clr);
	void SetDrawIfUnfocused(bool value);

protected:
	COLORREF m_clrBorder;
	COLORREF m_clrGuidText;
	COLORREF m_clrText;
	CString m_sGuidText;
	bool m_bTextColor;
	bool m_bDrawIfUnfocused;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


