// ViewCorneaCamera.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewCorneaCamera.h"

#include "CppUtil2.h"
#include "OctSystem2.h"
#include "KerFocus2.h"

using namespace CppUtil;
using namespace OctDevice;
using namespace OctSystem;
using namespace KerFocus;
using namespace std;


struct ViewCorneaCamera::ViewCorneaCameraImpl
{
	CvImage image;
	int width;
	int height;

	int frameRate;
	int frameCount;
	int frameSec;

	bool cameraInfo;
	bool keraoPoints;
	bool spotWindows;
	bool focusStatus;
	bool alignGuide;
	bool stageStatus;

	bool isFocusDist;
	bool isFocusCenter;
	float focusDist;
	float focusCentX;
	float focusCentY;

	int yPosMin;
	int yPosMax;
	int xPosMin;
	int xPosMax;
	int zPosMin;
	int zPosMax;
	int xCenter;
	int yCenter;
	int zCenter;
	int xOffset;
	int yOffset;
	int zOffset;

	int imageGray = 0;

	CorneaCameraImageCallback callback;

	ViewCorneaCameraImpl()
		: width(IR_CAMERA_IMAGE_WIDTH), height(IR_CAMERA_IMAGE_HEIGHT),
			frameRate(0), frameCount(0), frameSec(0), 
			cameraInfo(false), keraoPoints(false), spotWindows(false), focusStatus(true), alignGuide(false), 
			stageStatus(false),
			isFocusDist(false), focusDist(0.0f), isFocusCenter(false), focusCentX(0.0f), focusCentY(0.0f)
	{

	}

};

// ViewCorneaCamera

IMPLEMENT_DYNAMIC(ViewCorneaCamera, CStatic)

ViewCorneaCamera::ViewCorneaCamera()
	: d_ptr(make_unique<ViewCorneaCameraImpl>())
{
	getImpl().callback = std::bind(&ViewCorneaCamera::callbackCameraImage, this, std::placeholders::_1, std::placeholders::_2, 
								std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, 
								std::placeholders::_6, std::placeholders::_7, std::placeholders::_8);
}


ViewCorneaCamera::~ViewCorneaCamera()
{
}


void ViewCorneaCamera::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC, true);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawAlignGuide(pMemDC);
	drawSpotWindows(pMemDC);
	drawCameraInfo(pMemDC);
	drawKeratoPoints(pMemDC);
	drawFocusStatus(pMemDC);
	drawStageStatus(pMemDC);

	SetStretchBltMode(hDC, HALFTONE);
	StretchBlt(hDC, 0, 0, getWidth(), getHeight(),
		hMemDC, 0, 0, getImageWidth(), getImageHeight(), SRCCOPY);

	// Copy the bits from the memory DC into the current DC.
	// BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
	return;
}


void ViewCorneaCamera::drawCameraInfo(CDC * pDC)
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
	str.Format(_T("Cornea Ir %d fps"), getImpl().frameRate);

	auto board = Controller::getMainBoard();
	auto count = board->getCorneaIrCamera()->getErrorCount();
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


