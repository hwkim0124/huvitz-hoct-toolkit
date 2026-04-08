#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryRPE : public RetinaBoundary
	{
	public:
		BoundaryRPE(RetinaSegmenter* segm);
		virtual ~BoundaryRPE();

		BoundaryRPE(BoundaryRPE&& rhs);
		BoundaryRPE& operator=(BoundaryRPE&& rhs);
		BoundaryRPE(const BoundaryRPE& rhs) = delete;
		BoundaryRPE& operator=(const BoundaryRPE& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool refineBoundary(void);

		bool enforceSourceOrder(void);

	protected:
		bool designPathConstraints(void);
		bool refinePathConstraints(void);

		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool refineGraidentMap(void);
		bool refinePathCostMap(void);

		bool smoothBoundaryRPE(void);
		bool smoothRefinedRPE(void);
		bool resizeToMatchSource(void);


	private:
		struct BoundaryRPEImpl;
		std::unique_ptr<BoundaryRPEImpl> d_ptr;
		BoundaryRPEImpl& impl(void) const;
	};
}

