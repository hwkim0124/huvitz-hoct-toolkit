// ThicknessMap.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "ThicknessMap.h"


// ThicknessMap
using namespace Gdiplus;

IMPLEMENT_DYNAMIC(ThicknessMap, CWnd)

ThicknessMap::ThicknessMap()
	: m_scanPoint({ -1, -1 }), m_thicknessAtScanPoint(0.0f), m_showCorneaEtdrs(false)
{
	setTransparency(1.0);
}

ThicknessMap::~ThicknessMap()
{
}

void ThicknessMap::setScanPoint(int x, int y, float thickness)
{
	m_scanPoint = { x, y };
	m_thicknessAtScanPoint = thickness;
}

POINT ThicknessMap::getScanPoint()
{
	return m_scanPoint;
}

float ThicknessMap::getThicknessAtScanPoint()
{
	return m_thicknessAtScanPoint;
}

CRect ThicknessMap::getMapRect()
{
	CRect rc;
	GetClientRect(&rc);

	CRect rect;
	rect.top = rc.top;
	rect.left = (rc.Width() - rc.Height()) / 2;
	rect.bottom = rc.bottom;
	rect.right = (rc.Width() + rc.Height()) / 2;

	return rect;
}

CRect ThicknessMap::getTableRect()
{
	CRect rc;
	GetClientRect(&rc);

	CRect rect;
	rect.top = rc.top;
	rect.bottom = rc.bottom;
	rect.left = rc.right - 70;
	rect.right = rc.right;

	return rect;
}

void ThicknessMap::showCoirneaEtdrs(bool show)
{
	m_showCorneaEtdrs = show;
}

void ThicknessMap::drawScanPoint(CDC* pDC)
{
	Gdiplus::Graphics G(*pDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen pen(Gdiplus::Color::Black, 2.0f);

	POINT pt = m_scanPoint;

	// position
	if (pt.x >= 0 && pt.y >= 0) {
		int x1, x2, y1, y2;

		// cross
		x1 = pt.x - 5;
		y1 = pt.y - 5;
		x2 = pt.x + 5;
		y2 = pt.y + 5;
		G.DrawLine(&pen, x1, y1, x2, y2);

		x1 = pt.x - 5;
		y1 = pt.y + 5;
		x2 = pt.x + 5;
		y2 = pt.y - 5;
		G.DrawLine(&pen, x1, y1, x2, y2);
	}

	// thickness
	if (m_thicknessAtScanPoint >= 0.0f) {
		// - value
		CString thicknessStr;
		thicknessStr.Format(_T("%d"), (int)m_thicknessAtScanPoint);
		// - draw
		Gdiplus::Font fontThickness(_T("Noto Sans CJK KR Regular"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brushThickness(Gdiplus::Color::Black);
		Gdiplus::StringFormat sf;
		G.DrawString(thicknessStr, -1, &fontThickness, Gdiplus::PointF(pt.x - 10, pt.y + 10), &brushThickness);
	}
}

bool ThicknessMap::drawScanPointForReport(CDC* pDC, CRect rect)
{
	if (m_scanPoint.x < 0 || m_scanPoint.y < 0) {
		return false;
	}

	Gdiplus::Graphics G(*pDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen pen(Gdiplus::Color::Black, 2.0f);

	auto rcMap = getMapRect();
	if (rcMap.IsRectEmpty()) {
		return false;
	}

	POINT pt = {
		rect.left + (LONG)((float)rect.Width() / (float)rcMap.Width() * (float)(m_scanPoint.x - rcMap.left)),
		rect.top + (LONG)((float)rect.Height() / (float)rcMap.Height() * (float)(m_scanPoint.y - rcMap.top))
	};

	// position
	int x1, x2, y1, y2;

	// cross
	x1 = pt.x - 5;
	y1 = pt.y - 5;
	x2 = pt.x + 5;
	y2 = pt.y + 5;
	G.DrawLine(&pen, x1, y1, x2, y2);

	x1 = pt.x - 5;
	y1 = pt.y + 5;
	x2 = pt.x + 5;
	y2 = pt.y - 5;
	G.DrawLine(&pen, x1, y1, x2, y2);

	// thickness
	if (m_thicknessAtScanPoint >= 0.0f) {
		// - value
		CString thicknessStr;
		thicknessStr.Format(_T("%d"), (int)m_thicknessAtScanPoint);
		// - draw
		Gdiplus::Font fontThickness(_T("Noto Sans CJK KR Regular"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brushThickness(Gdiplus::Color::Black);
		Gdiplus::StringFormat sf;
		G.DrawString(thicknessStr, -1, &fontThickness,
			Gdiplus::PointF(pt.x - 10, pt.y + 10),
			&brushThickness);
	}

	return true;
}

BEGIN_MESSAGE_MAP(ThicknessMap, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// ThicknessMap 메시지 처리기입니다.



void ThicknessMap::OnPaint()
{
	CPaintDC dc(this);

	CRect rectMap = getMapRect();
	CRect rectTable = getTableRect();

	drawThicknessMap(&dc, rectMap);
	drawColorTable(&dc, rectTable, Gdiplus::Color::White);
	drawScanPoint(&dc);

	if (m_showCorneaEtdrs) {
		drawCorneaEtdrsChart(&dc, rectMap);
	}

	return;
}
