// RangeSlider.cpp : implementation file
//
/* Written By and (C) 2002 Jens Scheidtmann

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:


  1. Redistributions of source code must retain the above copyright notice, this list 
     of conditions and the following disclaimer. 

  2. Redistributions in binary form must reproduce the above copyright notice, this list 
     of conditions and the following disclaimer in the documentation and/or other 
     materials provided with the distribution. 

  3. The name of the author may not be used to endorse or promote products derived 
     from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



  alternatively (at your option) you can apply the Gnu Public License V 2 or above (GPL).

*/

#include "stdafx.h"
#include "GuiCtrl.h"
#include "RangeSlider.h"
#include "MemDC.h"

#include <math.h> // for fabs

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RANGESLIDER_CLASSNAME _T("MFCRangeSlider")

const UINT RANGE_CHANGED = ::RegisterWindowMessage(_T("RANGE_CHANGED{2E7439A4-12DB-42a2-BB95-3462B3EC39CE}CRangeSlider"));


/////////////////////////////////////////////////////////////////////////////
// CRangeSlider

CRangeSlider::CRangeSlider()
{
	RegisterWindowClass();
	m_Min = 0.0;
	m_Max = 1.0;
	m_Left = 0.0;
	m_Right = 1.0;
	m_bHorizontal = TRUE;
	m_nThumbWidth = 20;
	m_bTracking = FALSE;
	m_bInvertedMode = FALSE;
}

CRangeSlider::~CRangeSlider()
{
	m_bmpThumbN.DeleteObject();
	m_bmpThumbS.DeleteObject();
	m_brushChannelN.DeleteObject();
	m_brushChannelS.DeleteObject();
}

// Relay call to create
void CRangeSlider::Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext) 
{
	CWnd::Create(NULL, NULL, dwStyle, rect, pParentWnd, nID, pContext);
}


// Register the window class, so you can use this class as a custom control.
BOOL CRangeSlider::RegisterWindowClass(void) 
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, RANGESLIDER_CLASSNAME, &wndcls)))
	{
		// otherwise we need to register a new class
		wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc      = ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground    = NULL; // No Background brush (gives flicker)
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = RANGESLIDER_CLASSNAME;

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}


void CRangeSlider::SetBitmapChannel(UINT nNormal, UINT nActive)
{
	CBitmap bmpN, bmpS;

	bmpN.LoadBitmap(nNormal);
	bmpS.LoadBitmap(nActive);

	m_brushChannelN.CreatePatternBrush(&bmpN);
	m_brushChannelS.CreatePatternBrush(&bmpS);

	bmpN.DeleteObject();
	bmpS.DeleteObject();

	return;
}

void CRangeSlider::SetBitmapThumb(UINT nNormal, UINT nActive)
{
	m_bmpThumbN.LoadBitmap(nNormal);
	m_bmpThumbS.LoadBitmap(nActive);

	return;
}

BEGIN_MESSAGE_MAP(CRangeSlider, CWnd)
	//{{AFX_MSG_MAP(CRangeSlider)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CRangeSlider message handlers
void CRangeSlider::OnPaint() 
{
	CPaintDC dc(this);

	if (m_bHorizontal) {
		OnPaintHorizontal(dc);
	} else { // Vertical Mode
		OnPaintVertical(dc);
	}
}

