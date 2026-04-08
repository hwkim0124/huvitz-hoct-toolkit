#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// LeftButton

class GUICTRL_DLL_API LeftButton : public CButton
{
	DECLARE_DYNAMIC(LeftButton)

	const LPCTSTR kImgType = _T("PNG");
	const COLORREF kTxtClrNormal = RGB(244, 248, 245);
	const COLORREF kTxtClrPressed = RGB(244, 248, 245);
	const COLORREF kTxtClrDisabled = RGB(128, 128, 128);
public:
	LeftButton();
	virtual ~LeftButton();
	
	void loadResources(CString label, UINT imgNormal, UINT imgPressed, UINT imgDisabled = 0);
	void setDrawTextParams(UINT nFormat, int leftMargin, int rightMargin);
	void redrawButton(void);

private:
	void drawButton(CDC *pDC, GdiPlusBitmapResource *image, COLORREF clrText);

private:
	GdiPlusBitmapResource m_imageDisable;
	GdiPlusBitmapResource m_imageNormal;
	GdiPlusBitmapResource m_imagePush;

	UINT		m_nFormat;
	UINT		m_nLeftMargin;
	UINT		m_nRightMargin;
	CString		m_lable;
	CFont		m_font;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};


