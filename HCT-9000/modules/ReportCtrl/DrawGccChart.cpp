#include "stdafx.h"
#include "DrawGccChart.h"

using namespace Gdiplus;

DrawGccChart::DrawGccChart()
	: m_showChartColor(true)
{
	m_direction[0] = _T("S");
	m_direction[1] = _T("N");
	m_direction[2] = _T("I");
	m_direction[3] = _T("T");
}

DrawGccChart::~DrawGccChart()
{
}

void DrawGccChart::initialize(EyeSide side)
{
	m_eyeSide = side;

	return;
}

void DrawGccChart::setCompareType(EyeSide side)
{
	m_eyeSide = side;
}

void DrawGccChart::showChartColor(bool show)
{
	m_showChartColor = show;

	return;
}

bool DrawGccChart::isShowingGccChartColor()
{
	return m_showChartColor;
}

void DrawGccChart::clearData(void)
{
	for (size_t i = 0; i < 6; i++)
	{
		m_thickness[i] = 0.0f;
		m_percentile[i] = -1.0f;	// N/A
	}

	for (size_t i = 0; i < 2; i++) {
		m_thicknessSI[i] = 0.0f;
		m_percentileSI[i] = -1.0f;	// N/A
	}

	return;
}

void DrawGccChart::setGccChartThickness(int index, float thick)
{
	m_thickness[index] = thick;

	return;
}

float DrawGccChart::getGccChartThickness(int index)
{
	return m_thickness[index];
}

void DrawGccChart::setGccChartThicknessSI(int index, float thick)
{
	if (index < 0 || index >= 2) {
		return;
	}

	m_thicknessSI[index] = thick;
}

float DrawGccChart::getGccChartThicknessSI(int index)
{
	if (index < 0 || index >= 2) {
		return -1;
	}

	return m_thicknessSI[index];
}

void DrawGccChart::setGccChartPercentile(int index, float percent)
{
	m_percentile[index] = percent;

	return;
}

float DrawGccChart::getGccChartPercentile(int index)
{
	return m_percentile[index];
}

void DrawGccChart::setGccChartPercentileSI(int index, float percent)
{
	m_percentileSI[index] = percent;

	return;
}

float DrawGccChart::getGccChartPercentileSI(int index)
{
	return m_percentileSI[index];
}

void DrawGccChart::drawGccChart(CDC *pDC, CPoint ptCenter, float radius, Font &font, Color line, Color value, Color tsnit)
{
	int centerX, centerY;
	centerX = ptCenter.x;
	centerY = ptCenter.y;

	int radiusInner, radiusOuter;
	radiusInner = int(radius / 4);
	radiusOuter = int(radius);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen pen(line, 2.0f);
	Gdiplus::SolidBrush brushValue(value);
	Gdiplus::SolidBrush brushTsnit(tsnit);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::RectF rcOuter(centerX - radiusOuter, centerY - radiusOuter, radiusOuter * 2, radiusOuter * 2);
	Gdiplus::RectF rcInner(centerX - radiusInner, centerY - radiusInner, radiusInner * 2, radiusInner * 2);

	if (m_showChartColor)
	{
		// 색상 : 60도 부터 EyeSide가 OS이면 반시계, OD/OU이면 시계방향
		for (int i = 0; i < 6; i++)
		{
			float angleStart = (m_eyeSide == EyeSide::OS) ? (60 + (i * 60)) : (60 - (i * 60));
			float angleSweep = 60;
			
			Color color = getPercentileColor(m_percentile[i]);
			G.FillPie(&SolidBrush(color), rcOuter, -angleStart, -angleSweep);
		}

		Color color = Color::Black;
		G.FillEllipse(&Gdiplus::SolidBrush(color), rcInner);
	}

	// 두께값 : 90도 부터 EyeSide가 OS이면 반시계, OD/OU이면 시계방향
	for (int i = 0; i < 6; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (90 + (i * 60)) : (90 - (i * 60));
		float radian = M_PI * angle / 180;
		float radius = (radiusInner + radiusOuter) / 2;
		
		CString text;
		text.Format(_T("%d"), (int)m_thickness[i]);

		Gdiplus::PointF point;
		point.X = centerX + int(radius * cos(radian));
		point.Y = centerY - int(radius * sin(radian));
		G.DrawString(text, -1, &font, point, &SF, &brushValue);
	}

	// 경계선 : 60도 부터
	for (int i = 0; i < 6; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (60 + (i * 60)) : (60 - (i * 60));
		float radian = M_PI * angle / 180;

		int x1 = centerX + int(radiusInner * cos(radian));
		int y1 = centerY - int(radiusInner * sin(radian));
		int x2 = centerX + int(radiusOuter * cos(radian));
		int y2 = centerY - int(radiusOuter * sin(radian));
		G.DrawLine(&pen, x1, y1, x2, y2);
	}

	G.DrawEllipse(&pen, rcInner);
	G.DrawEllipse(&pen, rcOuter);

	// TSNI : 90도부터
	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (90 + (i * 90)) : (90 - (i * 90));
		float radian = M_PI * angle / 180;
		float radius = radiusOuter + 10;

		Gdiplus::PointF point;
		point.X = centerX + int(radius * cos(radian));
		point.Y = centerY - int(radius * sin(radian));
		G.DrawString(m_direction[i], -1, &font, point, &SF, &brushTsnit);
	}

	return;
}

