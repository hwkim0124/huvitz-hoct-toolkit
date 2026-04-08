// ViewRetinaCamera.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewRetinaCamera.h"

#include "CppUtil2.h"
#include "OctSystem2.h"
#include "RetFocus2.h"

using namespace CppUtil;
using namespace OctDevice;
using namespace OctSystem;
using namespace RetFocus;
using namespace std;


struct ViewRetinaCamera::ViewRetinaCameraImpl
{
	CvImage image;
	int width;
	int height;

	int frameRate;
	int frameCount;
	int frameSec;

	int imageGray = 0;

	bool cameraInfo;
	bool alignGuide;
	bool focusStatus;
	bool splitWindows;
	bool splitPoints;
	bool circleMask;
	bool workingDots;
	bool trackTarget;
	bool trackFeature;

	float ratioX = 1.0f;
	float ratioY = 1.0f;

	RetinaCameraImageCallback callback;

	ViewRetinaCameraImpl()
		: width(IR_CAMERA_FRAME_WIDTH), height(IR_CAMERA_IMAGE_HEIGHT),
		frameRate(0), frameCount(0), frameSec(0),
		cameraInfo(false), alignGuide(false), focusStatus(false), 
		splitWindows(false), splitPoints(false), circleMask(false), 
		workingDots(false), trackTarget(false), trackFeature(false)
	{

	}

};


// ViewRetinaCamera

IMPLEMENT_DYNAMIC(ViewRetinaCamera, CStatic)

ViewRetinaCamera::ViewRetinaCamera()
	: d_ptr(make_unique<ViewRetinaCameraImpl>())
{
	getImpl().callback = std::bind(&ViewRetinaCamera::callbackCameraImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}

ViewRetinaCamera::~ViewRetinaCamera()
{
}


// ViewRetinaCamera message handlers

void ViewRetinaCamera::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);


	CBitmap bitmap, *oldBmp;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	oldBmp = (CBitmap*)pMemDC->SelectObject(&bitmap);
	pMemDC->PatBlt(0, 0, getWidth(), getHeight(), BLACKNESS);


	// Copy the bits from the memory DC into the current DC.
	// BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);

	float ratio = (float)getHeight() / (float)getImageHeight();
	int width = (int)(getImageWidth() * ratio);
	int height = (int)(getImageHeight() * ratio);

	int sx = (getWidth() - width) / 2;
	int sy = (getHeight() - height) / 2;

	HDC hMemDC2 = CreateCompatibleDC(hDC);
	CDC* pMemDC2 = CDC::FromHandle(hMemDC2);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC, true);
	HGDIOBJ hobj = SelectObject(hMemDC2, hbmp);

	SetStretchBltMode(hMemDC, HALFTONE);
	StretchBlt(hMemDC, sx, sy, width, height,
		hMemDC2, 0, 0, getImageWidth(), getImageHeight(), SRCCOPY);

	BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);

	drawAlignGuide(pDC);
	drawCameraInfo(pDC);
	drawFocusStatus(pDC);
	drawSplitPoints(pDC);
	drawSplitWindows(pDC);
	drawWorkingDots(pDC);
	drawOpticDiscRegion(pDC);
	drawTrackTarget(pDC);
	drawTrackFeature(pDC);

	SelectObject(hMemDC2, hobj);
	pMemDC2->DeleteDC();
	DeleteObject(hbmp);

	pMemDC->SelectObject(oldBmp);
	pMemDC->DeleteDC();
	
	ReleaseDC(pDC);
	return;
}


