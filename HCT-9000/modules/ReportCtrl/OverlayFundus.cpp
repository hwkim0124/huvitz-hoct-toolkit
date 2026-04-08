// OverlayFundus.cpp : implementation file
//

#include "stdafx.h"
#include "ReportCtrl.h"
#include "OverlayFundus.h"


// OverlayFundus

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(OverlayFundus, CWnd)

OverlayFundus::OverlayFundus()
	: m_showEnfaceImage(false), m_showThicknessMap(false), m_showThicknessChart(false)
	, m_showScanPattern(false), m_showScanLine(false), m_scanPoint({ -1, -1 })
	, m_showDeviationMap(false), m_target(ANALYSIS), m_showScanPatternCenterCross(false), m_showAngioScanIndex(false), m_isHorizontal(true)
{
	DrawThicknessMap::setTransparency(0.5f);
	DrawRadiusMap::setTransparency(0.5f);
	//DrawDeviationMap::setTransparency(0.5f);
	DrawEtdrsChart::showChartColor(false);
	DrawRnflChart::showChartColor(false);
	DrawGccChart::showChartColor(false);
}

OverlayFundus::~OverlayFundus()
{
}

void OverlayFundus::initialize(EyeSide side)
{
	DrawRnflChart::initialize(side);
	DrawGccChart::initialize(side);
	DrawEtdrsChart::initialize(side);

	return;
}

void OverlayFundus::setCompareType(EyeSide side) 
{
	DrawRnflChart::setCompareType(side);
	DrawGccChart::setCompareType(side);
	DrawEtdrsChart::setCompareType(side);
}

void OverlayFundus::showEnfaceImage(bool show)
{
	m_showEnfaceImage = show;
	return;
}

void OverlayFundus::showDeviationImage(bool show)
{
	m_showDeviationMap = show;
	return;
}

void OverlayFundus::showThicknessMap(bool show)
{
	m_showThicknessMap = show;
	return;
}

void OverlayFundus::showThicknessChart(bool show)
{
	m_showThicknessChart = show;
	return;
}

void OverlayFundus::showRadiusMap(bool show)
{
	m_showRadiusMap = show;
	return;
}

void OverlayFundus::showScanPattern(bool show)
{
	m_showScanPattern = show;
	return;
}

void OverlayFundus::showScanLine(bool show)
{
	m_showScanLine = show;
	return;
}

void OverlayFundus::showScanPatternCenterCross(bool show)
{
	m_showScanPatternCenterCross = show;
}

void OverlayFundus::showAngioScanIndex(bool show, bool isHorizontal, int scanLines, int scanIdx)
{
	m_showAngioScanIndex = show;
	m_isHorizontal = isHorizontal;
	m_scanLines = scanLines;
	m_scanIdx = scanIdx;
}

void OverlayFundus::drawAngioScanIndex(CDC *pDC, CRect rect, OctScanPattern pattern, int imageIndex)
{
	CPen pen;
	CBrush brush;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 87, 79));
	brush.CreateStockObject(HOLLOW_BRUSH);

	// select brush and pen
	auto oldPen = pDC->SelectObject(&pen);
	auto oldBrsh = pDC->SelectObject(&brush);

	GetClientRect(&rect);

	CPoint ptCenter;
	ptCenter = rect.CenterPoint();

	int left = ptCenter.x + LONG(pattern.getStartX() * m_pixelPerMM);
	int right = ptCenter.x + LONG(pattern.getCloseX() * m_pixelPerMM);
	int top = ptCenter.y + LONG(pattern.getStartY() * m_pixelPerMM);
	int bottom = ptCenter.y + LONG(pattern.getCloseY() * m_pixelPerMM);
	int centerX = ptCenter.x + LONG(pattern.centerX() * m_pixelPerMM);
	int centerY = ptCenter.y + LONG(pattern.centerY() * m_pixelPerMM);
	int lines = pattern.getNumberOfScanLines();

	// draw
	pDC->Rectangle(left, top, right, bottom);

	if (m_isHorizontal) {
		int linePos = top + double(bottom - top) / (double)m_scanLines * (double)imageIndex;
		pDC->MoveTo(left, linePos);
		pDC->LineTo(right, linePos);

		// arrow
		pDC->MoveTo(right - 1, linePos);
		pDC->LineTo(right - 1 - 5, linePos - 5);
		pDC->MoveTo(right - 1, linePos);
		pDC->LineTo(right - 1 - 5, linePos + 5);
	}
	else {
		int linePos = left + double(right - left) / (double)m_scanLines * (double)imageIndex;
		pDC->MoveTo(linePos, top);
		pDC->LineTo(linePos, bottom);

		// arrow
		pDC->MoveTo(linePos, bottom - 1);
		pDC->LineTo(linePos - 5, bottom - 1 - 5);
		pDC->MoveTo(linePos, bottom - 1);
		pDC->LineTo(linePos + 5, bottom - 1 - 5);
	}

	// - finish
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrsh);

	pen.DeleteObject();
	brush.DeleteObject();
}

