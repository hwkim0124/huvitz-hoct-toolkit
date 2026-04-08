// ToggleButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "ToggleButton.h"


// ToggleButton

IMPLEMENT_DYNAMIC(ToggleButton, CButton)

ToggleButton::ToggleButton()
	: m_isEnable(true), m_isToggle(false), m_showText(false)
	, m_nIconRect(0, 0, 0, 0)
{
	m_nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	m_nLeftMargin = 0;
	m_nRightMargin = 0;

	m_clrNormal = RGB(0, 0, 0);
	m_clrNormalPush = RGB(0, 0, 0);
	m_clrToggle = RGB(0, 0, 0);
	m_clrTogglePush = RGB(0, 0, 0);
}

ToggleButton::~ToggleButton()
{
}

void ToggleButton::loadImages(UINT idNormal, UINT idToggle, UINT idDisable, LPCTSTR pType)
{
	m_imageDisable.Load(idDisable, pType);
	m_imageNormal.Load(idNormal, pType);
	m_imageNormalPush.Load(idNormal, pType);
	m_imageToggle.Load(idToggle, pType);
	m_imageTogglePush.Load(idToggle, pType);

	return;
}

void ToggleButton::loadImages(UINT idNormal, UINT idNormalPush, UINT idToggle, UINT idTogglePush, UINT idDisable, LPCTSTR pType)
{
	m_imageDisable.Load(idDisable, pType);
	m_imageNormal.Load(idNormal, pType);
	m_imageNormalPush.Load(idNormalPush, pType);
	m_imageToggle.Load(idToggle, pType);
	m_imageTogglePush.Load(idTogglePush, pType);

	return;
}

void ToggleButton::showText(bool show)
{
	m_showText = show;

	return;
}

void ToggleButton::setTextColor(COLORREF clrNormal, COLORREF clrToggle)
{
	m_clrNormal = clrNormal;
	m_clrNormalPush = clrNormal;
	m_clrToggle = clrToggle;
	m_clrTogglePush = clrToggle;

	return;
}

void ToggleButton::setDrawTextParams(UINT nFormat, int leftMargin, int rightMargin)
{
	m_nFormat = nFormat;
	m_nLeftMargin = leftMargin;
	m_nRightMargin = rightMargin;

	return;
}

void ToggleButton::redrawButton(void)
{
	return;
}

BOOL ToggleButton::EnableWindow(BOOL bEnable)
{
	m_isEnable = bEnable == TRUE;

	return CWnd::EnableWindow(bEnable);
}

void ToggleButton::setToggle(bool toggle)
{
	m_isToggle = toggle;

	if (m_hWnd)
	{
		Invalidate();
	}
}

bool ToggleButton::getToggle(void)
{
	return m_isToggle;
}

void ToggleButton::setIconRect(int x, int y, int width, int height)
{
	m_nIconRect = Gdiplus::Rect(x, y, width, height);
}


BEGIN_MESSAGE_MAP(ToggleButton, CButton)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// ToggleButton 메시지 처리기입니다.


int ToggleButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

BOOL ToggleButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void ToggleButton::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

BOOL ToggleButton::PreTranslateMessage(MSG* pMsg)
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


void ToggleButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL isPressed = (lpDrawItemStruct->itemState & ODS_SELECTED);

	if (m_isEnable)
	{
		if (m_isToggle)
		{
			if (isPressed)
			{
				drawButton(pDC, &m_imageTogglePush, m_clrTogglePush);
			}
			else
			{
				drawButton(pDC, &m_imageToggle, m_clrToggle);
			}
		}
		else
		{
			if (isPressed)
			{
				drawButton(pDC, &m_imageNormalPush, m_clrNormalPush);
			}
			else
			{
				drawButton(pDC, &m_imageNormal, m_clrNormal);
			}
		}
	}
	else
	{
		drawButton(pDC, &m_imageDisable, m_clrDisable);
	}


	return;
}

void ToggleButton::drawButton(CDC *pDC, GdiPlusBitmapResource *image, COLORREF clrText)
{
	Gdiplus::Graphics G(pDC->m_hDC);

	CRect rect;
	GetClientRect(&rect);

	if (image) {
		if (m_nIconRect.IsEmptyArea()) {
			G.DrawImage(image->m_pBitmap, Gdiplus::Rect(0, 0, rect.Width(), rect.Height()));
		}
		else {
			G.DrawImage(image->m_pBitmap, m_nIconRect);
		}
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