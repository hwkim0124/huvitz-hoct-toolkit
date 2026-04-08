#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewTopoPreview.h"

#include "CppUtil2.h"
#include "SigProc2.h"
#include "SigChain2.h"

using namespace std;
using namespace CppUtil;
using namespace SigProc;
using namespace SigChain;


struct ViewTopoPreview::ViewTopoPreviewImpl
{
	CvImage image;
	int width;
	int height;
	float quality;

	bool showDecorrs = false;
	bool showPattern = false;
	bool showLayers = true;
	bool showRangeGuide = false;
	bool showCurveGuide = false;
	bool isCorneaCenter;
	bool stretched;

	double refractiveIndex = 1.36f;
	float alignRange;
	float decorrAverage = 0.0f;
	float decorrStddev = 0.0f;
	float decorrMaxVal = 0.0f;
	float intensAverage = 0.0f;
	float intensStddev = 0.0f;
	float intensMaxVal = 0.0f;
	float averageCurvature = 0.0f;
	float majorCurvature = 0.0f;
	float minorCurvature = 0.0f;
	float horizontalCurvature = 0.0f;
	float verticalCurvature = 0.0f;
	float majorAxis = 0.0f;
	float minorAxis = 0.0f;

	float corneaOffsetX;
	float acqTime;
	float* decorrs = nullptr;

	vector<float> horzQualities;
	vector<float> vertQualities;
	vector<int> upperLayer;
	vector<int> lowerLayer;

	CPoint points[2];
	int numPoints;
	int srcWidth;
	int srcHeight;
	int	subGuidePos{ -1 };
	
	ViewTopoPreviewImpl() : alignRange(8.0f), acqTime(-1.0f),
		numPoints(0), stretched(false), corneaOffsetX(0.0f), isCorneaCenter(false)
	{

	}
};

IMPLEMENT_DYNAMIC(ViewTopoPreview, CStatic)

ViewTopoPreview::ViewTopoPreview()
	: d_ptr(make_unique<ViewTopoPreviewImpl>())
{
}


ViewTopoPreview::~ViewTopoPreview()
{
}


ViewTopoPreview::ViewTopoPreviewImpl & ViewTopoPreview::getImpl(void) const
{
	return *d_ptr;
}

void ViewTopoPreview::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}

void ViewTopoPreview::setDisplayStretched(bool flag)
{
	getImpl().stretched = flag;
	return;
}

void ViewTopoPreview::clearResultData(bool cont)
{
	getImpl().acqTime = 0.0f;
	getImpl().quality = 0.0f;
	getImpl().decorrAverage = 0.0f;
	getImpl().intensAverage = 0.0f;
	getImpl().decorrs = nullptr;

	if (!cont) {
		getImpl().horzQualities.clear();
		getImpl().vertQualities.clear();
	}
	return;
}

void ViewTopoPreview::setAcquisitionTime(float acqTime)
{
	getImpl().acqTime = acqTime;
	return;
}

void ViewTopoPreview::setQualityIndex(float quality)
{
	getImpl().quality = quality;
	return;
}

void ViewTopoPreview::setPatternQualities(int index, std::vector<float> stat)
{
	if (index == 0) {
		getImpl().horzQualities = stat;
	}
	else {
		getImpl().vertQualities = stat;
	}
	return;
}


void ViewTopoPreview::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	HBITMAP hbmp = getImpl().image.createDIBitmap(hDC);
	HGDIOBJ hobj = SelectObject(hMemDC, hbmp);

	drawAlignGuide(pMemDC);
	drawOverlayText(pMemDC);
	drawPatternQualities(pMemDC);
	drawEnfaceGuide(pMemDC);
	drawPixelsPitch(pMemDC);
	drawSlabLayers(pMemDC);
	drawSegmentLayer(pMemDC);

	/*CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	CRect rtBitmap;
	rtBitmap.SetRect(0, 0, getWidth(), getHeight());
	drawSegmentLayer(pMemDC, rtBitmap);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);*/

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

