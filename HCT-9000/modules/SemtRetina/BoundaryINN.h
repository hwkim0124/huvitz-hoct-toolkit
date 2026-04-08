#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryINN : public RetinaBoundary
	{
	public:
		BoundaryINN(RetinaSegmenter* segm);
		virtual ~BoundaryINN();

		BoundaryINN(BoundaryINN&& rhs);
		BoundaryINN& operator=(BoundaryINN&& rhs);
		BoundaryINN(const BoundaryINN& rhs) = delete;
		BoundaryINN& operator=(const BoundaryINN& rhs) = delete;

	public:
		bool detectBoundary(void);

	protected:
		bool resizeToMatchSource(void);

	private:
		struct BoundaryINNImpl;
		std::unique_ptr<BoundaryINNImpl> d_ptr;
		BoundaryINNImpl& impl(void) const;
	};
}

