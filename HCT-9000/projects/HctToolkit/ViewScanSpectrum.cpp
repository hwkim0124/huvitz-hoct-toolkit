// ViewScanSpectrum.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ViewScanSpectrum.h"

#include <vector>
#include <array>
#include <iostream>
#include <fstream>

#include "CppUtil2.h"
#include "SigProc2.h"
#include "SigChain2.h"

using namespace std;
using namespace CppUtil;
using namespace SigProc;
using namespace SigChain;


struct ViewScanSpectrum::ViewScanSpectrumImpl
{
	int width;
	int height;

	int numBuffers;
	int spectrumBufferIndex;
	int resampleBufferIndex;

	float plotRatioX;
	float plotRatioY;
	CRect plotRect;

	int markerIndex[2];
	int markerCount;

	float dataFWHM;
	int fwhmIndex1;
	int fwhmIndex2;

	int resampleDataMax;
	int resampleDataMin;
	int spectrumDataMax;
	int spectrumDataMin;

	float resampleData[BUFFER_SIZE];
	unsigned short spectrumData[BUFFER_SIZE];

	vector<array<unsigned short, DATA_WIDTH>> spectrums;
	vector<array<float, DATA_WIDTH>> resamples;

	SpectrumDataCallback cbSpectrumData;
	ResampleDataCallback cbResampleData;

	ViewScanSpectrumImpl() : numBuffers(1), spectrumBufferIndex(0), resampleBufferIndex(0),
							resampleDataMax(0), spectrumDataMax(0), resampleDataMin(0), spectrumDataMin(0), 
							markerCount(0), markerIndex{0},
							dataFWHM(0.0f), fwhmIndex1(0), fwhmIndex2(0)
	{

	}
};


// ViewScanSpectrum

IMPLEMENT_DYNAMIC(ViewScanSpectrum, CStatic)

