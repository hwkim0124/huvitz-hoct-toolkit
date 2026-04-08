#include "stdafx.h"
#include "DrawMeasureTool.h"


using namespace Gdiplus;

DrawMeasureTool::DrawMeasureTool()
	: m_drawingTool(DrawingTool::disable), m_showMeasure(false)
	, m_unitLength(_T("px")), m_unitArea(_T("px²"))
{
	clearMeasurePoints();

	m_drawTISAItemList = {
		make_pair(DrawingTool::tisa_500, DrawTISA(500.0)),
		make_pair(DrawingTool::tisa_750, DrawTISA(750.0)),
	};
}

DrawMeasureTool::~DrawMeasureTool()
{
}

void DrawMeasureTool::setUnit(CString unitLength, CString unitArea)
{
	m_unitLength = unitLength;
	m_unitArea = unitArea;
	return;
}

void DrawMeasureTool::setImageSize(int sx, int sy)
{
	m_imageSizeX = sx;
	m_imageSizeY = sy;
	return;
}

void DrawMeasureTool::setWindowSize(int sx, int sy)
{
	m_windowSizeX = sx;
	m_windowSizeY = sy;
	return;
}

void DrawMeasureTool::setPixelSize(double sx, double sy)
{
	m_pixelSizeX = sx;
	m_pixelSizeY = sy;
	return;
}

void DrawMeasureTool::showDrawing(bool show)
{
	m_showMeasure = show;
	return;
}

void DrawMeasureTool::setDrawingTool(DrawingTool tool)
{
	m_drawingTool = tool;

	m_drawLength.clear();
	m_drawAngle.clear();
	m_drawArea.clear();

	//for (auto& item : m_drawTISAItemList) {
	//	get<1>(item).clear();
	//}

	return;
}

auto DrawMeasureTool::getDrawingTool()->DrawingTool
{
	return m_drawingTool;
}

void DrawMeasureTool::clearMeasurePoints(void)
{
	m_vecLength.clear();
	m_vecAngle.clear();
	m_vecArea.clear();

	m_drawLength.clear();
	m_drawAngle.clear();
	m_drawArea.clear();
	m_drawCupDisc.clear();

	for (auto& item : m_drawTISAItemList) {
		get<1>(item).clear();
	}

	return;
}

void DrawMeasureTool::recalcMeasurePoints(double rateX, double rateY)
{
//	float rateX = (float)dst.cx / (float)src.cx;
//	float rateY = (float)dst.cy / (float)src.cy;

	for (size_t i = 0; i < m_vecLength.size(); i++)
	{
		TRACE("==================================\n");
		TRACE("%lf, %lf\n", rateX, rateY);
		TRACE("%d, %d\n", m_vecLength[i].points[0].X, INT((double)m_vecLength[i].points[0].X * rateX));
		TRACE("%d, %d\n", m_vecLength[i].points[0].Y, INT((double)m_vecLength[i].points[0].Y * rateY));
		TRACE("%d, %d\n", m_vecLength[i].points[1].X, INT((double)m_vecLength[i].points[1].X * rateX));
		TRACE("%d, %d\n", m_vecLength[i].points[1].Y, INT((double)m_vecLength[i].points[1].Y * rateY));

		m_vecLength[i].points[0].X = INT((double)m_vecLength[i].points[0].X * rateX);
		m_vecLength[i].points[0].Y = INT((double)m_vecLength[i].points[0].Y * rateY);
		m_vecLength[i].points[1].X = INT((double)m_vecLength[i].points[1].X * rateX);
		m_vecLength[i].points[1].Y = INT((double)m_vecLength[i].points[1].Y * rateY);
	}

	return;
}

void DrawMeasureTool::insertMeasurePoint(CPoint point)
{
	// 화면좌표 -> 이미지의 좌표
	int px = int(point.x * m_imageSizeX / (double)m_windowSizeX);
	int py = int(point.y * m_imageSizeY / (double)m_windowSizeY);

	insertMeasurePoint(px, py);

	return;
}

void DrawMeasureTool::insertMeasurePoint(int x, int y)
{
	// x, y는 이미지상의 좌표
	Gdiplus::PointF pointNew(x, y);

	if (m_drawingTool == DrawingTool::length)
	{
		m_drawLength.addPoints(pointNew);

		if (m_drawLength.isFull())
		{
			m_vecLength.push_back(m_drawLength);
			m_drawLength.clear();
		}
	}
	else if (m_drawingTool == DrawingTool::angle)
	{
		m_drawAngle.addPoints(pointNew);

		if(m_drawAngle.isFull())
		{
			m_vecAngle.push_back(m_drawAngle);
			m_drawAngle.clear();
		}
	}
	else if (m_drawingTool == DrawingTool::areaPolygon)
	{
		if (m_drawArea.size() < 3)
		{
			m_drawArea.addPoints(pointNew);
		}
		else
		{
			auto pointFront = m_drawArea.front();
			int distX = abs(pointFront.X - pointNew.X);
			int distY = abs(pointFront.Y - pointNew.Y);

			if (distX > 50 || distY > 50)
			{
				m_drawArea.addPoints(pointNew);
			}
			else
			{
				m_vecArea.push_back(m_drawArea);
				m_drawArea.clear();
			}
		}
	}
	else if (m_drawingTool == DrawingTool::tisa_500 || m_drawingTool == DrawingTool::tisa_750)
	{
		auto itr = find_if(m_drawTISAItemList.begin(), m_drawTISAItemList.end(),
			[this](auto x) { return get<0>(x) == m_drawingTool; });
		if (itr != m_drawTISAItemList.end()) {
			get<1>(*itr).addPoints(pointNew);
		}
	}

	return;
}

void DrawMeasureTool::clearCupDisc(void)
{
	m_drawCupDisc.clear();
	return;
}

void DrawMeasureTool::setTextColor(COLORREF textColor)
{
	m_textColor = textColor;
}

void DrawMeasureTool::setLastMoustPos(Gdiplus::PointF mousePt)
{
	m_lastMousePt = mousePt;
}

bool DrawMeasureTool::hasCupDiscPoints(void)
{
	return m_drawCupDisc.hasPoint;
}

bool DrawMeasureTool::getCupDiscPoint(CPoint point, bool &isDisc, int &index)
{
	Gdiplus::PointF pointWindow;
	pointWindow.X = point.x;
	pointWindow.Y = point.y;

	Gdiplus::PointF pointImage;
	pointImage = getImagePointFromWindowPoint(pointWindow);

	auto pointsCup = m_drawCupDisc.pointCup;
	for (size_t i = 0; i < pointsCup.size(); i++)
	{
		if (getPixelsBetweenPoint(pointImage, pointsCup[i]) < 50)
		{
			isDisc = false;
			index = (int)i;
			return true;
		}
	}

	auto pointDisc = m_drawCupDisc.pointDisc;
	for (size_t i = 0; i < pointDisc.size(); i++)
	{
		if (getPixelsBetweenPoint(pointImage, pointDisc[i]) < 50)
		{
			isDisc = true;
			index = (int)i;
			return true;
		}
	}

	return false;
}

