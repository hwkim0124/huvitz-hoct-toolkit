// ViewColorCamera.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewColorCamera.h"

#include "CppUtil2.h"
#include "OctSystem2.h"
#include "OctResult2.h"

using namespace CppUtil;
using namespace OctDevice;
using namespace OctSystem;
using namespace std;

struct ViewColorCamera::ViewColorCameraImpl
{
	CvImage image;
	CvImage frame;

	int width;
	int height;
	int flipMode;

	float wbParam1;
	float wbParam2;
	bool useAwb;
	bool useRadialCorrection;
	bool useRemoveReflectionLight;
	bool useMaskROI;
	bool alignGuide;
	bool initiated;	
	bool useLiveMode;
	bool circleGuide;

	int imagePosX = -1;
	int imagePosY = -1;
	int imageR;
	int imageG;
	int imageB;
	int imageGray;
	int imageCenterMean;
	int imageOverBrightPixelCnt;

	std::pair<int, int> centerROI;
	int radiusROI;

	int lightRadius;
	int shadowRadius;
	int peripheryRadius;
	int overBrightThrehold;

	std::vector <std::pair<unsigned int, unsigned int>> pixels;

	CPoint points[2];
	int numPoints;

	ColorCameraImageCallback cbImage;
	ColorCameraFrameCallback cbFrame;

	ViewColorCameraImpl() 
		: width(0), height(0), wbParam1(1.0), wbParam2(1.0), useAwb(false), useMaskROI(false), useLiveMode(false),
			useRadialCorrection(false), useRemoveReflectionLight(false), circleGuide(false), alignGuide(true), flipMode(0), numPoints(0),
			initiated(false)
	{

	}
};


// ViewColorCamera

IMPLEMENT_DYNAMIC(ViewColorCamera, CStatic)



ViewColorCamera::ViewColorCamera()
	: d_ptr(make_unique<ViewColorCameraImpl>())
{
	getImpl().cbImage = std::bind(&ViewColorCamera::callbackCameraImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	getImpl().cbFrame = std::bind(&ViewColorCamera::callbackCameraFrame, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);

}


ViewColorCamera::~ViewColorCamera()
{
}


void ViewColorCamera::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);
	
	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	// Copy the bits from the memory DC into the current DC.
	float ratio = (float)getHeight() / getImageHeight();
	int width = (int)(getImageWidth() * ratio);
	int height = (int)(getImageHeight() * ratio);

	int sx = (getWidth() - width) / 2;
	int sy = (getHeight() - height) / 2;

	RECT rect;
	GetClientRect(&rect);

	SetStretchBltMode(hDC, HALFTONE);
	//StretchBlt(hDC, 0, 0, getWidth(), getHeight(), 
	//		hMemDC, 0, 0, getImageWidth(), getImageHeight(), SRCCOPY);

	// This FillRect() causes the flickering on display. 
	if (getImpl().image.isEmpty()) {
		FillRect(hDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	else {
		StretchBlt(hDC, sx, sy, width, height,
			hMemDC, 0, 0, getImageWidth(), getImageHeight(), SRCCOPY);
	}

	drawPixelCoords(pDC);
	drawAlignGuide(pDC);
	drawPixelsPitch(pDC);
	drawImagePosition(pDC);
	if (!getImpl().useLiveMode){
		drawCircleGuide(pDC);
	}

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
	return;
}


void ViewColorCamera::drawCircleGuide(CDC * pDC)
{
	if (!getImpl().circleGuide) {
		return;
	}

	CPen pen, pen2, pen3, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	oldPen = pDC->SelectObject(&pen);

	int w = getWidth();
	int h = getHeight();
	int cx = w / 2;
	int cy = h / 2;

	float wRatio = 960.0f / (float)getImageWidth();
	float hRatio = 750.0f / (float)getImageHeight();
	float wLightRatio = getImpl().lightRadius * wRatio;
	float hLightRatio = getImpl().lightRadius * hRatio;
	float wShadowRatio = getImpl().shadowRadius * wRatio;
	float hShadowRatio = getImpl().shadowRadius * hRatio;
	float wPeripheryRatio = getImpl().peripheryRadius * wRatio;
	float hPeripheryRatio = getImpl().peripheryRadius * hRatio;

	CBrush* oldBrush = (CBrush*) pDC->SelectStockObject(NULL_BRUSH);
	pDC->Ellipse(cx - wLightRatio, cy - hLightRatio, cx + wLightRatio, cy + hLightRatio);

	pen2.CreatePen(PS_SOLID, 1, RGB(32, 32, 255));
	oldPen = pDC->SelectObject(&pen2);
	pDC->Ellipse(cx - wShadowRatio, cy - hShadowRatio, cx + wShadowRatio, cy + hShadowRatio);

	pen3.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	oldPen = pDC->SelectObject(&pen3);
	pDC->Ellipse(cx - wPeripheryRatio, cy - hPeripheryRatio, cx + wPeripheryRatio, cy + hPeripheryRatio);

//	pDC->Ellipse(cx - 100, cy - 100, cx + 100, cy + 100);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pen2.DeleteObject();
	pen3.DeleteObject();

	return;
}


void ViewColorCamera::drawPixelCoords(CDC * pDC)
{
	if (getImpl().pixels.size() <= 0) {
		return;
	}

	CBrush *oldBrush;
	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 2, RGB(32, 255, 32));
	oldPen = pDC->SelectObject(&pen);
	oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	for (auto pixel : getImpl().pixels) {
		int cx = pixel.first / 4;
		int cy = pixel.second / 4;
		pDC->Rectangle(CRect(cx - 5, cy - 5, cx + 5, cy + 5));
	}
	
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pen.DeleteObject();
	return;
}