ViewScanSpectrum::ViewScanSpectrum()
	: d_ptr(make_unique<ViewScanSpectrumImpl>())
{
	setAverageSize(1);

	getImpl().cbSpectrumData = std::bind(&ViewScanSpectrum::callbackSpectrumData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	getImpl().cbResampleData = std::bind(&ViewScanSpectrum::callbackResampleData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	return;
}


ViewScanSpectrum::~ViewScanSpectrum()
{
}


void ViewScanSpectrum::updateWindow(void)
{
	CDC* pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();
	HDC hMemDC = CreateCompatibleDC(hDC);
	CDC* pMemDC = CDC::FromHandle(hMemDC);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, getWidth(), getHeight());
	CBitmap* oldBmp = pMemDC->SelectObject(&bitmap);

	drawSpectrumGraph(pMemDC);
	drawSpectrumDataLine(pMemDC);
	drawResampleDataLine(pMemDC);
	drawGraphText(pMemDC);

	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}


void ViewScanSpectrum::drawSpectrumGraph(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	CRect rect(0, 0, width, height);
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	CPen pen, *old;
	pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	old = pDC->SelectObject(&pen);

	CRect rcPlot(20, 10, getWidth() - 20, getHeight() - 40);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.left, rcPlot.top);
	pDC->MoveTo(rcPlot.left, rcPlot.bottom);
	pDC->LineTo(rcPlot.right, rcPlot.bottom);

	getImpl().plotRect = rcPlot;
	getImpl().plotRatioX = (float)DATA_WIDTH / (float)rcPlot.Width();
	getImpl().plotRatioY = (float)LINE_CAMERA_PIXEL_VALUE_MAX / (float)rcPlot.Height();

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanSpectrum::drawSpectrumDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	unsigned short* data = getSpectrumLine();
	if (data == nullptr) {
		return;
	}

	CPen pen, pen2, *oldPen;
	pen.CreatePen(PS_SOLID, 1, RGB(32, 255, 32));
	pen2.CreatePen(PS_SOLID, 2, RGB(255, 32, 32));
	oldPen = pDC->SelectObject(&pen);

	float ratioX = getImpl().plotRatioX;
	float ratioY = getImpl().plotRatioY;
	int index = 0;
	int dataY = 0;
	int dataMax = 0, maxIdx = 0;
	int dataMin = 999999, minIdx = 0;
	for (int k = 0; k < wPlot; k++) {
		index = (int)(k * ratioX);
		dataY = (int)(data[index] / ratioY);

		if (k == 0) {
			pDC->MoveTo(orgX + k, orgY - dataY);
		}
		else {
			pDC->LineTo(orgX + k, orgY - dataY);
		}

		if (dataMax < data[index]) {
			dataMax = data[index];
			maxIdx = index;
		}
		if (dataMin > data[index]) {
			dataMin = data[index];
			minIdx = index;
		}
	}

	CFont font, *oldFont;
	font.CreateFontW(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	int dataMid = dataMax / 2;
	int midx1 = 0, midx2 = 0;
	for (index = 0; index < maxIdx; index++) {
		if (data[index] >= dataMid) {
			midx1 = index;
			break;
		}
	}
	for (index = DATA_WIDTH - 1; index >= maxIdx; index--) {
		if (data[index] >= dataMid) {
			midx2 = index;
			break;
		}
	}

	int x1 = orgX + (int)(midx1 / ratioX);
	int y1 = orgY - (int)(data[midx1] / ratioY);
	int x2 = orgX + (int)(midx2 / ratioX);
	int y2 = orgY - (int)(data[midx2] / ratioY);

	pDC->MoveTo(x1, y1);
	pDC->LineTo(x2, y2);

	double* wlens = ChainSetup::getResampler()->detectorWaveLength();
	getImpl().dataFWHM = (float)(wlens[midx2] - wlens[midx1]);
	getImpl().fwhmIndex1 = midx1;
	getImpl().fwhmIndex2 = midx2;


	if (getImpl().markerCount > 0)
	{
		int xpos, ypos;
		CString text;
		CRect rect;
		for (int i = 0; i < getImpl().markerCount; i++) {
			index = getImpl().markerIndex[i];
			xpos = orgX + (int)(index / ratioX);
			ypos = orgY - (int)(data[index] / ratioY);

			pDC->SelectObject(&pen2);
			pDC->Ellipse(xpos - 3, ypos - 3, xpos + 3, ypos + 3);

			if (ypos > (getImpl().plotRect.bottom - 80)) {
				ypos -= 80;
			}

			text.Format(_T("idx%d: %d"), i + 1, index);
			rect.SetRect(xpos, ypos + 10, xpos + 80, ypos + 25);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

			text.Format(_T("peak: %d"), data[index]);
			rect.SetRect(xpos, ypos + 25, xpos + 80, ypos + 40);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

				text.Format(_T("wlen: %.2lf"), wlens[index]);
			rect.SetRect(xpos, ypos + 40, xpos + 80, ypos + 55);
			pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);

			if (i == 1) {
				text.Format(_T("dist: %.2lf"), wlens[index] - wlens[getImpl().markerIndex[0]]);
				rect.SetRect(xpos, ypos + 55, xpos + 80, ypos + 70);
				pDC->DrawText(text, &rect, DT_LEFT | DT_SINGLELINE);
			}
		}
	}

	getImpl().spectrumDataMax = dataMax;
	getImpl().spectrumDataMin = dataMin;

	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldFont);
	pen.DeleteObject();
	pen2.DeleteObject();
	font.DeleteObject();
	return;
}


void ViewScanSpectrum::drawResampleDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	float* data = getResampleLine();
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
	int dataMax = 0;
	int dataMin = 0;
	for (int k = 0; k < wPlot; k++) {
		index = (int)(k * ratioX);
		dataY = (int)(data[index] / ratioY);

		if (k == 0) {
			pDC->MoveTo(orgX + k, orgY - dataY);
		}
		else {
			pDC->LineTo(orgX + k, orgY - dataY);
		}
		dataMax = (int)(dataMax < data[index] ? data[index] : dataMax);
		dataMin = (int)(dataMin > data[index] ? data[index] : dataMin);
	}

	getImpl().resampleDataMax = dataMax;
	getImpl().resampleDataMin = dataMin;

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void ViewScanSpectrum::drawGraphText(CDC * pDC)
{
	CRect rcPlot = getImpl().plotRect;

	CFont font, *oldFont;
	font.CreateFontW(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Arial"));
	oldFont = pDC->SelectObject(&font);

	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	CString str;
	str.Format(_T("%d"), LINE_CAMERA_PIXEL_VALUE_MAX);
	CRect rect = CRect(rcPlot.left + 5, rcPlot.top, rcPlot.left + 100, rcPlot.top + 20);
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);

	str.Format(_T("Spectrum max %d, min %d, fwhm %.2f (%d ~ %d) - Resample max %d, min %d"),
		getImpl().spectrumDataMax, getImpl().spectrumDataMin,
		getImpl().dataFWHM, getImpl().fwhmIndex1, getImpl().fwhmIndex2,
		getImpl().resampleDataMax, getImpl().resampleDataMin);

	rect = CRect(rcPlot.left + 10, rcPlot.bottom + 10, rcPlot.left + 480, rcPlot.bottom + 25);
	pDC->DrawText(str, &rect, DT_LEFT | DT_SINGLELINE);

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	return;
}