void ViewRetinaCamera::drawCameraInfo(CDC * pDC)
{
	if (!getImpl().cameraInfo) {
		return;
	}

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	str.Format(_T("Retina Ir %d fps"), getImpl().frameRate);

	auto board = Controller::getMainBoard();
	auto count = board->getRetinaIrCamera()->getErrorCount();
	if (count > 0) {
		str.Format(_T("Frame errors %d"), count);
	}

	CRect rect = CRect(10, getHeight() - 30, 180, getHeight()-10);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	str.Format(_T("Gray Value: %d"), d_ptr->imageGray);
	rect = CRect(10, getHeight() - 50, 180, getHeight() - 30);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewRetinaCamera::drawAlignGuide(CDC * pDC)
{
	if (!getImpl().alignGuide) {
		return;
	}

	CPen pen, pen2, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	oldPen = pDC->SelectObject(&pen);

	int w = getWidth();
	int h = getHeight();
	pDC->MoveTo(w / 2, 0);
	pDC->LineTo(w / 2, h-1);
	pDC->MoveTo(0, h / 2);
	pDC->LineTo(w-1, h / 2);

	int cx = w / 2;
	int cy = h / 2;

	for (int i = 1; i <= 3; i++) {
		pDC->MoveTo(cx + i * 100, cy - 5);
		pDC->LineTo(cx + i * 100, cy + 5);
		pDC->MoveTo(cx - i * 100, cy - 5);
		pDC->LineTo(cx - i * 100, cy + 5);
	}

	for (int i = 1; i <= 2; i++) {
		pDC->MoveTo(cx - 5, cy + i * 100);
		pDC->LineTo(cx + 5, cy + i * 100);
		pDC->MoveTo(cx - 5, cy - i * 100);
		pDC->LineTo(cx + 5, cy - i * 100);
	}

	auto center = GlobalSettings::splitFocusCenter();
	cx = (int)((center.first / 640.0f) * w);
	cy = (int)((center.second / 480.0f) * h);

	pen2.CreatePen(PS_SOLID, 1, RGB(32, 32, 255));
	pDC->SelectObject(&pen2);

	pDC->MoveTo(cx - 35, cy);
	pDC->LineTo(cx + 35, cy);
	pDC->MoveTo(cx, cy - 35);
	pDC->LineTo(cx, cy + 35);

	/*
	CBrush* oldBrush = (CBrush*) pDC->SelectStockObject(NULL_BRUSH);
	pDC->Ellipse(cx - 50, cy - 50, cx + 50, cy + 50);
	pDC->Ellipse(cx - 100, cy - 100, cx + 100, cy + 100);
	pDC->SelectObject(oldBrush);
	*/

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pen2.DeleteObject();
	return;
}


void ViewRetinaCamera::drawFocusStatus(CDC * pDC)
{
	if (!getImpl().focusStatus) {
		return;
	}

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	RetinaFocus2& focus = Controller::getRetinaAlign()->getRetinaFocus();
	if (focus.isSplitFocus()) {
		str.Format(_T("Split Offset: %.2f"), focus.getSplitOffset());
	}
	else {
		str.Format(_T("Split Offset: ---"));
	}

	CRect rect = CRect(getWidth() - 240, getHeight() - 60, getWidth(), getHeight() - 40);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewRetinaCamera::drawSplitPoints(CDC * pDC)
{
	if (!getImpl().splitPoints) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 32, 255);

	CPen pen1, pen2, *oldPen;
	pen1.CreatePen(PS_SOLID, 1, clr1);
	pen2.CreatePen(PS_SOLID, 1, clr2);
	oldPen = pDC->SelectObject(&pen1);
	CBrush *oldBrush; 

	oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	RetinaFocus2& focus = Controller::getRetinaAlign()->getRetinaFocus();
	RetinaFrame& frame = focus.getRetinaFrame();

	if (frame.isUpperSplit()) {
		int icx = (int)frame.getSplitUpperX();
		int icy = (int)frame.getSplitUpperY();
		pDC->MoveTo(icx - 2, icy);
		pDC->LineTo(icx + 2, icy);
		pDC->MoveTo(icx, icy - 2);
		pDC->LineTo(icx, icy + 2);

		int x1, y1, x2, y2;
		if (frame.getSplitWindow(0, x1, y1, x2, y2)) {
			pDC->Rectangle(x1, y1, x2, y2+1);
			// LogD() << "upper: " << x1 << ", " << x2 << ", " << y1 << ", " << y2;
		}
	}

	if (frame.isLowerSplit()) {
		int icx = (int)frame.getSplitLowerX();
		int icy = (int)frame.getSplitLowerY();
		pDC->MoveTo(icx - 2, icy);
		pDC->LineTo(icx + 2, icy);
		pDC->MoveTo(icx, icy - 2);
		pDC->LineTo(icx, icy + 2);

		int x1, y1, x2, y2;
		if (frame.getSplitWindow(1, x1, y1, x2, y2)) {
			pDC->Rectangle(x1, y1, x2, y2);
			// LogD() << "lower: " << x1 << ", " << x2 << ", " << y1 << ", " << y2;
		}
	}

	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pen1.DeleteObject();
	pen2.DeleteObject();
	return;
}


void ViewRetinaCamera::drawSplitWindows(CDC * pDC)
{
	if (!getImpl().splitWindows) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 32, 255);

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	RetinaFocus2& focus = Controller::getRetinaAlign()->getRetinaFocus();
	RetinaFrame& frame = focus.getRetinaFrame();

	CString cstr;
	CRect rect;

	if (frame.isUpperSplit()) {
		pDC->SetTextColor(clr1);
		int icx = (int)frame.getSplitUpperX();
		int icy = (int)frame.getSplitUpperY();
		cstr.Format(_T("x:%.2f,y:%.2f,fwhm:%.0f,snr:%.1f,peak:%.0f"), frame.getSplitUpperX(), frame.getSplitUpperY(),
			frame.getSplitUpperFwhm(), frame.getSplitUpperSNR(), frame.getSplitUpperPeak());
		rect.SetRect(icx - 160, icy - 36, icx + 160, icy - 12);
		pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
	}

	if (frame.isLowerSplit()) {
		pDC->SetTextColor(clr1);
		int icx = (int)frame.getSplitLowerX();
		int icy = (int)frame.getSplitLowerY();
		cstr.Format(_T("x:%.2f,y:%.2f,fwhm:%.0f,snr:%.1f,peak:%.0f"), frame.getSplitLowerX(), frame.getSplitLowerY(),
			frame.getSplitLowerFwhm(), frame.getSplitLowerSNR(), frame.getSplitLowerPeak());
		rect.SetRect(icx - 160, icy + 12, icx + 160, icy + 36);
		pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
	}

	pDC->SetBkMode(oldMode);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewRetinaCamera::drawWorkingDots(CDC * pDC)
{
	if (!getImpl().workingDots) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 255);

	CPen pen1, *oldPen;
	pen1.CreatePen(PS_SOLID, 1, clr1);
	oldPen = pDC->SelectObject(&pen1);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);

	RetinaFocus2& focus = Controller::getRetinaAlign()->getRetinaFocus();
	RetinaFrame& frame = focus.getRetinaFrame();

	CString cstr;
	CRect rect;
	float x, y;
	int size, mean;

	if (frame.getWorkingDot(0, x, y, size, mean)) {
		int icx = (int)x;
		int icy = (int)y;
		pDC->MoveTo(icx - 5, icy);
		pDC->LineTo(icx + 5, icy);
		pDC->MoveTo(icx, icy - 5);
		pDC->LineTo(icx, icy + 5);

		cstr.Format(_T("x:%.0f,y:%.0f,sz:%d,avg:%d"), x, y, size, mean);
		rect.SetRect(icx - 160, icy + 24, icx + 160, icy + 48);
		pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
	}

	if (frame.getWorkingDot(1, x, y, size, mean)) {
		int icx = (int)x;
		int icy = (int)y;
		pDC->MoveTo(icx - 5, icy);
		pDC->LineTo(icx + 5, icy);
		pDC->MoveTo(icx, icy - 5);
		pDC->LineTo(icx, icy + 5);

		cstr.Format(_T("x:%.0f,y:%.0f,sz:%d,avg:%d"), x, y, size, mean);
		rect.SetRect(icx - 160, icy + 24, icx + 160, icy + 48);
		pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
	}

	pDC->SelectObject(oldPen);
	pen1.DeleteObject();
	pDC->SetBkMode(oldMode);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewRetinaCamera::drawOpticDiscRegion(CDC * pDC)
{
	COLORREF clr1 = RGB(255, 32, 255);

	CPen pen1, *oldPen;
	pen1.CreatePen(PS_SOLID, 5, clr1);
	oldPen = pDC->SelectObject(&pen1);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	RetinaFocus2& focus = Controller::getRetinaAlign()->getRetinaFocus();
	RetinaFrame& frame = focus.getRetinaFrame();

	if (frame.isOpticDiscFound()) {
		int x1, y1, x2, y2;
		if (frame.getOpticDiscRegion(x1, y1, x2, y2)) {
			pDC->MoveTo(x1, y1);
			pDC->LineTo(x1, y2);
			pDC->LineTo(x2, y2);
			pDC->LineTo(x2, y1);
			pDC->LineTo(x1, y1);
		}
	}

	pDC->SelectObject(oldPen);
	pen1.DeleteObject();
	return;
}


