#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// ToggleButton

class GUICTRL_DLL_API ToggleButton : public CButton
{
	DECLARE_DYNAMIC(ToggleButton)

public:
	ToggleButton();
	virtual ~ToggleButton();

	void	loadImages(UINT idNormal, UINT idToggle, UINT idDisable = 0, LPCTSTR pType = _T("PNG"));
	void	loadImages(UINT idNormal, UINT idNormalPush, UINT idToggle, UINT idTogglePush, UINT idDisable = 0, LPCTSTR pType = _T("PNG"));
	void	showText(bool show);
	void	setTextColor(COLORREF clrNormal, COLORREF clrToggle);
	void	setDrawTextParams(UINT nFormat, int leftMargin, int rightMargin);
	void	redrawButton(void);

	BOOL	EnableWindow(BOOL bEnable = TRUE);
	void	setToggle(bool toggle);
	bool	getToggle(void);
	bool    getEnable(void) { return m_isEnable; }
	void	setIconRect(int x, int y, int width, int height);

private:
	void	drawButton(CDC *pDC, GdiPlusBitmapResource *image, COLORREF clrText);


private:
	GdiPlusBitmapResource m_imageDisable;
	GdiPlusBitmapResource m_imageNormal;
	GdiPlusBitmapResource m_imageNormalPush;
	GdiPlusBitmapResource m_imageToggle;
	GdiPlusBitmapResource m_imageTogglePush;

	COLORREF	m_clrDisable;
	COLORREF	m_clrNormal;
	COLORREF	m_clrNormalPush;
	COLORREF	m_clrToggle;
	COLORREF	m_clrTogglePush;

	bool		m_isEnable;
	bool		m_isToggle;
	bool		m_showText;

	UINT		m_nFormat;
	UINT		m_nLeftMargin;
	UINT		m_nRightMargin;

	Gdiplus::Rect	m_nIconRect;

	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