void DrawMeasureTool::moveCupDiscPoint(CPoint point, bool isDisc, int index)
{
	Gdiplus::PointF pointWindow;
	pointWindow.X = point.x;
	pointWindow.Y = point.y;

	Gdiplus::PointF pointImage;
	pointImage = getImagePointFromWindowPoint(pointWindow);

	if (isDisc)
	{
		m_drawCupDisc.pointDisc[index] = pointImage;
	}
	else
	{
		m_drawCupDisc.pointCup[index] = pointImage;
	}

	return;
}

void DrawMeasureTool::createCupDiscPoints(CPoint point)
{
	double radiusCup = 30.0f;
	double radiusDisc = 60.0f;

	m_drawCupDisc.hasPoint = true;

	for (size_t i = 0; i < 8; i++)
	{
		Gdiplus::PointF pointNew;
		double radian = (i * 45) * (M_PI / 180);

		pointNew.X = point.x + INT(radiusCup * cos(radian));
		pointNew.Y = point.y - INT(radiusCup * sin(radian));
		pointNew.X = INT(pointNew.X * m_imageSizeX / (double)m_windowSizeX);
		pointNew.Y = INT(pointNew.Y * m_imageSizeY / (double)m_windowSizeY);
		m_drawCupDisc.addPointCup(pointNew);

		pointNew.X = point.x + INT(radiusDisc * cos(radian));
		pointNew.Y = point.y - INT(radiusDisc * sin(radian));
		pointNew.X = INT(pointNew.X * m_imageSizeX / (double)m_windowSizeX);
		pointNew.Y = INT(pointNew.Y * m_imageSizeY / (double)m_windowSizeY);
		m_drawCupDisc.addPointDisc(pointNew);
	}

	return;
}

void DrawMeasureTool::insertCupPoint(CPoint point)
{
	Gdiplus::PointF pointNew(point.x, point.y);

	m_drawCupDisc.addPointCup(pointNew);

	return;
}

void DrawMeasureTool::insertDiscPoint(CPoint point)
{
	Gdiplus::PointF pointNew(point.x, point.y);

	m_drawCupDisc.addPointDisc(pointNew);

	return;
}

Gdiplus::PointF DrawMeasureTool::getWindowPointFromImagePoint(Gdiplus::PointF point)
{
	Gdiplus::PointF ret;

	ret.X = int(point.X * m_windowSizeX / (double)m_imageSizeX);
	ret.Y = int(point.Y * m_windowSizeY / (double)m_imageSizeY);

	return ret;
}

Gdiplus::PointF DrawMeasureTool::getImagePointFromWindowPoint(Gdiplus::PointF point)
{
	Gdiplus::PointF ret;

	ret.X = int(point.X * m_imageSizeX / (double)m_windowSizeX);
	ret.Y = int(point.Y * m_imageSizeY / (double)m_windowSizeY);

	return ret;
}


void DrawMeasureTool::removeMeasurePoint(void)
{
	if (m_drawingTool == DrawingTool::length)
	{
		m_drawLength.clear();

		if (m_vecLength.size() > 0)
		{
			m_vecLength.pop_back();
		}
	}
	else if (m_drawingTool == DrawingTool::areaPolygon)
	{
		m_drawArea.clear();

		if (m_vecArea.size() > 0)
		{
			m_vecArea.pop_back();
		}
	}
	else if (m_drawingTool == DrawingTool::tisa_500 || m_drawingTool == DrawingTool::tisa_750)
	{
		auto itr = find_if(m_drawTISAItemList.begin(), m_drawTISAItemList.end(),
			[this](auto x) { return get<0>(x) == m_drawingTool; });
		if (itr != m_drawTISAItemList.end()) {
			get<1>(*itr).clear();
		}
	}

	return;
}

void DrawMeasureTool::removeMeasureAngle(void)
{
	m_drawAngle.clear();

	if (m_vecAngle.size() > 0)
	{
		m_vecAngle.pop_back();
	}
}

void DrawMeasureTool::drawMeasure(CDC *pDC, CRect rect, CRect rcRoi)
{
	if (!m_showMeasure) {
		return;
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	// length
	if (drawPoints(&G, rect, m_drawLength.points, rcRoi)) {
		for (size_t i = 0; i < m_vecLength.size(); i++) {
			if (drawLines(&G, rect, m_vecLength[i].points, rcRoi) &&
				drawPoints(&G, rect, m_vecLength[i].points, rcRoi)) {
				drawLength(&G, rect, m_vecLength[i].points);
			}
		}
	}

	// angle
	if (drawPoints(&G, rect, m_drawAngle.points, rcRoi)) {
		if (drawLines(&G, rect, m_drawAngle.points, rcRoi)) {
			for (size_t i = 0; i < m_vecAngle.size(); i++) {
				if (drawPoints(&G, rect, m_vecAngle[i].points, rcRoi) &&
					drawLines(&G, rect, m_vecAngle[i].points, rcRoi)) {
					drawAngle(&G, rect, m_vecAngle[i].points);
				}
			}
		}
	}

	// area
	if (drawPoints(&G, rect, m_drawArea.points, rcRoi)) {
		if (drawLines(&G, rect, m_drawArea.points, rcRoi)) {
			for (size_t i = 0; i < m_vecArea.size(); i++) {
				if (drawPolygon(&G, rect, m_vecArea[i].points, rcRoi) &&
					drawPoints(&G, rect, m_vecArea[i].points, rcRoi)) {
					drawArea(&G, rect, m_vecArea[i].points);
				}
			}
		}
	}

	drawCupDiscRatio(&G, rect, m_drawCupDisc, rcRoi);

	// tisa
	for (auto& item : m_drawTISAItemList) {
		drawMeasureTISA(get<1>(item), &G, rect, rcRoi);
	}

	return;
}

bool DrawMeasureTool::drawLines(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> points, CRect rcRoi)
{
	// image points -> window points
	vector <Gdiplus::PointF> pointsWindow;

	for (size_t i = 0; i < points.size(); i++)
	{
		INT px = rect.left + int(points[i].X * rect.Width() / (double)m_imageSizeX);
		INT py = rect.top + int(points[i].Y * rect.Height() / (double)m_imageSizeY);

		if (!rcRoi.IsRectEmpty() && !rcRoi.PtInRect({ px, py })) {
			return false;
		}

		pointsWindow.push_back(Gdiplus::PointF(px, py));
	}

	// draw polygon
	Gdiplus::Pen pen(
		Gdiplus::Color(GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)),
		2.0f);
	G->DrawLines(&pen, pointsWindow.data(), (INT)pointsWindow.size());
	
	return true;
}