void ViewCorneaCamera::drawKeratoPoints(CDC * pDC)
{
	if (!getImpl().keraoPoints) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 32, 255);
	COLORREF clr3 = RGB(32, 255, 32);

	CPen pen1, pen2, pen3, *oldPen;
	pen1.CreatePen(PS_SOLID, 1, clr1);
	pen2.CreatePen(PS_SOLID, 1, clr2);
	pen3.CreatePen(PS_DASHDOT, 1, clr3);
	oldPen = pDC->SelectObject(&pen1);

	KeratoFocus* kerFocus = Controller::getCorneaAlign()->getKeratoFocus();
	float cx, cy;
	int size, thresh, icx, icy;
	CRect rect;

	for (int i = 0; i < kerFocus->countOfMireSpots(); i++) {
		if (kerFocus->getMireSpot(i, cx, cy, size, thresh, rect)) {
			pDC->SelectObject(&pen1);
			icx = (int)cx;
			icy = (int)cy;
			pDC->MoveTo(icx - 5, icy);
			pDC->LineTo(icx + 5, icy);
			pDC->MoveTo(icx, icy - 5);
			pDC->LineTo(icx, icy + 5);
		}
	}

	for (int i = 0; i < kerFocus->countOfFocusSpots(); i++) {
		if (kerFocus->getFocusSpot(i, cx, cy, size, thresh, rect)) {
			pDC->SelectObject(&pen2);
			icx = (int)cx;
			icy = (int)cy;
			pDC->MoveTo(icx - 5, icy);
			pDC->LineTo(icx + 5, icy);
			pDC->MoveTo(icx, icy - 5);
			pDC->LineTo(icx, icy + 5);
		}
	}

	if (kerFocus->getPupilWidth() < 400 && kerFocus->getPupilWidth() > 20) {
		int pup_sx = kerFocus->getPupilStartX();
		int pup_ex = kerFocus->getPupilCloseX();

		pDC->SelectObject(&pen3);
		pDC->MoveTo(pup_sx, 240 - 25);
		pDC->LineTo(pup_sx, 240 + 25);
		pDC->MoveTo(pup_ex, 240 - 25);
		pDC->LineTo(pup_ex, 240 + 25);
	}


	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	float m, n, a;
	if (kerFocus->getMireEllipse(cx, cy, m, n, a)) {
		str.Format(_T("CEN: %.0f, %.0f (%.1f, %.1f)"), cx, cy, 322.0f-cx, 240.0f-cy);
		rect = CRect(getWidth() - 240, 20, getWidth(), 40);
		pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

		str.Format(_T("MNA: %.1f, %.1f, %.0f"), m, n, a);
		rect = CRect(getWidth() - 240, 40, getWidth(), 60);
		pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);
	}
	else {
		str.Format(_T("CEN: ---"));
		rect = CRect(getWidth() - 240, 20, getWidth(), 40);
		pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

		str.Format(_T("MNA: ---"));
		rect = CRect(getWidth() - 240, 40, getWidth(), 60);
		pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);
	}

	float flen, mlen;
	if (kerFocus->getFocusLength(flen) && kerFocus->getMireLength(mlen)) {
		str.Format(_T("HL: %.1f, FL: %.1f => %.1f"), mlen, flen, mlen-flen);
	}
	else {
		str.Format(_T("HL: ---, FL: ---"));
	}

	rect = CRect(getWidth() - 240, 60, getWidth(), 80);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	pDC->SelectObject(oldPen);
	pen1.DeleteObject();
	pen2.DeleteObject();
	pen3.DeleteObject();
	font.DeleteObject();
	return;
}


