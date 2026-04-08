#pragma once

#include "SemtRetinaDef.h"
#include "RetinaSegmenter.h"

namespace SemtRetina
{
	class SEMTRETINA_DLL_API MacularSegmenter : public RetinaSegmenter
	{
	public:
		MacularSegmenter(RetinaSegmFrame* frame);
		virtual ~MacularSegmenter();

		MacularSegmenter(MacularSegmenter&& rhs);
		MacularSegmenter& operator=(MacularSegmenter&& rhs);
		MacularSegmenter(const MacularSegmenter& rhs) = delete;
		MacularSegmenter& operator=(const MacularSegmenter& rhs) = delete;

	public:
		bool segment(void) override;

	protected:
		void resetAlgorithms(void) override;

	private:
		struct MacularSegmenterImpl;
		std::unique_ptr<MacularSegmenterImpl> d_ptr;
		MacularSegmenterImpl& impl(void) const;
	};
}

