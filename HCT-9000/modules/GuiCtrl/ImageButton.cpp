//
// ImageButton.cpp
//
#include "stdafx.h"
#include "ImageButton.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ImageButton


ImageButton::ImageButton()
{
	m_enableToggle = false;
	m_isToggle = false;

	m_bShowText = false;
	m_nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	m_iLeftMargin = 0;
	m_iRightMargin = 0;

	m_clrStdText = RGB(0, 0, 0);
	m_clrPusText = RGB(0, 0, 0);
	m_clrAltText = RGB(0, 0, 0);
}

ImageButton::~ImageButton()
{
}


BEGIN_MESSAGE_MAP(ImageButton, CButton)
	//{{AFX_MSG_MAP(ImageButton)
	ON_WM_DRAWITEM()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void ImageButton::RefreshImage(void)
{
	m_dcStd.DeleteDC();
	m_dcAlt.DeleteDC();
	m_dcPus.DeleteDC();

	return;
}

bool ImageButton::LoadStdImage(UINT id, LPCTSTR pType)
{
	return m_pStdImage.Load(id, pType);
}

bool ImageButton::LoadAltImage(UINT id, LPCTSTR pType)
{
	return m_pAltImage.Load(id, pType);
}


bool ImageButton::LoadPusImage(UINT id, LPCTSTR pType)
{
	return m_pPusImage.Load(id, pType);
}

HBRUSH ImageButton::CtlColor(CDC* pDC, UINT nCtlColor)
{
	Gdiplus::Graphics graphics(pDC->m_hDC);
	CBitmap bmp, *pOldBitmap;

	CRect rect;
	GetClientRect(rect);

	
	// standard image
	if (m_dcStd.m_hDC == NULL && m_pStdImage)
	{
		graphics.DrawImage(m_pStdImage, 0, 0);

		m_dcStd.CreateCompatibleDC(pDC);
		bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = m_dcStd.SelectObject(&bmp);
		m_dcStd.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
		bmp.DeleteObject();
	}

	// pressed image
	if (m_dcPus.m_hDC == NULL && m_pPusImage)
	{
		graphics.DrawImage(m_pPusImage, 0, 0);

		m_dcPus.CreateCompatibleDC(pDC);
		bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = m_dcPus.SelectObject(&bmp);
		m_dcPus.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
		bmp.DeleteObject();
	}

	// alternate image
	if (m_dcAlt.m_hDC == NULL && m_pAltImage)
	{
		graphics.DrawImage(m_pAltImage, 0, 0);

		m_dcAlt.CreateCompatibleDC(pDC);
		bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = m_dcAlt.SelectObject(&bmp);
		m_dcAlt.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);
		bmp.DeleteObject();
	}


	return NULL;
}

//=============================================================================
// enables the toggle mode
// returns if it doesn't have the alternate image
//=============================================================================
void ImageButton::EnableToggle(bool enable)
{
	m_enableToggle = enable;
}

void ImageButton::SetToggle(bool toggle)
{
	m_isToggle = toggle;

	Invalidate();
}

bool ImageButton::GetToggle(void)
{
	return m_isToggle;
}

void ImageButton::ShowWindowText(bool bShow)
{
	m_bShowText = bShow;
}

void ImageButton::SetTextColor(COLORREF clrStd, COLORREF clrPus, COLORREF clrAlt)
{
	m_clrStdText = clrStd;
	m_clrPusText = clrPus;
	m_clrAltText = clrAlt;
}

void ImageButton::SetDrawTextParams(UINT nFormat, int leftMargin, int rightMargin)
{
	m_nFormat = nFormat;
	m_iLeftMargin = leftMargin;
	m_iRightMargin = rightMargin;
}

//=============================================================================
// set the control to owner draw
//=============================================================================
void ImageButton::PreSubclassWindow()
{
	// Set control to owner draw
	ModifyStyle(0, BS_OWNERDRAW, SWP_FRAMECHANGED);
//	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	CButton::PreSubclassWindow();
}

//=============================================================================
// disable double click 
//=============================================================================
BOOL ImageButton::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDBLCLK)
		pMsg->message = WM_LBUTTONDOWN;

	return CButton::PreTranslateMessage(pMsg);
}


//=============================================================================
// overide the erase function
//=============================================================================
BOOL ImageButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

//=============================================================================
// Paint the button depending on the state of the mouse
//=============================================================================
void ImageButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rect = lpDIS->rcItem;
	BOOL isPressed = (lpDIS->itemState & ODS_SELECTED);

	//	if(m_bEnableToggle && bIsPressed)
	//	{
	//		m_bToggle = !m_bToggle;
	//	}

	if (isPressed && m_pPusImage)
	{
		pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcPus, 0, 0, SRCCOPY);
		pDC->SetTextColor(m_clrPusText);
	}
	else if (!m_isToggle && m_pStdImage)
	{
		pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcStd, 0, 0, SRCCOPY);
		pDC->SetTextColor(m_clrStdText);
	}
	else if (m_isToggle && m_pAltImage)
	{
		pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcAlt, 0, 0, SRCCOPY);
		pDC->SetTextColor(m_clrAltText);
	}
	
	if (m_bShowText)
	{
		CString text;
		GetWindowText(text);

		DRAWTEXTPARAMS lpDTParams;
		lpDTParams.cbSize = sizeof(lpDTParams);
		lpDTParams.iLeftMargin = m_iLeftMargin;
		lpDTParams.iRightMargin = m_iRightMargin;
		lpDTParams.iTabLength = 4;
		lpDTParams.uiLengthDrawn = 0;

		pDC->SelectObject(this->GetFont());
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawTextEx(text, &rect, m_nFormat, &lpDTParams);
	}

	return;
}
