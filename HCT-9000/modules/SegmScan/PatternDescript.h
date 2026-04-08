#pragma once


#include "SegmScanDef.h"
#include "ScanRange.h"

#include <memory>


namespace SegmScan
{
	class SEGMSCAN_DLL_API PatternDescript
	{
	public:
		PatternDescript();
		PatternDescript(EyeSide side, ScanRegion region, PatternType type, int size, float axialRes);
		PatternDescript(EyeSide side, ScanRegion region, PatternType type, ScanPoint start, ScanPoint end, int size, float axialRes);
		virtual ~PatternDescript();

		PatternDescript(PatternDescript&& rhs);
		PatternDescript& operator=(PatternDescript&& rhs);
		PatternDescript(const PatternDescript& rhs);
		PatternDescript& operator=(const PatternDescript& rhs);


	private:
		struct PatternDescriptImpl;
		std::unique_ptr<PatternDescriptImpl> d_ptr;


	public:
		EyeSide getSide(void) const;
		ScanRegion getRegion(void) const;
		PatternType getType(void) const;
		int getNumberOfBscans(void) const;
		float getAxialResolution(void) const;

		void setSide(EyeSide side);
		void setRegion(ScanRegion region);
		void setType(PatternType type);
		void setNumberOfBscans(int size);
		void setAxialResolution(float axialRes);
		void setRange(ScanRange range);

		ScanPoint& getStartPos(void) const;
		ScanPoint& getEndPos(void) const;
		float getWidth(void) const;
		float getHeight(void) const;

		bool isLinePattern(void) const;
		bool isCubePattern(void) const;
	};
}