void OverlayFundus::setScanPattern(OctScanPattern pattern, target_t target)
{
	const int kMaxRadius = 180;

	m_scanPattern = pattern;
	m_scanLine.clear();
	m_scanLineHD.clear();
	m_target = target;

	CRect rect;
	GetClientRect(&rect);

	CPoint ptCenter;
	ptCenter = rect.CenterPoint();

	int left = ptCenter.x + LONG(pattern.getStartX() * m_pixelPerMM);
	int right = ptCenter.x + LONG(pattern.getCloseX() * m_pixelPerMM);
	int top = ptCenter.y + LONG(pattern.getStartY() * m_pixelPerMM);
	int bottom = ptCenter.y + LONG(pattern.getCloseY() * m_pixelPerMM);
	int centerX = ptCenter.x + LONG(pattern.centerX() * m_pixelPerMM);
	int centerY = ptCenter.y + LONG(pattern.centerY() * m_pixelPerMM);
	int lines = pattern.getNumberOfScanLines();
	int distHandle = kHandleSize / 2;

	// Prevent over sized scan line
	if (left < centerX - kMaxRadius) {
		left = centerX - kMaxRadius;
	}
	if (right > centerX + kMaxRadius) {
		right = centerX + kMaxRadius;
	}
	if (top < centerY - kMaxRadius) {
		top = centerY - kMaxRadius;
	}
	if (bottom > centerY + kMaxRadius) {
		bottom = centerY + kMaxRadius;
	}

	switch (pattern.getPatternName())
	{
	case OctGlobal::PatternName::MacularLine:
	case OctGlobal::PatternName::DiscLine:
	case OctGlobal::PatternName::AnteriorLine:
	case OctGlobal::PatternName::AnteriorFull:

	{
		ScanLine line;

		if (pattern.getDirection() == 0)
		{
			line.direction = 0;
			line.pt1.X = left;
			line.pt1.Y = centerY;
			line.pt2.X = right;
			line.pt2.Y = centerY;
			line.hasHandle = false;
		}
		else
		{
			line.direction = 1;
			line.pt1.X = centerX;
			line.pt1.Y = top;
			line.pt2.X = centerX;
			line.pt2.Y = bottom;
			line.hasHandle = false;
		}

		m_scanLine.push_back(line);
	}
	if (pattern.getPatternName() == OctGlobal::PatternName::AnteriorLine || pattern.getPatternName() == OctGlobal::PatternName::AnteriorFull)
	{
		const float angleDeg = pattern.getScanOffset().getScanAngle();
		if (std::fabs(angleDeg) > 0.001f)
		{
			const float kDegToRad = 3.14159265358979323846f / 180.0f;
			const float angleRad = angleDeg * kDegToRad;
			const float cosTheta = std::cos(angleRad);
			const float sinTheta = std::sin(angleRad);
			const float fCenterX = static_cast<float>(centerX);
			const float fCenterY = static_cast<float>(centerY);

			auto rotatePoint = [&](Gdiplus::Point point) -> Gdiplus::Point {
				const float dx = static_cast<float>(point.X) - fCenterX;
				const float dy = static_cast<float>(point.Y) - fCenterY;
				const float rx = dx * cosTheta - dy * sinTheta;
				const float ry = dx * sinTheta + dy * cosTheta;
				const int nx = static_cast<int>(std::lround(fCenterX + rx));
				const int ny = static_cast<int>(std::lround(fCenterY + ry));
				return Gdiplus::Point(nx, ny);
			};

			for (auto& scanLine : m_scanLine) {
				scanLine.pt1 = rotatePoint(scanLine.pt1);
				scanLine.pt2 = rotatePoint(scanLine.pt2);
				if (scanLine.hasHandle) {
					scanLine.ptH = rotatePoint(scanLine.ptH);
				}
			}
		}
	}
	break;

	case OctGlobal::PatternName::MacularAngio:
	case OctGlobal::PatternName::DiscAngio:
		setScanPatternAngio(pattern);
	break;

	case OctGlobal::PatternName::Macular3D:
	case OctGlobal::PatternName::Disc3D:
	case OctGlobal::PatternName::Anterior3D:
	case OctGlobal::PatternName::MacularDisc:
	{
		ScanLine line;

		if (pattern.getDirection() == 0)
		{
			float spaceY = pattern.getScanRangeY() * m_pixelPerMM / float(lines - 1);

			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
			{
				line.direction = 0;
				line.pt1.X = left;
				line.pt1.Y = top + int(i * spaceY);
				line.pt2.X = right;
				line.pt2.Y = top + int(i * spaceY);
				line.hasHandle = true;
				line.ptH.X = left - distHandle;
				line.ptH.Y = top + int(i * spaceY);
				m_scanLine.push_back(line);
			}

			// HD 
			line.direction = 0;
			line.pt1.X = left;
			line.pt1.Y = centerY;
			line.pt2.X = right;
			line.pt2.Y = centerY;
			line.hasHandle = true;
			line.ptH.X = left - distHandle;
			line.ptH.Y = centerY;
			m_scanLineHD.push_back(line);

			line.direction = 1;
			line.pt1.X = centerX;
			line.pt1.Y = top;
			line.pt2.X = centerX;
			line.pt2.Y = bottom;
			line.hasHandle = false;
			m_scanLineHD.push_back(line);
		}
		else
		{
			float spaceX = pattern.getScanRangeX() * m_pixelPerMM / float(lines - 1);

			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
			{
				line.direction = 1;
				line.pt1.X = left + int(i * spaceX);
				line.pt1.Y = top;
				line.pt2.X = left + int(i * spaceX);
				line.pt2.Y = bottom;
				line.hasHandle = true;
				line.ptH.X = left + int(i * spaceX);
				line.ptH.Y = top - distHandle;
				m_scanLine.push_back(line);
			}

			// HD 
			line.direction = 0;
			line.pt1.X = left;
			line.pt1.Y = centerY;
			line.pt2.X = right;
			line.pt2.Y = centerY;
			line.hasHandle = false;
			m_scanLineHD.push_back(line);

			line.direction = 1;
			line.pt1.X = centerX;
			line.pt1.Y = top;
			line.pt2.X = centerX;
			line.pt2.Y = bottom;
			line.hasHandle = true;
			line.ptH.X = centerX;
			line.ptH.Y = top - distHandle;
			m_scanLineHD.push_back(line);
		}


	}
	break;

	case OctGlobal::PatternName::MacularRaster:
	case OctGlobal::PatternName::DiscRaster:
	{
		ScanLine line;

		if (pattern.getDirection() == 0)
		{
			float spaceY = pattern.getScanRangeY() * m_pixelPerMM / float(lines - 1);

			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
			{
				line.direction = 0;
				line.pt1.X = left;
				line.pt1.Y = top + int(i * spaceY);
				line.pt2.X = right;
				line.pt2.Y = top + int(i * spaceY);
				line.hasHandle = true;
				line.ptH.X = left - distHandle;
				line.ptH.Y = top + int(i * spaceY);
				m_scanLine.push_back(line);
			}
		}
		else
		{
			float spaceX = pattern.getScanRangeX() * m_pixelPerMM / float(lines - 1);

			for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
			{
				line.direction = 1;
				line.pt1.X = left + int(i * spaceX);
				line.pt1.Y = top;
				line.pt2.X = left + int(i * spaceX);
				line.pt2.Y = bottom;
				line.hasHandle = true;
				line.ptH.X = left + int(i * spaceX);
				line.ptH.Y = top - distHandle;
				m_scanLine.push_back(line);
			}
		}
	}
	break;

	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
	{
		int radiusX = LONG(pattern.getScanRangeX() * m_pixelPerMM / 2);
		if (radiusX > kMaxRadius) {
			radiusX = kMaxRadius;
		}
		int radiusY = LONG(pattern.getScanRangeY() * m_pixelPerMM / 2);
		if (radiusY > kMaxRadius) {
			radiusY = kMaxRadius;
		}
	
		for (int i = 0; i < 12; i++)
		{
			float angle = i * 15;
			float radian = M_PI * angle / 180;

			ScanLine line;
			line.direction = 2;
			line.pt1.X = centerX - int(radiusX * cos(radian));
			line.pt1.Y = centerY - int(radiusY * sin(radian));
			line.pt2.X = centerX + int(radiusX * cos(radian));
			line.pt2.Y = centerY + int(radiusY * sin(radian));
			line.hasHandle = true;
			line.ptH.X = centerX - int((radiusX + distHandle) * cos(radian));
			line.ptH.Y = centerY - int((radiusY + distHandle) * sin(radian));
			m_scanLine.push_back(line);
		}
	}
	break;

	case OctGlobal::PatternName::Topography:
	{
		int radiusX = LONG(pattern.getScanRangeX() * m_pixelPerMM / 2);
		if (radiusX > kMaxRadius) {
			radiusX = kMaxRadius;
		}
		int radiusY = LONG(pattern.getScanRangeY() * m_pixelPerMM / 2);
		if (radiusY > kMaxRadius) {
			radiusY = kMaxRadius;
		}

		for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
		{
			float angle = i * (180.f / pattern.getNumberOfScanLines()); //11.25;
			float radian = M_PI * angle / 180;

			ScanLine line;
			line.direction = 2;
			line.pt1.X = centerX - int(radiusX * cos(radian));
			line.pt1.Y = centerY - int(radiusY * sin(radian));
			line.pt2.X = centerX + int(radiusX * cos(radian));
			line.pt2.Y = centerY + int(radiusY * sin(radian));
			line.hasHandle = true;
			line.ptH.X = centerX - int((radiusX + distHandle) * cos(radian));
			line.ptH.Y = centerY - int((radiusY + distHandle) * sin(radian));
			m_scanLine.push_back(line);
		}
	}
	break;

	case OctGlobal::PatternName::MacularCross:
	{
		float spaceMM = m_scanPattern.getScanLineSpace();
		if (spaceMM <= 0.0f) {
			spaceMM = 0.25f;
		}

		int space = int(spaceMM * m_pixelPerMM);

		for (int i = 0; i < 5; i++)
		{
			ScanLine line;
			line.direction = 0;
			line.pt1.X = left;
			line.pt1.Y = centerY + int((i - 2) * space);
			line.pt2.X = right;
			line.pt2.Y = centerY + int((i - 2) * space);
			line.hasHandle = true;
			line.ptH.X = left - distHandle;
			line.ptH.Y = centerY + int((i - 2) * space);
			m_scanLine.push_back(line);
		}
		for (int i = 5; i < 10; i++)
		{
			ScanLine line;
			line.direction = 1;
			line.pt1.X = centerX + int((i - 7) * space);
			line.pt1.Y = top;
			line.pt2.X = centerX + int((i - 7) * space);
			line.pt2.Y = bottom;
			line.hasHandle = true;
			line.ptH.X = centerX + int((i - 7) * space);
			line.ptH.Y = top - distHandle;
			m_scanLine.push_back(line);
		}
	}
	break;

	case OctGlobal::PatternName::MacularCircle:
	case OctGlobal::PatternName::DiscCircle:
	{
		ScanLine line;

		line.direction = 3;
		line.pt1.X = left;
		line.pt1.Y = top;
		line.pt2.X = right;
		line.pt2.Y = bottom;
		line.hasHandle = false;
		m_scanLine.push_back(line);
	}
	break;

	case OctGlobal::PatternName::DiscPoint:
	case OctGlobal::PatternName::AnteriorPoint:
	case OctGlobal::PatternName::AnteriorAngio:
		break;
	default:
		break;
	}

	return;
}

