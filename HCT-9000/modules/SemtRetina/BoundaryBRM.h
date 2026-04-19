#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryBRM : public RetinaBoundary
	{
	public:
		BoundaryBRM(RetinaSegmenter* segm);
		virtual ~BoundaryBRM();

		BoundaryBRM(BoundaryBRM&& rhs);
		BoundaryBRM& operator=(BoundaryBRM&& rhs);
		BoundaryBRM(const BoundaryBRM& rhs) = delete;
		BoundaryBRM& operator=(const BoundaryBRM& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool reconstructLayer(void);
		bool enforceSourceOrder(void);

	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);

		bool smoothBoundaryBRM(void);
		bool smoothRefinedBRM(void);
		bool resizeToMatchSource(void);


	private:
		struct BoundaryBRMImpl;
		std::unique_ptr<BoundaryBRMImpl> d_ptr;
		BoundaryBRMImpl& impl(void) const;
	};
}

