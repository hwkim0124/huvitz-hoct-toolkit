#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// FocusBar

class GUICTRL_DLL_API FocusBar : public CSliderCtrl
{
// Construction
public:
	FocusBar();
	bool m_bRedraw;
// Attributes
public:

// Operations
public:
	void SetThumbColors(COLORREF face, COLORREF highlight);
	BOOL LoadTrackImage(UINT id, LPCTSTR type = _T("PNG"));
	BOOL LoadThumbImage(UINT id, LPCTSTR type = _T("PNG"));

private:
	void DrawTrack(CDC *pDC, LPNMCUSTOMDRAW lpcd);
	void DrawThumb(CDC *pDC, LPNMCUSTOMDRAW lpcd);

private:
	GdiPlusBitmapResource m_imageTrack;
	GdiPlusBitmapResource m_imageThumb;

	HDC			m_dcBk;
	HBITMAP		m_bmpBk;
	HBITMAP     m_bmpBkOld;
	COLORREF	m_crThumbColor[2];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FocusBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~FocusBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(FocusBar)
		afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
};

