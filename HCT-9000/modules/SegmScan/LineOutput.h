#pragma once

#include "SegmScanDef.h"
#include "PatternOutput.h"


namespace SegmScan
{
	class SEGMSCAN_DLL_API LineOutput : public PatternOutput
	{
	public:
		LineOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, float axialRes = AXIAL_RESOLUTION);
		virtual ~LineOutput();

		LineOutput(LineOutput&& rhs);
		LineOutput& operator=(LineOutput&& rhs);
		LineOutput(const LineOutput& rhs);
		LineOutput& operator=(const LineOutput& rhs);

	private:
		struct LineOutputImpl;
		std::unique_ptr<LineOutputImpl> d_ptr;
		LineOutputImpl& getImpl(void) const;

	public:
		int getBscanIndexFromPosition(float mmX, float mmY) const;
		int getBscanIndexFromPosition(int posX, int posY, int width, int height) const;

		bool getBscanStartPosition(int index, float* mmX, float* mmY, bool center = true) const;
		bool getBscanStartPosition(int index, int width, int height, int* posX, int* posY) const;
		bool getBscanEndPosition(int index, float* mmX, float* mmY, bool center = true) const;
		bool getBscanEndPosition(int index, int width, int height, int* posX, int* posY) const;
	};
}
