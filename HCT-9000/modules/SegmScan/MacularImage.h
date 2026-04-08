#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>


namespace RetParam {
	class EnfaceDraw;
}


namespace CppUtil {
	class CvImage;
}


namespace Gdiplus {
	class Bitmap;
}


namespace SegmScan
{
	class BscanResult;
	class BscanImage;
	class PatternOutput;

	class SEGMSCAN_DLL_API MacularImage
	{
	public:
		MacularImage(PatternOutput *output);
		virtual ~MacularImage();

		MacularImage(MacularImage&& rhs);
		MacularImage& operator=(MacularImage&& rhs);
		MacularImage(const MacularImage& rhs);
		MacularImage& operator=(const MacularImage& rhs);

	protected:
		struct MacularImageImpl;
		std::unique_ptr<MacularImageImpl> d_ptr;

		CppUtil::CvImage* getImage(void) const;
		RetParam::EnfaceDraw* getDraw(void) const;

	public:
		void setPatternOutput(PatternOutput* output);
		PatternOutput* getPatternOutput(void) const;
		
		bool update(int width, int height, LayerType upper, LayerType lower);
		Gdiplus::Bitmap* getBitmap(void) const;
		std::vector<unsigned char> copyData(void) const;
		unsigned char getAt(int x, int y) const;

		int getWidth(void) const;
		int getHeight(void) const;
		bool isEmpty(void) const;

		BscanResult* getBscanResultFromPosition(int posX, int posY) const;
		BscanImage* getBscanImageFromPosition(int posX, int posY) const;
		int getBscanIndexFromPosition(int posX, int posY) const;
		bool getBscanStartPosition(int resultIdx, int* posX, int* posY) const;
		bool getBscanEndPosition(int resultIdx, int* posX, int* posY) const;
	};

	typedef std::vector<MacularImage> MacularImageVect;
}
