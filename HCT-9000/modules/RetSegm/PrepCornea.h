#pragma once

#include "RetSegmDef.h"
#include "Preprocess.h"


namespace RetSegm
{
	class ImageMat;
	class SegmImage;

	class RETSEGM_DLL_API PrepCornea : public Preprocess
	{
	public:
		PrepCornea();
		~PrepCornea();

	public:
		static bool createSampleImage(const SegmImage* imgSrc, SegmImage* imgOut, int width = SAMPLE_IMAGE_WIDTH, int height = SAMPLE_IMAGE_HEIGHT);
		static bool checkIfValidScanImage(SegmImage* imgSrc);
	};
}

