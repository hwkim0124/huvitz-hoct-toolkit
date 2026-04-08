#pragma once
#include "GuiCtrlDef.h"
#include "GdiPlusBitmap.h"

#define GdiPlusBitmapResource ImgResource

class GUICTRL_DLL_API GdiPlusBitmapResource : public GdiPlusBitmap
{
public:
	GdiPlusBitmapResource();
	GdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	GdiPlusBitmapResource(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	GdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst = NULL);
	virtual ~GdiPlusBitmapResource();

	void Empty();
	bool Load(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	bool Load(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
	bool Load(UINT id, UINT type, HMODULE hInst = NULL);

protected:
	HGLOBAL m_hBuffer;
};

