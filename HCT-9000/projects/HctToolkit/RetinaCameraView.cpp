// RetinaCameraView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "RetinaCameraView.h"

#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctDevice;
using namespace std;


struct RetinaCameraView::RetinaCameraViewImpl
{
	CvImage image;
	int width;
	int height;

	int frameRate;
	int frameCount;
	int frameSec;

	bool cameraInfo;
	bool alignGuide;

	IrCameraFrameCallback callback;

	RetinaCameraViewImpl()
		: width(IR_CAMERA_IMAGE_WIDTH), height(IR_CAMERA_IMAGE_HEIGHT),
		frameRate(0), frameCount(0), frameSec(0),
		cameraInfo(false), alignGuide(false)
	{

	}

};


// RetinaCameraView

IMPLEMENT_DYNAMIC(RetinaCameraView, CStatic)

RetinaCameraView::RetinaCameraView()
	: d_ptr(make_unique<RetinaCameraViewImpl>())
{
	getImpl().callback = std::bind(&RetinaCameraView::callbackIrCameraFrame, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}

RetinaCameraView::~RetinaCameraView()
{
}


// RetinaCameraView message handlers

void RetinaCameraView::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawAlignGuide(pMemDC);
	drawCameraInfo(pMemDC);

	// Copy the bits from the memory DC into the current DC.
	BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
	return;
}


void RetinaCameraView::drawCameraInfo(CDC * pDC)
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
	str.Format(_T("Retina Ir %d fps"), getImpl().frameRate);

	CRect rect = CRect(40, getHeight() - 50, 180, getHeight());
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void RetinaCameraView::drawAlignGuide(CDC * pDC)
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
	pDC->LineTo(w / 2, h-1);
	pDC->MoveTo(0, h / 2);
	pDC->LineTo(w-1, h / 2);

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


void RetinaCameraView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int RetinaCameraView::getWidth(void) const
{
	return getImpl().width;
}


int RetinaCameraView::getHeight(void) const
{
	return getImpl().height;
}


void RetinaCameraView::showCameraInfo(bool flag)
{
	getImpl().cameraInfo = flag;
	return;
}


void RetinaCameraView::showAlignGuide(bool flag)
{
	getImpl().alignGuide = flag;
	return;
}


IrCameraFrameCallback * RetinaCameraView::getCallbackFunction(void)
{
	return &getImpl().callback;
}

void RetinaCameraView::saveImage(CString path)
{
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("RETINA_%y%m%d_%H%M%S.bmp"));
	}
	getImpl().image.saveFile(wtoa(path));
	return;
}


RetinaCameraView::RetinaCameraViewImpl & RetinaCameraView::getImpl(void) const
{
	return *d_ptr;
}



void RetinaCameraView::callbackIrCameraFrame(unsigned char * data, unsigned int width, unsigned int height)
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


void RetinaCameraView::countFrameRate(void)
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


BEGIN_MESSAGE_MAP(RetinaCameraView, CStatic)
END_MESSAGE_MAP()

// RetinaCameraView message handlers
void RetinaCameraView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	return;
}
