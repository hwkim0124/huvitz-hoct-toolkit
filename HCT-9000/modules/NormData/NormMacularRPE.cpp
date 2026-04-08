#include "stdafx.h"
#include "NormMacularRPE.h"

#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;




struct NormMacularRPE::NormMacularRPEImpl
{
	std::map<int, MacularRPE_Data> asianMaleData;
	std::map<int, MacularRPE_Data> asianFemaleData;
	std::map<int, MacularRPE_Data> cocasMaleData;
	std::map<int, MacularRPE_Data> cocasFemaleData;
	std::map<int, MacularRPE_Data> latinoMaleData;
	std::map<int, MacularRPE_Data> latinoFemaleData;
	std::map<int, MacularRPE_Data> mixedMaleData;
	std::map<int, MacularRPE_Data> mixedFemaleData;

	NormMacularRPEImpl()
	{
	}
};


NormMacularRPE::NormMacularRPE() : d_ptr(make_unique<NormMacularRPEImpl>())
{
}


NormMacularRPE::~NormMacularRPE()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormMacularRPE::NormMacularRPE(NormMacularRPE && rhs) = default;
NormMacularRPE & NormData::NormMacularRPE::operator=(NormMacularRPE && rhs) = default;


void NormData::NormMacularRPE::insertData(Ethinicity race, Gender gender, int sector, float inclin,
	float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[MacularRPE_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, MacularRPE_Data(inclin, inters) });
	return;
}


void NormData::NormMacularRPE::insertData(Ethinicity race, Gender gender, int sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, MacularRPE_Data(inclin, inter, stddev) });
	return;
}


int NormData::NormMacularRPE::getPercentile(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto data = dataMap[sector];
	int percentiles[MacularRPE_Data::N_VALUES+1] = { 1, 5, 95, 99, 100 };

	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 0;
	for (; index < MacularRPE_Data::N_VALUES; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, percentiles[index] * 0.01f);

		if (value <= norm) {
			return percentiles[index];
		}
	}
	return percentiles[MacularRPE_Data::N_VALUES];
}


int NormData::NormMacularRPE::getPercentile2(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto data = dataMap[sector];
	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);

	int index = 1;
	for (; index < 100; index++) {
		// float norm = data.getNormValue(age, index);
		float norm = (float)boost::math::quantile(dist, index * 0.01f);
		if (value <= norm) {
			return index;
		}
	}
	return index;
}


float NormData::NormMacularRPE::getNormThickness(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return 0;
	}

	auto data = dataMap[sector];

	// int index = (percentile <= 1 ? 0 : percentile <= 5 ? 1 : percentile <= 95 ? 2 : percentile <= 99 ? 3 : 4);
	// float norm = data.getNormValue(age, index);
	float mean = data.getMean(age);
	boost::math::normal dist(mean, data.stddev);
	float norm = (float)boost::math::quantile(dist, percentile * 0.01f);
	return norm;
}


CppUtil::CvImage NormData::NormMacularRPE::makeImage(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height, bool isWide)
{
	CvImage msrImg, resImg;

	int mapWidth, mapHeight, mapSize;

	if (isWide) {
		mapWidth = WIDE_MACULAR_MAP_WIDTH;
		mapHeight = WIDE_MACULAR_MAP_HEIGHT;
		mapSize = WIDE_MACULAR_MAP_SIZE;
	}
	else {
		mapWidth = MAP_WIDTH;
		mapHeight = MAP_HEIGHT;
		mapSize = MAP_SIZE;
	}

	if (lines * points > 0) {
		msrImg.fromFloat32((const unsigned char*)&data[0], points, lines);
		msrImg.resize(mapWidth, mapHeight);

		auto vect = std::vector<float>(mapSize);
		for (int y = 0; y < mapHeight; y++) {
			for (int x = 0; x < mapWidth; x++) {
				int idx1 = y * mapWidth + (side == EyeSide::OD ? x + 1 : mapWidth - x);
				int idx2 = y * mapWidth + x;
				vect[idx2] = (float)getPercentile2(race, gender, age, side, idx1, msrImg.getAtInFloat(x, y));
			}
		}

		resImg.fromFloat32((const unsigned char*)&vect[0], mapWidth, mapHeight);
		resImg.resize(width, height);
	}

	return resImg;
}


std::map<int, MacularRPE_Data>& NormData::NormMacularRPE::getDataMap(Ethinicity race, Gender gender)
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


NormMacularRPE::NormMacularRPEImpl & NormData::NormMacularRPE::getImpl(void) const
{
	return *d_ptr;
}
