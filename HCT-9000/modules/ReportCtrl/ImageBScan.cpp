#include "stdafx.h"
#include "ImageBScan.h"


using namespace ReportCtrl;
using namespace Gdiplus;

ImageBScan::ImageBScan()
	: m_bscanColor(BScanColor::origin), m_rangeMin(0), m_rangeMax(255)
	, m_showDirection(false), m_scanPointX(-1)
	, m_rangeDisc({0, 0}), m_rangeCup({ 0, 0 }),m_panelX(0), m_panelY(0)
{
	m_lutRGB = cv::imread("LUT\\colorscale_bscan_rgb.bmp");
	m_lutPos = cv::imread("LUT\\colorscale_bscan_pos.bmp");
	m_lutNeg = cv::imread("LUT\\colorscale_bscan_neg.bmp");
	m_lutUse.create(1, 256, CV_8UC3);

	DrawMeasureTool::setUnit(_T("㎛"), _T("㎛²"));
}

ImageBScan::~ImageBScan()
{

}

void ImageBScan::saveImage(CString filepath)
{
	CT2CA pszConvertedAnsiString(filepath);
	std::string s(pszConvertedAnsiString);

	CvImage image;
	makeDisplayImage(image.getCvMat(), m_image.getCvMat(), m_bscanColor);
	image.saveFile(s);

	return;
}


void ImageBScan::setBScanColor(BScanColor color)
{
	m_bscanColor = color;
	recalcLUT();

	if (m_hWnd)
	{
		Invalidate();
	}

	return;
}

void ImageBScan::setBScanLevel(int min, int max)
{
	m_rangeMin = min;
	m_rangeMax = max;
	recalcLUT();

	if (m_hWnd)
	{
		Invalidate();
	}

	return;
}

void ImageBScan::setEyeSide(EyeSide side)
{
	m_eyeSide = side;

	return;
}

void ImageBScan::setScanPattern(OctScanPattern pattern)
{
	m_scanPattern = pattern;

	float sx = getScanRangeX(pattern) / getImgWidth();
	float sy = getScanRangeZ(pattern, getImgHeight()) / getImgHeight();

	DrawMeasureTool::setPixelSize(sx, sy);
	return;
}

void ImageBScan::showDirection(bool show, Gdiplus::Color color)
{
	m_showDirection = show;
	m_colorScanLine = color;

	return;
}

void ImageBScan::setSelectedScanLine(int idxLine)
{
	m_scanIndex = idxLine;
	m_scanAngle = getScanAngle(idxLine);

	return;
}

int ImageBScan::getSelectedScanLine()
{
	return m_scanIndex;
}

void ImageBScan::setSelectedScanLineHD(int idxLine)
{
	m_scanIndex = idxLine;
	m_scanAngle = (idxLine == 0) ? 0 : 270;

	return;
}

void ImageBScan::setOpticNerveDiscRange(int x1, int x2, COLORREF color)
{
	m_rangeDisc.first = x1;
	m_rangeDisc.second = x2;
	m_clrRangeDisc = color;

	return;
}

void ImageBScan::setOpticNerveCupRange(int x1, int x2, COLORREF color)
{
	m_rangeCup.first = x1;
	m_rangeCup.second = x2;
	m_clrRangeCup = color;

	return;
}

void ImageBScan::recalcLUT(void)
{
	cv::Mat lutSrc;
	float range = float(m_rangeMax - m_rangeMin);

	if (m_bscanColor == BScanColor::color)
	{
		lutSrc = m_lutRGB;
	}
	else if (m_bscanColor == BScanColor::positive)
	{
		lutSrc = m_lutPos;
	}
	else if (m_bscanColor == BScanColor::negative)
	{
		lutSrc = m_lutNeg;
	}
	else
	{
		return;
	}
	
	for (int i = 0; i < 256; i++)
	{
		int index;

		if (i <= m_rangeMin)
		{
			index = 0;
		}
		else if (i < m_rangeMax)
		{
			index = int(float(i - m_rangeMin) / (float)range * 255.0f);
		}
		else
		{
			index = 255;
		}

		m_lutUse.data[i * 3 + 0] = lutSrc.data[index * 3 + 0];
		m_lutUse.data[i * 3 + 1] = lutSrc.data[index * 3 + 1];
		m_lutUse.data[i * 3 + 2] = lutSrc.data[index * 3 + 2];
	}

	return;
}

