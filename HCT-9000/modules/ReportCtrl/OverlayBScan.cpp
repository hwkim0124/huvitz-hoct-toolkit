#include "stdafx.h"
#include "OverlayBScan.h"
#include "ReportCtrl.h"

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(OverlayBScan, CWnd)

OverlayBScan::OverlayBScan()
{
}


OverlayBScan::~OverlayBScan()
{
}

void OverlayBScan::initialize(EyeSide side)
{
	return;
}

void OverlayBScan::showScanPattern(bool show)
{
	m_showScanPattern = show;
	return;
}

void OverlayBScan::showScanLine(bool show)
{
	m_showScanLine = show;
	return;
}
//
//void OverlayBScan::setScanPattern(OctScanPattern pattern, target_t target)
//{
//	m_scanPattern = pattern;
//	m_scanLine.clear();
//	m_target = target;
//
//	CRect rect;
//	GetClientRect(&rect);
//
//	CPoint ptCenter;
//	ptCenter = rect.CenterPoint();
//
//	int left = ptCenter.x + LONG(pattern.getStartX() * m_pixelPerMM);
//	int right = ptCenter.x + LONG(pattern.getCloseX() * m_pixelPerMM);
//	int top = ptCenter.y + LONG(pattern.getStartY() * m_pixelPerMM);
//	int bottom = ptCenter.y + LONG(pattern.getCloseY() * m_pixelPerMM);
//	int centerX = ptCenter.x + LONG(pattern.centerX() * m_pixelPerMM);
//	int centerY = ptCenter.y + LONG(pattern.centerY() * m_pixelPerMM);
//	int lines = pattern.getNumberOfScanLines();
//	int distHandle = kHandleSize / 2;
//
//	switch (pattern.getPatternName())
//	{
//	case OctGlobal::PatternName::MacularLine:
//	case OctGlobal::PatternName::DiscLine:
//	case OctGlobal::PatternName::AnteriorLine:
//	case OctGlobal::PatternName::AnteriorFull:
//
//	{
//		ScanLine line;
//
//		if (pattern.getDirection() == 0)
//		{
//			line.direction = 0;
//			line.pt1.X = left;
//			line.pt1.Y = centerY;
//			line.pt2.X = right;
//			line.pt2.Y = centerY;
//			line.hasHandle = false;
//		}
//		else
//		{
//			line.direction = 1;
//			line.pt1.X = centerX;
//			line.pt1.Y = top;
//			line.pt2.X = centerX;
//			line.pt2.Y = bottom;
//			line.hasHandle = false;
//		}
//
//		m_scanLine.push_back(line);
//	}
//	break;
//
//	case OctGlobal::PatternName::Macular3D:
//	case OctGlobal::PatternName::Disc3D:
//	case OctGlobal::PatternName::Anterior3D:
//	case OctGlobal::PatternName::MacularDisc:
//	
//
//	{
//		ScanLine line;
//
//		if (pattern.getDirection() == 0)
//		{
//			float spaceY = pattern.getScanRangeY() * m_pixelPerMM / float(lines - 1);
//
//			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
//			{
//				line.direction = 0;
//				line.pt1.X = left;
//				line.pt1.Y = top + int(i * spaceY);
//				line.pt2.X = right;
//				line.pt2.Y = top + int(i * spaceY);
//				line.hasHandle = true;
//				line.ptH.X = left - distHandle;
//				line.ptH.Y = top + int(i * spaceY);
//				m_scanLine.push_back(line);
//			}
//		}
//		else
//		{
//			float spaceX = pattern.getScanRangeX() * m_pixelPerMM / float(lines - 1);
//
//			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
//			{
//				line.direction = 1;
//				line.pt1.X = left + int(i * spaceX);
//				line.pt1.Y = top;
//				line.pt2.X = left + int(i * spaceX);
//				line.pt2.Y = bottom;
//				line.hasHandle = true;
//				line.ptH.X = left + int(i * spaceX);
//				line.ptH.Y = top - distHandle;
//				m_scanLine.push_back(line);
//			}
//
//		}
//	}
//	break;
//
//	case OctGlobal::PatternName::MacularRaster:
//	case OctGlobal::PatternName::DiscRaster:
//	{
//		ScanLine line;
//
//		if (pattern.getDirection() == 0)
//		{
//			float spaceY = pattern.getScanRangeY() * m_pixelPerMM / float(lines - 1);
//
//			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
//			{
//				line.direction = 0;
//				line.pt1.X = left;
//				line.pt1.Y = top + int(i * spaceY);
//				line.pt2.X = right;
//				line.pt2.Y = top + int(i * spaceY);
//				line.hasHandle = true;
//				line.ptH.X = left - distHandle;
//				line.ptH.Y = top + int(i * spaceY);
//				m_scanLine.push_back(line);
//			}
//		}
//		else
//		{
//			float spaceX = pattern.getScanRangeX() * m_pixelPerMM / float(lines - 1);
//
//			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
//			{
//				line.direction = 1;
//				line.pt1.X = left + int(i * spaceX);
//				line.pt1.Y = top;
//				line.pt2.X = left + int(i * spaceX);
//				line.pt2.Y = bottom;
//				line.hasHandle = true;
//				line.ptH.X = left + int(i * spaceX);
//				line.ptH.Y = top - distHandle;
//				m_scanLine.push_back(line);
//			}
//		}
//	}
//	break;
//
//	case OctGlobal::PatternName::MacularRadial:
//	case OctGlobal::PatternName::DiscRadial:
//	case OctGlobal::PatternName::AnteriorRadial:
//	{
//		int radiusX = LONG(pattern.getScanRangeX() * m_pixelPerMM / 2);
//		if (radiusX > kMaxRadius) {
//			radiusX = kMaxRadius;
//		}
//		int radiusY = LONG(pattern.getScanRangeY() * m_pixelPerMM / 2);
//		if (radiusY > kMaxRadius) {
//			radiusY = kMaxRadius;
//		}
//
//		for (int i = 0; i < 12; i++)
//		{
//			float angle = i * 15;
//			float radian = M_PI * angle / 180;
//
//			ScanLine line;
//			line.direction = 2;
//			line.pt1.X = centerX - int(radiusX * cos(radian));
//			line.pt1.Y = centerY - int(radiusY * sin(radian));
//			line.pt2.X = centerX + int(radiusX * cos(radian));
//			line.pt2.Y = centerY + int(radiusY * sin(radian));
//			line.hasHandle = true;
//			line.ptH.X = centerX - int((radiusX + distHandle) * cos(radian));
//			line.ptH.Y = centerY - int((radiusY + distHandle) * sin(radian));
//			m_scanLine.push_back(line);
//		}
//	}
//	break;
//
//	case OctGlobal::PatternName::Topography:
//	{
//		int radiusX = LONG(pattern.getScanRangeX() * m_pixelPerMM / 2);
//		if (radiusX > kMaxRadius) {
//			radiusX = kMaxRadius;
//		}
//		int radiusY = LONG(pattern.getScanRangeY() * m_pixelPerMM / 2);
//		if (radiusY > kMaxRadius) {
//			radiusY = kMaxRadius;
//		}
//
//		for (int i = 0; i < 16; i++)
//		{
//			float angle = i * 11.25;
//			float radian = M_PI * angle / 180;
//
//			ScanLine line;
//			line.direction = 2;
//			line.pt1.X = centerX - int(radiusX * cos(radian));
//			line.pt1.Y = centerY - int(radiusY * sin(radian));
//			line.pt2.X = centerX + int(radiusX * cos(radian));
//			line.pt2.Y = centerY + int(radiusY * sin(radian));
//			line.hasHandle = true;
//			line.ptH.X = centerX - int((radiusX + distHandle) * cos(radian));
//			line.ptH.Y = centerY - int((radiusY + distHandle) * sin(radian));
//			m_scanLine.push_back(line);
//		}
//	}
//	break;
//
//	case OctGlobal::PatternName::MacularCross:
//	{
//		float spaceMM = m_scanPattern.getScanLineSpace();
//		if (spaceMM <= 0.0f) {
//			spaceMM = 0.25f;
//		}
//
//		int space = int(spaceMM * m_pixelPerMM);
//
//		for (int i = 0; i < 5; i++)
//		{
//			ScanLine line;
//			line.direction = 0;
//			line.pt1.X = left;
//			line.pt1.Y = centerY + int((i - 2) * space);
//			line.pt2.X = right;
//			line.pt2.Y = centerY + int((i - 2) * space);
//			line.hasHandle = true;
//			line.ptH.X = left - distHandle;
//			line.ptH.Y = centerY + int((i - 2) * space);
//			m_scanLine.push_back(line);
//		}
//		for (int i = 5; i < 10; i++)
//		{
//			ScanLine line;
//			line.direction = 1;
//			line.pt1.X = centerX + int((i - 7) * space);
//			line.pt1.Y = top;
//			line.pt2.X = centerX + int((i - 7) * space);
//			line.pt2.Y = bottom;
//			line.hasHandle = true;
//			line.ptH.X = centerX + int((i - 7) * space);
//			line.ptH.Y = top - distHandle;
//			m_scanLine.push_back(line);
//		}
//	}
//	break;
//
//	case OctGlobal::PatternName::MacularCircle:
//	case OctGlobal::PatternName::DiscCircle:
//	{
//		ScanLine line;
//
//		line.direction = 3;
//		line.pt1.X = left;
//		line.pt1.Y = top;
//		line.pt2.X = right;
//		line.pt2.Y = bottom;
//		line.hasHandle = false;
//		m_scanLine.push_back(line);
//	}
//	break;
//
//	case OctGlobal::PatternName::DiscPoint:
//	case OctGlobal::PatternName::AnteriorPoint:
//	case OctGlobal::PatternName::AnteriorAngio:
//		break;
//	default:
//		break;
//	}
//
//	return;
//}

