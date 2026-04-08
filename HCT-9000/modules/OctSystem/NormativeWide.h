#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <vector>


namespace NormData {
	class NormArchive;
}

namespace CppUtil {
	class CvImage;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API NormativeWide
	{
	public:
		NormativeWide();
		virtual ~NormativeWide();

	public:
		static bool initNormative(void);
		static bool isAvailabel(void);
		static std::string getDataFilePath(void);

		static int getPercentile_ETDRS(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_GCC(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_BisectGCC(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_QuadNFL(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_QuadRPE(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_ClockNFL(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_ClockRPE(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		static int getPercentile_DiscInfo(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);

		static std::vector<float> getGraph_TSNIT(Ethinicity race, Gender gender,
			int age, EyeSide side, int percentile, int dataSize = 256, int filter = 11);

		static CppUtil::CvImage getDeviation_DiscNFL(Ethinicity race, Gender gender,
			int age, EyeSide side, const std::vector<float>& data, int lines, int points,
			int width, int height);
		static CppUtil::CvImage getDeviation_DiscRPE(Ethinicity race, Gender gender,
			int age, EyeSide side, const std::vector<float>& data, int lines, int points,
			int width, int height);
		static CppUtil::CvImage getDeviation_MacularIPL(Ethinicity race, Gender gender,
			int age, EyeSide side, const std::vector<float>& data, int lines, int points,
			int width, int height);
		static CppUtil::CvImage getDeviation_MacularRPE(Ethinicity race, Gender gender,
			int age, EyeSide side, const std::vector<float>& data, int lines, int points,
			int width, int height);

		static float getNormThickness_MacularIPL(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, int percentile);
		static float getNormThickness_MacularRPE(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, int percentile);
		static float getNormThickness_DiscRPE(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, int percentile);
		static float getNormThickness_DiscNFL(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, int percentile);
		static float getNormThickness_RNFLAverage(Ethinicity race, Gender gender,
			int age, EyeSide side, int percentile);
		static float getNormThickness_RNFLSymmetry(Ethinicity race, Gender gender,
			int age, EyeSide side, int percentile);

		static float getNormValue_DiscInfo(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, int percentile);

	protected:
		struct NormativeWideImpl;
		static std::unique_ptr<NormativeWideImpl> d_ptr;
		static NormativeWideImpl& getImpl(void);

		static NormData::NormArchive& getArchive(void);
	};
}

