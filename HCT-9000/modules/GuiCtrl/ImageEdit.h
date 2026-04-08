#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmapResource.h"

// ImageEdit

class GUICTRL_DLL_API ImageEdit : public CEdit
{
	DECLARE_DYNAMIC(ImageEdit)

public:
	ImageEdit();
	virtual ~ImageEdit();
	void loadImage(UINT id, LPCTSTR pType = _T("PNG"));

private:
	ImgResource m_imageBK;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


