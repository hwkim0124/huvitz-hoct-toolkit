// ViewScanIntensity.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewScanIntensity.h"

#include <vector>
#include <array>
#include <iostream>
#include <fstream>

#include "SigProc2.h"
#include "CppUtil2.h"

using namespace std;
using namespace CppUtil;
using namespace SigProc;



struct ViewScanIntensity::ViewScanIntensityImpl
{
	DisplayMode dispMode;

	int width;
	int height;
	int idxLine;

	float plotRatioX;
	float plotRatioY;
	CRect plotRect;

	float intsData[FRAME_SIZE];
	float compData[FRAME_SIZE];
	float intsLine[FRAME_HEIGHT];
	float compLine[FRAME_HEIGHT];
	float backLine[FRAME_HEIGHT];

	int imageRows;
	int imageCols;
	int frameRows;
	int frameCols;
	int wPaddSize;

	Magnitude magnitude;
	float snrMean;
	float snrMaxVal;
	int snrMaxIdx;

	IntensityDataCallback cbIntensityData;

	ViewScanIntensityImpl() : idxLine(FRAME_WIDTH /2), dispMode(DisplayMode::INTENSITY), 
					imageRows(0), imageCols(0), frameRows(0), frameCols(0), wPaddSize(0)
	{

	}
};



// ViewScanIntensity

IMPLEMENT_DYNAMIC(ViewScanIntensity, CStatic)

ViewScanIntensity::ViewScanIntensity()
	: d_ptr(make_unique<ViewScanIntensityImpl>())
{
	getImpl().cbIntensityData = std::bind(&ViewScanIntensity::callbackIntensityData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}


ViewScanIntensity::~ViewScanIntensity()
{
}


void ViewScanIntensity::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	CBitmap* oldBmp = pMemDC->SelectObject(&bitmap);

	if (getImpl().dispMode == DisplayMode::SNR_RATIO) {
		drawSnrRatioGraph(pMemDC);
		drawSnrRatioLine(pMemDC);
		drawSnrRatioText(pMemDC);
	}
	else {
		drawIntensityGraph(pMemDC);
		drawIntensityDataLine(pMemDC);
		// drawCompensatedDataLine(pMemDC);
		// drawBackgroundDataLine(pMemDC);
		drawGraphText(pMemDC);
	}

	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}


