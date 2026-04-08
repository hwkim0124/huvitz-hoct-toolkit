#pragma once

#include "SemtRetinaDef.h"

namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API RetinaSegmFrame :
		public std::enable_shared_from_this<RetinaSegmFrame>
	{
	public:
		RetinaSegmFrame();
		virtual ~RetinaSegmFrame();

		RetinaSegmFrame(RetinaSegmFrame&& rhs);
		RetinaSegmFrame& operator=(RetinaSegmFrame&& rhs);
		RetinaSegmFrame(const RetinaSegmFrame& rhs) = delete;
		RetinaSegmFrame& operator=(const RetinaSegmFrame& rhs) = delete;

	public:
		void setBscanImage(const unsigned char* bits, int width, int height, float range = 6.0f, int index = 0);
		void setBscanImage(OctScanImage image);
		void setBscanIndex(int index);

		bool isImageSource(void) const;
		int bscanIndex(void) const;
		float scanRangeXmm(void) const;
		const OctScanImage& bscanImage(void) const;

		const std::vector<int>& boundaryILM(void) const;
		const std::vector<int>& boundaryNFL(void) const;
		const std::vector<int>& boundaryIPL(void) const;
		const std::vector<int>& boundaryOPL(void) const;
		const std::vector<int>& boundaryONL(void) const;
		const std::vector<int>& boundaryIOS(void) const;
		const std::vector<int>& boundaryRPE(void) const;
		const std::vector<int>& boundaryBRM(void) const;

		const std::vector<int>& boundaryINN(void) const;
		const std::vector<int>& boundaryOUT(void) const;

		const int onhDiscMarginMinX(void) const;
		const int onhDiscMarginMaxX(void) const;
		const int onhCupOpeningMinX(void) const;
		const int onhCupOpeningMaxX(void) const;
		const int onhDiscRimVoxelCount(void) const;
		const int onhDiscCupVoxelCount(void) const;
		const bool isONHDiscRegionDetected(void) const;
		const bool isONHCupRegionDetected(void) const;

	protected:
		virtual void resetRetinaSegmenter() = 0;

		void setRetinaSegmenter(RetinaSegmenter* segm);
		RetinaSegmenter* retinaSegmenter() const;

	private:
		struct RetinaSegmFrameImpl;
		std::unique_ptr<RetinaSegmFrameImpl> d_ptr;
		RetinaSegmFrameImpl& impl(void) const;
	};

}