// Paint in Horizontal Mode.
void CRangeSlider::OnPaintHorizontal(CDC &dc) 
{
	ASSERT(m_bHorizontal);

	// Calculate thumb size and x-axis width.
	CRect rectClient;
	GetClientRect(&rectClient);

	m_nThumbWidth = rectClient.Height();
	m_dx = rectClient.Width() - 2 * m_nThumbWidth;


	// draw channel
	dc.FillRect(&rectClient, &m_brushChannelN);


	// Check if [Left, Right] is contained in [Min, Max].
	ASSERT(m_Min <= m_Left);
	ASSERT(m_Left <= m_Right);
	ASSERT(m_Right <= m_Max);

	// Position of Left and Right
	int x1 = static_cast<int>((m_Left - m_Min)/(m_Max - m_Min) * m_dx + 0.5);
	int x2 = static_cast<int>((m_Right - m_Min)/(m_Max - m_Min) * m_dx + 0.5);


	CDC memDC;
	memDC.CreateCompatibleDC(&dc);


	// Draw Left Thumb
	m_rectLeft = CRect (CPoint(x1,0), CSize(m_nThumbWidth, m_nThumbWidth));

	if (m_bTracking && (m_TrackMode == TRACK_LEFT || m_TrackMode == TRACK_MIDDLE))
	{
		memDC.SelectObject(&m_bmpThumbS);
	}
	else
	{
		memDC.SelectObject(&m_bmpThumbN);
	}

	dc.BitBlt(m_rectLeft.left, m_rectLeft.top, m_rectLeft.Width(), m_rectLeft.Height(), &memDC, 0, 0, SRCCOPY);

	
	// Draw Right Thumb
	m_rectRight = CRect(CPoint(m_nThumbWidth + x2, 0), CSize(m_nThumbWidth, m_nThumbWidth));

	if (m_bTracking && (m_TrackMode == TRACK_RIGHT || m_TrackMode == TRACK_MIDDLE))
	{
		memDC.SelectObject(&m_bmpThumbS);
	}
	else
	{
		memDC.SelectObject(&m_bmpThumbN);
	}

	dc.BitBlt(m_rectRight.left, m_rectRight.top, m_rectRight.Width(), m_rectRight.Height(), &memDC, 0, 0, SRCCOPY);



	// Draw Area in between.
	CRect rectMiddle;
	rectMiddle = CRect(CPoint(m_nThumbWidth + x1 + 1, 1), CPoint(m_nThumbWidth + x2, m_nThumbWidth - 1));
	dc.FillRect(&rectMiddle, &m_brushChannelS);



	return;
}

