// ViewScanPreview.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewScanPreview.h"

#include "CppUtil2.h"
#include "SigProc2.h"
#include "SigChain2.h"

// ViewScanPreview
using namespace std;
using namespace CppUtil;
using namespace SigProc;
using namespace SigChain;


struct ViewScanPreview::ViewScanPreviewImpl
{
	CvImage image;
	CvImage imageOriginal;

	int width;
	int height;
	float quality;
	float sigRatio;
	int refPoint = 0;

	bool showOverlay;
	bool alignGuide;
	bool showDecorrs = false;
	bool showPattern = false;
	bool showLayers = true;
	bool showMeasure = false;

	float alignRange;

	float decorrAverage = 0.0f;
	float decorrStddev = 0.0f;
	float decorrMaxVal = 0.0f;
	float intensAverage = 0.0f;
	float intensStddev = 0.0f;
	float intensMaxVal = 0.0f;

	vector<float> horzQualities;
	vector<float> vertQualities;
	vector<int> upperLayer;
	vector<int> lowerLayer;
	vector<float> axialLengths;
	vector<float> lensThicknesses;
	vector<float> ccts;
	vector<float> acds;

	float acqTime;
	float triggerTimeStep;
	float* decorrs = nullptr;
	
	double refractiveIndex = 1.36f;

	CPoint points[2];
	int numPoints;
	int srcWidth;
	int srcHeight;
	bool stretched;

	float corneaOffsetX;
	bool isCorneaCenter;

	PreviewImageCallback2 callback;

	ViewScanPreviewImpl() : showOverlay(true), alignGuide(false), alignRange(6.0f), acqTime(-1.0f), triggerTimeStep(0.0f),
							numPoints(0), stretched(false), corneaOffsetX(0.0f), isCorneaCenter(false)
	{

	}
};


IMPLEMENT_DYNAMIC(ViewScanPreview, CStatic)

ViewScanPreview::ViewScanPreview()
	: d_ptr(make_unique<ViewScanPreviewImpl>())
{
	getImpl().callback = std::bind(&ViewScanPreview::callbackPreviewImage, this, std::placeholders::_1, std::placeholders::_2, 
		std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7);
}


ViewScanPreview::~ViewScanPreview()
{
}


void ViewScanPreview::updateWindow(void)
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
	drawDecorrelations(pMemDC);
	drawSlabLayers(pMemDC);
	drawCorneaCenter(pMemDC);
	drawAutoMeasureResult(pMemDC);

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


