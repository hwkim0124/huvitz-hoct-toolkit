#pragma once
#include "ImageBase.h"
#include "ReportCtrlDef.h"

class REPORTCTRL_DLL_API ImageEnface : public ImageBase
{
public:
	ImageEnface();
	virtual ~ImageEnface();

protected:
	void getDstArea(int *xDst, int *yDst, int *wDst, int *hDst);
};