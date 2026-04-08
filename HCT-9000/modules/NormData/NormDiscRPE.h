#pragma once

#include "NormDataDef.h"
#include "NormDeviationMap.h"

#include <string>
#include <memory>
#include <map>

namespace NormData
{
	class DiscRPE_Data
	{
	public:
		static const int N_VALUES = 4;

	public:
		DiscRPE_Data() {
			this->inclin = 0.0f;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = 0.0f;
			}
		}

		explicit DiscRPE_Data(float inclin_, float* inters_) {
			this->inclin = inclin_;
			for (int i = 0; i < N_VALUES; i++) {
				this->inters[i] = inters_[i];
			}
		}

		explicit DiscRPE_Data(float inclin_, float bias_, float stddev_) {
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


	class NORMDATA_DLL_API NormDiscRPE : NormDeviationMap
	{
	public:
		NormDiscRPE();
		virtual ~NormDiscRPE();

		NormDiscRPE(NormDiscRPE&& rhs);
		NormDiscRPE& operator=(NormDiscRPE&& rhs);

		// Prevent copy construction and assignment. 
		NormDiscRPE(const NormDiscRPE& rhs) = delete;
		NormDiscRPE& operator=(const NormDiscRPE& rhs) = delete;

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
		std::map<int, DiscRPE_Data>& getDataMap(Ethinicity race, Gender gender);

	private:
		struct NormDiscRPEImpl;
		std::unique_ptr<NormDiscRPEImpl> d_ptr;
		NormDiscRPEImpl& getImpl(void) const;
	};
}
