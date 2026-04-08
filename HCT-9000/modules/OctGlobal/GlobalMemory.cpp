#include "stdafx.h"
#include "GlobalMemory.h"
#include "GlobalScanDef.h"

using namespace OctGlobal;
using namespace std;


struct GlobalMemory::GlobalMemoryImpl
{
	unsigned short backgroundSpectrum[LINE_CAMERA_CCD_PIXELS];
	float octManualExposTime = 0.0f;

	

	GlobalMemoryImpl() 
		: backgroundSpectrum{0}
	{

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<GlobalMemory::GlobalMemoryImpl> GlobalMemory::d_ptr(new GlobalMemoryImpl());


GlobalMemory::GlobalMemory()
{
}


GlobalMemory::~GlobalMemory()
{
}


unsigned short * OctGlobal::GlobalMemory::getOctBackgroundSpectrum(void)
{
	return d_ptr->backgroundSpectrum;
}


void OctGlobal::GlobalMemory::setOctBackgroundSpectrum(unsigned short * data)
{
	memcpy((void*)d_ptr->backgroundSpectrum, (void*)data, sizeof(unsigned short)*LINE_CAMERA_CCD_PIXELS);
	return;
}

void OctGlobal::GlobalMemory::setOctManualExposureTime(float expTime)
{
	getImpl().octManualExposTime = expTime;
	return;
}

float OctGlobal::GlobalMemory::getOctManualExposureTime(bool clear)
{
	auto time = getImpl().octManualExposTime;
	if (clear) {
		getImpl().octManualExposTime = 0.0f;
	}
	return time;
}

bool OctGlobal::GlobalMemory::isOctManualExposureTime(void)
{
	auto time = getImpl().octManualExposTime;
	return (time >= 1.0f && 99.0f);
}

void OctGlobal::GlobalMemory::clearOctManualExposureTime(void)
{
	getImpl().octManualExposTime = 0.0f;
	return;
}

GlobalMemory::GlobalMemoryImpl & OctGlobal::GlobalMemory::getImpl(void)
{
	return *d_ptr;
}
