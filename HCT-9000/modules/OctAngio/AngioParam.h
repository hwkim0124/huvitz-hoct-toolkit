#pragma once

#include "OctAngioDef.h"


#include <memory>

namespace OctAngio
{
	class OCTANGIO_DLL_API AngioParam
	{
	public:
		AngioParam();
		virtual ~AngioParam();

		AngioParam(AngioParam&& rhs);
		AngioParam& operator=(AngioParam&& rhs);
		AngioParam(const AngioParam& rhs);
		AngioParam& operator=(const AngioParam& rhs);

	public:
		auto isVerticalScan(void) -> bool&;
		auto numberOfOverlaps(void) -> int&;
		auto numberOfPoints(void) -> int&;
		auto numberOfLines(void) -> int&;

		auto scanRangeX(void) -> float&;
		auto scanRangeY(void) -> float&;
		auto scanLineSpace(void) -> float&;
		auto foveaCenterX(void) -> float&;
		auto foveaCenterY(void) -> float&;

		auto lowerLayerType(void) -> OctGlobal::OcularLayerType&;
		auto upperLayerType(void) ->OctGlobal::OcularLayerType&;
		auto lowerLayerOffset(void) -> float&;
		auto upperLayerOffset(void) -> float&;

		auto useAxialAlign(void) -> bool&;
		auto useLateralAlign(void) -> bool&;
		auto usePixelAveraging(void) -> bool&;
		auto pixelAverageSize(void) -> int&;
		auto useDecorrCircular(void) -> bool&;
		auto useDifferOutput(void) -> bool&;
		auto useDecorrOutput(void) -> bool&;
		auto useReflectCorrection(void) -> bool&;
		
		auto decorrLowerThreshold(void) -> float&;
		auto intensLowerThreshold(void)-> int&;
		auto intensUpperthreshold(void)->int&;
		auto decorrNormLowerThreshold(void) -> float&;
		auto decorrNormUpperThreshold(void) -> float&;
		auto differNormLowerThreshold(void) -> float&;
		auto differNormUpperThreshold(void) -> float&;

		auto useLayersSelected(void) -> bool&;
		auto usePostProcessing(void) -> bool&;
		auto useNormProjection(void) -> bool&;
		auto useProjectArtifactRemoval(void) -> bool&;
		auto useMotionCorrection(void) -> bool&;
		auto useVascularLayers(void) -> bool&;
		auto useContrastEnhance(void) -> bool&;
		auto contrastClipLimit(void) -> float&;

		auto biasFieldSigma(void) -> float&;
		auto normalizeDropOff(void) -> float&;
		auto noiseReductionRate(void)-> float&;
		
	private:
		struct AngioParamImpl;
		std::unique_ptr<AngioParamImpl> d_ptr;
		AngioParamImpl& getImpl(void) const;
	};
}

