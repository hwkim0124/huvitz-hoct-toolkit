#include "stdafx.h"
#include "DrawRnflChart.h"
#include "DiscTestConfig.h"
#include "CppUtil2.h"

using namespace Gdiplus;

DrawRnflChart::DrawRnflChart()
	: m_showChartColor(true)
{
	m_direction[0] = _T("S");
	m_direction[1] = _T("N");
	m_direction[2] = _T("I");
	m_direction[3] = _T("T");
}

DrawRnflChart::~DrawRnflChart()
{
}

void DrawRnflChart::initialize(EyeSide side)
{
	m_eyeSide = side;

	return;
}

void DrawRnflChart::setCompareType(EyeSide side)
{
	m_eyeSide = side;
}

void DrawRnflChart::showChartColor(bool show)
{
	m_showChartColor = show;

	return;
}

bool DrawRnflChart::isShowingRnflChartColor()
{
	return m_showChartColor;
}

void DrawRnflChart::clearData(void)
{
	for (size_t i = 0; i < 4; i++)
	{
		m_thicknessQuad[i] = 0.0f;
		m_percentileQuad[i] = 0.0f;
	}

	for (size_t i = 0; i < 12; i++)
	{
		m_thicknessClock[i] = -1.0f;
		m_percentileClock[i] = -1.0f;
	}

	return;
}

void DrawRnflChart::setRnflQuadThickness(int index, float thickness)
{
	int indexs = index;
	m_thicknessQuad[index] = thickness;

	return;
}

void DrawRnflChart::setRnflClockThickness(int index, float thickness)
{
	m_thicknessClock[index] = thickness;

	return;
}

void DrawRnflChart::setRnflQuadPercentile(int index, float percentile)
{
	m_percentileQuad[index] = percentile;

	return;
}

void DrawRnflChart::setRnflClockPercentile(int index, float percentile)
{
	m_percentileClock[index] = percentile;

	return;
}

void DrawRnflChart::drawRnflChartQuad(CDC *pDC, CPoint ptCenter, int radius, Gdiplus::Font &font, Gdiplus::Color line, Gdiplus::Color value, Gdiplus::Color tsni)
{
	int centerX = ptCenter.x;
	int centerY = ptCenter.y;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen penLine(line, 2.0f);
	Gdiplus::SolidBrush brushValue(value);
	Gdiplus::SolidBrush brushDirection(tsni);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::Rect rcChart(centerX - radius, centerY - radius, radius * 2, radius * 2);
	

	for (int i = 0; i < 4; i++)
	{
		// 90 degree start, OS is counterclockwise, OD/OU is clockwise
		float angle = (m_eyeSide == EyeSide::OS) ? 90 + (i * 90) : 90 - (i * 90);
		float radian = M_PI * angle / 180;

		// color & outline
		float angleStart = angle - 45;
		float angleSweep = 90;

		if (m_showChartColor)
		{
			auto color = getPercentileColor(m_percentileQuad[i]);
			G.FillPie(&SolidBrush(color), rcChart, -angleStart, -angleSweep);
		}
		G.DrawPie(&penLine, rcChart, -angleStart, -angleSweep);

		// TSNI
		Gdiplus::PointF point1;
		point1.X = centerX + INT((radius / 2) * cos(radian));
		point1.Y = centerY - INT((radius / 2) * sin(radian));
		G.DrawString(m_direction[i], -1, &font, point1, &SF, &brushDirection);

		// Thickness value
		CString text;
		text.Format(_T("%d"), (int)m_thicknessQuad[i]);

		Gdiplus::PointF point2;
		point2.X = centerX + INT((radius + 15) * cos(radian));
		point2.Y = centerY - INT((radius + 15) * sin(radian));
		G.DrawString(text, -1, &font, point2, &SF, &brushValue);
	}
	
	return;
}

