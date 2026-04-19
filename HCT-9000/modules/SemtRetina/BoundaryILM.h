#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryILM : public RetinaBoundary
	{
	public:
		BoundaryILM(RetinaSegmenter* segm);
		virtual ~BoundaryILM();

		BoundaryILM(BoundaryILM&& rhs);
		BoundaryILM& operator=(BoundaryILM&& rhs);
		BoundaryILM(const BoundaryILM& rhs) = delete;
		BoundaryILM& operator=(const BoundaryILM& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool reconstructLayer(void);
		
	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool smoothBoundaryILM(void);
		bool smoothRefinedILM(void);
		bool resizeToMatchSource(void);

	private:
		struct BoundaryILMImpl;
		std::unique_ptr<BoundaryILMImpl> d_ptr;
		BoundaryILMImpl& impl(void) const;
	};
}

