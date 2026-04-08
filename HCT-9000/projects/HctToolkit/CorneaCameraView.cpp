// CorneaCameraView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "CorneaCameraView.h"

#include "CppUtil2.h"
#include "OctSystem2.h"
#include "KerFocus2.h"

using namespace CppUtil;
using namespace OctDevice;
using namespace OctSystem;
using namespace KerFocus;
using namespace std;

struct CorneaCameraView::CorneaCameraViewImpl
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

	CorneaCameraImageCallback callback;

	CorneaCameraViewImpl()
		: width(IR_CAMERA_IMAGE_WIDTH), height(IR_CAMERA_IMAGE_HEIGHT),
			frameRate(0), frameCount(0), frameSec(0), 
			cameraInfo(false), keraoPoints(false), spotWindows(false), focusStatus(true), alignGuide(false)
	{

	}

};

// CorneaCameraView

IMPLEMENT_DYNAMIC(CorneaCameraView, CStatic)

CorneaCameraView::CorneaCameraView()
	: d_ptr(make_unique<CorneaCameraViewImpl>())
{
	getImpl().callback = std::bind(&CorneaCameraView::callbackCameraImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


CorneaCameraView::~CorneaCameraView()
{
}


void CorneaCameraView::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawAlignGuide(pMemDC);
	drawSpotWindows(pMemDC);
	drawCameraInfo(pMemDC);
	drawKeratoPoints(pMemDC);
	drawFocusStatus(pMemDC);

	// Copy the bits from the memory DC into the current DC.
	BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
	return;
}


void CorneaCameraView::drawCameraInfo(CDC * pDC)
{
	if (!getImpl().cameraInfo) {
		return;
	}

	CFont font;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	str.Format(_T("Cornea Ir %d fps"), getImpl().frameRate);

	CRect rect = CRect(40, getHeight() - 50, 180, getHeight());
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void CorneaCameraView::drawKeratoPoints(CDC * pDC)
{
	if (!getImpl().keraoPoints) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 32, 255);

	CPen pen1, pen2, *oldPen;
	pen1.CreatePen(PS_SOLID, 1, clr1);
	pen2.CreatePen(PS_SOLID, 1, clr2);
	oldPen = pDC->SelectObject(&pen1);

	KeratoFocus* kerFocus = Controller::getCorneaFocus()->getKeratoFocus();
	float cx, cy;
	int size, thresh, icx, icy;
	CRect rect;

	for (int i = 0; i < kerFocus->getKeratoSpotsCount(); i++) {
		if (kerFocus->getKeratoSpot(i, cx, cy, size, thresh, rect)) {
			if (kerFocus->isFocusSpot(i)) {
				pDC->SelectObject(&pen2);
			}
			else {
				pDC->SelectObject(&pen1);
			}

			icx = (int)cx;
			icy = (int)cy;
			pDC->MoveTo(icx - 5, icy);
			pDC->LineTo(icx + 5, icy);
			pDC->MoveTo(icx, icy - 5);
			pDC->LineTo(icx, icy + 5);

			// pDC->Rectangle(&rect);
		}
	}

	pDC->SelectObject(oldPen);
	pen1.DeleteObject();
	pen2.DeleteObject();
	return;
}


void CorneaCameraView::drawFocusStatus(CDC * pDC)
{
	if (!getImpl().focusStatus) {
		return;
	}

	CFont font;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	KeratoFocus* kerFocus = Controller::getCorneaFocus()->getKeratoFocus();
	float level;
	if (kerFocus->getFocusLevel(level)) {
		str.Format(_T("Focus Level: %.1f"), level);
	}
	else {
		str.Format(_T("Focus Level: ---"));
	}

	CRect rect = CRect(getWidth()-160, getHeight() - 50, getWidth(), getHeight());
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void CorneaCameraView::drawAlignGuide(CDC * pDC)
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
		pDC->MoveTo(cx + i * 100, cy - 2);
		pDC->LineTo(cx + i * 100, cy + 2);
		pDC->MoveTo(cx - i * 100, cy - 2);
		pDC->LineTo(cx - i * 100, cy + 2);
	}

	for (int i = 1; i <= 2; i++) {
		pDC->MoveTo(cx - 2, cy + i * 100);
		pDC->LineTo(cx + 2, cy + i * 100);
		pDC->MoveTo(cx - 2, cy - i * 100);
		pDC->LineTo(cx + 2, cy - i * 100);
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


void CorneaCameraView::drawSpotWindows(CDC * pDC)
{
	if (!getImpl().spotWindows) {
		return;
	}

	COLORREF clr1 = RGB(255, 32, 32);
	COLORREF clr2 = RGB(32, 32, 255);

	CFont font;
	font.CreateFontW(16, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *oldFont = pDC->SelectObject(&font);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	KeratoFocus* kerFocus = Controller::getCorneaFocus()->getKeratoFocus();
	float cx, cy;
	int size, thresh, icx, icy;
	CRect rect;
	CString cstr;

	for (int i = 0; i < kerFocus->getKeratoSpotsCount(); i++) {
		if (kerFocus->getKeratoSpot(i, cx, cy, size, thresh, rect)) {
			if (kerFocus->isFocusSpot(i)) {
				pDC->SetTextColor(clr2);
			}
			else {
				pDC->SetTextColor(clr1);
			}

			icx = (int)cx;
			icy = (int)cy;

			// pDC->Rectangle(&rect);
			cstr.Format(_T("x:%d,y:%d,size:%d"), icx, icy, size);
			rect.SetRect(icx - 60, icy + 10, icx + 60, icy + 25);
			pDC->DrawTextW(cstr, &rect, DT_CENTER | DT_SINGLELINE);
		}
	}
	pDC->SetBkMode(oldMode);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void CorneaCameraView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int CorneaCameraView::getWidth(void) const
{
	return getImpl().width;
}


int CorneaCameraView::getHeight(void) const
{
	return getImpl().height;
}


void CorneaCameraView::showCameraInfo(bool flag)
{
	getImpl().cameraInfo = flag;
	return;
}


void CorneaCameraView::showKeratoPoints(bool flag)
{
	getImpl().keraoPoints = flag;
	return;
}


void CorneaCameraView::showFocusStatus(bool flag)
{
	getImpl().focusStatus = flag;
	return;
}


void CorneaCameraView::showAlignGuide(bool flag)
{
	getImpl().alignGuide = flag;
	return;
}


void CorneaCameraView::showSpotWindows(bool flag)
{
	getImpl().spotWindows = flag;
	return;
}


CorneaCameraImageCallback * CorneaCameraView::getCallbackFunction(void)
{
	return &getImpl().callback;
}


void CorneaCameraView::saveImage(CString path)
{
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("CORNEA_%y%m%d_%H%M%S.bmp"));
	}
	getImpl().image.saveFile(wtoa(path));
	return;
}


CorneaCameraView::CorneaCameraViewImpl & CorneaCameraView::getImpl(void) const
{
	return *d_ptr;
}


void CorneaCameraView::callbackCameraImage(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	countFrameRate();
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


void CorneaCameraView::countFrameRate(void)
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


BEGIN_MESSAGE_MAP(CorneaCameraView, CStatic)
END_MESSAGE_MAP()



// CorneaCameraView message handlers
void CorneaCameraView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	return;
}