bool ImageBScan::makeDisplayImage(cv::Mat &dstImage, cv::Mat &srcImage, BScanColor color)
{
	srcImage.copyTo(dstImage);

	if (color != BScanColor::origin)
	{
		cv::cvtColor(dstImage, dstImage, cv::COLOR_GRAY2RGB);
		cv::LUT(dstImage, m_lutUse, dstImage);
	}

	return true;
}

//void ImageBScan::getDstArea(int *xDst, int *yDst, int *wDst, int *hDst)
//{
//	if (m_pattern.isCorneaScan())
//	{
//		float rangeX = getScanRangeX(m_pattern);
//		float rangeZ = getScanRangeZ(m_pattern, m_image.getHeight());
//		int resizeHeight = int(getWndWidth() * rangeZ / rangeX);
//
//		*wDst = getWndWidth();
//		*hDst = resizeHeight;
//		*xDst = 0;
//		*yDst = int((getWndHeight() - resizeHeight) / 2);
//	}
//	else
//	{
//		*xDst = 0;
//		*yDst = 0;
//		*wDst = getWndWidth();
//		*hDst = getWndHeight();
//	}
//
//	return;
//}

void ImageBScan::clearMeasure(void)
{
	clearMeasurePoints();
	Invalidate();

	return;
}


void ImageBScan::clearSegmentLayer(void)
{
	m_layers.clear();

	m_rangeCup.first = 0;
	m_rangeCup.second = 0;

	m_rangeDisc.first = 0;
	m_rangeDisc.second = 0;


	return;
}

void ImageBScan::addSegmentLayer(SegmentLayer layer)
{
	m_layers.push_back(layer);

	return;
}

int ImageBScan::getScanAngle(int idxLine)
{
	int angle;

	auto name = m_scanPattern.getPatternName();
	switch (name)
	{
	case OctGlobal::PatternName::MacularCross:
	{
		int lines = m_scanPattern.getNumberOfScanLines();
		angle = (idxLine < lines / 2) ? 0 : 270;
	}
	break;
	case OctGlobal::PatternName::MacularRadial:
	case OctGlobal::PatternName::DiscRadial:
	case OctGlobal::PatternName::AnteriorRadial:
	{
		angle = -15 * idxLine;
	}
	break;
	case OctGlobal::PatternName::Topography:
	{
		int lines = m_scanPattern.getNumberOfScanLines();
		angle = (int)(-(180.f / lines) * (float)idxLine);
	}
	break;
	case OctGlobal::PatternName::AnteriorLine:
	case OctGlobal::PatternName::AnteriorFull:
	{
		angle = (m_scanPattern.getDirection() == 0) ? 0 : 90;
	}
	break;
	default:
	{
		angle = (m_scanPattern.getDirection() == 0) ? 0 : 270;
	}
	break;
	}

	if (name == OctGlobal::PatternName::AnteriorLine ||
		name == OctGlobal::PatternName::AnteriorFull)
	{
		angle = static_cast<int>(std::round(angle + m_scanAngleOffset));
	}

	return angle;
}

bool ImageBScan::getLayerPoints(vector <Gdiplus::Point>& out_point1,
		vector <Gdiplus::Point>& out_point2, SegmentLayer layer, CRect rect,
		bool ignoreDiscArea)
{
	if (layer.data.size() == 0) {
		return false;
	}

	int width = rect.Width();
	int disc_x1 = (int)((float)m_rangeDisc.first / (float)m_imageSizeX * (float)width);
	int disc_x2 = (int)((float)m_rangeDisc.second / (float)m_imageSizeX * (float)width);

	out_point1.clear();
	out_point2.clear();
	for (int i = 0; i < layer.data.size(); i++) {
		if (ignoreDiscArea) {
			out_point2.push_back(Gdiplus::Point(i, layer.data[i]));
		}
		else {
			if (i < disc_x1) {
				out_point1.push_back(Gdiplus::Point(i, layer.data[i]));
			}
			else if (i >= disc_x2) {
				out_point2.push_back(Gdiplus::Point(i, layer.data[i]));
			}
		}
	}

	return true;
}

