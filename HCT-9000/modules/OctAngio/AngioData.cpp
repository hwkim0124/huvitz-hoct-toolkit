#include "stdafx.h"
#include "AngioData.h"
#include "AngioLayout.h"
#include "CppUtil2.h"
#include "SigChain2.h"

#include <string>
#include <vector>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <numeric>

using namespace OctGlobal;
using namespace OctAngio;
using namespace CppUtil;
using namespace SigChain;
using namespace std;
using namespace cv;


struct AngioData::AngioDataImpl
{
	vector<vector<CvImage>> amplitudes;
	vector<vector<CvImage>> imags;
	vector<vector<CvImage>> reals;

	int dataWidth;
	int dataHeight;
	int dataSize;
	int dataRowEnd;
	int dataRowStart;
	int imageHeight;
	int imageWidth;
	int imageSize;

	bool axialAligned = false;

	AngioDataImpl() {
		imags = vector<vector<CvImage>>();
		reals = vector<vector<CvImage>>();
	};
};


AngioData::AngioData()
	: d_ptr(make_unique<AngioDataImpl>())
{
}


AngioData::~AngioData()
{
}


OctAngio::AngioData::AngioData(AngioData && rhs) = default;
AngioData & OctAngio::AngioData::operator=(AngioData && rhs) = default;


bool OctAngio::AngioData::fetchAmplitudesFromBuffer(int lines, int points, int repeats)
{
	if (lines != AngioSetup::numberOfAngioLines() ||
		points != AngioSetup::numberOfAngioPoints() ||
		repeats != AngioSetup::numberOfAngioRepeats()) {
		LogD() << "Angio amplitudes loading failed, not matched with buffer";
		return false;
	}

	getImpl().imageWidth = points;
	getImpl().imageHeight = 768;
	getImpl().imageSize = getImpl().imageWidth * getImpl().imageHeight;

	auto& data = AngioSetup::getAmplitudes();
	getImpl().amplitudes = std::move(data);

	if (!checkIfAmplitudesLoaded(lines, points, repeats)) {
		return false;
	}

	getImpl().axialAligned = false;
	return true;
}


bool OctAngio::AngioData::importAmplitudesFromImageFiles(int lines, int points, int repeats, const std::string dirPath)
{
	getImpl().imageWidth = points;
	getImpl().imageHeight = 768;
	getImpl().imageSize = getImpl().imageWidth * getImpl().imageHeight;

	getImpl().dataWidth = points;
	getImpl().dataHeight = 1025;
	getImpl().dataRowStart = 24;
	getImpl().dataRowEnd = (getImpl().dataRowStart + getImpl().imageHeight);
	getImpl().dataSize = getImpl().dataWidth * getImpl().dataHeight;

	LogD() << "Angio images data loading, lines: " << lines << ", points: " << points << ", repeats: " << repeats << ", dir_path: " << dirPath;

	getImpl().amplitudes = vector<vector<CvImage>>(lines);
	for (int i = 0; i < lines; i++) {
		getImpl().amplitudes[i] = vector<CvImage>(repeats);
	}

	/*
	// Complex data. 
	getImpl().reals = vector<vector<CvImage>>(lines);
	getImpl().imags = vector<vector<CvImage>>(lines);
	for (int i = 0; i < lines; i++) {
		getImpl().reals[i] = vector<CvImage>(repeats);
		getImpl().imags[i] = vector<CvImage>(repeats);
	}
	*/

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 0; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			int dataSize = getImpl().dataSize;
			workers.push_back(std::thread([tasks, k, dataSize, lines, repeats, dirPath, this]() {
				auto rvect = vector<float>(dataSize);
				auto ivect = vector<float>(dataSize);
				auto rbuff = (char*)&rvect[0];
				auto ibuff = (char*)&ivect[0];

				for (int i : tasks[k]) {
					for (int j = 0; j < repeats; j++) {
						if (!readImageDataFile(i, j, rbuff, ibuff, dataSize, dirPath)) {
							// return false;
						}
						else {
							auto reals = (float*)rbuff; 
							auto imags = (float*)ibuff; 
							makeAmplitudeFromImageData(i, j, reals, imags);
						}
					}
				}
			}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});

	if (!checkIfAmplitudesLoaded(lines, points, repeats)) {
		return false;
	}

	LogD() << "Angio amplitudes loaded, lines: " << lines << ", repeats: " << repeats;
	getImpl().axialAligned = false;
	return true;
}


