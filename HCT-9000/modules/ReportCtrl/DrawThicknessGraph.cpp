#include "stdafx.h"
#include "DrawThicknessGraph.h"


using namespace Gdiplus;

DrawThicknessGraph::DrawThicknessGraph()
	: m_showCaption(false), m_colorCaption(RGB(0, 0, 0))
	, m_showAxisValue(false), m_colorAxisValue(RGB(0, 0, 0))
	, m_colorBkGround(RGB(0, 0, 0)), m_colorAxisLine(RGB(0, 0, 0)), m_colorGridLine(RGB(128, 128, 128))
	, m_colorSeries(RGB(0, 0, 0))
	, m_isTsnit(false), m_rangeMin(0), m_rangeMax(500)
{
	m_fontCaption.CreateFontW(-21, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Noto Sans CJK KR Medium"));
	m_fontAxisValue.CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, DEFAULT_PITCH, _T("Calibri"));

	m_data[0].clear();
	m_data[1].clear();
}


DrawThicknessGraph::~DrawThicknessGraph()
{
}


void DrawThicknessGraph::initialize(void)
{
	m_data[0].clear();
	m_data[1].clear();
}

void DrawThicknessGraph::setTsnitChart(bool tsnit)
{
	m_isTsnit = tsnit;
	return;
}

void DrawThicknessGraph::showCaption(bool show)
{
	m_showCaption = show;
	return;
}

void DrawThicknessGraph::setCaptionFont(CFont* font)
{
	m_fontCaption.Attach(font);
	return;
}

void DrawThicknessGraph::setCaptionColor(COLORREF color)
{
	m_colorCaption = color;
	return;
}

void DrawThicknessGraph::showAxisValue(bool show)
{
	m_showAxisValue = show;
}

void DrawThicknessGraph::setAxisValueFont(CFont* font)
{
	m_fontAxisValue.Attach(font);
	return;
}

void DrawThicknessGraph::setAxisValueColor(COLORREF color)
{
	m_colorAxisValue = color;
	return;
}

void DrawThicknessGraph::setBkGroundColor(COLORREF color)
{
	m_colorBkGround = color;
	return;
}

void DrawThicknessGraph::setAxisLineColor(COLORREF color)
{
	m_colorAxisLine = color;
	return;
}

void DrawThicknessGraph::setGridLineColor(COLORREF color)
{
	m_colorGridLine = color;
	return;
}

void DrawThicknessGraph::setSeriesColor(COLORREF color)
{
	m_colorSeries = color;
	return;
}

COLORREF DrawThicknessGraph::getSeriesColor()
{
	return m_colorSeries;
}

void DrawThicknessGraph::setThicknessLayer(OcularLayerType upper, OcularLayerType lower)
{
	if (upper == OcularLayerType::ILM && lower == OcularLayerType::NFL)
	{
		m_rangeMin = 0;
		m_rangeMax = 200;
	}
	else if (upper == OcularLayerType::ILM && lower == OcularLayerType::RPE)
	{
		m_rangeMin = 0;
		m_rangeMax = 500;
	}
	else if (upper == OcularLayerType::EPI && lower == OcularLayerType::BOW)
	{
		m_rangeMin = 0;
		m_rangeMax = 200;

	}
	else if (upper == OcularLayerType::EPI && lower == OcularLayerType::END)
	{
		m_rangeMin = 0;
		m_rangeMax = 1000;
	}
	else if (upper == OcularLayerType::ILM && lower == OcularLayerType::IPL)
	{
		m_rangeMin = 0;
		m_rangeMax = 300;
	}
	else
	{

	}

	return;
}

void DrawThicknessGraph::setThicknessData(UINT series, std::vector <float> data)
{
	if (series < MAX_SERIES)
	{
		m_data[series].clear();
		m_data[series].assign(data.begin(), data.end());
	}

	return;
}


void DrawThicknessGraph::setNormaltiveData(vector <float> normal, vector <float> border, vector <float> outside)
{
	m_dataNormal = normal;
	m_dataBorder = border;
	m_dataOutside = outside;
	return;
}