void OverlayFundus::setScanPatternAngio(OctScanPattern pattern)
{
	CRect rect;
	GetClientRect(&rect);

	CPoint ptCenter;
	ptCenter = rect.CenterPoint();

	int left = 0;
	int top = 0;
	int right = rect.right;
	int bottom = rect.bottom;
	int centerX = ptCenter.x;
	int centerY = ptCenter.y;
	int lines = pattern.getNumberOfScanLines();
	int distHandle = kHandleSize / 2;

	ScanLine line;

	if (pattern.getDirection() == 0)
	{
		float spaceY = rect.Width() / float(lines - 1);

		for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
		{
			line.direction = 0;
			line.pt1.X = left;
			line.pt1.Y = top + int(i * spaceY);
			line.pt2.X = right;
			line.pt2.Y = top + int(i * spaceY);
			line.hasHandle = true;
			line.ptH.X = left - distHandle;
			line.ptH.Y = top + int(i * spaceY);
			m_scanLine.push_back(line);
		}

		// HD 
		line.direction = 0;
		line.pt1.X = left;
		line.pt1.Y = centerY;
		line.pt2.X = right;
		line.pt2.Y = centerY;
		line.hasHandle = true;
		line.ptH.X = left - distHandle;
		line.ptH.Y = centerY;
		m_scanLineHD.push_back(line);

		line.direction = 1;
		line.pt1.X = centerX;
		line.pt1.Y = top;
		line.pt2.X = centerX;
		line.pt2.Y = bottom;
		line.hasHandle = false;
		m_scanLineHD.push_back(line);
	}
	else
	{
		float spaceX = rect.Width() / float(lines - 1);

		for (int i = 0; i < pattern.getNumberOfScanLines(); i++)
		{
			line.direction = 1;
			line.pt1.X = left + int(i * spaceX);
			line.pt1.Y = top;
			line.pt2.X = left + int(i * spaceX);
			line.pt2.Y = bottom;
			line.hasHandle = true;
			line.ptH.X = left + int(i * spaceX);
			line.ptH.Y = top - distHandle;
			m_scanLine.push_back(line);
		}

		// HD 
		line.direction = 0;
		line.pt1.X = left;
		line.pt1.Y = centerY;
		line.pt2.X = right;
		line.pt2.Y = centerY;
		line.hasHandle = false;
		m_scanLineHD.push_back(line);

		line.direction = 1;
		line.pt1.X = centerX;
		line.pt1.Y = top;
		line.pt2.X = centerX;
		line.pt2.Y = bottom;
		line.hasHandle = true;
		line.ptH.X = centerX;
		line.ptH.Y = top - distHandle;
		m_scanLineHD.push_back(line);
	}
}