bool DrawMeasureTool::drawPolygon(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> points, CRect rcRoi, bool fill)
{
	// image points -> window points
	vector <Gdiplus::PointF> pointsWindow;

	for (size_t i = 0; i < points.size(); i++)
	{
		INT px = rect.left + int(points[i].X * rect.Width() / (double)m_imageSizeX);
		INT py = rect.top + int(points[i].Y * rect.Height() / (double)m_imageSizeY);

		if (!rcRoi.IsRectEmpty() && !rcRoi.PtInRect({ px, py })) {
			return false;
		}

		pointsWindow.push_back(Gdiplus::PointF(px, py));
	}

	// draw polygon
	if (fill) {
		Gdiplus::SolidBrush brush(Gdiplus::Color(90, GetRValue(m_textColor),
			GetGValue(m_textColor), GetBValue(m_textColor)));
		G->FillPolygon(&brush, pointsWindow.data(), pointsWindow.size());
	}
	//
	Gdiplus::Pen pen(
		Gdiplus::Color(GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)),
		2.0f);
	G->DrawPolygon(&pen, pointsWindow.data(), (INT)pointsWindow.size());

	return true;
}

bool DrawMeasureTool::drawCloseCurve(Gdiplus::Graphics *G, CRect rect,
	vector <Gdiplus::PointF> points, Gdiplus::Color color, CRect rcRoi)
{
	// image points -> window points
	vector <Gdiplus::PointF> pointsWindow;

	for (size_t i = 0; i < points.size(); i++)
	{
		INT px = rect.left + int(points[i].X * rect.Width() / (double)m_imageSizeX);
		INT py = rect.top + int(points[i].Y * rect.Height() / (double)m_imageSizeY);

		if (!rcRoi.IsRectEmpty() && !rcRoi.PtInRect({ px, py })) {
			return false;
		}

		pointsWindow.push_back(Gdiplus::PointF(px, py));
	}

	// draw closed curve
	Gdiplus::Pen pen(color, 2.0f);
	G->DrawClosedCurve(&pen, pointsWindow.data(), (INT)pointsWindow.size());

	return true;
}

void DrawMeasureTool::drawLength(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> points)
{
	// contents
	CString text;
	text.Format(_T("%.2f%s"), getLengthOfLine(points[0], points[1]), m_unitLength);

	// ui
	// - text
	// -: font
	Gdiplus::SolidBrush brushText(Gdiplus::Color(
		GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));
	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	// -: size
	const float kScaleRatioX = (float)rect.Width() / (float)m_imageSizeX;
	const float kScaleRatioY = (float)rect.Height() / (float)m_imageSizeY;
	// -: text bounds
	const float kOffsetRatio = sqrtf(pow(kScaleRatioX, 2) + pow(kScaleRatioY, 2));
	auto textRect = getLabelRectForLength(G, make_pair(points[0], points[1]), text,
		&font, kOffsetRatio);
	// -: position
	auto x = rect.left + textRect.X * kScaleRatioX;
	auto y = rect.top + textRect.Y * kScaleRatioY;

	if (x + textRect.Width > rect.right) {
		x = rect.right - textRect.Width - 5;
	}
	else if (x < 0){
		x = 5;
	}
	if (y + textRect.Height > rect.bottom) {
		y = rect.bottom - textRect.Height - 5;
	}
	else if (y < 0) {
		y = 5;
	}

	auto w = textRect.Width;
	auto h = textRect.Height;
	// -: draw
	G->DrawRectangle(&Gdiplus::Pen(&brushText), Gdiplus::Rect(x, y, w, h));
	G->DrawString(text, -1, &font, Gdiplus::PointF(x, y), &brushText);
	// - connection
	int cxOnLine = rect.left + (points[0].X + points[1].X) / 2 * kScaleRatioX;
	int cyOnLine = rect.top + (points[0].Y + points[1].Y) / 2 * kScaleRatioY;
	int nearestXOnTextBound = abs(cxOnLine - x) < abs(cxOnLine - (x + w)) ? x : x + w;
	int nearestYOnTextBound = abs(cyOnLine - y) < abs(cyOnLine - (y + h)) ? y : y + h;
	G->DrawLine(&Gdiplus::Pen(&brushText), cxOnLine, cyOnLine, nearestXOnTextBound,
		nearestYOnTextBound);
}

void DrawMeasureTool::drawAngle(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> points)
{
	// contents
	CString text;
	text.Format(_T("%.1f˚"), getAngleOfPoints(points[0], points[1], points[2]));

	// ui
	// - text
	// -: font
	Gdiplus::SolidBrush brushText(Gdiplus::Color(
		GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));
	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	// -: size
	const float kScaleRatioX = (float)rect.Width() / (float)m_imageSizeX;
	const float kScaleRatioY = (float)rect.Height() / (float)m_imageSizeY;
	// -: text bounds
	auto textRect = getLabelRectForPolygon(G, points, text, &font);
	// -: position
	auto x = rect.left + textRect.X * kScaleRatioX;
	auto y = rect.top + textRect.Y * kScaleRatioY;

	if (x + textRect.Width > rect.right) {
		x = rect.right - textRect.Width - 5;
	}
	if (y + textRect.Height > rect.bottom) {
		y = rect.bottom - textRect.Height - 5;
	}

	auto w = textRect.Width;
	auto h = textRect.Height;
	// -: draw
	G->DrawRectangle(&Gdiplus::Pen(&brushText), Gdiplus::Rect(x, y, w, h));
	G->DrawString(text, -1, &font, Gdiplus::PointF(x, y), &brushText);
	// - connection
	int cxOnLine = rect.left + points[1].X * kScaleRatioX;
	int cyOnLine = rect.top + points[1].Y * kScaleRatioY;
	int nearestXOnTextBound = abs(cxOnLine - x) < abs(cxOnLine - (x + w)) ? x : x + w;
	int nearestYOnTextBound = abs(cyOnLine - y) < abs(cyOnLine - (y + h)) ? y : y + h;
	G->DrawLine(&Gdiplus::Pen(&brushText), cxOnLine, cyOnLine, nearestXOnTextBound,
		nearestYOnTextBound);
}

