// TriStateButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "TriStateButton.h"


// TriStateButton

IMPLEMENT_DYNAMIC(TriStateButton, CButton)

TriStateButton::TriStateButton()
	: m_state(0)
{
}

TriStateButton::~TriStateButton()
{
}

void TriStateButton::loadImages(UINT id1, UINT id2, UINT id3, LPCTSTR pType)
{
	m_image[0].Load(id1, pType);
	m_image[1].Load(id2, pType);
	m_image[2].Load(id3, pType);

	return;
}

void TriStateButton::redrawButton(void)
{
	return;
}

void TriStateButton::setButtonState(UINT state)
{
	m_state = state;

	Invalidate();
}

BEGIN_MESSAGE_MAP(TriStateButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// TriStateButton 메시지 처리기입니다.


BOOL TriStateButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void TriStateButton::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

BOOL TriStateButton::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		pMsg->message = WM_LBUTTONDOWN;
		break;
	default:
		break;
	}
		
	return FALSE;
}


void TriStateButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL isPressed = (lpDrawItemStruct->itemState & ODS_SELECTED);

	drawButton(pDC, &m_image[m_state]);

	return;
}

void TriStateButton::drawButton(CDC *pDC, GdiPlusBitmapResource *image)
{
	Gdiplus::Graphics G(pDC->m_hDC);


	if (image)
	{
		G.DrawImage(image->m_pBitmap, 0, 0);
	}

	return;
}