void ViewScanPreview::drawOverlayText(CDC * pDC)
{
	if (!getImpl().showOverlay) {
		return;
	}

	CFont font, *oldFont;
	font.CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	auto oldColor = pDC->SetTextColor(RGB(32, 255, 32));
	pDC->SetBkMode(TRANSPARENT);

	static ULONGLONG tick = 0;
	static int count = 0;
	static float avg_fps = 0.0f;
	static float sum_fps = 0.0f;

	ULONGLONG curr = GetTickCount64();
	if (tick > 0 && curr > tick) {
		sum_fps += (float) (1000.0f / (curr - tick));
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
	rect = CRect(40, getHeight() - 60, 240, getHeight() - 40);
	pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

	if (getImpl().acqTime > 0.0f) {
		int speed = 0;
		if (getImpl().triggerTimeStep > 0.0f) {
			speed = (int)ceil((1.0 / getImpl().triggerTimeStep) * 1000.0);
		}
		text.Format(_T("Acq. Time: %.2f, Ascan Rate (kHz): %d"), getImpl().acqTime, speed);
		rect = CRect(40, getHeight() - 40, 400, getHeight() - 20);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	if (getImpl().refPoint > 0 && getImpl().quality > 3.5f) {
		text.Format(_T("Ref. Position: %d"), getImpl().refPoint);
		rect = CRect(240, getHeight() - 80, getWidth() - 40, getHeight() - 60);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	if (getImpl().sigRatio > 0.0f) {
		text.Format(_T("SNR: %.1f, Q Index: %.1f"), getImpl().sigRatio, getImpl().quality);
		rect = CRect(240, getHeight() - 60, getWidth() - 40, getHeight() - 40);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	/*
	if (getImpl().quality > 0.0f) {
		text.Format(_T("Q. Index: %.1f"), getImpl().quality);
		rect = CRect(240, getHeight() - 40, getWidth() - 40, getHeight() - 20);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}
	*/

	if (getImpl().sigRatio > 0.0f) {
		auto sigLevel = GlobalRecord::getOctImageSignalLevel();
		auto bgdLevel = GlobalRecord::getOctImageBackgroundLevel();	
		auto noiseLevel = GlobalRecord::getOctImageNoiseLevel();

		text.Format(_T("Sign: %.0f (%.2f), Back: %.0f (%.2f), Noise: %.2f"),
			sigLevel, log10(sigLevel), bgdLevel, log10(bgdLevel), noiseLevel);
		rect = CRect(240, getHeight() - 40, getWidth() - 40, getHeight() - 20);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	if (getImpl().decorrAverage > 0.0f) {
		text.Format(_T("Decorr.: %.2f (avg), %.3f (std), %.2f (max)"),
			getImpl().decorrAverage, getImpl().decorrStddev, getImpl().decorrMaxVal);
		rect = CRect(240, getHeight() - 80, getWidth() - 40, getHeight() - 60);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	if (getImpl().intensAverage > 0.0f) {
		text.Format(_T("Intens.: %.0f (avg), %.1f (std), %.0f (max)"),
			getImpl().intensAverage, getImpl().intensStddev, getImpl().intensMaxVal);
		rect = CRect(240, getHeight() - 60, getWidth() - 40, getHeight() - 40);
		pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldColor);
	font.DeleteObject();
	return;
}


void ViewScanPreview::drawPatternQualities(CDC * pDC)
{
	if (!getImpl().showPattern) {
		return;
	}

	CFont font, *oldFont;
	font.CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
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
			rect = CRect(240, 120, getWidth() - 40, 140);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("%4.1f   %4.1f   %4.1f"), data[3], data[4], data[5]);
			rect = CRect(240, 140, getWidth() - 40, 160);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("%4.1f   %4.1f   %4.1f"), data[6], data[7], data[8]);
			rect = CRect(240, 160, getWidth() - 40, 180);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
		}
		else if (horz.size() > 0) {
			text.Format(_T("----   %4.1f   ----"), horz[0]);
			rect = CRect(240, 120, getWidth() - 40, 140);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("----   %4.1f   ----"), horz[1]);
			rect = CRect(240, 140, getWidth() - 40, 160);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("----   %4.1f   ----"), horz[2]);
			rect = CRect(240, 160, getWidth() - 40, 180);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
		}
		else {
			text.Format(_T("----   ----   ----"));
			rect = CRect(240, 120, getWidth() - 40, 140);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("%4.1f   %4.1f   %4.1f"), vert[0], vert[1], vert[2]);
			rect = CRect(240, 140, getWidth() - 40, 160);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
			text.Format(_T("----   ----   ----"));
			rect = CRect(240, 160, getWidth() - 40, 180);
			pDC->DrawText(text, &rect, DT_RIGHT | DT_SINGLELINE);
		}
	}

	pDC->SelectObject(oldFont);
	pDC->SetTextColor(oldColor);
	font.DeleteObject();
	return;
}


void ViewScanPreview::drawAlignGuide(CDC * pDC)
{
	if (!getImpl().alignGuide) {
		return;
	}

	CPen pen, pen2, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	oldPen = pDC->SelectObject(&pen);
	pen2.CreatePen(PS_DASH, 1, RGB(128, 255, 128));

	int w = getWidth();
	int h = getHeight();
	pDC->MoveTo(w / 2, 0);
	pDC->LineTo(w / 2, h - 1);
	pDC->MoveTo(0, h / 2);
	pDC->LineTo(w - 1, h / 2);

	int pixMM = (int)(w / getImpl().alignRange);

	int cx = w / 2;
	int cy = h / 2;

	pDC->SelectObject(&pen2);
	for (int ix = cx - pixMM; ix >= 0; ix -= pixMM) {
		pDC->MoveTo(ix, 0);
		pDC->LineTo(ix, h - 1);
	}

	for (int ix = cx + pixMM; ix < w; ix += pixMM) {
		pDC->MoveTo(ix, 0);
		pDC->LineTo(ix, h - 1);
	}

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	pen2.DeleteObject();
	return;
}


