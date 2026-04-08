#include "stdafx.h"
#include "PatternSettings.h"


using namespace OctConfig;
using namespace std;


struct PatternSettings::PatternSettingsImpl
{
	pair<float, float> retinaScales[3];
	pair<float, float> retinaOffsets[3];
	pair<float, float> corneaScales[3];
	pair<float, float> corneaOffsets[3];
	pair<float, float> topographyScales[3];
	pair<float, float> topographyOffsets[3];
	int referRangeLowerSize;
	int referRangeUpperSize;

	PatternSettingsImpl() {
	}
};



PatternSettings::PatternSettings() :
	d_ptr(make_unique<PatternSettingsImpl>())
{
	initialize();
}


OctConfig::PatternSettings::~PatternSettings() = default;
OctConfig::PatternSettings::PatternSettings(PatternSettings && rhs) = default;
PatternSettings & OctConfig::PatternSettings::operator=(PatternSettings && rhs) = default;


OctConfig::PatternSettings::PatternSettings(const PatternSettings & rhs)
	: d_ptr(make_unique<PatternSettingsImpl>(*rhs.d_ptr))
{
}


PatternSettings & OctConfig::PatternSettings::operator=(const PatternSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::PatternSettings::initialize(void)
{
	resetToDefaultValues();
	return;
}


void OctConfig::PatternSettings::resetToDefaultValues(void)
{
	for (int i = 0; i < 3; i++) {
		getImpl().retinaScales[i] = pair<float, float>(1.0f, 1.0f);
		getImpl().corneaScales[i] = pair<float, float>(1.0f, 1.0f);
		getImpl().topographyScales[i] = pair<float, float>(1.0f, 1.0f);
		getImpl().retinaOffsets[i] = pair<float, float>(0.0f, 0.0f);
		getImpl().corneaOffsets[i] = pair<float, float>(0.0f, 0.0f);
		getImpl().topographyOffsets[i] = pair<float, float>(0.0f, 0.0f);
	}

	getImpl().referRangeUpperSize = 10000;
	getImpl().referRangeLowerSize = 28000;
	return;
}

std::pair<float, float>& OctConfig::PatternSettings::retinaPatternScale(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().retinaScales[index];
	}
	return getImpl().retinaScales[0];
}

std::pair<float, float>& OctConfig::PatternSettings::corneaPatternScale(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().corneaScales[index];
	}
	return getImpl().corneaScales[0];
}

std::pair<float, float>& OctConfig::PatternSettings::topographyPatternScale(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().topographyScales[index];
	}
	return getImpl().topographyScales[0];
}

std::pair<float, float>& OctConfig::PatternSettings::retinaPatternOffset(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().retinaOffsets[index];
	}
	return getImpl().retinaOffsets[0];
}

std::pair<float, float>& OctConfig::PatternSettings::corneaPatternOffset(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().corneaOffsets[index];
	}
	return getImpl().corneaOffsets[0];
}

std::pair<float, float>& OctConfig::PatternSettings::topographyPatternOffset(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().topographyOffsets[index];
	}
	return getImpl().topographyOffsets[0];
}

void OctConfig::PatternSettings::setRetinaPatternScale(std::pair<float, float> value, int index)
{
	if (index >= 0 && index < 3) {
		float val1 = min(max(value.first, 0.0f), 10.0f);
		float val2 = min(max(value.second, 0.0f), 10.0f);
		getImpl().retinaScales[index] = std::pair<float, float>(val1, val2);
	}
}

void OctConfig::PatternSettings::setCorneaPatternScale(std::pair<float, float> value, int index)
{
	if (index >= 0 && index < 3) {
		float val1 = min(max(value.first, 0.0f), 10.0f);
		float val2 = min(max(value.second, 0.0f), 10.0f);
		getImpl().corneaScales[index] = std::pair<float, float>(val1, val2);
	}
}

void OctConfig::PatternSettings::setTopographyPatternScale(std::pair<float, float> value, int index)
{
	if (index >= 0 && index < 3) {
		float val1 = min(max(value.first, 0.0f), 10.0f);
		float val2 = min(max(value.second, 0.0f), 10.0f);
		getImpl().topographyScales[index] = std::pair<float, float>(val1, val2);
	}
}

void OctConfig::PatternSettings::setRetinaPatternOffset(std::pair<float, float> value, int index)
{
	if (index >= 0 && index < 3) {
		float val1 = min(max(value.first, -99.0f), 99.0f);
		float val2 = min(max(value.second, -99.0f), 99.0f);
		getImpl().retinaOffsets[index] = std::pair<float, float>(val1, val2);
	}
}

void OctConfig::PatternSettings::setCorneaPatternOffset(std::pair<float, float> value, int index)
{
	if (index >= 0 && index < 3) {
		float val1 = min(max(value.first, -99.0f), 99.0f);
		float val2 = min(max(value.second, -99.0f), 99.0f);
		getImpl().corneaOffsets[index] = std::pair<float, float>(val1, val2);
	}
}

void OctConfig::PatternSettings::setTopographyPatternOffset(std::pair<float, float> value, int index)
{
	if (index >= 0 && index < 3) {
		float val1 = min(max(value.first, -99.0f), 99.0f);
		float val2 = min(max(value.second, -99.0f), 99.0f);
		getImpl().topographyOffsets[index] = std::pair<float, float>(val1, val2);
	}
}

int OctConfig::PatternSettings::getReferenceRangeLowerSize(void)
{
	return getImpl().referRangeLowerSize;
}

int OctConfig::PatternSettings::getReferenceRangeUpperSize(void)
{
	return getImpl().referRangeUpperSize;
}

void OctConfig::PatternSettings::setReferenceRangeLowerSize(int size)
{
	getImpl().referRangeLowerSize = size;
	return;
}

void OctConfig::PatternSettings::setReferenceRangeUpperSize(int size)
{
	getImpl().referRangeUpperSize = size;
	return;
}

PatternSettings::PatternSettingsImpl & OctConfig::PatternSettings::getImpl(void) const
{
	return *d_ptr;
}
