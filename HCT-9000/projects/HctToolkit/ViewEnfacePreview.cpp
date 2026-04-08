// ViewEnfacePreview.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewEnfacePreview.h"

#include "CppUtil2.h"

// ViewEnfacePreview
using namespace std;
using namespace CppUtil;


struct ViewEnfacePreview::ViewEnfacePreviewImpl
{
	CvImage image;
	int width;
	int height;
	bool equalize;
	float clipLimit;
	float scalar;

	float rangeX;
	float rangeY;
	float measureX;
	float measureY;
	int srcWidth;
	int srcHeight;
	bool showLines;
	bool showTopoLines;

	CPoint points[4];
	int numPoints;

	EnfaceImageCallback callback;

	ViewEnfacePreviewImpl() : equalize(false), showLines(false), showTopoLines(false), numPoints(0), clipLimit(16.0), scalar(0.0)
	{

	}
};


IMPLEMENT_DYNAMIC(ViewEnfacePreview, CStatic)

ViewEnfacePreview::ViewEnfacePreview()
	: d_ptr(make_unique<ViewEnfacePreviewImpl>())
{
	getImpl().callback = std::bind(&ViewEnfacePreview::callbackEnfaceImage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


ViewEnfacePreview::~ViewEnfacePreview()
{
}


BEGIN_MESSAGE_MAP(ViewEnfacePreview, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// ViewEnfacePreview message handlers

void ViewEnfacePreview::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawGridLines(pMemDC);
	drawTopoGridLines(pMemDC);
	drawPixelsPitch(pMemDC);

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
}


void ViewEnfacePreview::drawPixelsPitch(CDC * pDC)
{
	if (d_ptr->numPoints <= 0) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 128, 128));
	oldPen = pDC->SelectObject(&pen);

	pDC->MoveTo(d_ptr->points[0].x - 3, d_ptr->points[0].y);
	pDC->LineTo(d_ptr->points[0].x + 3, d_ptr->points[0].y);
	pDC->MoveTo(d_ptr->points[0].x, d_ptr->points[0].y - 5);
	pDC->LineTo(d_ptr->points[0].x, d_ptr->points[0].y + 5);

	if (d_ptr->numPoints > 1) {
		pDC->MoveTo(d_ptr->points[1].x - 3, d_ptr->points[1].y);
		pDC->LineTo(d_ptr->points[1].x + 3, d_ptr->points[1].y);
		pDC->MoveTo(d_ptr->points[1].x, d_ptr->points[1].y - 5);
		pDC->LineTo(d_ptr->points[1].x, d_ptr->points[1].y + 5);

		pDC->MoveTo(d_ptr->points[0]);
		pDC->LineTo(d_ptr->points[1]);
	}
	if (d_ptr->numPoints > 2) {
		pDC->MoveTo(d_ptr->points[2].x - 5, d_ptr->points[2].y);
		pDC->LineTo(d_ptr->points[2].x + 5, d_ptr->points[2].y);
		pDC->MoveTo(d_ptr->points[2].x, d_ptr->points[2].y - 3);
		pDC->LineTo(d_ptr->points[2].x, d_ptr->points[2].y + 3);
	}
	if (d_ptr->numPoints > 3) {
		pDC->MoveTo(d_ptr->points[3].x - 5, d_ptr->points[3].y);
		pDC->LineTo(d_ptr->points[3].x + 5, d_ptr->points[3].y);
		pDC->MoveTo(d_ptr->points[3].x, d_ptr->points[3].y - 3);
		pDC->LineTo(d_ptr->points[3].x, d_ptr->points[3].y + 3);

		pDC->MoveTo(d_ptr->points[2]);
		pDC->LineTo(d_ptr->points[3]);
	}

	float wRatio = (float)getSourceWidth() / (float)getWidth();
	float hRatio = (float)getSourceHeight() / (float)getHeight();

	CFont font, *oldFont;
	font.CreateFontW(18, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(255, 128, 128));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;

	text.Format(_T("Position-1: %d, %d"), (int)(d_ptr->points[0].x*wRatio), (int)(d_ptr->points[0].y*hRatio));
	rect = CRect(25, 20, 300, 40);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	if (d_ptr->numPoints > 1) {
		text.Format(_T("Position-2: %d, %d"), (int)(d_ptr->points[1].x*wRatio), (int)(d_ptr->points[1].y*hRatio));
		rect = CRect(25, 40, 300, 60);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}
	if (d_ptr->numPoints > 2) {
		text.Format(_T("Position-3: %d, %d"), (int)(d_ptr->points[2].x*wRatio), (int)(d_ptr->points[2].y*hRatio));
		rect = CRect(25, 60, 300, 80);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}
	if (d_ptr->numPoints > 3) {
		text.Format(_T("Position-4: %d, %d"), (int)(d_ptr->points[3].x*wRatio), (int)(d_ptr->points[3].y*hRatio));
		rect = CRect(25, 80, 300, 100);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (d_ptr->numPoints >= 2) {
		float w_pitch = (float)(abs(d_ptr->points[1].x - d_ptr->points[0].x)*wRatio);
		float w_length = (getSourceWidth()/w_pitch) * d_ptr->rangeX;
		float w_pixel = w_length / getSourceWidth();
		text.Format(_T("Width, Pixel Res.: %.2f, %.2f"), w_length, w_pixel*1000.0f);
		rect = CRect(25, getHeight()-60, 300, getHeight()-40);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (d_ptr->numPoints >= 4) {
		float h_pitch = (float)(abs(d_ptr->points[3].y - d_ptr->points[2].y)*hRatio);
		float h_length = (getSourceHeight() / h_pitch) * d_ptr->rangeY;
		float h_pixel = h_length / getSourceHeight();
		text.Format(_T("Height, Pixel Res.: %.2f, %.2f"), h_length, h_pixel*1000.0f);
		rect = CRect(25, getHeight() - 40, 300, getHeight() - 20);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;

}


void ViewEnfacePreview::setEqualization(bool flag, float limit, float scalar)
{
	getImpl().equalize = flag;
	getImpl().clipLimit = limit;
	getImpl().scalar = scalar;
	return;
}


void ViewEnfacePreview::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ViewEnfacePreview::setScanRange(float x, float y)
{
	getImpl().rangeX = x;
	getImpl().rangeY = y;
	return;
}


void ViewEnfacePreview::setMeasureRange(float x, float y)
{
	getImpl().measureX = x;
	getImpl().measureY = y;
	return;
}


void ViewEnfacePreview::showGridLines(bool flag)
{
	getImpl().showLines = flag;
	updateWindow();
	return;
}

void ViewEnfacePreview::showTopoGridLines(bool flag)
{
	getImpl().showTopoLines = flag;
	updateWindow();
	return;
}

int ViewEnfacePreview::getWidth(void) const
{
	return getImpl().width;
}


int ViewEnfacePreview::getHeight(void) const
{
	return getImpl().height;
}


int ViewEnfacePreview::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int ViewEnfacePreview::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}


int ViewEnfacePreview::getSourceWidth(void) const
{
	return getImpl().srcWidth;
}


int ViewEnfacePreview::getSourceHeight(void) const
{
	return getImpl().srcHeight;
}

unsigned char* ViewEnfacePreview::getImageBitData(void) const
{
	return getImpl().image.getBitsData();
}


EnfaceImageCallback * ViewEnfacePreview::getCallbackFunction(void)
{
	return &getImpl().callback;
}


bool ViewEnfacePreview::saveImage(CString & filename)
{
	CString path = filename;
	if (path.IsEmpty()) {
		CTime time = CTime::GetCurrentTime();
		path = time.Format(_T("ENFACE_%y%m%d_%H%M%S.png"));
	}
	if (!getImpl().image.isEmpty()) {
		filename = path;
		return getImpl().image.saveFile(wtoa(path));
	}
	return false;
}


ViewEnfacePreview::ViewEnfacePreviewImpl & ViewEnfacePreview::getImpl(void) const
{
	return *d_ptr;
}


void ViewEnfacePreview::drawGridLines(CDC * pDC)
{
	if (!getImpl().showLines) {
		return;
	}
	if (getImpl().rangeX <= 0.0f || getImpl().rangeY <= 0.0f) {
		return;
	}

	CPen pen, pen2, *oldPen;
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
	int xPerMM = (int)(w / getImpl().rangeX);
	int yPerMM = (int)(h / getImpl().rangeY);
	int xCount = (int)(getImpl().rangeX / 2.0f);
	int yCount = (int)(getImpl().rangeY / 2.0f);

	for (int i = 1; i <= xCount; i++) {
		pDC->MoveTo(cx + i * xPerMM, cy - 5);
		pDC->LineTo(cx + i * xPerMM, cy + 5);
		pDC->MoveTo(cx - i * xPerMM, cy - 5);
		pDC->LineTo(cx - i * xPerMM, cy + 5);
	}

	for (int i = 1; i <= yCount; i++) {
		pDC->MoveTo(cx - 5, cy + i * yPerMM);
		pDC->LineTo(cx + 5, cy + i * yPerMM);
		pDC->MoveTo(cx - 5, cy - i * yPerMM);
		pDC->LineTo(cx + 5, cy - i * yPerMM);
	}

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pen2.DeleteObject();
	return;
}


void ViewEnfacePreview::drawTopoGridLines(CDC * pDC)
{
	if (!getImpl().showTopoLines) {
		return;
	}
	if (getImpl().rangeX <= 0.0f || getImpl().rangeY <= 0.0f) {
		return;
	}

	CPen pen, pen2, *oldPen;
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
	int pixMM = (int)(w / getImpl().rangeX);
	int halfPixMM = pixMM / 2;

	bool flag = false;

	CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	for (int ix = halfPixMM; ix < w; ix += halfPixMM) {
		pDC->Ellipse(cx - ix, cy - ix, cx + ix, cy + ix);
		if (flag) {
			ix += halfPixMM;
			flag = false;
		}
		else
			flag = true;
	}

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pen2.DeleteObject();
	return;
}

void ViewEnfacePreview::callbackEnfaceImage(const unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	getImpl().srcWidth = width;
	getImpl().srcHeight = height;

	if (width == getWidth() && height == getHeight()) {
		if (getImpl().image.fromBitsData(data, width, height)) {
			if (getImpl().equalize) {
				getImpl().image.equalizeHistogram(d_ptr->clipLimit, (int)d_ptr->scalar);
			}
			getImpl().image.flipVertical();
			updateWindow();
		}
	}
	else {
		CvImage image;
		if (image.fromBitsData(data, width, height)) {
			int w2 = getWidth();
			int h2 = getHeight(); //  int(getHeight() * (d_ptr->rangeY / d_ptr->rangeX));
			image.resizeTo(&getImpl().image, w2, h2);

			if (getImpl().equalize) {
				getImpl().image.equalizeHistogram(d_ptr->clipLimit, (int)d_ptr->scalar);
			}
			getImpl().image.flipVertical();
			updateWindow();
		}
	}

	return;
}


void ViewEnfacePreview::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
}


void ViewEnfacePreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (d_ptr->numPoints >= 4) {
		d_ptr->numPoints = 0;
	}

	d_ptr->points[d_ptr->numPoints] = point;
	d_ptr->numPoints++;

	updateWindow();

	CStatic::OnLButtonDown(nFlags, point);
}


void ViewEnfacePreview::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (d_ptr->numPoints > 0) {
		d_ptr->numPoints = 0;
		updateWindow();
	}

	CStatic::OnRButtonDown(nFlags, point);
}

bool ViewEnfacePreview::IsEmptyImage(void) const
{
	if (d_ptr->image.isEmpty())
		return true;

	return false;
}
