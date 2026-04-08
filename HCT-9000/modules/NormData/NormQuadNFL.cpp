#include "stdafx.h"
#include "NormQuadNFL.h"
#include "DiscTestConfig.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;




struct NormQuadNFL::NormQuadNFLImpl
{
	std::map<std::string, QuadNFL_Data> asianMaleData;
	std::map<std::string, QuadNFL_Data> asianFemaleData;
	std::map<std::string, QuadNFL_Data> cocasMaleData;
	std::map<std::string, QuadNFL_Data> cocasFemaleData;
	std::map<std::string, QuadNFL_Data> latinoMaleData;
	std::map<std::string, QuadNFL_Data> latinoFemaleData;
	std::map<std::string, QuadNFL_Data> mixedMaleData;
	std::map<std::string, QuadNFL_Data> mixedFemaleData;

	NormQuadNFLImpl()
	{
	}
};


NormQuadNFL::NormQuadNFL() : d_ptr(make_unique<NormQuadNFLImpl>())
{
}


NormQuadNFL::~NormQuadNFL()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormQuadNFL::NormQuadNFL(NormQuadNFL && rhs) = default;
NormQuadNFL & NormData::NormQuadNFL::operator=(NormQuadNFL && rhs) = default;


void NormData::NormQuadNFL::insertData(Ethinicity race, Gender gender, const char * sector, float inclin,
	float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[QuadNFL_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, QuadNFL_Data(inclin, inters) });
	return;
}


void NormData::NormQuadNFL::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, QuadNFL_Data(inclin, inter, stddev) });
	return;
}


int NormData::NormQuadNFL::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	if (DiscTest::IsEnabled()) {
		auto tsnitData = s_quadTsnitData;
		if (!tsnitData.empty()) {
			return getPercentileWithTsnit(race, gender, age, side, sector, value,
				false, side, false, (int)side - 1);
		}
	}

	// Original
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto sectStr = std::string(getSectorStr(side, sector));

	auto data = dataMap[sectStr];
	int percentiles[QuadNFL_Data::N_VALUES + 1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < QuadNFL_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);

		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[QuadNFL_Data::N_VALUES];
}


const char* NormData::NormQuadNFL::getSectorStr(EyeSide side, int sector)
{
	switch (sector) {
	case 0:
		return "SUP";
	case 1:
		return "NAS"; // side == EyeSide::OD ? "NAS" : "TEM";
	case 2:
		return "INF";
	case 3:
		return "TEM"; // side == EyeSide::OD ? "TEM" : "NAS";
	}
	return "SUP";
}


std::map<std::string, QuadNFL_Data>& NormData::NormQuadNFL::getDataMap(Ethinicity race, Gender gender)
{
	if (race == Ethinicity::ASIAN) {
		if (gender == Gender::MALE) {
			return getImpl().asianMaleData;
		}
		else {
			return getImpl().asianFemaleData;
		}
	}
	else if (race == Ethinicity::COCASIAN) {
		if (gender == Gender::MALE) {
			return getImpl().cocasMaleData;
		}
		else {
			return getImpl().cocasFemaleData;
		}
	}
	else if (race == Ethinicity::LATINO) {
		if (gender == Gender::MALE) {
			return getImpl().latinoMaleData;
		}
		else {
			return getImpl().latinoFemaleData;
		}
	}
	else {
		if (gender == Gender::MALE) {
			return getImpl().mixedMaleData;
		}
		else {
			return getImpl().mixedFemaleData;
		}
	}
}


NormQuadNFL::NormQuadNFLImpl & NormData::NormQuadNFL::getImpl(void) const
{
	return *d_ptr;
}

std::vector<float> NormData::NormQuadNFL::s_quadTsnitData;
std::vector<float> NormData::NormQuadNFL::s_quadTsnitData95;
std::vector<float> NormData::NormQuadNFL::s_quadTsnitData5;
std::vector<float> NormData::NormQuadNFL::s_quadTsnitData1;

void NormData::NormQuadNFL::setTsnitData(const std::vector<float>& tsnitData)
{
	s_quadTsnitData = tsnitData;
}

void NormData::NormQuadNFL::setTsnitData95(const std::vector<float>& tsnitData)
{
	s_quadTsnitData95 = tsnitData;
}

void NormData::NormQuadNFL::setTsnitData5(const std::vector<float>& tsnitData)
{
	s_quadTsnitData5 = tsnitData;
}

void NormData::NormQuadNFL::setTsnitData1(const std::vector<float>& tsnitData)
{
	s_quadTsnitData1 = tsnitData;
}

const std::vector<float>& NormData::NormQuadNFL::getTsnitData()
{
	return s_quadTsnitData;
}

