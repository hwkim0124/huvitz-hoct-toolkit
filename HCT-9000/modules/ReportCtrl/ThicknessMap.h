#pragma once
#include "ReportCtrlDef.h"
#include "DrawThicknessMap.h"
#include "DrawCorneaEtdrsChart.h"

// ThicknessMap
using namespace ReportCtrl;

class REPORTCTRL_DLL_API ThicknessMap : public CWnd, public DrawThicknessMap, public DrawCorneaEtdrsChart
{
	DECLARE_DYNAMIC(ThicknessMap)

public:
	ThicknessMap();
	virtual ~ThicknessMap();

	void setScanPoint(int x, int y, float thickness);
	POINT getScanPoint();
	float getThicknessAtScanPoint();
	CRect getMapRect();
	CRect getTableRect();
	void showCoirneaEtdrs(bool show);
	void drawScanPoint(CDC* pDC);
	bool drawScanPointForReport(CDC* pDC, CRect rect);

protected:
	POINT m_scanPoint;
	float m_thicknessAtScanPoint;
	bool m_showCorneaEtdrs;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


