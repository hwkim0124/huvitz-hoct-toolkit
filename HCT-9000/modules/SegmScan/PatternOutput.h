#pragma once

#include "SegmScanDef.h"
#include "BscanImage.h"
#include "BscanResult.h"
#include "PatternDescript.h"

#include <memory>
#include <vector>


namespace RetSegm {
	class SegmImage;
	class SegmLayer;
}


namespace SegmScan
{
	class BscanResult;
	class BscanImage;

	class SEGMSCAN_DLL_API PatternOutput
	{
	public:
		PatternOutput(EyeSide side, ScanRegion region, PatternType type, ScanPoint start, ScanPoint end, int size, float axialRes=AXIAL_RESOLUTION);
		virtual ~PatternOutput();

		PatternOutput(PatternOutput&& rhs);
		PatternOutput& operator=(PatternOutput&& rhs);
		PatternOutput(const PatternOutput& rhs);
		PatternOutput& operator=(const PatternOutput& rhs);

	private:
		struct PatternOutputImpl;
		std::unique_ptr<PatternOutputImpl> d_ptr;

	public:
		PatternDescript* getDescript(void) const;
		void setDescript(PatternDescript& descript);

		void addResult(BscanResult& result);
		void addResult(BscanResult&& result);
		BscanResult* getResult(int index) const;
		int getResultCount(void) const;

		BscanImage* getImage(int resultIdx, int imageIdx = 0) const;
		std::vector<BscanImage*> getFirstBscanImages(void) const;
		std::vector<RetSegm::SegmImage*> getSegmImages(ImageType type = ImageType::RESIZED) const;
		std::vector<RetSegm::SegmLayer*> getSegmLayers(LayerType type) const;

		virtual int getBscanIndexFromPosition(float mmX, float mmY) const;
		virtual int getBscanIndexFromPosition(int posX, int posY, int width, int height) const;

		virtual bool getBscanStartPosition(int index, float* mmX, float* mmY, bool center = true) const;
		virtual bool getBscanStartPosition(int index, int width, int height, int* posX, int* posY) const;
		virtual bool getBscanEndPosition(int index, float* mmX, float* mmY, bool center = true) const;
		virtual bool getBscanEndPosition(int index, int width, int height, int* posX, int* posY) const;

	};
}
