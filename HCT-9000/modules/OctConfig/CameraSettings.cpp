#include "stdafx.h"
#include "CameraSettings.h"

#include <vector>

using namespace OctConfig;
using namespace std;


struct CameraSettings::CameraSettingsImpl
{
	vector<CameraItem> cameras;

	CameraSettingsImpl()
	{
	}
};


CameraSettings::CameraSettings() :
	d_ptr(make_unique<CameraSettingsImpl>())
{
	initialize();
}


OctConfig::CameraSettings::~CameraSettings() = default;
OctConfig::CameraSettings::CameraSettings(CameraSettings && rhs) = default;
CameraSettings & OctConfig::CameraSettings::operator=(CameraSettings && rhs) = default;


OctConfig::CameraSettings::CameraSettings(const CameraSettings & rhs)
	: d_ptr(make_unique<CameraSettingsImpl>(*rhs.d_ptr))
{
}


CameraSettings & OctConfig::CameraSettings::operator=(const CameraSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CameraItem * OctConfig::CameraSettings::getRetinaCamera(void) const
{
	return getCameraItem(CameraType::RETINA);
}


CameraItem * OctConfig::CameraSettings::getCorneaCamera(void) const
{
	return getCameraItem(CameraType::CORNEA);
}


CameraItem * OctConfig::CameraSettings::getFundusCamera(void) const
{
	return getCameraItem(CameraType::FUNDUS);
}


CameraItem * OctConfig::CameraSettings::getCameraItem(CameraType type) const
{
	for (auto& item : d_ptr->cameras) {
		if (item.isType(type)) {
			return &item;
		}
	}
	return nullptr;
}


CameraItem * OctConfig::CameraSettings::getCameraItem(const char * name) const
{
	for (auto& item : d_ptr->cameras) {
		if (item.isName(name)) {
			return &item;
		}
	}
	return nullptr;
}


CameraItem * OctConfig::CameraSettings::getCameraItem(int index) const
{
	if (index < 0 || index >= getImpl().cameras.size()) {
		return nullptr;
	}
	return &getImpl().cameras[index];
}


int OctConfig::CameraSettings::getCameraListCount(void) const
{
	return (int)getImpl().cameras.size();
}


CameraSettings::CameraSettingsImpl & OctConfig::CameraSettings::getImpl(void) const
{
	return *d_ptr;
}



void OctConfig::CameraSettings::initialize(void)
{
	vector<CameraItem>& list = getImpl().cameras;
	list.clear();
	list.emplace_back(CAMERA_RETINA_NAME, CameraType::RETINA);
	list.emplace_back(CAMERA_CORNEA_NAME, CameraType::CORNEA);
	list.emplace_back(CAMERA_FUNDUS_NAME, CameraType::FUNDUS);

	resetToDefaultValues();
	return;
}


void OctConfig::CameraSettings::resetToDefaultValues(void)
{
	for (int i = 0; i < CAMERA_PRESET_SIZE; i++) {
		getRetinaCamera()->analogGain(i) = CAMERA_RETINA_INIT_AGAIN;
		getRetinaCamera()->digitalGain(i) = CAMERA_RETINA_INIT_DGAIN;
		getCorneaCamera()->analogGain(i) = CAMERA_CORNEA_INIT_AGAIN;
		getCorneaCamera()->digitalGain(i) = CAMERA_CORNEA_INIT_DGAIN;
		getFundusCamera()->analogGain(i) = CAMERA_COLOR_INIT_AGAIN;
		getFundusCamera()->digitalGain(i) = CAMERA_COLOR_INIT_DGAIN;
	}

	getRetinaCamera()->analogGain(1) = CAMERA_RETINA_INIT_AGAIN2;
	getRetinaCamera()->analogGain(2) = CAMERA_RETINA_INIT_AGAIN3;
	getRetinaCamera()->digitalGain(1) = CAMERA_RETINA_INIT_DGAIN2;
	getRetinaCamera()->digitalGain(2) = CAMERA_RETINA_INIT_DGAIN3;

	getRetinaCamera()->shiftX() = 0;
	getRetinaCamera()->shiftY() = 0;

	getCorneaCamera()->shiftX() = 0;
	getCorneaCamera()->shiftY() = 0;

	getFundusCamera()->shiftX() = 0;
	getFundusCamera()->shiftY() = 0;
	return;
}
