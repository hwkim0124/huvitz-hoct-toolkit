#include "stdafx.h"
#include "NormClockNFL.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"
#include "DiscTestConfig.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;




struct NormClockNFL::NormClockNFLImpl
{
	std::map<std::string, ClockNFL_Data> asianMaleData;
	std::map<std::string, ClockNFL_Data> asianFemaleData;
	std::map<std::string, ClockNFL_Data> cocasMaleData;
	std::map<std::string, ClockNFL_Data> cocasFemaleData;
	std::map<std::string, ClockNFL_Data> latinoMaleData;
	std::map<std::string, ClockNFL_Data> latinoFemaleData;
	std::map<std::string, ClockNFL_Data> mixedMaleData;
	std::map<std::string, ClockNFL_Data> mixedFemaleData;

	NormClockNFLImpl()
	{
	}
};


NormClockNFL::NormClockNFL() : d_ptr(make_unique<NormClockNFLImpl>())
{
}


NormClockNFL::~NormClockNFL()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormClockNFL::NormClockNFL(NormClockNFL && rhs) = default;
NormClockNFL & NormData::NormClockNFL::operator=(NormClockNFL && rhs) = default;


void NormData::NormClockNFL::insertData(Ethinicity race, Gender gender, const char * sector, float inclin,
	float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[ClockNFL_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, ClockNFL_Data(inclin, inters) });
	return;
}


void NormData::NormClockNFL::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, ClockNFL_Data(inclin, inter, stddev) });
	return;
}


int NormData::NormClockNFL::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	if (DiscTest::IsEnabled()) {
		auto tsnitData = s_clockTsnitData;
		if (!tsnitData.empty()) {
			return getPercentileWithTsnit(race, gender, age, side, sector, value,
				false, side, false, (int)side - 1);
		}
	}

	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto sectStr = std::string(getSectorStr(side, sector));

	auto data = dataMap[sectStr];
	int percentiles[ClockNFL_Data::N_VALUES+1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < ClockNFL_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);
		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[ClockNFL_Data::N_VALUES];
}


const char* NormData::NormClockNFL::getSectorStr(EyeSide side, int sector)
{
	switch (sector) {
	case 0:
		return "SUP";
	case 1:
		return "S_N"; // side == EyeSide::OD ? "S_N" : "S_T";
	case 2:
		return "N_S"; // side == EyeSide::OD ? "N_S" : "T_S";
	case 3:
		return "NAS";
	case 4:
		return "N_I"; // side == EyeSide::OD ? "N_I" : "T_I";
	case 5:
		return "I_N"; // side == EyeSide::OD ? "I_N" : "I_T";
	case 6:
		return "INF";
	case 7:
		return "I_T"; // side == EyeSide::OD ? "I_T" : "I_N";
	case 8:
		return "T_I"; // side == EyeSide::OD ? "T_I" : "N_I";
	case 9:
		return "TEM";
	case 10:
		return "T_S"; // side == EyeSide::OD ? "T_S" : "N_S";
	case 11:
		return "S_T"; // side == EyeSide::OD ? "S_T" : "S_N";
	}
	return "SUP";
}


std::map<std::string, ClockNFL_Data>& NormData::NormClockNFL::getDataMap(Ethinicity race, Gender gender)
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


NormClockNFL::NormClockNFLImpl & NormData::NormClockNFL::getImpl(void) const
{
	return *d_ptr;
}

std::vector<float> NormData::NormClockNFL::s_clockTsnitData;
std::vector<float> NormData::NormClockNFL::s_clockTsnitData95;
std::vector<float> NormData::NormClockNFL::s_clockTsnitData5;
std::vector<float> NormData::NormClockNFL::s_clockTsnitData1;

void NormData::NormClockNFL::setTsnitData(const std::vector<float>& tsnitData)
{
	s_clockTsnitData = tsnitData;
}

void NormData::NormClockNFL::setTsnitData95(const std::vector<float>& tsnitData)
{
	s_clockTsnitData95 = tsnitData;
}

void NormData::NormClockNFL::setTsnitData5(const std::vector<float>& tsnitData)
{
	s_clockTsnitData5 = tsnitData;
}

void NormData::NormClockNFL::setTsnitData1(const std::vector<float>& tsnitData)
{
	s_clockTsnitData1 = tsnitData;
}

