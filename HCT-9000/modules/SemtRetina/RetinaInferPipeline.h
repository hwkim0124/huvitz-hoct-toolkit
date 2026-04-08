#pragma once

#include "SemtRetinaDef.h"

namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API RetinaInferPipeline
	{
	public:
		RetinaInferPipeline(RetinaSegmenter* segm);
		virtual ~RetinaInferPipeline();

		RetinaInferPipeline(RetinaInferPipeline&& rhs);
		RetinaInferPipeline& operator=(RetinaInferPipeline&& rhs);
		RetinaInferPipeline(const RetinaInferPipeline& rhs) = delete;
		RetinaInferPipeline& operator=(const RetinaInferPipeline& rhs) = delete;

	public:
		bool inferLayerProbMaps(void);
		int probMapWidth(void) const;
		int probMapHeight(void) const;

		const float* probMapVitreous(void) const;
		const float* probMapRnfl(void) const;
		const float* probMapIplOpl(void) const;
		const float* probMapOnl(void) const;
		const float* probMapRpe(void) const;
		const float* probMapChoroid(void) const;
		const float* probMapSclera(void) const;
		const float* probMapDiscHead(void) const;

	private:
		struct RetinaInferPipelineImpl;
		std::unique_ptr<RetinaInferPipelineImpl> d_ptr;
		RetinaInferPipelineImpl& impl(void) const;
	};
}

