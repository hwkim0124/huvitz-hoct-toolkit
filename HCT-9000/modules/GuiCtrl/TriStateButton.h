#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// TriStateButton

class GUICTRL_DLL_API TriStateButton : public CButton
{
	DECLARE_DYNAMIC(TriStateButton)

public:
	TriStateButton();
	virtual ~TriStateButton();

	void	loadImages(UINT id1, UINT id2, UINT id3, LPCTSTR pType = _T("PNG"));
	void	redrawButton(void);
	void	setButtonState(UINT state);

private:
	void	drawButton(CDC *pDC, GdiPlusBitmapResource *image);

private:
	UINT	m_state;
	GdiPlusBitmapResource m_image[3];
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};