void ViewColorCamera::drawAlignGuide(CDC * pDC)
{
	if (!getImpl().alignGuide) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	oldPen = pDC->SelectObject(&pen);

	int w = getWidth();
	int h = getHeight();

	float wRatio = (float)getWidth() / 960.0f;
	float hRatio = (float)getHeight() / 750.0f;

	pDC->MoveTo(w / 2, 0);
	pDC->LineTo(w / 2, h - 1);
	pDC->MoveTo(0, h / 2);
	pDC->LineTo(w - 1, h / 2);

	int cx = w / 2;
	int cy = h / 2;

	for (int i = 1; i <= 5; i++) {
		pDC->MoveTo(cx + i * 100 * wRatio, cy - 5 * hRatio);
		pDC->LineTo(cx + i * 100 * wRatio, cy + 5 * hRatio);
		pDC->MoveTo(cx - i * 100 * wRatio, cy - 5 * hRatio);
		pDC->LineTo(cx - i * 100 * wRatio, cy + 5 * hRatio);
	}

	for (int i = 1; i <= 5; i++) {
		pDC->MoveTo(cx - 5 * wRatio, cy + i * 100 * hRatio);
		pDC->LineTo(cx + 5 * wRatio, cy + i * 100 * hRatio);
		pDC->MoveTo(cx - 5 * wRatio, cy - i * 100 * hRatio);
		pDC->LineTo(cx + 5 * wRatio, cy - i * 100 * hRatio);
	}

	/*
	CBrush* oldBrush = (CBrush*) pDC->SelectStockObject(NULL_BRUSH);
	pDC->Ellipse(cx - 50, cy - 50, cx + 50, cy + 50);
	pDC->Ellipse(cx - 100, cy - 100, cx + 100, cy + 100);
	pDC->SelectObject(oldBrush);
	*/

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ViewColorCamera::drawPixelsPitch(CDC * pDC)
{
	if (d_ptr->numPoints <= 0) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 128, 128));
	oldPen = pDC->SelectObject(&pen);

	pDC->MoveTo(d_ptr->points[0].x-5, d_ptr->points[0].y);
	pDC->LineTo(d_ptr->points[0].x+5, d_ptr->points[0].y);
	pDC->MoveTo(d_ptr->points[0].x, d_ptr->points[0].y-10);
	pDC->LineTo(d_ptr->points[0].x, d_ptr->points[0].y+10);

	if (d_ptr->numPoints > 1) {
		pDC->MoveTo(d_ptr->points[1].x - 5, d_ptr->points[1].y);
		pDC->LineTo(d_ptr->points[1].x + 5, d_ptr->points[1].y);
		pDC->MoveTo(d_ptr->points[1].x, d_ptr->points[1].y - 10);
		pDC->LineTo(d_ptr->points[1].x, d_ptr->points[1].y + 10);

		pDC->MoveTo(d_ptr->points[0]);
		pDC->LineTo(d_ptr->points[1]);
	}

	float wRatio = (float)getImageWidth() / (float)getWidth();
	float hRatio = (float)getImageHeight() / (float)getHeight();

	CFont font, *oldFont;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(255, 128, 128));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;
	auto width = getWidth();

	text.Format(_T("Position-1: %d, %d"), (int)(d_ptr->points[0].x*wRatio), (int)(d_ptr->points[0].y*hRatio));
	rect = CRect(width - 250, 20, width - 20, 60);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	if (d_ptr->numPoints > 1) {
		text.Format(_T("Position-2: %d, %d"), (int)(d_ptr->points[1].x*wRatio), (int)(d_ptr->points[1].y*hRatio));
		rect = CRect(width - 250, 40, width - 20, 80);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

		int pitch = (int)(abs(d_ptr->points[1].x - d_ptr->points[0].x)*wRatio);
		text.Format(_T("Pixels Pitch: %d (P = %.2f)"), pitch, (float)1700 / (float)pitch);
		rect = CRect(width - 250, 60, width - 20, 100);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewColorCamera::drawImagePosition(CDC * pDC)
{
	if (d_ptr->initiated && d_ptr->imagePosX >= 0 && d_ptr->imagePosY >= 0) {

	}
	else {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 128, 128));
	oldPen = pDC->SelectObject(&pen);

	CFont font, *oldFont;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(255, 128, 128));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;

	text.Format(_T("Image Point: %d, %d"), (int)(d_ptr->imagePosX), (int)(d_ptr->imagePosY));
	rect = CRect(50, 20, 400, 60);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	text.Format(_T("RGB Values: %d, %d, %d"), d_ptr->imageR, d_ptr->imageG, d_ptr->imageB);
	rect = CRect(50, 40, 400, 80);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	text.Format(_T("Gray Value: %d"), d_ptr->imageGray);
	rect = CRect(50, 60, 400, 100);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	text.Format(_T("Center Mean Gray: %d"), d_ptr->imageCenterMean);
	rect = CRect(50, 100, 400, 140);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	text.Format(_T("Over Bright Pix: %d"), d_ptr->imageOverBrightPixelCnt);
	rect = CRect(50, 120, 400, 160);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewColorCamera::setViewSize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int ViewColorCamera::getWidth(void) const
{
	return getImpl().width;
}


