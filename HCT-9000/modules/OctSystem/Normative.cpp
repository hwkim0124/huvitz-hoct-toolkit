#include "stdafx.h"
#include "Normative.h"

#include "CppUtil2.h"
#include "NormData2.h"

using namespace OctSystem;
using namespace NormData;
using namespace CppUtil;
using namespace std;


struct Normative::NormativeImpl
{
	NormArchive normDB;

};


// Direct initialization of static smart pointer.
std::unique_ptr<Normative::NormativeImpl> Normative::d_ptr(new NormativeImpl());

std::vector<float> Normative::TSNIT = {};

Normative::Normative()
{
}


Normative::~Normative()
{
}


bool OctSystem::Normative::initNormative(void)
{
	auto path = getDataFilePath();
	if (getArchive().openDatabase(path.c_str())) {
		LogD() << "Normative DB opened, " << getArchive().getDBFilePath();
		/*
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 0, 192.0f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 1, 309.0f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 2, 304.0f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 3, 321.0f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 4, 304.1f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 5, 282.7f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 6, 305.0f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 7, 269.3f);
		LogD() << "ETDRS: " << getPercentile_ETDRS(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 8, 269.6f);
		LogD() << "GCC: " << getPercentile_GCC(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 0, 110.0f);

		auto graph = getGraph_TSNIT(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 5, 128);

		auto data = vector<float>(512*96, 100);
		auto image = getDeviation_MacularIPL(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, data, 96, 512, 512, 96);

		LogD() << "Disc Info: " << getPercentile_DiscInfo(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 0, 0.25f);
		LogD() << "Disc Info: " << getNormValue_DiscInfo(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 0, 5);
		LogD() << "Disc Info: " << getNormValue_DiscInfo(Ethinicity::COCASIAN, Gender::MALE, 40, EyeSide::OD, 0, 95);
		*/
		return true;
	}
	else {
		LogE() << "Normative DB open failed, " << getArchive().getDBFilePath();
	}
	return false;
}


bool OctSystem::Normative::isAvailabel(void)
{
	return getArchive().isFetched();
}

void OctSystem::Normative::setTsnit(const std::vector<float>& values) {
	TSNIT = values;
	getArchive().getQuadNFL().setTsnitData(values);
	getArchive().getClockNFL().setTsnitData(values);
}

void OctSystem::Normative::setTsnitNorm(const std::vector<float>& values95, const std::vector<float>& values5, const std::vector<float>& values1)
{
	getArchive().getQuadNFL().setTsnitData95(values95);
	getArchive().getQuadNFL().setTsnitData5(values5);
	getArchive().getQuadNFL().setTsnitData1(values1);

	getArchive().getClockNFL().setTsnitData95(values95);
	getArchive().getClockNFL().setTsnitData5(values5);
	getArchive().getClockNFL().setTsnitData1(values1);
}

const std::vector<float>& OctSystem::Normative::getTsnit() {
    return TSNIT;
}

std::string OctSystem::Normative::getDataFilePath(void)
{
	const char* NORM_DB_FILE_NAME = "ndb.db";

	std::string path;
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	path = wtoa(buffer);
	path += "\\";
	path += NORM_DB_FILE_NAME;
	return path;
}


int OctSystem::Normative::getPercentile_ETDRS(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getETDRS().getPercentile(race, gender, age, side, sector, value);
	return result;
}


int OctSystem::Normative::getPercentile_GCC(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getGCC().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::Normative::getPercentile_BisectGCC(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getBisectGCC().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::Normative::getPercentile_QuadNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getQuadNFL().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::Normative::getPercentile_QuadNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value,
	bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	auto tsnitData = getTsnit();

	if (tsnitData.empty()) {
		return getPercentile_QuadNFL(race, gender, age, side, sector, value);
	}

	auto tsnitPercentiles = getTsnitPercentiles(tsnitData, race, gender, age,
		isCompareType ? realEyeSide : side);

	return calculateWeightedRegionPercentile(0, sector, tsnitPercentiles,
		isCompareType, realEyeSide, isMacularDisc, dataIndex);
}

int OctSystem::Normative::getPercentile_ClockNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value,
	bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	auto tsnitData = getTsnit();
	if (tsnitData.empty()) {
		return getPercentile_ClockNFL(race, gender, age, side, sector, value);
	}

	auto tsnitPercentiles = getTsnitPercentiles(tsnitData, race, gender, age,
		isCompareType ? realEyeSide : side);

	return calculateWeightedRegionPercentile(1, sector, tsnitPercentiles,
		isCompareType, realEyeSide, isMacularDisc, dataIndex);
}