void OverlayFundus::setWndPixelPer1MM(float pixel)
{
	m_pixelPerMM = pixel;

	return;
}

void OverlayFundus::clearEnfaceImage(void)
{
	m_imageEnface.clear();

	return;
}

void OverlayFundus::clearDeviationImage(void)
{
	clearDeviationMapData();

	return;
}

void OverlayFundus::setEnfaceImage(CvImage image)
{
	m_imageEnface = image;
	
	return;
}

void OverlayFundus::setDeviationImage(CvImage image, std::pair<int, int> mapSize,
	std::pair<int, int> center, std::pair<OcularLayerType, OcularLayerType> layers,
	float mapRangeRatio)
{
	setDeviationMapData(image.copyDataInFloats(), mapSize,
		make_pair(image.getWidth(), image.getHeight()), center, mapRangeRatio);

	m_layers = layers;

	return;
}

void OverlayFundus::setChartCenter(float cx, float cy)
{
	m_chartCenterX = cx;
	m_chartCenterY = cy;

	return;
}
void OverlayFundus::setChartCenterForMacularDisc(float discCenterX, float discCenterY, float macularCenterX, float macularCenterY)
{
	m_chartCenterDiscX = discCenterX;
	m_chartCenterDiscY = discCenterY;
	m_chartCenterMacularX = macularCenterX;
	m_chartCenterMacularY = macularCenterY;
}

