#pragma once

#include "OctAngioDef.h"
#include "AngioLayers.h"

#include <memory>


namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class AngioData;
	class AngioLayers;
	class AngioLayout;

	class OCTANGIO_DLL_API AngioDecorr
	{
	public:
		AngioDecorr();
		virtual ~AngioDecorr();

		AngioDecorr(AngioDecorr&& rhs);
		AngioDecorr& operator=(AngioDecorr&& rhs);

	public:
		auto estimateThreshold(const AngioLayout& layout, const AngioData& data, const AngioLayers& layers) -> bool;

		auto calculateSignals(const AngioLayout& layout, const AngioData& data, const AngioLayers& layers,
							int overlaps, int avgOffsetInn=0, int avgOffsetOut=0, float threshRatioInn=1.0f, float threshRatioOut=1.0f) -> bool;
		auto updateProjectionMask(const AngioLayout& layout, const LayerArrays& uppers, const LayerArrays& lowers, bool outerFlow = false, int avgSize = ANGIO_PROJ_MASK_AVG_SIZE) -> bool;

		auto updateProjectionProfiles(const AngioLayout& layout, const AngioLayers& layers,
							bool calcStats = false, bool applyPAR = false, bool modelEye = false,
							const std::vector<float>& maskDecorr = std::vector<float>(),
							const std::vector<float>& maskDiffer = std::vector<float>(), 
							int avgOffsetInn = ANGIO_PROJ_IMAGE_AVG_SIZE, 
							int avgOffsetOut = ANGIO_PROJ_IMAGE_AVG_SIZE, float maskWeight=1.0f) -> bool;
	
		bool checkIfDecorrelationsLoaded(int lines, int points, int repeats);
		bool calculateBscanImageStats(const AngioLayout& layout);
		bool calculateProjectionStats(bool modelEye);

		bool normalizeProjectionProfiles(const AngioLayout& layout, const AngioLayers& layers, float decorrMin, float decorrMax, float differMin, 
										float differMax, float dropoff);
		bool normalizeDifferAngiogram(const AngioLayout& layout, const AngioLayers& layers, float rangeMin, float rangeMax, float sizeMin, bool modelEye);
		bool normalizeDecorrAngiogram(const AngioLayout& layout, const AngioLayers& layers, float rangeMin, float rangeMax, float sizeMin, bool modelEye);
		bool denoiseProjectionProfiles(int lines, int points, float decorrMin, float decorrMax);

		std::vector<CppUtil::CvImage>& decorrelations(void) const;
		std::vector<CppUtil::CvImage>& differentials(void) const;
		std::vector<float>& decorrAngiogram(void) const;
		std::vector<float>& differAngiogram(void) const;
		std::vector<float>& outputAngiogram(void) const;
		std::vector<float>& decorrProjectionMax(void) const;
		std::vector<float>& decorrProjectionMask(void) const;
		std::vector<float>& differProjectionMask(void) const;
		std::vector<float>& decorrAxialOffsets(void) const;

		CppUtil::CvImage decorrAngiogramImage(bool axialMax=false) const;
		CppUtil::CvImage differAngiogramImage(void) const;

		float* getDecorrelationData(int index);
		float* getDifferentialsData(int index);

		CppUtil::CvImage getDecorrelationImage(int index);
		CppUtil::CvImage getDifferentialsImage(int index);

		float& backgroundThreshold(void);

		void getDecorrelationStat(float& mean, float& stdev, float& maxval);
		void getIntensityStat(float& mean, float& stdev, float& maxval);
		void getDecorrelationStatOfBscan(int index, float& mean, float& stdev, float& maxval);
		void getIntensityStatOfBscan(int index, float& mean, float& stdev, float& maxval);


	private:
		struct AngioDecorrImpl;
		std::unique_ptr<AngioDecorrImpl> d_ptr;
		AngioDecorrImpl& getImpl(void) const;
	};
}