std::vector<float> OctSystem::Normative::getTsnitPercentiles(const std::vector<float>& tsnitThickness,
	Ethinicity race, Gender gender, int age, EyeSide side)
{
	std::vector<float> percentiles(tsnitThickness.size());

	std::vector<float> normal95 = getGraph_TSNIT(race, gender, age, side, 95, tsnitThickness.size());
	std::vector<float> border5 = getGraph_TSNIT(race, gender, age, side, 5, tsnitThickness.size());
	std::vector<float> outside1 = getGraph_TSNIT(race, gender, age, side, 1, tsnitThickness.size());

	for (size_t i = 0; i < tsnitThickness.size(); i++) {
		float thickness = tsnitThickness[i];

		if (i < normal95.size() && i < border5.size() && i < outside1.size()) {
			float norm95 = normal95[i];   // 95%
			float norm5 = border5[i];     // 5%
			float norm1 = outside1[i];    // 1%

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
		else {
			percentiles[i] = 0.50f;
		}
	}

	return percentiles;
}

int OctSystem::Normative::calculateWeightedRegionPercentile(int regionType, int regionIndex,
	const std::vector<float>& tsnitPercentiles, bool isCompareType,
	EyeSide realEyeSide, bool isMacularDisc, int dataIndex)
{
	bool needsEyeSideConversion = false;
	if (isCompareType && ((int)realEyeSide - 1 != dataIndex)) {
		needsEyeSideConversion = true;
	}

	std::vector<int> tsnitIndices = getRegionTsnitIndices(regionType, regionIndex, needsEyeSideConversion);

	if (tsnitIndices.empty()) {
		return 50;
	}

	// Count colors in this region
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
	//LogI() << "regionIndex : " << regionIndex << "  " << "white : " << whiteCount << "  " << "green : " << greenCount << "  " << "yellow : " << yellowCount << "  " << "red : " << redCount << "  " << "total : " << totalCount;

	float redRatio = (float)redCount / totalCount;
	float abnormalRatio = (float)(redCount + yellowCount) / totalCount;

	return colorToPercentileValue(redRatio, abnormalRatio, whiteCount, greenCount);
}

std::vector<int> OctSystem::Normative::getRegionTsnitIndices(int regionType, int regionIndex, bool needsEyeSideConversion)
{
	std::vector<int> indices;

	if (regionType == 0) {
		// Quad chart
		// 0=Superior, 1=Nasal, 2=Inferior, 3=Temporal
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
	}
	else if (regionType == 1) {
		// Clock chart
		// OD/OS conversion: 0,1,2,...,11 -> 0,11,10,9,...,1

		int actualIndex = regionIndex;
		if (needsEyeSideConversion) {
			if (regionIndex == 0) {
				actualIndex = 0;
			}
			else {
				actualIndex = 12 - regionIndex; // 1 becomes 11, 2 becomes 10, etc.
			}
			if (actualIndex >= 12) actualIndex = 11;
		}

		switch (actualIndex) {
		case 0: // 53-74 (22 points)
			for (int i = 53; i <= 74; i++) indices.push_back(i % 256);
			break;
		case 1: // 75-95 (21 points)
			for (int i = 75; i <= 95; i++) indices.push_back(i % 256);
			break;
		case 2: // 96-116 (21 points)
			for (int i = 96; i <= 116; i++) indices.push_back(i % 256);
			break;
		case 3: // 117-138 (22 points)
			for (int i = 117; i <= 138; i++) indices.push_back(i % 256);
			break;
		case 4: // 139-159 (21 points)
			for (int i = 139; i <= 159; i++) indices.push_back(i % 256);
			break;
		case 5: // 160-180 (21 points)
			for (int i = 160; i <= 180; i++) indices.push_back(i % 256);
			break;
		case 6: // 181-202 (22 points)
			for (int i = 181; i <= 202; i++) indices.push_back(i % 256);
			break;
		case 7: // 203-223 (21 points)
			for (int i = 203; i <= 223; i++) indices.push_back(i % 256);
			break;
		case 8: // 224-244 (21 points)
			for (int i = 224; i <= 244; i++) indices.push_back(i % 256);
			break;
		case 9: // 245-255 + 0-10 (22 points)
			for (int i = 245; i <= 255; i++) indices.push_back(i % 256);
			for (int i = 0; i <= 10; i++) indices.push_back(i % 256);
			break;
		case 10: // 11-31 (21 points)
			for (int i = 11; i <= 31; i++) indices.push_back(i % 256);
			break;
		case 11: // 32-52 (21 points)
			for (int i = 32; i <= 52; i++) indices.push_back(i % 256);
			break;
		}
	}

	return indices;
}

int OctSystem::Normative::colorToPercentileValue(float redRatio, float abnormalRatio, int whiteCount, int greenCount)
{
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


int OctSystem::Normative::getPercentile_QuadRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getQuadRPE().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::Normative::getPercentile_ClockNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getClockNFL().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::Normative::getPercentile_ClockRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getClockRPE().getPercentile(race, gender, age, side, sector, value);
	return result;
}


int OctSystem::Normative::getPercentile_DiscInfo(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	try {
		auto result = getArchive().getDiscInfo().getPercentile(race, gender, age, side, sector, value);
		return result;
	}
	catch (...) {
		return 0;
	}
}


std::vector<float> OctSystem::Normative::getGraph_TSNIT(Ethinicity race, Gender gender, int age, EyeSide side, int percentile, int dataSize, int filter)
{
	auto result = getArchive().getTSNIT().getNormGraph(race, gender, age, side, percentile, dataSize, filter);
	return result;
}


CppUtil::CvImage OctSystem::Normative::getDeviation_DiscNFL(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getDiscNFL().makeImage(race, gender, age, side, data, lines, points, width, height);
	return result;
}


CppUtil::CvImage OctSystem::Normative::getDeviation_DiscRPE(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getDiscRPE().makeImage(race, gender, age, side, data, lines, points, width, height);
	return result;
}


CppUtil::CvImage OctSystem::Normative::getDeviation_MacularIPL(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getMacularIPL().makeImage(race, gender, age, side, data, lines, points, width, height);
	return result;
}


CppUtil::CvImage OctSystem::Normative::getDeviation_MacularRPE(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getMacularRPE().makeImage(race, gender, age, side, data, lines, points, width, height);
	return result;
}

float OctSystem::Normative::getNormThickness_MacularIPL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getMacularIPL().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::Normative::getNormThickness_MacularRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getMacularRPE().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::Normative::getNormThickness_DiscRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getDiscRPE().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::Normative::getNormThickness_DiscNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getDiscNFL().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::Normative::getNormThickness_RNFLAverage(Ethinicity race, Gender gender, int age, EyeSide side, int percentile)
{
	auto result = getArchive().getRNFLAverage().getNormThickness(race, gender, age, side, percentile);
	return result;
}

float OctSystem::Normative::getNormThickness_RNFLSymmetry(Ethinicity race, Gender gender, int age, EyeSide side, int percentile)
{
	auto result = getArchive().getRNFLSymmetry().getNormThickness(race, gender, age, side, percentile);
	return result;
}


float OctSystem::Normative::getNormValue_DiscInfo(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	try {
		auto result = getArchive().getDiscInfo().getNormValue(race, gender, age, side, sector, percentile);
		return result;
	}
	catch (...) {
		return 0.0f;
	}
}


Normative::NormativeImpl & OctSystem::Normative::getImpl(void)
{
	return *d_ptr;
}


NormData::NormArchive & OctSystem::Normative::getArchive(void)
{
	return getImpl().normDB;
}
