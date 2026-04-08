#pragma once

#include "RetSegmDef.h"

namespace RetSegm
{
	class ImageMat;
	class SegmImage;

	class RETSEGM_DLL_API Preprocess
	{
	public:
		Preprocess();


	public:
		static bool createSampleImage(const SegmImage* imgSrc, SegmImage* imgOut, int width = SAMPLE_IMAGE_WIDTH, int height = SAMPLE_IMAGE_HEIGHT);
		static bool smoothBackgroundNoise(const SegmImage* imgSrc, SegmImage* imgOut);
		static bool collectImageStats(SegmImage* imgSrc);
		static bool collectColumnStats(SegmImage* imgSrc);

		static bool checkIfValidScanImage(SegmImage* imgSrc, int index);
		static bool checkIfValidScanImageOfCornea(SegmImage* imgSrc, int index);

	public:
		static bool createSampleImageByRatio(const SegmImage* imgSrc, SegmImage* imgOut, float widthRatio, float heightRatio);
		static bool createSampleImageBySize(const SegmImage* imgSrc, SegmImage* imgOut, int targetWidth, int targetHeight);
		static bool suppressMoirePattern(SegmImage* imgSrc);
		static bool checkValidityOfScanImage(SegmImage* srcImg);

	public:
		static bool createSampleImage(const ImageMat& srcImage, ImageMat& outImage, float widthRatio, float heightRatio);
		static bool createSampleImage(const ImageMat& srcImage, ImageMat& outImage, int targetWidth, int targetHeight);
		static bool collectImageStats(ImageMat& srcImg);
		static bool collectColumnStats(ImageMat& srcImg);
		static bool smoothBackgroundNoise(ImageMat& srcImg, ImageMat& outImg);
		static bool suppressMoirePattern(ImageMat& srcImg);
		static bool checkValidityOfScanImage(ImageMat& srcImg);
	};
}

