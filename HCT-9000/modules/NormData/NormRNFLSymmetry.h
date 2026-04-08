#pragma once

#include "NormDataDef.h"

#include <string>
#include <memory>
#include <map>

namespace NormData
{
	class RNFL_Symmetry_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		RNFL_Symmetry_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit RNFL_Symmetry_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit RNFL_Symmetry_Data(float inclin_, float bias_, float stddev_) {
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

	class NORMDATA_DLL_API NormRNFLSymmetry
	{
	public:
		NormRNFLSymmetry();
		virtual ~NormRNFLSymmetry();

		NormRNFLSymmetry(NormRNFLSymmetry&& rhs);
		NormRNFLSymmetry& operator=(NormRNFLSymmetry&& rhs);

		// Prevent copy construction and assignment. 
		NormRNFLSymmetry(const NormRNFLSymmetry& rhs) = delete;
		NormRNFLSymmetry& operator=(const NormRNFLSymmetry& rhs) = delete;

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
			int age, EyeSide side, int percentile);

	protected:
		std::map<std::string, RNFL_Symmetry_Data>& getDataMap(Ethinicity race, Gender gender);

	private:
		struct NormRNFLSymmetryImpl;
		std::unique_ptr<NormRNFLSymmetryImpl> d_ptr;
		NormRNFLSymmetryImpl& getImpl(void) const;
	};
}
