#pragma once

#include "SemtRetinaDef.h"
#include "BscanImageData.h"


namespace SemtRetina
{
	class SEMTRETINA_DLL_API BscanResampler
	{
	public:
		BscanResampler();
		virtual ~BscanResampler();

		BscanResampler(BscanResampler&& rhs);
		BscanResampler& operator=(BscanResampler&& rhs);
		BscanResampler(const BscanResampler& rhs) = delete;
		BscanResampler& operator=(const BscanResampler& rhs) = delete;

	public:
		bool runResampling(OctScanImage image, int width, int height);

		bool checkRetinaSegmentable(void) const;

		int sourceWidth(void) const;
		int sourceHeight(void) const;
		int sampleWidth(void) const;
		int sampleHeight(void) const;
		int sampleIndex(void) const;

		BscanImageData* imageSource(void) const;
		BscanImageData* imageSample(void) const;
		BscanImageData* imageCoarse(void) const;

	protected:

	private:
		struct BscanResamplerImpl;
		std::unique_ptr<BscanResamplerImpl> d_ptr;
		BscanResamplerImpl& impl(void) const;
	};
}