void ViewRetinaCamera::drawTrackFeature(CDC* pDC)
{
	if (!getImpl().trackFeature) {
		return;
	}

	COLORREF clr1 = RGB(255, 160, 0);
	COLORREF clr2 = RGB(32, 255, 32);
	COLORREF clr3 = RGB(0, 96, 255);
	COLORREF oldColor;

	CPen pen1, pen2, pen3, * oldPen;
	pen1.CreatePen(PS_SOLID, 1, clr1);
	pen2.CreatePen(PS_SOLID, 1, clr2);
	pen3.CreatePen(PS_SOLID, 1, clr3);
	oldPen = pDC->SelectObject(&pen1);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont* oldFont = pDC->SelectObject(&font);

	CBrush* oldBrush;
	oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	int w = getWidth();
	int h = getHeight();
	
	auto* track = RetinaTracking::getInstance();
	if (track) {
		if (true) {
			float roi_mean = 0.0f, roi_stdev = 0.0f;
			float targ_mean = 0.0f, targ_stdev = 0.0f;
			float dx = 0.0f, dy = 0.0f;
			track->getFrameStatistics(roi_mean, roi_stdev, targ_mean, targ_stdev);

			CString cstr;
			CRect rect;
			cstr.Format(_T("roi: %d (%.2f), targ: %d (%.2f)"), (int)roi_mean, roi_stdev, (int)targ_mean, targ_stdev);
			rect.SetRect(10, 10, w, 10 + 20);

			oldColor = pDC->SetTextColor(clr2);
			pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);

			{ // if (track->isTargetRegistered()) {
				int fx, fy, fw, fh;
				float value = 0.0f;
				if (track->getTrackingRegionInfo2(fx, fy, fw, fh, value)) {
					pDC->SelectObject(&pen3);
					int ifx = (int)(fx * getImpl().ratioX);
					int ify = (int)(fy * getImpl().ratioY);
					int ifw = (int)(fw * getImpl().ratioX);
					int ifh = (int)(fh * getImpl().ratioY);
					pDC->Rectangle(ifx, ify, ifx + ifw, ify + ifh);
				}
			}

			{ // if (track->isTrackingPrepared()) {
				int sx, sy, sw, sh;
				bool pending = false;
				track->getScanningRegionInfo2(sx, sy, sw, sh, pending);

				if (pending) {
					pDC->SelectObject(&pen1);
				}
				else {
					pDC->SelectObject(&pen2);
				}

				int isx = (int)(sx * getImpl().ratioX);
				int isy = (int)(sy * getImpl().ratioY);
				int isw = (int)(sw * getImpl().ratioX);
				int ish = (int)(sh * getImpl().ratioY);
				pDC->Rectangle(isx, isy, isx + isw, isy + ish);

				float ratio = track->getGrabbingImageRatio();
				int y1 = isy + (int)(ish * ratio);
				pDC->MoveTo(isx, y1);
				pDC->LineTo(isx + isw, y1);

				float score, cx, cy, dx, dy;
				if (track->getTrackingTargetResult(score, cx, cy, dx, dy)) {
					cstr.Format(_T("target: %.3f, moved: %.2f, %.2f"), score, dx, dy);
					rect.SetRect(10, 30, w, 30 + 20);
					pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
				}
			}
		}
	}

	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldColor);
	pen1.DeleteObject();
	pen2.DeleteObject();
	pen3.DeleteObject();
	font.DeleteObject();
	return;
}