void ViewTopoPreview::drawOverlayText(CDC * pDC)
{

	CFont font, *oldFont;
	font.CreateFontW(22, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	auto oldColor = pDC->SetTextColor(RGB(32, 255, 32));
	pDC->SetBkMode(TRANSPARENT);

	static ULONGLONG tick = 0;
	static int count = 0;
	static float avg_fps = 0.0f;
	static float sum_fps = 0.0f;

	ULONGLONG curr = GetTickCount64();
	if (tick > 0 && curr > tick) {
		sum_fps += (float)(1000.0f / (curr - tick));
	}
	tick = curr;

	if (++count % 10 == 0) {
		avg_fps = sum_fps / 10.0f;
		sum_fps = 0.0f;
		count = 0;
	}

	CString text;
	CRect rect;

	text.Format(_T("FPS Mean: %.0f"), avg_fps);
	rect = CRect(40, getHeight() - 60, 240, getHeight() - 35);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	if (getImpl().acqTime > 0.0f) {
		text.Format(_T("Acq. Time: %.2f"), getImpl().acqTime);
		rect = CRect(40, getHeight() - 35, 240, getHeight() - 10);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (getImpl().quality > 0.0f) {
		text.Format(_T("SNR Rate: %.1f"), getImpl().quality);
		rect = CRect(40, getHeight() - 85, 240, getHeight() - 60);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (getImpl().horizontalCurvature > 0.0f) {
		text.Format(_T("Horizontal Curvature: %.2f"), getImpl().horizontalCurvature);
		rect = CRect(240, getHeight() - 85, getWidth() - 40, getHeight() - 60);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}
	else if (getImpl().verticalCurvature > 0.0f) {
		text.Format(_T("Vertical Curvature: %.2f"), getImpl().verticalCurvature);
		rect = CRect(240, getHeight() - 85, getWidth() - 40, getHeight() - 60);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	if (getImpl().averageCurvature > 0.0f) {
		text.Format(_T("Average Curvature: %.2f"), getImpl().averageCurvature);
		rect = CRect(240, getHeight() - 60, getWidth() - 40, getHeight() - 35);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	if (getImpl().majorCurvature > 0.0f) {
		text.Format(_T("R1: %.2f(%.f¡Æ), R2: %.2f(%.f¡Æ)"),
			getImpl().majorCurvature, getImpl().majorAxis, getImpl().minorCurvature, getImpl().minorAxis);
		rect =CRect(240, getHeight() - 35, getWidth() - 40, getHeight() - 10);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldColor);
	font.DeleteObject();
	return;
}


void ViewTopoPreview::drawPatternQualities(CDC * pDC)
{
	CFont font, *oldFont;
	font.CreateFontW(22, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	auto oldColor = pDC->SetTextColor(RGB(32, 255, 32));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;

	auto& horz = getImpl().horzQualities;
	auto& vert = getImpl().vertQualities;
	if (horz.size() > 0 || vert.size() > 0) {
		if (horz.size() > 0 && vert.size() > 0) {
			auto data = vector<float>(9);
			data[0] = (horz[0] + vert[0]) / 2;
			data[1] = (horz[0] + vert[1]) / 2;
			data[2] = (horz[0] + vert[2]) / 2;
			data[3] = (horz[1] + vert[0]) / 2;
			data[4] = (horz[1] + vert[1]) / 2;
			data[5] = (horz[1] + vert[2]) / 2;
			data[6] = (horz[2] + vert[0]) / 2;
			data[7] = (horz[2] + vert[1]) / 2;
			data[8] = (horz[2] + vert[2]) / 2;

			text.Format(_T("%4.1f   %4.1f   %4.1f"), data[0], data[1], data[2]);
			rect = CRect(240, 120, getWidth() - 40, 150);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("%4.1f   %4.1f   %4.1f"), data[3], data[4], data[5]);
			rect = CRect(240, 150, getWidth() - 40, 180);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("%4.1f   %4.1f   %4.1f"), data[6], data[7], data[8]);
			rect = CRect(240, 180, getWidth() - 40, 210);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
		}
		else if (horz.size() > 0) {
			text.Format(_T("----   %4.1f   ----"), horz[0]);
			rect = CRect(240, 120, getWidth() - 40, 150);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("----   %4.1f   ----"), horz[1]);
			rect = CRect(240, 150, getWidth() - 40, 180);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("----   %4.1f   ----"), horz[2]);
			rect = CRect(240, 180, getWidth() - 40, 210);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
		}
		else {
			text.Format(_T("----   ----   ----"));
			rect = CRect(240, 120, getWidth() - 40, 150);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("%4.1f   %4.1f   %4.1f"), vert[0], vert[1], vert[2]);
			rect = CRect(240, 150, getWidth() - 40, 180);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("----   ----   ----"));
			rect = CRect(240, 180, getWidth() - 40, 210);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
		}
	}

	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldColor);
	font.DeleteObject();
	return;
}