void OverlayBScan::setScanLine(CRect rtClient)
{
	m_scanLine.clear();

	CPoint ptCenter;
//	ptCenter = rtClient.CenterPoint();

	int left = rtClient.left;
	int right = rtClient.right;
	int top = rtClient.top;
	int bottom = rtClient.bottom;
//	int centerX = ptCenter.x;
//	int centerY = ptCenter.y;
	int lines = rtClient.Width();
	int distHandle = kHandleSize / 2; //변경필요
	//
	BScanLine line;


	//float spaceY = rtClient.Width() / float(lines - 1);

	for (int i = 0; i <lines; i++)
	{
		line.direction = 0;
		line.pt1.X = left + i;
		line.pt1.Y = top;// * spaceY);
		line.pt2.X = left + i;
		line.pt2.Y = bottom;// *spaceY);
		line.hasHandle = true;
		line.ptH.X = left + i;// left;// -distHandle;
		line.ptH.Y = top + 10;// top + int(i);// *spaceY);
		line.ptH2.X = left + i;
		line.ptH2.Y = bottom - 10;
		line.hasHandle = true;
		m_scanLine.push_back(line);
	}

	return;
}


void OverlayBScan::setWndPixelPer1MM(float pixel)
{
	m_pixelPerMM = pixel;

	return;
}

