// PushButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "PushButton.h"


// PushButton

IMPLEMENT_DYNAMIC(PushButton, CButton)

PushButton::PushButton()
	: m_showText(false)
{
	m_nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	m_nLeftMargin = 0;
	m_nRightMargin = 0;

	m_clrDisable = RGB(0, 0, 0);
	m_clrNormal = RGB(0, 0, 0);
	m_clrPush = RGB(0, 0, 0);
}

PushButton::~PushButton()
{
}

void PushButton::loadImages(UINT idNormal, UINT idPush, UINT idDisable, LPCTSTR pType)
{
	m_imageNormal.Load(idNormal, pType);
	m_imagePush.Load(idPush, pType);
	m_imageDisable.Load(idDisable, pType);

	return;
}

void PushButton::showText(bool show)
{
	m_showText = show;

	return;
}

void PushButton::setTextColor(COLORREF clrNormal, COLORREF clrToggle, COLORREF clrDisabled)
{
	m_clrNormal = clrNormal;
	m_clrPush = clrToggle;
	m_clrDisable = clrDisabled;

	return;
}

void PushButton::setDrawTextParams(UINT nFormat, int leftMargin, int rightMargin)
{
	m_nFormat = nFormat;
	m_nLeftMargin = leftMargin;
	m_nRightMargin = rightMargin;

	return;
}

void PushButton::redrawButton(void)
{
	return;
}

BEGIN_MESSAGE_MAP(PushButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// PushButton 메시지 처리기입니다.


BOOL PushButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void PushButton::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

BOOL PushButton::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		pMsg->message = WM_LBUTTONDOWN;
		break;
	default:
		break;
	}

	return false;
}


void PushButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL isPressed = (lpDrawItemStruct->itemState & ODS_SELECTED);

	if (IsWindowEnabled())
	{
		if (isPressed)
		{
			drawButton(pDC, &m_imagePush, m_clrPush);
		}
		else
		{
			drawButton(pDC, &m_imageNormal, m_clrNormal);
		}
	}
	else
	{
		drawButton(pDC, &m_imageDisable, m_clrDisable);
	}

	return;
}



void PushButton::drawButton(CDC *pDC, GdiPlusBitmapResource *image, COLORREF clrText)
{
	Gdiplus::Graphics G(pDC->m_hDC);

	CRect rect;
	GetClientRect(&rect);

	if (image) {
		G.DrawImage(image->m_pBitmap, Gdiplus::Rect(0, 0, rect.Width(), rect.Height()));
	}

	if (m_showText) {
		CString text;
		GetWindowText(text);

		DRAWTEXTPARAMS lpDTParams;
		lpDTParams.cbSize = sizeof(lpDTParams);
		lpDTParams.iLeftMargin = m_nLeftMargin;
		lpDTParams.iRightMargin = m_nRightMargin;
		lpDTParams.iTabLength = 4;
		lpDTParams.uiLengthDrawn = 0;

		pDC->SetTextColor(clrText);
		pDC->SelectObject(this->GetFont());
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawTextEx(text, &rect, m_nFormat, &lpDTParams);
	}

	return;
}