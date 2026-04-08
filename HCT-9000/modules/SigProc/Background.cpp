#include "stdafx.h"
#include "Background.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <iterator>

#include "CppUtil2.h"


using namespace SigProc;
using namespace CppUtil;
using namespace std;


struct Background::BackgroundImpl
{
	unsigned short data[LINE_CAMERA_CCD_PIXELS];
	bool empty;

	BackgroundImpl() : data { 0 }, empty(true)
	{
	}
};


Background::Background() :
	d_ptr(make_unique<BackgroundImpl>())
{
}


Background::~Background()
{
}


SigProc::Background::Background(Background && rhs) = default;
Background & SigProc::Background::operator=(Background && rhs) = default;


void SigProc::Background::makeBackgroundSpectrum(const unsigned short * data, int width, int height)
{
	if (data == nullptr || width != LINE_CAMERA_CCD_PIXELS) {
		return;
	}

	unsigned int buff[LINE_CAMERA_CCD_PIXELS] = { 0 };
	unsigned short bgrd[LINE_CAMERA_CCD_PIXELS] = { 0 };
	int padd = height / 10;

	int sidx = padd;
	int eidx = height - padd;
	int size = (eidx - sidx + 1);

	// Make an average axial scan profile from resampled data.
	for (int i = 0; i < width; i++) {
		for (int j = sidx; j < eidx; j++) {
			buff[i] += data[i + j * width];
		}
		bgrd[i] = (unsigned short)(buff[i] / size);
	}

	setProfileData(bgrd);
	return;
}


void SigProc::Background::setProfileData(const unsigned short * data, int size)
{
	memcpy(getImpl().data, data, size * sizeof(unsigned short));
	getImpl().empty = false;
	return;
}


unsigned short * SigProc::Background::getProfileData(void) const
{
	return getImpl().data;
}


bool SigProc::Background::isEmpty(void) const
{
	return getImpl().empty;
}


void SigProc::Background::setEmpty(bool flag) 
{
	getImpl().empty = flag;
	return;
}


void SigProc::Background::dumpToFile(void)
{
	std::string path = ".//background.bin";

	std::ofstream file(path, std::ios::out | std::ofstream::binary);
	file.write((char *)getProfileData(), sizeof(unsigned short)*LINE_CAMERA_CCD_PIXELS);
	file.close();
	return;
}


Background::BackgroundImpl & SigProc::Background::getImpl(void) const
{
	return *d_ptr;
}
