#include "stdafx.h"
#include "NormTSNIT.h"


#include <stdio.h>
#include <string>
#include <map>

#include <boost/math/distributions/normal.hpp>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;




struct NormTSNIT::NormTSNITImpl
{
	std::map<int, TSNIT_Data> asianMaleData;
	std::map<int, TSNIT_Data> asianFemaleData;
	std::map<int, TSNIT_Data> cocasMaleData;
	std::map<int, TSNIT_Data> cocasFemaleData;
	std::map<int, TSNIT_Data> latinoMaleData;
	std::map<int, TSNIT_Data> latinoFemaleData;
	std::map<int, TSNIT_Data> mixedMaleData;
	std::map<int, TSNIT_Data> mixedFemaleData;

	NormTSNITImpl()
	{
	}
};


NormTSNIT::NormTSNIT() : d_ptr(make_unique<NormTSNITImpl>())
{
}


NormTSNIT::~NormTSNIT()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormTSNIT::NormTSNIT(NormTSNIT && rhs) = default;
NormTSNIT & NormData::NormTSNIT::operator=(NormTSNIT && rhs) = default;


void NormData::NormTSNIT::insertData(Ethinicity race, Gender gender, int sector, float inclin,
	float inter1, float inter2, float inter3, float inter4, float inter5)
{
	auto& dataMap = getDataMap(race, gender);

	float inters[TSNIT_Data::N_VALUES];
	inters[0] = inter5;
	inters[1] = inter4;
	// inters[2] = inter1;
	inters[2] = inter2;
	inters[3] = inter3;

	dataMap.insert({ sector, TSNIT_Data(inclin, inters) });
	return;
}


void NormData::NormTSNIT::insertData(Ethinicity race, Gender gender, int sector, float inclin, float inter, float stddev)
{
	auto& dataMap = getDataMap(race, gender);
	dataMap.insert({ sector, TSNIT_Data(inclin, inter, stddev) });
	return;
}


std::vector<float> NormData::NormTSNIT::getNormGraph(Ethinicity race, Gender gender, int age, EyeSide side, int percentile, int dataSize, int filter)
{
	auto& dataMap = getDataMap(race, gender);
	if (dataMap.empty()) {
		return vector<float>();
	}

	int index = (percentile <= 1 ? 0 : (percentile <= 5 ? 1 : (percentile <= 95 ? 2 : 3))); //  (percentile <= 99 ? 3 : 4))));

	vector<float> data = vector<float>();
	/*
	if (side == EyeSide::OD) {
		for (auto iter = cbegin(dataMap); iter != cend(dataMap); ++iter) {
			float norm = iter->second.getNormValue(age, index);
			data.push_back(norm);
		}
	}
	else {
		for (auto iter = crbegin(dataMap); iter != crend(dataMap); ++iter) {
			float norm = iter->second.getNormValue(age, index);
			data.push_back(norm);
		}
	}
	*/

	for (auto iter = cbegin(dataMap); iter != cend(dataMap); ++iter) {
		// float norm = iter->second.getNormValue(age, index);
		float mean = iter->second.getMean(age);
		boost::math::normal dist(mean, iter->second.stddev);
		float norm = (float)boost::math::quantile(dist, percentile * 0.01f);

		data.push_back(norm);
	}

	vector<float> graph = vector<float>(dataSize);

	if (dataSize != data.size()) {
		float rate = (float)data.size() / dataSize;
		for (int i = 0; i < dataSize; i++) {
			graph[i] = data[(int)(i * rate)];
		}
	}
	else {
		graph = data;
	}

	if (filter > 1) {
		graph = applyAveraging(graph, filter);
	}

	return graph;
}


std::map<int, TSNIT_Data>& NormData::NormTSNIT::getDataMap(Ethinicity race, Gender gender)
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

std::vector<float> NormData::NormTSNIT::applyAveraging(const std::vector<float>& raws, int filter)
{
	auto data = raws;
	auto dlen = (int)data.size();

	int half = filter / 2;
	int tcnt;
	float tsum;

	for (int i = 0; i < dlen; i++) {
		tsum = 0.0f; tcnt = 0;
		for (int j = i - half, k = 0; j <= i + half; j++) {
			if (j < 0) {
				k = dlen + j;
			}
			else if (j >= dlen) {
				k = j - dlen;
			}
			else {
				k = j;
			}
			tsum += raws[k];
			tcnt += 1;
		}

		if (tcnt > 0) {
			data[i] = tsum / tcnt;
		}
	}

	return data;
}


NormTSNIT::NormTSNITImpl & NormData::NormTSNIT::getImpl(void) const
{
	return *d_ptr;
}
