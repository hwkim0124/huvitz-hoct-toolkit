#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewScrollColorCamera.h"

#include "CppUtil2.h"
#include "OctSystem2.h"
#include "OctResult2.h"

using namespace CppUtil;
using namespace OctDevice;
using namespace OctSystem;
using namespace std;


struct ViewScrollColorCamera::ViewScrollColorCameraImpl
{
	CvImage image;
	CvImage frame;

	int viewWidth;
	int viewHeight;

	int minWidth;
	int minHeight;
	int maxWidth;
	int maxHeight;

	int timeCount;

	int flipMode;

	float wbParam1;
	float wbParam2;
	bool useAwb;
	bool useRadialCorrection;
	bool useMaskROI;
	bool initiated;
	bool useLiveMode;

	std::pair<int, int> centerROI;
	int radiusROI;

	ColorCameraImageCallback cbImage;
	ColorCameraFrameCallback cbFrame;

	ViewScrollColorCameraImpl()
		: wbParam1(1.0), wbParam2(1.0), useAwb(false), useMaskROI(false), useLiveMode(false),
		useRadialCorrection(false), flipMode(0), initiated(false), timeCount(0), 
		viewWidth(0), viewHeight(0), minWidth(0), minHeight(0), maxWidth(0), maxHeight(0)
	{

	}
};


// ViewColorCamera

IMPLEMENT_DYNCREATE(ViewScrollColorCamera, CScrollView)


ViewScrollColorCamera::ViewScrollColorCamera()
	: d_ptr(make_unique<ViewScrollColorCameraImpl>())
{
	getImpl().cbImage = std::bind(&ViewScrollColorCamera::callbackCameraImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	getImpl().cbFrame = std::bind(&ViewScrollColorCamera::callbackCameraFrame, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
}


ViewScrollColorCamera::~ViewScrollColorCamera()
{
	//delete this;
}


BEGIN_MESSAGE_MAP(ViewScrollColorCamera, CScrollView)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// ViewColorCamera 그리기입니다.

void ViewScrollColorCamera::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
}

BOOL ViewScrollColorCamera::PreCreateWindow(CREATESTRUCT& cs)
{
	CSize sizeTotal;
	sizeTotal = CSize(getImpl().viewWidth, getImpl().viewHeight);
	//	//// TODO: 이 뷰의 전체 크기를 계산합니다.
	SetScrollSizes(MM_TEXT, sizeTotal);
	return CScrollView::PreCreateWindow(cs);
}

BOOL ViewScrollColorCamera::OnEraseBkgnd(CDC* pDC)
{
	//CBrush br(RGB(255, 255, 255));
	//FillORect(pDC, &br);
	return FALSE;
	return CScrollView::OnEraseBkgnd(pDC);
}

void ViewScrollColorCamera::OnDraw(CDC* pDC)
{
	updateWindow();
}

int ViewScrollColorCamera::getViewWidth(void)
{
	return getImpl().viewWidth;
}

int ViewScrollColorCamera::getViewHeight(void)
{
	return getImpl().viewHeight;
}

int ViewScrollColorCamera::getImageWidth(void)
{
	return getImpl().image.getWidth();
}

int ViewScrollColorCamera::getFrameWidth(void)
{
	return getImpl().frame.getWidth();
}

int ViewScrollColorCamera::getImageHeight(void)
{
	return getImpl().image.getHeight();
}

void ViewScrollColorCamera::setViewWidth(int Width)
{
	if (Width > getImpl().maxWidth) {
		getImpl().viewWidth = getImpl().maxWidth;
	}
	else if (Width < getImpl().minWidth) {
		getImpl().viewWidth = getImpl().minWidth;
	}
	else {
		getImpl().viewWidth = Width;
	}
}

void ViewScrollColorCamera::setViewHeight(int Height)
{
	if (Height > getImpl().maxHeight) {
		getImpl().viewHeight = getImpl().maxHeight;
	}
	else if (Height < getImpl().minHeight) {
		getImpl().viewHeight = getImpl().minHeight;
	}
	else {
		getImpl().viewHeight = Height;
	}
}

void ViewScrollColorCamera::setInitSize(int Width, int Height)
{
	getImpl().viewWidth = Width / 2;
	getImpl().maxWidth = Width;
	getImpl().minWidth = Width / 4;
	getImpl().viewHeight = Height / 2;
	getImpl().maxHeight = Height;
	getImpl().minHeight = Height / 4;

	CSize sizeTotal;
	sizeTotal = CSize(getImpl().viewWidth, getImpl().viewHeight);
	SetScrollSizes(MM_TEXT, sizeTotal);
}

int ViewScrollColorCamera::getTimeCount(void)
{
	return getImpl().timeCount;
}

void ViewScrollColorCamera::setTimeCount(int t)
{
	getImpl().timeCount = t;
}

ViewScrollColorCamera::ViewScrollColorCameraImpl & ViewScrollColorCamera::getImpl(void) const
{
	return *d_ptr;
}


void ViewScrollColorCamera::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);
	SetStretchBltMode(hDC, HALFTONE);

	CRect rect;
	GetClientRect(&rect);

	float ratio = (float)getImageWidth() / getViewWidth();

	int xStart;
	int yStart;

	int nVertScroll;
	int nHorzScroll;

	SetStretchBltMode(hDC, HALFTONE);

	if (getImpl().image.isEmpty()) {
		FillRect(hDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}

	else {
		if ((getViewWidth() <= rect.Width()) && (getViewHeight() <= rect.Height())) {
			xStart = (rect.Width() - getViewWidth()) / 2;
			yStart = (rect.Height() - getViewHeight()) / 2;
			StretchBlt(hDC, xStart, yStart, getViewWidth(), getViewHeight(),
				hMemDC, 0, 0, getImageWidth(), getImageHeight(), SRCCOPY);
		}
		else if ((getViewWidth() <= rect.Width()) && (getViewHeight() > rect.Height())) {
			xStart = (rect.Width() - getViewWidth()) / 2;
			yStart = 0;
			nVertScroll = GetScrollPos(SB_VERT) * ratio;
			StretchBlt(hDC, xStart, yStart, getViewWidth(), getViewHeight(),
				hMemDC, 0, nVertScroll, getImageWidth(), getImageHeight(), SRCCOPY);
		}

		else if ((getViewWidth() > rect.Width()) && (getViewHeight() <= rect.Height())) {
			xStart = 0;
			yStart = (rect.Height() - getViewHeight()) / 2;
			nHorzScroll = GetScrollPos(SB_HORZ) * ratio;
			StretchBlt(hDC, xStart, yStart, getViewWidth(), getViewHeight(),
				hMemDC, nHorzScroll, 0, getImageWidth(), getImageHeight(), SRCCOPY);
		}
		else {
			xStart = 0;
			yStart = 0;
			nVertScroll = GetScrollPos(SB_VERT) * ratio;
			nHorzScroll = GetScrollPos(SB_HORZ) * ratio;
			StretchBlt(hDC, xStart, yStart, getViewWidth(), getViewHeight(),
				hMemDC, nHorzScroll, nVertScroll, getImageWidth(), getImageHeight(), SRCCOPY);
		}
	}

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
}