void ViewTopoPreview::drawAlignGuide(CDC * pDC)
{
	if (getImpl().showRangeGuide) {
		CPen pen, pen2, penUnder, *oldPen;
		pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
		penUnder.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = pDC->SelectObject(&pen);
		pen2.CreatePen(PS_DASH, 1, RGB(128, 255, 128));

		int w = getWidth();
		int h = getHeight();
		pDC->MoveTo(w / 2, 0);
		pDC->LineTo(w / 2, h - 1);
		pDC->MoveTo(0, h / 2);
		pDC->LineTo(w - 1, h / 2);

		oldPen = pDC->SelectObject(&penUnder);
		pDC->MoveTo(0, h / 2 + 10);
		pDC->LineTo(w - 1, h / 2 + 10);

		pDC->MoveTo(0, h / 2 + 20);
		pDC->LineTo(w - 1, h / 2 + 20);

		int pixMM = (int)(w / getImpl().alignRange);
		int halfPixMM = pixMM / 2;

		int cx = w / 2;
		int cy = h / 2;

		pDC->SelectObject(&pen2);
		
		bool flag = false;

		for (int ix = 0; ix < w; ix += halfPixMM) {
			pDC->MoveTo(ix, 0);
			pDC->LineTo(ix, h - 1);

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
		penUnder.DeleteObject();
	}
	else if (getImpl().showCurveGuide) {
		CRect rect;
		GetClientRect(&rect);

		CPen penHorz(PS_SOLID, 1, RGB(255, 0, 0));
		CPen penVert(PS_SOLID, 2, RGB(0, 204, 192));

		pDC->SelectObject(&penVert);
		pDC->MoveTo(rect.Width() / 2, 0);
		pDC->LineTo(rect.Width() / 2, rect.Height());

		/*pDC->SelectObject(&penHorz);
		pDC->MoveTo(0, rect.Height() / 5);
		pDC->LineTo(rect.Width(), rect.Height() / 5);
		pDC->MoveTo(0, rect.Height() / 5 * 4);
		pDC->LineTo(rect.Width(), rect.Height() / 5 * 4);*/

		/*pDC->SelectObject(&penHorz);
		pDC->MoveTo(0, rect.Height() / 10 * 4);
		pDC->LineTo(rect.Width(), rect.Height() / 10 * 4);
		pDC->MoveTo(0, rect.Height() / 40 * 33);
		pDC->LineTo(rect.Width(), rect.Height() / 40 * 33);*/
		pDC->SelectObject(&penHorz);
		pDC->MoveTo(0, rect.Height() / 5);
		pDC->LineTo(rect.Width(), rect.Height() / 5);
		pDC->MoveTo(0, rect.Height() / 40 * 24);
		pDC->LineTo(rect.Width(), rect.Height() / 40 * 24);


		penHorz.DeleteObject();
		penVert.DeleteObject();
	}

	return;
}


void ViewTopoPreview::drawEnfaceGuide(CDC * pDC)
{
	CPen pen, *oldPen;
	pen.CreatePen(PS_DOT, 1, RGB(32, 32, 255));
	oldPen = pDC->SelectObject(&pen);

	CFont font, *oldFont;
	font.CreateFontW(22, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(32, 255, 32));
	pDC->SetBkMode(TRANSPARENT);

	float ratio = (float)getSourceHeight() / getHeight();

	auto pair = GlobalSettings::enfacePreviewRange();
	int y1 = (int)(pair.first * ratio);
	int y2 = (int)(pair.second * ratio);
	int size = (pair.second - pair.first + 1);

	if (size > 0 && size < 768) {
		if (y1 > 0 && y1 < (getHeight() - 1)) {
			pDC->MoveTo(0, y1);
			pDC->LineTo(getWidth() - 1, y1);
		}

		if (y2 > 0 && y2 < (getHeight() - 1)) {
			pDC->MoveTo(0, y2);
			pDC->LineTo(getWidth() - 1, y2);
		}

		CString text;
		CRect rect;

		float axialResol = (float)GlobalSettings::getRetinaScanAxialResolution();
		text.Format(_T("Thickness: %.2f"), size * axialResol);
		rect = CRect(40, getHeight() - 50, 240, getHeight() - 30);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldFont);
	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ViewTopoPreview::drawPixelsPitch(CDC * pDC)
{
	if (d_ptr->numPoints <= 0) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 128, 128));
	oldPen = pDC->SelectObject(&pen);

	pDC->MoveTo(d_ptr->points[0].x - 10, d_ptr->points[0].y);
	pDC->LineTo(d_ptr->points[0].x + 10, d_ptr->points[0].y);
	pDC->MoveTo(d_ptr->points[0].x, d_ptr->points[0].y - 5);
	pDC->LineTo(d_ptr->points[0].x, d_ptr->points[0].y + 5);

	if (d_ptr->numPoints > 1) {
		pDC->MoveTo(d_ptr->points[1].x - 10, d_ptr->points[1].y);
		pDC->LineTo(d_ptr->points[1].x + 10, d_ptr->points[1].y);
		pDC->MoveTo(d_ptr->points[1].x, d_ptr->points[1].y - 5);
		pDC->LineTo(d_ptr->points[1].x, d_ptr->points[1].y + 5);

		pDC->MoveTo(d_ptr->points[0]);
		pDC->LineTo(d_ptr->points[1]);
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
	rect = CRect(40, 50, 400, 75);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	if (d_ptr->numPoints > 1) {
		text.Format(_T("Position-2: %d, %d"), (int)(d_ptr->points[1].x*wRatio), (int)(d_ptr->points[1].y*hRatio));
		rect = CRect(40, 75, 400, 100);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

		double pixResol = ChainSetup::getAxialPixelResolution(getImpl().refractiveIndex);
		float y_pitch = (float)(fabs(d_ptr->points[1].y - d_ptr->points[0].y)*hRatio);
		text.Format(_T("Y-Pitch: %.2f (Thickness = %.2f)"), y_pitch, y_pitch * pixResol);
		rect = CRect(40, 100, 400, 125);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}

void ViewTopoPreview::drawSlabLayers(CDC * pDC)
{
	if (!getImpl().showLayers) {
		return;
	}

	if (getImpl().upperLayer.empty() && getImpl().lowerLayer.empty()) {
		return;
	}

	auto iw = getWidth();
	auto ih = getHeight();
	float hRatio = (float)ih / 768;

	auto upper = getImpl().upperLayer;
	auto lower = getImpl().lowerLayer;

	if (upper.size() > 0) {
		float lw = (float)upper.size();
		for (int x = 0; x < iw; x++) {
			int idx = (int)(lw * ((float)x / (float)iw));
			int y1 = (int)(upper[idx] * (getImpl().stretched ? hRatio : 1.0f));

			pDC->SetPixel(x, y1, RGB(255, 128, 128));
		}
	}

	if (lower.size() > 0) {
		float lw = (float)lower.size();
		for (int x = 0; x < iw; x++) {
			int idx = (int)(lw * ((float)x / (float)iw));
			int y2 = (int)(lower[idx] * (getImpl().stretched ? hRatio : 1.0f));

			pDC->SetPixel(x, y2, RGB(128, 255, 128));
		}
	}
	return;
}

void ViewTopoPreview::drawScanImage(const OctScanImage * image, bool vflip)
{
	if (!image) {
		return;
	}

	auto data = image->getData();
	auto width = image->getWidth();
	auto height = image->getHeight();
	auto quality = image->getQuality();

	getImpl().srcWidth = width;
	getImpl().srcHeight = height;
	getImpl().quality = quality;

	if (width == getWidth() && height == getHeight()) {
		if (getImpl().image.fromBitsData(data, width, height)) {
			if (vflip) {
				getImpl().image.flipVertical();
			}
			updateWindow();
		}
	}
	else {
		CvImage image;
		if (image.fromBitsData(data, width, height)) {
			if (getImpl().stretched) {
				image.resizeTo(&getImpl().image, getWidth(), getHeight());
			}
			else {
				image.resizeTo(&getImpl().image, getWidth(), getHeight());
				getImpl().srcHeight = getHeight();
			}
			if (vflip) {
				getImpl().image.flipVertical();
			}
			updateWindow();
		}
	}
	return;
}


void ViewTopoPreview::drawSegmentLayer(CDC *pDC)
{
	Gdiplus::Graphics G(pDC->m_hDC);
	G.SetPageUnit(Gdiplus::UnitPixel);
	G.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

	float scaleX = getWidth() / (float)getSourceWidth();
	float scalseY = getHeight()/ (float)getSourceHeight();

	vector <Gdiplus::Point> points;
	for (int i = 0; i < m_layer.size(); i++) {
		points.push_back(Gdiplus::Point(i * scaleX, m_layer[i] * scalseY));
	}

	Gdiplus::Pen pen(Gdiplus::Color(234, 232, 42), 2);
	G.DrawLines(&pen, points.data(), points.size());

	return;
}

void ViewTopoPreview::setLayerPoints(std::vector<int>& points)
{
	m_layer.assign(points.begin(), points.end());

	return;
}

void ViewTopoPreview::clearLayerPoints(void)
{
	m_layer.clear();

	return;
}

void ViewTopoPreview::setAverageCurvature(float value)
{
	getImpl().averageCurvature = value;

	return;
}

void ViewTopoPreview::clearAverageCurvature(void)
{
	getImpl().averageCurvature = 0.0f;

	return;
}

void ViewTopoPreview::clearHorizontalCurvature(void)
{
	getImpl().horizontalCurvature = 0.0f;

	return;
}

void ViewTopoPreview::clearVerticalCurvature(void)
{
	getImpl().verticalCurvature = 0.0f;

	return;
}

void ViewTopoPreview::setMajorCurvature(float value)
{
	getImpl().majorCurvature = value;

	return;
}

void ViewTopoPreview::setHorizontalCurvature(float value)
{
	getImpl().horizontalCurvature = value;

	return;
}

void ViewTopoPreview::setVerticalCurvature(float value)
{
	getImpl().verticalCurvature = value;

	return;
}

float ViewTopoPreview::getHorizontalCurvature(void)
{
	return getImpl().horizontalCurvature;
}

float ViewTopoPreview::getVerticalCurvature(void)
{
	return getImpl().verticalCurvature;
}

void ViewTopoPreview::clearMajorCurvature(void)
{
	getImpl().majorCurvature = 0.0f;

	return;
}

void ViewTopoPreview::setMinorCurvature(float value)
{
	getImpl().minorCurvature = value;

	return;
}

void ViewTopoPreview::clearMinorCurvature(void)
{
	getImpl().minorCurvature = 0.0f;

	return;
}

void ViewTopoPreview::setMajorAxis(float value)
{
	getImpl().majorAxis = value;

	return;
}

void ViewTopoPreview::clearMajorAxis(void)
{
	getImpl().majorAxis = 0.0f;

	return;
}

void ViewTopoPreview::setMinorAxis(float value)
{
	getImpl().minorAxis = value;

	return;
}

void ViewTopoPreview::clearMinorAxis(void)
{
	getImpl().minorAxis = 0.0f;

	return;
}

void ViewTopoPreview::setImage(unsigned char *data, unsigned int width, unsigned int height, int sigCenter)
{
	if (data == nullptr) {
		return;
	}

	getImpl().srcWidth = width;
	getImpl().srcHeight = height;


	if (width == getWidth() && height == getHeight()) {
		if (getImpl().image.fromBitsData(data, width, height)) {
			updateWindow();
		}
	}
	else {
		CvImage image;
		if (image.fromBitsData(data, width, height)) {
			if (getImpl().stretched) {
				image.resizeTo(&getImpl().image, getWidth(), getHeight());
			}
			else {
				image.resizeTo(&getImpl().image, getWidth(), height);
				getImpl().srcHeight = getHeight();
			}
			updateWindow();
		}
	}
	return;
}


int ViewTopoPreview::getWidth(void) const
{
	return getImpl().width;
}

int ViewTopoPreview::getHeight(void) const
{
	return getImpl().height;
}

int ViewTopoPreview::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}

int ViewTopoPreview::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}

int ViewTopoPreview::getSourceWidth(void) const
{
	return getImpl().srcWidth;
}

int ViewTopoPreview::getSourceHeight(void) const
{
	return getImpl().srcHeight;
}

void ViewTopoPreview::setSubGuidePos(int value)
{
	getImpl().subGuidePos = value;

	return;
}

void ViewTopoPreview::setRangeGuideline(bool stat)
{
	getImpl().showRangeGuide = stat;

	return;
}

void ViewTopoPreview::setCurveGuideline(bool stat)
{
	getImpl().showCurveGuide = stat;

	return;
}