void NormData::NormQuadNFL::clearTsnitData()
{
	s_quadTsnitData.clear();
}

void NormData::NormQuadNFL::clearTsnitDataAll()
{
	s_quadTsnitData.clear();
	s_quadTsnitData1.clear();
	s_quadTsnitData5.clear();
	s_quadTsnitData95.clear();
}

std::vector<float> NormData::NormQuadNFL::calculateTsnitPercentiles(const std::vector<float>& tsnitThickness,
	Ethinicity race, Gender gender, int age, EyeSide side)
{
	std::vector<float> percentiles(tsnitThickness.size());

	for (size_t i = 0; i < tsnitThickness.size(); i++) {
		float thickness = tsnitThickness[i];
		float norm95 = s_quadTsnitData95[i];   // 95%
		float norm5 = s_quadTsnitData5[i];     // 5%
		float norm1 = s_quadTsnitData1[i];    // 1%

		if (thickness >= norm95) {
			percentiles[i] = 0.97f;   // white
		}
		else if (thickness >= norm5) {
			percentiles[i] = 0.50f;   // green
		}
		else if (thickness >= norm1) {
			percentiles[i] = 0.03f;   // yellow
		}
		else {
			percentiles[i] = 0.005f;  // red
		}
	}

	return percentiles;
}

int NormData::NormQuadNFL::calculateWeightedRegionPercentile(int sector, const std::vector<float>& tsnitPercentiles,
	bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	bool needsEyeSideConversion = false;
	if (isCompareType && ((int)realEyeSide - 1 != dataIndex)) {
		needsEyeSideConversion = true;
	}

	std::vector<int> tsnitIndices = getQuadTsnitIndices(sector, needsEyeSideConversion);

	if (tsnitIndices.empty()) {
		return 50;
	}

	int whiteCount = 0;  // (>=0.95)
	int greenCount = 0;  // (0.05~0.95)
	int yellowCount = 0; // (0.01~0.05)
	int redCount = 0;    // (<0.01)

	for (int idx : tsnitIndices) {
		if (idx >= 0 && idx < (int)tsnitPercentiles.size()) {
			float percentile = tsnitPercentiles[idx];

			if (percentile >= 0.95f) {
				whiteCount++;
			}
			else if (percentile >= 0.05f) {
				greenCount++;
			}
			else if (percentile >= 0.01f) {
				yellowCount++;
			}
			else {
				redCount++;
			}
		}
	}

	int totalCount = tsnitIndices.size();
	float redRatio = (float)redCount / totalCount;
	float abnormalRatio = (float)(redCount + yellowCount) / totalCount;

	if (redRatio >= 0.1f) {
		return 1; // Red -> <1%
	}
	else if (abnormalRatio >= 0.15f) {
		return 3; // Yellow -> ~3%
	}
	else if (whiteCount >= greenCount) {
		return 99; // White -> >95%
	}
	else {
		return 50; // Green -> Normal range
	}
}

std::vector<int> NormData::NormQuadNFL::getQuadTsnitIndices(int regionIndex, bool needsEyeSideConversion)
{
	std::vector<int> indices;

	// Quad chart: 0=Superior, 1=Nasal, 2=Inferior, 3=Temporal
	// OD/OS conversion: 0,1,2,3 -> 0,3,2,1
	int actualIndex = regionIndex;
	if (needsEyeSideConversion) {
		switch (regionIndex) {
		case 0: actualIndex = 0; break;
		case 1: actualIndex = 3; break;
		case 2: actualIndex = 2; break;
		case 3: actualIndex = 1; break;
		}
	}

	switch (actualIndex) {
	case 0: // Superior 32-95
		for (int i = 32; i <= 95; i++) indices.push_back(i % 256);
		break;
	case 1: // Nasal 96-159
		for (int i = 96; i <= 159; i++) indices.push_back(i % 256);
		break;
	case 2: // Inferior 160-223
		for (int i = 160; i <= 223; i++) indices.push_back(i % 256);
		break;
	case 3: // Temporal 224-255 + 0-31
		for (int i = 224; i <= 255; i++) indices.push_back(i % 256);
		for (int i = 0; i <= 31; i++) indices.push_back(i % 256);
		break;
	}

	return indices;
}

int NormData::NormQuadNFL::getPercentileWithTsnit(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value,
	bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	auto tsnitPercentiles = calculateTsnitPercentiles(s_quadTsnitData, race, gender, age,
		isCompareType ? realEyeSide : side);

	return calculateWeightedRegionPercentile(sector, tsnitPercentiles,
		isCompareType, realEyeSide, isMacularDisc, dataIndex);
}
