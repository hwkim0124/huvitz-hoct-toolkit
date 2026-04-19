#pragma once

#include "SemtRetinaDef.h"
#include "RetinaSegmFrame.h"


namespace SemtRetina
{
	class MacularSegmenter;

	class SEMTRETINA_DLL_API MacularSegmFrame : public RetinaSegmFrame
	{
	public:
		MacularSegmFrame();
		virtual ~MacularSegmFrame();

		MacularSegmFrame(MacularSegmFrame&& rhs);
		MacularSegmFrame& operator=(MacularSegmFrame&& rhs);
		MacularSegmFrame(const MacularSegmFrame& rhs) = delete;
		MacularSegmFrame& operator=(const MacularSegmFrame& rhs) = delete;

	public:
		bool segmentMacularLayers(bool angio);

	protected:
		void resetRetinaSegmenter() override;
		MacularSegmenter* macularSegmenter() const;

	private:
		struct MacularSegmFrameImpl;
		std::unique_ptr<MacularSegmFrameImpl> d_ptr;
		MacularSegmFrameImpl& impl(void) const;
	};

}
