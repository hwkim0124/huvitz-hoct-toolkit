#include "stdafx.h"
#include "ImageRetina.h"



ImageRetina::ImageRetina()
	: m_showWorkingDotLine(false), m_showSplitFocusGuide(false), m_showMasking(false)
	, m_showFixationTarget(false), m_fixationRow(kFixationCenterRow), m_fixationCol(kFixationCenterCol)
	, m_showFixationPosition(false), m_showTrackingTarget(false), m_overlaps(1), m_isHorizontal(true)
{
}

ImageRetina::~ImageRetina()
{
}

void ImageRetina::setImage(unsigned char *data, unsigned int width, unsigned int height)
{
	if (data == nullptr)
	{
		return;
	}

	CvImage image = CvImage();
	if (image.fromBitsData(data, width, height))
	{
		cv::Mat mat = image.getCvMat();

		// masking
		cv::Mat mask = cv::Mat::ones(height, width, CV_8UC1);
		cv::Scalar colorMask(0, 0, 0);
		cv::circle(mask, cv::Point(width / 2, height / 2), width / 2, colorMask, -1, CV_AA);
		mat.setTo(0, mask);

		// direction mark
		int mark_x = int(width / 64.0f * 56);
		int mark_y = int(height / 64.0f * 8);
		int mark_radius = int(width / 64.0f);
		cv::Scalar colorMark(120, 153, 238);
		cv::circle(mat, cv::Point(mark_x, mark_y), mark_radius, colorMark, -1, CV_AA);

		ImageBase::setImage(image);
	}

	return;
}

void ImageRetina::saveImage(CString filePath)
{
	if (!m_image.isEmpty())
	{
		ImageBase::saveImage(filePath);
	}
	
	return;
}

CvImage ImageRetina::getImage(void)
{
	return m_image;
}

void ImageRetina::showWorkingDotLine(bool show)
{
	m_showWorkingDotLine = show; 

	return;
}

void ImageRetina::showSplitFocusGuide(bool show)
{
	m_showSplitFocusGuide = show;

	return;
}

void ImageRetina::showMasking(bool show, COLORREF color)
{
	m_showMasking = show;
	m_clrMask = color;

	return;
}

void ImageRetina::setWndPixelPer1MM(double pixel)
{
	m_wndPixelPer1MM = pixel;

	return;
}

void ImageRetina::setSplitGuideCenter(int x, int y)
{
	m_splitGuideCenterX = x;
	m_splitGuideCenterY = y;

	return;
}

void ImageRetina::getDstArea(int *xDst, int *yDst, int *wDst, int *hDst)
{
	CRect rect;
	GetClientRect(&rect);

	CPoint point;
	point = rect.CenterPoint();

	float dx, dy;
	getDimension(dx, dy);

	int width, height;
	width = int(dx * m_wndPixelPer1MM);
	height = int(dy * m_wndPixelPer1MM);

	*xDst = point.x - (width / 2);
	*yDst = point.y - (height / 2);
	*wDst = width;
	*hDst = height;

	return;
}

void ImageRetina::showFixationTarget(bool show)
{
	m_showFixationTarget = show;

	return;
}

void ImageRetina::showTrackingTarget(bool show, int overlaps, bool isHorizontal, int scanLines)
{
	m_showTrackingTarget = show;
	m_isHorizontal = isHorizontal;
	m_overlaps = overlaps;
	m_scanLines = scanLines;
}

void ImageRetina::showFixationPosition(bool show)
{
	m_showFixationPosition = show;
}

bool ImageRetina::setFixationTarget(CPoint point)
{
	CRect rect;
	GetWindowRect(&rect);

	CPoint ptCenter;
	ptCenter = rect.CenterPoint();

	int offsetX = point.x - ptCenter.x;
	int offsetY = point.y - ptCenter.y;
	double pixelPerFixationOffset = kFixationSpaceSize * m_wndPixelPer1MM;

	int row = kFixationCenterRow + int(offsetX / pixelPerFixationOffset);
	int col = kFixationCenterCol + int(offsetY / pixelPerFixationOffset);

	return setFixationTarget(row, col);
}

bool ImageRetina::setFixationTarget(int row, int col)
{
	// limitation check
	CRect rect;
	GetClientRect(&rect);

	double pixelPerFixationOffset = kFixationSpaceSize * m_wndPixelPer1MM;

	int x = rect.CenterPoint().x + long((row - kFixationCenterRow) * pixelPerFixationOffset);
	int y = rect.CenterPoint().y + long((col - kFixationCenterCol) * pixelPerFixationOffset);

	if (!rect.PtInRect(CPoint(x, y))) {
		return false;
	}

	//
	m_fixationRow = row;
	m_fixationCol = col;

	return true;
}

