// HctFocusBar.cpp : implementation file
//

#include "stdafx.h"
#include "FocusBar.h"
#include "windows.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FocusBar

FocusBar::FocusBar()
{
	m_dcBk = NULL;

	m_crThumbColor[0] = RGB(255, 255, 255);
	m_crThumbColor[1] = RGB(255, 0, 0);
}

FocusBar::~FocusBar()
{	
	::SelectObject(m_dcBk, m_bmpBkOld);
	::DeleteObject(m_bmpBk);
	::DeleteDC(m_dcBk);			
}


BEGIN_MESSAGE_MAP(FocusBar, CSliderCtrl)
	//{{AFX_MSG_MAP(HctFocusBar)
		ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
		ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FocusBar message handlers
//-------------------------------------------------------------------
//


void FocusBar::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
//
// Description	:	Sent by the slider control to notify the parent window 
//					about drawing operations. This notification is sent in 
//					the form of a WM_NOTIFY message.
// Parameters	:	pNMHDR - a pointer to a NM_CUSTOMDRAW structure.
//					pResult - value depends on the current drawing state.
{
	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;
	CDC *pDC = CDC::FromHandle(lpcd->hdc);
	DWORD dwStyle = this->GetStyle();
	switch(lpcd->dwDrawStage)
	{
		case CDDS_PREPAINT:
			
			*pResult = CDRF_NOTIFYITEMDRAW;
		break;
		case CDDS_ITEMPREPAINT:
		{
			switch(lpcd->dwItemSpec)
			{
				case TBCD_TICS:
					*pResult = CDRF_DODEFAULT;
					break;
				case TBCD_THUMB:
					DrawThumb(pDC, lpcd);
					*pResult = CDRF_SKIPDEFAULT;
				break;
				case TBCD_CHANNEL:
					DrawTrack(pDC, lpcd);
					*pResult = CDRF_SKIPDEFAULT;
					break;
			}
			break;
		}
	}
}

void FocusBar::DrawTrack(CDC *pDC, LPNMCUSTOMDRAW lpcd)
{
	CClientDC clientDC(GetParent());
	CRect crect;
	CRect wrect;
	GetClientRect(crect);
	GetWindowRect(wrect);
	GetParent()->ScreenToClient(wrect);

	if (m_dcBk == NULL)
	{
		m_dcBk = CreateCompatibleDC(clientDC.m_hDC);
		m_bmpBk = CreateCompatibleBitmap(clientDC.m_hDC, crect.Width(), crect.Height());
		m_bmpBkOld = (HBITMAP)::SelectObject(m_dcBk, m_bmpBk);
		::BitBlt(m_dcBk, 0, 0, crect.Width(), crect.Height(), clientDC.m_hDC, wrect.left, wrect.top, SRCCOPY);
	}

	//This bit does the tics marks transparently.
	//create a memory dc to hold a copy of the oldbitmap data that includes the tics,
	//because when we add the background in we will lose the tic marks
	HDC hSaveHDC;
	HBITMAP hSaveBmp;
	int iWidth = crect.Width();
	int iHeight = crect.Height();
	hSaveHDC = ::CreateCompatibleDC(pDC->m_hDC);
	hSaveBmp = ::CreateCompatibleBitmap(hSaveHDC, iWidth, iHeight);
	HBITMAP hSaveCBmpOld = (HBITMAP)::SelectObject(hSaveHDC, hSaveBmp);			
	//set the colours for the monochrome mask bitmap
	COLORREF crOldBack = ::SetBkColor(pDC->m_hDC, RGB(0,0,0));
	COLORREF crOldText = ::SetTextColor(pDC->m_hDC, RGB(255,255,255));		
	::BitBlt(hSaveHDC, 0, 0, iWidth, iHeight, pDC->m_hDC, crect.left, crect.top, SRCCOPY);
	::BitBlt(pDC->m_hDC, 0, 0, iWidth, iHeight, m_dcBk, 0, 0, SRCCOPY);
	::BitBlt(pDC->m_hDC, 0, 0, iWidth, iHeight, hSaveHDC, 0, 0, SRCAND);
	//restore and clean up
	::SetBkColor(pDC->m_hDC, crOldBack);
	::SetTextColor(pDC->m_hDC, crOldText);
	::SelectObject(hSaveHDC, hSaveCBmpOld);
	::DeleteObject(hSaveBmp);
	::DeleteDC(hSaveHDC);


	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Image *image = m_imageTrack;

	INT x, y;
	x = crect.CenterPoint().x - image->GetWidth() / 2;
	y = crect.CenterPoint().y - image->GetHeight() / 2;
	G.DrawImage(image, x, y);

	return;
}

void FocusBar::DrawThumb(CDC *pDC, LPNMCUSTOMDRAW lpcd)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Image *image = m_imageThumb;

	CRect rectThumb;
	GetThumbRect(&rectThumb);

	G.DrawImage(image, rectThumb.left, rectThumb.top);

	return;
}

BOOL FocusBar::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void FocusBar::SetThumbColors(COLORREF face, COLORREF highlight)
{
	m_crThumbColor[0] = face;
	m_crThumbColor[1] = highlight;

	return;
}

BOOL FocusBar::LoadTrackImage(UINT id, LPCTSTR type)
{
	return m_imageTrack.Load(id, type);
}

BOOL FocusBar::LoadThumbImage(UINT id, LPCTSTR type)
{
	return m_imageThumb.Load(id, type);
}
