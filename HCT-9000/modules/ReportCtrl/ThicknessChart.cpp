// ThicknessChart.cpp : implementation file
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "ThicknessChart.h"


// ThicknessChart
using namespace ReportCtrl;
using namespace Gdiplus;

IMPLEMENT_DYNAMIC(ThicknessChart, CWnd)

ThicknessChart::ThicknessChart()
{

}

ThicknessChart::~ThicknessChart()
{
}

void ThicknessChart::initialize(EyeSide side)
{
	DrawEtdrsChart::initialize(side);
	DrawGccChart::initialize(side);
	DrawRnflChart::initialize(side);
	return;
}

void ThicknessChart::setCompareType(EyeSide side)
{
	DrawEtdrsChart::setCompareType(side);
	DrawGccChart::setCompareType(side);
	DrawRnflChart::setCompareType(side);
}

void ThicknessChart::showChartColor(bool show)
{
	DrawEtdrsChart::showChartColor(show);
	DrawGccChart::showChartColor(show);
	DrawRnflChart::showChartColor(show);
	return;
}

void ThicknessChart::setChartType(ThickChartType type)
{
	m_chartType = type;
	return;
}

ThickChartType ThicknessChart::getChartType(void)
{
	return m_chartType;
}

void ThicknessChart::setColor(Gdiplus::Color line, Gdiplus::Color text, Gdiplus::Color tsni)
{
	m_colorLine = line;
	m_colorValue = text;
	m_colorTsnit = tsni;

	return;
}

BEGIN_MESSAGE_MAP(ThicknessChart, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// ThicknessChart message handlers
void ThicknessChart::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	CRect rectTable;
	rectTable.SetRect(340, 30, 380, 30 + 170);

	Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	if (m_chartType == ThickChartType::etdrs) {
		float radius = 80;
		if (isShowingEtdrsChartColor()) {
			CPoint ptCenterChart(0 + 180, 30 + 85);
			drawEtdrsChart(&dc, ptCenterChart, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
			drawEtdrsColorTable(&dc, rectTable, Gdiplus::Color::White, 14);
		}
		else {
			CPoint ptCenterChart(rect.Width() / 2, 30 + 85);
			drawEtdrsChart(&dc, ptCenterChart, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
		}
	}
	else if (m_chartType == ThickChartType::gcc) {
		float radius = 60;

		if (isShowingGccChartColor()) {
			CPoint ptCenterChart(rect.Width() / 5, 30 + 85);
			CPoint ptCenterChartSI(rect.Width() * 3 / 5, 30 + 85);
			drawGccChart(&dc, ptCenterChart, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
			drawGccChartSI(&dc, ptCenterChartSI, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
			drawGccColorTable(&dc, rectTable, Gdiplus::Color::White, 14);
		}
		else {
			CPoint ptCenterChart(rect.Width() / 4, 30 + 85);
			CPoint ptCenterChartSI(rect.Width() * 3 / 4, 30 + 85);
			drawGccChart(&dc, ptCenterChart, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
			drawGccChartSI(&dc, ptCenterChartSI, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
		}
	}
	else if (m_chartType == ThickChartType::rnfl) {
		float radius = 52;
		if (isShowingRnflChartColor()) {
			CPoint ptCenterQuad(0 + 85, 30 + 85);
			drawRnflChartQuad(&dc, ptCenterQuad, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
			CPoint ptCenterClock(170 + 85, 30 + 85);
			drawRnflChartClock(&dc, ptCenterClock, radius, font, m_colorLine, m_colorValue);
			drawRnflColorTable(&dc, rectTable, Gdiplus::Color::White, 14);
		}
		else {
			CPoint ptCenterQuad(rect.Width() / 4, 30 + 85);
			drawRnflChartQuad(&dc, ptCenterQuad, radius, font, m_colorLine, m_colorValue, m_colorTsnit);
			CPoint ptCenterClock(rect.Width() * 3 / 4, 30 + 85);
			drawRnflChartClock(&dc, ptCenterClock, radius, font, m_colorLine, m_colorValue);
		}
	}

	return;
}