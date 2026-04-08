// SkinScrollbar.cpp : implementation file
//

#include "stdafx.h"
#include "ScrollbarEx.h"

// CScrollBarEx

IMPLEMENT_DYNAMIC(CScrollBarEx, CScrollBar)

CScrollBarEx::CScrollBarEx()
{
	m_pParent = NULL;
	
	m_bEnable			   = true;
	m_bHorizontal          = FALSE;
	m_bMouseDown           = FALSE;
	m_bMouseOverArrowRight = FALSE;
	m_bMouseOverArrowLeft  = FALSE;
	m_bMouseOverArrowUp    = FALSE;
	m_bMouseOverArrowDown  = FALSE;
	m_bMouseDownArrowLeft  = FALSE;
	m_bMouseDownArrowRight = FALSE;
	m_bMouseDownArrowUp    = FALSE;
	m_bMouseDownArrowDown  = FALSE;
	m_bDragging            = FALSE;
	m_bThumbHover          = FALSE;
	
	m_rcThumb			= CRect(-1,-1,-1,-1);
	m_rcClient			= CRect(-1,-1,-1,-1);
	m_rcLeftArrow		= CRect(-1,-1,-1,-1);
	m_rcRightArrow		= CRect(-1,-1,-1,-1);
	m_rcUpArrow			= CRect(-1,-1,-1,-1);
	m_rcDownArrow		= CRect(-1,-1,-1,-1);

	m_nPos              = 0;
	m_nMinPos           = 0;
	m_nMaxPos           = 0;
	m_nRange            = 0;
	m_nPage				= 0;
	m_nThumbLeft        = 20;
	m_nThumbTop         = 20;
//	m_nBitmapWidth      = SIZE_WIDTH;
//	m_nBitmapHeight     = SIZE_HEIGHT;

	m_nButtonWidth = 66;
	m_nButtonHeight = 60;

	OnDeleteBitmaps();
}

CScrollBarEx::~CScrollBarEx()
{	
	OnDeleteBitmaps();
}

BEGIN_MESSAGE_MAP(CScrollBarEx, CScrollBar)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CScrollBarEx message handlers

void CScrollBarEx::OnDeleteBitmaps()
{
	m_bmpThumb[0].DeleteObject();
	m_bmpThumb[1].DeleteObject();
	m_bmpChannel[0].DeleteObject();
	m_bmpChannel[1].DeleteObject();
	m_bmpArrowUp[0].DeleteObject();
	m_bmpArrowUp[1].DeleteObject();
	m_bmpArrowDown[0].DeleteObject();
	m_bmpArrowDown[1].DeleteObject();
	m_bmpArrowLeft[0].DeleteObject();
	m_bmpArrowLeft[1].DeleteObject();
	m_bmpArrowRight[0].DeleteObject();
	m_bmpArrowRight[1].DeleteObject();

	return;
}

BOOL CScrollBarEx::CreateContol(CWnd* pWnd, bool bHorizontal, CRect rcSize, UINT ID)
{
	if (pWnd == NULL) return false;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;

	bHorizontal ? dwStyle |= SBS_HORZ : dwStyle |= SBS_VERT ;

	return CreateFromWindow(dwStyle,pWnd,rcSize,ID);
}

BOOL CScrollBarEx::InitControl(CWnd* pWnd, bool bHorizontal)
{
	if (pWnd == NULL) return false;

	CRect rect;
	GetWindowRect(&rect);
	pWnd->ScreenToClient(&rect);

	UINT id = GetDlgCtrlID();
	BOOL ret = DestroyWindow();
	if (ret == FALSE) return false;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;

	bHorizontal ? dwStyle |= SBS_HORZ : dwStyle |= SBS_VERT ;

	return CreateFromWindow(dwStyle,pWnd,rect,id);
}

void CScrollBarEx::SetBitmapThumb(UINT nNormal, UINT nActive)
{
	m_bmpThumb[0].LoadBitmapW(nNormal);
	m_bmpThumb[1].LoadBitmapW(nActive);

	return;
}