void ImageRetina::getFixationTarget(CPoint *point)
{
	CRect rect;
	GetClientRect(&rect);

	double pixelPerFixationOffset = kFixationSpaceSize * m_wndPixelPer1MM;
	
	point->x = rect.CenterPoint().x + long((m_fixationRow - kFixationCenterRow) * pixelPerFixationOffset);
	point->y = rect.CenterPoint().y + long((m_fixationCol - kFixationCenterCol) * pixelPerFixationOffset);

	return;
}

void ImageRetina::getFixationTarget(int *row, int *col)
{
	*row = m_fixationRow;
	*col = m_fixationCol;

	return;
}

double ImageRetina::getWndPixelPer1MM(void)
{
	return m_wndPixelPer1MM;
}


BEGIN_MESSAGE_MAP(ImageRetina, ImageBase)
	ON_WM_PAINT()
END_MESSAGE_MAP()


void ImageRetina::OnPaint()
{
	CPaintDC dc(this);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, getWndWidth(), getWndHeight());

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	memDC.SelectObject(&bitmap);

	drawImage(&memDC);
	drawWorkingDotLine(&memDC);
	drawSplitFocusGuide(&memDC);
	drawMasking(&memDC);
	drawFixationTarget(&memDC);
	drawTrackingTarget(&memDC);
	
	dc.BitBlt(0, 0, getWndWidth(), getWndHeight(), &memDC, 0, 0, SRCCOPY);

	bitmap.DeleteObject();
	memDC.DeleteDC();

	return;
}

void ImageRetina::drawToDC(CDC *pDC, CRect rtClient, float pixelPerMM)
{
	int x = rtClient.left;
	int y = rtClient.top;
	int w = rtClient.Width();
	int h = rtClient.Height();

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, w, h);

	CRect rtBitmap;
	rtBitmap.SetRect(0, 0, w, h);
	
	CRect rtImage;
	rtImage.left = rtBitmap.CenterPoint().x - (m_actualLengthX * pixelPerMM / 2);
	rtImage.right = rtBitmap.CenterPoint().x + (m_actualLengthX * pixelPerMM / 2);
	rtImage.top = rtBitmap.CenterPoint().y - (m_actualLengthX * pixelPerMM / 2);
	rtImage.bottom = rtBitmap.CenterPoint().y + (m_actualLengthX * pixelPerMM / 2);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);

	drawImage(&memDC, rtImage);
	drawMasking(&memDC, rtBitmap, rtImage);

	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(x, y, w, h, &memDC, 0, 0, w, h, SRCCOPY);

	bitmap.DeleteObject();
	memDC.DeleteDC();


	return;
}

void ImageRetina::drawToDCForReport(CDC *pDC, CRect rtClient, float pixelPerMM)
{
	int x = rtClient.left;
	int y = rtClient.top;
	int w = rtClient.Width();
	int h = rtClient.Height();

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, w, h);

	CRect rtBitmap;
	rtBitmap.SetRect(0, 0, w, h);

	CRect rtImage;
	rtImage.left = rtBitmap.CenterPoint().x - (m_actualLengthX * pixelPerMM / 2);
	rtImage.right = rtBitmap.CenterPoint().x + (m_actualLengthX * pixelPerMM / 2);
	rtImage.top = rtBitmap.CenterPoint().y - (m_actualLengthX * pixelPerMM / 2);
	rtImage.bottom = rtBitmap.CenterPoint().y + (m_actualLengthX * pixelPerMM / 2);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);

	// masking
	CvImage imageMasked = CvImage();
	m_image.getCvMat().copyTo(imageMasked.getCvMat());
	if (!m_flipVert) {
		imageMasked.flipVertical();
	}
	ImageBase::maskIRFundusImage(imageMasked.getCvMat(), imageMasked.getCvMat().rows / 2);
	memDC.FillSolidRect(rtBitmap, RGB(255, 255, 255));

	drawImage(&memDC, imageMasked, rtImage);
	//drawMasking(&memDC, rtBitmap, rtImage);

	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt(x, y, w, h, &memDC, 0, 0, w, h, SRCCOPY);

	bitmap.DeleteObject();
	memDC.DeleteDC();

	return;
}

