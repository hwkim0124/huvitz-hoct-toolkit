#include "stdafx.h"
#include "AngioSetup.h"

#include "CppUtil2.h"
#include "SigProc2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace SigChain;
using namespace SigProc;
using namespace CppUtil;
using namespace std;

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <boost/format.hpp>
#include <numeric>

struct AngioSetup::AngioSetupImpl
{
	vector<float> imageRealBuffer;
	vector<float> imageImagBuffer;
	vector<float> imageMagnBuffer;
	vector<vector<CvImage>> amplitudes;
	vector<CvImage> previews;
	vector<float> decorrelations;

	int previewIdx;

	bool saveBuffersToFile = false;
	bool saveFramesToFile = false;
	bool previewDecorrs = false;
	bool ediScanning = false;

	int previewWidth;
	int imageWidth;
	int imageHeight;
	int linePaddSize;
	int repeats;
	int lines;
	int points;
	bool vertical;

	int dataHeight = 768;
	int dataRowStart = 24;
	int dataRowEnd = (dataRowStart + dataHeight);

	std::string scanDateTime = "";

	AngioSetupImpl() : imageWidth(0), imageHeight(0), linePaddSize(0), repeats(2), saveBuffersToFile(false), vertical(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<AngioSetup::AngioSetupImpl> AngioSetup::d_ptr(new AngioSetupImpl());


AngioSetup::AngioSetup()
{
}


AngioSetup::~AngioSetup()
{
}


void SigChain::AngioSetup::setAngiogramLayout(int lines, int points, int repeats, bool vertical)
{	
	getImpl().lines = lines;
	getImpl().points = points;
	getImpl().repeats = repeats;
	getImpl().vertical = vertical;

	getImpl().amplitudes = vector<vector<CvImage>>(lines);
	for (int i = 0; i < lines; i++) {
		getImpl().amplitudes[i] = vector<CvImage>(repeats);
	}

	getImpl().previews = vector<CvImage>(repeats);
	getImpl().previewIdx = 0;

	getImpl().imageWidth = points;
	getImpl().previewWidth = 1024;

	auto ttime = time(0);
	tm local;
	localtime_s(&local, &ttime);
	long date = (1900 + local.tm_year) * 10000 + (1 + local.tm_mon) * 100 + local.tm_mday;
	long time = local.tm_hour * 10000 + (local.tm_min) * 100 + local.tm_sec;
	getImpl().scanDateTime = to_string(date) + to_string(time);

	// setSavingBuffersToFiles(true);
	return;
}


void SigChain::AngioSetup::setImageRepeats(int repeats)
{
	getImpl().repeats = repeats;
	return;
}


void SigChain::AngioSetup::resetImageBuffers(int width, int height, int linePadds)
{
	int imageSize = (width + linePadds) * height;
	int buffSize = getImageBufferSize();
	if (imageSize != buffSize) {
		getImpl().imageWidth = width;
		getImpl().imageHeight = height;
		getImpl().linePaddSize = linePadds;

		getImpl().imageImagBuffer = vector<float>(imageSize, 0.0f);
		getImpl().imageRealBuffer = vector<float>(imageSize, 0.0f);
	}
	return;
}


float * SigChain::AngioSetup::getImageRealBuffer(void)
{
	return &getImpl().imageRealBuffer[0];
}


float * SigChain::AngioSetup::getImageImagBuffer(void)
{
	return &getImpl().imageImagBuffer[0];
}


void SigChain::AngioSetup::copyImageRealsToBuffer(float * data, int width, int height, int linePadds, bool vflip)
{
	auto bptr = getImageRealBuffer();
	if (bptr != nullptr) {
		if (vflip) {
			for (int i = 0; i < height; i++) {
				auto src = (data + i * (width + linePadds));
				auto dst = (bptr + (height - i - 1) * width);
				memcpy(dst, src, sizeof(float)*width);
			}
		}
		else {
			for (int i = 0; i < height; i++) {
				auto src = (data + i * (width + linePadds));
				auto dst = (bptr + i * width);
				memcpy(dst, src, sizeof(float)*width);
			}
		}
	}
	return;
}


void SigChain::AngioSetup::copyImageImagsToBuffer(float * data, int width, int height, int linePadds, bool vflip)
{
	auto bptr = getImageImagBuffer();
	if (bptr != nullptr) {
		if (vflip) {
			for (int i = 0; i < height; i++) {
				auto src = (data + i * (width + linePadds));
				auto dst = (bptr + (height - i - 1) * width);
				memcpy(dst, src, sizeof(float)*width);
			}
		}
		else {
			for (int i = 0; i < height; i++) {
				auto src = (data + i * (width + linePadds));
				auto dst = (bptr + i * width);
				memcpy(dst, src, sizeof(float)*width);
			}
		}
	}
	return;
}


int SigChain::AngioSetup::getBufferWidth(void)
{
	return (getImpl().imageWidth + getImpl().linePaddSize);
}


int SigChain::AngioSetup::getBufferHeight(void)
{
	return getImpl().imageHeight;
}


int SigChain::AngioSetup::getImageBufferSize(void)
{
	int size = (getImpl().imageWidth + getImpl().linePaddSize) * getImpl().imageHeight;
	return size;
}


void SigChain::AngioSetup::dumpImageBuffersToFile(int index)
{
	int buffSize = getImageBufferSize();
	if (buffSize <= 0) {
		return;
	}

	wstring exportPath = _T("D://hoct_angio");
	if (CreateDirectory(exportPath.c_str(), NULL) || 
		ERROR_ALREADY_EXISTS == GetLastError()) {
		wstring bufferPath = _T("D://hoct_angio//buffer");
		if (CreateDirectory(bufferPath.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError()) {
			wstring indexPath = _T("D://hoct_angio//buffer//");
			indexPath += atow(getImpl().scanDateTime);
			if (CreateDirectory(indexPath.c_str(), NULL) ||
				ERROR_ALREADY_EXISTS == GetLastError()) {
				exportPath = indexPath;
			}
		}
	}

	int repeats = getImpl().repeats;
	int major = getIndexOfLineFromBufferIndex(index);
	int minor = getIndexOfRepeatFromBufferIndex(index);

	/*
	if ((major % 2) != 0) {
		auto p1 = getImageRealBuffer();
		auto p2 = getImageImagBuffer();
		float temp;
		int w = getBufferWidth();
		int h = getBufferHeight();

		for (int y = 0; y < h; y++) {
			int ridx = y * w;
			for (int x1 = 0, x2 = (w-1); x1 < (w / 2); x1++, x2--) {
				temp = *(p1 + ridx + x1);
				*(p1 + ridx + x1) = *(p1 + ridx + x2);
				*(p1 + ridx + x2) = temp;
				temp = *(p2 + ridx + x1);
				*(p2 + ridx + x1) = *(p2 + ridx + x2);
				*(p2 + ridx + x2) = temp;
			}
		}
	}
	*/
	
	std::string path1 = (boost::format("%s//%03d_%02d_r.bin") % wtoa(exportPath).c_str() % major % minor).str();
	std::ofstream file(path1, std::ios::out | std::ofstream::binary);
	file.write((char*)getImageRealBuffer(), sizeof(float)*buffSize);
	file.close();

	std::string path2 = (boost::format("%s//%03d_%02d_i.bin") % wtoa(exportPath).c_str() % major % minor).str();
	std::ofstream file2(path2, std::ios::out | std::ofstream::binary);
	file2.write((char*)getImageImagBuffer(), sizeof(float)*buffSize);
	file2.close();

	LogD() << "Angio image dumped, index: " << index << ", repeats: " << repeats << ", w: " << getImpl().imageWidth << ", h: " << getImpl().imageHeight << ", padd: " << getImpl().linePaddSize;
	LogD() << "Real file path: " << path1;
	LogD() << "Imag file path: " << path2;
	return;
}


void SigChain::AngioSetup::saveAmplitudesAsBinaryFiles(const std::string dirPath)
{
	auto& ampls = getImpl().amplitudes;
	if (ampls.empty()) {
		LogD() << "Amplitude data is empty to save files";
		return;
	}

	int count = 0;
	for (int i = 0; i < ampls.size(); i++) {
		for (int j = 0; j < ampls[i].size(); j++) {
			if (ampls[i][j].isEmpty()) {
				continue;
			}

			std::string path;
			if (ampls[i].size() > 1) {
				path = (boost::format("%s//%03d_%02d.dat") % dirPath % i % j).str();
			}
			else {
				path = (boost::format("%s//%03d.dat") % dirPath % i).str();
			}

			try {
				int dsize = ampls[i][j].getWidth() * ampls[i][j].getHeight();
				float* bits = (float*)ampls[i][j].getBitsData();

				if (dsize > 0 && bits != nullptr) {
					std::ofstream file(path, std::ios::out | std::ofstream::binary);
					vector<unsigned short> sdat(dsize);
					for (int k = 0; k < dsize; k++) {
						sdat[k] = (unsigned short)bits[k];
					}
					file.write((const char*)(&sdat[0]), sizeof(unsigned short)*dsize);
					file.close();
					count += 1;
				}
				else {
					throw L"Exception";
				}
			}
			catch (...) {
				LogD() << "Failed to write amplitude data, path: " << path;
			}
		}
	}

	LogD() << "Amplitude data files saved, count: " << count;
	return;
}



bool SigChain::AngioSetup::storeAmplitudesToPatternBuffer(int index, float* buff)
{
	/*
	int major = getIndexOfLineFromBufferIndex(index);
	int minor = getIndexOfRepeatFromBufferIndex(index);

	bool res = getAngiogram().loadAmplitudeFromBuffer(major, minor, buff);
	LogD() << "Angio image index: " << index << " (" << major << ", " << minor << "), data buffer loaded: " << res;
	*/

	if (index < 0 || index >= numberOfAngioImages()) {
		return false;
	}

	int major = getIndexOfLineFromBufferIndex(index);
	int minor = getIndexOfRepeatFromBufferIndex(index);
	int width = getDataWidth();
	int height = getDataHeight();

	getImpl().amplitudes[major][minor].fromFloat32((const unsigned char*)&buff[0], width, height);
	// getImpl().amplitudes[index] = vector<float>(buff, buff+getImageBufferSize());
	return true;
}


bool SigChain::AngioSetup::storeAmplitudesToPreviewBuffer(float * buff)
{
	if (!getImpl().previewDecorrs) {
		return true;
	}

	int index = getImpl().previewIdx % getImpl().repeats;
	getImpl().previewIdx += 1;

	int width = getImpl().previewWidth; //  getDataWidth();
	int height = getDataHeight();
	getImpl().previews[index].fromFloat32((const unsigned char*)&buff[0], width, height);

	if (!(getImpl().previewIdx % getImpl().repeats)) {
		updateDecorrelationsOfPreview();
		getImpl().previewIdx = 0;
	}
	return true;
}


bool SigChain::AngioSetup::updateDecorrelationsOfPreview(void)
{
	if (getImpl().previews.size() < getImpl().repeats) {
		return false;
	}

	auto ampl = getImpl().previews[0];
	float mean, stdev;
	ampl.getMeanStddev(&mean, &stdev);

	float thresh1 = mean + stdev * 1.0f;
	float thresh2 = mean * 10.0f;

	LogD() << "Angio preview, mean: " << mean << ", stdev: " << stdev << ", thresh: " << thresh1 << ", " << thresh2;

	int size = ampl.getWidth() * ampl.getHeight();
	vector<float> decor = vector<float>(size, 0.0f);

	for (int n = 0; n < (getImpl().repeats - 1); n++) {
		auto amp1 = (float*)getImpl().previews[n].getBitsData();
		auto amp2 = (float*)getImpl().previews[n + 1].getBitsData();

		for (int k = 0; k < size; k++) {
			auto a1 = amp1[k];
			auto a2 = amp2[k];

			if (a1 > thresh1 && a2 > thresh1) {
				a1 = min(a1, thresh2);
				a2 = min(a2, thresh2);

				decor[k] += (std::pow((a1 - a2), 2.0f) / (std::pow(a1, 2.0f) + std::pow(a2, 2.0f)));
			}
		}
	}

	std::for_each(std::begin(decor), std::end(decor), [&](float& elem) {
		elem = elem / (getImpl().repeats - 1);
	});

	getImpl().decorrelations = decor;
	return true;
}


float * SigChain::AngioSetup::getDecorrelation(void)
{
	auto size = getImpl().decorrelations.size();
	if (size == 0 || size != getPreviewSize()) {
		return nullptr;
	}
	return &(getImpl().decorrelations[0]);
}


int SigChain::AngioSetup::numberOfAngioImages(void)
{
	return numberOfAngioLines() * numberOfAngioRepeats();
}


int SigChain::AngioSetup::numberOfAngioRepeats(void)
{
	return getImpl().repeats;
}


int SigChain::AngioSetup::numberOfAngioLines(void)
{
	return getImpl().lines;
}


int SigChain::AngioSetup::numberOfAngioPoints(void)
{
	return getImpl().points;
}


int SigChain::AngioSetup::getIndexOfLineFromBufferIndex(int index)
{
	int repeats = getImpl().repeats;
	int major = index / repeats;
	return major;
}


int SigChain::AngioSetup::getIndexOfRepeatFromBufferIndex(int index)
{
	int repeats = getImpl().repeats;
	int minor = index % repeats;
	return minor;
}


bool SigChain::AngioSetup::isVerticalScan(void)
{
	return getImpl().vertical;
}


std::vector<vector<CvImage>>& SigChain::AngioSetup::getAmplitudes(void)
{
	return getImpl().amplitudes;
}


int SigChain::AngioSetup::getDataSize(void)
{
	return (getDataWidth() * getDataHeight());
}


int SigChain::AngioSetup::getDataWidth(void)
{
	return getImpl().imageWidth;
}


int SigChain::AngioSetup::getDataHeight(void)
{
	return getImpl().dataHeight;
}

int SigChain::AngioSetup::getPreviewWidth(void)
{
	return getImpl().previewWidth;
}


int SigChain::AngioSetup::getPreviewSize(void)
{
	return getPreviewWidth() * getDataHeight();
}


bool SigChain::AngioSetup::makeAmplitudesOfLineRepeats(int idxLine)
{
	/*
	int sidx = idxLine * getImpl().repeats;
	int eidx = (idxLine + 1) * getImpl().repeats;

	LogD() << "Amplitudes of Lines: " << idxLine;

	getImpl().amplitudes.clear();

	for (int i = sidx; i < eidx; i++) {
		if (!readFileToImageBuffers(i)) {
			return false;
		}

		auto reals = getImageRealBuffer();
		auto imags = getImageImagBuffer();
		vector<float> data = vector<float>(getDataSize(), 0.0f);
		
		int width = getBufferWidth();
		int height = getBufferHeight();

		int cnt = 0;
		for (int r = getImpl().dataRowStart; r < getImpl().dataRowEnd; r++) {
			for (int c = 0; c < width; c++) {
				int k = r * width + c; 
				data[cnt++] = sqrt(pow(reals[k], 2.0f) + pow(imags[k], 2.0f));
			}
		}

		getImpl().amplitudes.push_back(data);
	}

	makeDecorrelationOfAmplitudes();
	*/
	return true;
}


bool SigChain::AngioSetup::makeDecorrelationOfAmplitudes(void)
{
	/*
	if (getImpl().amplitudes.size() < getImpl().repeats) {
		return false;
	}

	auto ampl = getImpl().amplitudes[0];
	auto size = ampl.size();
	auto sum = accumulate(std::begin(ampl), std::end(ampl), 0.0f);
	auto mean = sum / ampl.size();

	float accum = 0.0f;
	std::for_each(std::begin(ampl), std::end(ampl), [&](const float d) {
		accum += (d - mean) * (d - mean);
	});
	float stdev = sqrt(accum / (ampl.size() - 1));
	float thresh = mean + stdev * 2.0f;

	LogD() << "Decor. thresh: " << thresh << ", mean: " << mean << ", stdev: " << stdev;

	vector<float> decor = vector<float>(size, 0.0f);
	
	for (int n = 0; n < (getImpl().repeats - 1); n++) {
		auto amp1 = getImpl().amplitudes[n];
		auto amp2 = getImpl().amplitudes[n + 1];

		int cnt = 0;
		for (int k = 0; k < size; k++) {
			auto d1 = amp1[k];
			auto d2 = amp2[k];

			if (d1 > thresh && d2 > thresh) {
				float c = ((d1 * d2) / (0.5f * pow(d1, 2.0f) + 0.5f * pow(d2, 2.0f)));
				decor[cnt] += (1.0f - c);
				// LogD() << "n: " << n << ", k: " << k << " c: " << c;
			}
			cnt++;
		}
	}

	std::for_each(std::begin(decor), std::end(decor), [&](float& elem) {
		elem = elem / (getImpl().repeats - 1);
	});

	getImpl().decorrelations = decor;
	*/
	return true;
}


const int SigChain::AngioSetup::getNumberOfRepeatedData(void)
{
	return (int) getImpl().amplitudes.size();
}

/*
const vector<float> SigChain::AngioSetup::getAmplitudes(int frameIdx)
{
	if (frameIdx >= 0 && frameIdx < getNumberOfRepeatedData()) {
		return getImpl().amplitudes[frameIdx];
	}
	return vector<float>();
}
*/


const vector<float> SigChain::AngioSetup::getDecorrelations(void)
{
	return getImpl().decorrelations;
}


bool SigChain::AngioSetup::isSavingBuffersToFiles(void)
{
	return getImpl().saveBuffersToFile;
}

bool SigChain::AngioSetup::isSavingFramesToFiles(void)
{
	return getImpl().saveFramesToFile;
}

bool SigChain::AngioSetup::isEdiScanningMode(void)
{
	return getImpl().ediScanning;
}

void SigChain::AngioSetup::setEdiScanningMode(bool flag)
{
	getImpl().ediScanning = flag;
	return;
}

void SigChain::AngioSetup::setSavingFramesToFiles(bool flag)
{
	getImpl().saveFramesToFile = flag;
	return;
}

void SigChain::AngioSetup::setSavingBuffersToFiles(bool flag)
{
	getImpl().saveBuffersToFile = flag;
	return;
}

void SigChain::AngioSetup::setPreviewDecorrelations(bool flag)
{
	getImpl().previewDecorrs = flag;
	return;
}


const float SigChain::AngioSetup::getDecorrMean(int row1, int row2, int axialIdx)
{
	auto decors = getImpl().decorrelations;
	if (decors.size() <= 0 || axialIdx >= getDataWidth()) {
		return 0.0f;
	}

	row1 = min(max(row1, 0), getDataHeight()-1);
	row2 = max(max(row2, 0), getDataHeight()-1);

	float dsum = 0.0f;
	float dcnt = 0.0f;
	int dw = getDataWidth();

	if (axialIdx >= 0) {
		for (int y = row1; y <= row2; y++) {
			dsum += decors[y*dw + axialIdx];
			dcnt += 1.0f;
		}
	}
	else {
		for (int y = row1; y <= row2; y++) {
			for (int x = 0; x < dw; x++) {
				dsum += decors[y*dw + x];
				dcnt += 1.0f;
			}
		}
	}
	
	float dmean = (dcnt > 0.0f ? (dsum / dcnt) : 0.0f);
	return dmean;
}


const float SigChain::AngioSetup::getDecorrStdev(int row1, int row2, int axialIdx)
{
	auto decors = getImpl().decorrelations;
	if (decors.size() <= 0 || axialIdx >= getDataWidth()) {
		return 0.0f;
	}

	row1 = min(max(row1, 0), getDataHeight() - 1);
	row2 = max(max(row2, 0), getDataHeight() - 1);

	float dmean = getDecorrMean(row1, row2);

	float dsum = 0.0f;
	float dcnt = 0.0f;
	int dw = getDataWidth();

	if (axialIdx >= 0) {
		for (int y = row1; y <= row2; y++) {
			dsum += pow((decors[y*dw + axialIdx] - dmean), 2.0f);
			dcnt += 1.0f;
		}
	}
	else {
		for (int y = row1; y <= row2; y++) {
			for (int x = 0; x < dw; x++) {
				dsum += pow((decors[y*dw + x] - dmean), 2.0f);
				dcnt += 1.0f;
			}
		}
	}

	float dstd = (dcnt > 0.0f ? sqrt(dsum / dcnt) : 0.0f);
	return dstd;
}


const float SigChain::AngioSetup::getDecorrMax(int row1, int row2, int axialIdx)
{
	auto decors = getImpl().decorrelations;
	if (decors.size() <= 0 || axialIdx >= getDataWidth()) {
		return 0.0f;
	}

	row1 = min(max(row1, 0), getDataHeight() - 1);
	row2 = max(max(row2, 0), getDataHeight() - 1);

	float dmax = 0.0f;
	int dw = getDataWidth();

	if (axialIdx >= 0) {
		for (int y = row1; y <= row2; y++) {
			dmax = max(decors[y*dw + axialIdx], dmax);
		}
	}
	else {
		for (int y = row1; y <= row2; y++) {
			for (int x = 0; x < dw; x++) {
				dmax = max(decors[y*dw + x], dmax);
			}
		}
	}

	return dmax;
}


AngioSetup::AngioSetupImpl & SigChain::AngioSetup::getImpl(void)
{
	return *d_ptr;
}

