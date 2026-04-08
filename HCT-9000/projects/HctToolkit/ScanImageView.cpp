// ScanImageView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ScanImageView.h"

#include "CppUtil2.h"

// ScanImageView
using namespace std;
using namespace CppUtil;


struct ScanImageView::ScanImageViewImpl
{
	CvImage image;
	int width;
	int height;
	float quality;

	PreviewImageCallback callback;

	ScanImageViewImpl()
	{

	}
};


IMPLEMENT_DYNAMIC(ScanImageView, CStatic)

ScanImageView::ScanImageView()
	: d_ptr(make_unique<ScanImageViewImpl>())
{
	getImpl().callback = std::bind(&ScanImageView::callbackPreviewImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}


ScanImageView::~ScanImageView()
{
}


void ScanImageView::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawOverlayText(pMemDC);

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


void ScanImageView::drawOverlayText(CDC * pDC)
{
	CString str;
	str.Format(_T("SNR Rate: %.1f"), getImpl().quality);

	CFont font, *oldFont;
	font.CreateFontW(24, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(32, 255, 32));
	pDC->SetBkMode(TRANSPARENT);
	
	CRect rect = CRect(getWidth() - 200, getHeight() - 50, getWidth(), getHeight());
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ScanImageView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int ScanImageView::getWidth(void) const
{
	return getImpl().width;
}


int ScanImageView::getHeight(void) const
{
	return getImpl().height;
}


PreviewImageCallback * ScanImageView::getCallbackFunction(void)
{
	return &getImpl().callback;
}


ScanImageView::ScanImageViewImpl & ScanImageView::getImpl(void) const
{
	return *d_ptr;
}


void ScanImageView::callbackPreviewImage(unsigned char * data, unsigned int width, unsigned int height, float quality)
{
	if (data == nullptr) {
		return;
	}

	if (getImpl().image.fromBitsData(data, width, height)) {
		getImpl().quality = quality;
		updateWindow();
	}

	return;
}


BEGIN_MESSAGE_MAP(ScanImageView, CStatic)
END_MESSAGE_MAP()



// ScanImageView message handlers


void ScanImageView::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
}
