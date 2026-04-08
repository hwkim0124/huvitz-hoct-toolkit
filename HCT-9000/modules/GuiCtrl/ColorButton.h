#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// ColorButton

class GUICTRL_DLL_API ColorButton : public CButton
{
	DECLARE_DYNAMIC(ColorButton)

public:
	ColorButton();
	virtual ~ColorButton();

	void setColor(COLORREF color);

protected:
	COLORREF m_color;

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
};


