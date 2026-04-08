// ScanIntensityView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ScanIntensityView.h"

#include <vector>
#include <array>
#include <iostream>
#include <fstream>

#include "CppUtil2.h"

using namespace std;
using namespace CppUtil;



struct ScanIntensityView::ScanIntensityViewImpl
{
	int width;
	int height;
	int idxLine;

	float plotRatioX;
	float plotRatioY;
	CRect plotRect;

	float intsData[BUFFER_SIZE];
	float compData[BUFFER_SIZE];
	float intsLine[DATA_HEIGHT];
	float compLine[DATA_HEIGHT];
	float backLine[DATA_HEIGHT];

	IntensityDataCallback cbIntensityData;

	ScanIntensityViewImpl() : idxLine(DATA_WIDTH/2)
	{

	}
};



// ScanIntensityView

IMPLEMENT_DYNAMIC(ScanIntensityView, CStatic)

ScanIntensityView::ScanIntensityView()
	: d_ptr(make_unique<ScanIntensityViewImpl>())
{
	getImpl().cbIntensityData = std::bind(&ScanIntensityView::callbackIntensityData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

}


ScanIntensityView::~ScanIntensityView()
{
}


void ScanIntensityView::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	CBitmap* oldBmp = pMemDC->SelectObject(&bitmap);

	drawIntensityGraph(pMemDC);
	drawIntensityDataLine(pMemDC);
	drawCompensatedDataLine(pMemDC);
	drawBackgroundDataLine(pMemDC);
	drawGraphText(pMemDC);

	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}


void ScanIntensityView::drawIntensityGraph(CDC * pDC)
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
	getImpl().plotRatioX = (float)DATA_HEIGHT / (float)rcPlot.Width();
	getImpl().plotRatioY = (float)SPECTRO_CALIB_INTENSITY_MAX / (float)rcPlot.Height();

	pDC->SelectObject(oldPen);
	pen.DeleteObject();
	return;
}


void ScanIntensityView::drawIntensityDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	float* data = getIntensityLine(DATA_WIDTH/2);
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


void ScanIntensityView::drawCompensatedDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	float* data = getCompensatedLine(DATA_WIDTH / 2);
	if (data == nullptr) {
		return;
	}

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 32, 255));
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


void ScanIntensityView::drawBackgroundDataLine(CDC * pDC)
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


void ScanIntensityView::drawGraphText(CDC * pDC)
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


void ScanIntensityView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int ScanIntensityView::getWidth(void) const
{
	return getImpl().width;
}


int ScanIntensityView::getHeight(void) const
{
	return getImpl().height;
}


void ScanIntensityView::setCurrentLine(int index)
{
	getImpl().idxLine = index;
	return;
}


float * ScanIntensityView::getIntensityLine(int index)
{
	if (index < 0 || index >= DATA_WIDTH) {
		return nullptr;
	}
	for (int i = 0; i < DATA_HEIGHT; i++) {
		getImpl().intsLine[i] = getImpl().intsData[i*BUFFER_WIDTH + index];
	}
	return getImpl().intsLine;
}


float * ScanIntensityView::getIntensityData(void)
{
	return getImpl().intsData;
}


float * ScanIntensityView::getCompensatedLine(int index)
{
	if (index < 0 || index >= DATA_WIDTH) {
		return nullptr;
	}
	for (int i = 0; i < DATA_HEIGHT; i++) {
		getImpl().compLine[i] = getImpl().compData[i*BUFFER_WIDTH + index];
	}
	return getImpl().compLine;
}


float * ScanIntensityView::getCompensatedData(void)
{
	return getImpl().compData;
}


float * ScanIntensityView::getBackgroundLine(void)
{
	return getImpl().backLine;
}


void ScanIntensityView::setIntensityLine(float* data, int index)
{
	for (int i = 0; i < DATA_HEIGHT; i++) {
		getImpl().intsData[i*BUFFER_WIDTH + index] = data[i];
	}
	return;
}