BEGIN_MESSAGE_MAP(OverlayBScan, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// OverlayBScan message handlers

void OverlayBScan::OnPaint()
{
	CPaintDC dc(this);
	if (m_showScanLine)
	{
	//	drawScanLine(&dc, 1);
		drawScanLine(&dc, 0);
	}

	//drawScanPoint(&dc, m_scanPoint, Gdiplus::Color::White);

	return;
}


void OverlayBScan::drawOverlayForReport(CDC *pDC, CRect rtClient, float pixelPerMM)
{
	auto pattern = m_scanPattern;
	auto name = m_scanPattern.getPatternName();

	CPoint ptCenter;
	ptCenter.x = rtClient.CenterPoint().x + LONG(pattern.centerX() * pixelPerMM);
	ptCenter.y = rtClient.CenterPoint().y + LONG(pattern.centerY() * pixelPerMM);

	CRect rtScan;
	rtScan.left = ptCenter.x - LONG(pattern.getScanRangeX() * pixelPerMM / 2);
	rtScan.right = ptCenter.x + LONG(pattern.getScanRangeX() * pixelPerMM / 2);
	rtScan.top = ptCenter.y - LONG(pattern.getScanRangeY() * pixelPerMM / 2);
	rtScan.bottom = ptCenter.y + LONG(pattern.getScanRangeY() * pixelPerMM / 2);

	// scan pattern
	if (m_showScanPattern) {
		drawScanPatternForReport(pDC, rtScan, rtClient, m_scanPattern, pixelPerMM);
	}

	// scan line
	if (m_showScanLine) {
		if (m_scanPattern.getPatternName() == PatternName::AnteriorLine ||
			m_scanPattern.getPatternName() == PatternName::MacularLine ||
			m_scanPattern.getPatternName() == PatternName::DiscCircle) {
			drawScanLine(pDC, rtClient, pixelPerMM, 0);
		}
		else {
			drawScanLine(pDC, rtClient, pixelPerMM, 0);
			drawScanLine(pDC, rtClient, pixelPerMM, 1);
		}
	}

	// 1ptm
	drawScanPointForReport(pDC, Gdiplus::Color::White, rtScan);

	return;
}


void OverlayBScan::drawScanPointForReport(CDC *pDC, Gdiplus::Color color, CRect rect)
{
	if (m_scanPoint.x < 0 || m_scanPoint.y < 0) {
		return;
	}

	auto rcMap = getScanPatternRect();

	POINT pt = {
		rect.left + (LONG)((float)rect.Width() / (float)rcMap.Width() * (float)(m_scanPoint.x - rcMap.left)),
		rect.top + (LONG)((float)rect.Height() / (float)rcMap.Height() * (float)(m_scanPoint.y - rcMap.top))
	};

	Gdiplus::Pen pen(color, 2.0f);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	// positon
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

void OverlayBScan::setBScanIndex(int index, int idxLine)
{
	if (idxLine < 768)
	{
		m_scanLineSelected[0] = m_scanLine[idxLine];
	}

	return;
}

void OverlayBScan::drawScanPattern(CDC *pDC, CRect rect, OctScanPattern pattern)
{
	switch (pattern.getPatternName())
	{
	case OctGlobal::PatternName::MacularLine:
	case OctGlobal::PatternName::DiscLine:
	case OctGlobal::PatternName::AnteriorLine:
	case OctGlobal::PatternName::MacularCross:
	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
	case OctGlobal::PatternName::MacularRaster:
	case OctGlobal::PatternName::DiscRaster:
	case OctGlobal::PatternName::Topography:
	case OctGlobal::PatternName::AnteriorFull:
		drawPatternLines(pDC);
		break;
	case OctGlobal::PatternName::Macular3D:
	case OctGlobal::PatternName::Disc3D:
	case OctGlobal::PatternName::Anterior3D:
	case OctGlobal::PatternName::MacularDisc:
//		drawPatternRect(pDC, rect);
		break;
		//}
		break;
		break;
	default:
		break;
	}

	return;
}

void OverlayBScan::drawScanPatternForReport(CDC *pDC, CRect rtScan, CRect rtClient,
	OctScanPattern pattern, float pixelPerMM)
{
	switch (pattern.getPatternName()) {
	case OctGlobal::PatternName::MacularLine:
	case OctGlobal::PatternName::AnteriorLine:
	case OctGlobal::PatternName::DiscLine:
	case OctGlobal::PatternName::AnteriorFull:
		break;
	case OctGlobal::PatternName::MacularCross:
	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
	case OctGlobal::PatternName::MacularRaster:
	case OctGlobal::PatternName::DiscRaster:
		drawPatternLinesForReport(pDC, rtClient, pixelPerMM);
		break;
	default:
		drawScanPattern(pDC, rtScan, pattern);
		break;
	}
}

void OverlayBScan::drawPatternLines(CDC *pDC)
{
	for (int i = 0; i < m_scanLine.size(); i++)
	{
		drawLine(pDC, m_scanLine[i]);
	}

	return;
}

void OverlayBScan::drawPatternLinesForReport(CDC *pDC, CRect rect, float pixelPerMM)
{
	CRect rtClient;
	GetClientRect(&rtClient);

	for (auto line : m_scanLine) {
		int x1 = rect.CenterPoint().x + ((line.pt1.X - rtClient.CenterPoint().x) / m_pixelPerMM)
			* pixelPerMM;
		int y1 = rect.CenterPoint().y + ((line.pt1.Y - rtClient.CenterPoint().y) / m_pixelPerMM)
			* pixelPerMM;
		int x2 = rect.CenterPoint().x + ((line.pt2.X - rtClient.CenterPoint().x) / m_pixelPerMM)
			* pixelPerMM;
		int y2 = rect.CenterPoint().y + ((line.pt2.Y - rtClient.CenterPoint().y) / m_pixelPerMM)
			* pixelPerMM;

		drawLine(pDC, x1, y1, x2, y2);
	}
}

void OverlayBScan::drawScanLine(CDC *pDC, int index)
{
	Gdiplus::Color color;
	color = (index == 0) ? kColorLine1 : kColorLine2;

	BScanLine line = m_scanLineSelected[index];

	if (line.direction == 3)
	{
	//	drawArrowCircle(pDC, line.pt1.X, line.pt1.Y, line.pt2.X, line.pt2.Y, color);
	}
	else
	{
		drawArrowLine(pDC, line, color);
	}

	return;
}

void OverlayBScan::drawScanLine(CDC *pDC, CRect rect, float pixelPerMM, int index)
{
	Gdiplus::Color color;
	color = (index == 0) ? kColorLine1 : kColorLine2;

	CRect rtClient;
	GetClientRect(&rtClient);

	// 기존 Client에 대한 좌표를 새로운 화면에 대한 좌표로..
	BScanLine line = m_scanLineSelected[index];
	int x1 = rect.CenterPoint().x + ((line.pt1.X - rtClient.CenterPoint().x) / m_pixelPerMM) * pixelPerMM;
	int y1 = rect.CenterPoint().y + ((line.pt1.Y - rtClient.CenterPoint().y) / m_pixelPerMM) * pixelPerMM;
	int x2 = rect.CenterPoint().x + ((line.pt2.X - rtClient.CenterPoint().x) / m_pixelPerMM) * pixelPerMM;
	int y2 = rect.CenterPoint().y + ((line.pt2.Y - rtClient.CenterPoint().y) / m_pixelPerMM) * pixelPerMM;

	if (line.direction == 3)
	{
//		drawArrowCircle(pDC, x1, y1, x2, y2, color);
	}
	else
	{
		drawArrowLine(pDC, x1, y1, x2, y2, color);
	}

	return;
}


void OverlayBScan::drawLine(CDC *pDC, int x1, int y1, int x2, int y2)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Color penColor(255, 87, 79);

	Gdiplus::Pen pen(penColor, 1.0f);

	G.DrawLine(&pen, x1, y1, x2, y2);

	return;
}

void OverlayBScan::drawArrowLine(CDC *pDC, int x1, int y1, int x2, int y2, Gdiplus::Color color)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Point points[5] =
	{
		Gdiplus::Point(0, 0),
		Gdiplus::Point(-3,-5),
		Gdiplus::Point(0, -3),
		Gdiplus::Point(3,-5),
		Gdiplus::Point(0,0),
	};

	Gdiplus::GraphicsPath path;
	path.AddPolygon(points, 5);

	Gdiplus::Pen pen(color, 2.0f);
	pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));

	G.DrawLine(&pen, x1, y1, x2, y2);


	return;
}