void ViewScanPreview::drawEnfaceGuide(CDC * pDC)
{
	CPen pen, *oldPen;
	pen.CreatePen(PS_DOT, 1, RGB(32, 32, 255));
	oldPen = pDC->SelectObject(&pen);

	CFont font, *oldFont;
	font.CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(32, 255, 32));
	pDC->SetBkMode(TRANSPARENT);

	float ratio = (float)getSourceHeight() / getHeight();

	auto pair = GlobalSettings::enfacePreviewRange();
	int y1 = (int)(pair.first * ratio);
	int y2 = (int)(pair.second * ratio);
	int size = (pair.second - pair.first + 1);

	if (size > 0 && size < 768) {
		if (y1 > 0 && y1 < (getHeight()-1)) {
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


void ViewScanPreview::drawPixelsPitch(CDC * pDC)
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


void ViewScanPreview::drawDecorrelations(CDC * pDC)
{
	if (!getImpl().showDecorrs) {
		return;
	}
	
	auto decorrs = getImpl().decorrs;
	if (!decorrs) {
		// decorrs = AngioSetup::getDecorrelation();
		if (!decorrs) {
			return;
		}
	}

	auto* p = decorrs;
	auto sw = getImpl().srcWidth;
	auto sh = getImpl().srcHeight;
	auto iw = getWidth();
	auto ih = getHeight();

	float wRatio = (float)iw / (float)sw;
	float hRatio = (float)ih / (float)sh;
	hRatio = (getImpl().stretched ? hRatio : 1.0f);

	CBrush brush;
	COLORREF color;
	float thresh = 0.15f; // 0.35f; // 0.15f;

	int x1, y1, x2, y2;
	for (int y = 0; y < sh; y++) {
		for (int x = 0; x < sw; x++) {
			float dc = p[y*sw + x];
			if (dc < thresh) {
				continue;
			}

			x1 = (int)(x * wRatio);
			y1 = (int)(y * hRatio);
			x2 = (int)((x + 1) * wRatio);
			y2 = (int)((y + 1) * hRatio);

			if (y1 >= ih) {
				break;
			}

			float ratio = ((dc - thresh) <= 0 ? 0.0f : (dc - thresh) * 2.0f);

			color = RGB(
				max(min((int)(ratio * 255), 255), 0),
				max(min((int)(ratio * 0), 255), 0),
				max(min((int)(ratio * 255), 255), 0)
			);

			brush.CreateSolidBrush(color);
			pDC->FillRect(CRect(x1, y1, x2, y2), &brush);
			brush.DeleteObject();
		}
	}
	return;
}


void ViewScanPreview::drawSlabLayers(CDC * pDC)
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


void ViewScanPreview::drawCorneaCenter(CDC * pDC)
{
	if (!getImpl().isCorneaCenter) {
		return;
	}

	CPen pen, *oldPen;
	pen.CreatePen(PS_DOT, 2, RGB(255, 128, 64));
	oldPen = pDC->SelectObject(&pen);

	auto iw = getWidth();
	auto ih = getHeight();
	auto sw = getImpl().srcWidth;
	auto sh = getImpl().srcHeight;
	float wRatio = (float)iw / (float)sw;

	int cx = (int)((sw / 2) * wRatio);

	int x1 = (int)(getImpl().corneaOffsetX*wRatio);
	int y1 = 0;

	pDC->MoveTo(x1, y1);
	pDC->LineTo(x1, ih - 1);

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ViewScanPreview::drawAutoMeasureResult(CDC * pDC)
{
	if (!getImpl().showMeasure) {
		return;
	}

	CFont font, *oldFont;
	font.CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(255, 64, 64));
	pDC->SetBkMode(TRANSPARENT);

	CString text;
	CRect rect;

	if (!getImpl().axialLengths.empty()) {
		text.Format(_T("Axial Lengths"));
		rect = CRect(40, 125, 400, 150);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

		int count = 1;
		const auto& vect = getImpl().axialLengths;
		auto data = vector<float>();
		for (const auto v : vect) {
			if (v > 0.0f) {
				text.Format(_T("%d : %.4f"), count, v);
				rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
				pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
				data.push_back(v);
				count++;
			}
		}

		text.Format(_T("Avg: %.4f"), [&]() -> float {
			return (data.empty() ? 0.0f : accumulate(data.cbegin(), data.cend(), 0.0f) / data.size());
		}());
		rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}
	else if (!getImpl().lensThicknesses.empty()) {
		text.Format(_T("Lens Thickness"));
		rect = CRect(40, 125, 400, 150);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

		int count = 1;
		const auto& vect = getImpl().lensThicknesses;
		auto data = vector<float>();
		for (const auto v : vect) {
			if (v > 0.0f) {
				text.Format(_T("%d : %.4f"), count, v);
				rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
				pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
				data.push_back(v);
				count++;
			}
		}

		text.Format(_T("Avg: %.4f"), [&]() -> float {
			return (data.empty() ? 0.0f : accumulate(data.cbegin(), data.cend(), 0.0f) / data.size());
		}());
		rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}
	else if (!getImpl().acds.empty()) {
		text.Format(_T("Anterior Chamber Depths"));
		rect = CRect(40, 125, 400, 150);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

		int count = 1;
		const auto& vect = getImpl().acds;
		auto data = vector<float>();
		for (const auto v : vect) {
			if (v > 0.0f) {
				text.Format(_T("%d : %.4f"), count, v);
				rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
				pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
				data.push_back(v);
				count++;
			}
		}

		text.Format(_T("Avg: %.4f"), [&]() -> float {
			return (data.empty() ? 0.0f : accumulate(data.cbegin(), data.cend(), 0.0f) / data.size());
		}());
		rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}
	else if (!getImpl().ccts.empty()) {
		text.Format(_T("Central Cornea Thickness"));
		rect = CRect(40, 125, 400, 150);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

		int count = 1;
		const auto& vect = getImpl().ccts;
		auto data = vector<float>();
		for (const auto v : vect) {
			if (v > 0.0f) {
				text.Format(_T("%d : %.4f"), count, v);
				rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
				pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
				data.push_back(v);
				count++;
			}
		}

		text.Format(_T("Avg: %.4f"), [&]() -> float {
			return (data.empty() ? 0.0f : accumulate(data.cbegin(), data.cend(), 0.0f) / data.size());
		}());
		rect = CRect(40, 125 + count * 25, 400, 150 + count * 25);
		pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
	}

	pDC->SelectObject(oldFont);
	return;
}


void ViewScanPreview::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ViewScanPreview::setDecorrelationValues(float avg, float stdev, float maxv)
{
	getImpl().decorrAverage = avg;
	getImpl().decorrStddev = stdev;
	getImpl().decorrMaxVal = maxv;
	return;
}


void ViewScanPreview::setIntensityValues(float avg, float stdev, float maxv)
{
	getImpl().intensAverage = avg;
	getImpl().intensStddev = stdev;
	getImpl().intensMaxVal = maxv;
	return;
}


void ViewScanPreview::setSlabLayers(std::vector<int> upper, std::vector<int> lower)
{
	getImpl().upperLayer = upper;
	getImpl().lowerLayer = lower;
	return;
}


int ViewScanPreview::getWidth(void) const
{
	return getImpl().width;
}


int ViewScanPreview::getHeight(void) const
{
	return getImpl().height;
}


int ViewScanPreview::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int ViewScanPreview::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}


int ViewScanPreview::getSourceWidth(void) const
{
	return getImpl().srcWidth;
}


int ViewScanPreview::getSourceHeight(void) const
{
	return getImpl().srcHeight;
}

double ViewScanPreview::getRefractiveIndex(void) const
{
	return getImpl().refractiveIndex;
}


void ViewScanPreview::showOverlayInfo(bool flag)
{
	getImpl().showOverlay = flag;
	return;
}


void ViewScanPreview::showAlignGuide(bool flag, float range)
{
	getImpl().alignGuide = flag;
	getImpl().alignRange = range;
	return;
}


void ViewScanPreview::overlayDecorrelations(bool flag)
{
	getImpl().showDecorrs = flag;
	return;
}


void ViewScanPreview::showPatternQualities(bool flag)
{
	getImpl().showPattern = flag;
	return;
}


void ViewScanPreview::showAutoMeasureResult(bool flag)
{
	getImpl().showMeasure = flag;
	return;
}


void ViewScanPreview::setAcquisitionTime(float acqTime)
{
	getImpl().acqTime = acqTime;
	return;
}

void ViewScanPreview::setTriggerTimeStep(float timeStep)
{
	getImpl().triggerTimeStep = timeStep;
	return;
}

void ViewScanPreview::setQualityIndex(float quality)
{
	getImpl().quality = quality;
	return;
}


void ViewScanPreview::setDisplayStretched(bool flag)
{
	getImpl().stretched = flag;
	return;
}


void ViewScanPreview::setDecorrelationData(float * data)
{
	getImpl().decorrs = data;
	return;
}

void ViewScanPreview::setRefractiveIndex(double refractiveIndex)
{
	getImpl().refractiveIndex = refractiveIndex;
	return;
}


void ViewScanPreview::setPatternQualities(int index, std::vector<float> stat)
{
	if (index == 0) {
		getImpl().horzQualities = stat;
	}
	else {
		getImpl().vertQualities = stat;
	}
	return;
}


void ViewScanPreview::clearResultData(bool cont)
{
	getImpl().acqTime = 0.0f;
	getImpl().triggerTimeStep = 0.0f;
	getImpl().quality = 0.0f;
	getImpl().decorrAverage = 0.0f;
	getImpl().intensAverage = 0.0f;
	getImpl().decorrs = nullptr;

	if (!cont) {
		getImpl().horzQualities.clear();
		getImpl().vertQualities.clear();
	}

	getImpl().lensThicknesses.clear();
	getImpl().axialLengths.clear();
	getImpl().ccts.clear();
	getImpl().acds.clear();
	return;
}


void ViewScanPreview::setAxialLengths(std::vector<float> vect)
{
	getImpl().axialLengths = vect;
	return;
}


void ViewScanPreview::setLensThickness(std::vector<float> vect)
{
	getImpl().lensThicknesses = vect;
	return;
}

void ViewScanPreview::setAnteriorChamberDepths(std::vector<float> vect)
{
	getImpl().acds = vect;
	return;
}

void ViewScanPreview::setCorneaCenterThickness(std::vector<float> vect)
{
	getImpl().ccts = vect;
	return;
}


void ViewScanPreview::drawScanImage(const OctScanImage * image, bool vflip)
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
		getImpl().image.copyTo(&getImpl().imageOriginal);
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
			if (vflip) {
				getImpl().image.flipVertical();
			}
			updateWindow();
			image.copyTo(&getImpl().imageOriginal);
		}
	}
	return;
}


