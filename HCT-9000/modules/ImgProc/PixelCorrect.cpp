#include "stdafx.h"
#include "PixelCorrect.h"

#include "BadPixelDetector.h"
#include "BadPixelCorrector.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <set>


using namespace ImgProc;
using namespace std;


struct PixelCorrect::PixelCorrectImpl
{
	std::unique_ptr<BadPixelDetector> bpd;
	std::unique_ptr<BadPixelCorrector> bpc;

	std::vector<std::string> whites;
	std::vector<std::string> blacks;
	std::vector<std::pair<unsigned int, unsigned int>> bads;

	int darkR;
	int darkG;
	int darkB;
	int brightR;
	int brightG;
	int brightB;

	PixelCorrectImpl() : bpd(nullptr), bpc(nullptr),
		darkR(500), darkG(400), darkB(300), brightR(3900), brightB(3900), brightG(3900)
	{
		bpc = std::make_unique<BadPixelCorrector>(FUNDUS_FRAME_WIDTH, FUNDUS_FRAME_HEIGHT, BadPixelCorrector::BAYER_FORMAT::GB);
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<PixelCorrect::PixelCorrectImpl> PixelCorrect::d_ptr(new PixelCorrectImpl());


PixelCorrect::PixelCorrect()
{
}


PixelCorrect::~PixelCorrect()
{
}


int ImgProc::PixelCorrect::readWhitePictures(const std::vector<std::wstring>& fileList)
{
	int count = 0;
	d_ptr->whites.clear();
	for (auto f : fileList) {
		if (SystemFunc::isFile(f)) {
			d_ptr->whites.push_back(wtoa(f));
			count++;
		}
	}
	return count;
}


int ImgProc::PixelCorrect::readBlackPictures(const std::vector<std::wstring>& fileList)
{
	int count = 0;
	d_ptr->blacks.clear();
	for (auto f : fileList) {
		if (SystemFunc::isFile(f)) {
			d_ptr->blacks.push_back(wtoa(f));
			count++;
		}
	}
	return count;
}


int ImgProc::PixelCorrect::detectBadPixels(int numImages, int width, int height)
{
	if (d_ptr->blacks.size() < numImages || d_ptr->whites.size() < numImages ||
		d_ptr->blacks.size() != d_ptr->whites.size()) {
		return -1;
	}

	d_ptr->bpd = std::make_unique<BadPixelDetector>(width, height, BadPixelDetector::BAYER_FORMAT::GB);
	d_ptr->bpd->setBrightImages(d_ptr->whites);
	d_ptr->bpd->setDarkImages(d_ptr->blacks);

	d_ptr->bpd->setDarkThresholdR(d_ptr->darkR);
	d_ptr->bpd->setDarkThresholdG(d_ptr->darkG);
	d_ptr->bpd->setDarkThresholdB(d_ptr->darkB);
	d_ptr->bpd->setBrightThresholdR(d_ptr->brightR);
	d_ptr->bpd->setBrightThresholdG(d_ptr->brightG);
	d_ptr->bpd->setBrightThresholdB(d_ptr->brightB);

	try {
		d_ptr->bpd->detect();
	}
	catch (...) {
		return -1;
	}

	auto badPixels = d_ptr->bpd->getBadPixels();
	d_ptr->bads.clear();
	for (auto b : badPixels) {
		d_ptr->bads.push_back(b);
	}

	d_ptr->bpc = std::make_unique<BadPixelCorrector>(width, height, BadPixelCorrector::BAYER_FORMAT::GB);
	d_ptr->bpc->setBadPixels(badPixels);

	return countBadPixels();
}


int ImgProc::PixelCorrect::countBadPixels(void)
{
	if (isDetected()) {
		return (int) d_ptr->bads.size();
	}
	return 0;
}


int ImgProc::PixelCorrect::getBadPixelY(int index)
{
	if (index >= 0 && index < countBadPixels()) {
		return (int) d_ptr->bads[index].second;
	}
	return -1;
}


int ImgProc::PixelCorrect::getBadPixelX(int index)
{
	if (index >= 0 && index < countBadPixels()) {
		return (int)d_ptr->bads[index].first;
	}
	return -1;
}


std::vector<std::pair<unsigned int, unsigned int>>& ImgProc::PixelCorrect::getBadPixelCoords(void)
{
	return d_ptr->bads;
}


bool ImgProc::PixelCorrect::correctBadPixels(unsigned short* frame)
{
	if (isDetected()) {
		d_ptr->bpc->correct_median(frame);
		return true;
	}
	return false;
}


bool ImgProc::PixelCorrect::isDetected(void)
{
	return (d_ptr->bpd != nullptr && d_ptr->bpc != nullptr);
}


void ImgProc::PixelCorrect::setDarkThresholds(int r, int g, int b)
{
	d_ptr->darkR = r;
	d_ptr->darkG = g;
	d_ptr->darkB = b;
	return;
}


void ImgProc::PixelCorrect::setBrightThresholds(int r, int g, int b)
{
	d_ptr->brightR = r;
	d_ptr->brightG = g;
	d_ptr->brightB = b;
	return;
}


bool ImgProc::PixelCorrect::exportBadPixels(std::wstring filename)
{
	wofstream file;
	file.open(filename);
	int count = 1;
	for (auto pixel : d_ptr->bads) {
		file << "[" << count << "] : " << pixel.first << ", " << pixel.second << "\r\n";
		count++;
	}
	file.close();
	return false;
}


PixelCorrect::PixelCorrectImpl & ImgProc::PixelCorrect::getImpl(void)
{
	return *d_ptr;
}