void ViewScanIntensity::drawIntensityGraph(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	CRect rect(0, 0, width, height);
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	oldPen = pDC->SelectObject(&pen);

	CRect rcPlot(20, 10, getWidth() - 20, getHeight() - 40);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.left, rcPlot.top);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.right, rcPlot.bottom);

	getImpl().plotRect = rcPlot;
	getImpl().plotRatioX = (float)SPECTRO_CALIB_FRAME_DATA_HEIGHT / (float)rcPlot.Width();
	getImpl().plotRatioY = (float)SPECTRO_CALIB_INTENSITY_MAX / (float)rcPlot.Height();

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ViewScanIntensity::drawIntensityDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	int index = getImpl().imageCols / 2;
	float* data = getIntensityLine(index);
	if (data == nullptr) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	old = pDC->SelectObject(&pen);

	float ratioX = getImpl().plotRatioX;
	float ratioY = getImpl().plotRatioY;
	int dataY = 0;
	for (int k = 0; k < wPlot; k++) {
		index = (int)(k * ratioX);
		dataY = (int)(data[index] / ratioY);

		if (k == 0) {
			pDC->MoveTo(orgX + k, orgY - dataY);
		}
		else {
			pDC->LineTo(orgX + k, orgY - dataY);
		}
	}

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanIntensity::drawCompensatedDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	int index = getImpl().imageCols / 2;
	float* data = getCompensatedLine(index);
	if (data == nullptr) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 32, 255));
	old = pDC->SelectObject(&pen);

	float ratioX = getImpl().plotRatioX;
	float ratioY = getImpl().plotRatioY;
	int dataY = 0;
	for (int k = 0; k < wPlot; k++) {
		index = (int)(k * ratioX);
		dataY = (int)(data[index] / ratioY);

		if (k == 0) {
			pDC->MoveTo(orgX + k, orgY - dataY);
		}
		else {
			pDC->LineTo(orgX + k, orgY - dataY);
		}
	}

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanIntensity::drawBackgroundDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	float* data = getBackgroundLine();
	if (data == nullptr) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	old = pDC->SelectObject(&pen);

	float ratioX = getImpl().plotRatioX;
	float ratioY = getImpl().plotRatioY;
	int index = 0;
	int dataY = 0;
	for (int k = 0; k < wPlot; k++) {
		index = (int)(k * ratioX);
		dataY = (int)(data[index] / ratioY);

		if (k == 0) {
			pDC->MoveTo(orgX + k, orgY - dataY);
		}
		else {
			pDC->LineTo(orgX + k, orgY - dataY);
		}
	}

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanIntensity::drawGraphText(CDC * pDC)
{
	CRect rcPlot = getImpl().plotRect;

	CFont font, *oldFont;
	font.CreateFontW(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	str.Format(_T("%d"), SPECTRO_CALIB_INTENSITY_MAX);
	CRect rect = CRect(rcPlot.left + 5, rcPlot.top, rcPlot.left + 100, rcPlot.top + 20);
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);

	evaluateIntensityLine(str);
	rect = CRect(rcPlot.left + 10, rcPlot.bottom + 10, rcPlot.left + 480, rcPlot.bottom + 25);
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanIntensity::drawSnrRatioGraph(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	CRect rect(0, 0, width, height);
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	CPen pen, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	oldPen = pDC->SelectObject(&pen);

	CRect rcPlot(20, 10, getWidth() - 20, getHeight() - 40);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.left, rcPlot.top);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.right, rcPlot.bottom);

	getImpl().plotRect = rcPlot;
	getImpl().plotRatioX = (float)getImpl().magnitude.imageCols() / (float)rcPlot.Width();
	getImpl().plotRatioY = (float)SNR_RATIO_MAX / (float)rcPlot.Height();

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ViewScanIntensity::drawSnrRatioLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	if (getImpl().magnitude.frameCols() <= 0) {
		return;
	}

	float* data = getImpl().magnitude.getSnrRatios();
	if (data == nullptr) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 32, 32));
	old = pDC->SelectObject(&pen);

	float ratioX = getImpl().plotRatioX;
	float ratioY = getImpl().plotRatioY;
	int index = 0;
	int dataY = 0;
	int count = 0;
	float dsum = 0.0f;
	float dmax = 0.0f;
	int midx = 0;

	for (int k = 1; k < wPlot; k++) {
		index = (int)(k * ratioX);
		dataY = (int)(data[index] / ratioY);

		if (k == 1) {
			pDC->MoveTo(orgX + k, orgY - dataY);
		}
		else {
			pDC->LineTo(orgX + k, orgY - dataY);
		}

		if (data[index] > dmax) {
			dmax = data[index];
			midx = index;
		}
		dsum += data[index];
		count++;
	}

	getImpl().snrMaxIdx = midx;
	getImpl().snrMaxVal = dmax;
	getImpl().snrMean = dsum / count;

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanIntensity::drawSnrRatioText(CDC * pDC)
{
	CRect rcPlot = getImpl().plotRect;

	CFont font, *oldFont;
	font.CreateFontW(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	str.Format(_T("%.1f"), SNR_RATIO_MAX);
	CRect rect = CRect(rcPlot.left + 5, rcPlot.top, rcPlot.left + 100, rcPlot.top + 20);
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);


	str.Format(_T("Average SNR ---, max --- at ---"));

	float* data = getImpl().magnitude.getSnrRatios();
	if (data != nullptr) {
		str.Format(_T("Average SNR %.1f, max %.1f at %d"), getImpl().snrMean, getImpl().snrMaxVal, getImpl().snrMaxIdx);
	}
	rect = CRect(rcPlot.left + 10, rcPlot.bottom + 10, rcPlot.left + 480, rcPlot.bottom + 25);
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanIntensity::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ViewScanIntensity::setDisplayMode(DisplayMode mode)
{
	getImpl().dispMode = mode;
	return;
}


int ViewScanIntensity::getWidth(void) const
{
	return getImpl().width;
}


int ViewScanIntensity::getHeight(void) const
{
	return getImpl().height;
}


void ViewScanIntensity::setCurrentLine(int index)
{
	getImpl().idxLine = index;
	return;
}


float * ViewScanIntensity::getIntensityLine(int index)
{
	if (index < 0 || index >= getImpl().imageCols) {
		return nullptr;
	}
	for (int i = 0; i < getImpl().imageRows; i++) {
		getImpl().intsLine[i] = getImpl().intsData[i*FRAME_WIDTH + index];
	}
	return getImpl().intsLine;
}


float * ViewScanIntensity::getIntensityData(void)
{
	return getImpl().intsData;
}


float * ViewScanIntensity::getCompensatedLine(int index)
{
	if (index < 0 || index >= getImpl().imageCols) {
		return nullptr;
	}
	for (int i = 0; i < getImpl().imageRows; i++) {
		getImpl().compLine[i] = getImpl().compData[i*FRAME_WIDTH + index];
	}
	return getImpl().compLine;
}


float * ViewScanIntensity::getCompensatedData(void)
{
	return getImpl().compData;
}


float * ViewScanIntensity::getBackgroundLine(void)
{
	return getImpl().backLine;
}


void ViewScanIntensity::setIntensityLine(float* data, int index)
{
	for (int i = 0; i < getImpl().imageRows; i++) {
		getImpl().intsData[i*FRAME_WIDTH + index] = data[i];
	}
	return;
}


void ViewScanIntensity::setCompensatedLine(float * data, int index)
{
	for (int i = 0; i < getImpl().imageRows; i++) {
		getImpl().compData[i*FRAME_WIDTH + index] = data[i];
	}
	return;
}


void ViewScanIntensity::setBackgroundLine(float * data)
{
	if (data == nullptr) {
		data = getIntensityLine();
	}

	for (int i = 0; i < getImpl().imageRows; i++) {
		getImpl().backLine[i] = data[i];
	}
	return;
}


bool ViewScanIntensity::evaluateIntensityLine(CString & text)
{
	text.Format(_T("Peak --- at ---, FWHM --- (--- ~ ---)"));

	float* data = getIntensityLine();
	if (data == nullptr) {
		return false;
	}

	float maxVal = 0;
	int maxIdx = 0;
	int margin = 50;

	for (int i = margin; i < (getImpl().imageRows - margin); i++) {
		if (maxVal < data[i]) {
			maxVal = data[i];
			maxIdx = i;
		}
	}

	float halfMax = maxVal / 2.0f;
	int halfIdx[2] = { 0 };

	for (int i = maxIdx; i >= margin; i--) {
		halfIdx[0] = i;
		if (halfMax < data[i]) {
			break;
		}
	}

	for (int i = maxIdx; i < (getImpl().imageRows - margin); i++) {
		halfIdx[1] = i;
		if (halfMax < data[i]) {
			break;
		}
	}

	int idx1, idx2;
	float val1, val2, dist, fwhm;

	idx1 = halfIdx[0] + 1;
	idx2 = halfIdx[0];
	val1 = data[idx1];
	val2 = data[idx2];

	dist = 1.0f - ((halfMax - val2) / (val1 - val2));
	fwhm = (maxIdx - idx1 + dist);

	idx1 = halfIdx[1] - 1;
	idx2 = halfIdx[1];
	val1 = data[idx1];
	val2 = data[idx2];

	dist = 1.0f - ((halfMax - val2) / (val1 - val2));
	fwhm += (idx1 - maxIdx + dist);

	text.Format(_T("Peak %.0f at %d, FWHM %.2f (%d ~ %d)"), maxVal, maxIdx, fwhm, halfIdx[0], halfIdx[1]);
	return true;
}


bool ViewScanIntensity::saveIntensityData(const std::wstring & path)
{
	float* pBuff = getIntensityData();
	float* pLine = pBuff;
	int lineSize = FRAME_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	while (lineCount < getImpl().imageCols) {
		ofs.write(reinterpret_cast<char*>(pLine), lineSize);
		pLine += FRAME_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Save intensity data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanIntensity::loadIntensityData(const std::wstring & path)
{
	float* pBuff = getIntensityData();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = FRAME_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast != lineSize) {
			break;
		}
		pLine += FRAME_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Load intensity data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanIntensity::saveCompensatedData(const std::wstring & path)
{
	float* pBuff = getCompensatedData();
	float* pLine = pBuff;
	int lineSize = FRAME_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	while (lineCount < FRAME_WIDTH) {
		ofs.write(reinterpret_cast<char*>(pLine), lineSize);
		pLine += FRAME_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Save compensated data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanIntensity::loadCompensatedData(const std::wstring & path)
{
	float* pBuff = getCompensatedData();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = FRAME_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast != lineSize) {
			break;
		}
		pLine += FRAME_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Load compensated data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanIntensity::saveBackgroundData(const std::wstring & path)
{
	float* pBuff = getBackgroundLine();
	float* pLine = pBuff;
	int lineSize = FRAME_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);
	ofs.write(reinterpret_cast<char*>(pLine), lineSize);
	lineCount++;

	DebugOut2() << "Save background data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanIntensity::loadBackgroundData(const std::wstring & path)
{
	float* pBuff = getBackgroundLine();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = FRAME_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	if (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast == lineSize) {
			lineCount++;
		}
	}

	DebugOut2() << "Load background data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


IntensityDataCallback * ViewScanIntensity::getIntensityCallback(void) const
{
	return &getImpl().cbIntensityData;
}


ViewScanIntensity::ViewScanIntensityImpl & ViewScanIntensity::getImpl(void) const
{
	return *d_ptr;
}


void ViewScanIntensity::callbackIntensityData(float * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	getImpl().frameRows = FRAME_HEIGHT;
	getImpl().frameCols = FRAME_WIDTH;
	getImpl().imageRows = height;
	getImpl().imageCols = width;
	getImpl().wPaddSize = getImpl().frameCols - getImpl().imageCols;

	// Frame size with data is fixed with 1025 x 1025 (in case less ascans 1024).
	memcpy((void*)getImpl().intsData, (void*)data, sizeof(float)*getImpl().frameRows*getImpl().frameCols);

	getImpl().magnitude.setIntensityData(getImpl().intsData, getImpl().frameCols, 
										 getImpl().frameRows, getImpl().wPaddSize, 0);

	if (getImpl().dispMode == DisplayMode::SNR_RATIO)
	{
		bool ret = false;
		ret = getImpl().magnitude.buildHistogram(IMAGE_ROW_OFFSET, 0, 
												getImpl().imageRows,
												getImpl().imageCols, 
												HISTOGRAM_BINS, 
												HISTOGRAM_UNIT);
		if (!ret) {
		//	return;
		}

		ret = getImpl().magnitude.updateSignalToNoiseRatios(SIGNAL_RATIO, NOISE_RATIO);
		if (!ret) {
		//	return;
		}
	}

	updateWindow();
	return;
}


BEGIN_MESSAGE_MAP(ViewScanIntensity, CStatic)
END_MESSAGE_MAP()



// ViewScanIntensity message handlers




void ViewScanIntensity::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
}
