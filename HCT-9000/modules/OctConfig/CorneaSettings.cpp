#include "stdafx.h"
#include "CorneaSettings.h"


using namespace OctConfig;
using namespace std;


struct CorneaSettings::CorneaSettingsImpl
{
	int pixelsPerMM;
	float smallPupilSize;

	CorneaSettingsImpl() {

	}
};



CorneaSettings::CorneaSettings() :
	d_ptr(make_unique<CorneaSettingsImpl>())
{
	initialize();
}



OctConfig::CorneaSettings::~CorneaSettings() = default;
OctConfig::CorneaSettings::CorneaSettings(CorneaSettings && rhs) = default;
CorneaSettings & OctConfig::CorneaSettings::operator=(CorneaSettings && rhs) = default;


OctConfig::CorneaSettings::CorneaSettings(const CorneaSettings & rhs)
	: d_ptr(make_unique<CorneaSettingsImpl>(*rhs.d_ptr))
{
}


CorneaSettings & OctConfig::CorneaSettings::operator=(const CorneaSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::CorneaSettings::initialize(void)
{
	resetToDefaultValues();
	return;
}


void OctConfig::CorneaSettings::resetToDefaultValues(void)
{
	getImpl().pixelsPerMM = CORNEA_CAMERA_PIXELS_PER_MM;
	getImpl().smallPupilSize = CORNEA_SMALL_PUPIL_SIZE;
	return;
}


int OctConfig::CorneaSettings::getPixelsPerMM(void) const
{
	return getImpl().pixelsPerMM;
}


float OctConfig::CorneaSettings::getSmallPupilSize(void) const
{
	return getImpl().smallPupilSize;
}


void OctConfig::CorneaSettings::setPixelsPerMM(int pixels)
{
	getImpl().pixelsPerMM = min(max(pixels, CORNEA_CAMERA_PIXELS_PER_MM_MIN), CORNEA_CAMERA_PIXELS_PER_MM_MAX);
	return;
}


void OctConfig::CorneaSettings::setSmallPupilSize(float size)
{
	getImpl().smallPupilSize = min(max(size, CORNEA_SMALL_PUPIL_SIZE_MIN), CORNEA_SMALL_PUPIL_SIZE_MAX);
	return;
}


CorneaSettings::CorneaSettingsImpl & OctConfig::CorneaSettings::getImpl(void) const
{
	return *d_ptr;
}

