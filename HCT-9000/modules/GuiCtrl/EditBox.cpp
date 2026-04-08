// EditBox.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "EditBox.h"


// EditBox

IMPLEMENT_DYNAMIC(EditBox, CEdit)

EditBox::EditBox()
{
	m_clrBorder = RGB(43, 132, 240);
	m_clrGuidText = RGB(52, 52, 52);
	m_sGuidText.Empty();
	m_bTextColor = false;
	m_bDrawIfUnfocused = false;
}

EditBox::~EditBox()
{
}

void EditBox::SetMargins(int left, int top, int right, int bottom)
{
	CRect rect;
	GetWindowRect(rect);
	rect.OffsetRect(-rect.left, -rect.top);

	CRect rectText;
	rectText.SetRect(rect.left + left, rect.top + top, rect.right - right, rect.bottom - bottom);

	CEdit::SetRect(rectText);
	return;
}

void EditBox::SetBorderColor(COLORREF clr)
{
	m_clrBorder = clr;

	return;
}

void EditBox::SetGuidText(CString text, COLORREF clr)
{
	m_sGuidText = text;
	m_clrGuidText = clr;

	return;
}

void EditBox::SetTextColor(COLORREF clr)
{
	m_clrText = clr;
	m_bTextColor = true;
}

void EditBox::SetDrawIfUnfocused(bool value)
{
	m_bDrawIfUnfocused = value;
}

BEGIN_MESSAGE_MAP(EditBox, CEdit)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// EditBox 메시지 처리기입니다.

void EditBox::OnPaint()
{
	CEdit::OnPaint();

	CClientDC dc(this);

	if (GetFocus() == this)
	{
		CRect rect;
		GetClientRect(&rect);

		CBrush brush(m_clrBorder);
		dc.FrameRect(&rect, &brush);
	}
	else
	{
		CString text;
		GetWindowText(text);

		CRect rect;
		GetRect(&rect);

		if (text.IsEmpty() && !m_sGuidText.IsEmpty())
		{
			dc.SelectObject(GetFont());
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(m_clrGuidText);
			dc.DrawText(m_sGuidText, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
		}
		else {
			if (m_bDrawIfUnfocused) {
				if (m_bTextColor) {
					dc.SetTextColor(m_clrText);
				}
				dc.SelectObject(GetFont());
				dc.SetBkMode(TRANSPARENT);
				dc.DrawText(text, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
			}
		}
	}



	return;
}