void ViewCorneaCamera::drawFocusStatus(CDC * pDC)
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
	
	/*
	KeratoFocus* kerFocus = Controller::getCorneaAlign()->getKeratoFocus();
	float level;
	if (kerFocus->getFocusLevel(level)) {
		str.Format(_T("Focus Level: %.1f"), level);
	}
	else {
		str.Format(_T("Focus Level: ---"));
	}
	*/

	if (getImpl().isFocusCenter) {
		str.Format(_T("Eye Center: %.0f, %.0f"), getImpl().focusCentX, getImpl().focusCentY);
	}
	else {
		str.Format(_T("Eye Center: ---"));
	}

	CRect rect = CRect(getWidth() - 160, getHeight() - 70, getWidth(), getHeight()-50);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	if (getImpl().isFocusDist) {
		str.Format(_T("Focus Dist.: %.1f"), getImpl().focusDist);
	}
	else {
		str.Format(_T("Focus Dist.: ---"));
	}

	rect = CRect(getWidth()-160, getHeight() - 50, getWidth(), getHeight() - 30);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	KeratoFocus* kerFocus = Controller::getCorneaAlign()->getKeratoFocus();
	// str.Format(_T("Light Extent: %.2f"), kerFocus->getLightExtent());
	int width = kerFocus->getPupilWidth();
	int pixels = GlobalSettings::corneaPixelsPerMM();
	float size = (pixels > 0 ? (float) width / (float) pixels : 0.0f);
	str.Format(_T("Pupil Width: %d (%.0f)"), kerFocus->getPupilWidth(), size);

	rect = CRect(getWidth() - 160, getHeight() - 30, getWidth(), getHeight() - 10);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewCorneaCamera::drawStageStatus(CDC * pDC)
{
	if (!getImpl().stageStatus) {
		return;
	}

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;

	auto board = Controller::getMainBoard();
	
	static int count = 0;
	if (++count % 10 == 0) {
		board->getStageXMotor()->updateStatus();
		board->getStageYMotor()->updateStatus();
		board->getStageZMotor()->updateStatus();
		count = 0;
	}


	auto xmotor = board->getStageXMotor();
	d_ptr->xPosMax = max(xmotor->getPosition(), xmotor->getRangeMax());
	d_ptr->xPosMin = min(xmotor->getPosition(), xmotor->getRangeMin());
	d_ptr->xCenter = (d_ptr->xPosMin + d_ptr->xPosMax) / 2;
	d_ptr->xOffset = d_ptr->xCenter - (xmotor->getRangeMax() + xmotor->getRangeMin()) / 2;

	text.Format(_T("X pos: %d, [%d, %d] => %d"), xmotor->getPosition(), xmotor->getRangeMin(), xmotor->getRangeMax(), d_ptr->xOffset);
	rect = CRect(40, 20, 360, 40);
	pDC->DrawTextW(text, &rect, DT_LEFT | DT_SINGLELINE);

	auto ymotor = board->getStageYMotor();
	d_ptr->yPosMax = max(ymotor->getPosition(), ymotor->getRangeMax());
	d_ptr->yPosMin = min(ymotor->getPosition(), ymotor->getRangeMin());
	d_ptr->yCenter = (d_ptr->yPosMin + d_ptr->yPosMax) / 2;
	d_ptr->yOffset = d_ptr->yCenter - (ymotor->getRangeMax() + ymotor->getRangeMin()) / 2;

	// text.Format(_T("Y pos: %d, [%d, %d] => %d"), ymotor->getPosition(), ymotor->getRangeMin(), ymotor->getRangeMax(), d_ptr->yOffset);
	text.Format(_T("Y pos: %d, [%d, %d]"), ymotor->getPosition(), ymotor->getRangeMin(), ymotor->getRangeMax());
	rect = CRect(40, 40, 360, 60);
	pDC->DrawTextW(text, &rect, DT_LEFT | DT_SINGLELINE);

	auto zmotor = board->getStageZMotor();
	d_ptr->zPosMax = max(zmotor->getPosition(), zmotor->getRangeMax());
	d_ptr->zPosMin = min(zmotor->getPosition(), zmotor->getRangeMin());
	d_ptr->zCenter = (d_ptr->zPosMin + d_ptr->zPosMax) / 2;
	d_ptr->zOffset = d_ptr->zCenter - (zmotor->getRangeMax() + zmotor->getRangeMin()) / 2;

	text.Format(_T("Z pos: %d, [%d, %d] => %d"), zmotor->getPosition(), zmotor->getRangeMin(), zmotor->getRangeMax(), d_ptr->zOffset);
	rect = CRect(40, 60, 360, 80);
	pDC->DrawTextW(text, &rect, DT_LEFT | DT_SINGLELINE);

	if (Controller::isStageAtLeftEnd() || Controller::isStageAtRightEnd()) {
		text.Format(_T("X limit: %d - %d"), Controller::isStageAtLeftEnd(), Controller::isStageAtRightEnd());
		rect = CRect(40, 80, 360, 100);
		pDC->DrawTextW(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (Controller::isStageAtUpperEnd() || Controller::isStageAtLowerEnd()) {
		text.Format(_T("Y limit: %d - %d"), Controller::isStageAtUpperEnd(), Controller::isStageAtLowerEnd());
		rect = CRect(40, 100, 360, 120);
		pDC->DrawTextW(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (Controller::isStageAtFrontEnd() || Controller::isStageAtRearEnd()) {
		text.Format(_T("Z limit: %d - %d"), Controller::isStageAtFrontEnd(), Controller::isStageAtRearEnd());
		rect = CRect(40, 120, 360, 140);
		pDC->DrawTextW(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewCorneaCamera::drawAlignGuide(CDC * pDC)
{
	if (!getImpl().alignGuide) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	oldPen = pDC->SelectObject(&pen);

	int w = getWidth();
	int h = getHeight();
	pDC->MoveTo(w / 2, 0);
	pDC->LineTo(w / 2, h - 1);
	pDC->MoveTo(0, h / 2);
	pDC->LineTo(w - 1, h / 2);

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


void ViewCorneaCamera::drawSpotWindows(CDC * pDC)
{
	if (!getImpl().spotWindows) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(0, 128, 255);

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	KeratoFocus* kerFocus = Controller::getCorneaAlign()->getKeratoFocus();
	float cx, cy;
	int size, thresh, icx, icy;
	CRect rect;
	CString cstr;

	for (int i = 0; i < kerFocus->countOfMireSpots(); i++) {
		if (kerFocus->getMireSpot(i, cx, cy, size, thresh, rect)) {
			pDC->SetTextColor(clr1);
			icx = (int)cx;
			icy = (int)cy;

			// pDC->Rectangle(&rect);
			cstr.Format(_T("x:%d,y:%d,sz:%d"), icx, icy, size);
			rect.SetRect(icx - 100, icy + 10, icx + 100, icy + 25);
			pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
		}
	}

	for (int i = 0; i < kerFocus->countOfFocusSpots(); i++) {
		if (kerFocus->getFocusSpot(i, cx, cy, size, thresh, rect)) {
			pDC->SetTextColor(clr2);
			icx = (int)cx;
			icy = (int)cy;

			// pDC->Rectangle(&rect);
			cstr.Format(_T("x:%d,y:%d,sz:%d"), icx, icy, size);
			rect.SetRect(icx - 100, icy + 10, icx + 100, icy + 25);
			pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
		}
	}

	pDC->SetBkMode(oldMode);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewCorneaCamera::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int ViewCorneaCamera::getWidth(void) const
{
	return getImpl().width;
}


int ViewCorneaCamera::getHeight(void) const
{
	return getImpl().height;
}


int ViewCorneaCamera::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int ViewCorneaCamera::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}


void ViewCorneaCamera::showCameraInfo(bool flag)
{
	getImpl().cameraInfo = flag;
	return;
}


void ViewCorneaCamera::showKeratoPoints(bool flag)
{
	getImpl().keraoPoints = flag;
	return;
}


void ViewCorneaCamera::showFocusStatus(bool flag)
{
	getImpl().focusStatus = flag;
	return;
}


void ViewCorneaCamera::showAlignGuide(bool flag)
{
	getImpl().alignGuide = flag;
	return;
}


void ViewCorneaCamera::showSpotWindows(bool flag)
{
	getImpl().spotWindows = flag;
	return;
}


void ViewCorneaCamera::showStageStatus(bool flag)
{
	getImpl().stageStatus = flag;

	d_ptr->xPosMin = 99999;
	d_ptr->yPosMin = 99999;
	d_ptr->zPosMin = 99999;
	d_ptr->xPosMax = -99999;
	d_ptr->yPosMax = -99999;
	d_ptr->zPosMax = -99999;
	return;
}


CorneaCameraImageCallback * ViewCorneaCamera::getCallbackFunction(void)
{
	return &getImpl().callback;
}


bool ViewCorneaCamera::saveImage(CString& filename)
{
	CString dirname = _T(".//export");
	CreateDirectory(dirname, NULL);

	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("CORNEA_%y%m%d_%H%M%S.png"));
	}
	if (!getImpl().image.isEmpty()) {
		path = dirname + _T("//") + path;
		filename = path;
		return getImpl().image.saveFile(wtoa(path));
	}
	return false;
}


ViewCorneaCamera::ViewCorneaCameraImpl & ViewCorneaCamera::getImpl(void) const
{
	return *d_ptr;
}


void ViewCorneaCamera::callbackCameraImage(unsigned char * data, unsigned int width, unsigned int height, bool isCenter, float centX, float centY, bool isFocus, float distZ)
{
	if (data == nullptr) {
		return;
	}

	getImpl().isFocusDist = isFocus;
	getImpl().isFocusCenter = isCenter;
	getImpl().focusDist = distZ;
	getImpl().focusCentX = centX;
	getImpl().focusCentY = centY;

	countFrameRate();
	/*
	if (getImpl().image.fromBitsData(data, width, height)) {
		updateWindow();
	}
	*/
	if (width == getWidth() && height == getHeight()) {
		if (getImpl().image.fromBitsData(data, width, height)) {
			updateWindow();
		}
	}
	else {
		CvImage image;
		if (image.fromBitsData(data, width, height)) {
			image.resizeTo(&getImpl().image, getWidth(), getHeight());
			updateWindow();
		}
	}
	return;
}


void ViewCorneaCamera::countFrameRate(void)
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


BEGIN_MESSAGE_MAP(ViewCorneaCamera, CStatic)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// ViewCorneaCamera message handlers
void ViewCorneaCamera::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	updateWindow();
	return;
}

void ViewCorneaCamera::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	CDC* pDC = GetDC();
	if (pDC) {
		COLORREF color = pDC->GetPixel(point);
		int r = (int)GetRValue(color);
		int g = (int)GetGValue(color);
		int b = (int)GetBValue(color);
		d_ptr->imageGray = int(r*0.2126 + g*0.7152 + b*0.0722);

	}

	updateWindow();

	CStatic::OnMouseMove(nFlags, point);
}
