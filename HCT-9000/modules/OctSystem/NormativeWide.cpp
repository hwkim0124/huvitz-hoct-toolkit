#include "stdafx.h"
#include "NormativeWide.h"

#include "CppUtil2.h"
#include "NormData2.h"

using namespace OctSystem;
using namespace NormData;
using namespace CppUtil;
using namespace std;


struct NormativeWide::NormativeWideImpl
{
	NormArchive normDB;

};


// Direct initialization of static smart pointer.
std::unique_ptr<NormativeWide::NormativeWideImpl> NormativeWide::d_ptr(new NormativeWideImpl());


NormativeWide::NormativeWide()
{
}


NormativeWide::~NormativeWide()
{
}


bool OctSystem::NormativeWide::initNormative(void)
{
	auto path = getDataFilePath();
	bool isWide = true;
	if (getArchive().openDatabase(path.c_str(), isWide)) {
		LogD() << "Normative-Wide DB opened, " << getArchive().getDBFilePath();
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
		LogE() << "Normative-Wide DB open failed, " << getArchive().getDBFilePath();
	}
	return false;
}


bool OctSystem::NormativeWide::isAvailabel(void)
{
	return getArchive().isFetched();
}

std::string OctSystem::NormativeWide::getDataFilePath(void)
{
	const char* NORM_DB_FILE_NAME = "ndbw.db";

	std::string path;
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	path = wtoa(buffer);
	path += "\\";
	path += NORM_DB_FILE_NAME;
	return path;
}


int OctSystem::NormativeWide::getPercentile_ETDRS(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getETDRS().getPercentile(race, gender, age, side, sector, value);
	return result;
}


int OctSystem::NormativeWide::getPercentile_GCC(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getGCC().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::NormativeWide::getPercentile_BisectGCC(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getBisectGCC().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::NormativeWide::getPercentile_QuadNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getQuadNFL().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::NormativeWide::getPercentile_QuadRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getQuadRPE().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::NormativeWide::getPercentile_ClockNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getClockNFL().getPercentile(race, gender, age, side, sector, value);
	return result;
}

int OctSystem::NormativeWide::getPercentile_ClockRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	auto result = getArchive().getClockRPE().getPercentile(race, gender, age, side, sector, value);
	return result;
}


int OctSystem::NormativeWide::getPercentile_DiscInfo(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value)
{
	try {
		auto result = getArchive().getDiscInfo().getPercentile(race, gender, age, side, sector, value);
		return result;
	}
	catch (...) {
		return 0;
	}
}


std::vector<float> OctSystem::NormativeWide::getGraph_TSNIT(Ethinicity race, Gender gender, int age, EyeSide side, int percentile, int dataSize, int filter)
{
	auto result = getArchive().getTSNIT().getNormGraph(race, gender, age, side, percentile, dataSize, filter);
	return result;
}


CppUtil::CvImage OctSystem::NormativeWide::getDeviation_DiscNFL(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getDiscNFL().makeImage(race, gender, age, side, data, lines, points, width, height, true);
	return result;
}


CppUtil::CvImage OctSystem::NormativeWide::getDeviation_DiscRPE(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getDiscRPE().makeImage(race, gender, age, side, data, lines, points, width, height, true);
	return result;
}


CppUtil::CvImage OctSystem::NormativeWide::getDeviation_MacularIPL(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getMacularIPL().makeImage(race, gender, age, side, data, lines, points, width, height, true);
	return result;
}


CppUtil::CvImage OctSystem::NormativeWide::getDeviation_MacularRPE(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height)
{
	auto result = getArchive().getMacularRPE().makeImage(race, gender, age, side, data, lines, points, width, height, true);
	return result;
}

float OctSystem::NormativeWide::getNormThickness_MacularIPL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getMacularIPL().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::NormativeWide::getNormThickness_MacularRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getMacularRPE().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::NormativeWide::getNormThickness_DiscRPE(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getDiscRPE().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::NormativeWide::getNormThickness_DiscNFL(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	auto result = getArchive().getDiscNFL().getNormThickness(race, gender, age, side, sector, percentile);
	return result;
}

float OctSystem::NormativeWide::getNormThickness_RNFLAverage(Ethinicity race, Gender gender, int age, EyeSide side, int percentile)
{
	auto result = getArchive().getRNFLAverage().getNormThickness(race, gender, age, side, percentile);
	return result;
}

float OctSystem::NormativeWide::getNormThickness_RNFLSymmetry(Ethinicity race, Gender gender, int age, EyeSide side, int percentile)
{
	auto result = getArchive().getRNFLSymmetry().getNormThickness(race, gender, age, side, percentile);
	return result;
}


float OctSystem::NormativeWide::getNormValue_DiscInfo(Ethinicity race, Gender gender, int age, EyeSide side, int sector, int percentile)
{
	try {
		auto result = getArchive().getDiscInfo().getNormValue(race, gender, age, side, sector, percentile);
		return result;
	}
	catch (...) {
		return 0.0f;
	}
}


NormativeWide::NormativeWideImpl & OctSystem::NormativeWide::getImpl(void)
{
	return *d_ptr;
}


NormData::NormArchive & OctSystem::NormativeWide::getArchive(void)
{
	return getImpl().normDB;
}