void ViewScanSpectrum::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ViewScanSpectrum::setAverageSize(int size)
{
	getImpl().numBuffers = (size <= 1 ? 1 : (size + 1));

	getImpl().spectrums.clear();
	getImpl().resamples.clear();
	for (int k = 0; k < getImpl().numBuffers; k++) {
		getImpl().spectrums.push_back(array<unsigned short, DATA_WIDTH>{0});
		getImpl().resamples.push_back(array<float, DATA_WIDTH>{0});
	}

	getImpl().spectrumBufferIndex = 0;
	getImpl().resampleBufferIndex = 0;
	return;
}


int ViewScanSpectrum::getAverageSize(void)
{
	return getImpl().numBuffers;
}


int ViewScanSpectrum::getWidth(void) const
{
	return getImpl().width;
}


int ViewScanSpectrum::getHeight(void) const
{
	return getImpl().height;
}


unsigned short * ViewScanSpectrum::getSpectrumLine(void)
{
	size_t size = getImpl().spectrums.size();
	if (size <= 0) {
		return nullptr;
	}

	if (size == 1) {
		return getImpl().spectrums[0].data();
	}
	else {
		// Make average spectrum data.
		unsigned short numData = (unsigned short)(size - 1);
		getImpl().spectrums[numData].assign(0);
		unsigned short* data = getImpl().spectrums[numData].data();
		unsigned short* buff;

		for (int k = 0; k < numData; k++) {
			buff = getImpl().spectrums[k].data();
			for (int j = 0; j < DATA_WIDTH; j++) {
				data[j] += buff[j];
			}
		}

		for (int j = 0; j < DATA_WIDTH; j++) {
			data[j] /= numData;
		}
		return data;
	}
}


unsigned short * ViewScanSpectrum::getSpectrumData(void)
{
	return getImpl().spectrumData;
}


float * ViewScanSpectrum::getResampleLine(void)
{
	size_t size = getImpl().resamples.size();
	if (size <= 0) {
		return nullptr;
	}

	if (size == 1) {
		return getImpl().resamples[0].data();
	}
	else {
		// Make average resample data.
		unsigned short numData = (unsigned short)(size - 1);
		getImpl().resamples[numData].assign(0);
		float* data = getImpl().resamples[numData].data();
		float* buff;

		for (int k = 0; k < numData; k++) {
			buff = getImpl().resamples[k].data();
			for (int j = 0; j < DATA_WIDTH; j++) {
				data[j] += buff[j];
			}
		}

		for (int j = 0; j < DATA_WIDTH; j++) {
			data[j] /= numData;
		}
		return data;
	}
}


float * ViewScanSpectrum::getResampleData(void)
{
	return getImpl().resampleData;
}


int ViewScanSpectrum::getDataWidth(void)
{
	return DATA_WIDTH;
}


int ViewScanSpectrum::getDataHeight(void)
{
	return DATA_HEIGHT;
}


void ViewScanSpectrum::initSpectrumLine(unsigned short * data, int index)
{
	if (getAverageSize() != 1) {
		setAverageSize(1);
	}

	unsigned short* buffer = getImpl().spectrums[0].data();
	int dataIdx = DATA_WIDTH * index;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(unsigned short) * DATA_WIDTH);
	return;
}


void ViewScanSpectrum::initResampleLine(float * data, int index)
{
	if (getAverageSize() != 1) {
		setAverageSize(1);
	}

	float* buffer = getImpl().resamples[0].data();
	int dataIdx = DATA_WIDTH * index;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(float) * DATA_WIDTH);
	return;
}