int ViewColorCamera::getHeight(void) const
{
	return getImpl().height;
}


int ViewColorCamera::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int ViewColorCamera::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}


int ViewColorCamera::getFrameWidth(void) const
{
	return getImpl().frame.getWidth();
}


int ViewColorCamera::getFrameHeight(void) const
{
	return getImpl().frame.getHeight();
}


unsigned char * ViewColorCamera::getFrameData(void) const
{
	if (d_ptr->frame.isEmpty()) {
		return nullptr;
	}
	return d_ptr->frame.getBitsData();
}

CvImage ViewColorCamera::getOriginalImage(void) const
{
	return getImpl().image;
}

bool ViewColorCamera::processWhiteBalance(float * param1, float * param2)
{
	float radius = 0.6f;
	bool result =  getImpl().image.processGrayWorld(radius, param1, param2);
	return result;
}


void ViewColorCamera::setWhiteBalanceParameters(float param1, float param2)
{
	getImpl().wbParam1 = param1;
	getImpl().wbParam2 = param2;
	return;
}


bool ViewColorCamera::useAutoWhiteBalance(bool isset, bool flag)
{
	if (isset) {
		getImpl().useAwb = flag;
	}
	return getImpl().useAwb;
}


bool ViewColorCamera::useRadialColorCorrection(bool isset, bool flag)
{
	if (isset) {
		getImpl().useRadialCorrection = flag;
	}
	return getImpl().useRadialCorrection;
}


