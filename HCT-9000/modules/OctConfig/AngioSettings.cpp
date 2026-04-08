#include "stdafx.h"
#include "AngioSettings.h"

#include <vector>

using namespace OctConfig;
using namespace std;


struct AngioSettings::AngioSettingsImpl
{
	float decorThreshold;
	int averageOffset;
	float motionThreshold;
	float motionOverPoints;
	float motionDistRatio;
	int filterOrients;
	float filterSigma;
	float filterDivider;
	float filterWeight;
	float enhanceParam;
	float biasFieldSigma;

	AngioSettingsImpl()
	{
	}
};


AngioSettings::AngioSettings() :
	d_ptr(make_unique<AngioSettingsImpl>())
{
	initialize();
}


OctConfig::AngioSettings::~AngioSettings() = default;
OctConfig::AngioSettings::AngioSettings(AngioSettings && rhs) = default;
AngioSettings & OctConfig::AngioSettings::operator=(AngioSettings && rhs) = default;


OctConfig::AngioSettings::AngioSettings(const AngioSettings & rhs)
	: d_ptr(make_unique<AngioSettingsImpl>(*rhs.d_ptr))
{
}


AngioSettings & OctConfig::AngioSettings::operator=(const AngioSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


AngioSettings::AngioSettingsImpl & OctConfig::AngioSettings::getImpl(void) const
{
	return *d_ptr;
}



void OctConfig::AngioSettings::initialize(void)
{
	resetToDefaultValues();
	return;
}


void OctConfig::AngioSettings::resetToDefaultValues(void)
{
	getImpl().decorThreshold = 0.0f; // 0.15f;
	getImpl().averageOffset = ANGIO_AVERAGE_OFFSET; // 3;
	getImpl().motionThreshold = ANGIO_MOTION_THRESHOLD;
	getImpl().motionOverPoints = ANGIO_MOTION_OVER_POINTS;
	getImpl().motionDistRatio = ANGIO_MOTION_DIST_RATIO;
	getImpl().filterOrients = ANGIO_GABOR_FILTER_ORIENTS;
	getImpl().filterSigma = ANGIO_GABOR_FILTER_SIGMA;
	getImpl().filterDivider = ANGIO_GABOR_FILTER_DIVIDER;
	getImpl().filterWeight = ANGIO_GABOR_FILTER_WEIGHT; // 0.5f;
	getImpl().enhanceParam = ANGIO_ENHANCE_PARAM;
	getImpl().biasFieldSigma = ANGIO_BIAS_FIELD_SIGMA;
	return;
}

float OctConfig::AngioSettings::getDecorThreshold(void)
{
	return getImpl().decorThreshold;
}

int OctConfig::AngioSettings::getAverageOffset(void)
{
	return getImpl().averageOffset;
}

float OctConfig::AngioSettings::getMotionThreshold(void)
{
	return getImpl().motionThreshold;
}

float OctConfig::AngioSettings::getMotionOverPoints(void)
{
	return getImpl().motionOverPoints;
}

float OctConfig::AngioSettings::getMotionDistRatio(void)
{
	return getImpl().motionDistRatio;
}

int OctConfig::AngioSettings::getFilterOrients(void)
{
	return getImpl().filterOrients;
}

float OctConfig::AngioSettings::getFilterSigma(void)
{
	return getImpl().filterSigma;
}

float OctConfig::AngioSettings::getFilterDivider(void)
{
	return getImpl().filterDivider;
}

float OctConfig::AngioSettings::getFilterWeight(void)
{
	return getImpl().filterWeight;
}

float OctConfig::AngioSettings::getEnhanceParam(void)
{
	return getImpl().enhanceParam;
}

float OctConfig::AngioSettings::getBiasFieldSigma(void)
{
	return getImpl().biasFieldSigma;
}

void OctConfig::AngioSettings::setDecorThreshold(float value)
{
	getImpl().decorThreshold = min(max(value, 0.0f), 1.0f);
}

void OctConfig::AngioSettings::setAverageOffset(int value)
{
	getImpl().averageOffset = min(max(value, 0), 20);
}

void OctConfig::AngioSettings::setMotionThreshold(float value)
{
	getImpl().motionThreshold = min(max(value, 0.0f), 5.0f);
}

void OctConfig::AngioSettings::setMotionOverPoints(float value)
{
	getImpl().motionOverPoints = min(max(value, 0.0f), 1.0f);
}

void OctConfig::AngioSettings::setMotionDistRatio(float value)
{
	getImpl().motionDistRatio = min(max(value, 0.0f), 10.0f);
}

void OctConfig::AngioSettings::setFilterOrients(int value)
{
	getImpl().filterOrients = min(max(value, 0), 32);
}

void OctConfig::AngioSettings::setFilterSigma(float value)
{
	getImpl().filterSigma = min(max(value, 0.1f), 10.0f);
}

void OctConfig::AngioSettings::setFilterDivider(float value)
{
	getImpl().filterDivider = min(max(value, 0.1f), 10.0f);
}

void OctConfig::AngioSettings::setFilterWeight(float value)
{
	getImpl().filterWeight = min(max(value, 0.0f), 1.0f);
}

void OctConfig::AngioSettings::setEnhanceParam(float value)
{
	getImpl().enhanceParam = min(max(value, 0.1f), 10.0f);
}

void OctConfig::AngioSettings::setBiasFieldSigma(float value)
{
	getImpl().biasFieldSigma = min(max(value, 0.0f), 100.0f);
}