// Paint in Vertical Mode.
void CRangeSlider::OnPaintVertical(CDC &dc) {
	ASSERT(!m_bHorizontal);
	CRect ClientRect;
	GetClientRect(&ClientRect);

	int dy = ClientRect.Width();
//	m_nThumbWidth = dy * 3 / 4;

	m_dx = ClientRect.Height() - 2 * m_nThumbWidth;

	CRect ClipBox;
	dc.GetClipBox(&ClipBox);

		// Erase Background.
	CGdiObject *pBr  = dc.SelectStockObject(GRAY_BRUSH);
	CGdiObject *pPen = dc.SelectStockObject(BLACK_PEN);
	dc.Rectangle(&ClipBox);

		// Check if [Left, Right] is contained in [Min, Max].
	ASSERT(m_Min <= m_Left);
	ASSERT(m_Left <= m_Right);
	ASSERT(m_Right <= m_Max);

		// Position of Left and Right
	int x1 = static_cast<int>((m_Left - m_Min)/(m_Max - m_Min) * m_dx + 0.5);
	int x2 = static_cast<int>((m_Right - m_Min)/(m_Max - m_Min) * m_dx + 0.5);
	dc.SelectStockObject(BLACK_PEN);
	CBrush ButtonFace;
	ButtonFace.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	dc.SelectObject(ButtonFace);
	// Draw Left Arrow
	m_rectLeft = CRect (CPoint(0, x1), CSize(dy, m_nThumbWidth));
	{ 
		CRect & rect = m_rectLeft;
		dc.Rectangle(&rect);
			// Make appearance 3D
		if (!m_bTracking || m_TrackMode != TRACK_LEFT) {
			rect.DeflateRect(1,1,2,2);
			CPen pen1;
			pen1.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNHIGHLIGHT));
			dc.SelectObject(pen1);
			dc.MoveTo(rect.left, rect.bottom);
			dc.LineTo(rect.left, rect.top);
			dc.LineTo(rect.right, rect.top);
			CPen pen2;
			pen2.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
			dc.SelectObject(pen2);
			dc.LineTo(rect.right, rect.bottom);
			dc.LineTo(rect.left, rect.bottom);
		}

			// Draw Arrow
		dc.SelectStockObject(BLACK_PEN);
		dc.SelectStockObject(BLACK_BRUSH);
		CPoint poly [] = { 
			CPoint(dy / 3, x1 + m_nThumbWidth / 3), 
			CPoint(dy*2/3, x1 + m_nThumbWidth / 3),
			CPoint(dy / 2, x1 + m_nThumbWidth*2/3),
		};
		dc.Polygon(poly, 3);
	}

	// Draw Right Arrow
	m_rectRight = CRect(CPoint(0,m_nThumbWidth + x2), CSize(dy, m_nThumbWidth));
	{
		CRect & rect = m_rectRight;
		dc.SelectObject(ButtonFace);
		dc.Rectangle(&rect);

			// Make appearance 3D
		if (!m_bTracking || m_TrackMode != TRACK_RIGHT) {
			rect.DeflateRect(1,1,2,2);
			CPen pen1;
			pen1.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNHIGHLIGHT));
			dc.SelectObject(pen1);
			dc.MoveTo(rect.left, rect.bottom);
			dc.LineTo(rect.left, rect.top);
			dc.LineTo(rect.right, rect.top);
			CPen pen2;
			pen2.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_BTNSHADOW));
			dc.SelectObject(pen2);
			dc.LineTo(rect.right, rect.bottom);
			dc.LineTo(rect.left, rect.bottom);
		}

			// Draw Arrow
		dc.SelectStockObject(BLACK_PEN);
		dc.SelectStockObject(BLACK_BRUSH);
		CPoint poly [] = { 
			CPoint(dy / 3, m_nThumbWidth + x2 + m_nThumbWidth * 2 / 3),
			CPoint(dy*2/3, m_nThumbWidth + x2 + m_nThumbWidth * 2 / 3),
			CPoint(dy / 2, m_nThumbWidth + x2 + m_nThumbWidth / 3),
		};
		dc.Polygon(poly, 3);
	}

	dc.SelectStockObject(WHITE_BRUSH);
	// Draw Area in between.
	CRect rect = CRect(CPoint(1, m_nThumbWidth + x1 + 1), CPoint (dy - 1, m_nThumbWidth + x2));

	dc.SelectStockObject(NULL_PEN);
	dc.SelectStockObject(WHITE_BRUSH);
	dc.Rectangle(&rect);

	// Store value for drawing the focus rectangle
	CRect focusRect = rect;

	// Draw Area of InnerMin, Max.
	/*
	if (m_bVisualMinMax) {
		ASSERT(m_Min <= m_VisualMax);
		ASSERT(m_VisualMin <= m_VisualMax);
		ASSERT(m_VisualMax <= m_Max);

			// See wether [InnerMin, InnerMax] geschnitten [Left, Right] is non-empty.
			// Then Draw
		if (m_VisualMin < m_Right && m_VisualMax > m_Left) {
			int visualMin = static_cast<int>((m_VisualMin - m_Min)/(m_Max - m_Min) * m_dx + 0.5);
			int visualMax = static_cast<int>((m_VisualMax - m_Min)/(m_Max - m_Min) * m_dx + 0.5);

				// Stay in [m_Left, m_Right] intervall.

			visualMin = max(visualMin + m_nThumbWidth, x1 + m_nThumbWidth);
			visualMax = min(visualMax + m_nThumbWidth, x2 + m_nThumbWidth);

			rect = CRect(dy - 1, visualMin, 1, visualMax);
			CBrush br;
			br.CreateSolidBrush(RGB(255,255,150)); // light yellow;
			dc.SelectObject(br);
			dc.SelectStockObject(NULL_PEN);
			dc.Rectangle(&rect);
			dc.SelectStockObject(HOLLOW_BRUSH);
		}
	}
	*/
	if (GetFocus() == this) {
		focusRect.DeflateRect(3,3);
		dc.DrawFocusRect(&focusRect);
	}
	dc.SelectObject(pBr);
	dc.SelectObject(pPen);
}