BEGIN_MESSAGE_MAP(OverlayFundus, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// OverlayFundus message handlers


void OverlayFundus::OnPaint()
{
	CPaintDC dc(this);
	CRect rtPattern = getScanPatternRect();
	CRect rtDevMap = getScanDeviationMapRect(m_scanPattern);
	CPoint ptPatternCenter = getScanPatternCenter();

	if (m_showEnfaceImage)
	{
		drawEnfaceImage(&dc, rtPattern);
	}

	if (m_showDeviationMap) {
		drawDeviationMap(dc.GetSafeHdc(), rtDevMap, m_scanPattern.getPatternName(), m_layers);
	}

	if (m_showThicknessMap)
	{
		drawThicknessMap(&dc, rtPattern);
	}

	if (m_showRadiusMap)
	{
		drawRadiusMap(&dc, rtPattern);
	}

	if (m_showScanPattern)
	{
		drawScanPattern(&dc, rtPattern, m_scanPattern);
	}

	if (m_showScanLine)
	{
		drawScanLine(&dc, 1);
		drawScanLine(&dc, 0);
	}

	if (m_showThicknessChart) {
		drawThicknessChart(&dc, CRect(), m_pixelPerMM);
	}

	if (m_showAngioScanIndex) {
		drawAngioScanIndex(&dc, rtPattern, m_scanPattern, m_scanIdx);
	}

	drawScanPoint(&dc, m_scanPoint, Gdiplus::Color::White);

	return;
}


void OverlayFundus::drawOverlayForReport(CDC *pDC, CRect rtClient, float pixelPerMM)
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

	// enface
	if (m_showEnfaceImage &&
		(name == PatternName::Macular3D || name == PatternName::Disc3D ||
			name == PatternName::MacularDisc || name == PatternName::MacularAngio ||
			name == PatternName::DiscAngio)) {
		drawEnfaceImage(pDC, rtScan);
	}

	// deviation map
	if (m_showDeviationMap &&
		(name == PatternName::Macular3D || name == PatternName::Disc3D ||
			name == PatternName::MacularDisc)) {
		drawDeviationMap(pDC->GetSafeHdc(), rtScan, name, m_layers);
	}

	// radius map
	if (m_showRadiusMap &&
		name == PatternName::AnteriorRadial) {
		drawRadiusMap(pDC, rtScan);
	}

	// thickness map
	if (m_showThicknessMap &&
		(name == PatternName::Macular3D || name == PatternName::MacularRaster ||
			name == PatternName::MacularRadial || name == PatternName::Disc3D ||
			name == PatternName::DiscRaster || name == PatternName::DiscRadial ||
			name == PatternName::AnteriorRadial || name == PatternName::MacularDisc ||
			name == PatternName::MacularAngio || name == PatternName::DiscAngio)) {
		drawThicknessMap(pDC, rtScan);
	}

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

	// thickness chart
	if (m_showThicknessChart) {
		drawThicknessChart(pDC, rtScan, pixelPerMM);
	}

	// 1ptm
	drawScanPointForReport(pDC, Gdiplus::Color::White, rtScan);

	return;
}


