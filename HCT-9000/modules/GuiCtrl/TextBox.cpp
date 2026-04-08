// TextBox.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "TextBox.h"


// TextBox

IMPLEMENT_DYNAMIC(TextBox, CStatic)

TextBox::TextBox()
{
	m_text.Empty();
	m_nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	m_clrText = RGB(255, 255, 255);
}

TextBox::~TextBox()
{
}


BEGIN_MESSAGE_MAP(TextBox, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CHctTextBox 메시지 처리기입니다.

void TextBox::SetAlign(UINT nFormat)
{
	m_nFormat = nFormat;
}

void TextBox::SetTextColor(COLORREF clr)
{
	m_clrText = clr;
}

void TextBox::SetInteger(int value)
{
	m_text.Format(_T("%d"), value);
	Invalidate(TRUE);
}

void TextBox::SetDouble(double value)
{
	m_text.Format(_T("%.2f"), value);
	Invalidate(TRUE);
}

void TextBox::SetString(CString value)
{
	m_text = value;
	Invalidate(TRUE);
}

void TextBox::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	dc.SelectObject(this->GetFont());
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(m_clrText);
	dc.DrawText(m_text, &rect, m_nFormat);

	return;
}