void OverlayBScan::drawLine(CDC *pDC, BScanLine line)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::Color penColor(255, 87, 79);
	Gdiplus::Pen pen(penColor, 1.0f);
	G.DrawLine(&pen, line.pt1, line.pt2);

	return;
}

void OverlayBScan::drawArrowLine(CDC *pDC, BScanLine line, Gdiplus::Color color)
{
	CRect rtClient;
	GetClientRect(&rtClient);
	line = m_scanLineSelected[0];
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	// arrow line
	Gdiplus::Point points[6] =
	{
		Gdiplus::Point(0,0),
		Gdiplus::Point(-5,0),
		Gdiplus::Point(-5,-2),
		Gdiplus::Point(0,-6),
		Gdiplus::Point(5,-2),
		Gdiplus::Point(5,0)
		

		

		/*Gdiplus::Point(0, 0),
		Gdiplus::Point(-3,-5),
		Gdiplus::Point(0, -3),
		Gdiplus::Point(3,-5),
		Gdiplus::Point(0,0),*/
	};
	Gdiplus::GraphicsPath path;
	path.AddPolygon(points, 6);

	Gdiplus::Pen pen(color, 2.0f);

	pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));
	//pen.GetCustomStartCap(&Gdiplus::CustomLineCap(&path2, nullptr));
	G.DrawLine(&pen, line.pt1, line.pt2);

	// handle
	if (line.hasHandle)
	{
		Gdiplus::Point points[6] =
		{
			Gdiplus::Point(0,0),
			Gdiplus::Point(-5,0),
			Gdiplus::Point(-5,-2),
			Gdiplus::Point(0,-6),
			Gdiplus::Point(5,-2),
			Gdiplus::Point(5,0)
		};

		/*{
			Gdiplus::Point(0,0),
			Gdiplus::Point(-6,-5),
			Gdiplus::Point(-6,-12),
			Gdiplus::Point(6,-12),
			Gdiplus::Point(6,-5),
			Gdiplus::Point(0,0),
		};*/
		Gdiplus::GraphicsPath path;
		path.AddPolygon(points, 6);


		Gdiplus::Pen pen(color, 2.0f);
		pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));
		G.DrawLine(&pen, line.ptH, line.pt1);
	}

	return;
}

