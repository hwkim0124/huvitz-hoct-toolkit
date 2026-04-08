#include "stdafx.h"
#include "NormDiscInfo.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;


struct NormDiscInfo::NormDiscInfoImpl
{
	std::map<std::string, DiscInfo_Data> asianMaleData;
	std::map<std::string, DiscInfo_Data> asianFemaleData;
	std::map<std::string, DiscInfo_Data> cocasMaleData;
	std::map<std::string, DiscInfo_Data> cocasFemaleData;
	std::map<std::string, DiscInfo_Data> latinoMaleData;
	std::map<std::string, DiscInfo_Data> latinoFemaleData;
	std::map<std::string, DiscInfo_Data> mixedMaleData;
	std::map<std::string, DiscInfo_Data> mixedFemaleData;

	NormDiscInfoImpl()
	{
	}
};


NormDiscInfo::NormDiscInfo() : d_ptr(make_unique<NormDiscInfoImpl>())
{
}


NormDiscInfo::~NormDiscInfo()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormDiscInfo::NormDiscInfo(NormDiscInfo && rhs) = default;
NormDiscInfo & NormData::NormDiscInfo::operator=(NormDiscInfo && rhs) = default;

void NormData::NormDiscInfo::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[DiscInfo_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, DiscInfo_Data(inclin, inters) });
	return;
}


void NormData::NormDiscInfo::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, DiscInfo_Data(inclin, inter, stddev) });
	return;
}


int NormData::NormDiscInfo::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto sectStr = std::string(getSectorStr(side, sector));

	auto data = dataMap[sectStr];
	int percentiles[DiscInfo_Data::N_VALUES + 1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < DiscInfo_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);
		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[DiscInfo_Data::N_VALUES];
}


const char * NormData::NormDiscInfo::getSectorStr(EyeSide side, int sector)
{
	switch (sector) {
	case 0:
		return "R_H";
	case 1:
		return "R_V";
	case 2:
		return "R_A"; // side == EyeSide::OD ? "I_N" : "I_T";
	case 3:
		return "RIM";
	case 4:
		return "DISC"; // side == EyeSide::OD ? "I_T" : "I_N";
	case 5:
		return "CUP";
	}
	return "CUP";
}


float NormData::NormDiscInfo::getNormValue(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0.0f;
	}

	auto sectStr = std::string(getSectorStr(side, sector));
	auto data = dataMap[sectStr];

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);
	float norm = (float)boost::math::quantile(dist, percentile * 0.01f);
	return norm;
}


std::map<std::string, DiscInfo_Data>& NormData::NormDiscInfo::getDataMap(Ethinicity race, Gender gender)
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


NormDiscInfo::NormDiscInfoImpl & NormData::NormDiscInfo::getImpl(void) const
{
	return *d_ptr;
}