bool ViewColorCamera::useRemoveReflectionLight(bool isset, bool flag, int lightRadius, int shadowRadius, int peripheryRadius)
{
	if (isset) {
		getImpl().useRemoveReflectionLight = flag;
		getImpl().lightRadius = lightRadius;
		getImpl().shadowRadius = shadowRadius;
		getImpl().peripheryRadius = peripheryRadius;
	}
	return getImpl().useRemoveReflectionLight;
}

void ViewColorCamera::setOverBrightPixelThreshold(int threshold)
{
	getImpl().overBrightThrehold = threshold;

	return;
}

bool ViewColorCamera::useMaskROI(bool isset, bool flag, int x, int y, int radius)
{
	if (isset) {
		getImpl().useMaskROI = flag;
		if (flag) {
			getImpl().centerROI = std::pair<int, int>(x, y);
			getImpl().radiusROI = radius;
		}
	}
	return getImpl().useMaskROI;
}

bool ViewColorCamera::useLiveMode(bool isset, bool flag)
{
	if (isset) {
		getImpl().useLiveMode = flag;
	}
	return getImpl().useLiveMode;
}

void ViewColorCamera::setPixelCoords(std::vector<std::pair<unsigned int, unsigned int>> coords)
{
	getImpl().pixels = coords;
	updateWindow();
	return;
}


void ViewColorCamera::clearPixelCoords(void)
{
	getImpl().pixels.clear();
	return;
}


void ViewColorCamera::showAlignGuide(bool flag)
{
	getImpl().alignGuide = flag;
	return;
}

void ViewColorCamera::showCircleGuide(bool flag)
{
	getImpl().circleGuide = flag;
	return;
}

bool ViewColorCamera::loadFrameData(CString filename)
{
	/*
	if (!filename.IsEmpty()) {
		if (getImpl().frame.loadFileAsYml(wtoa(filename))) {
			unsigned char* idata = const_cast<unsigned char*>(getImpl().frame.getBitsData());
			getImpl().image.fromBayer16s(idata, getImpl().frame.getWidth(), getImpl().frame.getHeight(), 
											CvImage::FlipMode::NONE, 0.25);
			updateWindow();
			return true;
		}
	}
	*/
	return false;
}


bool ViewColorCamera::saveFrameData(CString& filename)
{
	/*
	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("COLOR_%y%m%d_%H%M%S.dng"));
	}
	if (!getImpl().frame.isEmpty()) {
		filename = path;
		return getImpl().frame.saveFileAsRaw(wtoa(path));
	}
	*/
	/*
	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("COLOR_%y%m%d_%H%M%S.yml"));
	}
	if (!getImpl().frame.isEmpty()) {
		filename = path;
		return getImpl().frame.saveFileAsYml(wtoa(path));
	}
	*/
	CString dirname = _T(".//export");
	CreateDirectory(dirname, NULL);


	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("COLOR_%y%m%d_%H%M%S.raw"));
	}
	if (!getImpl().frame.isEmpty()) {
		path = dirname + _T("//") + path;
		filename = path;
		getImpl().frame.flipVertical();
		return getImpl().frame.saveFileAsBinary(wtoa(path));
	}
	return false;
}


bool ViewColorCamera::saveImage(CString& filename)
{
	CString dirname = _T(".//export");
	CreateDirectory(dirname, NULL);

	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("COLOR_%y%m%d_%H%M%S.png"));
	}
	if (!getImpl().image.isEmpty()) {
		path = dirname + _T("//") + path;
		filename = path;
		return getImpl().image.saveFile(wtoa(path), true);
	}
	return false;
}