void DrawRnflChart::drawRnflChartClock(CDC *pDC, CPoint ptCenter, int radius, Gdiplus::Font &font, Gdiplus::Color line, Gdiplus::Color value)
{
	int centerX = ptCenter.x;
	int centerY = ptCenter.y;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen penLine(line, 2.0f);
	Gdiplus::SolidBrush brushValue(value);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::Rect rcChart(centerX - radius, centerY - radius, radius * 2, radius * 2);

	for (int i = 0; i < 12; i++)
	{
		// 90 degree start, OS is counterclockwise, OD/OU is clockwise
		float angle = (m_eyeSide == EyeSide::OS) ? 90 + (i * 30) : 90 - (i * 30);
		float radian = M_PI * angle / 180;

		// color & outline
		float startAngle = -angle - 15;

		if (m_showChartColor)
		{
			auto color = getPercentileColor(m_percentileClock[i]);
			G.FillPie(&SolidBrush(color), rcChart, startAngle, 30);
		}
		G.DrawPie(&penLine, rcChart, startAngle, 30);

		// Thickness value
		CString text;
		text.Format(_T("%d"), (int)m_thicknessClock[i]);

		Gdiplus::PointF point2;
		point2.X = centerX + INT((radius + 15) * cos(radian));
		point2.Y = centerY - INT((radius + 15) * sin(radian));
		G.DrawString(text, -1, &font, point2, &SF, &brushValue);
	}


	return;
}

void DrawRnflChart::drawRnflColorTable(CDC *pDC, CRect rect, Gdiplus::Color value, int fontSize)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	CString text = _T("");
	StringFormat SF;
	SF.SetAlignment(StringAlignmentNear);
	SF.SetLineAlignment(StringAlignmentNear);
	Font font(_T("Calibri"), fontSize, FontStyleRegular, UnitPixel);
	Gdiplus::SolidBrush brushRect(Gdiplus::Color::White);
	Gdiplus::SolidBrush brushText(value);
	Gdiplus::Pen pen(Gdiplus::Color::Black);
	
	Gdiplus::PointF point(rect.left, rect.top + 10);
	G.DrawString(_T("Distribution\nof Normals"), -1, &font, point, &SF, &brushText);

	INT x, y, w, h;
	x = rect.left + 10;
	y = rect.top + 50;
	w = 30;
	h = 25;

	if (DiscTest::IsEnabled()) {
		brushRect.SetColor(getPercentileColor(0.97f));
	}
	else {
		brushRect.SetColor(getPercentileColor(1.00));
	}

	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("95%"), -1, &font, Gdiplus::PointF(x + 35, y + 18), &brushText);

	y += h;
	if (DiscTest::IsEnabled()) {
		brushRect.SetColor(getPercentileColor(0.50f));
	}
	else {
		brushRect.SetColor(getPercentileColor(0.95));
	}

	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("5%"), -1, &font, Gdiplus::PointF(x + 35, y + 18), &brushText);

	y += h;
	if (DiscTest::IsEnabled()) {
		brushRect.SetColor(getPercentileColor(0.03f));
	}
	else {
		brushRect.SetColor(getPercentileColor(0.05));
	}

	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("1%"), -1, &font, Gdiplus::PointF(x + 35, y + 18), &brushText);

	y += h;
	if (DiscTest::IsEnabled()) {
		brushRect.SetColor(getPercentileColor(0.005f));
	}
	else {
		brushRect.SetColor(getPercentileColor(0.01));
	}

	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	return;
}

Gdiplus::Color DrawRnflChart::getPercentileColor(float percent)
{
	Gdiplus::Color ret;

	if (DiscTest::IsEnabled()) {
		if (percent > 0.95f) {
			ret = Color(253, 255, 254); // White
		}
		else if (percent >= 0.05f) {
			ret = Color(140, 199, 98); // Green
		}
		else if (percent > 0.01f) {
			ret = Color(249, 239, 104); // Yellow
		}
		else {
			ret = Color(242, 106, 112); // Red
		}
	}
	else {
		if (percent > 0.95f)
		{
			ret = Color(253, 255, 254);
		}
		else if (percent >= 0.95f)
		{
			ret = Color(140, 199, 98);
		}
		else if (percent >= 0.05f)
		{
			ret = Color(249, 239, 104);
		}
		else if (percent >= 0.01f)
		{
			ret = Color(242, 106, 112);
		}
		else
		{
			ret = Color::Transparent;
		}
	}

	return ret;
}