void CRangeSlider::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();
	Invalidate(FALSE);

	if (!m_bTracking) {
		// Hit Testing into Rects.
		// Left, Middle or Right?
		CRect rect;
		GetClientRect(&rect);
		CRect middleRect (0,0,0,0);
		if (m_bHorizontal)
			middleRect = CRect(m_rectLeft.right + 1, 0, m_rectRight.left - 1, rect.bottom);
		else 
			middleRect = CRect(0, m_rectLeft.bottom + 1, m_rectLeft.right, m_rectRight.top - 1);

		if (m_rectLeft.PtInRect(point)) {
			m_bTracking = TRUE;
			m_TrackMode = TRACK_LEFT;
			m_ClickOffset = point - m_rectLeft.CenterPoint();
		} else if (m_rectRight.PtInRect(point)) {
			m_bTracking = TRUE;
			m_TrackMode = TRACK_RIGHT;
			m_ClickOffset = point - m_rectRight.CenterPoint();
		} else if (middleRect.PtInRect(point)) {
			m_bTracking = TRUE;
			m_TrackMode = TRACK_MIDDLE;
			m_ClickOffset = point - middleRect.CenterPoint();
		}
		if (m_bTracking)
			SetCapture();
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void CRangeSlider::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bTracking) {
		int x = 0;
		if (m_bHorizontal)
			x = point.x - m_ClickOffset.x;
		else
			x = point.y - m_ClickOffset.y;

		CRect rect;
		WPARAM changed = 0;
		switch (m_TrackMode) {
		case TRACK_LEFT: {
			double oldLeft = m_Left;
			m_Left = static_cast<double>(x - m_nThumbWidth / 2) / m_dx * (m_Max - m_Min) + m_Min; 
			if (m_Left >= m_Right)
				m_Left = m_Right;
			if (m_Left <= m_Min)
				m_Left = m_Min;

			if (oldLeft != m_Left)
				changed = RS_LEFTCHANGED;
			rect = m_rectLeft;
		} break;
		case TRACK_RIGHT: {
			double oldRight = m_Right;
			m_Right = static_cast<double>(x - m_nThumbWidth * 3 / 2) / m_dx * (m_Max - m_Min) + m_Min;
			if (m_Right <= m_Left)
				m_Right = m_Left;
			if (m_Right >= m_Max)
				m_Right = m_Max;
			if (oldRight != m_Right)
				changed = RS_RIGHTCHANGED;
			rect = m_rectRight;
		} break;
		case TRACK_MIDDLE:
			{
				double delta = m_Right - m_Left;
				ASSERT(delta >= 0.0);
				m_Left  = static_cast<double>(x - m_nThumbWidth) / m_dx * (m_Max - m_Min) + m_Min - delta/2.0;
				m_Right = static_cast<double>(x - m_nThumbWidth) / m_dx * (m_Max - m_Min) + m_Min + delta/2.0; 
				if (m_Left <= m_Min) {
					m_Left = m_Min;
					m_Right = m_Left + delta;
				}
				if (m_Right >= m_Max) {
					m_Right = m_Max;
					m_Left = m_Right - delta;
				}
				changed = RS_BOTHCHANGED;
			}
			break;
		default:
			TRACE("Unknown Track Mode\n");
			ASSERT(FALSE);
			break;
		}
		::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, changed, static_cast<LPARAM>(0));
		Invalidate(FALSE);
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CRangeSlider::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTracking) {
		m_bTracking = FALSE;
		::ReleaseCapture();
		Invalidate(FALSE);
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CRangeSlider::SetMinMax(double min, double max) {
	if (min > max) {
		double x = max;
		max = min;
		min = x;
	}

	m_Min = (m_bInvertedMode) ? -max : min;
	m_Max = (m_bInvertedMode) ? -min : max;
	Normalize();
	Invalidate();
}

void CRangeSlider::SetRange(double left, double right) {
	m_Left = (m_bInvertedMode) ? -right : left;
	m_Right = (m_bInvertedMode) ? -left : right;
	Normalize();
	Invalidate();
}

void CRangeSlider::NormalizeVisualMinMax(void) {
		// Exchange if reversed.
	if (m_VisualMax < m_VisualMin) {
		double dummy = m_VisualMin;
		m_VisualMin = m_VisualMax;
		m_VisualMax = dummy;
	}
	if (m_VisualMin < m_Min) {
		m_VisualMin = m_Min;
		if (m_VisualMax < m_VisualMin) 
			m_VisualMax = m_VisualMin;
	}
	if (m_VisualMax > m_Max) {
		m_VisualMax = m_Max;
		if (m_VisualMin > m_VisualMax)
			m_VisualMin = m_VisualMax;
	}
}

void CRangeSlider::Normalize(void) {
	if (m_Left < m_Min) {
		m_Left = m_Min;
		::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, RS_LEFTCHANGED, 0);
		if (m_Right < m_Left) {
			m_Right = m_Left;
			::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, RS_RIGHTCHANGED, 0);
		}
	}
	if (m_Right > m_Max) {
		m_Right = m_Max;
		::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, RS_RIGHTCHANGED, 0);
		if (m_Left > m_Right) {
			m_Left = m_Right;
			::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, RS_LEFTCHANGED, 0);
		}
	}
}

