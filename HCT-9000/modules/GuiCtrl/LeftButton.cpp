// LeftButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "GuiCtrl.h"
#include "LeftButton.h"


// LeftButton

IMPLEMENT_DYNAMIC(LeftButton, CButton)

LeftButton::LeftButton()
{
	m_nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	m_nLeftMargin = 0;
	m_nRightMargin = 0;

	m_font.CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 0, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, _T("Noto Sans CJK KR Medium"));
}

LeftButton::~LeftButton()
{
}

void LeftButton::loadResources(CString label, UINT imgNormal, UINT imgPressed, UINT imgDisabled)
{
	m_imageNormal.Load(imgNormal, kImgType);
	m_imagePush.Load(imgPressed, kImgType);
	m_imageDisable.Load(imgDisabled, kImgType);
	m_lable = label;
}

void LeftButton::setDrawTextParams(UINT nFormat, int leftMargin, int rightMargin)
{
	m_nFormat = nFormat;
	m_nLeftMargin = leftMargin;
	m_nRightMargin = rightMargin;

	return;
}

void LeftButton::redrawButton(void)
{
	return;
}

BEGIN_MESSAGE_MAP(LeftButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// LeftButton 메시지 처리기입니다.


BOOL LeftButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void LeftButton::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);

	CButton::PreSubclassWindow();
}

BOOL LeftButton::PreTranslateMessage(MSG* pMsg)
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


void LeftButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL isPressed = (lpDrawItemStruct->itemState & ODS_SELECTED);

	if (IsWindowEnabled())
	{
		if (isPressed)
		{
			drawButton(pDC, &m_imagePush, kTxtClrPressed);
		}
		else
		{
			drawButton(pDC, &m_imageNormal, kTxtClrNormal);
		}
	}
	else
	{
		drawButton(pDC, &m_imageDisable, kTxtClrDisabled);
	}

	return;
}



void LeftButton::drawButton(CDC *pDC, GdiPlusBitmapResource *image, COLORREF clrText)
{
	Gdiplus::Graphics G(pDC->m_hDC);

	// image
	if (image) {
		Gdiplus::Rect rect(0, 0, 120, 84);
		G.DrawImage(image->m_pBitmap, rect);
	}

	// text
	{
		CRect rect(1, 61, 119, 83);

		DRAWTEXTPARAMS lpDTParams;
		lpDTParams.cbSize = sizeof(lpDTParams);
		lpDTParams.iLeftMargin = m_nLeftMargin;
		lpDTParams.iRightMargin = m_nRightMargin;
		lpDTParams.iTabLength = 4;
		lpDTParams.uiLengthDrawn = 0;

		pDC->SetTextColor(clrText);
		pDC->SelectObject(&m_font);
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawTextEx(m_lable, &rect, m_nFormat, &lpDTParams);
	}

	return;
}