bool OctAngio::AngioData::importAmplitudesFromDataFiles(int lines, int points, int repeats, const std::string dirPath, const std::string fileName)
{
	getImpl().imageWidth = points;
	getImpl().imageHeight = 768;
	getImpl().imageSize = getImpl().imageWidth * getImpl().imageHeight;

	LogD() << "Angio amplitudes data loading, lines: " << lines << ", points: " << points << ", repeats: " << repeats << ", dir_path: " << dirPath;

	std::string path;
	path = (boost::format("%s//%s") % dirPath % fileName).str();

	fstream fs;
	fs.open(path);
	if (fs.fail()) {
		LogD() << "Angio data file not found, path: " << path;
		return false;
	}

	getImpl().amplitudes = vector<vector<CvImage>>(lines);
	for (int i = 0; i < lines; i++) {
		getImpl().amplitudes[i] = vector<CvImage>(repeats);
	}

	try {
		int dataSize = getImpl().imageSize;
		int width = getImpl().imageWidth;
		int height = getImpl().imageHeight;
		int headSize = 256;
		int byteSize = dataSize * sizeof(unsigned short) * lines * repeats + headSize;

		auto svect = vector<unsigned short>(dataSize);
		auto fvect = vector<float>(dataSize);
		auto buff = (char*)&svect[0];

		std::ifstream file(path, std::ios::in | std::ifstream::binary);
		if (!file.good()) {
			return false;
		}
		
		file.seekg(0, ios::end);
		auto fsize = file.tellg();
		file.seekg(0, ios::beg);

		if (byteSize > fsize) {
			for (int i = 0; i < lines; i++) {
				for (int j = 0; j < repeats; j++) {
					file.read((char*)buff, sizeof(unsigned short)*dataSize);
					for (int k = 0; k < dataSize; k++) {
						fvect[k] = (float)svect[k];
					}
					getImpl().amplitudes[i][j].fromFloat32((const unsigned char*)&fvect[0], width, height);
				}
			}
		}
		else {
			file.read((char*)buff, headSize);

			for (int j = 0; j < repeats; j++) {
				for (int i = 0; i < lines; i++) {
					file.read((char*)buff, sizeof(unsigned short)*dataSize);
					for (int k = 0; k < dataSize; k++) {
						fvect[k] = (float)svect[k];
					}
					getImpl().amplitudes[i][j].fromFloat32((const unsigned char*)&fvect[0], width, height);
				}
			}
		}

		file.close();
	}
	catch (...) {
		LogD() << "Angio data file read failed, path: " << path;
		return false;
	}

	if (!checkIfAmplitudesLoaded(lines, points, repeats)) {
		return false;
	}

	LogD() << "Angio amplitudes loaded, lines: " << lines << ", repeats: " << repeats;
	getImpl().axialAligned = false;
	return true;
}


bool OctAngio::AngioData::exportAmplitudesToDataFiles(int lines, int repeats, const std::string dirPath, const std::string fileName)
{
	auto& amplitudes = getImpl().amplitudes;
	if (amplitudes.empty()) {
		return false;
	}

	LogD() << "Angio amplitudes data exporting, lines: " << lines << ", repeats: " << repeats << ", dirPath: " << dirPath;

	for (int i = 0; i < lines; i++) {
		if (amplitudes[i].size() < repeats) {
			LogE() << "Angio amplitudes empty, at line: " << i;
			return false;
		}
	}

	std::string path, path2;
	path = (boost::format("%s//%s") % dirPath % fileName).str();

	try {
		int dataSize = getImpl().imageSize;
		int width = getImpl().imageWidth;
		int height = getImpl().imageHeight;
		int headSize = 256;

		auto svect = vector<unsigned short>(dataSize);

		std::ofstream file(path, std::ios::out | std::ofstream::binary);
		file.write((char*)(&svect[0]), headSize);

		for (int j = 0; j < repeats; j++) {
			for (int i = 0; i < lines; i++) {
				int imgSize = amplitudes[i][j].getWidth() * amplitudes[i][j].getHeight();
				float *bits = (float*)amplitudes[i][j].getBitsData();

				if (dataSize == imgSize && bits != nullptr) {
					for (int k = 0; k < dataSize; k++) {
						svect[k] = (unsigned short)bits[k];
					}
					file.write((char*)(&svect[0]), sizeof(unsigned short)*dataSize);
				}
				else {
					LogE() << "Angio amplitudes empty, at line: " << i << ", frame: " << j;
					file.close();
					return false;
				}
			}
		}
		file.close();
	}
	catch (...) {
		LogD() << "Angio data file write failed, path: " << path;
		return false;
	}
	return true;
}