void DrawThicknessGraph::drawThicknessGraph(CDC *pDC, CRect rect, bool report)
{
	CRect rectGraph;
	rectGraph.left = rect.left + 40;
	rectGraph.right = rect.right - 10;
	rectGraph.top = rect.top + 20;
	rectGraph.bottom = rect.bottom - 20;

	// series
	if (m_isTsnit)
	{
		if (report)
		{
			drawThicknessGraph(pDC, rectGraph, RGB(0, 0, 0), RGB(128, 128, 128), RGB(0, 0, 0));
			drawNormative(pDC, rectGraph);
			drawSeries(pDC, rectGraph, m_data[0], Gdiplus::Color(GetRValue(m_colorSeries),
				GetGValue(m_colorSeries), GetBValue(m_colorSeries)));
		}
		else
		{
			drawThicknessGraph(pDC, rectGraph, m_colorAxisLine, m_colorGridLine, m_colorAxisValue);
			drawNormative(pDC, rectGraph);
			drawSeries(pDC, rectGraph, m_data[0], Gdiplus::Color(GetRValue(m_colorSeries),
				GetGValue(m_colorSeries), GetBValue(m_colorSeries)));
		}
	}
	else
	{
		if (report)
		{
			drawThicknessGraph(pDC, rectGraph, RGB(0, 0, 0), RGB(255, 255, 255), RGB(0, 0, 0));
			drawSeries(pDC, rectGraph, m_data[0], Gdiplus::Color(0xff, 0xfc, 0x00));
		}
		else
		{
			drawThicknessGraph(pDC, rectGraph, m_colorAxisLine, m_colorGridLine, m_colorAxisValue);
			drawSeries(pDC, rectGraph, m_data[1], Gdiplus::Color(0x00, 0xff, 0xff));
			drawSeries(pDC, rectGraph, m_data[0], Gdiplus::Color(0xff, 0xfc, 0x00));
		}
	}

	return;
}

void DrawThicknessGraph::drawThicknessGraphOnlySeries(CDC *pDC, CRect rect, Gdiplus::Color color, bool report)
{
	CRect rectGraph;
	rectGraph.left = rect.left + 40;
	rectGraph.right = rect.right - 10;
	rectGraph.top = rect.top + 20;
	rectGraph.bottom = rect.bottom - 20;

	if (m_isTsnit) {
		if (report) {
			drawSeries(pDC, rectGraph, m_data[0], color);
		}
		else {
			drawSeries(pDC, rectGraph, m_data[0], color);
		}
	}
	else {
		if (report) {
			drawSeries(pDC, rectGraph, m_data[0], color);
		}
		else {
			drawSeries(pDC, rectGraph, m_data[1], color);
			drawSeries(pDC, rectGraph, m_data[0], color);
		}
	}

	return;
}