void OverlayFundus::drawEnfaceImage(CDC *pDC, CRect rect_in)
{
	if (m_imageEnface.isEmpty())
	{
		return;
	}

	CRect rect;
	if (m_target == ANALYSIS) {
		rect = rect_in;
	}
	else {
		if (m_scanPattern.getPatternName() == PatternName::MacularAngio ||
			m_scanPattern.getPatternName() == PatternName::DiscAngio) {
			GetClientRect(rect);
		}
		else {
			rect = rect_in;
		}
	}

	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = m_imageEnface.createDIBitmap(hDC, true);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	int xSrc, ySrc, wSrc, hSrc;
	int xDst, yDst, wDst, hDst;
	xSrc = 0;
	ySrc = 0;
	wSrc = m_imageEnface.getWidth();
	hSrc = m_imageEnface.getHeight();
	xDst = rect.left;
	yDst = rect.top;
	wDst = rect.Width();
	hDst = rect.Height();

	SetStretchBltMode(hDC, HALFTONE);
	StretchBlt(hDC, xDst, yDst, wDst, hDst, hMemDC, xSrc, ySrc, wSrc, hSrc, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	DeleteObject(hbmp);

	return;
}

void OverlayFundus::drawThicknessChart(CDC *pDC, CRect rect, float pixelPerMM)
{
	if (!m_showThicknessChart) {
		return;
	}

	CRect rtClient;
	GetClientRect(rtClient);

	CPoint ptPatternCenter = getScanPatternCenter();

	CPoint ptChartCenter;
	if (rect.IsRectEmpty()) {
		ptChartCenter.x = ptPatternCenter.x + LONG(m_chartCenterX * pixelPerMM);
		ptChartCenter.y = ptPatternCenter.y + LONG(m_chartCenterY * pixelPerMM);
	}
	else {
		float fZoomRatioW = (float)rect.Width() / (float)rtClient.Width();
		float fZoomRatioH = (float)rect.Height() / (float)rtClient.Height();

		ptChartCenter.x = LONG((float)rtClient.CenterPoint().x * fZoomRatioW) +
			LONG(m_chartCenterX * pixelPerMM) + rect.left;
		ptChartCenter.y = LONG((float)rtClient.CenterPoint().y * fZoomRatioH) +
			LONG(m_chartCenterY * pixelPerMM) + rect.top;
	}

	if (m_chartType == ThickChartType::etdrs) {
		float radius = 3 * pixelPerMM;
		Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

		Gdiplus::Color colorGreen(0, 255, 0);
		drawEtdrsChart(pDC, ptChartCenter, radius, font, colorGreen, colorGreen, colorGreen);
	}
	else if (m_chartType == ThickChartType::gcc) {
		float radius = 2 * pixelPerMM;

		Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

		Gdiplus::Color colorGreen(0, 255, 0);
		drawGccChart(pDC, ptChartCenter, radius, font, colorGreen, colorGreen, colorGreen);
	}
	else if (m_chartType == ThickChartType::rnfl) {
		float radius = 1.73f * pixelPerMM * 1.1;

		Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

		Gdiplus::Color colorGreen(0, 255, 0);
		drawRnflChartQuad(pDC, ptChartCenter, radius, font, colorGreen, colorGreen, colorGreen);
	}
	else if (m_chartType == ThickChartType::etdrs_rnfl) {
		
		Gdiplus::Font font(_T("Calibri"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::Color colorGreen(0, 255, 0);

		CPoint ptChartCenterCustom;
		float radius;

		float fZoomRatioW = (float)rect.Width() / (float)rtClient.Width();
		float fZoomRatioH = (float)rect.Height() / (float)rtClient.Height();

		//rnfl
		radius = 1.73f * pixelPerMM * 1.1;

		ptChartCenterCustom.x = LONG((float)rtClient.CenterPoint().x * fZoomRatioW) +
			LONG(m_chartCenterDiscX * pixelPerMM) + rect.left;
		ptChartCenterCustom.y = LONG((float)rtClient.CenterPoint().y * fZoomRatioH) +
			LONG(m_chartCenterDiscY * pixelPerMM) + rect.top;

		drawRnflChartQuad(pDC, ptChartCenterCustom, radius, font, colorGreen, colorGreen, colorGreen);

		//etdrs
		radius = 3 * pixelPerMM;

		ptChartCenterCustom.x = LONG((float)rtClient.CenterPoint().x * fZoomRatioW) +
			LONG(m_chartCenterMacularX * pixelPerMM) + rect.left;
		ptChartCenterCustom.y = LONG((float)rtClient.CenterPoint().y * fZoomRatioH) +
			LONG(m_chartCenterMacularY * pixelPerMM) + rect.top;

		drawEtdrsChart(pDC, ptChartCenterCustom, radius, font, colorGreen, colorGreen, colorGreen);
	}
}

void OverlayFundus::drawScanPointForReport(CDC *pDC, Gdiplus::Color color, CRect rect)
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

void OverlayFundus::setScanIndex(int index, int idxLine)
{
	if (idxLine < m_scanLine.size())
	{
		m_scanLineSelected[index] = m_scanLine[idxLine];
	}

	return;
}


void OverlayFundus::setScanIndexHD(int index, int idxLine)
{
	if (idxLine < m_scanLineHD.size())
	{
		m_scanLineSelected[index] = m_scanLineHD[idxLine];
	}

	return;
}


void OverlayFundus::setChartType(ThickChartType type)
{
	m_chartType = type;

	return;
}

void OverlayFundus::drawScanPattern(CDC *pDC, CRect rect, OctScanPattern pattern)
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
		drawPatternRect(pDC, rect);
		break;
	case OctGlobal::PatternName::MacularAngio:
	case OctGlobal::PatternName::DiscAngio:
		//if (m_target == ANALYSIS) {
			drawPatternRect(pDC, rect);
		//}
		break;
	case OctGlobal::PatternName::DiscCircle:
		drawPatternCircle(pDC, rect);
		break;
	default:
		break;
	}

	return;
}

void OverlayFundus::drawScanPatternForReport(CDC *pDC, CRect rtScan, CRect rtClient,
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

void OverlayFundus::drawPatternLines(CDC *pDC)
{
	for (int i = 0; i < m_scanLine.size(); i++)
	{
		drawLine(pDC, m_scanLine[i]);
	}

	return;
}

void OverlayFundus::drawPatternLinesForReport(CDC *pDC, CRect rect, float pixelPerMM)
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

void OverlayFundus::drawPatternRect(CDC *pDC, CRect rect)
{
	CPen pen;
	CBrush brush;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 87, 79));
	brush.CreateStockObject(HOLLOW_BRUSH);

	// select brush and pen
	auto oldPen = pDC->SelectObject(&pen);
	auto oldBrsh = pDC->SelectObject(&brush);

	// draw
	pDC->Rectangle(rect);

	// center cross
	if (m_showScanPatternCenterCross)
	{
		auto cx = rect.CenterPoint().x;
		auto cy = rect.CenterPoint().y;

		pDC->MoveTo({ cx - kCenterCrossSize, cy });
		pDC->LineTo({ cx + kCenterCrossSize, cy });
		pDC->MoveTo({ cx, cy - kCenterCrossSize });
		pDC->LineTo({ cx, cy + kCenterCrossSize });
	}

	// restore graphic objects
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrsh);

	pen.DeleteObject();
	brush.DeleteObject();

	// 
	//Gdiplus::Graphics G(pDC->m_hDC);
	//Gdiplus::Pen pen(Gdiplus::Color(255, 87, 79), 1.0f);
	//G.DrawRectangle(&pen, rect.left, rect.top, rect.Width(), rect.Height());

	return;
}