bool OctAngio::AngioData::checkIfAmplitudesLoaded(int lines, int points, int repeats)
{
	auto& amplitudes = getImpl().amplitudes;
	if (amplitudes.size() == 0 || lines == 0 || repeats == 0 || points == 0) {
		return false;
	}

	for (int i = 0; i < lines; i++) {
		if (amplitudes[i].size() < repeats) {
			LogD() << "Angio amplitudes empty, at line: " << i;
			return false;
		}
		for (int j = 0; j < repeats; j++) {
			if (amplitudes[i][j].isEmpty()) {
				LogD() << "Angio amplitudes empty, at line: " << i << ", repeat: " << j;
				return false;
			}
			if (amplitudes[i][j].getWidth() != points) {
				LogD() << "Angio amplitudes width invalid, at line: " << i << ", repeat: " << j << ", points: " << amplitudes[i][j].getWidth();
				return false;
			}
		}
	}
	return true;
}


bool OctAngio::AngioData::appendAmplitudesToCircular(int lines, int repeats)
{
	for (int i = 0; i < lines; i++) {
		if (getImpl().amplitudes[i].size() == repeats) {
			CvImage image;
			getImpl().amplitudes[i][0].copyTo(&image);
			getImpl().amplitudes[i].push_back(image);
		}
	}
	return true;
}


bool OctAngio::AngioData::alignRepeatedAmplitudes(int lines, int repeats, bool axial, bool lateral, const LayerArrays& uppers, const LayerArrays& lowers)
{
	CppUtil::ClockTimer::start();
	LogD() << "Angio align amplitudes, axial: " << axial << ", lateral: " << lateral;

	if (!axial || getImpl().axialAligned) {
		return true;
	}

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 0; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k, repeats, lateral, &uppers, &lowers, this]() {
				auto& amplitudes = getImpl().amplitudes;
				CvProcess cvProc;

				for (int i : tasks[k]) {
					for (int j = 1; j < amplitudes[i].size(); j++) {
						// The default ecc threshold between iterations should be smaller than 0.001.
						bool result = cvProc.registerAngioImages(i, j, amplitudes[i][0], amplitudes[i][j], 500, 1e-4);
					}
				}
				/*
				for (int i : tasks[k]) {
					registerAmplitudes(i, uppers, lowers);
				}
				*/
			}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angio amplitudes aligned, elapsed: " << msec;
	getImpl().axialAligned = true;
	return true;
}

bool OctAngio::AngioData::computeSequentialDisplacements(int lines, int repeats, std::vector<float>& trans_xlist, std::vector<float>& trans_ylist, std::vector<float>& ecc_list)
{
	CppUtil::ClockTimer::start();
	LogD() << "Computing sequential displacements, lines: " << lines << ", repeats: " << repeats;

	auto& amplitudes = getImpl().amplitudes;
	int success = 0;

	if (lines != getImpl().amplitudes.size()) {
		LogD() << "Sequential displacements computation failed, not matched with amplitudes buffer";
		return false;
	}

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 1; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	trans_xlist = std::vector<float>(lines);
	trans_ylist = std::vector<float>(lines);
	ecc_list = std::vector<float>(lines);

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k, &trans_xlist, &trans_ylist, &ecc_list, this]() {
				auto& amplitudes = getImpl().amplitudes;
				CvProcess cvProc;

				for (int i : tasks[k]) {
					for (int j = 1; j < amplitudes[i].size(); j++) {
						float tx, ty, ecc;
						bool result = cvProc.computeTranslation(amplitudes[i - 1][0], amplitudes[i][0], tx, ty, ecc, 500, 1e-4);
						if (result) {
							trans_xlist[i] = tx;
							trans_ylist[i] = ty;
							ecc_list[i] = ecc;
						}
					}
				}
				}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Sequential displacements computed, elapsed: " << msec;
	return true;
}


Amplitudes& OctAngio::AngioData::getAmplitudes(void) const
{
	return getImpl().amplitudes;
}

Imaginaries & OctAngio::AngioData::getImaginaries(void) const
{
	return getImpl().imags;
}

Reals & OctAngio::AngioData::getReals(void) const
{
	return getImpl().reals;
}