void DrawThicknessGraph::drawThicknessGraph(CDC *pDC, CRect rect, COLORREF clrAxisLine, COLORREF clrGridLine, COLORREF clrValue)
{
	pDC->SelectObject(m_fontAxisValue);
	pDC->SetTextColor(clrValue);
	pDC->SetBkMode(TRANSPARENT);

	CPen penAxisLine, penGridLine;
	penAxisLine.CreatePen(PS_SOLID, 1, clrAxisLine);
	penGridLine.CreatePen(PS_SOLID, 1, clrGridLine);


	// grid line
	pDC->SelectObject(&penGridLine);

	for (size_t col = 0; col < 5; col++)
	{
		int py = rect.top + int(rect.Height() * col / 5);
		pDC->MoveTo(rect.left, py);
		pDC->LineTo(rect.right, py);
	}

	for (size_t row = 1; row <= 4; row++)
	{
		int px = rect.left + int(rect.Width() * row / 4);
		pDC->MoveTo(px, rect.top);
		pDC->LineTo(px, rect.bottom);
	}

	// axis line
	pDC->SelectObject(&penAxisLine);
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.left, rect.bottom);
	pDC->LineTo(rect.right, rect.bottom);

	// y-axis value
	CString value;

	int px = rect.left - 40;
	int py = rect.top;

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(Gdiplus::StringAlignmentNear);
	SF.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	Gdiplus::Color clrText;
	clrText.SetFromCOLORREF(clrValue);
	Gdiplus::SolidBrush brushText(clrText);

	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 5);
	G.DrawString(value, -1, &font, Gdiplus::PointF(px, py), &SF, &brushText);

	py += int(rect.Height() / 5);
	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 4);
	G.DrawString(value, -1, &font, Gdiplus::PointF(px, py), &SF, &brushText);

	py += int(rect.Height() / 5);
	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 3);
	G.DrawString(value, -1, &font, Gdiplus::PointF(px, py), &SF, &brushText);

	py += int(rect.Height() / 5);
	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 2);
	G.DrawString(value, -1, &font, Gdiplus::PointF(px, py), &SF, &brushText);

	py += int(rect.Height() / 5);
	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 1);
	G.DrawString(value, -1, &font, Gdiplus::PointF(px, py), &SF, &brushText);

	py += int(rect.Height() / 5);
	value.Format(_T("0зн"));
	G.DrawString(value, -1, &font, Gdiplus::PointF(px, py), &SF, &brushText);

//	py += int(rect.Height() / 5);
//	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 5);
//	pDC->DrawText(value, CRect(px, py - 8, px + 35, py + 8), DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
//
//	py += int(rect.Height() / 5);
//	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 4);
//	pDC->DrawText(value, CRect(px, py - 8, px + 35, py + 8), DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
//
//	py += int(rect.Height() / 5);
//	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 3);
//	pDC->DrawText(value, CRect(px, py - 8, px + 35, py + 8), DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
//
//	py += int(rect.Height() / 5);
//	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 2);
//	pDC->DrawText(value, CRect(px, py - 8, px + 35, py + 8), DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
//
//	py += int(rect.Height() / 5);
//	value.Format(_T("%d"), (m_rangeMax - m_rangeMin) / 5 * 1);
//	pDC->DrawText(value, CRect(px, py - 8, px + 35, py + 8), DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
//
//	py += int(rect.Height() / 5);
//	value.Format(_T("%dзн"), (m_rangeMax - m_rangeMin) / 5 * 0);
//	pDC->DrawText(value, CRect(px, py - 16, px + 35, py), DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);

	// x- axis value
	if (m_isTsnit)
	{
		int px = rect.left;
		int py = rect.bottom;

		px = rect.left;
		G.DrawString(_T("T"), -1, &font, Gdiplus::PointF(px - 7, py + 10), &SF, &brushText);
		px = rect.left + int(rect.Width() * 1 / 4);
		G.DrawString(_T("S"), -1, &font, Gdiplus::PointF(px - 7, py + 10), &SF, &brushText);
		px = rect.left + int(rect.Width() * 2 / 4);
		G.DrawString(_T("N"), -1, &font, Gdiplus::PointF(px - 7, py + 10), &SF, &brushText);
		px = rect.left + int(rect.Width() * 3 / 4);
		G.DrawString(_T("I"), -1, &font, Gdiplus::PointF(px - 7, py + 10), &SF, &brushText);
		px = rect.left + int(rect.Width() * 4 / 4);
		G.DrawString(_T("T"), -1, &font, Gdiplus::PointF(px - 7, py + 10), &SF, &brushText);
	}

	return;
}