void CScrollBarEx::SetBitmapArrowUp(UINT nNormal, UINT nActive)
{
	m_bmpArrowUp[0].LoadBitmapW(nNormal);
	m_bmpArrowUp[1].LoadBitmapW(nActive);

	return;
}

void CScrollBarEx::SetBitmapArrowDown(UINT nNormal, UINT nActive)
{
	m_bmpArrowDown[0].LoadBitmapW(nNormal);
	m_bmpArrowDown[1].LoadBitmapW(nActive);

	return;
}

void CScrollBarEx::SetBitmapArrowLeft(UINT nNormal, UINT nActive)
{
	m_bmpArrowLeft[0].LoadBitmapW(nNormal);
	m_bmpArrowLeft[1].LoadBitmapW(nActive);

	return;
}

void CScrollBarEx::SetBitmapArrowRight(UINT nNormal, UINT nActive)
{
	m_bmpArrowRight[0].LoadBitmapW(nNormal);
	m_bmpArrowRight[1].LoadBitmapW(nActive);

	return;
}

BOOL CScrollBarEx::CreateFromWindow(DWORD dwStyle, CWnd* pParentWnd, CRect rcRect, UINT nId)
{
	ASSERT(pParentWnd);
	ASSERT(IsWindow(pParentWnd->m_hWnd));

//	if (rcRect.Width() < SIZE_WIDTH)
//	{
//		rcRect.right = rcRect.left + SIZE_WIDTH;
//	}
//
//	if (rcRect.Height() < m_nButtonHeight)
//	{
//		rcRect.bottom = rcRect.top + m_nButtonHeight;
//	}

	// hide placeholder window
	//::ShowWindow(hWnd, SW_HIDE);

	return CreateFromRect(dwStyle, pParentWnd, rcRect, nId);
}

BOOL CScrollBarEx::CreateFromWindow(DWORD dwStyle, CWnd* pParentWnd, HWND hWnd, UINT nId)
{
	ASSERT(pParentWnd);
	ASSERT(IsWindow(pParentWnd->m_hWnd));
	ASSERT(::IsWindow(hWnd));

	CRect rect;
	::GetWindowRect(hWnd, &rect);
	pParentWnd->ScreenToClient(&rect);

	if (rect.Width() < SIZE_WIDTH)
	{
		rect.right = rect.left + SIZE_WIDTH;
	}

	if (rect.Height() < m_nBitmapHeight)
	{
		rect.bottom = rect.top + m_nBitmapHeight;
	}

	// hide placeholder window
	::ShowWindow(hWnd, SW_HIDE);

	return CreateFromRect(dwStyle, pParentWnd, rect, nId);
}

BOOL CScrollBarEx::CreateFromRect(DWORD dwStyle,
	CWnd* pParentWnd,
	CRect& rect,
	UINT nId)
{
	ASSERT(pParentWnd);
	ASSERT(IsWindow(pParentWnd->m_hWnd));

	m_pParent = pParentWnd;

	m_bHorizontal = (dwStyle & SBS_VERT) ? FALSE : TRUE;

	BOOL bResult = 	CWnd::Create(
		AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, 0, 0, 0),
		_T(""), dwStyle, rect, pParentWnd, nId);

	if (bResult)
	{
		OnInitScrollbar();

//		m_nBitmapWidth = SIZE_WIDTH;
//		m_nBitmapHeight = SIZE_HEIGHT;

		GetClientRect(&m_rcClient);

		m_rcLeftArrow = CRect(m_rcClient.left, m_rcClient.top, m_rcClient.left + SIZE_WIDTH, m_rcClient.bottom);
		m_rcRightArrow = CRect(m_rcClient.right - SIZE_WIDTH, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
		m_rcUpArrow = CRect(m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.top + m_nButtonHeight);
		m_rcDownArrow = CRect(m_rcClient.left, m_rcClient.bottom - m_nButtonHeight, m_rcClient.right, m_rcClient.bottom);

		if (m_bHorizontal)
		{

		}
		else
		{
			m_rcChannel.top = m_rcUpArrow.bottom;
			m_rcChannel.bottom = m_rcDownArrow.top;
			m_rcChannel.left = m_rcClient.left;
			m_rcChannel.right = m_rcClient.right;
		}
	}

	return bResult;
}