void ViewRetinaCamera::drawTrackTarget(CDC * pDC)
{
	if (!getImpl().trackTarget) {
		return;
	}

	if (!RetinaTrack2::isTrackingPrepared()) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 255, 32);
	COLORREF clr3 = RGB(32, 32, 255);
	COLORREF oldColor;

	CPen pen1, pen2, pen3, *oldPen;
	pen1.CreatePen(PS_SOLID, 2, clr1);
	pen2.CreatePen(PS_SOLID, 2, clr2);
	pen3.CreatePen(PS_SOLID, 2, clr3);
	oldPen = pDC->SelectObject(&pen1);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);

	CBrush *oldBrush;
	oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	int w = getWidth();
	int h = getHeight();
	int iw = (int)(getImageWidth() * getImpl().ratioX);
	int ih = (int)(getImageHeight() * getImpl().ratioY);

	int sx = (getWidth() - iw) / 2;
	int sy = (getHeight() - ih) / 2;

	int tw, th, tx, ty;
	float val;

	// LogD() << "getTargetInfo";
	if (RetinaTrack2::getTrackingTargetInfo(tx, ty, tw, th, val, true))
	{
		LogD() << "Target: " << tx << ", " << ty << ", " << tw << ", " << th << ", " << getImpl().ratioX << ", " << getImpl().ratioY;

		int itx = (int)(tx * getImpl().ratioX);
		int ity = (int)(ty * getImpl().ratioY);
		int itw = (int)(tw * getImpl().ratioX);
		int ith = (int)(th * getImpl().ratioY);

		int x1 = itx - itw / 2 + sx;
		int y1 = ity - ith / 2 + sy;
		int x2 = itx + itw / 2 + sx;
		int y2 = ity + ith / 2 + sy;

		x1 = min(max(0, x1), w);
		x2 = min(max(0, x2), w);
		y1 = min(max(0, y1), h-25);
		y2 = min(max(0, y2), h-25);

		pDC->Rectangle(x1, y1, x2, y2);

		CString cstr;
		CRect rect;
		cstr.Format(_T("%d,%d,%.0f"), (int)tx, (int)ty, val);
		rect.SetRect(itx - 50, y2 + 5, itx + 50, y2 + 25);

		oldColor = pDC->SetTextColor(clr1);
		pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
		pDC->SetTextColor(oldColor);

		if (RetinaTrack2::getTrackingMatchedInfo(tx, ty, tw, th, val, true))
		{
			pDC->SelectObject(&pen2);
			// LogD() << "Target: " << tx << ", " << ty << ", " << tw << ", " << th << ", " << getImpl().ratioX << ", " << getImpl().ratioY;

			int itx = (int)(tx * getImpl().ratioX);
			int ity = (int)(ty * getImpl().ratioY);
			int itw = (int)(tw * getImpl().ratioX);
			int ith = (int)(th * getImpl().ratioY);

			int x1 = itx - itw / 2 + sx;
			int y1 = ity - ith / 2 + sy;
			int x2 = itx + itw / 2 + sx;
			int y2 = ity + ith / 2 + sy;

			x1 = min(max(0, x1), w);
			x2 = min(max(0, x2), w);
			y1 = min(max(0, y1), h-25);
			y2 = min(max(0, y2), h-25);

			pDC->Rectangle(x1, y1, x2, y2);

			CString cstr;
			CRect rect;
			cstr.Format(_T("%d,%d,%.2f"), (int)tx, (int)ty, val);
			rect.SetRect(itx - 50, y2 + 5, itx + 50, y2 + 25);

			oldColor = pDC->SetTextColor(clr2);
			pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
			pDC->SetTextColor(oldColor);
		}
	}

	bool pending;
	if (RetinaTrack2::getScanningRegionInfo(tx, ty, tw, th, pending, true))
	{
		pDC->SelectObject(&pen3);
		//LogD() << "Scanning: " << sx << ", " << sy << ", " << sw << ", " << sh << ", " << pending << ", ratio: " << getImpl().ratioX;

		int itx = (int)(tx * getImpl().ratioX);
		int ity = (int)(ty * getImpl().ratioY);
		int itw = (int)(tw * getImpl().ratioX);
		int ith = (int)(th * getImpl().ratioY);

		int x1 = itx - itw / 2 + sx; 
		int y1 = ity - ith / 2 + sy;
		int x2 = itx + itw / 2 + sx;
		int y2 = ity + ith / 2 + sy;

		x1 = min(max(0, x1), w);
		x2 = min(max(0, x2), w);
		y1 = min(max(0, y1), h - 25);
		y2 = min(max(0, y2), h - 25);

		pDC->Rectangle(x1, y1, x2, y2);
	}
	
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldFont);
	pen1.DeleteObject();
	pen2.DeleteObject();
	pen3.DeleteObject();
	font.DeleteObject();
	return;
}



