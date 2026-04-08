#include "stdafx.h"
#include "RetinaSettings.h"


using namespace OctConfig;
using namespace std;


struct RetinaSettings::RetinaSettingsImpl
{
	bool useEnhance;
    bool useROI;
	bool useCenterMask;

	pair<int, int> splitCenter;
	pair<int, int> splitKernel;
    pair<int, int> centerROI;
	pair<int, int> centerMask;
    int radiusROI;
	int maskSize;

	float enhanceClipLimit;
	int enhanceClipScalar;

	RetinaSettingsImpl() {

	}
};


RetinaSettings::RetinaSettings() :
	d_ptr(make_unique<RetinaSettingsImpl>())
{
	initialize();
}



OctConfig::RetinaSettings::~RetinaSettings() = default;
OctConfig::RetinaSettings::RetinaSettings(RetinaSettings && rhs) = default;
RetinaSettings & OctConfig::RetinaSettings::operator=(RetinaSettings && rhs) = default;


OctConfig::RetinaSettings::RetinaSettings(const RetinaSettings & rhs)
	: d_ptr(make_unique<RetinaSettingsImpl>(*rhs.d_ptr))
{
}


RetinaSettings & OctConfig::RetinaSettings::operator=(const RetinaSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::RetinaSettings::initialize(void)
{
	resetToDefaultValues();
	return;
}


void OctConfig::RetinaSettings::resetToDefaultValues(void)
{
	getImpl().useEnhance = true;
	getImpl().useROI = true;
	getImpl().useCenterMask = true;

	setSplitCenter(std::pair<int, int>(RETINA_SPLIT_CENTER_INIT_X, RETINA_SPLIT_CENTER_INIT_Y));
	setSplitKernel(std::pair<int, int>(RETINA_SPLIT_KERNAL_INIT_WIDTH, RETINA_SPLIT_KERNAL_INIT_HEIGHT));
	setCenterPointOfROI(std::pair<int, int>(RETINA_ROI_INIT_CENTER_X, RETINA_ROI_INIT_CENTER_Y));
	setRadiusOfROI(RETINA_ROI_INIT_RADIUS);

	setCenterPointOfMask(std::pair<int, int>(RETINA_ROI_INIT_CENTER_X, RETINA_ROI_INIT_CENTER_Y));
	setCenterMaskSize(RETINA_CENTER_MASK_INIT_SIZE);

	setEnhanceClipLimit(RETINA_ENHANCE_CLIP_LIMIT);
	setEnhanceClipScalar(RETINA_ENHANCE_CLIP_SCALAR);
	return;
}


std::pair<int, int> OctConfig::RetinaSettings::getSplitCenter(void) const
{
	return getImpl().splitCenter;
}


std::pair<int, int> OctConfig::RetinaSettings::getSplitKernel(void) const
{
	return getImpl().splitKernel;
}


void OctConfig::RetinaSettings::setSplitCenter(std::pair<int, int> center)
{
	getImpl().splitCenter = center;
	return;
}


void OctConfig::RetinaSettings::setSplitKernel(std::pair<int, int> kernel)
{
	getImpl().splitKernel = kernel;
	return;
}


std::pair<int, int> OctConfig::RetinaSettings::getCenterPointOfROI(void) const
{
	return getImpl().centerROI;
}


std::pair<int, int> OctConfig::RetinaSettings::getCenterPointOfMask(void) const
{
	return getImpl().centerMask;
}


int OctConfig::RetinaSettings::getRadiusOfROI(void) const
{
	return getImpl().radiusROI;
}

int OctConfig::RetinaSettings::getCenterMaskSize(void) const
{
	return getImpl().maskSize;
}


void OctConfig::RetinaSettings::setCenterPointOfROI(std::pair<int, int> center)
{
	if (center.first <= 0 || center.first > 640 || center.second <= 0 || center.second > 480) {
		setCenterPointOfROI(std::pair<int, int>(RETINA_ROI_INIT_CENTER_X, RETINA_ROI_INIT_CENTER_Y));
	}
	else {
		getImpl().centerROI = center;
	}
	return;
}


void OctConfig::RetinaSettings::setCenterPointOfMask(std::pair<int, int> center)
{
	if (center.first <= 0 || center.first > 640 || center.second <= 0 || center.second > 480) {
		auto roi = getImpl().centerROI;
		if (roi.first > 0 && roi.first < 640 && roi.second > 0 && roi.second < 480) {
			getImpl().centerMask = roi;
		}
		else {
			setCenterPointOfMask(std::pair<int, int>(RETINA_ROI_INIT_CENTER_X, RETINA_ROI_INIT_CENTER_Y));
		}
	}
	else {
		getImpl().centerMask = center;
	}
	return;
}


void OctConfig::RetinaSettings::setRadiusOfROI(int radius)
{
	getImpl().radiusROI = radius;
	return;
}


void OctConfig::RetinaSettings::setCenterMaskSize(int size)
{
	size = min(max(size, 0), 120);
	getImpl().maskSize = size;
	return;
}

float OctConfig::RetinaSettings::getEnhanceClipLimit(void) const
{
	return getImpl().enhanceClipLimit;
}

int OctConfig::RetinaSettings::getEnhanceClipScalar(void) const 
{
	return getImpl().enhanceClipScalar;
}


void OctConfig::RetinaSettings::setEnhanceClipLimit(float value)
{
	if (std::isnan(value) || value < 0.0f || value >= 10.0f) {
		value = RETINA_ENHANCE_CLIP_LIMIT;
	}
	getImpl().enhanceClipLimit = value;
	return;
}


void OctConfig::RetinaSettings::setEnhanceClipScalar(int value)
{
	if (value < 1 || value >= 100) {
		value = RETINA_ENHANCE_CLIP_SCALAR;
	}
	getImpl().enhanceClipScalar = value;
	return;
}


bool OctConfig::RetinaSettings::useImageEnhance(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useEnhance = flag;
	}
	return getImpl().useEnhance;
}


bool OctConfig::RetinaSettings::useCenterMask(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useCenterMask = flag;
	}
	return getImpl().useCenterMask;
}


bool OctConfig::RetinaSettings::useROI(bool isset, bool flag) const
{
    if (isset) {
        getImpl().useROI = flag;
    }
    return getImpl().useROI;
}


RetinaSettings::RetinaSettingsImpl & OctConfig::RetinaSettings::getImpl(void) const
{
	return *d_ptr;
}