void DrawMeasureTool::drawArea(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> points)
{
	// contents
	CString text;
	text.Format(_T("%.2f%s"), getAreaOfPolygon(points), m_unitArea);

	// ui
	// - text
	// -: font
	Gdiplus::SolidBrush brushText(Gdiplus::Color(
		GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));
	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	// -: size
	const float kScaleRatioX = (float)rect.Width() / (float)m_imageSizeX;
	const float kScaleRatioY = (float)rect.Height() / (float)m_imageSizeY;
	// -: text bounds
	auto textRect = getLabelRectForPolygon(G, points, text, &font);
	// -: position
	auto x = rect.left + textRect.X * kScaleRatioX;
	auto y = rect.top + textRect.Y * kScaleRatioY;
	auto w = textRect.Width;
	auto h = textRect.Height;
	// -: draw
	G->DrawRectangle(&Gdiplus::Pen(&brushText), Gdiplus::Rect(x, y, w, h));
	G->DrawString(text, -1, &font, Gdiplus::PointF(x, y), &brushText);
	// - connection
	int cxOnLine = rect.left + points[0].X * kScaleRatioX;
	int cyOnLine = rect.top + points[0].Y * kScaleRatioY;
	int nearestXOnTextBound = abs(cxOnLine - x) < abs(cxOnLine - (x + w)) ? x : x + w;
	int nearestYOnTextBound = abs(cyOnLine - y) < abs(cyOnLine - (y + h)) ? y : y + h;
	G->DrawLine(&Gdiplus::Pen(&brushText), cxOnLine, cyOnLine, nearestXOnTextBound,
		nearestYOnTextBound);
}

void DrawMeasureTool::drawCupDiscRatio(Gdiplus::Graphics *G, CRect rect, DrawCupDisc cupDisc,
	CRect rcRoi)
{
	if (!hasCupDiscPoints()) {
		return;
	}

	if (!drawCloseCurve(G, rect, cupDisc.pointDisc, Gdiplus::Color(0, 255, 0), rcRoi)) {
		return;
	}

	if (!drawPoints(G, rect, cupDisc.pointDisc, rcRoi)) {
		return;
	}

	if (!drawCloseCurve(G, rect, cupDisc.pointCup, Gdiplus::Color(255, 255, 0), rcRoi)) {
		return;
	}

	if (!drawPoints(G, rect, cupDisc.pointCup, rcRoi)) {
		return;
	}

	// contents
	float areaDisc = getAreaOfPolygon(cupDisc.pointDisc);
	float areaCup = getAreaOfPolygon(cupDisc.pointCup);

	CString text = _T("");
	text.Format(_T("%.2f"), areaCup / areaDisc);

	// ui
	// - font
	// -: font
	Gdiplus::SolidBrush brushText(Gdiplus::Color(
		GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));
	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	// -: size
	const float kScaleRatioX = (float)rect.Width() / (float)m_imageSizeX;
	const float kScaleRatioY = (float)rect.Height() / (float)m_imageSizeY;
	// -: text bounds
	auto textRect = getLabelRectForPolygon(G, cupDisc.pointDisc, text, &font);
	// -: position
	auto x = rect.left + textRect.X * kScaleRatioX;
	auto y = rect.top + textRect.Y * kScaleRatioY;
	auto w = textRect.Width;
	auto h = textRect.Height;
	// -: draw
	G->DrawRectangle(&Gdiplus::Pen(&brushText), Gdiplus::Rect(x, y, w, h));
	G->DrawString(text, -1, &font, Gdiplus::PointF(x, y), &brushText);
	// - connection
	int lastIndex = cupDisc.pointDisc.size() - 1;
	int cxOnLine = rect.left + cupDisc.pointDisc[lastIndex].X * kScaleRatioX;
	int cyOnLine = rect.top + cupDisc.pointDisc[lastIndex].Y * kScaleRatioY;
	int nearestXOnTextBound = abs(cxOnLine - x) < abs(cxOnLine - (x + w)) ? x : x + w;
	int nearestYOnTextBound = abs(cyOnLine - y) < abs(cyOnLine - (y + h)) ? y : y + h;
	G->DrawLine(&Gdiplus::Pen(&brushText), cxOnLine, cyOnLine, nearestXOnTextBound,
		nearestYOnTextBound);

	return;
}

bool DrawMeasureTool::drawPoints(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> points,
	CRect rcRoi)
{
	for (size_t i = 0; i < points.size(); i++)
	{
		drawPoint(G, rect, points[i], rcRoi);
	}

	return true;
}

bool DrawMeasureTool::drawPoint(Gdiplus::Graphics *G, CRect rect, Gdiplus::PointF point, CRect rcRoi)
{
	Gdiplus::SolidBrush brush1(Gdiplus::Color(255, 255, 255));
	Gdiplus::SolidBrush brush2(
		Gdiplus::Color(GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));

	INT px = rect.left + int(point.X * rect.Width() / (double)m_imageSizeX);
	INT py = rect.top + int(point.Y * rect.Height() / (double)m_imageSizeY);

	if (!rcRoi.IsRectEmpty() && !rcRoi.PtInRect({ px, py })) {
		return false;
	}

	G->FillEllipse(&brush1, INT(px - 4), INT(py - 4), (INT)9, (INT)9);
	G->FillEllipse(&brush2, INT(px - 2), INT(py - 2), (INT)5, (INT)5);

	return true;
}

bool DrawMeasureTool::drawCircle(Gdiplus::Graphics *G, CRect rect, Gdiplus::PointF center, int radius, CRect rcRoi)
{
	Gdiplus::Pen pen(Gdiplus::Color(255, 255, 255), 2);

	INT px = rect.left + int(center.X * rect.Width() / (double)m_imageSizeX);
	INT py = rect.top + int(center.Y * rect.Height() / (double)m_imageSizeY);
	auto iRadius = int(radius * rect.Height() / (double)m_imageSizeY);

	if (!rcRoi.IsRectEmpty() && !rcRoi.PtInRect({ px, py })) {
		return false;
	}

	auto x = px - iRadius;
	auto y = py - iRadius;
	auto w = 2 * iRadius;
	auto h = 2 * iRadius;

	G->DrawEllipse(&pen, x, y, w, h);

	return true;
}

void DrawMeasureTool::drawMeasureTISA(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi)
{
	if (target.size() == 1) {
		drawMeasureTISA_1(target, G, rect, rcRoi);
	}
	else if (target.size() == 2) {
		drawMeasureTISA_2(target, G, rect, rcRoi);
	}
	else if (target.size() == 3) {
		drawMeasureTISA_3(target, G, rect, rcRoi);
	}
	else if (target.size() == 4) {
		drawMeasureTISA_4(target, G, rect, rcRoi);
	}
}