void ViewScanPreview::callbackPreviewImage(const unsigned char * data, unsigned int width, unsigned int height, 
	float quality, float sigRatio, unsigned int refPoint, unsigned int idxOfImage)
{
	if (data == nullptr) {
		return;
	}

	if (idxOfImage > 0) {
		return;
	}
	
	getImpl().srcWidth = width;
	getImpl().srcHeight = height;
	getImpl().quality = quality;
	getImpl().sigRatio = sigRatio;
	getImpl().refPoint = refPoint;

	/*
	if (GlobalRecord::checkIfCorneaTrackTargetFound(true)) {
		vector<int> antes;
		GlobalRecord::readCorneaTrackAnteriorCurve(antes);
		float centerX, centerY;
		GlobalRecord::readCorneaTrackAnteriorCenter(centerX, centerY);
		LogD() << "Move scan center: " << centerX << ", " << centerY;
		if (fabs(centerX) < 3.0f) {
			OctSystem::Controller::moveScanCenterPosition(centerX, centerY);
		}
		getImpl().upperLayer = antes;
	}
	*/


	float offsetX;
	if (OctSystem::CorneaTrack::getCorneaCenterOffsetX(offsetX, idxOfImage)) {
		vector<int> antes;
		GlobalRecord::readCorneaTrackAnteriorCurve(antes);
		getImpl().upperLayer = antes;
		getImpl().corneaOffsetX = offsetX;
		getImpl().isCorneaCenter = true;
	}
	else {
		getImpl().isCorneaCenter = false;
	}


	if (width == getWidth() && height == getHeight()) {
		if (getImpl().image.fromBitsData(data, width, height)) {
			updateWindow();
			getImpl().image.copyTo(&getImpl().imageOriginal);
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
			image.copyTo(&getImpl().imageOriginal);
		}
	}
	return;
}