int OctAngio::AngioData::dataWidth(void) const
{
	return getImpl().imageWidth;
}

int OctAngio::AngioData::dataHeight(void) const
{
	return getImpl().imageHeight;
}


unsigned char * OctAngio::AngioData::getGrayscaledDataBits(int lineIdx, int repeatIdx, bool vflip)
{
	static vector<unsigned char> grays;

	auto data = getAmplitudes();
	if (data.size() < lineIdx || data[lineIdx].size() < repeatIdx) {
		return nullptr;
	}

	CvImage image;
	data[lineIdx][repeatIdx].copyTo(&image);

	auto* bits = (float*)image.getBitsData();
	auto size = image.getWidth() * image.getHeight();
	if (size < 0) {
		return nullptr;
	}

	if (vflip) {
		image.flipVertical();
	}

	grays = vector<unsigned char>(size);
	for (int i = 0; i < size; i++) {
		auto gval = (int)(((log10(bits[i]) - 2.34f) / (4.5f - 2.34f)) * 255);
		grays[i] = max(min(gval, 255), 0);
	}

	return &grays[0];
}


bool OctAngio::AngioData::readDataFile(int lineIdx, int repeatIdx, char * buff, int dataSize, const std::string dirPath)
{
	std::string path;
	path = (boost::format("%s//%03d_%02d.dat") % dirPath % lineIdx % repeatIdx).str();

	try {
		std::ifstream file(path, std::ios::in | std::ofstream::binary);
		file.read((char*)buff, sizeof(unsigned short)*dataSize);
		file.close();
	}
	catch (...) {
		LogD() << "Angio data file read failed, path: " << path;
		return false;
	}
	return true;
}


bool OctAngio::AngioData::writeDataFile(int lineIdx, int repeatIdx, char * buff, int dataSize, const std::string dirPath)
{
	std::string path;
	path = (boost::format("%s//%03d_%02d.dat") % dirPath % lineIdx % repeatIdx).str();

	try {
		std::ofstream file(path, std::ios::out | std::ofstream::binary);
		file.write((char*)buff, sizeof(unsigned short)*dataSize);
		file.close();
	}
	catch (...) {
		LogD() << "Angio data file write failed, path: " << path;
		return false;
	}
	return true;
}


bool OctAngio::AngioData::readImageDataFile(int lineIdx, int repeatIdx, char * rbuff, char * ibuff, int dataSize, const std::string dirPath)
{
	int major = lineIdx;
	int minor = repeatIdx;

	std::string path1, path2;
	path1 = (boost::format("%s//%03d_%02d_r.bin") % dirPath % major % minor).str();
	path2 = (boost::format("%s//%03d_%02d_i.bin") % dirPath % major % minor).str();

	try {
		std::ifstream file(path1, std::ios::in | std::ofstream::binary);
		if (!file.good()) {
			return false;
		}
		file.read(rbuff, sizeof(float)*dataSize);
		file.close();
	}
	catch (...) {
		LogD() << "Angio real file read failed, path: " << path1;
		return false;
	}

	try {
		std::ifstream file2(path2, std::ios::in | std::ofstream::binary);
		if (!file2.good()) {
			return false;
		}
		file2.read(ibuff, sizeof(float)*dataSize);
		file2.close();
	}
	catch (...) {
		LogD() << "Angio imag file read failed, path: " << path2;
		return false;
	}
	return true;
}


bool OctAngio::AngioData::makeAmplitudeFromImageData(int lineIdx, int repeatIdx, const float * rbuff, const float * ibuff)
{
	if (rbuff == nullptr || ibuff == nullptr) {
		return false;
	}

	auto& amplitudes = getImpl().amplitudes;
	if (amplitudes.size() < lineIdx || amplitudes[lineIdx].size() < repeatIdx) {
		return false;
	}

	bool isComplex = (getImpl().imags.size() == amplitudes.size() && getImpl().reals.size() == amplitudes.size());

	int rowStart = getImpl().dataRowStart;
	int rowEnd = getImpl().dataRowEnd;
	int dsize = getImpl().dataSize;

	int width = getImpl().imageWidth;
	int height = getImpl().imageHeight;
	auto isize = getImpl().imageSize;

	vector<float> data = vector<float>(isize, 0.0f);
	vector<float> imag = vector<float>(isize, 0.0f);
	vector<float> real = vector<float>(isize, 0.0f);

	auto reals = (float*)rbuff; 
	auto imags = (float*)ibuff; 

	int index, cnt = 0;
	for (int r = rowStart; r < rowEnd; r++) {
		for (int c = 0; c < width; c++) {
			index = r * width + c;
			if (isComplex) {
				real[cnt] = reals[index]; // pow(reals[index], 2.0f);
				imag[cnt] = imags[index]; // pow(imags[index], 2.0f);
			}
			data[cnt++] = sqrt(pow(reals[index], 2.0f) + pow(imags[index], 2.0f));
		}
	}

	amplitudes[lineIdx][repeatIdx].fromFloat32((const unsigned char*)&data[0], width, height);

	if (isComplex) {
		getImpl().reals[lineIdx][repeatIdx].fromFloat32((const unsigned char*)&real[0], width, height);
		getImpl().imags[lineIdx][repeatIdx].fromFloat32((const unsigned char*)&imag[0], width, height);
	}
	return true;
}