void DrawGccChart::drawGccChartSI(CDC *pDC, CPoint ptCenter, float radius, Font &font, Color line, Color value, Color tsnit)
{
	int centerX, centerY;
	centerX = ptCenter.x;
	centerY = ptCenter.y;

	int radiusInner, radiusOuter;
	radiusInner = int(radius / 4);
	radiusOuter = int(radius);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen pen(line, 2.0f);
	Gdiplus::SolidBrush brushValue(value);
	Gdiplus::SolidBrush brushTsnit(tsnit);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::RectF rcOuter(centerX - radiusOuter, centerY - radiusOuter, radiusOuter * 2, radiusOuter * 2);
	Gdiplus::RectF rcInner(centerX - radiusInner, centerY - radiusInner, radiusInner * 2, radiusInner * 2);

	if (m_showChartColor)
	{
		Color color0 = getPercentileColor(m_percentileSI[0]);
		G.FillPie(&SolidBrush(color0), rcOuter, -0, -180);

		Color color1 = getPercentileColor(m_percentileSI[1]);
		G.FillPie(&SolidBrush(color1), rcOuter, -180, -180);

		Color color = Color::Black;
		G.FillEllipse(&Gdiplus::SolidBrush(color), rcInner);
	}

	// 두께값
	{
		float radius = (radiusInner + radiusOuter) / 2;

		CString textS, textI;
		textS.Format(_T("%d"), (int)m_thicknessSI[0]);
		textI.Format(_T("%d"), (int)m_thicknessSI[1]);

		Gdiplus::PointF pointS, pointI;
		pointS.X = centerX + int(radius * cos(M_PI / 2));
		pointS.Y = centerY - int(radius * sin(M_PI / 2));
		pointI.X = centerX + int(radius * cos(M_PI * 3 / 2));
		pointI.Y = centerY - int(radius * sin(M_PI * 3 / 2));

		G.DrawString(textS, -1, &font, pointS, &SF, &brushValue);
		G.DrawString(textI, -1, &font, pointI, &SF, &brushValue);
	}

	// 경계선
	{
		int x1[2], x2[2], y1[2], y2[2];

		x1[0] = centerX + int(radiusInner * cos(0));
		y1[0] = centerY - int(radiusInner * sin(0));
		x2[0] = centerX + int(radiusOuter * cos(0));
		y2[0] = centerY - int(radiusOuter * sin(0));

		x1[1] = centerX + int(radiusInner * cos(M_PI));
		y1[1] = centerY - int(radiusInner * sin(M_PI));
		x2[1] = centerX + int(radiusOuter * cos(M_PI));
		y2[1] = centerY - int(radiusOuter * sin(M_PI));

		G.DrawLine(&pen, x1[0], y1[0], x2[0], y2[0]);
		G.DrawLine(&pen, x1[1], y1[1], x2[1], y2[1]);
	}

	G.DrawEllipse(&pen, rcInner);
	G.DrawEllipse(&pen, rcOuter);

	// TSNI : 90도부터
	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? 90 + (i * 90) : 90 - (i * 90);
		float radian = M_PI * angle / 180;
		float radius = radiusOuter + 10;

		Gdiplus::PointF point;
		point.X = centerX + int(radius * cos(radian));
		point.Y = centerY - int(radius * sin(radian));
		G.DrawString(m_direction[i], -1, &font, point, &SF, &brushTsnit);
	}

	return;
}

void DrawGccChart::drawGccColorTable(CDC *pDC, CRect rect, Gdiplus::Color colorText, int fontSize)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	CString text = _T("");
	StringFormat SF;
	SF.SetAlignment(StringAlignmentNear);
	SF.SetLineAlignment(StringAlignmentNear);
	Font font(_T("Calibri"), fontSize, FontStyleRegular, UnitPixel);

	Gdiplus::SolidBrush brushRect(Color::White);
	Gdiplus::SolidBrush brushText(colorText);
	Gdiplus::Pen pen(Gdiplus::Color::Black);

	Gdiplus::PointF point(rect.left, rect.top);
	G.DrawString(_T("Distribution\nof Normals"), -1, &font, point, &SF, &brushText);

	INT x, y, w, h;
	x = rect.left + 10;
	y = rect.top + 40;
	w = 30;
	h = 20;

	brushRect.SetColor(getPercentileColor(1.00));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("99%"), -1, &font, Gdiplus::PointF(x + 35, y + 12), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.99));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("95%"), -1, &font, Gdiplus::PointF(x + 35, y + 12), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.95));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("5%"), -1, &font, Gdiplus::PointF(x + 35, y + 12), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.05));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("1%"), -1, &font, Gdiplus::PointF(x + 35, y + 12), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.01));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	return;
}

Gdiplus::Color DrawGccChart::getPercentileColor(float percent)
{
	Gdiplus::Color ret;

	if (percent > 0.99f) {
		ret.SetFromCOLORREF(RGB(247, 205, 205));
	}
	else if (percent >= 0.99f)
	{
		ret.SetFromCOLORREF(RGB(248, 247, 207));
	}
	else if (percent >= 0.95f)
	{
		ret.SetFromCOLORREF(RGB(84, 253, 94));
	}
	else if (percent >= 0.05f)
	{
		ret.SetFromCOLORREF(RGB(255, 252, 90));
	}
	else if (percent >= 0.01f)
	{
		ret.SetFromCOLORREF(RGB(253, 69, 73));
	}
	else
	{
		ret = Gdiplus::Color(128, 128, 128);
	}

	return ret;
}