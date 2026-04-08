#pragma once
#include "DrawEtdrsChart.h"
#include "DrawGccChart.h"
#include "DrawRnflChart.h"

// ThicknessChart

using namespace ReportCtrl;

class REPORTCTRL_DLL_API ThicknessChart : public CWnd, public DrawEtdrsChart, public DrawGccChart, public DrawRnflChart
{
	DECLARE_DYNAMIC(ThicknessChart)

public:
	ThicknessChart();
	virtual ~ThicknessChart();
	void initialize(EyeSide side);
	void setCompareType(EyeSide side);
	void showChartColor(bool show);

	void setChartType(ThickChartType type);
	ThickChartType getChartType(void);
	void setColor(Gdiplus::Color line, Gdiplus::Color value, Gdiplus::Color tsnit);

protected:
	ThickChartType m_chartType;
	bool m_showChartColor;

	Gdiplus::Color m_colorLine;
	Gdiplus::Color m_colorValue;
	Gdiplus::Color m_colorTsnit;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
};


