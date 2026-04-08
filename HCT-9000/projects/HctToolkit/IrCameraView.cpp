// IrCameraView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "IrCameraView.h"

#include "CppUtil2.h"

// IrCameraView
using namespace std;
using namespace CppUtil;
using namespace OctDevice;
using namespace Gdiplus;


struct IrCameraView::IrCameraViewImpl 
{
	CvImage image;
	int width;
	int height;

	bool overlay;

	int frameRate;
	int frameCount;
	int frameSec;

	unique_ptr<Gdiplus::CachedBitmap> bitmap;
	IrCameraFrameCallback callback;

	IrCameraViewImpl()
		: width(IR_CAMERA_IMAGE_WIDTH), height(IR_CAMERA_IMAGE_HEIGHT), 
			frameRate(0), frameCount(0), frameSec(0), 
			overlay(true)
	{
	}
};


IMPLEMENT_DYNAMIC(IrCameraView, CStatic)

IrCameraView::IrCameraView() 
	: d_ptr(make_unique<IrCameraViewImpl>())
{
	getImpl().callback = std::bind(&IrCameraView::callbackIrCamera, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


IrCameraView::~IrCameraView()
{
}


void IrCameraView::updateWindow(void)
{
	// Invalidate(FALSE);

	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawOverlay(pMemDC);

	// Copy the bits from the memory DC into the current DC.
	BitBlt(hDC, 0, 0, getImpl().width, getImpl().height, hMemDC, 0, 0, SRCCOPY);
	/*
	StretchBlt(hDC, 0, 0, getImpl().width, getImpl().height,
		hMemDC, IR_CAMERA_FRAME_PADD_SIZE, 0,
		getImpl().image.getWidth(), getImpl().image.getHeight(), SRCCOPY);
	*/
	

	SelectObject(hMemDC, hobj);
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	DeleteObject(hbmp);
	return;
}


void IrCameraView::drawOverlay(CDC * pDC)
{
	if (!getImpl().overlay) {
		return;
	}

	CString str;
	str.Format(_T("%d fps"), getImpl().frameRate);

	CFont font;
	font.CreateFontW(24, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	CFont *pold = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);

	int sx = getWidth() - 140;
	int sy = getHeight() - 50;
	int ex = sx + 120;
	int ey = sy + 30;

	CRect rect = CRect(sx, sy, ex, ey);
	pDC->DrawTextW(str, &rect, DT_LEFT | DT_SINGLELINE);
	pDC->SelectObject(pold);
	font.DeleteObject();
	return;
}


void IrCameraView::setOverlay(bool flag)
{
	getImpl().overlay = flag;
	return;
}


IrCameraFrameCallback * IrCameraView::getCallbackFunction(void)
{
	return &getImpl().callback;
}


void IrCameraView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int IrCameraView::getWidth(void) const
{
	return getImpl().width;
}


int IrCameraView::getHeight(void) const
{
	return getImpl().height;
}


void IrCameraView::callbackIrCamera(unsigned char * data, unsigned int width, unsigned int height)
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


void IrCameraView::countFrameRate(void)
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


void IrCameraView::drawOverlay(Graphics & G)
{
	Gdiplus::Font font(L"Arial", 12, FontStyleRegular, UnitPoint);
	Gdiplus::SolidBrush brush(Color(0, 255, 0));

	wstring str;
	str = to_wstring(getImpl().frameRate);
	str += L" fps";
	G.DrawString(str.data(), (int)str.length(), &font, PointF((float)getWidth()-100, (float)getHeight()-50), &brush);
	return;
}


IrCameraView::IrCameraViewImpl & IrCameraView::getImpl(void) const
{
	return *d_ptr;
}



BEGIN_MESSAGE_MAP(IrCameraView, CStatic)
END_MESSAGE_MAP()



// IrCameraView message handlers


void IrCameraView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	return;
	/*
	// TODO:  Add your code to draw the specified item
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics G(*pDC);

	Bitmap window(getImpl().width, getImpl().height);
	Graphics memG(&window);

	Bitmap* image = getImpl().image.getBitmap();
	memG.DrawImage(image, 0, 0);

	// Bitmap* pBmp = getImpl().image.getBitmap();
	// Graphics memG(pBmp);
	drawOverlay(memG);

	// getImpl().bitmap = make_unique<CachedBitmap>(&window, &G);
	CachedBitmap* bitmap = new CachedBitmap(&window, &G);
	// G.DrawCachedBitmap(getImpl().bitmap.get(), 0, 0);
	G.DrawCachedBitmap(bitmap, 0, 0);
	delete bitmap;
	ReleaseDC(pDC);
	*/
}
