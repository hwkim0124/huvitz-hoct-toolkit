#include "stdafx.h"
#include "NormArchive.h"
#include "Database.h"
#include "NormETDRS.h"
#include "NormGCC.h"
#include "NormClockNFL.h"
#include "NormClockRPE.h"
#include "NormQuadNFL.h"
#include "NormQuadRPE.h"
#include "NormTSNIT.h"
#include "NormDiscNFL.h"
#include "NormDiscRPE.h"
#include "NormDiscInfo.h"
#include "NormMacularIPL.h"
#include "NormMacularRPE.h"
#include "NormBisectGCC.h"
#include "NormRNFLAverage.h"
#include "NormRNFLSymmetry.h"

#include <stdio.h>
#include <string>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;



struct NormArchive::NormArchiveImpl
{
	std::string dbFilePath;
	bool isFetched;

	NormETDRS normETDRS;
	NormGCC normGCC;
	NormClockNFL normClockNFL;
	NormClockRPE normClockRPE;
	NormQuadNFL normQuadNFL;
	NormQuadRPE normQuadRPE;
	NormTSNIT normTSNIT;
	NormDiscNFL normDiscNFL;
	NormDiscRPE normDiscRPE;
	NormDiscInfo normDiscInfo;
	NormMacularIPL normMacularIPL;
	NormMacularRPE normMacularRPE;
	NormBisectGCC normBisectGCC;
	NormRNFLAverage normRNFLAverage;
	NormRNFLSymmetry normRNFLSymmetry;

	NormArchiveImpl() : dbFilePath(""), isFetched(false)
	{
	}
};


NormArchive::NormArchive() : d_ptr(make_unique<NormArchiveImpl>())
{
}


NormArchive::~NormArchive()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormArchive::NormArchive(NormArchive && rhs) = default;
NormArchive & NormData::NormArchive::operator=(NormArchive && rhs) = default;

bool NormData::NormArchive::openDatabase(const char * path, bool isWide)
{
	try
	{
		std::string path2;

		if (path == nullptr) {
			wchar_t buffer[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, buffer);
			path2 = wtoa(buffer);
			path2 += "\\";
			path2 += NORM_DB_FILE_NAME;
		}
		else {
			path2 = path;
		}
		getImpl().dbFilePath = path2;

		if (isWide) {
			if (!fetchETDRS() ||
				!fetchGCC() ||
				!fetchClockNFL() ||
				!fetchClockRPE() ||
				!fetchQuadNFL() ||
				!fetchQuadRPE() ||
				!fetchTSNIT() ||
				!fetchDiscNFL() ||
				//!fetchDiscRPE() ||
				!fetchMacularIPL() ||
				!fetchMacularRPE() ||
				!fetchBisectGCC()) {
			}
			else {
				fetchRNFLAverage();
				fetchRNFLSymmetry();

				fetchDiscInfo();
				getImpl().isFetched = true;
			}
		}
		else {
			if (!fetchETDRS() ||
				!fetchGCC() ||
				!fetchClockNFL() ||
				!fetchClockRPE() ||
				!fetchQuadNFL() ||
				!fetchQuadRPE() ||
				!fetchTSNIT() ||
				!fetchDiscNFL() ||
				!fetchDiscRPE() ||
				!fetchMacularIPL() ||
				!fetchMacularRPE() ||
				!fetchBisectGCC()) {
			}
			else {
				fetchRNFLAverage();
				fetchRNFLSymmetry();

				fetchDiscInfo();
				getImpl().isFetched = true;
			}
		}

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}
	return false;
}


std::string NormData::NormArchive::getDBFilePath(void) const
{
	return getImpl().dbFilePath;
}


bool NormData::NormArchive::isFetched(void) const
{
	return getImpl().isFetched;
}


NormETDRS & NormData::NormArchive::getETDRS(void) const
{
	return getImpl().normETDRS;
}


NormGCC & NormData::NormArchive::getGCC(void) const
{
	return getImpl().normGCC;
}

NormQuadNFL & NormData::NormArchive::getQuadNFL(void) const
{
	return getImpl().normQuadNFL;
}

NormQuadRPE & NormData::NormArchive::getQuadRPE(void) const
{
	return getImpl().normQuadRPE;
}

NormClockNFL & NormData::NormArchive::getClockNFL(void) const
{
	return getImpl().normClockNFL;
}

NormClockRPE & NormData::NormArchive::getClockRPE(void) const
{
	return getImpl().normClockRPE;
}

NormTSNIT & NormData::NormArchive::getTSNIT(void) const
{
	return getImpl().normTSNIT;
}

NormDiscNFL & NormData::NormArchive::getDiscNFL(void) const
{
	return getImpl().normDiscNFL;
}

NormDiscRPE & NormData::NormArchive::getDiscRPE(void) const
{
	return getImpl().normDiscRPE;
}

NormDiscInfo & NormData::NormArchive::getDiscInfo(void) const
{
	return getImpl().normDiscInfo;
}

NormMacularIPL & NormData::NormArchive::getMacularIPL(void) const
{
	return getImpl().normMacularIPL;
}

NormMacularRPE & NormData::NormArchive::getMacularRPE(void) const
{
	return getImpl().normMacularRPE;
}

NormBisectGCC & NormData::NormArchive::getBisectGCC(void) const
{
	return getImpl().normBisectGCC;
}

