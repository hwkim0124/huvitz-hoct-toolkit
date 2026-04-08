#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// PushButton

class GUICTRL_DLL_API PushButton : public CButton
{
	DECLARE_DYNAMIC(PushButton)

public:
	PushButton();
	virtual ~PushButton();

	void	loadImages(UINT idNormal, UINT idPush, UINT idDisable = 0, LPCTSTR pType = _T("PNG"));
	void	showText(bool show);
	void	setTextColor(COLORREF clrNormal, COLORREF clrToggle, COLORREF clrDisabled = 0);
	void	setDrawTextParams(UINT nFormat, int leftMargin, int rightMargin);
	void	redrawButton(void);

private:
	void	drawButton(CDC *pDC, GdiPlusBitmapResource *image, COLORREF clrText);

private:
	GdiPlusBitmapResource m_imageDisable;
	GdiPlusBitmapResource m_imageNormal;
	GdiPlusBitmapResource m_imagePush;

	COLORREF	m_clrDisable;
	COLORREF	m_clrNormal;
	COLORREF	m_clrPush;

	bool		m_showText;

	UINT		m_nFormat;
	UINT		m_nLeftMargin;
	UINT		m_nRightMargin;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};


