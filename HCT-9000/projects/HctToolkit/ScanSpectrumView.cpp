// ScanSpectrumView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "ScanSpectrumView.h"

#include <vector>
#include <array>
#include <iostream>
#include <fstream>

#include "CppUtil2.h"

using namespace std;
using namespace CppUtil;


struct ScanSpectrumView::ScanSpectrumViewImpl
{
	int width;
	int height;

	int numBuffers;
	int idxSpectrum;
	int idxResample;

	float plotRatioX;
	float plotRatioY;
	CRect plotRect;

	float resampleData[BUFFER_SIZE];
	unsigned short spectrumData[BUFFER_SIZE];

	vector<array<unsigned short, DATA_WIDTH>> spectrums;
	vector<array<float, DATA_WIDTH>> resamples;

	SpectrumDataCallback cbSpectrumData;
	ResampleDataCallback cbResampleData;

	ScanSpectrumViewImpl() : numBuffers(1), idxSpectrum(0), idxResample(0)
	{

	}
};


// ScanSpectrumView

IMPLEMENT_DYNAMIC(ScanSpectrumView, CStatic)

ScanSpectrumView::ScanSpectrumView()
	: d_ptr(make_unique<ScanSpectrumViewImpl>())
{
	setAverageSize(1);

	getImpl().cbSpectrumData = std::bind(&ScanSpectrumView::callbackSpectrumData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	getImpl().cbResampleData = std::bind(&ScanSpectrumView::callbackResampleData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	return;
}


ScanSpectrumView::~ScanSpectrumView()
{
}


void ScanSpectrumView::updateWindow(void)
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

	BitBlt(hDC, 0, 0, getWidth(), getHeight(), hMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(oldBmp);

	bitmap.DeleteObject();
	DeleteDC(hMemDC);
	ReleaseDC(pDC);
	return;
}


void ScanSpectrumView::drawSpectrumGraph(CDC * pDC)
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


void ScanSpectrumView::drawSpectrumDataLine(CDC * pDC)
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


void ScanSpectrumView::drawResampleDataLine(CDC * pDC)
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


void ScanSpectrumView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void ScanSpectrumView::setAverageSize(int size)
{
	getImpl().numBuffers = (size <= 1 ? 1 : (size + 1));

	getImpl().spectrums.clear();
	getImpl().resamples.clear();
	for (int k = 0; k < getImpl().numBuffers; k++) {
		getImpl().spectrums.push_back(array<unsigned short, DATA_WIDTH>{0});
		getImpl().resamples.push_back(array<float, DATA_WIDTH>{0});
	}

	getImpl().idxSpectrum = 0;
	getImpl().idxResample = 0;
	return;
}


int ScanSpectrumView::getAverageSize(void)
{
	return getImpl().numBuffers;
}


int ScanSpectrumView::getWidth(void) const
{
	return getImpl().width;
}


int ScanSpectrumView::getHeight(void) const
{
	return getImpl().height;
}


unsigned short * ScanSpectrumView::getSpectrumLine(void)
{
	size_t size = getImpl().spectrums.size();
	if (size <= 0) {
		return nullptr;
	}

	if (size == 1) {
		return getImpl().spectrums[0].data();
	}
	else {
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


unsigned short * ScanSpectrumView::getSpectrumData(void)
{
	return getImpl().spectrumData;
}


float * ScanSpectrumView::getResampleLine(void)
{
	size_t size = getImpl().resamples.size();
	if (size <= 0) {
		return nullptr;
	}

	if (size == 1) {
		return getImpl().resamples[0].data();
	}
	else {
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


float * ScanSpectrumView::getResampleData(void)
{
	return getImpl().resampleData;
}


int ScanSpectrumView::getDataWidth(void)
{
	return DATA_WIDTH;
}


int ScanSpectrumView::getDataHeight(void)
{
	return DATA_HEIGHT;
}


void ScanSpectrumView::initSpectrumLine(unsigned short * data, int index)
{
	if (getAverageSize() != 1) {
		setAverageSize(1);
	}

	unsigned short* buffer = getImpl().spectrums[0].data();
	int dataIdx = DATA_WIDTH * index;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(unsigned short) * DATA_WIDTH);
	return;
}


void ScanSpectrumView::initResampleLine(float * data, int index)
{
	if (getAverageSize() != 1) {
		setAverageSize(1);
	}

	float* buffer = getImpl().resamples[0].data();
	int dataIdx = DATA_WIDTH * index;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(float) * DATA_WIDTH);
	return;
}


bool ScanSpectrumView::saveSpectrumData(const std::wstring & path)
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


bool ScanSpectrumView::loadSpectrumData(const std::wstring & path)
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


bool ScanSpectrumView::saveResampleData(const std::wstring & path)
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


bool ScanSpectrumView::loadResampleData(const std::wstring & path)
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


SpectrumDataCallback * ScanSpectrumView::getSpectrumCallback(void) const
{
	return &getImpl().cbSpectrumData;
}


ResampleDataCallback * ScanSpectrumView::getResampleCallback(void) const
{
	return &getImpl().cbResampleData;
}


ScanSpectrumView::ScanSpectrumViewImpl & ScanSpectrumView::getImpl(void) const
{
	return *d_ptr;
}


void ScanSpectrumView::callbackSpectrumData(unsigned short * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	int buffIdx = getImpl().idxSpectrum;
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
	getImpl().idxSpectrum = buffIdx;

	memcpy((void*)getImpl().spectrumData, (void*)data, sizeof(unsigned short)*width*height);

	updateWindow();
	return;
}


void ScanSpectrumView::callbackResampleData(float * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	int buffIdx = getImpl().idxResample;
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
	getImpl().idxResample = buffIdx;

	memcpy((void*)getImpl().resampleData, (void*)data, sizeof(float)*width*height);

	updateWindow();
	return;
}


BEGIN_MESSAGE_MAP(ScanSpectrumView, CStatic)
END_MESSAGE_MAP()



// ScanSpectrumView message handlers

void ScanSpectrumView::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{

	// TODO:  Add your code to draw the specified item
}