void ImageRetina::drawWorkingDotLine(CDC *pDC)
{
	if (!m_showWorkingDotLine)
	{
		return;
	}

	int lenY = 4;
	int lenX = getWndWidth() / 5;
	int posY = getWndHeight() / 2 - lenY / 2;

	CRect rtLeft, rtRight;
	rtLeft.SetRect(0, posY, lenX, posY + lenY);
	rtRight.SetRect(getWndWidth() - lenX, posY, getWndWidth(), posY + lenY);

	CBrush brush;
	brush.CreateSolidBrush(RGB(29, 192, 193));

	pDC->FillRect(rtLeft, &brush);
	pDC->FillRect(rtRight, &brush);

	brush.DeleteObject();

	return;
}

void ImageRetina::drawSplitFocusGuide(CDC *pDC)
{
	if (m_image.isEmpty())
	{
		return;
	}
	if (!m_showSplitFocusGuide)
	{
		return;
	}

	CRect rect;
	GetClientRect(&rect);

	int xDst, yDst, wDst, hDst;
	getDstArea(&xDst, &yDst, &wDst, &hDst);

	int px = (rect.Width() / 2) + int(m_splitGuideCenterX * (float)wDst / (float)getImgWidth());
	int py = (rect.Height() / 2) + int(m_splitGuideCenterY * (float)hDst / (float)getImgHeight());

	CPen pen;
	pen.CreatePen(PS_SOLID, 3, RGB(255, 87, 79));

	pDC->SelectObject(&pen);
	pDC->MoveTo(px, py - 20);
	pDC->LineTo(px, py + 20);

	pen.DeleteObject();
	return;
}

void ImageRetina::drawMasking(CDC* pDC)
{
	if (!m_showMasking) {
		return;
	}

	CRect rect;
	GetClientRect(&rect);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWndWidth(), getWndHeight());

	CBrush brush1(m_clrMask);
	CBrush brush2(RGB(255, 0, 255));

	int xDst, yDst, wDst, hDst;
	getDstArea(&xDst, &yDst, &wDst, &hDst);
	
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);
	memDC.FillSolidRect(&rect, m_clrMask);
	memDC.SelectObject(brush2);
	memDC.Ellipse(xDst, yDst, xDst + wDst, yDst + hDst);

	pDC->TransparentBlt(0, 0, rect.Width(), rect.Height(), &memDC, 
		0, 0, rect.Width(), rect.Height(), RGB(255, 0, 255));


	return;
}

void ImageRetina::drawMasking(CDC* pDC, CRect rtClient, CRect rtMask)
{
	if (!m_showMasking) {
		return;
	}

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, rtClient.Width(), rtClient.Height());

	CBrush brush1(m_clrMask);
	CBrush brush2(RGB(255, 0, 255));

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);
	memDC.FillSolidRect(&rtClient, m_clrMask);
	memDC.SelectObject(brush2);
	memDC.Ellipse(rtMask);

	pDC->TransparentBlt(rtClient.left, rtClient.top, rtClient.Width(), rtClient.Height(), &memDC, 
		0, 0, rtClient.Width(), rtClient.Height(), RGB(255, 0, 255));

	return;
}

