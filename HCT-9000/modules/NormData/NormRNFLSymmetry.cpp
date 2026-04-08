#include "stdafx.h"
#include "NormRNFLSymmetry.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;


struct NormRNFLSymmetry::NormRNFLSymmetryImpl
{
	std::map<std::string, RNFL_Symmetry_Data> asianMaleData;
	std::map<std::string, RNFL_Symmetry_Data> asianFemaleData;
	std::map<std::string, RNFL_Symmetry_Data> cocasMaleData;
	std::map<std::string, RNFL_Symmetry_Data> cocasFemaleData;
	std::map<std::string, RNFL_Symmetry_Data> latinoMaleData;
	std::map<std::string, RNFL_Symmetry_Data> latinoFemaleData;
	std::map<std::string, RNFL_Symmetry_Data> mixedMaleData;
	std::map<std::string, RNFL_Symmetry_Data> mixedFemaleData;

	NormRNFLSymmetryImpl()
	{
	}
};

NormRNFLSymmetry::NormRNFLSymmetry() : d_ptr(make_unique<NormRNFLSymmetryImpl>())
{
}


NormRNFLSymmetry::~NormRNFLSymmetry()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormRNFLSymmetry::NormRNFLSymmetry(NormRNFLSymmetry && rhs) = default;
NormRNFLSymmetry & NormData::NormRNFLSymmetry::operator=(NormRNFLSymmetry && rhs) = default;


void NormData::NormRNFLSymmetry::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[RNFL_Symmetry_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, RNFL_Symmetry_Data(inclin, inters) });
	return;
}

void NormData::NormRNFLSymmetry::insertData(Ethinicity race, Gender gender, const char * sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, RNFL_Symmetry_Data(inclin, inter, stddev) });
	return;
}

int NormData::NormRNFLSymmetry::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto sectStr = std::string(getSectorStr(side, sector));

	auto data = dataMap[sectStr];
	int percentiles[RNFL_Symmetry_Data::N_VALUES + 1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < RNFL_Symmetry_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);
		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[RNFL_Symmetry_Data::N_VALUES];
}

const char * NormData::NormRNFLSymmetry::getSectorStr(EyeSide side, int sector)
{
	return "CORR";
}

float NormData::NormRNFLSymmetry::getNormThickness(Ethinicity race, Gender gender, int age, EyeSide side, int percentile)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto data = dataMap["CORR"];

	// int index = (percentile <= 1 ? 0 : percentile <= 5 ? 1 : percentile <= 95 ? 2 : percentile <= 99 ? 3 : 4);
	// float norm = data.getNormValue(age, index);
	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);
	float norm = (float)boost::math::quantile(dist, percentile * 0.01f);
	return norm;
}

std::map<std::string, RNFL_Symmetry_Data>& NormData::NormRNFLSymmetry::getDataMap(Ethinicity race, Gender gender)
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

NormRNFLSymmetry::NormRNFLSymmetryImpl & NormData::NormRNFLSymmetry::getImpl(void) const
{
	return *d_ptr;
}