void OctAngio::AngioData::registerAmplitudes(int lineIdx, const LayerArrays& uppers, const LayerArrays& lowers)
{
	auto& ampls = getImpl().amplitudes;
	if (ampls[lineIdx].size() < 2) {
		return;
	}

	auto& list = ampls[lineIdx];
	auto& base = list[0];
	int w = base.getWidth();
	int h = base.getHeight();

	auto roiUpps = vector<int>(w);
	auto roiLows = vector<int>(w);
	auto roiSize = vector<int>(w, 0);

	/*
	auto& upps = uppers[lineIdx];
	auto& lows = lowers[lineIdx];
	if (upps.size() == w && std::none_of(upps.cbegin(), upps.cend(), [&](int e) { return e < 0 || e >= h; })) {
		roiUpps = uppers[lineIdx];
	}
	if (lows.size() == w && std::none_of(upps.cbegin(), upps.cend(), [&](int e) { return e < 0 || e >= h; })) {
		roiLows = lowers[lineIdx];
	}
	*/

	if (false && roiUpps.size() > 0 && roiLows.size() > 0) {
		for_each(roiUpps.begin(), roiUpps.end(), [=](int &e) { return min(max(e, 10), h - 10); });
		for_each(roiLows.begin(), roiLows.end(), [=](int &e) { return min(max(e, 10), h - 10); });
		std::transform(roiUpps.cbegin(), roiUpps.cend(), roiLows.cbegin(), roiSize.begin(), [](int e1, int e2) {
			return max(e2 - e1 + 1, 0);
		});
	}
	else {
		CvImage bias;
		int bw = w / 2;
		int bh = h / 2;
		base.resizeFastTo(&bias, bw, bh);
		bias.applyGaussianBlur(9.0);

		float *p = (float*)bias.getBitsData();

		for (int c = 0; c < bw; c++) {
			float peak = -9999.0f;
			int ppos = 0;
			for (int r = 0; r < bh; r++) {
				const float val = p[r*bw + c];
				if (peak < val) {
					peak = val;
					ppos = r;
				}
			}
			ppos *= 2;
			int u = max(ppos - 120, 15);
			int l = min(ppos + 120, h - 15);
			int s = l - u + 1;

			for (int n = 0; n <= 1; n++) {
				roiUpps[c * 2 + n] = u;
				roiLows[c * 2 + n] = l;
				roiSize[c * 2 + n] = s;
			}
			/*
			if (lineIdx == 0) {
				LogD() << "upp: " << u << ", low: " << l;
			}
			*/
		}
	}


	for (int j = 1; j < list.size(); j++) {
		float* p1 = (float*)base.getBitsData();
		float* p2 = (float*)list[j].getBitsData();
		auto cents = vector<float>(w, 0.0f);

		for (int c = 0; c < w; c++) {
			int padds = 10;
			int spos = roiUpps[c];
			int epos = roiLows[c];
			int cnts = roiSize[c];

			if (cnts <= 0) {
				continue;
			}

			auto v1 = vector<float>(cnts + padds * 2, 0.0f);
			auto v2 = vector<float>(cnts + padds * 2, 0.0f);
			auto vc = vector<float>(cnts, 0.0f);

			for (int r = spos - padds, k = 0; r <= epos + padds; r++, k++) {
				v1[k] = *(p1 + r*w + c) / 255.0f;
				v2[k] = *(p2 + r*w + c) / 255.0f;
			}

			auto corrs = vector<float>();

			for (int n = -padds; n <= padds; n++) {
				transform(v2.begin() + padds + n, v2.end() - padds + n, v1.begin() + padds, vc.begin(), multiplies<float>());
				float sum = std::accumulate(vc.begin(), vc.end(), 0.0f);
				corrs.push_back(sum);

				/*
				if (lineIdx == 18) {
					LogD() << "c: " << c << ", n: " << n << ", sum: " << sum;
				}
				*/
			}

			int half = 3;
			auto ppos = max_element(corrs.cbegin() + half, corrs.cend() - half);
			if (*ppos < 150.0f) {
				continue;
			}

			// auto ppos = corrs.cbegin() + corrs.size() / 2;
			auto minc = *min_element(ppos - half, ppos + half + 1);
			auto pidx = (int)distance(corrs.cbegin(), ppos);

			float wsum = 0.0f;
			float dsum = 0.0f;
			int sigma = 2;
			for (int k = pidx - sigma; k <= pidx + sigma; k++) {
				dsum += (corrs[k] - minc) * k;
				wsum += (corrs[k] - minc);
			}
			float cent = (wsum > 0.0f ? dsum / wsum : 0.0f) - padds;
			cents[c] = cent;

			/*
			if (lineIdx == 0) {
				LogD() << "c: " << c << ", upps: " << upps[c] << ", lows: " << lows[c] << ", pidx: " << pidx << ", cent: " << cent << ", minc: " << minc;
			}
			*/
		}

		for (int c = 0; c < w; c++) {
			auto data = vector<float>(h, 0.0f);
			for (int m = 0; m < h; m++) {
				data[m] = *(p2 + m*w + c);
			}

			int ksize = w / 16;
			int sidx = max(c - ksize, 0);
			int eidx = min(c + ksize, w - 1);
			float csum = 0.0f;
			float ccnt = 0;
			for (int i = sidx; i <= eidx; i++) {
				csum += cents[i];
				ccnt += 1;
			}
			float cavg = csum / ccnt;

			float x = cavg; // trans[c];
			float a = 3;
			float L[6] = { 0.0f };
			float PI = (float)CppUtil::NumericFunc::PI();
			for (int i = (int)(floor(x) - a + 1), k = 0; i <= (floor(x) + a); i++, k++) {
				float t = x - i;
				float v = 0.0f;
				if (t == 0.0f) {
					v = 1.0f;
				}
				else if (-a <= t && t < a) {
					v = a * sin(PI*t) * sin(PI*t/a) / (PI*PI*t*t);
				}
				else {
					v = 0.0f;
				}
				L[k] = v;
			}

			for (int m = 0; m < h; m++) {
				float fidx = m + x; 
				int nidx = (int)floor(fidx);
				int sidx = (int)(nidx - a + 1);
				int eidx = (int)(nidx + a);
				if (sidx < 0 || eidx >= h) {
					continue;
				}

				float intp = 0.0f;
				for (int n = sidx, k = 0; n <= eidx; n++, k++) {
					intp += data[n] * L[k];
				}
				*(p2 + m*w + c) = max(intp, 0.0f);
			}
		}
	}
	return;
}

