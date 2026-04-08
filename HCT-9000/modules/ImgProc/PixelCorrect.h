#pragma once

#include "ImgProcDef.h"

#include <memory>
#include <string>


namespace ImgProc
{
	class IMGPROC_DLL_API PixelCorrect
	{
	public:
		PixelCorrect();
		virtual ~PixelCorrect();

	public:
		static int readWhitePictures(const std::vector<std::wstring>& fileList);
		static int readBlackPictures(const std::vector<std::wstring>& fileList);
		static int detectBadPixels(int numImages = PIXEL_CORRECT_IMAGES_NUM, int width = FUNDUS_FRAME_WIDTH, int height = FUNDUS_FRAME_HEIGHT);

		static int countBadPixels(void) ;
		static int getBadPixelY(int index) ;
		static int getBadPixelX(int index);
		static std::vector<std::pair<unsigned int, unsigned int>>& getBadPixelCoords(void);

		static bool correctBadPixels(unsigned short* frame);
		static bool isDetected(void);

		static void setDarkThresholds(int r, int g, int b);
		static void setBrightThresholds(int r, int g, int b);

		static bool exportBadPixels(std::wstring filename = L"badpixels.txt");

	protected:


	private:
		struct PixelCorrectImpl;
		static std::unique_ptr<PixelCorrectImpl> d_ptr;
		static PixelCorrectImpl& getImpl(void);
	};

}