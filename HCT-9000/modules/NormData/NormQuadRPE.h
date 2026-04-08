#pragma once

#include "NormDataDef.h"

#include <string>
#include <memory>
#include <map>

namespace NormData
{
	class QuadRPE_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		QuadRPE_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit QuadRPE_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit QuadRPE_Data(float inclin_, float bias_, float stddev_) {
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


	class NORMDATA_DLL_API NormQuadRPE
	{
	public:
		NormQuadRPE();
		virtual ~NormQuadRPE();

		NormQuadRPE(NormQuadRPE&& rhs);
		NormQuadRPE& operator=(NormQuadRPE&& rhs);

		// Prevent copy construction and assignment. 
		NormQuadRPE(const NormQuadRPE& rhs) = delete;
		NormQuadRPE& operator=(const NormQuadRPE& rhs) = delete;

	public:
		void insertData(Ethinicity race, Gender gender,
			const char* sector, float inclin,
			float inter1, float inter2, float inter3, float inter4, float inter5);
		void insertData(Ethinicity race, Gender gender,
			const char* sector, float inclin, float inter, float stddev);

		int getPercentile(Ethinicity race, Gender gender,
			int age, EyeSide side, int sector, float value);
		const char* getSectorStr(EyeSide side, int sector);

	protected:
		std::map<std::string, QuadRPE_Data>& getDataMap(Ethinicity race, Gender gender);

	private:
		struct NormQuadRPEImpl;
		std::unique_ptr<NormQuadRPEImpl> d_ptr;
		NormQuadRPEImpl& getImpl(void) const;
	};
}
