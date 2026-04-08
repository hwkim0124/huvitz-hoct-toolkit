#pragma once
#include "GuiCtrlDef.h"

//----------------------------------------------------------
// ScrollbarEx Control
//----------------------------------------------------------
// Programmed by William Kim
//----------------------------------------------------------

#define TIMER_MOUSE_OVER_BUTTON		1	// mouse is over an arrow button, and left button is down
#define TIMER_LBUTTON_PRESSED		2	// mouse is over an arrow button, and left button has just been pressed
#define TIMER_MOUSE_OVER_THUMB		3	// mouse is over thumb
#define TIMER_MOUSE_HOVER_BUTTON	4	// mouse is over an arrow button
#define MOUSE_OVER_BUTTON_TIME		50

#define SIZE_WIDTH  50
//#define SIZE_HEIGHT 40
// CScrollBarEx

class GUICTRL_DLL_API  CScrollBarEx : public CScrollBar
{
	DECLARE_DYNAMIC(CScrollBarEx)
public:
	CScrollBarEx();
	virtual ~CScrollBarEx();

public :
	//----- Dynamic Create Control -----//
	BOOL CreateContol(CWnd* pWnd, bool bHorizontal, CRect rcSize, UINT ID);

	//----- Init Control -----//
	BOOL InitControl(CWnd* pWnd, bool bHorizontal = true);

	//----- Set Enable & Disable Control -----//
	void SetEnable(bool bEnable) { m_bEnable = bEnable; EnableWindow(bEnable); Invalidate(); }

	//----- Set Bitmap -----//
	void SetBitmapThumb(UINT nNormal, UINT nActive);
	void SetBitmapArrowUp(UINT nNormal, UINT nActive);
	void SetBitmapArrowDown(UINT nNormal, UINT nActive);
	void SetBitmapArrowLeft(UINT nNormal, UINT nActive);
	void SetBitmapArrowRight(UINT nNormal, UINT nActive);

	//----- Pos & Range -----//
	int	GetScrollPos() const { return m_nPos; }
	int	SetScrollPos(int nPos, BOOL bRedraw = TRUE);
	void SetScrollInfo(int nMinPos, int nMaxPos, int nPage, BOOL bRedraw = TRUE);


private :
	CWnd *	m_pParent;				// control parent 
	BOOL	m_bThumbHover;			// TRUE = mouse is over thumb
	BOOL	m_bMouseOverArrowRight;	// TRUE = mouse over right arrow
	BOOL	m_bMouseOverArrowLeft;	// TRUE = mouse over left arrow
	BOOL	m_bMouseOverArrowUp;	// TRUE = mouse over up arrow
	BOOL	m_bMouseOverArrowDown;	// TRUE = mouse over down arrow
	BOOL	m_bMouseDownArrowRight;	// TRUE = mouse over right arrow & left button down
	BOOL	m_bMouseDownArrowLeft;	// TRUE = mouse over left arrow & left button down
	BOOL	m_bMouseDownArrowUp;	// TRUE = mouse over up arrow & left button down
	BOOL	m_bMouseDownArrowDown;	// TRUE = mouse over down arrow & left button down
	BOOL	m_bMouseDown;			// TRUE = mouse over thumb & left button down
	BOOL	m_bDragging;			// TRUE = thumb is being dragged
	BOOL	m_bHorizontal;			// TRUE = horizontal scroll bar
	bool	m_bEnable;
	
	int		m_nThumbLeft;			// left margin of thumb
	int		m_nThumbTop;			// top margin of thumb
	int		m_nBitmapHeight;		// height of arrow and thumb bitmaps
	int		m_nBitmapWidth;			// width of arrow and thumb bitmaps
	int		m_nPos;					// current thumb position in scroll units
	int		m_nMinPos;				// minimum scrolling position
	int		m_nMaxPos;				// maximum scrolling position
	int		m_nRange;				// absolute value of max - min pos
	int		m_nPage;		

	int		m_nButtonWidth;
	int		m_nButtonHeight;
	int		m_nThumbWidth;
	int		m_nThumbHeight;


	CRect	m_rcThumb;				// current rect for thumb
	CRect	m_rcClient;				// control client rect
	CRect	m_rcChannel;
	CRect	m_rcLeftArrow;			// left arrow rect
	CRect	m_rcRightArrow;			// right arrow rect
	CRect	m_rcUpArrow;			// up arrow rect
	CRect	m_rcDownArrow;			// down arrow rect

	CBitmap m_bmpThumb[2];			// Thumb 이미지.	
	CBitmap m_bmpChannel[2];		// 막대 바 이미지.
	CBitmap m_bmpArrowUp[2];			// 위 화살표 이미지.
	CBitmap m_bmpArrowDown[2];		// 아래 화살표 이미지.
	CBitmap m_bmpArrowLeft[2];		// 왼쪽 화살표 이미지.
	CBitmap m_bmpArrowRight[2];		// 오른쪽 화살표 이미지.
	
	BOOL CreateFromWindow(DWORD dwStyle,CWnd* pParentWnd,HWND hStatic,UINT nId);
	BOOL CreateFromWindow(DWORD dwStyle,CWnd* pParentWnd,CRect rcRect,UINT nId);
	BOOL CreateFromRect(DWORD dwStyle,CWnd* pParentWnd,CRect& rect,UINT nId);
	
	void OnInitScrollbar();			// 스크롤바를 구성하는 이미지들을 불러오는 함수. (또는 그리는 부분.)
	void LimitThumbPosition();		// 스크롤바의 동작 범위 제한 설정.
	void SetPositionFromThumb();	// Thumb 위치 지정 계산.		
	void UpdateThumbPosition();		// Thumb 위치 지정 계산.
	void ScrollLeft();
	void ScrollRight();
	void ScrollUp();
	void ScrollDown();
	void PageUp();
	void PageDown();
	
//	void Draw(Graphics* pG);
	void DrawHorizontal(CDC *pDC);
	void DrawVertical(CDC *pDC);

	void OnDeleteBitmaps();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


