#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryONL : public RetinaBoundary
	{
	public:
		BoundaryONL(RetinaSegmenter* segm);
		virtual ~BoundaryONL();

		BoundaryONL(BoundaryONL&& rhs);
		BoundaryONL& operator=(BoundaryONL&& rhs);
		BoundaryONL(const BoundaryONL& rhs) = delete;
		BoundaryONL& operator=(const BoundaryONL& rhs) = delete;

	public:
		bool detectBoundary(void);

	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool smoothBoundaryONL(void);
		bool resizeToMatchSource(void);

	private:
		struct BoundaryONLImpl;
		std::unique_ptr<BoundaryONLImpl> d_ptr;
		BoundaryONLImpl& impl(void) const;
	};
}

