#pragma once

#include "NormDataDef.h"
#include "NormDeviationMap.h"

#include <string>
#include <memory>
#include <map>

namespace NormData
{
	class MacularIPL_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		MacularIPL_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit MacularIPL_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit MacularIPL_Data(float inclin_, float bias_, float stddev_) {
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


	class NORMDATA_DLL_API NormMacularIPL : NormDeviationMap
	{
	public:
		NormMacularIPL();
		virtual ~NormMacularIPL();

		NormMacularIPL(NormMacularIPL&& rhs);
		NormMacularIPL& operator=(NormMacularIPL&& rhs);

		// Prevent copy construction and assignment. 
		NormMacularIPL(const NormMacularIPL& rhs) = delete;
		NormMacularIPL& operator=(const NormMacularIPL& rhs) = delete;

	public:
		void insertData(Ethinicity race, Gender gender,
			int sector, float inclin,
			float inter1, float inter2, float inter3, float inter4, float inter5);
		void insertData(Ethinicity race, Gender gender,
			int sector, float inclin, float inter, float stddev);

		int getPercentile(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		int getPercentile2(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);

		float getNormThickness(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, int percentile);

		CppUtil::CvImage makeImage(Ethinicity race, Gender gender, int age, EyeSide side,
			const std::vector<float>& data,
			int lines, int points, int width, int height, bool isWide = false) override;

	protected:
		std::map<int, MacularIPL_Data>& getDataMap(Ethinicity race, Gender gender);

	private:
		struct NormMacularIPLImpl;
		std::unique_ptr<NormMacularIPLImpl> d_ptr;
		NormMacularIPLImpl& getImpl(void) const;
	};
}