NormRNFLAverage & NormData::NormArchive::getRNFLAverage(void) const
{
	return getImpl().normRNFLAverage;
}

NormRNFLSymmetry & NormData::NormArchive::getRNFLSymmetry(void) const
{
	return getImpl().normRNFLSymmetry;
}

bool NormData::NormArchive::fetchETDRS(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getETDRS(), db, "ETDRS", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative ETDRS fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchGCC(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getGCC(), db, "GCC", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative GCC fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchQuadNFL(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getQuadNFL(), db, "NFL_Quad", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative NFL Quad fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchQuadRPE(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getQuadRPE(), db, "RPE_Quad", Ethinicity::MIXED, Gender::FEMALE);
		LogD() << "Normative RPE Quad fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchClockNFL(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getClockNFL(), db, "NFL_Clock", Ethinicity::MIXED, Gender::FEMALE);
		LogD() << "Normative NFL Clock fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchClockRPE(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getClockRPE(), db, "RPE_Clock", Ethinicity::MIXED, Gender::FEMALE);
		LogD() << "Normative RPE_Clock fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

bool NormData::NormArchive::fetchTSNIT(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForIntegerSector(getTSNIT(), db, "TSNIT", Ethinicity::MIXED, Gender::FEMALE);
		LogD() << "Normative TSNIT fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

bool NormData::NormArchive::fetchDiscNFL(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForIntegerSector(getDiscNFL(), db, "Disc_Nfl_HOR", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative Disc NFL fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

bool NormData::NormArchive::fetchDiscRPE(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForIntegerSector(getDiscRPE(), db, "Disc_Rpe_HOR", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative Disc RPE fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchDiscInfo(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getDiscInfo(), db, "Disc_Info", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative Disc Info fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchMacularIPL(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForIntegerSector(getMacularIPL(), db, "Macular_Ipl_HOR", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative Macular IPL fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

bool NormData::NormArchive::fetchMacularRPE(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForIntegerSector(getMacularRPE(), db, "Macular_Rpe_HOR", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative Macular RPE fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}


bool NormData::NormArchive::fetchBisectGCC(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getBisectGCC(), db, "GCC_SI", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative GCC SI fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

bool NormData::NormArchive::fetchRNFLAverage(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getRNFLAverage(), db, "RNFL_AVERAGE", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative RNFL Average fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

bool NormData::NormArchive::fetchRNFLSymmetry(void)
{
	try {
		SQLite::Database db(getDBFilePath());

		int fetched = 0;

		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::ASIAN, Gender::MALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::ASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::COCASIAN, Gender::MALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::COCASIAN, Gender::FEMALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::LATINO, Gender::MALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::LATINO, Gender::FEMALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::OTHER, Gender::MALE);
		fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::OTHER, Gender::FEMALE);
		//fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::MIXED, Gender::MALE);
		//fetched += fetchForStringSector(getRNFLSymmetry(), db, "RNFL_SYMMETRY", Ethinicity::MIXED, Gender::FEMALE);

		LogD() << "Normative RNFL Symmetry fetched, " << fetched << " records";

		return true;
	}
	catch (std::exception& e) {
		LogE() << "exception: " << e.what() << std::endl;
	}

	return false;
}

template <typename NormType, typename DBClsType>
int NormData::NormArchive::fetchForStringSector(NormType& norm, DBClsType& db, std::string strTable,
	Ethinicity race, Gender gender)
{
	std::stringbuf strBuf;
	std::ostream stream(&strBuf);

	stream << "SELECT * FROM ";
	stream << strTable;
	stream << " WHERE Gender=";
	stream << static_cast<int>(gender);
	stream << " AND Race=";
	stream << static_cast<int>(race);

	SQLite::Statement query(db, strBuf.str());

	try {
		int fetched = 0;
		
		while (query.executeStep()) {
			norm.insertData(
				race,
				gender,
				(const char*)query.getColumn(2),
				(float)(double)query.getColumn(3),
				(float)(double)query.getColumn(4),
				(float)(double)query.getColumn(5));
			fetched++;
		}
		return fetched;
	}
	catch (std::exception& e) {
		throw e;
	}
}

template <typename NormType, typename DBClsType>
int NormData::NormArchive::fetchForIntegerSector(NormType& norm, DBClsType& db, std::string strTable,
	Ethinicity race, Gender gender)
{
	std::stringbuf strBuf;
	std::ostream stream(&strBuf);

	stream << "SELECT * FROM ";
	stream << strTable;
	stream << " WHERE Gender=";
	stream << static_cast<int>(gender);
	stream << " AND Race=";
	stream << static_cast<int>(race);
	stream << " ORDER BY Sector ASC";

	SQLite::Statement query(db, strBuf.str());

	try {
		int fetched = 0;

		while (query.executeStep()) {
			norm.insertData(
				race,
				gender,
				(int)(double)query.getColumn(2),
				(float)(double)query.getColumn(3),
				(float)(double)query.getColumn(4),
				(float)(double)query.getColumn(5));
			fetched++;
		}
		return fetched;
	}
	catch (std::exception& e) {
		throw e;
	}
}

NormArchive::NormArchiveImpl & NormData::NormArchive::getImpl(void) const
{
	return *d_ptr;
}
