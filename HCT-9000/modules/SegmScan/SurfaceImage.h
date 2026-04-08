#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <string>
#include <vector>


namespace Gdiplus {
	class Bitmap;
}


namespace SegmScan
{
	class BscanResult;
	class BscanImage;
	class PatternOutput;


	class SEGMSCAN_DLL_API SurfaceImage
	{
	public:
		SurfaceImage();
		SurfaceImage(float width, float height);
		SurfaceImage(const std::wstring& path, float width, float height);
		SurfaceImage(const unsigned char* pBits, int rows, int cols, float width, float height);
		virtual ~SurfaceImage();

		SurfaceImage(SurfaceImage&& rhs);
		SurfaceImage& operator=(SurfaceImage&& rhs);
		SurfaceImage(const SurfaceImage& rhs);
		SurfaceImage& operator=(const SurfaceImage& rhs);

	private:
		struct SurfaceImageImpl;
		std::unique_ptr<SurfaceImageImpl> d_ptr;
		SurfaceImageImpl& getImpl(void) const;

	public:
		void setPatternOutput(PatternOutput* output);
		PatternOutput* getPatternOutput(void) const;

		Gdiplus::Bitmap* getBitmap(void) const;

		bool loadFile(const std::wstring& path);
		bool loadBitsData(const unsigned char* pBits, int width, int height, int padding = 0);
		bool isEmpty(void) const;

		void setSurfaceRegion(float width, float height);
		float getSurfaceWidth(void) const;
		float getSurfaceHeight(void) const;

		void setDisplaySize(int wpix, int hpix);
		int getWidth(void) const;
		int getHeight(void) const;

		float getRegionWidth(void) const;
		float getRegionHeight(void) const;

		BscanResult* getBscanResultFromPosition(int posX, int posY) const;
		BscanImage* getBscanImageFromPosition(int posX, int posY) const;
		int getBscanIndexFromPosition(int posX, int posY) const;
		bool getBscanStartPosition(int resultIdx, int* posX, int* posY) const;
		bool getBscanEndPosition(int resultIdx, int* posX, int* posY) const;
	};

	typedef std::vector<SurfaceImage> SurfaceImageVect;
}