const std::vector<float>& NormData::NormClockNFL::getTsnitData()
{
	return s_clockTsnitData;
}

void NormData::NormClockNFL::clearTsnitData()
{
	s_clockTsnitData.clear();
}

void NormData::NormClockNFL::clearTsnitDataAll()
{
	s_clockTsnitData.clear();
	s_clockTsnitData1.clear();
	s_clockTsnitData5.clear();
	s_clockTsnitData95.clear();
}


std::vector<float> NormData::NormClockNFL::calculateTsnitPercentiles(const std::vector<float>& tsnitThickness,
	Ethinicity race, Gender gender, int age, EyeSide side)
{
	std::vector<float> percentiles(tsnitThickness.size());

	for (size_t i = 0; i < tsnitThickness.size(); i++) {
		float thickness = tsnitThickness[i];
		float norm95 = s_clockTsnitData95[i];   // 95%
		float norm5 = s_clockTsnitData5[i];     // 5%
		float norm1 = s_clockTsnitData1[i];    // 1%

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

int NormData::NormClockNFL::calculateWeightedRegionPercentile(int sector, const std::vector<float>& tsnitPercentiles,
	bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	bool needsEyeSideConversion = false;
	if (isCompareType && ((int)realEyeSide - 1 != dataIndex)) {
		needsEyeSideConversion = true;
	}

	std::vector<int> tsnitIndices = getClockTsnitIndices(sector, needsEyeSideConversion);

	if (tsnitIndices.empty()) {
		return 50;
	}

	int whiteCount = 0, greenCount = 0, yellowCount = 0, redCount = 0;

	for (int idx : tsnitIndices) {
		if (idx >= 0 && idx < (int)tsnitPercentiles.size()) {
			float percentile = tsnitPercentiles[idx];
			if (percentile >= 0.95f) whiteCount++;
			else if (percentile >= 0.05f) greenCount++;
			else if (percentile >= 0.01f) yellowCount++;
			else redCount++;
		}
	}

	int totalCount = tsnitIndices.size();
	float redRatio = (float)redCount / totalCount;
	float abnormalRatio = (float)(redCount + yellowCount) / totalCount;

	if (redRatio >= 0.1f) return 1;
	else if (abnormalRatio >= 0.15f) return 3;
	else if (whiteCount >= greenCount) return 99;
	else return 50;
}

std::vector<int> NormData::NormClockNFL::getClockTsnitIndices(int regionIndex, bool needsEyeSideConversion)
{
	std::vector<int> indices;

	int actualIndex = regionIndex;
	if (needsEyeSideConversion) {
		if (regionIndex == 0) actualIndex = 0;
		else actualIndex = 12 - regionIndex;
		if (actualIndex >= 12) actualIndex = 11;
	}

	switch (actualIndex) {
	case 0: for (int i = 53; i <= 74; i++) indices.push_back(i % 256); break;
	case 1: for (int i = 75; i <= 95; i++) indices.push_back(i % 256); break;
	case 2: for (int i = 96; i <= 116; i++) indices.push_back(i % 256); break;
	case 3: for (int i = 117; i <= 138; i++) indices.push_back(i % 256); break;
	case 4: for (int i = 139; i <= 159; i++) indices.push_back(i % 256); break;
	case 5: for (int i = 160; i <= 180; i++) indices.push_back(i % 256); break;
	case 6: for (int i = 181; i <= 202; i++) indices.push_back(i % 256); break;
	case 7: for (int i = 203; i <= 223; i++) indices.push_back(i % 256); break;
	case 8: for (int i = 224; i <= 244; i++) indices.push_back(i % 256); break;
	case 9:
		for (int i = 245; i <= 255; i++) indices.push_back(i % 256);
		for (int i = 0; i <= 10; i++) indices.push_back(i % 256);
		break;
	case 10: for (int i = 11; i <= 31; i++) indices.push_back(i % 256); break;
	case 11: for (int i = 32; i <= 52; i++) indices.push_back(i % 256); break;
	}

	return indices;
}

int NormData::NormClockNFL::getPercentileWithTsnit(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value,
	bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	auto tsnitPercentiles = calculateTsnitPercentiles(s_clockTsnitData, race, gender, age,
		isCompareType ? realEyeSide : side);

	return calculateWeightedRegionPercentile(sector, tsnitPercentiles,
		isCompareType, realEyeSide, isMacularDisc, dataIndex);
}