void CRangeSlider::SetVisualMinMax(double VisualMin, double VisualMax) {
	m_VisualMin = (m_bInvertedMode) ? -VisualMax : VisualMin;
	m_VisualMax = (m_bInvertedMode) ? -VisualMin : VisualMax;
	NormalizeVisualMinMax();
	Invalidate();
}


UINT CRangeSlider::OnGetDlgCode() 
{
	return DLGC_WANTARROWS;
}

void CRangeSlider::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000)!= 0; // T if Ctrl is pressed.

	UINT Key1, Key2;
	if (m_bHorizontal) {
		Key1 = VK_RIGHT; Key2 = VK_LEFT;
	} else {
		Key1 = VK_DOWN; Key2 = VK_UP;
	}
	
	if ((nChar == Key1 || nChar == Key2) && !bCtrl) {
		double dx = (m_Max - m_Min) / m_dx;
		if (dx != 0.0 ) {
			int left = static_cast<int>((m_Left - m_Min) / dx + 0.5);
			int right = static_cast<int>((m_Right - m_Min) / dx + 0.5);
			BOOL bShift = ::GetKeyState(VK_SHIFT) < 0; // T if Shift is pressed.
			if (nChar == Key2) {
				if (!bShift) // Shift not pressed => move intervall
					left--;
				right--; 
			} else {
				if (!bShift) // Shift not pressed => move intervall
					left++;
				right++;
			}
			if (left >= 0 && right <= m_dx) {
				m_Left = m_Min + left * dx;
				m_Right = m_Min + right * dx;
			}
			if (!bShift)
				::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, RS_BOTHCHANGED, 0);
			else 
				::SendMessage(GetParent()->GetSafeHwnd(), RANGE_CHANGED, RS_LEFTCHANGED, 0);
			Invalidate();
		}
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CRangeSlider::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	Invalidate();
}

void CRangeSlider::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	Invalidate();	
}

void CRangeSlider::SetVerticalMode(BOOL bVerticalMode) {
	if (m_bHorizontal != bVerticalMode) 
		return; 

	m_bHorizontal = !bVerticalMode;
	Invalidate();
}

void CRangeSlider::SetInvertedMode(BOOL bInvertedMode) {
	if (m_bInvertedMode == bInvertedMode)
		return;

	m_bInvertedMode = bInvertedMode;

	double dummy = m_Left;
	m_Left = - m_Right;
	m_Right = -dummy;

	dummy = m_Min;
	m_Min = - m_Max;
	m_Max = - dummy;

	dummy = m_VisualMin;
	m_VisualMin = - m_VisualMax;
	m_VisualMax = - dummy;

	Invalidate();
}