void DrawMeasureTool::drawMeasureTISA_1(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi)
{
	Gdiplus::PointF mousePos((double)m_imageSizeX / rect.Width() * m_lastMousePt.X,
		(double)m_imageSizeY / rect.Height() * m_lastMousePt.Y);

	auto radius = getCurRangePxTISA(target);
	auto& scleralSpurItem = target.getItem(DrawTISA::PointType::ScleralSpur);
	auto ptOnScleralSpur = get<2>(scleralSpurItem);
	get<1>(scleralSpurItem) = true;

	// circle
	drawCircle(G, rect, ptOnScleralSpur, radius, rcRoi);

	// center
	drawPoint(G, rect, ptOnScleralSpur, rcRoi);

	// line : not support because of touch interface
	//{
	//	auto angle = atan2(mousePos.Y - ptOnScleralSpur.Y, mousePos.X - ptOnScleralSpur.X);
	//	auto projItem = getPtOnCircleTISA(rect, ptOnScleralSpur, angle, radius);
	//	auto ptOnCircle = get<0>(projItem);

	//	vector<Gdiplus::PointF> line = {
	//		ptOnScleralSpur, ptOnCircle
	//	};

	//	drawLines(G, rect, line, rcRoi);
	//	drawPoint(G, rect, ptOnCircle, rcRoi);
	//}
}
void DrawMeasureTool::drawMeasureTISA_2(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi)
{
	auto radius = getCurRangePxTISA(target);
	auto scleralSpurItem = target.getItem(DrawTISA::PointType::ScleralSpur);
	auto ptOnScleralSpur = get<2>(scleralSpurItem);

	auto& pointOnCorneaItem = target.getItem(DrawTISA::PointType::PointOnCornea);
	auto ptOnCornea = get<2>(pointOnCorneaItem);

	auto angle = atan2(ptOnCornea.Y - ptOnScleralSpur.Y, ptOnCornea.X - ptOnScleralSpur.X);
	auto projItem = getPtOnCircleTISA(rect, ptOnScleralSpur, angle, radius);
	auto ptOnCircle = get<0>(projItem);
	auto angleProj = get<1>(projItem);

	if (!get<1>(pointOnCorneaItem)) {
		get<2>(pointOnCorneaItem) = ptOnCircle;
		get<1>(pointOnCorneaItem) = true;
	}

	auto perpLineItem = getPerpendicularLineTISA(rect, ptOnCircle, angleProj);
	auto linePerpendicacular = get<0>(perpLineItem);
	auto aPerp = get<1>(perpLineItem);
	auto bPerp = get<2>(perpLineItem);

	// line on cornea
	vector<Gdiplus::PointF> lineOnCornea = {
		ptOnScleralSpur, ptOnCircle
	};

	// point on iris : not support because of touch interface
	//Gdiplus::PointF mousePos((double)m_imageSizeX / rect.Width() * m_lastMousePt.X,
	//	(double)m_imageSizeY / rect.Height() * m_lastMousePt.Y);
	//auto ptOnIris = getPtOnIrisTISA(mousePos, ptOnCircle, aPerp, bPerp);

	//{
	//	Gdiplus::SolidBrush brushText(Gdiplus::Color(
	//		GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));
	//	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	//	CString szLog;
	//	szLog.Format(_T("[%.2f, %.2f]"), ptOnCircle.X, ptOnCircle.Y);

	//	G->DrawString(szLog, -1, &font, Gdiplus::PointF(300, 50), &brushText);
	//}

	// draw
	// - line on cornea
	drawLines(G, rect, lineOnCornea, rcRoi);
	// - perpendicular line
	drawLines(G, rect, linePerpendicacular, rcRoi);
	// - pt on scleral spur
	drawPoint(G, rect, ptOnScleralSpur, rcRoi);
	// - pt on cornea
	drawPoint(G, rect, ptOnCircle, rcRoi);
	// - pt on iris : not support because of touch interface
	//drawPoint(G, rect, ptOnIris, rcRoi);
}

void DrawMeasureTool::drawMeasureTISA_3(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi)
{
	auto scleralSpurItem = target.getItem(DrawTISA::PointType::ScleralSpur);
	auto ptOnScleralSpur = get<2>(scleralSpurItem);

	auto pointOnCorneaItem = target.getItem(DrawTISA::PointType::PointOnCornea);
	auto ptOnCornea = get<2>(pointOnCorneaItem);

	auto& pointOnIrisForRangeItem = target.getItem(DrawTISA::PointType::PointOnIrisForRange);
	auto clickedPtOnIrisForRange = get<2>(pointOnIrisForRangeItem);

	// confirm the point on iris for range
	auto radius = getCurRangePxTISA(target);
	auto angle = atan2(ptOnCornea.Y - ptOnScleralSpur.Y, ptOnCornea.X - ptOnScleralSpur.X);
	auto projItem = getPtOnCircleTISA(rect, ptOnScleralSpur, angle, radius);
	auto ptOnCircle = get<0>(projItem);
	auto angleProj = get<1>(projItem);

	auto perpLineItemForPtOnCornea = getPerpendicularLineTISA(rect, ptOnCornea, angleProj);
	auto aPerpForPtOnCornea = get<1>(perpLineItemForPtOnCornea);
	auto bPerpForPtOnCornea = get<2>(perpLineItemForPtOnCornea);

	auto ptOnIrisForRange = getPtOnIrisTISA(clickedPtOnIrisForRange, ptOnCornea, aPerpForPtOnCornea, bPerpForPtOnCornea);
	if (!get<1>(pointOnIrisForRangeItem)) {
		get<2>(pointOnIrisForRangeItem) = ptOnIrisForRange;
		get<1>(pointOnIrisForRangeItem) = true;
	}

	auto perpLineItemForScleralSpur = getPerpendicularLineTISA(rect, ptOnScleralSpur, angleProj);
	auto aPerpForScleralSpur = get<1>(perpLineItemForScleralSpur);
	auto bPerpForScleralSpur = get<2>(perpLineItemForScleralSpur);

	// guide line
	Gdiplus::PointF mousePos((double)m_imageSizeX / rect.Width() * m_lastMousePt.X,
		(double)m_imageSizeY / rect.Height() * m_lastMousePt.Y);
	if (mousePos.X - ptOnIrisForRange.X != 0) {
		// - between ptOnIrisForRange and mousePos : not support because of touch interface
		//auto aForGuideLineOnIris = (mousePos.Y - ptOnIrisForRange.Y) / (mousePos.X - ptOnIrisForRange.X);
		//auto bForGuideLineOnIris = ptOnIrisForRange.Y - aForGuideLineOnIris * ptOnIrisForRange.X;
		//vector<Gdiplus::PointF> guideLineOnIris = {
		//	Gdiplus::PointF(0, bForGuideLineOnIris),
		//	Gdiplus::PointF(m_imageSizeX, aForGuideLineOnIris * m_imageSizeX + bForGuideLineOnIris),
		//};
		//drawLines(G, rect, guideLineOnIris, rcRoi);
	}

	// draw
	// - guide line between ptOnIrisForScleralSpur and ptOnScleralSpur
	vector<Gdiplus::PointF> guideLinePerpendicular = {
		Gdiplus::PointF(0, bPerpForScleralSpur),
		Gdiplus::PointF(m_imageSizeX, aPerpForScleralSpur * m_imageSizeX + bPerpForScleralSpur),
	};
	drawLines(G, rect, guideLinePerpendicular, rcRoi);
	// - pt on scleral spur
	drawPoint(G, rect, ptOnScleralSpur, rcRoi);
	// - pt on cornea
	drawPoint(G, rect, ptOnCircle, rcRoi);
	// - pt on iris
	drawPoint(G, rect, ptOnIrisForRange, rcRoi);
}

