#pragma once

#include "SemtSegmDef.h"

#include <limits.h>

namespace SemtSegm
{
	class BscanSegmentator;

	class SEMTSEGM_DLL_API OptimalLayer
	{
	public:
		OptimalLayer(BscanSegmentator* pSegm);
		virtual ~OptimalLayer();

		OptimalLayer(OptimalLayer&& rhs);
		OptimalLayer& operator=(OptimalLayer&& rhs);
		OptimalLayer(const OptimalLayer& rhs) = delete;
		OptimalLayer& operator=(const OptimalLayer& rhs) = delete;

	public:
		std::vector<int>& getOptimalPath() const;
		void setRegionDimension(int width, int height, bool stretchY = true);
		void applyVeritcalShifts(std::vector<int> shifts, bool inverse = false);

	protected:
		bool searchPathMinCost();
		bool interpolateByLinearFitt(const std::vector<int>& input, std::vector<int>& output, 
			bool sideFitt, int moveSpan = 3, float sideData = 0.025f);

		OcularImage* getCostMap() const;
		OcularImage* getWeightMap() const;
		OcularImage* getBiasesMap() const;

		std::vector<int>& getUppers() const;
		std::vector<int>& getLowers() const;
		std::vector<int>& getDeltas() const;
		std::vector<int> getPoints(int shift = 0) const;

		const BscanSegmentator* getSegmentator() const;

		static const int PATH_COST_MAX = SHRT_MAX;

	private:
		struct OptimalLayerImpl;
		std::unique_ptr<OptimalLayerImpl> d_ptr;
		OptimalLayerImpl& getImpl(void) const;
	};
}