void OverlayFundus::drawPatternCircle(CDC *pDC, CRect rect)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(255, 87, 79), 1.0f);
	G.DrawEllipse(&pen, rect.left, rect.top, rect.Width(), rect.Height());

	// center cross
	if (m_showScanPatternCenterCross)
	{
		auto cx = rect.CenterPoint().x;
		auto cy = rect.CenterPoint().y;

		G.DrawLine(&pen, cx - kCenterCrossSize, cy, cx + kCenterCrossSize, cy);
		G.DrawLine(&pen, cx, cy - kCenterCrossSize, cx, cy + kCenterCrossSize);
	}

	return;
}

void OverlayFundus::drawScanLine(CDC *pDC, int index)
{
	Gdiplus::Color color;
	color = (index == 0) ? kColorLine1 : kColorLine2;

	ScanLine line = m_scanLineSelected[index];

	if (line.direction == 3)
	{
		drawArrowCircle(pDC, line.pt1.X, line.pt1.Y, line.pt2.X, line.pt2.Y, color);
	}
	else
	{
		drawArrowLine(pDC, line, color);
	}

	return;
}

void OverlayFundus::drawScanLine(CDC *pDC, CRect rect, float pixelPerMM, int index)
{
	Gdiplus::Color color;
	color = (index == 0) ? kColorLine1 : kColorLine2;
	
	CRect rtClient;
	GetClientRect(&rtClient);

	// 기존 Client에 대한 좌표를 새로운 화면에 대한 좌표로..
	ScanLine line = m_scanLineSelected[index];
	int x1 = rect.CenterPoint().x + ((line.pt1.X - rtClient.CenterPoint().x) / m_pixelPerMM) * pixelPerMM;
	int y1 = rect.CenterPoint().y + ((line.pt1.Y - rtClient.CenterPoint().y) / m_pixelPerMM) * pixelPerMM;
	int x2 = rect.CenterPoint().x + ((line.pt2.X - rtClient.CenterPoint().x) / m_pixelPerMM) * pixelPerMM;
	int y2 = rect.CenterPoint().y + ((line.pt2.Y - rtClient.CenterPoint().y) / m_pixelPerMM) * pixelPerMM;

	if (line.direction == 3)
	{
		drawArrowCircle(pDC, x1, y1, x2, y2, color);
	}
	else
	{
		drawArrowLine(pDC, x1, y1, x2, y2, color);
	}

	return;
}

void OverlayFundus::drawCircle(CDC *pDC, CRect rect)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen pen(Gdiplus::Color(255, 87, 79), 1.0f);
	G.DrawEllipse(&pen, rect.left, rect.top, rect.Width(), rect.Height());

	return;
}

void OverlayFundus::drawArrowCircle(CDC *pDC, int x1, int y1, int x2, int y2, Gdiplus::Color color)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

//	Gdiplus::Point points[5] =
//	{
//		Gdiplus::Point(0, 0),
//		Gdiplus::Point(-3,-5),
//		Gdiplus::Point(0, -3),
//		Gdiplus::Point(3,-5),
//		Gdiplus::Point(0,0),
//	};
//
//	Gdiplus::GraphicsPath path;
//	path.AddPolygon(points, 5);
//
//	Gdiplus::Pen pen(color, 2.0f);
//	pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));

	Gdiplus::Pen pen(color, 2.0f);
	G.DrawEllipse(&pen, x1, y1, x2 - x1, y2 - y1);

	return;
}

void OverlayFundus::drawLine(CDC *pDC, int x1, int y1, int x2, int y2)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Color penColor(255, 87, 79);

	Gdiplus::Pen pen(penColor, 1.0f);

	G.DrawLine(&pen, x1, y1, x2, y2);

	return;
}

void OverlayFundus::drawArrowLine(CDC *pDC, int x1, int y1, int x2, int y2, Gdiplus::Color color)
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

