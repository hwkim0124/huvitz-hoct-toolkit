#pragma once

#include "SemtRetinaDef.h"


namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API RetinaBandExtractor
	{
	public:
		RetinaBandExtractor(RetinaSegmenter* segm);
		virtual ~RetinaBandExtractor();

		RetinaBandExtractor(RetinaBandExtractor&& rhs);
		RetinaBandExtractor& operator=(RetinaBandExtractor&& rhs);
		RetinaBandExtractor(const RetinaBandExtractor& rhs) = delete;
		RetinaBandExtractor& operator=(const RetinaBandExtractor& rhs) = delete;

	public:
		bool estimateHorizontalBounds(void);
		bool detectInnerRetinaBoundary(void);
		bool detectOuterRetinaBoundary(void);
		bool detectOpticNerveHeadRegion(void);

		void setNerveHeadRangeX(int x1, int x2) const;
		bool getNerveHeadRangeX(int& x1, int& x2) const;
		bool isNerveHeadRangeValid(void) const;
		bool isRetinaOnNerveHeadMarginLeft(void) const;
		bool isRetinaOnNerveHeadMarginRight(void) const;
		bool isRetinaOnNerveHeadMarginBoth(void) const;

		std::vector<int> innerYs(void) const;
		std::vector<int> outerYs(void) const;
		std::vector<int> innerYsFull(void) const;
		std::vector<int> outerYsFull(void) const;

		int retinaBeginX(void) const;
		int retinaEndX(void) const;
		int opticDiscMinX(void) const;
		int opticDiscMaxX(void) const;


	protected:

	private:
		struct RetinaBandExtractorImpl;
		std::unique_ptr<RetinaBandExtractorImpl> d_ptr;
		RetinaBandExtractorImpl& impl(void) const;
	};
}
