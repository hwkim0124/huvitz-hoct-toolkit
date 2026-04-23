#pragma once

#include "SemtRetinaDef.h"
#include "CvImage.h"


#include <string>
#include <memory>
#include <vector>

namespace CppUtil {
	class CvImage;
}

namespace SemtRetina
{
	class SEMTRETINA_DLL_API BscanImageData : public CppUtil::CvImage
	{
	public:
		BscanImageData();
		virtual ~BscanImageData();

		BscanImageData(BscanImageData&& rhs);
		BscanImageData& operator=(BscanImageData&& rhs);
		BscanImageData(const BscanImageData& rhs);
		BscanImageData& operator=(const BscanImageData& rhs);

	public:
		void estimateStatitics(void);
		void estimateColSnRatios(float percentBgd = 0.25f, float percentObj = 0.99f);
		void resizeToMatchSample(int sampleWidth, int sampleHeight);
		void upscaleStatistics(int width, int height);

		float imageMean(void) const;
		float imageStdev(void) const;

		const std::vector<float>& columnMeans(void) const;
		const std::vector<float>& columnBgdMeans(void) const;
		const std::vector<float>& columnBgdStdev(void) const;
		const std::vector<float>& columnObjMeans(void) const;
		const std::vector<float>& columnObjStdev(void) const;
		const std::vector<float>& columnSnRatios(void) const;

		const std::vector<float>& rowMeans(void) const;

	private:
		struct BscanImageDataImpl;
		std::unique_ptr<BscanImageDataImpl> d_ptr;
		BscanImageDataImpl& impl(void) const;
	};
}