void ViewRetinaCamera::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int ViewRetinaCamera::getWidth(void) const
{
	return getImpl().width;
}


int ViewRetinaCamera::getHeight(void) const
{
	return getImpl().height;
}


int ViewRetinaCamera::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int ViewRetinaCamera::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}


void ViewRetinaCamera::updateImage(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	CvImage image;
	if (image.fromBitsData(data, width, height)) {
		image.resizeTo(&getImpl().image, getWidth(), getHeight());
		updateWindow();
	}
	return;
}


void ViewRetinaCamera::showCameraInfo(bool flag)
{
	getImpl().cameraInfo = flag;
	return;
}


void ViewRetinaCamera::showAlignGuide(bool flag)
{
	getImpl().alignGuide = flag;
	return;
}


void ViewRetinaCamera::showFocusStatus(bool flag)
{
	getImpl().focusStatus = flag;
	return;
}


void ViewRetinaCamera::showSplitPoints(bool flag)
{
	getImpl().splitPoints = flag;
	return;
}


void ViewRetinaCamera::showSplitWindows(bool flag)
{
	getImpl().splitWindows = flag;
	return;
}


void ViewRetinaCamera::showCircleMask(bool flag)
{
	getImpl().circleMask = flag;
	return;
}


void ViewRetinaCamera::showWorkingDots(bool flag)
{
	getImpl().workingDots = flag;
	return;
}