void DrawMeasureTool::drawMeasureTISA_4(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi)
{
	auto scleralSpurItem = target.getItem(DrawTISA::PointType::ScleralSpur);
	auto ptOnScleralSpur = get<2>(scleralSpurItem);

	auto pointOnCorneaItem = target.getItem(DrawTISA::PointType::PointOnCornea);
	auto ptOnCornea = get<2>(pointOnCorneaItem);

	auto ptOnIrisForRangeItem = target.getItem(DrawTISA::PointType::PointOnIrisForRange);
	auto ptOnIrisForRange = get<2>(ptOnIrisForRangeItem);

	auto& pointOnIrisForScleralSpurItem = target.getItem(DrawTISA::PointType::PointOnIrisForScleralSpur);
	auto clickedPtOnIrisForRange = get<2>(pointOnIrisForScleralSpurItem);

	// get the cross point
	// - the line being along with iris line
	auto aForGuideLineOnIris = 0.0;
	auto bForGuideLineOnIris = 0.0;
	{
		aForGuideLineOnIris = (clickedPtOnIrisForRange.Y - ptOnIrisForRange.Y) /
			(clickedPtOnIrisForRange.X - ptOnIrisForRange.X);
		bForGuideLineOnIris = ptOnIrisForRange.Y - aForGuideLineOnIris * ptOnIrisForRange.X;
	}
	// - perpendicular line from the scleral spur point
	auto aPerpForScleralSpur = 0.0;
	auto bPerpForScleralSpur = 0.0;
	{
		auto radius = getCurRangePxTISA(target);
		auto angle = atan2(ptOnCornea.Y - ptOnScleralSpur.Y, ptOnCornea.X - ptOnScleralSpur.X);
		auto projItem = getPtOnCircleTISA(rect, ptOnScleralSpur, angle, radius);
		auto ptOnCircle = get<0>(projItem);
		auto angleProj = get<1>(projItem);
		auto perpLineItemForScleralSpur = getPerpendicularLineTISA(rect, ptOnScleralSpur, angleProj);

		aPerpForScleralSpur = get<1>(perpLineItemForScleralSpur);
		bPerpForScleralSpur = get<2>(perpLineItemForScleralSpur);
	}
	auto ptX = -(bForGuideLineOnIris - bPerpForScleralSpur) / (aForGuideLineOnIris - aPerpForScleralSpur);
	auto ptY = aPerpForScleralSpur * ptX + bPerpForScleralSpur;
	Gdiplus::PointF ptOnIrisForScleralSpur(ptX, ptY);

	// confirm point
	if (!get<1>(pointOnIrisForScleralSpurItem)) {
		get<2>(pointOnIrisForScleralSpurItem) = ptOnIrisForScleralSpur;
		get<1>(pointOnIrisForScleralSpurItem) = true;
	}

	// draw
	// - shape
	//   : area
	vector<Gdiplus::PointF> points = {
		ptOnScleralSpur, ptOnCornea, ptOnIrisForRange, ptOnIrisForScleralSpur
	};
	drawPolygon(G, rect, points, rcRoi, true);
	// - points
	//   : pt on scleral spur
	drawPoint(G, rect, ptOnScleralSpur, rcRoi);
	//   : pt on cornea
	drawPoint(G, rect, ptOnCornea, rcRoi);
	//   : pt on iris 
	drawPoint(G, rect, ptOnIrisForRange, rcRoi);
	//   : pt on iris for scleral spur
	drawPoint(G, rect, ptOnIrisForScleralSpur, rcRoi);
	// - label
	drawMeasureTISA_result(target, G, rect, rcRoi);
}

