#pragma once

#include "SemtRetinaDef.h"
#include "RetinaSegmFrame.h"


namespace SemtRetina
{
	class OpticDiscSegmenter;

	class SEMTRETINA_DLL_API OpticDiscSegmFrame : public RetinaSegmFrame
	{
	public:
		OpticDiscSegmFrame();
		virtual ~OpticDiscSegmFrame();

		OpticDiscSegmFrame(OpticDiscSegmFrame&& rhs);
		OpticDiscSegmFrame& operator=(OpticDiscSegmFrame&& rhs);
		OpticDiscSegmFrame(const OpticDiscSegmFrame& rhs) = delete;
		OpticDiscSegmFrame& operator=(const OpticDiscSegmFrame& rhs) = delete;

	public:
		bool segmentLayers(bool angio);

	protected:
		void resetRetinaSegmenter() override;
		OpticDiscSegmenter* opticDiscSegmenter() const;

	private:
		struct OpticDiscSegmFrameImpl;
		std::unique_ptr<OpticDiscSegmFrameImpl> d_ptr;
		OpticDiscSegmFrameImpl& impl(void) const;
	};

}