void OverlayFundus::drawLine(CDC *pDC, ScanLine line)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);

	Gdiplus::Color penColor(255, 87, 79);
	Gdiplus::Pen pen(penColor, 1.0f);

	G.DrawLine(&pen, line.pt1, line.pt2);

	return;
}

void OverlayFundus::drawArrowLine(CDC *pDC, ScanLine line, Gdiplus::Color color)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	// arrow line
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
	G.DrawLine(&pen, line.pt1, line.pt2);

	// handle
	if (line.hasHandle)
	{
		Gdiplus::Point points[6] =
		{
			Gdiplus::Point(0,0),
			Gdiplus::Point(-6,-5),
			Gdiplus::Point(-6,-12),
			Gdiplus::Point(6,-12),
			Gdiplus::Point(6,-5),
			Gdiplus::Point(0,0),
		};
		Gdiplus::GraphicsPath path;
		path.AddPolygon(points, 6);

		Gdiplus::Pen pen(color, 2.0f);
		pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));
		G.DrawLine(&pen, line.ptH, line.pt1);
	}

	return;
}

void OverlayFundus::drawScanPoint(CDC *pDC, POINT pt, Gdiplus::Color color)
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

bool OverlayFundus::isChartArea(CPoint point)
{
	CPoint ptCenter = getChartCenter();
	LONG offsetX = point.x - ptCenter.x;
	LONG offsetY = point.y - ptCenter.y;

	float distance = sqrt(offsetX * offsetX + offsetY * offsetY) / m_pixelPerMM;


	if (m_chartType == ThickChartType::etdrs)
	{
		return (distance < 3.0f);
	}
	else if (m_chartType == ThickChartType::gcc)
	{
		return (distance < 2.0f);
	}
	else if (m_chartType == ThickChartType::rnfl)
	{
		return (distance < 1.73f);
	}
	else if (m_chartType == ThickChartType::etdrs_rnfl)
	{
		return (distance < 3.0f);
	}
	return false;
}

int OverlayFundus::getCloseScanIndex(CPoint point)
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
		}
	}

	return ret;
}

int OverlayFundus::getCloseScanLine(CPoint point, int idxDisp, function<float(ScanLine, CPoint)> fnGetDist)
{
	float minDist = 48.0f;		//넉넉하게 잡음...
	int idxScan = -1;
	int direction = m_scanLineSelected[idxDisp].direction;

	for (size_t i = 0; i < m_scanLine.size(); i++)
	{
		ScanLine line = m_scanLine[i];

		if (line.direction == direction)
		{
			float dist = 0;
			if (fnGetDist == nullptr) {
				dist = getDistanceHandle(line, point);
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

float OverlayFundus::getDistance(ScanLine line, CPoint point)
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

float OverlayFundus::getDistanceHandle(ScanLine line, CPoint point)
{
	Point ptHandle = line.ptH;
	float a = fabs(ptHandle.X - point.x);
	float b = fabs(ptHandle.Y - point.y);
	
	return sqrt((a * a) + (b * b));
}

CRect OverlayFundus::getScanPatternRect(void)
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

CRect OverlayFundus::getScanDeviationMapRect(OctScanPattern pattern)
{
	CRect rtClient;
	GetClientRect(&rtClient);

	CPoint ptCenter;
	ptCenter.x = rtClient.CenterPoint().x + LONG(pattern.centerX() * m_pixelPerMM);
	ptCenter.y = rtClient.CenterPoint().y + LONG(pattern.centerY() * m_pixelPerMM);

	CRect rtScan;
	rtScan.left = ptCenter.x - LONG(pattern.getScanRangeX() * m_pixelPerMM / 2);
	rtScan.right = ptCenter.x + LONG(pattern.getScanRangeX() * m_pixelPerMM / 2);
	rtScan.top = ptCenter.y - LONG(pattern.getScanRangeY() * m_pixelPerMM / 2);
	rtScan.bottom = ptCenter.y + LONG(pattern.getScanRangeY() * m_pixelPerMM / 2);

	return rtScan;
}

CPoint OverlayFundus::getScanPatternCenter()
{
	CRect rtClient;
	GetClientRect(&rtClient);

	CPoint ptClientCenter;
	ptClientCenter = rtClient.CenterPoint();

	CPoint ptPatternCenter;
	ptPatternCenter.x = ptClientCenter.x + LONG(m_scanPattern.centerX() * m_pixelPerMM);
	ptPatternCenter.y = ptClientCenter.y + LONG(m_scanPattern.centerY() * m_pixelPerMM);
	
	return ptPatternCenter;
}

CPoint OverlayFundus::getChartCenter(void)
{
	CPoint ptPatternCenter;
	ptPatternCenter = getScanPatternCenter();

	CPoint ptChartCenter;
	ptChartCenter.x = ptPatternCenter.x + LONG(m_chartCenterX * m_pixelPerMM);
	ptChartCenter.y = ptPatternCenter.y + LONG(m_chartCenterY * m_pixelPerMM);

	return ptChartCenter;
}

void OverlayFundus::setScanPoint(POINT pt)
{
	m_scanPoint = pt;
}

POINT OverlayFundus::getScanPoint()
{
	return m_scanPoint;
}