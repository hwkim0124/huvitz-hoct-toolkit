#pragma once

#include "NormDataDef.h"

#include <string>
#include <memory>
#include <map>

namespace NormData
{
	class RNFL_Average_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		RNFL_Average_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit RNFL_Average_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit RNFL_Average_Data(float inclin_, float bias_, float stddev_) {
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

	class NORMDATA_DLL_API NormRNFLAverage
	{
	public:
		NormRNFLAverage();
		virtual ~NormRNFLAverage();

		NormRNFLAverage(NormRNFLAverage&& rhs);
		NormRNFLAverage& operator=(NormRNFLAverage&& rhs);

		// Prevent copy construction and assignment. 
		NormRNFLAverage(const NormRNFLAverage& rhs) = delete;
		NormRNFLAverage& operator=(const NormRNFLAverage& rhs) = delete;

	public:
		void insertData(Ethinicity race, Gender gender,
			const char* sector, float inclin,
			float inter1, float inter2, float inter3, float inter4, float inter5);

		void insertData(Ethinicity race, Gender gender,
			const char* sector, float inclin, float inter, float stddev);

		int getPercentile(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		const char* getSectorStr(EyeSide side, int sector);

		float getNormThickness(Ethinicity race, Gender gender,
			int age, EyeSide side,int percentile);

	protected:
		std::map<std::string, RNFL_Average_Data>& getDataMap(Ethinicity race, Gender gender);

	private:
		struct NormRNFLAverageImpl;
		std::unique_ptr<NormRNFLAverageImpl> d_ptr;
		NormRNFLAverageImpl& getImpl(void) const;
	};
}

