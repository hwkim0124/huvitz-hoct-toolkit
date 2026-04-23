#pragma once

#include "SemtRetinaDef.h"

namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API RetinaSegmCriteria
	{
	public:
		RetinaSegmCriteria(RetinaSegmenter* segm);
		virtual ~RetinaSegmCriteria();

		RetinaSegmCriteria(RetinaSegmCriteria&& rhs);
		RetinaSegmCriteria& operator=(RetinaSegmCriteria&& rhs);
		RetinaSegmCriteria(const RetinaSegmCriteria& rhs) = delete;
		RetinaSegmCriteria& operator=(const RetinaSegmCriteria& rhs) = delete;

	protected:
		float sampleScaleX(void) const;
		float sampleScaleY(void) const;
		float sampleSpaceX(void) const;
		float sampleSpaceY(void) const;

		float sourceScaleX(void) const;
		float sourceScaleY(void) const;
		float sourceSpaceX(void) const;
		float sourceSpaceY(void) const;

		float imageScaleX(void) const;
		float imageScaleY(void) const;
		float pixelSpaceX(void) const;
		float pixelSpaceY(void) const;

	public:
		void setSampleScaleFactors(float scaleX, float scaleY);
		void setSampleDimensions(int width, int height, float spaceX, float spaceY);
		void setSourceDimensions(int width, int height, float spaceX, float spaceY);
		void enableSampleDimensions(void);
		void enableSourceDimensions(void);
		bool fromSampleDimensions(void) const;

		int getColumnSnRatiosSmoothWindow(void) const;
		int getVitreousSizeToTriggerMidpoint(void) const;
		int getUpwardOffsetToInnerBound(void) const;
		int getSmoothWindowToInnerBound(void) const;
			
		int getDownwardOffsetToOuterBound(void) const;
		int getSmoothWindowToOuterBound(void) const;

		int getOpticDiscHeadWidthMin(void) const;
		int getOpticDiscSideWidthMin(void) const;
		int getOpticDiscHeadDepthMin(void) const;
		int getOpticDiscHeadMergeDist(void) const;
		int getOpticDiscCupDepthMin(void) const;

		int getGradientKernelRowsILM(void) const;
		int getGradientKernelColsILM(void) const;
		int getPathCostRangeDeltaILM(void) const;
		int getPathDiscRangeDeltaILM(void) const;
		int getPathDownwardMarginILM(void) const;
		int getPathSmoothWindowILM(void) const;
		int getLayerSmoothWindowILM(void) const;
		int getlayerUpperSpaceMinILM(void) const;

		int getPathSideMarginSlopeWidth(void) const;

		int getPathCostRangeDeltaONL(void) const;
		int getPathDiscRangeDeltaONL(void) const;
		int getPathTopLowerMarginONL(void) const;
		int getPathSmoothWindowONL(void) const;
		int getLayerSmoothWindowONL(bool local) const;

		int getGradientKernelRowsNFL(void) const;
		int getGradientKernelColsNFL(void) const;
		int getPathCostRangeDeltaNFL(void) const;
		int getPathDiscRangeDeltaNFL(void) const;
		int getPathDiscUpperSpaceNFL(void) const;
		int getPathSmoothWindowNFL(void) const;
		int getLayerSmoothWindowNFL(void) const;
		int getLayerUpperSpaceMinNFL(void) const;
		int getLayerOffsetMaxNFL(void) const;

		int getGradientKernelRowsOPL(void) const;
		int getGradientKernelColsOPL(void) const;
		int getPathCostRangeDeltaOPL(void) const;
		int getPathDiscRangeDeltaOPL(void) const;
		int getPathDiscUpwardMarginOPL(void) const;
		int getPathSmoothWindowOPL(void) const;
		int getLayerSmoothWindowOPL(void) const;
		int getLayerUpperSpaceMinOPL(void) const;
		int getLayerLowerSpaceMaxOPL(void) const;
		int getLayerOffsetMinOPL(void) const;
		int getLayerOffsetMaxOPL(void) const;

		int getGradientKernelRowsIPL(void) const;
		int getGradientKernelColsIPL(void) const;
		int getPathCostRangeDeltaIPL(void) const;
		int getPathDiscRangeDeltaIPL(void) const;
		int getPathSmoothWindowIPL(void) const;
		int getLayerSmoothWindowIPL(void) const;
		int getLayerOffsetMinIPL(void) const;
		int getLayerOffsetMaxIPL(void) const;

		int getGradientKernelRowsIOS(void) const;
		int getGradientKernelColsIOS(void) const;
		int getPathCostRangeDeltaIOS(void) const;
		int getPathDiscLowerSpaceMinIOS(void) const;
		int getPathDiscLowerSpaceMaxIOS(void) const;
		int getLayerUpperSpaceMaxIOS(void) const;
		int getLayerLowerSpaceMaxIOS(void) const;
		int getLayerSmoothWindowIOS(bool local) const;

		int getGradientKernelRowsRPE(void) const;
		int getGradientKernelColsRPE(void) const;
		int getPathCostRangeDeltaRPE(void) const;
		int getPathSmoothWindowRPE(void) const;
		int getPathCostRangeLimitRPE(void) const;
		int getPathUpwardOffsetRPE(void) const;
		int getLayerLowerOffsetMinRPE(void) const;
		int getLayerLowerOffsetMaxRPE(void) const;
		int getLayerDistanceMinRPE(void) const;
		int getLayerDistanceMaxRPE(void) const;
		int getLayerSmoothWindowRPE(bool local) const;

		int getGradientKernelRowsBRM(void) const;
		int getGradientKernelColsBRM(void) const;
		int getPathCostRangeDeltaBRM(void) const;
		int getPathDiscUpperSpaceMinBRM(void) const;
		int getPathDiscUpperSpaceMaxBRM(void) const;
		int getPathDownwardOffsetBRM(void) const;
		int getLayerLowerSpaceMinBRM(void) const;
		int getLayerLowerSpaceMaxBRM(void) const;

		int getLayerDownwardOffsetBRM(void) const;
		int getLayerDiscOffsetMinBRM(void) const;
		int getLayerDiscOffsetMaxBRM(void) const;
		int getLayerSmoothWindowBRM(bool local) const;

	private:
		struct RetinaSegmCriteriaImpl;
		std::unique_ptr<RetinaSegmCriteriaImpl> d_ptr;
		RetinaSegmCriteriaImpl& impl(void) const;
	};
}