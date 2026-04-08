// SpectrumView.cpp : implementation file
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "SpectrumView.h"

#include <vector>
#include <array>

#include "CppUtil2.h"

using namespace std;
using namespace CppUtil;


struct SpectrumView::SpectrumViewImpl
{
	int width;
	int height;

	int numBuffers;
	int idxSpectrum;
	int idxResample;

	float plotRatioX;
	float plotRatioY;
	CRect plotRect;

	float resample_data[LINE_CAMERA_CCD_PIXELS * 1024];

	vector<array<unsigned short, LINE_CAMERA_CCD_PIXELS>> spectrums;
	vector<array<float, LINE_CAMERA_CCD_PIXELS>> resamples;

	SpectrumDataCallback cbSpectrumData;
	ResampleDataCallback cbResampleData;

	SpectrumViewImpl() : numBuffers(1), idxSpectrum(0), idxResample(0)
	{

	}
};


// SpectrumView

IMPLEMENT_DYNAMIC(SpectrumView, CStatic)

SpectrumView::SpectrumView()
	: d_ptr(make_unique<SpectrumViewImpl>())
{
	setAverageSize(1);

	getImpl().cbSpectrumData = std::bind(&SpectrumView::callbackSpectrumData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	getImpl().cbResampleData = std::bind(&SpectrumView::callbackResampleData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


SpectrumView::~SpectrumView()
{
}


void SpectrumView::updateWindow(void)
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


void SpectrumView::drawSpectrumGraph(CDC * pDC)
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
	getImpl().plotRatioX = (float)LINE_CAMERA_CCD_PIXELS / (float)rcPlot.Width();
	getImpl().plotRatioY = (float)LINE_CAMERA_PIXEL_VALUE_MAX / (float)rcPlot.Height();

	pDC->SelectObject(old);
	pen.DeleteObject();
	return;
}


void SpectrumView::drawSpectrumDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	unsigned short* data = getSpectrumData();
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


void SpectrumView::drawResampleDataLine(CDC * pDC)
{
	int width = getWidth();
	int height = getHeight();

	int orgX = getImpl().plotRect.left;
	int orgY = getImpl().plotRect.bottom;
	int wPlot = getImpl().plotRect.Width();
	int hPlot = getImpl().plotRect.Height();

	float* data = getResampleData();
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


void SpectrumView::setDisplaySize(int width, int height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


int SpectrumView::getWidth(void) const
{
	return getImpl().width;
}


int SpectrumView::getHeight(void) const
{
	return getImpl().height;
}


void SpectrumView::setAverageSize(int size)
{
	getImpl().numBuffers = (size <= 1 ? 1 : (size + 1));

	getImpl().spectrums.clear();
	getImpl().resamples.clear();
	for (int k = 0; k < getImpl().numBuffers; k++) {
		getImpl().spectrums.push_back(array<unsigned short, LINE_CAMERA_CCD_PIXELS>{0});
		getImpl().resamples.push_back(array<float, LINE_CAMERA_CCD_PIXELS>{0});
	}
	return;
}


unsigned short * SpectrumView::getSpectrumData(void)
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
			for (int j = 0; j < LINE_CAMERA_CCD_PIXELS; j++) {
				data[j] += buff[j];
			}
		}

		for (int j = 0; j < LINE_CAMERA_CCD_PIXELS; j++) {
			data[j] /= numData;
		}
		return data;
	}
}


float * SpectrumView::getResampleData(void)
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
			for (int j = 0; j < LINE_CAMERA_CCD_PIXELS; j++) {
				data[j] += buff[j];
			}
		}

		for (int j = 0; j < LINE_CAMERA_CCD_PIXELS; j++) {
			data[j] /= numData;
		}
		return data;
	}
}


float * SpectrumView::getResampleData2(void)
{
	return getImpl().resample_data;
}


SpectrumDataCallback * SpectrumView::getSpectrumCallback(void) const
{
	return &getImpl().cbSpectrumData;
}


ResampleDataCallback * SpectrumView::getResampleCallback(void) const
{
	return &getImpl().cbResampleData;
}


BEGIN_MESSAGE_MAP(SpectrumView, CStatic)
END_MESSAGE_MAP()



// SpectrumView message handlers


SpectrumView::SpectrumViewImpl & SpectrumView::getImpl(void) const
{
	return *d_ptr;
}


void SpectrumView::callbackSpectrumData(unsigned short * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	int buffIdx = getImpl().idxSpectrum;
	unsigned short* buffer = getImpl().spectrums[buffIdx].data();
	int dataIdx = (height / 2) * width;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(unsigned short) * LINE_CAMERA_CCD_PIXELS);

	int numBuff = getImpl().numBuffers;
	if (numBuff == 1) {
		buffIdx = 0;
	}
	else {
		buffIdx++;
		buffIdx = (buffIdx >= (numBuff - 1) ? 0 : buffIdx);
	}
	getImpl().idxSpectrum = buffIdx;

	updateWindow();
	return;
}


void SpectrumView::callbackResampleData(float * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	int buffIdx = getImpl().idxResample;
	float* buffer = getImpl().resamples[buffIdx].data();
	int dataIdx = (height / 2) * width;
	memcpy((void*)buffer, (void*)(data + dataIdx), sizeof(float) * LINE_CAMERA_CCD_PIXELS);

	int numBuff = getImpl().numBuffers;
	if (numBuff == 1) {
		buffIdx = 0;
	}
	else {
		buffIdx++;
		buffIdx = (buffIdx >= (numBuff - 1) ? 0 : buffIdx);
	}
	getImpl().idxResample = buffIdx;

	memcpy((void*)getImpl().resample_data, (void*)data, sizeof(float)*width*height);

	updateWindow();
	return;
}


void SpectrumView::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	// TODO:  Add your code to draw the specified item
}
