// ColorButton.cpp : implementation file
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "ColorButton.h"


// ColorButton

IMPLEMENT_DYNAMIC(ColorButton, CButton)

ColorButton::ColorButton()
{

}

ColorButton::~ColorButton()
{
}

void ColorButton::setColor(COLORREF color)
{
	m_color = color;
}

BEGIN_MESSAGE_MAP(ColorButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// ColorButton message handlers

BOOL ColorButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void ColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	CRect rcClient;
	GetClientRect(&rcClient);

	pDC->FillRect(&rcClient, new CBrush(m_color));
}

void ColorButton::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}
