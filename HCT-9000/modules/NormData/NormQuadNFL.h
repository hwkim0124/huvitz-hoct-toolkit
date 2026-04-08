#pragma once

#include "NormDataDef.h"

#include <string>
#include <memory>
#include <map>

namespace NormData
{
	class QuadNFL_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		QuadNFL_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit QuadNFL_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit QuadNFL_Data(float inclin_, float bias_, float stddev_) {
			this->inclin = inclin_;
			this->bias = bias_;
			this->stddev = stddev_;
		}

		float getNormValue(int age, int index) {
			float norm = this->inclin * age + this->inters[index];
			return norm;
		}

		float getMean(int age) {
			float mean = this->inclin * age + this->bias;
			return mean;
		}

	public:
		float inclin = 0.0f;
		float bias = 0.0f;
		float stddev = 0.0f;
		float inters[N_VALUES] = { 0.0f };
	};


	class NORMDATA_DLL_API NormQuadNFL
	{
	public:
		NormQuadNFL();
		virtual ~NormQuadNFL();

		NormQuadNFL(NormQuadNFL&& rhs);
		NormQuadNFL& operator=(NormQuadNFL&& rhs);

		// Prevent copy construction and assignment. 
		NormQuadNFL(const NormQuadNFL& rhs) = delete;
		NormQuadNFL& operator=(const NormQuadNFL& rhs) = delete;

	public:
		void insertData(Ethinicity race, Gender gender,
			const char* sector, float inclin,
			float inter1, float inter2, float inter3, float inter4, float inter5);
		void insertData(Ethinicity race, Gender gender,
			const char* sector, float inclin, float inter, float stddev);

		int getPercentile(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		const char* getSectorStr(EyeSide side, int sector);

		static void setTsnitData(const std::vector<float>& tsnitData);
		static void setTsnitData95(const std::vector<float>& tsnitData);
		static void setTsnitData5(const std::vector<float>& tsnitData);
		static void setTsnitData1(const std::vector<float>& tsnitData);
		static const std::vector<float>& getTsnitData();
		static void clearTsnitData();
		static void clearTsnitDataAll();

		int getPercentileWithTsnit(Ethinicity race, Gender gender, int age, EyeSide side, int sector, float value,
			bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex);

	private:
		std::vector<float> calculateTsnitPercentiles(const std::vector<float>& tsnitThickness,
			Ethinicity race, Gender gender, int age, EyeSide side);
		int calculateWeightedRegionPercentile(int sector, const std::vector<float>& tsnitPercentiles,
			bool isCompareType, EyeSide realEyeSide, bool isMacularDisc, int dataIndex);
		std::vector<int> getQuadTsnitIndices(int regionIndex, bool needsEyeSideConversion);

		static std::vector<float> s_quadTsnitData;
		static std::vector<float> s_quadTsnitData95;
		static std::vector<float> s_quadTsnitData5;
		static std::vector<float> s_quadTsnitData1;

	protected:
		std::map<std::string, QuadNFL_Data>& getDataMap(Ethinicity race, Gender gender);

	private:
		struct NormQuadNFLImpl;
		std::unique_ptr<NormQuadNFLImpl> d_ptr;
		NormQuadNFLImpl& getImpl(void) const;
	};
}
