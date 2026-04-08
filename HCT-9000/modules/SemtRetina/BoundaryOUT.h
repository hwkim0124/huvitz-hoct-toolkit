#pragma once

#include "SemtRetinaDef.h"
#include "RetinaBoundary.h"


namespace SemtRetina
{
	class RetinaSegmenter;
	class RetinaBoundary;

	class SEMTRETINA_DLL_API BoundaryOUT : public RetinaBoundary
	{
	public:
		BoundaryOUT(RetinaSegmenter* segm);
		virtual ~BoundaryOUT();

		BoundaryOUT(BoundaryOUT&& rhs);
		BoundaryOUT& operator=(BoundaryOUT&& rhs);
		BoundaryOUT(const BoundaryOUT& rhs) = delete;
		BoundaryOUT& operator=(const BoundaryOUT& rhs) = delete;

	public:
		bool detectBoundary(void);

	protected:
		bool resizeToMatchSource(void);

	private:
		struct BoundaryOUTImpl;
		std::unique_ptr<BoundaryOUTImpl> d_ptr;
		BoundaryOUTImpl& impl(void) const;
	};
}

