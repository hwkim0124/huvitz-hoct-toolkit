#pragma once
#include "ReportCtrlDef.h"
#include "DrawRadiusMap.h"

using namespace ReportCtrl;

class REPORTCTRL_DLL_API RadiusMap : public CWnd, public DrawRadiusMap
{
	DECLARE_DYNAMIC(RadiusMap)

public:
	RadiusMap();
	virtual ~RadiusMap();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};

