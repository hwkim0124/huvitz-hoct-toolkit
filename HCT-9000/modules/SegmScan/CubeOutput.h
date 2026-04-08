#pragma once

#include "SegmScanDef.h"
#include "PatternOutput.h"


namespace SegmScan
{
	class SEGMSCAN_DLL_API CubeOutput : public PatternOutput
	{
	public:
		CubeOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, bool vertical = false, float axialRes = AXIAL_RESOLUTION);
		virtual ~CubeOutput();

		CubeOutput(CubeOutput&& rhs);
		CubeOutput& operator=(CubeOutput&& rhs);
		CubeOutput(const CubeOutput& rhs);
		CubeOutput& operator=(const CubeOutput& rhs);

	private:
		struct CubeOutputImpl;
		std::unique_ptr<CubeOutputImpl> d_ptr;

	public:
		bool isVerticalDirection(void) const;

		int getBscanIndexFromPosition(float mmX, float mmY) const;
		int getBscanIndexFromPosition(int posX, int posY, int width, int height) const;

		bool getBscanStartPosition(int index, float* mmX, float* mmY, bool center = true) const;
		bool getBscanStartPosition(int index, int width, int height, int* posX, int* posY) const;
		bool getBscanEndPosition(int index, float* mmX, float* mmY, bool center = true) const;
		bool getBscanEndPosition(int index, int width, int height, int* posX, int* posY) const;
	};
}