void ViewRetinaCamera::showTrackTarget(bool flag)
{
	getImpl().trackTarget = flag;
	return;
}

void ViewRetinaCamera::showTrackFeature(bool flag)
{
	getImpl().trackFeature = flag;
	return;
}


RetinaCameraImageCallback * ViewRetinaCamera::getCallbackFunction(void)
{
	return &getImpl().callback;
}


bool ViewRetinaCamera::saveImage(CString& filename)
{
	CString dirname = _T(".//export");
	CreateDirectory(dirname, NULL);

	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("RETINA_%y%m%d_%H%M%S.png"));
	}
	if (!getImpl().image.isEmpty()) {
		path = dirname + _T("//") + path;
		filename = path;
		return getImpl().image.saveFile(wtoa(path));
	}
	return false;
}


ViewRetinaCamera::ViewRetinaCameraImpl & ViewRetinaCamera::getImpl(void) const
{
	return *d_ptr;
}



void ViewRetinaCamera::callbackCameraImage(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	countFrameRate();

	getImpl().ratioX = (float) getWidth() / width;
	getImpl().ratioY = (float) getHeight() / height;
	getImpl().ratioX = getImpl().ratioY;

	if (width == getWidth() && height == getHeight()) {
		if (getImpl().image.fromBitsData(data, width, height)) {
			/*
			if (getImpl().circleMask && GlobalSettings::useRetinaROI()) {
				auto pt1 = GlobalSettings::startOfRetinaROI();
				auto pt2 = GlobalSettings::closeOfRetinaROI();
				getImpl().image.drawCircleMask(pt1.first, pt1.second, pt2.first, pt2.second);
			}
			*/
			updateWindow();
		}
	}
	else {
		CvImage image;
		if (getImpl().image.fromBitsData(data, width, height)) {
			// image.resizeTo(&getImpl().image, getWidth(), getHeight());
			updateWindow();
		}
	}
	return;
}


void ViewRetinaCamera::countFrameRate(void)
{
	int current = CTime::GetCurrentTime().GetSecond();
	if (getImpl().frameSec != current) {
		getImpl().frameSec = current;
		getImpl().frameRate = getImpl().frameCount;
		getImpl().frameCount = 0;
	}
	else {
		getImpl().frameCount++;
	}
	return;
}


BEGIN_MESSAGE_MAP(ViewRetinaCamera, CStatic)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// ViewRetinaCamera message handlers
void ViewRetinaCamera::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	updateWindow();
	return;
}


void ViewRetinaCamera::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDC* pDC = GetDC();
	if (pDC) {
		COLORREF color = pDC->GetPixel(point);
		int r = (int)GetRValue(color);
		int g = (int)GetGValue(color);
		int b = (int)GetBValue(color);
		d_ptr->imageGray = (int)(r*0.2126 + g*0.7152 + b*0.0722);
	}

	updateWindow();

	CStatic::OnMouseMove(nFlags, point);
}
