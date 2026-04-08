#pragma once

#include "SegmScanDef.h"
#include "PatternOutput.h"


namespace SegmScan
{
	class SEGMSCAN_DLL_API RasterOutput : public PatternOutput
	{
	public:
		RasterOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, float axialRes);
		virtual ~RasterOutput();

		RasterOutput(RasterOutput&& rhs);
		RasterOutput& operator=(RasterOutput&& rhs);
		RasterOutput(const RasterOutput& rhs);
		RasterOutput& operator=(const RasterOutput& rhs);

	private:
		struct RasterOutputImpl;
		std::unique_ptr<RasterOutputImpl> d_ptr;
		RasterOutputImpl& getImpl(void) const;

	public:
		int getBscanIndexFromPosition(float mmX, float mmY) const;
		int getBscanIndexFromPosition(int posX, int posY, int width, int height) const;

		bool getBscanStartPosition(int index, float* mmX, float* mmY, bool center = true) const;
		bool getBscanStartPosition(int index, int width, int height, int* posX, int* posY) const;
		bool getBscanEndPosition(int index, float* mmX, float* mmY, bool center = true) const;
		bool getBscanEndPosition(int index, int width, int height, int* posX, int* posY) const;
	};
}