bool ViewColorCamera::saveCalibrationImage(CString& filename)
{
	return getImpl().image.saveFile(wtoa(filename), true);
}

void ViewColorCamera::updateFrameImage(void)
{
	auto data = getImpl().frame.getBitsData();
	if (data == nullptr) {
		return;
	}

	clearPixelCoords();
	processCameraData(data, d_ptr->frame.getWidth(), d_ptr->frame.getHeight(), d_ptr->flipMode);
	updateWindow();
	return;
}


ColorCameraFrameCallback * ViewColorCamera::getFrameCallbackFunction(void)
{
	return &getImpl().cbFrame;
}


ColorCameraImageCallback * ViewColorCamera::getImageCallbackFunction(void)
{
	return &getImpl().cbImage;
}


ViewColorCamera::ViewColorCameraImpl & ViewColorCamera::getImpl(void) const
{
	return *d_ptr;
}


void ViewColorCamera::callbackCameraImage(unsigned char * data, unsigned int width, unsigned int height, unsigned int frameCount)
{
	if (data == nullptr) {
		return;
	}

	/*
	if (getImpl().image.fromRGB24(data, width, height)) {
		if (useAutoWhiteBalance()) {
			float param1 = getImpl().wbParam1;
			float param2 = getImpl().wbParam2;
			getImpl().image.balanceColorByGrayWorld(param1, param2);
		}

		if (useRadialColorCorrection()) {
			auto xs = GlobalSettings::levelCorrectionCoordsX();
			auto ys = GlobalSettings::levelCorrectionCoordsY();
			getImpl().image.processRadialColorCorrection(xs, ys);
		}
		updateWindow();
	}
	*/


	return;
}


void ViewColorCamera::callbackCameraFrame(unsigned char * data, unsigned int width, unsigned int height, unsigned int frameCount, unsigned int flipMode)
{
	if (data == nullptr) {
		return;
	}

	if (getImpl().frame.fromRaw16(data, width, height)) {
	}

	d_ptr->flipMode = flipMode;
	updateFrameImage();
	d_ptr->initiated = true;

	return;
}


void ViewColorCamera::processCameraData(unsigned char* data, unsigned int width, unsigned int height, unsigned int flipMode)
{
	// convertBayer16ToRGB16()
	bool useFundusFILR = GlobalSettings::useFundusFILR_Enable();

	CvImage::FlipMode mode;
	switch (flipMode) {
	case 0:
		mode = CvImage::FlipMode::NONE; break;
	case 1:
		mode = CvImage::FlipMode::VERTICAL; break;
	case 2:
		mode = CvImage::FlipMode::HORIZONTAL; break;
	case 3 :
		mode = CvImage::FlipMode::BOTH; break;
	}

	// Raw camera pixel is 10 bit data.
	getImpl().image.fromBayer16s(data, width, height, mode, useFundusFILR);

	// XXX wrap it around with optiona
	getImpl().image.denoise();

	if (useAutoWhiteBalance()) {
		float param1 = getImpl().wbParam1;
		float param2 = getImpl().wbParam2;
		getImpl().image.balanceColorByGrayWorld(param1, param2);
	}

	if (useRadialColorCorrection()) {
		if (GlobalSettings::useFundusFILR_Enable()) {
			auto xs = GlobalSettings::levelCorrectionCoordsX_FILR();
			auto ys = GlobalSettings::levelCorrectionCoordsY_FILR();
			auto roi_pos0 = GlobalSettings::startOfFundusROI();
			auto roi_pos1 = GlobalSettings::closeOfFundusROI();
			getImpl().image.processRadialColorCorrection(xs, ys, roi_pos0, roi_pos1, useFundusFILR);
		}
		else {
			auto xs = GlobalSettings::levelCorrectionCoordsX();
			auto ys = GlobalSettings::levelCorrectionCoordsY();
			auto roi_pos0 = GlobalSettings::startOfFundusROI();
			auto roi_pos1 = GlobalSettings::closeOfFundusROI();
			getImpl().image.processRadialColorCorrection(xs, ys, roi_pos0, roi_pos1);
		}
		
    }

	// convertRGB16ToRGB8()
	if (useFundusFILR) {
		if (getImpl().useLiveMode) {
			getImpl().image.flipVertical();
			const double scale_ratio = 0.0625;
			getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0);
		}
		else {
			const double scale_ratio = 0.0125;
			getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0);
		}
	}
	else {
		const double scale_ratio = 0.0625;
		getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0);
	}

	if (useMaskROI()) {
		auto center = d_ptr->centerROI;
		auto radius = d_ptr->radiusROI;
		getImpl().image.applyMaskROI(center, radius, false);
	}

	if (useRemoveReflectionLight()) {
		removeReflectionLight(getImpl().lightRadius, getImpl().shadowRadius, getImpl().peripheryRadius);
	}

	getImpl().imageCenterMean = calculateCenterMean(getImpl().lightRadius);
	getImpl().imageOverBrightPixelCnt = calculateOverBrightPixel(getImpl().lightRadius, getImpl().overBrightThrehold);
	return;
}