void ViewScrollColorCamera::setWhiteBalanceParameters(float param1, float param2)
{
	getImpl().wbParam1 = param1;
	getImpl().wbParam2 = param2;
	return;
}


bool ViewScrollColorCamera::useAutoWhiteBalance(bool isset, bool flag)
{
	if (isset) {
		getImpl().useAwb = flag;
	}
	return getImpl().useAwb;
}


bool ViewScrollColorCamera::useRadialColorCorrection(bool isset, bool flag)
{
	if (isset) {
		getImpl().useRadialCorrection = flag;
	}
	return getImpl().useRadialCorrection;
}


bool ViewScrollColorCamera::useMaskROI(bool isset, bool flag, int x, int y, int radius)
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

bool ViewScrollColorCamera::useLiveMode(bool isset, bool flag)
{
	if (isset) {
		getImpl().useLiveMode = flag;
	}
	return getImpl().useLiveMode;
}



void ViewScrollColorCamera::updateFrameImage(void)
{
	auto data = getImpl().frame.getBitsData();
	if (data == nullptr) {
		return;
	}

	processCameraData(data, d_ptr->frame.getWidth(), d_ptr->frame.getHeight(), d_ptr->flipMode);
	updateWindow();

	return;
}


ColorCameraFrameCallback * ViewScrollColorCamera::getFrameCallbackFunction(void)
{
	return &getImpl().cbFrame;
}


ColorCameraImageCallback * ViewScrollColorCamera::getImageCallbackFunction(void)
{
	return &getImpl().cbImage;
}

void ViewScrollColorCamera::callbackCameraImage(unsigned char * data, unsigned int width, unsigned int height, unsigned int flipMode)
{
	if (data == nullptr) {
		return;
	}
	return;
}


void ViewScrollColorCamera::callbackCameraFrame(unsigned char * data, unsigned int width, unsigned int height, unsigned int frameCount, unsigned int flipMode)
{
	if (data == nullptr) {
		return;
	}

	if (getImpl().frame.fromRaw16(data, width, height)) {
	}
	t = clock();
	d_ptr->flipMode = flipMode;
	updateFrameImage();
	d_ptr->initiated = true;
	t = clock() - t;
	setTimeCount(t);

	return;
}


void ViewScrollColorCamera::processCameraData(unsigned char* data, unsigned int width, unsigned int height, unsigned int flipMode)
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
	case 3:
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
			const double scale_ratio = 0.0225;
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
	return;
}

void ViewScrollColorCamera::zoomIn(void)
{
	setViewWidth(getViewWidth() * 2);
	setViewHeight(getViewHeight() * 2);

	setScroll();

	updateWindow();
}

void ViewScrollColorCamera::zoomOut(void)
{
	//Invalidate(true);
	setViewWidth(getViewWidth() / 2);
	setViewHeight(getViewHeight() / 2);

	setScroll();

	CDC* pDC = GetDC();
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));
	updateWindow();
}

void ViewScrollColorCamera::setScroll(void)
{
	CSize sizeTotal;
	sizeTotal = CSize(getViewWidth(), getViewHeight());
	SetScrollSizes(MM_TEXT, sizeTotal);
}