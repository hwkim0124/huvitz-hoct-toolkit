#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryIOS : public RetinaBoundary
	{
	public:
		BoundaryIOS(RetinaSegmenter* segm);
		virtual ~BoundaryIOS();

		BoundaryIOS(BoundaryIOS&& rhs);
		BoundaryIOS& operator=(BoundaryIOS&& rhs);
		BoundaryIOS(const BoundaryIOS& rhs) = delete;
		BoundaryIOS& operator=(const BoundaryIOS& rhs) = delete;

	public:
		bool detectBoundary(void);
		bool enforceSourceOrder(void);

	protected:
		bool designPathConstraints(void);
		bool prepareGradientMap(void);
		bool preparePathCostMap(void);
		bool smoothBoundaryIOS(void);
		bool resizeToMatchSource(void);


	private:
		struct BoundaryIOSImpl;
		std::unique_ptr<BoundaryIOSImpl> d_ptr;
		BoundaryIOSImpl& impl(void) const;
	};
}

