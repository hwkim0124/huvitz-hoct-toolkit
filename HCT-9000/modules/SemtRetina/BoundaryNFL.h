#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryNFL : public RetinaBoundary
	{
	public:
		BoundaryNFL(RetinaSegmenter* segm);
		virtual ~BoundaryNFL();

		BoundaryNFL(BoundaryNFL&& rhs);
		BoundaryNFL& operator=(BoundaryNFL&& rhs);
		BoundaryNFL(const BoundaryNFL& rhs) = delete;
		BoundaryNFL& operator=(const BoundaryNFL& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool enforceSourceOrder(void);

	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool smoothBoundaryNFL(void);
		bool resizeToMatchSource(void);


	private:
		struct BoundaryNFLImpl;
		std::unique_ptr<BoundaryNFLImpl> d_ptr;
		BoundaryNFLImpl& impl(void) const;
	};
}