void CScrollBarEx::OnInitScrollbar()
{
	CClientDC dc(this);

	OnDeleteBitmaps();
}

BOOL CScrollBarEx::OnEraseBkgnd(CDC* pDC)
{
	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

void CScrollBarEx::ScrollLeft()
{
	if (m_nPos > 0)
		m_nPos--;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_LINELEFT,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

void CScrollBarEx::ScrollRight()
{
	if (m_nPos < m_nRange)
		m_nPos++;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_LINERIGHT,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

void CScrollBarEx::ScrollUp()
{
	if (m_nPos > 0)
		m_nPos--;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

void CScrollBarEx::ScrollDown()
{
	if (m_nPos < m_nRange - m_nPage)
		m_nPos++;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN,0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

void CScrollBarEx::PageUp()
{
	m_nPos = m_nPos - m_nPage;

	if (m_nPos < 0)
		m_nPos = 0;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEUP, 0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}

void CScrollBarEx::PageDown()
{
	m_nPos = m_nPos + m_nPage;

	if (m_nPos > m_nRange - m_nPage)
		m_nPos = m_nRange - m_nPage;
	if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
		m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEDOWN, 0), (LPARAM)m_hWnd);
	UpdateThumbPosition();
}


void CScrollBarEx::SetPositionFromThumb()
{
	double dPixels, dMax, dInterval, dPos;

	LimitThumbPosition();
	dMax = m_nRange;

	if (m_bHorizontal)
	{
		dPixels   = m_rcClient.Width() - 3*SIZE_WIDTH;
		dInterval = dMax / dPixels;
		dPos      = dInterval * (m_nThumbLeft - SIZE_WIDTH);
	}
	else
	{
		dPixels   = m_rcClient.Height() - 2 * m_nButtonHeight;
		dInterval = dMax / dPixels;
//		dInterval = (double)m_rcChannel.Height() / (double)m_nRange;
//		m_nThumbTop = m_rcChannel.top + int(dInterval * m_nPos);
		dPos      = dInterval * (m_nThumbTop - m_nButtonHeight);
	}

	m_nPos = (int) (dPos);
	if (m_nPos < 0)
		m_nPos = 0;
	if (m_nPos > m_nRange - m_nPage)
		m_nPos = m_nRange - m_nPage;
}

void CScrollBarEx::UpdateThumbPosition()
{
	double dPixels, dMax, dInterval, dPos;

	dMax = m_nRange;
	dPos = m_nPos;

	if (m_bHorizontal)
	{
		dPixels   = m_rcClient.Width() - 3*SIZE_WIDTH;
		dInterval = dPixels / dMax;
		double dThumbLeft = dPos * dInterval + 0.5;
		m_nThumbLeft = SIZE_WIDTH + (int)dThumbLeft;
	}
	else
	{
		dInterval = (double)m_rcChannel.Height() / (double)m_nRange;
		m_nThumbTop = int(m_rcChannel.top + dInterval * m_nPos + 0.5);
		m_nThumbHeight = int(dInterval * m_nPage);
	}

//	LimitThumbPosition();

	Invalidate();
}

void CScrollBarEx::LimitThumbPosition()
{
	if (m_bHorizontal)
	{
		//if ((m_nThumbLeft + m_nBitmapWidth*2) > (m_rcClient.Width() - m_nBitmapWidth))
		if ((m_nThumbLeft + SIZE_WIDTH) > (m_rcClient.Width() - SIZE_WIDTH))
			m_nThumbLeft = m_rcClient.Width() - 2*SIZE_WIDTH;
			//m_nThumbLeft = m_rcClient.Width() - 3*m_nBitmapWidth;

		if (m_nThumbLeft < (m_rcClient.left + SIZE_WIDTH))
			m_nThumbLeft = m_rcClient.left + SIZE_WIDTH;
	}
	else
	{
		if ((m_nThumbTop + m_nThumbHeight) > (m_rcClient.Height() - m_nButtonHeight))
			m_nThumbTop = m_rcClient.Height() - m_nButtonHeight - m_nThumbHeight;

		if (m_nThumbTop < (m_rcClient.top + m_nButtonHeight))
			m_nThumbTop = m_rcClient.top + m_nButtonHeight;
	}
}

void CScrollBarEx::SetScrollInfo(int nMinPos, int nMaxPos, int nPage, BOOL bRedraw /*= TRUE*/)
{
	m_nMinPos = nMinPos;
	m_nMaxPos = nMaxPos;

	if (m_nMinPos < m_nMaxPos)
		m_nRange = m_nMaxPos - m_nMinPos + 1;
	else
		m_nRange = m_nMinPos - m_nMaxPos + 1;

	m_nPage = nPage;

	UpdateThumbPosition();

	if (bRedraw)
		Invalidate();
}

int CScrollBarEx::SetScrollPos(int nPos, BOOL bRedraw /*= TRUE*/)
{
	int nOldPos = m_nPos;

	m_nPos = nPos;

	UpdateThumbPosition();

	if (bRedraw)
		Invalidate();

	return nOldPos;
}

void CScrollBarEx::OnPaint()
{
	CPaintDC dc(this);


	if (m_bHorizontal)
	{
		DrawHorizontal(&dc);
	}
	else
	{
		DrawVertical(&dc);
	}

	return;
}

void CScrollBarEx::DrawHorizontal(CDC *pDC)
{
//	//=====  draw left arrow  =====//
//	CRect rectLeft(m_rcLeftArrow);
//	if (m_bMouseDownArrowLeft)
//		rectLeft.OffsetRect(1, 1);
//
//	if (m_bMouseOverArrowLeft)
//		pG->DrawImage(m_pBmpLeftArrowHi,Rect(rectLeft.left, rectLeft.top,rectLeft.Width(), rectLeft.Height()), 0, 0, SIZE_WIDTH, SIZE_HEIGHT, UnitPixel);
//	else
//		pG->DrawImage(m_pBmpLeftArrow,Rect(rectLeft.left, rectLeft.top,rectLeft.Width(), rectLeft.Height()), 0, 0, SIZE_WIDTH, SIZE_HEIGHT, UnitPixel);
//
//	int nChannelStart = m_rcClient.left + SIZE_WIDTH;
//	int nChannelWidth = m_rcClient.Width() - 2*SIZE_WIDTH;
//
//	//=====  draw channel  =====//
//	m_rcThumb.left   = m_rcClient.left + m_nThumbLeft;
//	m_rcThumb.right  = m_rcThumb.left + SIZE_WIDTH;
//	m_rcThumb.top    = m_rcClient.top;
//	m_rcThumb.bottom = m_rcThumb.top + m_rcClient.Height();
//
//	CRect rectChannelRight(m_rcThumb.left + SIZE_WIDTH/2, m_rcClient.top, nChannelStart + nChannelWidth, m_rcClient.bottom);
//	
//	//channel은 source를 1,1로 해야한다. 만약, 다르면 그라데이션 효과가 발생.
//	pG->DrawImage(m_pBmpChannel,Rect(rectChannelRight.left, rectChannelRight.top,rectChannelRight.Width(), rectChannelRight.Height()),0,0,1,1,UnitPixel);
//
//	CRect rectChannelLeft(nChannelStart, m_rcClient.top,m_rcThumb.left + SIZE_WIDTH/2, m_rcClient.bottom);
//
//	pG->DrawImage(m_pBmpChannel,Rect(rectChannelLeft.left, rectChannelLeft.top,rectChannelLeft.Width(), rectChannelLeft.Height()),0,0,1,1,UnitPixel);
//
//	//=====  draw right arrow  =====//
//	CRect rectRight(m_rcRightArrow);
//	
//	if (m_bMouseDownArrowRight)
//		rectRight.OffsetRect(1, 1);
//
//	if (m_bMouseOverArrowRight)
//		pG->DrawImage(m_pBmpRightArrowHi,Rect(rectRight.left, rectRight.top,rectRight.Width(), rectRight.Height()),0,0,SIZE_WIDTH,SIZE_HEIGHT,UnitPixel);
//	else
//		pG->DrawImage(m_pBmpRightArrow,Rect(rectRight.left, rectRight.top,rectRight.Width(), rectRight.Height()),0,0,SIZE_WIDTH,SIZE_HEIGHT,UnitPixel);
//
//	if (m_nRange)
//	{
//		//=====  draw thumb  =====//
//		if (m_bThumbHover)
//			pG->DrawImage(m_pBmpThumbHi,Rect(m_rcThumb.left, m_rcThumb.top,m_rcThumb.Width(), m_rcThumb.Height()),0,0,SIZE_WIDTH,SIZE_HEIGHT,UnitPixel);
//		else
//			pG->DrawImage(m_pBmpThumb,Rect(m_rcThumb.left, m_rcThumb.top,m_rcThumb.Width(), m_rcThumb.Height()),0,0,SIZE_WIDTH,SIZE_HEIGHT,UnitPixel);
//	}
//	else
//	{
//		m_rcThumb = CRect(-1,-1,-1,-1);
//	}
}

void CScrollBarEx::DrawVertical(CDC *pDC)
{
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	BITMAP bmInfo;

	m_rcThumb.left = m_rcChannel.left;
	m_rcThumb.right = m_rcChannel.right;
	m_rcThumb.top = m_nThumbTop;
	m_rcThumb.bottom = m_nThumbTop + m_nThumbHeight;

	//=====  draw channel  =====//
	pDC->FillSolidRect(m_rcChannel, RGB(240, 241, 242));

	//=====  draw thumb  =====//
//	memDC.SelectObject(m_bmpThumb[m_bThumbHover]);
//	m_bmpThumb[m_bThumbHover].GetBitmap(&bmInfo);
//	pDC->StretchBlt(m_rcThumb.left, m_rcThumb.top, m_rcThumb.Width(), m_rcThumb.Height(), &memDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
	pDC->FillSolidRect(m_rcThumb, RGB(214, 215, 218));

	//=====  draw up arrow  =====//
	memDC.SelectObject(m_bmpArrowUp[m_bMouseOverArrowUp]);
	m_bmpArrowUp[m_bMouseOverArrowUp].GetBitmap(&bmInfo);
	pDC->StretchBlt(m_rcUpArrow.left, m_rcUpArrow.top, m_rcUpArrow.Width(), m_rcUpArrow.Height(), &memDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);

	//=====  draw down arrow  =====//
	memDC.SelectObject(m_bmpArrowDown[m_bMouseOverArrowDown]);
	m_bmpArrowDown[m_bMouseOverArrowDown].GetBitmap(&bmInfo);
	pDC->StretchBlt(m_rcDownArrow.left, m_rcDownArrow.top, m_rcDownArrow.Width(), m_rcDownArrow.Height(), &memDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);

	return;
}

void CScrollBarEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nRange < m_nPage)
	{
		return;
	}

	KillTimer(TIMER_MOUSE_OVER_BUTTON);
	KillTimer(TIMER_LBUTTON_PRESSED);
	SetCapture();
	SetFocus();

	if (m_bHorizontal)
	{
		m_bMouseDownArrowLeft = FALSE;
		m_bMouseDownArrowRight = FALSE;
		
		CRect rectThumb(m_nThumbLeft, 0, m_nThumbLeft + SIZE_WIDTH,m_rcClient.Height());

		if (rectThumb.PtInRect(point))
		{
			m_bMouseDown = TRUE;
		}
		else if (m_rcRightArrow.PtInRect(point))
		{
			m_bMouseDownArrowRight = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 200, NULL);
		}
		else if (m_rcLeftArrow.PtInRect(point))
		{
			m_bMouseDownArrowLeft = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 200, NULL);
		}
		else	// button down in channel
		{
			m_nThumbLeft = point.x - (SIZE_WIDTH) / 2;
			SetPositionFromThumb();

			if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
				m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos),(LPARAM)m_hWnd);
		}
		Invalidate();
	}
	else
	{
		CRect rectThumb(0, m_nThumbTop, m_rcClient.Width(), m_nThumbTop + m_nThumbHeight);

		if (rectThumb.PtInRect(point))
		{
			m_bMouseDown = TRUE;
		}
		else if (m_rcDownArrow.PtInRect(point))
		{
			m_bMouseDownArrowDown = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 150, NULL);
		}
		else if (m_rcUpArrow.PtInRect(point))
		{
			m_bMouseDownArrowUp = TRUE;
			SetTimer(TIMER_LBUTTON_PRESSED, 150, NULL);
		}
		else	// button down in channel
		{
			if (point.y < m_rcThumb.top)
			{
				PageUp();
			}
			if (point.y > m_rcThumb.bottom)
			{
				PageDown();
			}
		}
		Invalidate();
	}
	CWnd::OnLButtonDown(nFlags, point);
}


void CScrollBarEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nRange < m_nPage)
	{
		return;
	}

	UpdateThumbPosition();
	KillTimer(TIMER_MOUSE_OVER_BUTTON);
	KillTimer(TIMER_LBUTTON_PRESSED);
	ReleaseCapture();

	if (m_bHorizontal)
	{
		CRect rectThumb(m_nThumbLeft, 0, m_nThumbLeft + SIZE_WIDTH, m_rcClient.Height());

		m_bMouseDownArrowLeft = FALSE;
		m_bMouseDownArrowRight = FALSE;

		if (m_rcLeftArrow.PtInRect(point))
		{
			ScrollLeft();
		}
		else if (m_rcRightArrow.PtInRect(point))
		{
			ScrollRight();
		}
		else if (rectThumb.PtInRect(point))
		{
			m_bThumbHover = TRUE;
			SetTimer(TIMER_MOUSE_OVER_THUMB, 50, NULL);
		}
		Invalidate();
	}
	else
	{
		CRect rectThumb(0, m_nThumbTop, m_rcClient.Width(), m_nThumbTop + m_nThumbHeight);

		m_bMouseDownArrowUp = FALSE;
		m_bMouseDownArrowDown = FALSE;

		if (m_rcUpArrow.PtInRect(point))
		{
			ScrollUp();
		}
		else if (m_rcDownArrow.PtInRect(point))
		{
			ScrollDown();
		}
		else if (rectThumb.PtInRect(point))
		{
			m_bThumbHover = TRUE;
			SetTimer(TIMER_MOUSE_OVER_THUMB, 50, NULL);
		}

		Invalidate();
	}

	m_bMouseDown = FALSE;
	m_bDragging = FALSE;

	CWnd::OnLButtonUp(nFlags, point);
}

