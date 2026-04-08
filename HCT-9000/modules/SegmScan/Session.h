#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SegmScanDef.h"
#include "ScanRange.h"


namespace SegmScan
{
	class PatternOutput;
	class LineOutput;
	class CubeOutput;
	class RasterOutput;
	class MacularReport;

	class SEGMSCAN_DLL_API Session
	{
	public:
		Session();
		virtual ~Session();

		Session(Session&& rhs);
		Session& operator=(Session&& rhs);
		// Session(const Session& rhs);
		// Session& operator=(const Session& rhs);

	private:
		struct SessionImpl;
		std::unique_ptr<SessionImpl> d_ptr;

	public:
		void setPatternOutput(PatternOutput* output);
		PatternOutput* getPatternOutput(void);
		
		LineOutput* createLineOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, float axialRes=AXIAL_RESOLUTION);
		CubeOutput* createCubeOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, bool vertical=false, float axialRes=AXIAL_RESOLUTION);
		RasterOutput* createRasterOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, float axialRes = AXIAL_RESOLUTION);
		bool executeAnalysis(void);

		MacularReport* getMacularReport(void) const;
		bool updateMacularReport(void);
	};
}