void ImageBScan::setScanPointX(int value, int width)
{
	m_scanPointX = value;
	m_widthForScanPoint = width;
}

BEGIN_MESSAGE_MAP(ImageBScan, ImageBase)
	ON_WM_PAINT()
END_MESSAGE_MAP()


void ImageBScan::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	drawToDC(&dc, rect, m_bscanColor);

	return;
}

void ImageBScan::drawToDC(CDC *pDC, CRect rect, BScanColor color)
{
	int wndX = rect.left;
	int wndY = rect.top;
	int wndW = rect.Width();
	int wndH = rect.Height();
	int bmpW = getWndWidth();
	int bmpH = getWndHeight();

	// Since segmentation is drawn to fit the screen size, it is drawn to the screen size and then stretched.
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, bmpW, bmpH);

	CRect rtBitmap;
	rtBitmap.SetRect(0, 0, bmpW, bmpH);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);

	CvImage image;
	makeDisplayImage(image.getCvMat(), m_image.getCvMat(), color);

	if (!image.isEmpty())
	{
		drawImage(&memDC, rtBitmap, image);
		drawSegmentLayer(&memDC, rtBitmap);
		drawOpticNerveArea(&memDC, rtBitmap);
		drawMeasure(&memDC, rtBitmap);

		drawScanPointLine(&memDC, rect);
	}

	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(wndX, wndY, wndW, wndH, &memDC, 0, 0, bmpW, bmpH, SRCCOPY);

	if (m_showDirection)
	{
		//int dirIconX = wndW / 40;
		//int dirIconY = dirIconX;
		//int dirIconSize = wndW / 20;

		int dirIconX = (m_panelX >= 0) ? 30 : (m_panelX*-1) + 30;
		int dirIconY = (m_panelY >= 0) ? 30 : (m_panelY*-1) + 30;
		int dirIconSize = 38;

		Gdiplus::Rect rect(wndX + dirIconX, wndY + dirIconY, dirIconSize, dirIconSize);
		drawScanDirection(pDC, rect, m_colorScanLine);
	}

	bitmap.DeleteObject();
	memDC.DeleteDC();

	return;
}

void ImageBScan::drawToDCRaw(CDC *pDC, CRect rect)
{
	int wndX = rect.left;
	int wndY = rect.top;
	int wndW = rect.Width();
	int wndH = rect.Height();
	int bmpW = getWndWidth();
	int bmpH = getWndHeight();

	// segmentation이 화면 사이즈에 맞게 그려지므로, 화면 사이즈로 그린후 Stretch 한다.
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, bmpW, bmpH);

	CRect rtBitmap;
	rtBitmap.SetRect(0, 0, bmpW, bmpH);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);

	CvImage image;
	makeDisplayImage(image.getCvMat(), m_image.getCvMat(), BScanColor::origin);

	if (!image.isEmpty()) {
		drawImage(&memDC, rtBitmap, image);
	}

	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(wndX, wndY, wndW, wndH, &memDC, 0, 0, bmpW, bmpH, SRCCOPY);

	if (m_showDirection)
	{
		//int dirIconX = wndW / 40;
		//int dirIconY = dirIconX;
		//int dirIconSize = wndW / 20;

		int dirIconX = (m_panelX >= 0) ? 30 : (m_panelX*-1) + 30;
		int dirIconY = (m_panelY >= 0) ? 30 : (m_panelY*-1) + 30;
		int dirIconSize = 38;

		Gdiplus::Rect rect(wndX + dirIconX, wndY + dirIconY, dirIconSize, dirIconSize);
		drawScanDirection(pDC, rect, m_colorScanLine);
	}

	bitmap.DeleteObject();
	memDC.DeleteDC();
}

