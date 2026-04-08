#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"


// ImageButton.h : header file
//

class GUICTRL_DLL_API ImageButton : public CButton
{
public:
	ImageButton();
	virtual ~ImageButton();

	void	RefreshImage(void);
	bool	LoadAltImage(UINT id, LPCTSTR pType = _T("PNG"));
	bool	LoadStdImage(UINT id, LPCTSTR pType = _T("PNG"));
	bool	LoadPusImage(UINT id, LPCTSTR pType = _T("PNG"));

	void	EnableToggle(bool enable = TRUE);
	void	SetToggle(bool toggle);
	bool	GetToggle(void);
	
	void	ShowWindowText(bool bShow = TRUE);
	void	SetTextColor(COLORREF clrStd, COLORREF clrPus, COLORREF clrAlt);
	void	SetDrawTextParams(UINT nFormat, int leftMargin = 0, int rightMargin = 0);


protected:
	bool	m_enableToggle;
	bool	m_isToggle;

	GdiPlusBitmapResource m_pStdImage;
	GdiPlusBitmapResource m_pPusImage;
	GdiPlusBitmapResource m_pAltImage;

	COLORREF m_clrStdText;
	COLORREF m_clrPusText;
	COLORREF m_clrAltText;

	BOOL	m_bShowText;
	
	UINT	m_nFormat;
	int		m_iLeftMargin;
	UINT	m_iRightMargin;
	
	CDC		m_dcStd;
	CDC		m_dcAlt;
	CDC		m_dcPus;


	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
