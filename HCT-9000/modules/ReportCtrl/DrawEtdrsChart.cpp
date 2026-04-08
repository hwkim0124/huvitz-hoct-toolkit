#include "stdafx.h"
#include "DrawEtdrsChart.h"

using namespace Gdiplus;

DrawEtdrsChart::DrawEtdrsChart()
	: m_showChartColor(true)
{
	m_direction[0] = _T("S");
	m_direction[1] = _T("N");
	m_direction[2] = _T("I");
	m_direction[3] = _T("T");
}

DrawEtdrsChart::~DrawEtdrsChart()
{
}

void DrawEtdrsChart::initialize(EyeSide side)
{
	m_eyeSide = side;

	return;
}

void DrawEtdrsChart::setCompareType(EyeSide side)
{
	m_eyeSide = side;
}

void DrawEtdrsChart::showChartColor(bool show)
{
	m_showChartColor = show;

	return;
}

bool DrawEtdrsChart::isShowingEtdrsChartColor()
{
	return m_showChartColor;
}

void DrawEtdrsChart::clearData(void)
{
	m_thickCenter = 0.0f;
	m_percentCenter = -1.0f;

	for (size_t i = 0; i < 4; i++)
	{
		m_thickInner[i] = 0.0f;
		m_thickOuter[i] = 0.0f;
		m_percentInner[i] = -1.0f;
		m_percentOuter[i] = -1.0f;
	}

	return;
}

void DrawEtdrsChart::setEtdrsChartThickCenter(float thick)
{
	m_thickCenter = thick;
	return;
}

void DrawEtdrsChart::setEtdrsChartThickInner(int index, float thick)
{
	m_thickInner[index] = thick;
	return;
}

void DrawEtdrsChart::setEtdrsChartThickOuter(int index, float thick)
{
	m_thickOuter[index] = thick;
	return;
}

float DrawEtdrsChart::getEtdrsChartThickCenter(void)
{
	return m_thickCenter;
}

float DrawEtdrsChart::getEtdrsChartThickInner(int index)
{
	return m_thickInner[index];
}

float DrawEtdrsChart::getEtdrsChartThickOuter(int index)
{
	return m_thickOuter[index];
}

void DrawEtdrsChart::setEtdrsChartPercentCenter(float value)
{
	m_percentCenter = value;

	return;
}

void DrawEtdrsChart::setEtdrsChartPercentInner(int index, float value)
{
	m_percentInner[index] = value;
	return;
}

void DrawEtdrsChart::setEtdrsChartPercentOuter(int index, float value)
{
	m_percentOuter[index] = value;
	return;
}

void DrawEtdrsChart::drawEtdrsChart(CDC *pDC, CPoint ptCenter, float radius, Font &font, Color line, Color value, Color tsnit)
{
	int centerX, centerY;
	centerX = ptCenter.x;
	centerY = ptCenter.y;

	int radiusCenter, radiusInner, radiusOuter;
	radiusCenter = int(radius / 3);
	radiusInner = int(radius / 3 * 2);
	radiusOuter = int(radius);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::RectF rcOuter(centerX - radiusOuter, centerY - radiusOuter, radiusOuter * 2, radiusOuter * 2);
	Gdiplus::RectF rcInner(centerX - radiusInner, centerY - radiusInner, radiusInner * 2, radiusInner * 2);
	Gdiplus::RectF rcCenter(centerX - radiusCenter, centerY - radiusCenter, radiusCenter * 2, radiusCenter * 2);

	Gdiplus::SolidBrush brushValue(value);
	Gdiplus::SolidBrush brushTsnit(tsnit);
	Gdiplus::Pen pen(line, 2.0f);

	if (m_showChartColor)
	{
		// 색상 : 45도 부터 EyeSide가 OS이면 반시계, OD/OU이면 시계방향
		for (int i = 0; i < 4; i++)
		{
			float angleStart = (m_eyeSide == EyeSide::OS) ? (45 + (i * 90)) : (45 - (i * 90));
			float angleSweep = 90;

			Color outerColor = getPercentileColor(m_percentOuter[i]);
			Color innerColor = getPercentileColor(m_percentInner[i]);

			G.FillPie(&SolidBrush(outerColor), rcOuter, -angleStart, -angleSweep);
			G.FillPie(&SolidBrush(innerColor), rcInner, -angleStart, -angleSweep);
		}

		Color centerColor = getPercentileColor(m_percentCenter);
		G.FillEllipse(&SolidBrush(centerColor), rcCenter);
	}

	// 경계선 : 45도 부터
	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (45 + (i * 90)) : (45 - (i * 90));
		float radian = M_PI * angle / 180;

		int x1 = centerX + int(radiusCenter * cos(radian));
		int y1 = centerY - int(radiusCenter * sin(radian));
		int x2 = centerX + int(radiusOuter * cos(radian));
		int y2 = centerY - int(radiusOuter * sin(radian));
		G.DrawLine(&pen, x1, y1, x2, y2);
	}

	G.DrawEllipse(&pen, rcCenter);
	G.DrawEllipse(&pen, rcInner);
	G.DrawEllipse(&pen, rcOuter);


	// 두께값
	CString text = _T("");
	Gdiplus::PointF ptText;

	ptText.X = centerX;
	ptText.Y = centerY;
	text.Format(_T("%d"), (int)m_thickCenter);
	G.DrawString(text, -1, &font, ptText, &SF, &brushValue);

	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (90 + (i * 90)) : (90 - (i * 90));
		float radian = M_PI * angle / 180;
		float radius1 = (radiusInner + radiusCenter) / 2;
		float radius2 = (radiusOuter + radiusInner) / 2;

		// inner text
		ptText.X = centerX + int(radius1 * cos(radian));
		ptText.Y = centerY - int(radius1 * sin(radian));
		text.Format(_T("%d"), (int)m_thickInner[i]);
		G.DrawString(text, -1, &font, ptText, &SF, &brushValue);

		// outer
		ptText.X = centerX + int(radius2 * cos(radian));
		ptText.Y = centerY - int(radius2 * sin(radian));
		text.Format(_T("%d"), (int)m_thickOuter[i]);
		G.DrawString(text, -1, &font, ptText, &SF, &brushValue);
	}


	// TSNI : 90도부터
	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? (90 + (i * 90)) : (90 - (i * 90));
		float radian = M_PI * angle / 180;
		float radius = radiusOuter + 10;

		ptText.X = centerX + int(radius * cos(radian));
		ptText.Y = centerY - int(radius * sin(radian));
		G.DrawString(m_direction[i], -1, &font, ptText, &SF, &brushTsnit);
	}

	return;
}

void DrawEtdrsChart::drawEtdrsColorTable(CDC *pDC, CRect rect, Gdiplus::Color colorText, int fontSize)
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

Gdiplus::Color DrawEtdrsChart::getPercentileColor(float percent)
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