void OctAngio::AngioData::registerAmplitudes2(int lineIdx, const LayerArrays & uppers, const LayerArrays & lowers)
{
	auto& ampls = getImpl().amplitudes;
	if (ampls[lineIdx].size() < 2) {
		return;
	}

	auto& reals = getImpl().reals;
	auto& imags = getImpl().imags;
	bool isComplex = (imags.size() == ampls.size() && reals.size() == ampls.size());

	auto& list = ampls[lineIdx];
	auto& base = list[0];
	int w = base.getWidth();
	int h = base.getHeight();

	auto roi_upps = vector<int>(w);
	auto roi_lows = vector<int>(w);
	auto roi_size = vector<int>(w, 0);

	const float BIAS_BLUR_SIZE = 9.0f;
	const int ROI_WINDOW = 128; // 64;
	const int ROI_MARGIN = 32; // 16;
	const int ROI_EXTRAS = 32; // 16;
	const float CORR_PEAK_MIN = 150.0f;
	const int CORR_MASK_SIZE = 7;
	const int CORR_MASK_HALF = (CORR_MASK_SIZE / 2);
	const int CORR_PEAK_SIGMA = 2;

	CvImage bias;
	int bias_w = w / 2;
	int bias_h = h / 2;
	base.resizeFastTo(&bias, bias_w, bias_h);
	bias.applyGaussianBlur(BIAS_BLUR_SIZE);

	float *p_bias = (float*)bias.getBitsData();

	for (int c = 0; c < bias_w; c++) {
		float peak = -9999.0f;
		int ppos = 0;
		for (int r = 0; r < bias_h; r++) {
			const float val = p_bias[r*bias_w + c];
			if (peak < val) {
				peak = val;
				ppos = r;
			}
		}
		ppos *= 2;

		int y1 = max(ppos - ROI_WINDOW, ROI_MARGIN);
		int y2 = min(ppos + ROI_WINDOW, h - ROI_MARGIN);
		int sz = y2 - y1 + 1;

		for (int n = 0; n <= 1; n++) {
			roi_upps[c * 2 + n] = y1;
			roi_lows[c * 2 + n] = y2;
			roi_size[c * 2 + n] = sz;
		}
	}
	
	for (int j = 1; j < list.size(); j++) {
		float* p1 = (float*)base.getBitsData();
		float* p2 = (float*)list[j].getBitsData();

		float* p_real = (isComplex ? (float*)reals[lineIdx][j].getBitsData() : nullptr);
		float* p_imag = (isComplex ? (float*)imags[lineIdx][j].getBitsData() : nullptr);
		float* r_base = (isComplex ? (float*)reals[lineIdx][0].getBitsData() : nullptr);
		float* i_base = (isComplex ? (float*)imags[lineIdx][0].getBitsData() : nullptr);

		auto cents = vector<float>(w, 0.0f);

		for (int c = 0; c < w; c++) {
			int padd = ROI_EXTRAS;
			int y1 = roi_upps[c];
			int y2 = roi_lows[c];
			int size = roi_size[c];

			if (size <= 0) {
				continue;
			}

			auto v1 = vector<float>(size + padd * 2, 0.0f);
			auto v2 = vector<float>(size + padd * 2, 0.0f);
			auto vc = vector<float>(size, 0.0f);

			for (int r = y1 - padd, k = 0; r <= y2 + padd; r++, k++) {
				v1[k] = *(p1 + r*w + c) / 255.0f;
				v2[k] = *(p2 + r*w + c) / 255.0f;
			}

			auto corrs = vector<float>();
			for (int n = -padd; n <= padd; n++) {
				transform(v2.begin() + padd + n, v2.end() - padd + n, v1.begin() + padd, vc.begin(), multiplies<float>());
				float sum = std::accumulate(vc.begin(), vc.end(), 0.0f);
				corrs.push_back(sum);
			}

			int half = CORR_MASK_HALF;
			auto ppos = max_element(corrs.cbegin() + half, corrs.cend() - half);
			if (*ppos < CORR_PEAK_MIN) {
				continue;
			}

			auto minc = *min_element(ppos - half, ppos + half + 1);
			auto pidx = (int)distance(corrs.cbegin(), ppos);

			float wsum = 0.0f;
			float dsum = 0.0f;
			int sigma = CORR_PEAK_SIGMA;
			for (int k = pidx - sigma; k <= pidx + sigma; k++) {
				dsum += (corrs[k] - minc) * k;
				wsum += (corrs[k] - minc);
			}
			float cent = (wsum > 0.0f ? dsum / wsum : 0.0f) - padd;
			cents[c] = cent;
		}

		for (int c = 0; c < w; c++) {
			auto data = vector<float>(h, 0.0f);
			auto real = vector<float>(h, 0.0f);
			auto imag = vector<float>(h, 0.0f);
			auto phase = vector<float>(h, 0.0f);
			auto pbase = vector<float>(h, 0.0f);
			auto pmove = vector<float>(h, 0.0f);

			for (int m = 0; m < h; m++) {
				data[m] = *(p2 + m*w + c);
			}

			if (isComplex) {
				for (int m = 0; m < h; m++) {
					real[m] = *(p_real + m*w + c);
					imag[m] = *(p_imag + m*w + c);
					
					float a = atan2(imag[m], real[m]);
					phase[m] = (float)(a < 0.0 ? 2.0f * NumericFunc::PI() + a : a);

					a = atan2(*(i_base + m*w + c), *(r_base + m*w + c));
					pbase[m] = (float)(a < 0.0 ? 2.0f * NumericFunc::PI() + a : a);
				}
			}

			/*
			if (lineIdx == 42) { // 160) { // 122) {
				if (c == 116) { // 20) { // 282) {
					if (j == 1) {
						LogD() << "\n";
						LogD() << "Overlap index: " << (j - 1);
						for (int m = 160; m < 320; m++) {
							LogD() << m << " : " << *(r_base + m*w + c) << ", " << *(i_base + m*w + c) << ", " << *(p1 + m*w + c); // << ", " << pbase[m];
						}
					}

					LogD() << "\n";
					LogD() << "Overlap index: " << j;
					for (int m = 160; m < 320; m++) {
						LogD() << m << " : " << real[m] << ", " << imag[m] << ", " << data[m]; // << ", " << phase[m];
					}
				}
			}
			*/

			int ksize = w / 16;
			int sidx = max(c - ksize, 0);
			int eidx = min(c + ksize, w - 1);
			float csum = 0.0f;
			float ccnt = 0;
			for (int i = sidx; i <= eidx; i++) {
				csum += cents[i];
				ccnt += 1;
			}
			float cavg = csum / ccnt;

			float x = cavg; // round(cavg); // trans[c];
			float a = 3;
			float L[6] = { 0.0f };
			float PI = (float)CppUtil::NumericFunc::PI();
			for (int i = (int)(floor(x) - a + 1), k = 0; i <= (floor(x) + a); i++, k++) {
				float t = x - i;
				float v = 0.0f;
				if (t == 0.0f) {
					v = 1.0f;
				}
				else if (-a <= t && t < a) {
					v = a * sin(PI*t) * sin(PI*t / a) / (PI*PI*t*t);
				}
				else {
					v = 0.0f;
				}
				L[k] = v;
			}

			for (int m = 0; m < h; m++) {
				float fidx = m + x;
				int nidx = (int)floor(fidx);
				int sidx = (int)(nidx - a + 1);
				int eidx = (int)(nidx + a);
				if (sidx < 0 || eidx >= h) {
					continue;
				}

				float bias = 0.0f; // 16000.0f;
				float intp = 0.0f;
				for (int n = sidx, k = 0; n <= eidx; n++, k++) {
					intp += data[n] * L[k];
				}
				*(p2 + m*w + c) = max(intp, 0.0f);
				// *(p2 + m*w + c) = data[(int)fidx];
				// *(p2 + m*w + c) = sqrt(imag[m] * imag[m]);

				if (isComplex) {
					/*
					float intp_r = 0.0f;
					for (int n = sidx, k = 0; n <= eidx; n++, k++) {
						intp_r += (real[n] + bias) * L[k];
					}
					*(p_real + m*w + c) = intp_r - bias;

					float intp_i = 0.0f;
					for (int n = sidx, k = 0; n <= eidx; n++, k++) {
						intp_i += (imag[n] + bias) * L[k];
					}
					*(p_imag + m*w + c) = intp_i - bias;

					*(p2 + m*w + c) = sqrt(intp_r*intp_r + intp_i*intp_i);
					*/
					/*
					float intp_a = 0.0f;
					for (int n = sidx, k = 0; n <= eidx; n++, k++) {
						intp_a += (phase[n] + bias) * L[k];
					}
					pmove[m] = intp_a - bias;
					*/
					/*
					float intp_a = 0.0f;
					for (int n = sidx, k = 0; n <= eidx; n++, k++) {
						intp_a += (phase[n] + bias) * L[k];
					}

					auto intp_r = *(p2 + m*w + c) * cos(intp_a);
					auto intp_i = *(p2 + m*w + c) * sin(intp_a);

					*(p2 + m*w + c) = sqrt(intp_r*intp_r + intp_i*intp_i);
					*(p_real + m*w + c) = intp_r - bias;
					*(p_imag + m*w + c) = intp_i - bias;
					*/
				}
			}

			/*
			if (isComplex) {
				if (lineIdx == 122) { // 160) { // 122) {
					if (c == 282) { // 20) { // 282) {
						LogD() << "\nAligned" << ", " << x;
						LogD() << "Line index: " << j;
						for (int m = 230; m < 450; m++) { */
							/*
							float r_val = *(p_real + m*w + c);
							float i_val = *(p_imag + m*w + c);
							float a_val = sqrt(r_val * r_val + i_val * i_val);
							LogD() << m << " : " << r_val << ", " << i_val << ", " << a_val << ", " << *(p2 + m*w + c);
							*/ /*
							LogD() << m << " : " << *(p2 + m*w + c) << ", " << pmove[m];
						}
					}
				}
			} */
		}
	}
	return;
}


AngioData::AngioDataImpl & OctAngio::AngioData::getImpl(void) const
{
	return *d_ptr;
}