PreviewImageCallback2 * ViewScanPreview::getCallbackFunction(void)
{
	return &getImpl().callback;
}


struct ExportFilePath {
	CString operator()(CString fname) {
		CString path = fname;
		if (path.IsEmpty()) {
			CTime time = CTime::GetCurrentTime();
			path = time.Format(_T("PREVIEW_%y%m%d_%H%M%S.png"));
		}

		CString dirname = _T(".//export");
		CreateDirectory(dirname, NULL);
		path = dirname + _T("//") + path;
		return path;
	}
};


bool ViewScanPreview::saveImage(CString & filename)
{
	if (!getImpl().image.isEmpty()) {
		filename = ExportFilePath()(filename);
		return getImpl().image.saveFile(wtoa(filename), true);
	}
	return false;
}


bool ViewScanPreview::saveImageOriginal(CString & filename)
{
	if (!getImpl().imageOriginal.isEmpty()) {
		filename = ExportFilePath()(filename);
		return getImpl().imageOriginal.saveFile(wtoa(filename), true);
	}
	return false;
}


ViewScanPreview::ViewScanPreviewImpl & ViewScanPreview::getImpl(void) const
{
	return *d_ptr;
}


BEGIN_MESSAGE_MAP(ViewScanPreview, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// ViewScanPreview message handlers


void ViewScanPreview::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
}


void ViewScanPreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (d_ptr->numPoints >= 2) {
		d_ptr->numPoints = 0;
	}

	d_ptr->points[d_ptr->numPoints] = point;
	d_ptr->numPoints++;

	updateWindow();

	CStatic::OnLButtonDown(nFlags, point);
}


void ViewScanPreview::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (d_ptr->numPoints > 0) {
		d_ptr->numPoints = 0;
		updateWindow();
	}

	CStatic::OnRButtonDown(nFlags, point);
}
