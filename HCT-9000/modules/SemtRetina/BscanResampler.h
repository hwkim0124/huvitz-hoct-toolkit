#pragma once

#include "SemtRetinaDef.h"
#include "BscanImageData.h"


namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API BscanResampler
	{
	public:
		BscanResampler(RetinaSegmenter* segm);
		virtual ~BscanResampler();

		BscanResampler(BscanResampler&& rhs);
		BscanResampler& operator=(BscanResampler&& rhs);
		BscanResampler(const BscanResampler& rhs) = delete;
		BscanResampler& operator=(const BscanResampler& rhs) = delete;

	public:
		bool runResampling(OctScanImage image, bool angio);

		bool checkRetinaSegmentable(void) const;
		bool createSourceEdgeMaps(void) const;

		int sourceWidth(void) const;
		int sourceHeight(void) const;
		int sampleWidth(void) const;
		int sampleHeight(void) const;
		int sampleIndex(void) const;
		bool isAngio(void) const;

		float sourcePixelSpaceX(void) const;
		float sourcePixelSpaceY(void) const;
		float samplePixelSpaceX(void) const;
		float samplePixelSpaceY(void) const;
		float sampleScaleRatioX(void) const;
		float sampleScaleRatioY(void) const;

		BscanImageData* imageSource(void) const;
		BscanImageData* imageSample(void) const;
		BscanImageData* imageCoarse(void) const;

		BscanImageData* sourceRiseEdge(void) const;
		BscanImageData* sourceRiseEdgeHigh(void) const;
		BscanImageData* sourceFallEdge(void) const;
		BscanImageData* sourceFallEdgeHigh(void) const;

	protected:

	private:
		struct BscanResamplerImpl;
		std::unique_ptr<BscanResamplerImpl> d_ptr;
		BscanResamplerImpl& impl(void) const;
	};
}