void DrawMeasureTool::drawMeasureTISA_result(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi)
{
	if (target.size() < 4) {
		return;
	}

	// font
	Gdiplus::SolidBrush brushText(Gdiplus::Color(
		GetRValue(m_textColor), GetGValue(m_textColor), GetBValue(m_textColor)));
	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	const float kScaleRatioX = (float)rect.Width() / (float)m_imageSizeX;
	const float kScaleRatioY = (float)rect.Height() / (float)m_imageSizeY;

	// values
	auto ptOnScleralSpur = get<2>(target.getItem(DrawTISA::PointType::ScleralSpur));
	auto ptOnCornea = get<2>(target.getItem(DrawTISA::PointType::PointOnCornea));
	auto ptOnIrisForRange = get<2>(target.getItem(DrawTISA::PointType::PointOnIrisForRange));
	auto ptOnIrisForScleralSpur = get<2>(target.getItem(DrawTISA::PointType::PointOnIrisForScleralSpur));
	vector<Gdiplus::PointF> ptList = {
		ptOnScleralSpur, ptOnCornea, ptOnIrisForRange, ptOnIrisForScleralSpur
	};

	// label & value
	auto resultItem = getTisaAod(target);
	auto tisa = get<0>(resultItem);
	auto aod = get<1>(resultItem);
	// - tisa
	{
		CString szValue;
		szValue.Format(_T("TISA : %.3f mm²"), tisa);

		// -: text bounds
		auto textRect = getLabelRectForPolygon(G, ptList, szValue, &font);
		// -: position
		auto x = rect.left + textRect.X * kScaleRatioX;
		auto y = rect.top + textRect.Y * kScaleRatioY;
		auto w = textRect.Width;
		auto h = textRect.Height;
		// -: draw
		G->DrawRectangle(&Gdiplus::Pen(&brushText), Gdiplus::Rect(x, y, w, h));
		G->DrawString(szValue, -1, &font, Gdiplus::PointF(x, y), &brushText);
		// - connection
		auto minX = (float)(*std::min_element(ptList.begin(), ptList.end(), [](auto a, auto b) { return a.X > b.X; })).X;
		auto maxX = (float)(*std::max_element(ptList.begin(), ptList.end(), [](auto a, auto b) { return a.X > b.X; })).X;
		auto minY = (float)(*std::min_element(ptList.begin(), ptList.end(), [](auto a, auto b) { return a.Y > b.Y; })).Y;
		auto maxY = (float)(*std::max_element(ptList.begin(), ptList.end(), [](auto a, auto b) { return a.Y > b.Y; })).Y;
		Gdiplus::PointF ptOnValue((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);

		int cxOnArea = rect.left + ptOnValue.X * kScaleRatioX;
		int cyOnArea = rect.top + ptOnValue.Y * kScaleRatioY;
		int nearestXOnTextBound = abs(cxOnArea - x) < abs(cxOnArea - (x + w)) ? x : x + w;
		int nearestYOnTextBound = abs(cyOnArea - y) < abs(cyOnArea - (y + h)) ? y : y + h;
		G->DrawLine(&Gdiplus::Pen(&brushText), cxOnArea, cyOnArea, nearestXOnTextBound,
			nearestYOnTextBound);
	}
	// - aod
	{
		CString szValue;
		szValue.Format(_T("AOD : %.3f mm"), aod);

		// -: text bounds
		const float kOffsetRatio = sqrtf(pow(kScaleRatioX, 2) + pow(kScaleRatioY, 2));
		auto textRect = getLabelRectForLength(G, make_pair(ptOnCornea, ptOnIrisForRange), szValue,
			&font, kOffsetRatio);
		// -: position
		auto x = rect.left + textRect.X * kScaleRatioX;
		auto y = rect.top + textRect.Y * kScaleRatioY;
		auto w = textRect.Width;
		auto h = textRect.Height;
		// -: draw
		G->DrawRectangle(&Gdiplus::Pen(&brushText), Gdiplus::Rect(x, y, w, h));
		G->DrawString(szValue, -1, &font, Gdiplus::PointF(x, y), &brushText);
		// - connection
		auto px = (ptOnCornea.X + ptOnIrisForRange.X) / 2.0;
		auto py = (ptOnCornea.Y + ptOnIrisForRange.Y) / 2.0;
		Gdiplus::PointF ptOnValue(px, py);

		int cxOnArea = rect.left + ptOnValue.X * kScaleRatioX;
		int cyOnArea = rect.top + ptOnValue.Y * kScaleRatioY;
		int nearestXOnTextBound = abs(cxOnArea - x) < abs(cxOnArea - (x + w)) ? x : x + w;
		int nearestYOnTextBound = abs(cyOnArea - y) < abs(cyOnArea - (y + h)) ? y : y + h;
		G->DrawLine(&Gdiplus::Pen(&brushText), cxOnArea, cyOnArea, nearestXOnTextBound,
			nearestYOnTextBound);
	}
}

double DrawMeasureTool::getPixelsBetweenPoint(Gdiplus::PointF point1, Gdiplus::PointF point2)
{
	float distX = (point1.X - point2.X);
	float distY = (point1.Y - point2.Y);

	return sqrt(distX * distX + distY * distY);
}

double DrawMeasureTool::getLengthOfLine(Gdiplus::PointF point1, Gdiplus::PointF point2)
{
	double distX = (point1.X - point2.X) * m_pixelSizeX;
	double distY = (point1.Y - point2.Y) * m_pixelSizeY;

	return sqrt(distX * distX + distY * distY);
}

double DrawMeasureTool::getAngleOfPoints(Gdiplus::PointF point1, Gdiplus::PointF point2, Gdiplus::PointF point3)
{
	double x1 = (point1.X - point2.X) * m_pixelSizeX;
	double x2 = (point3.X - point2.X) * m_pixelSizeX;
	double y1 = (point1.Y - point2.Y) * m_pixelSizeY;
	double y2 = (point3.Y - point2.Y) * m_pixelSizeY;

	double alpha = atan2(y1 * x2 - x1 * y2, x1 * x2 + y1 * y2);
	double angle = fabs(alpha * 180.0f / M_PI + 0.5f);

	return angle;
}

double DrawMeasureTool::getAreaOfPolygon(vector <Gdiplus::PointF> points)
{
	float pixelSize = m_pixelSizeX * m_pixelSizeY;
	float pixelCount = 0;		// in polygon

	int size = points.size();
	int j = size - 1;
	for (int i = 0; i < size; i++)
	{
		pixelCount += (points[j].X + points[i].X) * (points[j].Y - points[i].Y);
		j = i;
	}

	return abs(pixelCount * pixelSize / 2);
}

Gdiplus::RectF DrawMeasureTool::getLabelRectForLength(Gdiplus::Graphics *G,
	pair<Gdiplus::PointF, Gdiplus::PointF> points, CString text, Gdiplus::Font* pFont,
	float offsetRatio)
{
	int offset = 0;
	int shiftX = 0;
	if (offsetRatio > 1.0f) {
		offset = 10;
		shiftX = 10;
	}
	else if (offsetRatio < 0.5f) {
		offset = 300;
		shiftX = -100;
	}
	else {
		offset = 70 / offsetRatio;
		shiftX = -10;
	}

	Gdiplus::RectF textRect;

	CRect rcLabel;

	int cx = (get<0>(points).X + get<1>(points).X) / 2;
	int cy = (get<0>(points).Y + get<1>(points).Y) / 2;

	G->MeasureString(text, text.GetLength(), pFont, Gdiplus::PointF(0, 0), &textRect);

	double angle = atan2(get<1>(points).Y - get<0>(points).Y, get<1>(points).X - get<0>(points).X) + M_PI_2;
	if (get<1>(points).X == get<0>(points).X && get<1>(points).Y == get<0>(points).Y) {
		rcLabel.left = cx + offset * (float)cos(M_PI_4);
		rcLabel.top = cy + offset * (float)sin(M_PI_4);
		rcLabel.right = rcLabel.left + textRect.Width;
		rcLabel.bottom = rcLabel.top + textRect.Height;
	}
	else {
		if (get<1>(points).X == get<0>(points).X) {
			angle = 0;
		}
		else if (get<1>(points).Y == get<0>(points).Y) {
			angle = M_PI_2;
		}
		if (angle >= 0 && angle <= M_PI_2) {			// 1사분면
			rcLabel.left = cx + offset * (float)cos(angle) + shiftX;
			rcLabel.top = cy + offset * (float)sin(angle);
			rcLabel.right = rcLabel.left + textRect.Width;
			rcLabel.bottom = rcLabel.top + textRect.Height;
		}
		else if (angle > M_PI_2 && angle <= M_PI) {		// 4사분면
			rcLabel.left = cx - offset * (float)cos(angle) + shiftX;
			rcLabel.bottom = cy - offset * (float)sin(angle);
			rcLabel.right = rcLabel.left + textRect.Width;
			rcLabel.top = rcLabel.bottom - textRect.Height;
		}
		else if (angle > M_PI) {						// 3사분면
			rcLabel.right = cx + offset * (float)cos(angle);
			rcLabel.bottom = cy + offset * (float)sin(angle);
			rcLabel.left = rcLabel.right - textRect.Width;
			rcLabel.top = rcLabel.bottom - textRect.Height;
		}
		else {											// 2사분면
			rcLabel.right = cx - offset * (float)cos(angle);
			rcLabel.top = cy - offset * (float)sin(angle);
			rcLabel.left = rcLabel.right - textRect.Width;
			rcLabel.bottom = rcLabel.top + textRect.Height;
		}
	}

	return Gdiplus::RectF(rcLabel.left, rcLabel.top, rcLabel.Width(), rcLabel.Height());
}

Gdiplus::RectF DrawMeasureTool::getLabelRectForPolygon(Gdiplus::Graphics *G,
	vector<Gdiplus::PointF> points, CString text, Gdiplus::Font* pFont)
{
	const int kOffset = 20;

	Gdiplus::RectF textRect;

	CRect rcLabel;

	G->MeasureString(text, text.GetLength(), pFont, Gdiplus::PointF(0, 0), &textRect);

	int maxX = 0;
	int maxY = 0;
	for (auto point : points) {
		maxX = max(maxX, point.X);
		maxY = max(maxY, point.Y);
	}

	rcLabel.left = maxX + kOffset;
	rcLabel.top = maxY + kOffset;
	rcLabel.right = rcLabel.left + textRect.Width;
	rcLabel.bottom = rcLabel.top + textRect.Height;

	return Gdiplus::RectF(rcLabel.left, rcLabel.top, rcLabel.Width(), rcLabel.Height());
}

auto DrawMeasureTool::getPtOnCircleTISA(CRect rect, Gdiplus::PointF center, double angle, double radius)->pair<Gdiplus::PointF, double>
{
	auto ratioWndRectForImageW = (double)rect.Width() / (double)m_imageSizeX;
	auto ratioWndRectForImageH = (double)rect.Height() / (double)m_imageSizeY;

	auto radiusProj = ratioWndRectForImageH * radius;

	auto xOnCircle = (double)center.X + radiusProj * cos(angle);
	auto yOnCircle = (double)center.Y + radiusProj * sin(angle);

	auto pointsWindowCX = (double)rect.left + (double)center.X * ratioWndRectForImageW;
	auto pointsWindowCY = (double)rect.top + (double)center.Y * ratioWndRectForImageH;

	auto pointsWindowRX = (double)rect.left + xOnCircle * ratioWndRectForImageW;
	auto pointsWindowRY = (double)rect.top + yOnCircle * ratioWndRectForImageH;

	auto angleProj = atan2(pointsWindowRY - pointsWindowCY, pointsWindowRX - pointsWindowCX);
	auto xOnCircleProj = pointsWindowCX + radiusProj * cos(angleProj);
	auto yOnCircleProj = pointsWindowCY + radiusProj * sin(angleProj);

	auto xOnCircleRevProj = xOnCircleProj / ratioWndRectForImageW;
	auto yOnCircleRevProj = yOnCircleProj / ratioWndRectForImageH;

	return make_pair(Gdiplus::PointF(xOnCircleRevProj, yOnCircleRevProj), angleProj);
};

auto DrawMeasureTool::getPerpendicularLineTISA(CRect rect, Gdiplus::PointF ptOnCircle, double angleProj)->tuple<vector<Gdiplus::PointF>, double, double>
{
	auto dxProj = 100.0;
	auto dyProj = tan(angleProj) * dxProj;
	auto dxProjPerp = -dyProj;
	auto dyProjPerp = dxProj;
	auto dxPrep = dxProjPerp * (double)m_imageSizeX / (double)rect.Width();
	auto dyPrep = dyProjPerp * (double)m_imageSizeY / (double)rect.Height();

	vector<Gdiplus::PointF> linePerpendicacular;
	double aPerp = 0.0;
	double bPerp = 0.0;
	if (dxPrep == 0) {
		aPerp = dxPrep == 0 ? INT_MAX : dyPrep / dxPrep;
		bPerp = (double)ptOnCircle.Y - aPerp * (double)ptOnCircle.X;
		linePerpendicacular = {
			Gdiplus::PointF((double)ptOnCircle.X, 0), Gdiplus::PointF((double)ptOnCircle.X, m_imageSizeY)
		};
	}
	else {
		aPerp = dyPrep / dxPrep;
		bPerp = (double)ptOnCircle.Y - aPerp * (double)ptOnCircle.X;
		linePerpendicacular = {
			Gdiplus::PointF(0, bPerp), Gdiplus::PointF(m_imageSizeX, aPerp * m_imageSizeX + bPerp)
		};
	}

	return make_tuple(linePerpendicacular, aPerp, bPerp);
}

auto DrawMeasureTool::getPtOnIrisTISA(Gdiplus::PointF clickedPos, Gdiplus::PointF ptOnCornea, double slopeOfPerpLine,
	double interceptOfPerpLine)->Gdiplus::PointF
{
	Gdiplus::PointF ptOnIris;
	
	if (clickedPos.Y < ptOnCornea.Y) {
		ptOnIris.Y = ptOnCornea.Y;
	}
	else if (clickedPos.Y > m_imageSizeY) {
		ptOnIris.Y = m_imageSizeY;
	}
	else {
		ptOnIris.Y = clickedPos.Y;
	}
	ptOnIris.X = (ptOnIris.Y - interceptOfPerpLine) / slopeOfPerpLine;

	return ptOnIris;
}

auto DrawMeasureTool::getCurRangePxTISA(DrawTISA& target)->double
{
	return target.rangeUM / m_pixelSizeY;
}

auto DrawMeasureTool::getTisaAod(DrawTISA& target)->pair<double, double>
{
	// values
	auto ptOnScleralSpur = get<2>(target.getItem(DrawTISA::PointType::ScleralSpur));
	auto ptOnCornea = get<2>(target.getItem(DrawTISA::PointType::PointOnCornea));
	auto ptOnIrisForRange = get<2>(target.getItem(DrawTISA::PointType::PointOnIrisForRange));
	auto ptOnIrisForScleralSpur = get<2>(target.getItem(DrawTISA::PointType::PointOnIrisForScleralSpur));
	
	
	auto l1 = sqrt(pow((ptOnCornea.X - ptOnScleralSpur.X) * m_pixelSizeX, 2) + pow((ptOnCornea.Y - ptOnScleralSpur.Y) * m_pixelSizeY, 2));
	auto l2 = sqrt(pow((ptOnIrisForScleralSpur.X - ptOnScleralSpur.X) * m_pixelSizeX, 2) + pow((ptOnIrisForScleralSpur.Y - ptOnScleralSpur.Y) * m_pixelSizeY, 2));
	auto l3 = sqrt(pow((ptOnIrisForRange.X - ptOnCornea.X) * m_pixelSizeX, 2) + pow((ptOnIrisForRange.Y - ptOnCornea.Y) * m_pixelSizeY, 2));

	auto tisa = (l1 * max(l2, l3) - l1 * abs(l2 - l3) / 2.0) / 1000000.0;
	auto aod = l2 / 1000.0;

	return make_pair(tisa, aod);
}
