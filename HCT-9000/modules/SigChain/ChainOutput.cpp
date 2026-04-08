#include "stdafx.h"
#include "ChainOutput.h"

#include <chrono>
#include <mutex>

#include "CppUtil2.h"

using namespace SigChain;
using namespace CppUtil;
using namespace std;


struct ChainOutput::ChainOutputImpl
{
	float previewQuality;
	float preivewSignalRatio;
	int previewReferPoint;
	unsigned long countOfPreviews;
	chrono::system_clock::time_point previewTimePoint;

	uint8_t enfaceBuffer[FRAME_IMAGE_BUFFER_SIZE];
	int enfaceWidth;
	int enfaceHeight;
	unsigned long countOfEnfaces;
	chrono::system_clock::time_point enfaceTimePoint;

	mutex mutexAccess;

	ChainOutputImpl() : enfaceBuffer{ 0 }, countOfPreviews(0), countOfEnfaces(0)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ChainOutput::ChainOutputImpl> ChainOutput::d_ptr(new ChainOutputImpl());


ChainOutput::ChainOutput()
{
	clearResultOfLastImage();
}


ChainOutput::~ChainOutput()
{
}


void SigChain::ChainOutput::clearResultOfLastImage(void)
{
	getImpl().previewQuality = 0.0f;
	getImpl().preivewSignalRatio = 0.0f;
	getImpl().previewReferPoint = 0;
	getImpl().countOfPreviews = 0;
	getImpl().previewTimePoint = chrono::system_clock::now();

	getImpl().enfaceWidth = 0;
	getImpl().enfaceHeight = 0;
	getImpl().countOfEnfaces = 0;
	getImpl().enfaceTimePoint = chrono::system_clock::now();
	return;
}


void SigChain::ChainOutput::setPreviewImageResult(float qidx, float sig_ratio, int refPoint)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	getImpl().previewQuality = qidx;
	getImpl().preivewSignalRatio = sig_ratio;
	getImpl().previewReferPoint = refPoint;
	getImpl().previewTimePoint = chrono::system_clock::now();
	getImpl().countOfPreviews++;
	return;
}


bool SigChain::ChainOutput::getPreviewImageResult(float* qidx, float* sig_ratio, int* refPoint, unsigned long* count)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	if (getImpl().countOfPreviews <= 0) {
		return false;
	}

	if (qidx != nullptr) {
		*qidx = getImpl().previewQuality;
	}
	if (sig_ratio != nullptr) {
		*sig_ratio = getImpl().preivewSignalRatio;
	}
	if (refPoint != nullptr) {
		*refPoint = getImpl().previewReferPoint;
	}
	if (count != nullptr) {
		*count = getImpl().countOfPreviews;
	}
	return true;
}


void SigChain::ChainOutput::makeAxialProjectionOfRetina(uint8_t * image, uint8_t * lateral, int width, int height)
{
	CvImage cvImg;
	cvImg.fromBitsData(image, width, height);
	cvImg.blur(21, 5);

	auto inns = vector<int>(width, 0);
	auto outs = vector<int>(width, height - 1);
	uint8_t *p = (uint8_t*)cvImg.getBitsData();

	for (int c = 0; c < width; c++) {
		int vmax = 0, imax = 0;
		for (int r = 0; r < height; r += 4) {
			int idx = r * width + c;
			if (p[idx] >= vmax) {
				vmax = p[idx];
				imax = r;
			}
		}
		inns[c] = max(imax - 100, 0);
		outs[c] = min(imax + 100, height - 1);
	}

	for (int c = 0; c < width; c++) {
		int sum = 0, cnt = 0;
		for (int r = inns[c]; r <= outs[c]; r++) {
			int idx = r * width + c;
			sum += image[idx];
			cnt += 1;
		}
		lateral[c] = sum / cnt;
	}

	/*
	CvImage cvImg;
	cvImg.fromBitsData(image, width, height);
	cvImg.applyGradientConvolution(21, 7, true);

	float mean, stdev;
	cvImg.getMeanStddev(&mean, &stdev);

	int thresh = (int)(mean + stdev * 0.5f);
	uint8_t *p = (uint8_t*)cvImg.getBitsData();

	auto inns = vector<int>(width, 0);
	auto outs = vector<int>(width, height - 1);
	for (int c = 0; c < width; c++) {
		int vmax = 0, imax = 0;
		for (int r = 0; r < height; r += 4) {
			int idx = r * width + c;
			if (p[idx] >= vmax) {
				vmax = p[idx];
				imax = r;
			}
		}
		inns[c] = imax;
	}

	auto data = inns;
	for (int c = 0; c < width; c++) {
		int c1 = max(c - 2, 0);
		int c2 = min(c + 2, width - 1);
		int sum = 0, cnt = 0;
		for (int k = c1; k <= c2; k++) {
			sum += data[k];
			cnt += 1;
		}
		inns[c] = sum / cnt;
		outs[c] = min(inns[c] + 50, height - 1);
	}

	for (int c = 0; c < width; c++) {
		int sum = 0, cnt = 0;
		for (int r = inns[c]; r <= outs[c]; r++) {
			int idx = r * width + c;
			sum += image[idx];
			cnt += 1;
		}
		lateral[c] = sum / cnt;
	}
	*/
	return;
}


void SigChain::ChainOutput::setLateralLineOfEnface(uint8_t * buffer, int lineIdx, int lineSize, int numLines, bool vertical, bool reverse)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);

	if (!vertical) {
		uint8_t* dst = d_ptr->enfaceBuffer + lineIdx * lineSize;
		int buffSize = lineSize * sizeof(uint8_t);

		if (reverse) {
			for (int c1 = 0, c2 = (lineSize-1); c1 < lineSize; c1++, c2--) {
				*(dst + c1) = *(buffer + c2);
			}
		}
		else {
			memcpy(dst, buffer, buffSize);
		}
	}
	else {
		uint8_t* dst = d_ptr->enfaceBuffer + lineIdx;
		if (reverse) {
			for (int r1 = 0, r2 = (lineSize - 1); r1 < lineSize; r1++, r2--) {
				*(dst + r1 * numLines) = *(buffer + r2 * numLines);
			}
		}
		else {
			for (int r = 0; r < lineSize; r++) {
				*(dst + r * numLines) = *(buffer + r);
			}
		}
	}
	return;
}


void SigChain::ChainOutput::setEnfaceImageResult(int width, int height)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->enfaceWidth = width;
	d_ptr->enfaceHeight = height;
	d_ptr->enfaceTimePoint = chrono::system_clock::now();
	getImpl().countOfEnfaces++;
	return;
}


std::uint8_t * SigChain::ChainOutput::getEnfaceImageBuffer(void)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	return d_ptr->enfaceBuffer;
}


int SigChain::ChainOutput::getEnfaceImageWidth(void)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	return d_ptr->enfaceWidth;
}


int SigChain::ChainOutput::getEnfaceImageHeight(void)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	return d_ptr->enfaceHeight;
}


ChainOutput::ChainOutputImpl & SigChain::ChainOutput::getImpl(void)
{
	return *d_ptr;
}
