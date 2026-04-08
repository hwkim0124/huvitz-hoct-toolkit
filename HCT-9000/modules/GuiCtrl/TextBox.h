#pragma once
#include "GuiCtrlDef.h"

// TextBox

class GUICTRL_DLL_API TextBox : public CStatic
{
	DECLARE_DYNAMIC(TextBox)

public:
	TextBox();
	virtual ~TextBox();

	void SetAlign(UINT nformat = DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	void SetTextColor(COLORREF clr);
	void SetInteger(int value);
	void SetDouble(double value);
	void SetString(CString value);

private:
	COLORREF m_clrText;
	UINT m_nFormat;
	CString	m_text;

	

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


