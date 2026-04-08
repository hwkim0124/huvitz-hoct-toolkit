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
	class OCTSYSTEM_DLL_API Normative
	{
	public:
		Normative();
		virtual ~Normative();

	public:
		static bool initNormative(void);
		static bool isAvailabel(void);
		static std::string getDataFilePath(void);

	public:
		static void setTsnit(const std::vector<float>& values);
		static void	setTsnitNorm(const std::vector<float>& values95, const std::vector<float>& values5, const std::vector<float>& values1);
		static const std::vector<float>& getTsnit();
		static int getPercentile_QuadNFL(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value,
			bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex);
		static int getPercentile_ClockNFL(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value,
			bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex);

	private:
		static std::vector<float> getTsnitPercentiles(const std::vector<float>& tsnitThickness,
			Ethinicity race, Gender gender, int age, EyeSide side);
		static int calculateWeightedRegionPercentile(int regionType, int regionIndex,
			const std::vector<float>& tsnitPercentiles, bool isCompareType,
			EyeSide realEyeSide, bool isMacularDisc, int dataIndex);
		static std::vector<int> getRegionTsnitIndices(int regionType, int regionIndex,
			bool needsEyeSideConversion);
		static int colorToPercentileValue(float redRatio, float abnormalRatio,
			int whiteCount, int greenCount);

		static std::vector<float> TSNIT;

	public:
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
			int age, EyeSide side, int percentile, int dataSize=256, int filter = 11);
		
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
		struct NormativeImpl;
		static std::unique_ptr<NormativeImpl> d_ptr;
		static NormativeImpl& getImpl(void);

		static NormData::NormArchive& getArchive(void);
	};
}