void ViewColorCamera::removeReflectionLight(int centerRadius, int shadowRadius, int peripheryRadius)
{
	getImpl().image.applyRemoveReflectionLight(centerRadius, shadowRadius, peripheryRadius);
	return;
}

int ViewColorCamera::calculateCenterMean(int lightRadius)
{
	int centerMean = getImpl().image.calculateCenterMean(lightRadius);

	return centerMean;
}

int ViewColorCamera::calculateOverBrightPixel(int lightRadius, int threshold)
{
	int overPixelCnt = getImpl().image.calculateOverBrightPixel(lightRadius, threshold);

	return overPixelCnt;
}


BEGIN_MESSAGE_MAP(ViewColorCamera, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// ViewColorCamera message handlers
void ViewColorCamera::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	updateWindow();
	return;
}


void ViewColorCamera::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (d_ptr->numPoints >= 2) {
		d_ptr->numPoints = 0;
	}

	d_ptr->points[d_ptr->numPoints] = point;
	d_ptr->numPoints++;

	updateWindow();

	CStatic::OnLButtonDown(nFlags, point);
}


void ViewColorCamera::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (d_ptr->numPoints > 0) {
		d_ptr->numPoints = 0;
		updateWindow();
	}

	CStatic::OnRButtonDown(nFlags, point);
}


void ViewColorCamera::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!getImpl().image.isEmpty() && d_ptr->initiated) {
		float ratio = (float)getHeight() / getImageHeight();
		int x = point.x;
		int y = point.y;
		int img_x = (int)(point.x * (1.0f / ratio));
		int img_y = (int)(point.y * (1.0f / ratio));

		d_ptr->imagePosX = img_x;
		d_ptr->imagePosY = img_y;

		CDC* pDC = GetDC();
		if (pDC) {
			COLORREF color = pDC->GetPixel(point);
			int r = (int)GetRValue(color);
			int g = (int)GetGValue(color);
			int b = (int)GetBValue(color);
			int gray = r*0.2126 + g*0.7152 + b*0.0722;

			/*
			auto r = getImpl().image.getRedAt(img_x, img_y);
			auto g = getImpl().image.getGreenAt(img_x, img_y);
			auto b = getImpl().image.getBlueAt(img_x, img_y);
			*/

			d_ptr->imageR = r;
			d_ptr->imageG = g;
			d_ptr->imageB = b;
			d_ptr->imageGray = gray;
		}
		
		updateWindow();
		// LogT() << "Position: " << img_x << ", " << img_y << ", rgb " << r << ", " << g << ", " << b;
	}

	CStatic::OnMouseMove(nFlags, point);
}




