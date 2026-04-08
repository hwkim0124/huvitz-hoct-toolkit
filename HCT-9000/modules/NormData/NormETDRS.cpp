#include "stdafx.h"
#include "NormETDRS.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;




struct NormETDRS::NormETDRSImpl
{
	std::map<std::string, ETDRS_Data> asianMaleData;
	std::map<std::string, ETDRS_Data> asianFemaleData;
	std::map<std::string, ETDRS_Data> cocasMaleData;
	std::map<std::string, ETDRS_Data> cocasFemaleData;
	std::map<std::string, ETDRS_Data> latinoMaleData;
	std::map<std::string, ETDRS_Data> latinoFemaleData;
	std::map<std::string, ETDRS_Data> mixedMaleData;
	std::map<std::string, ETDRS_Data> mixedFemaleData;

	NormETDRSImpl()
	{
	}
};


NormETDRS::NormETDRS() : d_ptr(make_unique<NormETDRSImpl>())
{
}


NormETDRS::~NormETDRS()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormETDRS::NormETDRS(NormETDRS && rhs) = default;
NormETDRS & NormData::NormETDRS::operator=(NormETDRS && rhs) = default;


void NormData::NormETDRS::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, 
	float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[ETDRS_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, ETDRS_Data(inclin, inters)});
	return;
}


void NormData::NormETDRS::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, ETDRS_Data(inclin, inter, stddev) });
	return;
}


int NormData::NormETDRS::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto sectStr = std::string(getSectorStr(side, sector));

	auto data = dataMap[sectStr];
	int percentiles[ETDRS_Data::N_VALUES+1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < ETDRS_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);
		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[ETDRS_Data::N_VALUES];
}


const char* NormData::NormETDRS::getSectorStr(EyeSide side, int sector)
{
	switch (sector) {
	case 0:
		return "CEN";
	case 1:
		return "I_S";
	case 2:
		return "I_N"; // side == EyeSide::OD ? "I_N" : "I_T";
	case 3:
		return "I_I";
	case 4:
		return "I_T"; // side == EyeSide::OD ? "I_T" : "I_N";
	case 5:
		return "O_S";
	case 6:
		return "O_N"; // side == EyeSide::OD ? "O_N" : "O_T";
	case 7:
		return "O_I";
	case 8:
		return "O_T"; // side == EyeSide::OD ? "O_T" : "O_N";
	}
	return "CEN";
}


std::map<std::string, ETDRS_Data>& NormData::NormETDRS::getDataMap(Ethinicity race, Gender gender)
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


NormETDRS::NormETDRSImpl & NormData::NormETDRS::getImpl(void) const
{
	return *d_ptr;
}