void CScrollBarEx::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_nRange < m_nPage)
	{
		return;
	}

	BOOL bOldThumbHover = m_bThumbHover;
	m_bThumbHover = FALSE;

	if (m_rcThumb.PtInRect(point))
		m_bThumbHover = TRUE;

	if (m_bMouseDown)
		m_bDragging = TRUE;

	BOOL bOldHover = m_bMouseOverArrowRight |
		m_bMouseOverArrowLeft  |
		m_bMouseOverArrowUp    |
		m_bMouseOverArrowDown;

	m_bMouseOverArrowRight = FALSE;
	m_bMouseOverArrowLeft  = FALSE;
	m_bMouseOverArrowUp    = FALSE;
	m_bMouseOverArrowDown  = FALSE;

	if (m_bHorizontal)
	{
		if (m_rcLeftArrow.PtInRect(point))
			m_bMouseOverArrowLeft = TRUE;
		else if (m_rcRightArrow.PtInRect(point))
			m_bMouseOverArrowRight = TRUE;
	}
	else
	{
		if (m_rcUpArrow.PtInRect(point))
			m_bMouseOverArrowUp = TRUE;
		else if (m_rcDownArrow.PtInRect(point))
			m_bMouseOverArrowDown = TRUE;
	}

	BOOL bNewHover = m_bMouseOverArrowRight |
		m_bMouseOverArrowLeft  |
		m_bMouseOverArrowUp    |
		m_bMouseOverArrowDown;

	if (bNewHover)
		SetTimer(TIMER_MOUSE_HOVER_BUTTON, 80, NULL);

	if (bOldHover != bNewHover)
		Invalidate();

	if (m_bDragging)
	{
		if (m_bHorizontal)
		{
			m_nThumbLeft = point.x - (SIZE_WIDTH) / 2;

			SetPositionFromThumb();

			if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
				m_pParent->SendMessage(WM_HSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos), (LPARAM)m_hWnd);
		}
		else
		{
			m_nThumbTop = point.y - (m_nThumbHeight / 2);
			SetPositionFromThumb();
			if (m_pParent && ::IsWindow(m_pParent->m_hWnd))
			{
				m_pParent->SendMessage(WM_VSCROLL, MAKELONG(SB_THUMBTRACK, m_nPos), (LPARAM)m_hWnd);
			}
		}

		Invalidate();
	}

	if (bOldThumbHover != m_bThumbHover)
	{
		Invalidate();
		SetTimer(TIMER_MOUSE_OVER_THUMB, 50, NULL);
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CScrollBarEx::OnTimer(UINT_PTR nIDEvent)
{
	CPoint point;
	::GetCursorPos(&point);
	ScreenToClient(&point);

	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (nIDEvent == TIMER_MOUSE_HOVER_BUTTON)
	{
		m_bMouseOverArrowRight = FALSE;
		m_bMouseOverArrowLeft  = FALSE;
		m_bMouseOverArrowUp    = FALSE;
		m_bMouseOverArrowDown  = FALSE;

		if (m_bHorizontal)
		{
			if (m_rcLeftArrow.PtInRect(point))
				m_bMouseOverArrowLeft = TRUE;
			else if (m_rcRightArrow.PtInRect(point))
				m_bMouseOverArrowRight = TRUE;
		}
		else
		{
			if (m_rcUpArrow.PtInRect(point))
				m_bMouseOverArrowUp = TRUE;
			else if (m_rcDownArrow.PtInRect(point))
				m_bMouseOverArrowDown = TRUE;
		}

		if (!m_bMouseOverArrowLeft  &&
			!m_bMouseOverArrowRight && 
			!m_bMouseOverArrowUp    &&
			!m_bMouseOverArrowDown)
		{
			KillTimer(nIDEvent);
			Invalidate();
		}
	}
	else if (nIDEvent == TIMER_MOUSE_OVER_BUTTON)	// mouse is in an arrow button, and left button is down
	{
		if (m_bMouseDownArrowLeft)
			ScrollLeft();
		if (m_bMouseDownArrowRight)
			ScrollRight();
		if (m_bMouseDownArrowUp)
			ScrollUp();
		if (m_bMouseDownArrowDown)
			ScrollDown();

		if (!rect.PtInRect(point))
		{
			m_bMouseDownArrowLeft  = FALSE;
			m_bMouseDownArrowRight = FALSE;
			m_bMouseDownArrowUp    = FALSE;
			m_bMouseDownArrowDown  = FALSE;
		}
		if (!m_bMouseDownArrowLeft  &&
			!m_bMouseDownArrowRight && 
			!m_bMouseDownArrowUp    &&
			!m_bMouseDownArrowDown)
		{
			KillTimer(nIDEvent);
			Invalidate();
		}
	}
	else if (nIDEvent == TIMER_LBUTTON_PRESSED)	// mouse is in an arrow button, and left button has just been pressed
	{
		KillTimer(nIDEvent);

		if (m_bMouseDownArrowLeft  || 
			m_bMouseDownArrowRight || 
			m_bMouseDownArrowUp    || 
			m_bMouseDownArrowDown)
		{
			// debounce left click
			SetTimer(TIMER_MOUSE_OVER_BUTTON, MOUSE_OVER_BUTTON_TIME, NULL);
			Invalidate();
		}
	}
	else if (nIDEvent == TIMER_MOUSE_OVER_THUMB)	// mouse is over thumb
	{
		if (!m_rcThumb.PtInRect(point))
		{
			// no longer over thumb, restore thumb color
			m_bThumbHover = FALSE;
			KillTimer(nIDEvent);
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			Invalidate();
		}
	}

	CWnd::OnTimer(nIDEvent);
}
