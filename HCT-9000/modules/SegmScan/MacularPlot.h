#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>


namespace RetParam {
	class EnfacePlot;
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


	class SEGMSCAN_DLL_API MacularPlot 
	{
	public:
		MacularPlot(PatternOutput* output);
		virtual ~MacularPlot();

		MacularPlot(MacularPlot&& rhs);
		MacularPlot& operator=(MacularPlot&& rhs);
		MacularPlot(const MacularPlot& rhs);
		MacularPlot& operator=(const MacularPlot& rhs);

	private:
		struct MacularPlotImpl;
		std::unique_ptr<MacularPlotImpl> d_ptr;

		CppUtil::CvImage* getImage(void) const;
		RetParam::EnfacePlot* getPlot(void) const;

	public:
		void setPatternOutput(PatternOutput* output);
		PatternOutput* getPatternOutput(void) const;

		bool update(int width, int height, LayerType upper, LayerType lower);
		Gdiplus::Bitmap* getBitmap(void) const;
		std::vector<float> copyData(void) const;
		float getAt(int x, int y) const;

		int getWidth(void) const;
		int getHeight(void) const;
		bool isEmpty(void) const;

		BscanResult* getBscanResultFromPosition(int posX, int posY) const;
		BscanImage* getBscanImageFromPosition(int posX, int posY) const;
		int getBscanIndexFromPosition(int posX, int posY) const;
		bool getBscanStartPosition(int resultIdx, int* posX, int* posY) const;
		bool getBscanEndPosition(int resultIdx, int* posX, int* posY) const;

		float getSectionThickness(Section sect) const;
		float getAverageThickness(Section sect) const;
	};

	typedef std::vector<MacularPlot> MacularPlotVect;
}