bool ViewScanSpectrum::saveSpectrumData(const std::wstring & path)
{
	unsigned short* pBuff = getSpectrumData();
	unsigned short* pLine = pBuff;
	int lineSize = DATA_WIDTH * sizeof(unsigned short);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	while (lineCount < DATA_HEIGHT) {
		ofs.write(reinterpret_cast<char*>(pLine), lineSize);
		pLine += DATA_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Save spectrum data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanSpectrum::loadSpectrumData(const std::wstring & path)
{
	unsigned short* pBuff = getSpectrumData();
	unsigned short* pLine = pBuff;
	size_t readLast;
	int lineSize = DATA_WIDTH * sizeof(unsigned short);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize) ;
		readLast = ifs.gcount();
		if (readLast != lineSize) {
			break;
		}
		pLine += DATA_WIDTH;
		lineCount++;
	}

	if (lineCount > 0) {
		initSpectrumLine(pBuff, lineCount / 2);
	}

	DebugOut2() << "Load spectrum data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanSpectrum::saveResampleData(const std::wstring & path)
{
	float* pBuff = getResampleData();
	float* pLine = pBuff;
	int lineSize = DATA_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);

	while (lineCount < DATA_HEIGHT) {
		ofs.write(reinterpret_cast<char*>(pLine), lineSize);
		pLine += DATA_WIDTH;
		lineCount++;
	}

	DebugOut2() << "Save resample data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool ViewScanSpectrum::loadResampleData(const std::wstring & path)
{
	float* pBuff = getResampleData();
	float *pLine = pBuff;
	size_t readLast;
	int lineSize = DATA_WIDTH * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	while (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast != lineSize) {
			break;
		}
		pLine += DATA_WIDTH;
		lineCount++;
	}

	if (lineCount > 0) {
		initResampleLine(pBuff, lineCount / 2);
	}

	DebugOut2() << "Load resample data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


SpectrumDataCallback * ViewScanSpectrum::getSpectrumCallback(void) const
{
	return &getImpl().cbSpectrumData;
}


ResampleDataCallback * ViewScanSpectrum::getResampleCallback(void) const
{
	return &getImpl().cbResampleData;
}


ViewScanSpectrum::ViewScanSpectrumImpl & ViewScanSpectrum::getImpl(void) const
{
	return *d_ptr;
}


void ViewScanSpectrum::callbackSpectrumData(unsigned short * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	// Copy spectrum data at center line. 
	int buffIdx = getImpl().spectrumBufferIndex;
	unsigned short* buffer = getImpl().spectrums[buffIdx].data();
	int dataIdx = (height / 2) * width;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(unsigned short) * DATA_WIDTH);

	int numBuff = getImpl().numBuffers;
	if (numBuff == 1) {
		buffIdx = 0;
	}
	else {
		buffIdx++;
		buffIdx = (buffIdx >= (numBuff - 1) ? 0 : buffIdx);
	}
	getImpl().spectrumBufferIndex = buffIdx;

	// Store entire spectrum data of bscan. 
	memcpy((void*)getImpl().spectrumData, (void*)data, sizeof(unsigned short)*width*height);

	updateWindow();
	return;
}


void ViewScanSpectrum::callbackResampleData(float * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	// Copy resample data at center line. 
	int buffIdx = getImpl().resampleBufferIndex;
	float* buffer = getImpl().resamples[buffIdx].data();
	int dataIdx = (height / 2) * width;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(float) * DATA_WIDTH);

	int numBuff = getImpl().numBuffers;
	if (numBuff == 1) {
		buffIdx = 0;
	}
	else {
		buffIdx++;
		buffIdx = (buffIdx >= (numBuff - 1) ? 0 : buffIdx);
	}
	getImpl().resampleBufferIndex = buffIdx;

	// Store entire resample data of bscan.
	memcpy((void*)getImpl().resampleData, (void*)data, sizeof(float)*width*height);

	updateWindow();
	return;
}


BEGIN_MESSAGE_MAP(ViewScanSpectrum, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// ViewScanSpectrum message handlers

void ViewScanSpectrum::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
	updateWindow();
}


void ViewScanSpectrum::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (getImpl().plotRect.PtInRect(point)) {
		int xpos = point.x - getImpl().plotRect.left;
		int didx = (int)(xpos * getImpl().plotRatioX);

		if (getImpl().markerCount == 0 || getImpl().markerCount >= 2) {
			getImpl().markerIndex[0] = didx;
			getImpl().markerCount = 1;
		}
		else {
			if (didx > getImpl().markerIndex[0]) {
				getImpl().markerIndex[1] = didx;
				getImpl().markerCount = 2;
			}
		}
	}
	CStatic::OnLButtonDown(nFlags, point);
}


void ViewScanSpectrum::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	getImpl().markerCount = 0;

	CStatic::OnRButtonDown(nFlags, point);
}
