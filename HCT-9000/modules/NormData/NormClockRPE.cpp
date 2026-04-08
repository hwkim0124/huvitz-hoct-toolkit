#include "stdafx.h"
#include "NormClockRPE.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;




struct NormClockRPE::NormClockRPEImpl
{
	std::map<std::string, ClockRPE_Data> asianMaleData;
	std::map<std::string, ClockRPE_Data> asianFemaleData;
	std::map<std::string, ClockRPE_Data> cocasMaleData;
	std::map<std::string, ClockRPE_Data> cocasFemaleData;
	std::map<std::string, ClockRPE_Data> latinoMaleData;
	std::map<std::string, ClockRPE_Data> latinoFemaleData;
	std::map<std::string, ClockRPE_Data> mixedMaleData;
	std::map<std::string, ClockRPE_Data> mixedFemaleData;

	NormClockRPEImpl()
	{
	}
};


NormClockRPE::NormClockRPE() : d_ptr(make_unique<NormClockRPEImpl>())
{
}


NormClockRPE::~NormClockRPE()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormClockRPE::NormClockRPE(NormClockRPE && rhs) = default;
NormClockRPE & NormData::NormClockRPE::operator=(NormClockRPE && rhs) = default;


void NormData::NormClockRPE::insertData(Ethinicity race, Gender gender, const char * sector, float inclin,
	float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[ClockRPE_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, ClockRPE_Data(inclin, inters) });
	return;
}


void NormData::NormClockRPE::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, ClockRPE_Data(inclin, inter, stddev) });
	return;
}


int NormData::NormClockRPE::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto sectStr = std::string(getSectorStr(side, sector));

	auto data = dataMap[sectStr];
	int percentiles[ClockRPE_Data::N_VALUES+1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < ClockRPE_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);
		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[ClockRPE_Data::N_VALUES];
}


const char* NormData::NormClockRPE::getSectorStr(EyeSide side, int sector)
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


std::map<std::string, ClockRPE_Data>& NormData::NormClockRPE::getDataMap(Ethinicity race, Gender gender)
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


NormClockRPE::NormClockRPEImpl & NormData::NormClockRPE::getImpl(void) const
{
	return *d_ptr;
}