auto ImageBScan::getLayerPointAt(OcularLayerType layerType, int x)->std::pair<bool, int>
{
	auto itr = std::find_if(m_layers.begin(), m_layers.end(),
		[layerType](SegmentLayer item) {
			return (item.type == layerType);
		});
	if (itr == m_layers.end()) {
		return make_pair(false, -1);
	}
	
	auto layer = *itr;
	return make_pair(true, layer.data[x]);
}

void ImageBScan::drawImage(CDC *pDC, CRect rect, CvImage image)
{
	if (image.isEmpty())
	{
		return;
	}

	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = image.createDIBitmap(hDC, m_flipVert);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	SetStretchBltMode(hDC, HALFTONE);
	StretchBlt(hDC, rect.left, rect.top, rect.Width(), rect.Height(), hMemDC,
		0, 0, image.getWidth(), image.getHeight(), SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	DeleteObject(hbmp);

	return;
}

void ImageBScan::drawSegmentLayer(CDC *pDC, CRect rect)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	for (auto layer : m_layers) {
		if (!layer.show) {
			continue;
		}

		vector <Gdiplus::Point> points1;
		vector <Gdiplus::Point> points2;
		if (!getLayerPoints(points1, points2, layer, rect,
			layer.type == OctGlobal::OcularLayerType::ILM)) {
			continue;
		}

		BYTE r = GetRValue(layer.rgb);
		BYTE g = GetGValue(layer.rgb);
		BYTE b = GetBValue(layer.rgb);
		Gdiplus::Pen pen(Gdiplus::Color(r, g, b), 2);

		if (points1.size()) {
			G.DrawLines(&pen, points1.data(), points1.size());
		}
		if (points2.size()) {
			G.DrawLines(&pen, points2.data(), points2.size());
		}
	}
}

void ImageBScan::drawOpticNerveArea(CDC *pDC, CRect rect)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	
	// disc color
	auto r = GetRValue(m_clrRangeDisc);
	auto g = GetGValue(m_clrRangeDisc);
	auto b = GetBValue(m_clrRangeDisc);
	Gdiplus::Pen penDisc(Gdiplus::Color(r, g, b), 3.0f);
	// cup color
	r = GetRValue(m_clrRangeCup);
	g = GetGValue(m_clrRangeCup);
	b = GetBValue(m_clrRangeCup);
	Gdiplus::Pen penCup(Gdiplus::Color(r, g, b), 3.0f);

	penDisc.SetDashStyle(Gdiplus::DashStyleDash);
	penCup.SetDashStyle(Gdiplus::DashStyleDash);

	int width = rect.Width();
	int height = rect.Height();
	int disc_x1 = (int)((float)m_rangeDisc.first / (float)m_imageSizeX * (float)width);
	int disc_x2 = (int)((float)m_rangeDisc.second / (float)m_imageSizeX * (float)width);
	int cup_x1 = (int)((float)m_rangeCup.first / (float)m_imageSizeX * (float)width);
	int cup_x2 = (int)((float)m_rangeCup.second / (float)m_imageSizeX * (float)width);

	// disc1
	if (disc_x1 > 0 && disc_x2 > 0) {
		G.DrawLine(&penDisc, disc_x1, 0, disc_x1, height);
		G.DrawLine(&penDisc, disc_x2, 0, disc_x2, height);
	}

	// cup
	if (cup_x1 > 0 && cup_x2 > 0) {
		G.DrawLine(&penCup, cup_x1, 0, cup_x1, height);
		G.DrawLine(&penCup, cup_x2, 0, cup_x2, height);
	}

	return;
}

