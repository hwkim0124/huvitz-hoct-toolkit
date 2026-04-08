#pragma once

#include "NormDataDef.h"

#include <string>
#include <memory>

namespace NormData
{
	class NormETDRS;
	class NormGCC;
	class NormQuadNFL;
	class NormQuadRPE;
	class NormClockNFL;
	class NormClockRPE;
	class NormTSNIT;
	class NormDiscNFL;
	class NormDiscRPE;
	class NormDiscInfo;
	class NormMacularIPL;
	class NormMacularRPE;
	class NormBisectGCC;
	class NormRNFLAverage;
	class NormRNFLSymmetry;

	class NORMDATA_DLL_API NormArchive
	{
	protected:
		const char* NORM_DB_FILE_NAME = "ndb.db";

	public:
		NormArchive();
		virtual ~NormArchive();

		NormArchive(NormArchive&& rhs);
		NormArchive& operator=(NormArchive&& rhs);

		// Prevent copy construction and assignment. 
		NormArchive(const NormArchive& rhs) = delete;
		NormArchive& operator=(const NormArchive& rhs) = delete;

	public:
		bool openDatabase(const char* path = nullptr, bool isWide = false);
		std::string getDBFilePath(void) const;
		bool isFetched(void) const;

		NormETDRS& getETDRS(void) const;
		NormGCC& getGCC(void) const;
		NormQuadNFL& getQuadNFL(void) const;
		NormQuadRPE& getQuadRPE(void) const;
		NormClockNFL& getClockNFL(void) const;
		NormClockRPE& getClockRPE(void) const;
		NormTSNIT& getTSNIT(void) const;
		NormDiscNFL& getDiscNFL(void) const;
		NormDiscRPE& getDiscRPE(void) const;
		NormDiscInfo& getDiscInfo(void) const;
		NormMacularIPL& getMacularIPL(void) const;
		NormMacularRPE& getMacularRPE(void) const;
		NormBisectGCC& getBisectGCC(void) const;
		NormRNFLAverage& getRNFLAverage(void) const;
		NormRNFLSymmetry& getRNFLSymmetry(void) const;

	protected:
		bool fetchETDRS(void);
		bool fetchGCC(void);
		bool fetchQuadNFL(void);
		bool fetchQuadRPE(void);
		bool fetchClockNFL(void);
		bool fetchClockRPE(void);
		bool fetchTSNIT(void);
		bool fetchDiscNFL(void);
		bool fetchDiscRPE(void);
		bool fetchDiscInfo(void);
		bool fetchMacularIPL(void);
		bool fetchMacularRPE(void);
		bool fetchBisectGCC(void);
		bool fetchRNFLAverage(void);
		bool fetchRNFLSymmetry(void);

		template <typename NormType, typename DBClsType>
		int fetchForStringSector(NormType& norm, DBClsType& db, std::string strTable, Ethinicity race,
			Gender gender);
		template <typename NormType, typename DBClsType>
		int fetchForIntegerSector(NormType& norm, DBClsType& db, std::string strTable, Ethinicity race,
			Gender gender);

	private:
		struct NormArchiveImpl;
		std::unique_ptr<NormArchiveImpl> d_ptr;
		NormArchiveImpl& getImpl(void) const;
	};
}
