#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

#include <vector>

// AnimateButton

class GUICTRL_DLL_API AnimateButton : public CButton
{
public:
	AnimateButton();
	virtual ~AnimateButton();

	void loadStdImage(UINT id, LPCTSTR type = _T("PNG"));
	void loadAniImage(UINT *id, UINT count, LPCTSTR type = _T("PNG"));

	void play(void);
	void stop(void);

private:
	GdiPlusBitmapResource m_stdImage;
	std::vector <GdiPlusBitmapResource> m_aniImages;

	bool m_bPlay;
	UINT m_nIndex;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void PreSubclassWindow();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};


