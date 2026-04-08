#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryOPL : public RetinaBoundary
	{
	public:
		BoundaryOPL(RetinaSegmenter* segm);
		virtual ~BoundaryOPL();

		BoundaryOPL(BoundaryOPL&& rhs);
		BoundaryOPL& operator=(BoundaryOPL&& rhs);
		BoundaryOPL(const BoundaryOPL& rhs) = delete;
		BoundaryOPL& operator=(const BoundaryOPL& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool enforceSourceOrder(void);

	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool smoothBoundaryOPL(void);
		bool resizeToMatchSource(void);


	private:
		struct BoundaryOPLImpl;
		std::unique_ptr<BoundaryOPLImpl> d_ptr;
		BoundaryOPLImpl& impl(void) const;
	};
}