void ScanIntensityView::setCompensatedLine(float * data, int index)
{
	for (int i = 0; i < DATA_HEIGHT; i++) {
		getImpl().compData[i*BUFFER_WIDTH + index] = data[i];
	}
	return;
}


void ScanIntensityView::setBackgroundLine(float * data)
{
	if (data == nullptr) {
		data = getIntensityLine();
	}

	for (int i = 0; i < DATA_HEIGHT; i++) {
		getImpl().backLine[i] = data[i];
	}
	return;
}


bool ScanIntensityView::evaluateIntensityLine(CString & text)
{
	text.Format(_T("Peak --- at ---, FWHM --- (--- ~ ---)"));

	float* data = getIntensityLine();
	if (data == nullptr) {
		return false;
	}

	float maxVal = 0;
	int maxIdx = 0;
	for (int i = 50; i < DATA_HEIGHT; i++) {
		if (maxVal < data[i]) {
			maxVal = data[i];
			maxIdx = i;
		}
	}

	float halfVal = maxVal / 2.0f;
	int halfIdx[2] = { 0 };
	int FWHM = 0;
	for (int i = maxIdx; i >= 0; i--) {
		if (halfVal <= data[i]) {
			halfIdx[0] = i;
		}
		else {
			break;
		}
	}

	for (int i = maxIdx; i < DATA_HEIGHT; i++) {
		if (halfVal <= data[i]) {
			halfIdx[1] = i;
		}
		else {
			break;
		}
	}

	FWHM = halfIdx[1] - halfIdx[0];
	text.Format(_T("Peak %.0f at %d, FWHM %d (%d ~ %d)"), maxVal, maxIdx, FWHM, halfIdx[0], halfIdx[1]);
	return true;
}


bool ScanIntensityView::saveIntensityData(const std::wstring & path)
{
	float* pBuff = getIntensityData();
	float* pLine = pBuff;
	int lineSize = BUFFER_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	while (lineCount < DATA_HEIGHT) {
		ofs.write(reinterpret_cast<char*>(pLine), lineSize);
		pLine += BUFFER_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Save intensity data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ScanIntensityView::loadIntensityData(const std::wstring & path)
{
	float* pBuff = getIntensityData();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = BUFFER_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast != lineSize) {
			break;
		}
		pLine += BUFFER_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Load intensity data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ScanIntensityView::saveCompensatedData(const std::wstring & path)
{
	float* pBuff = getCompensatedData();
	float* pLine = pBuff;
	int lineSize = BUFFER_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	while (lineCount < DATA_HEIGHT) {
		ofs.write(reinterpret_cast<char*>(pLine), lineSize);
		pLine += BUFFER_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Save compensated data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ScanIntensityView::loadCompensatedData(const std::wstring & path)
{
	float* pBuff = getCompensatedData();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = BUFFER_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast != lineSize) {
			break;
		}
		pLine += BUFFER_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Load compensated data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ScanIntensityView::saveBackgroundData(const std::wstring & path)
{
	float* pBuff = getBackgroundLine();
	float* pLine = pBuff;
	int lineSize = DATA_HEIGHT * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);
	ofs.write(reinterpret_cast<char*>(pLine), lineSize);
	lineCount++;

	DebugOut2() << "Save background data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ScanIntensityView::loadBackgroundData(const std::wstring & path)
{
	float* pBuff = getBackgroundLine();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = DATA_HEIGHT * sizeof(float);
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


IntensityDataCallback * ScanIntensityView::getIntensityCallback(void) const
{
	return &getImpl().cbIntensityData;
}


ScanIntensityView::ScanIntensityViewImpl & ScanIntensityView::getImpl(void) const
{
	return *d_ptr;
}


void ScanIntensityView::callbackIntensityData(float * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	memcpy((void*)getImpl().intsData, (void*)data, sizeof(float)*BUFFER_SIZE);
	updateWindow();
	return;
}


BEGIN_MESSAGE_MAP(ScanIntensityView, CStatic)
END_MESSAGE_MAP()



// ScanIntensityView message handlers




void ScanIntensityView::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{

	// TODO:  Add your code to draw the specified item
}
