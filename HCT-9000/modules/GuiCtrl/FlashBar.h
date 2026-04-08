#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// FlashBar

class GUICTRL_DLL_API FlashBar : public CWnd
{
	DECLARE_DYNAMIC(FlashBar)

public:
	FlashBar();
	virtual ~FlashBar();

	void LoadImages(UINT id1, UINT id2, LPCTSTR type = _T("PNG"));
	void SetMaxValue(UINT value);
	void SetValue(UINT value);

private:
	GdiPlusBitmapResource m_image[2];

	UINT m_maxValue;
	UINT m_value;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};


