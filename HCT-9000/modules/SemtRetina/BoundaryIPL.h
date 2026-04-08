#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryIPL : public RetinaBoundary
	{
	public:
		BoundaryIPL(RetinaSegmenter* segm);
		virtual ~BoundaryIPL();

		BoundaryIPL(BoundaryIPL&& rhs);
		BoundaryIPL& operator=(BoundaryIPL&& rhs);
		BoundaryIPL(const BoundaryIPL& rhs) = delete;
		BoundaryIPL& operator=(const BoundaryIPL& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool enforceSourceOrder(void);

	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool smoothBoundaryIPL(void);
		bool resizeToMatchSource(void);


	private:
		struct BoundaryIPLImpl;
		std::unique_ptr<BoundaryIPLImpl> d_ptr;
		BoundaryIPLImpl& impl(void) const;
	};
}

