#pragma once

#include "NormDataDef.h"

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace NormData
{
	class TSNIT_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		TSNIT_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit TSNIT_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit TSNIT_Data(float inclin_, float bias_, float stddev_) {
			this->inclin = inclin_;
			this->bias = bias_;
			this->stddev = stddev_;
		}

		float getNormValue(int age, int index) const {
			float norm = this->inclin * age + this->inters[index];
			return norm;
		}

		float getMean(int age) const {
			float mean = this->inclin * age + this->bias;
			return mean;
		}

	public:
		float inclin = 0.0f;
		float bias = 0.0f;
		float stddev = 0.0f;
		float inters[N_VALUES] = { 0.0f };
	};


	class NORMDATA_DLL_API NormTSNIT
	{
	public:
		NormTSNIT();
		virtual ~NormTSNIT();

		NormTSNIT(NormTSNIT&& rhs);
		NormTSNIT& operator=(NormTSNIT&& rhs);

		// Prevent copy construction and assignment. 
		NormTSNIT(const NormTSNIT& rhs) = delete;
		NormTSNIT& operator=(const NormTSNIT& rhs) = delete;

	public:
		void insertData(Ethinicity race, Gender gender,
			int sector, float inclin,
			float inter1, float inter2, float inter3, float inter4, float inter5);
		void insertData(Ethinicity race, Gender gender,
			int sector, float inclin, float inter, float stddev);

		std::vector<float> getNormGraph(Ethinicity race, Gender gender, int age, EyeSide side, int percentile, int dataSize = 256, int filter = 11);

	protected:
		std::map<int, TSNIT_Data>& getDataMap(Ethinicity race, Gender gender);
		std::vector<float> applyAveraging(const std::vector<float>& raws, int filter);

	private:
		struct NormTSNITImpl;
		std::unique_ptr<NormTSNITImpl> d_ptr;
		NormTSNITImpl& getImpl(void) const;
	};
}