void ImageRetina::drawFixationTarget(CDC *pDC)
{
	if (!m_showFixationTarget) {
		return;
	}

	CPoint point;
	getFixationTarget(&point);

	Gdiplus::Graphics G(pDC->m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(0, 255, 0), 4.0f);

	G.DrawLine(&pen, point.x - 8, point.y, point.x + 8, point.y);
	G.DrawLine(&pen, point.x, point.y - 8, point.x, point.y + 8);

	if (m_showFixationPosition)
	{
		Gdiplus::Font fontMeasureInfo(_T("Noto Sans CJK KR Regular"), 16,
			Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brushMeasureInfo(Gdiplus::Color(0, 255, 0));
		Gdiplus::StringFormat sfLeftTop;
		sfLeftTop.SetAlignment(Gdiplus::StringAlignmentNear);

		CString strPos;
		strPos.Format(_T("%d, %d"), m_fixationRow, m_fixationCol);

		G.DrawString(strPos, -1, &fontMeasureInfo, Gdiplus::PointF(point.x - 22, point.y + 10),
			&sfLeftTop, &brushMeasureInfo);
	}


	return;
}

void ImageRetina::drawTrackingTarget(CDC *pDC)
{
	if (!m_showTrackingTarget) {
		return;
	}

	// 기능 없음
	//drawTrackingTargetRetina(pDC);
	drawTrackingTargetDisc(pDC);
}

void ImageRetina::drawTrackingTargetRetina(CDC *pDC)
{
	if (!OctSystem::RetinaTrack::isTargetRegistered()) {
		return;
	}

	COLORREF oldColor;

	CPen penCircleOk, penCircleNotOk, penCross, *oldPen = nullptr;
	penCircleOk.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	penCircleNotOk.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	penCross.CreatePen(PS_SOLID, 2, RGB(32, 255, 32));
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	CBrush *oldBrush;
	oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	CRect rtClient;
	GetClientRect(&rtClient);

	float ratioX = (float)rtClient.Width() / 640.0f;
	float ratioY = (float)rtClient.Height() / 480.0f;

	int tx = -1;
	int ty = -1;

	// 표시 범위 30px 로 고정
	const int kDispRadius = 50.0f;
	//int radiusX = (int)(OctSystem::RetinaTrack::getTargetFocusDistance() * ratioX);
	//int radiusY = (int)(OctSystem::RetinaTrack::getTargetFocusDistance() * ratioX);
	int radiusX = (int)(kDispRadius * ratioX);
	int radiusY = (int)(kDispRadius * ratioX);
	if (OctSystem::RetinaTrack::getTargetCenter(tx, ty)) {
		tx = (int)(tx * ratioX);
		ty = (int)(ty * ratioY);

		if (OctSystem::RetinaTrack::isGrabHolding()) {
			oldPen = pDC->SelectObject(&penCircleNotOk);
		}
		else {
			oldPen = pDC->SelectObject(&penCircleOk);
		}

		// 범위
		pDC->Ellipse(CRect(tx - radiusX, ty - radiusY, tx + radiusX, ty + radiusY));
	}

	int dx = -1;
	int dy = -1;
	if (OctSystem::RetinaTrack::getDetectedCenter(dx, dy)) {
		dx = (int)(dx * ratioX);
		dy = (int)(dy * ratioY);

		// 고정 값이 범위 표시 ui 에 맞추기 위해 배율 조정
		float ratioForFixedCircle = kDispRadius / OctSystem::RetinaTrack::getTargetFocusDistance();

		// target center 를 중심으로 했을 때의 상대적인 반대 위치를 표시한다.
		// - 상대 위치를 구한다.
		int rdx = (dx - tx) * ratioForFixedCircle;
		int rdy = (dy - ty) * ratioForFixedCircle;
		// - x/y 축을 반전시킨 좌표를 구한다.
		dx = tx - rdx;
		dy = ty - rdy;

		// 십자가
		oldPen = pDC->SelectObject(&penCross);
		pDC->MoveTo(dx - radiusX, dy);
		pDC->LineTo(dx + radiusX, dy);
		pDC->MoveTo(dx, dy - radiusY);
		pDC->LineTo(dx, dy + radiusY);

		// 위쪽으로 벗어 났을 때, 십자가 위 rotate 화살 표 표시
		const int kArrawWingLen = 5;
		if (dy > ty + radiusY) {
			int left = dx - radiusX;
			int right = dx + radiusX;
			int bottom = dy - radiusY;
			int top = bottom - (int)((float)radiusX * 1.5f);
			POINT ptArcStart{ right, bottom - radiusY };
			POINT ptArcEnd{ left, bottom - radiusY };

			pDC->Arc(left, top, right, bottom, ptArcStart.x, ptArcStart.y, ptArcEnd.x, ptArcEnd.y);

			POINT ptArrPoint = { ptArcStart.x - 2, ptArcStart.y + 2 };
			pDC->MoveTo(ptArrPoint);
			pDC->LineTo(ptArrPoint.x, ptArrPoint.y - kArrawWingLen);
			pDC->MoveTo(ptArrPoint);
			pDC->LineTo(ptArrPoint.x - kArrawWingLen, ptArrPoint.y);
		}
		// 아래쪽으로 벗어 났을 때, 십자가 아래 rotate 화살 표 표시
		else if (dy < ty - radiusY)
		{
			int left = dx - radiusX;
			int right = dx + radiusX;
			int top = dy + radiusY;
			int bottom = top + (int)((float)radiusX * 1.5f);
			POINT ptArcEnd{ right, top + radiusY };
			POINT ptArcStart{ left, top + radiusY };

			pDC->Arc(left, top, right, bottom, ptArcStart.x, ptArcStart.y, ptArcEnd.x, ptArcEnd.y);

			POINT ptArrPoint = { ptArcEnd.x - 2, ptArcEnd.y - 2 };
			pDC->MoveTo(ptArrPoint);
			pDC->LineTo(ptArrPoint.x, ptArrPoint.y + kArrawWingLen);
			pDC->MoveTo(ptArrPoint);
			pDC->LineTo(ptArrPoint.x - kArrawWingLen, ptArrPoint.y);
		}
	}

	if (oldPen != nullptr) {
		pDC->SelectObject(oldPen);
	}
	pDC->SelectObject(oldBrush);
	penCircleOk.DeleteObject();
	penCircleNotOk.DeleteObject();
	penCross.DeleteObject();
}

void ImageRetina::drawTrackingTargetDisc(CDC *pDC)
{
	const bool kCropped = true;
	const COLORREF kColorScanningRegion = RGB(255, 87, 79);
	const COLORREF kColorTargetOn = RGB(0, 255, 0);
	const COLORREF kColorTargetOff = RGB(255, 0, 0);
	const COLORREF kColorTargetOn2 = RGB(79, 255, 87); 
	const COLORREF kColorTargetOff2 = RGB(255, 87, 79);

	int cx = 0;
	int cy = 0;
	int w = 0;
	int h = 0;
	bool pending = true;
	float value;

	CRect rtClient;
	GetClientRect(&rtClient);

	auto radius = GlobalSettings::radiusOfRetinaROI();
	auto dispOffsetX = (rtClient.Width() - radius * 2) / 2;
	auto dispOffsetY = (rtClient.Height() - radius * 2) / 2;

	int oldMode = pDC->SetBkMode(TRANSPARENT);
	CBrush *oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	auto track = OctSystem::RetinaTracking::getInstance();
	if (track->getScanningRegionInfo(cx, cy, w, h, pending, kCropped)) {
		int cx_disp = cx + dispOffsetX;
		int cy_disp = cy + dispOffsetY;

		int left = cx_disp - w / 2;
		int top = cy_disp - h / 2;
		int right = left + w;
		int bottom = top + h;

		CPen pen;
		int grabIndex = track->getGrabbingImageIndex();
		int imageIndex = grabIndex / m_overlaps;

		if (!pending) {
			pen.CreatePen(PS_SOLID, 1, kColorTargetOn2);
		}
		else {
			pen.CreatePen(PS_SOLID, 1, kColorScanningRegion);
		}

		//if (pending) {
		//	pen.CreatePen(PS_SOLID, 1, kColorScanningRegion);
		//	/*
		//	if (grabIndex == 0) {
		//		pen.CreatePen(PS_SOLID, 1, kColorTargetOn);
		//	}
		//	else {
		//		pen.CreatePen(PS_SOLID, 1, kColorTargetOff2);
		//	}
		//	*/
		//}
		//else {
		//	pen.CreatePen(PS_SOLID, 1, kColorTargetOn2);
		//}
		CPen* pOldPen = pDC->SelectObject(&pen);

		// region
		pDC->Rectangle(left, top, right, bottom);

		// scanning line
		// int imageIndex = OctSystem::RetinaTrack2::getLastImageIndexGrabbed() / m_overlaps;
		
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
		pDC->SelectObject(pOldPen);
		pen.DeleteObject();
	}

	/*
	// if (OctSystem::RetinaTrack2::getTrackingRegionInfo(cx, cy, w, h, value, kCropped)) {
	if (rett->getTrackingRegionInfo(cx, cy, w, h, value, kCropped)) {
		CPen pen;

		if (pending) {
			pen.CreatePen(PS_SOLID, 1, kColorTargetOff);
		}
		else {
			pen.CreatePen(PS_SOLID, 1, kColorTargetOn);
		}

		int cx_disp = cx + dispOffsetX;
		int cy_disp = cy + dispOffsetY;

		int left = cx_disp - w / 2;
		int top = cy_disp - h / 2;
		int right = left + w;
		int bottom = top + h;

		CPen* pOldPen = pDC->SelectObject(&pen);
		pDC->Rectangle(left, top, right, bottom);

		// finish
		pDC->SelectObject(pOldPen);
		pen.DeleteObject();
	}
	*/

	// restore
	pDC->SelectObject(oldBrush);
	pDC->SetBkMode(oldMode);
}