void ImageBScan::drawScanDirection(CDC *pDC, Gdiplus::Rect rect, Gdiplus::Color color)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	int radiusX = rect.Width / 2;
	int radiusY = rect.Height / 2;
	int centerX = rect.X + radiusX;
	int centerY = rect.Y + radiusY;

	Gdiplus::SolidBrush brush(Gdiplus::Color(128, 128, 128, 128));
	G.FillRectangle(&brush, rect);

	Gdiplus::StringFormat SF;
	SF.SetAlignment(StringAlignmentCenter);
	SF.SetLineAlignment(StringAlignmentCenter);

	Gdiplus::Font font(_T("Noto Sans CJK KR Medium"), 10, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	CString textSnit[4] = { _T("S"), _T("N"), _T("I"), _T("T") };

	for (int i = 0; i < 4; i++)
	{
		float angle = (m_eyeSide == EyeSide::OS) ? 90 + (i * 90) : 90 - (i * 90);
		float radian = M_PI * angle / 180;

		Gdiplus::PointF point;
		point.X = centerX + int((radiusX + 6) * cos(radian));
		point.Y = centerY - int((radiusY + 6) * sin(radian));
		G.DrawString(textSnit[i], -1, &font, point, &SF, &Gdiplus::SolidBrush(Gdiplus::Color::Gray));
	}

	// pen
	auto patternName = m_scanPattern.getPatternName();
	if (patternName == PatternName::DiscCircle)
	{
		int arrowSizeX = round(((float)rect.Width / 12.0f));
		int arrowSizeY = round((float)rect.Width / 7.0f);
		Gdiplus::Point points[5] =
		{
			Gdiplus::Point(3,-5),
			Gdiplus::Point(0, 0),
			Gdiplus::Point(-3,-5),
			Gdiplus::Point(0, -3),
			Gdiplus::Point(3,-5)
		};

		Gdiplus::GraphicsPath path;
		path.AddPolygon(points, 5);

		Gdiplus::Pen pen(color, 2.0f);
		pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));

		G.DrawEllipse(&pen, rect);

		// direction
		int x1 = rect.X + rect.Width;
		int y1 = rect.Y + rect.Height / 2;
		if (m_eyeSide == EyeSide::OS) {
			G.DrawLine(&pen, x1, y1, x1, y1 - 1);
		}
		else {
			G.DrawLine(&pen, x1, y1, x1, y1 + 1);
		}
	}
	else
	{
		int arrowSizeX = rect.Width / 11;
		int arrowSizeY = rect.Width / 7;
		Gdiplus::Point points[5] =
		{
			Gdiplus::Point(arrowSizeX,-arrowSizeY),
			Gdiplus::Point(0, 0),
			Gdiplus::Point(-arrowSizeX,-arrowSizeY),
			Gdiplus::Point(0, -arrowSizeX),
			Gdiplus::Point(arrowSizeX,-arrowSizeY)
		};

		Gdiplus::GraphicsPath path;
		path.AddPolygon(points, 5);

		Gdiplus::Pen pen(color, 2.0f);
		pen.SetCustomEndCap(&Gdiplus::CustomLineCap(&path, nullptr));


		float radian = M_PI * m_scanAngle / 180;
		
		int x1 = centerX - int(radiusX * cos(radian));
		int y1 = centerY + int(radiusY * sin(radian));
		int x2 = centerX + int(radiusX * cos(radian));
		int y2 = centerY - int(radiusY * sin(radian));
		
		G.DrawLine(&pen, x1, y1, x2, y2);
	}

	return;
}

void ImageBScan::drawScanPointLine(CDC *pDC, CRect rect)
{
	if (m_scanPointX < 0 || m_widthForScanPoint < 0) {
		return;
	}

	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	Gdiplus::Pen pen(Gdiplus::Color(70, 255, 255, 0), 3.0f);

	CRect cRect;
	GetClientRect(&cRect);
	auto x = (int)(((float)cRect.Width() / (float)m_widthForScanPoint) * (float)m_scanPointX);

	G.DrawLine(&pen, x, 0, x, rect.Width());
}

BOOL ImageBScan::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN)
	{
		if (m_drawingTool == DrawingTool::length ||
			m_drawingTool == DrawingTool::angle ||
			m_drawingTool == DrawingTool::areaPolygon ||
			m_drawingTool == DrawingTool::tisa_500 ||
			m_drawingTool == DrawingTool::tisa_750)
		{
			CRect rect;
			GetWindowRect(&rect);

			CPoint point;
			point.x = pMsg->pt.x - rect.left;
			point.y = pMsg->pt.y - rect.top;

			insertMeasurePoint(point);

			Invalidate();
		}
	}

	return ImageBase::PreTranslateMessage(pMsg);
}

void ImageBScan::setPanelPostion(int panelX, int panelY)
{
	m_panelX = panelX;
	m_panelY = panelY;
}