void OverlayBScan::drawScanPoint(CDC *pDC, POINT pt, Gdiplus::Color color)
{
	if (pt.x < 0 || pt.y < 0) {
		return;
	}

	int x1, x2, y1, y2;

	Gdiplus::Pen pen(color, 2.0f);

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

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

int OverlayBScan::getCloseScanIndex(CPoint point)
{
	int ret = -1;
	float minDist = (kHandleSize / 2);

	for (size_t i = 0; i < 2; i++)
	{
		auto scanLine = m_scanLineSelected[i];
		if (scanLine.hasHandle)
		{
			float dist = getDistanceHandle(scanLine, point);
			if (dist < minDist)
			{
				minDist = dist;
				ret = i;
			}
			float dist2 = getDistanceHandle2(scanLine, point);
			if (dist2 < minDist)
			{
				minDist = dist;
				ret = i;
			}
		}
	}

	return ret;
}

int OverlayBScan::getCloseScanLine(CPoint point, int idxDisp, function<float(BScanLine, CPoint)> fnGetDist)
{
	float minDist = 48.0f;		//넉넉하게 잡음...
	
//	float minDist = 2.0f;		//넉넉하게 잡음...


	int idxScan = -1;
	int direction = m_scanLineSelected[idxDisp].direction;

	for (size_t i = 0; i < m_scanLine.size(); i++)
	{
		BScanLine line = m_scanLine[i];

		if (line.direction == direction)
		{
			float dist = 0;
			if (fnGetDist == nullptr) {
				dist = getDistanceHandle(line, point);
				float dist2 = getDistanceHandle2(line, point);
				if (dist > dist2)
				{
					dist = dist2;
				}
			}
			else {
				dist = fnGetDist(line, point);
			}

			if (dist < minDist)
			{
				minDist = dist;
				idxScan = i;
			}
		}
	}

	return idxScan;
}

float OverlayBScan::getDistance(BScanLine line, CPoint point)
{
	Point pt1 = line.pt1;
	Point pt2 = line.pt2;

	// ax + by + c = 0
	float a = (pt1.Y - pt2.Y);
	float b = (pt2.X - pt1.X);
	float c = (pt1.X * pt2.Y) - (pt2.X * pt1.Y);

	// distance = |ax + by + c| / (a^2 + b^2) ^ (1/2)
	return fabs((a * point.x) + (b * point.y) + c) / sqrt((a * a) + (b * b));
}

float OverlayBScan::getDistanceHandle(BScanLine line, CPoint point)
{
	Point ptHandle = line.ptH;
	float a = fabs(ptHandle.X - point.x);
	float b = fabs(ptHandle.Y - point.y);

	return sqrt((a * a) + (b * b));
}

float OverlayBScan::getDistanceHandle2(BScanLine line, CPoint point)
{
	Point ptHandle = line.ptH2;
	float a = fabs(ptHandle.X - point.x);
	float b = fabs(ptHandle.Y - point.y);

	return sqrt((a * a) + (b * b));
}

CRect OverlayBScan::getScanPatternRect(void)
{
	CRect rtClient;
	GetClientRect(&rtClient);

	CPoint ptClientCenter;
	ptClientCenter = rtClient.CenterPoint();

	CRect rtPattern;
	rtPattern.left = ptClientCenter.x + LONG(m_scanPattern.getStartX() * m_pixelPerMM);
	rtPattern.right = ptClientCenter.x + LONG(m_scanPattern.getCloseX() * m_pixelPerMM);
	rtPattern.top = ptClientCenter.y + LONG(m_scanPattern.getStartY() * m_pixelPerMM);
	rtPattern.bottom = ptClientCenter.y + LONG(m_scanPattern.getCloseY() * m_pixelPerMM);

	return rtPattern;
}

void OverlayBScan::setScanPoint(POINT pt)
{
	m_scanPoint = pt;
}

POINT OverlayBScan::getScanPoint()
{
	return m_scanPoint;
}