void DrawThicknessGraph::drawSeries(CDC *pDC, CRect rect, std::vector <float> data, Gdiplus::Color color)
{
	size_t size = data.size();
	float pixel_x = rect.Width() / float(size - 1);
	float pixel_y = rect.Height() / float(m_rangeMax - m_rangeMin);

	vector <Gdiplus::Point> points(size);

	for (size_t i = 0; i < size; i++)
	{
		int value = data[i];
		value = min(value, m_rangeMax);

		points[i].X = rect.left + int(i * pixel_x);
		points[i].Y = rect.bottom - int(value * pixel_y);
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Pen pen(color, 2.0f);

	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	G.DrawLines(&pen, points.data(), (INT)points.size());

	return;
}


void DrawThicknessGraph::drawNormative(CDC *pDC, CRect rect)
{
	if (m_dataNormal.empty() || m_dataBorder.empty() || m_dataOutside.empty())
	{
		return;
	}

	size_t size = 256;
	float pixel_x = rect.Width() / float(size - 1);
	float pixel_y = rect.Height() / float(m_rangeMax - m_rangeMin);

	vector <Gdiplus::Point> pointNormal(size);
	vector <Gdiplus::Point> pointBorder(size);
	vector <Gdiplus::Point> pointOutside(size);

	for (size_t i = 0; i < size; i++)
	{
		float value;

		value = min(m_dataNormal[i], (float)m_rangeMax);
		pointNormal[i].X = rect.left + int(i * pixel_x);
		pointNormal[i].Y = rect.bottom - int(value * pixel_y);

		value = min(m_dataBorder[i], (float)m_rangeMax);
		pointBorder[i].X = rect.left + int(i * pixel_x);
		pointBorder[i].Y = rect.bottom - int(value * pixel_y);

		value = min(m_dataOutside[i], (float)m_rangeMax);
		pointOutside[i].X = rect.left + int(i * pixel_x);
		pointOutside[i].Y = rect.bottom - int(value * pixel_y);
	}

	pointNormal.push_back(Gdiplus::Point(rect.right, rect.bottom));
	pointNormal.push_back(Gdiplus::Point(rect.left, rect.bottom));
	pointBorder.push_back(Gdiplus::Point(rect.right, rect.bottom));
	pointBorder.push_back(Gdiplus::Point(rect.left, rect.bottom));
	pointOutside.push_back(Gdiplus::Point(rect.right, rect.bottom));
	pointOutside.push_back(Gdiplus::Point(rect.left, rect.bottom));

	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::SolidBrush brush(Color::White);

	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	brush.SetColor(getPercentileColor(0.95));
	G.FillRectangle(&brush, rect.left, rect.top, rect.Width(), rect.Height());

	brush.SetColor(getPercentileColor(0.05));
	G.FillPolygon(&brush, pointNormal.data(), (INT)pointNormal.size());

	brush.SetColor(getPercentileColor(0.01));
	G.FillPolygon(&brush, pointBorder.data(), (INT)pointBorder.size());

	brush.SetColor(getPercentileColor(0.00));
	G.FillPolygon(&brush, pointOutside.data(), (INT)pointOutside.size());

	return;
}


void DrawThicknessGraph::drawColorTable(CDC *pDC, Gdiplus::RectF rect_in, Gdiplus::Color value, int fontSize)
{
	CRect rect(rect_in.GetLeft(), rect_in.GetTop(), rect_in.GetRight(), rect_in.GetBottom());

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

	brushRect.SetColor(getPercentileColor(0.95));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("95%"), -1, &font, Gdiplus::PointF(x + 35, y + 18), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.05));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("5%"), -1, &font, Gdiplus::PointF(x + 35, y + 18), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.01));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);
	G.DrawString(_T("1%"), -1, &font, Gdiplus::PointF(x + 35, y + 18), &brushText);

	y += h;
	brushRect.SetColor(getPercentileColor(0.00));
	G.FillRectangle(&brushRect, x, y, w, h);
	G.DrawRectangle(&pen, x, y, w, h);

	return;
}

Gdiplus::Color DrawThicknessGraph::getPercentileColor(float percent)
{
	Gdiplus::Color ret;

	if (percent >= 0.95f)
	{
		ret.SetFromCOLORREF(RGB(253, 255, 254));
	}
	else if (percent >= 0.05f)
	{
		ret.SetFromCOLORREF(RGB(140, 199, 98));
	}
	else if (percent >= 0.01f)
	{
		ret.SetFromCOLORREF(RGB(249, 239, 104));
	}
	else
	{
		ret.SetFromCOLORREF(RGB(242, 106, 112));
	}

	return ret;
}

