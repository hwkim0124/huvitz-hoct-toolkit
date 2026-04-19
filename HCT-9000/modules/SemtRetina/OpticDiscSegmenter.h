#pragma once

#include "SemtRetinaDef.h"
#include "RetinaSegmenter.h"

namespace SemtRetina
{
	class SEMTRETINA_DLL_API OpticDiscSegmenter : public RetinaSegmenter
	{
	public:
		OpticDiscSegmenter(RetinaSegmFrame* frame);
		virtual ~OpticDiscSegmenter();

		OpticDiscSegmenter(OpticDiscSegmenter&& rhs);
		OpticDiscSegmenter& operator=(OpticDiscSegmenter&& rhs);
		OpticDiscSegmenter(const OpticDiscSegmenter& rhs) = delete;
		OpticDiscSegmenter& operator=(const OpticDiscSegmenter& rhs) = delete;

	public:
		bool segment(bool angio) override;

	protected:
		void resetAlgorithms(void) override;

	private:
		struct OpticDiscSegmenterImpl;
		std::unique_ptr<OpticDiscSegmenterImpl> d_ptr;
		OpticDiscSegmenterImpl& impl(void) const;
	};
}
