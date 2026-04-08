#pragma once

#include "OctPatternDef.h"
#include "PatternScan.h"

#include <memory>


namespace OctPattern
{
	class OCTPATTERN_DLL_API MeasureScan : public PatternScan
	{
	public:
		MeasureScan();
		virtual ~MeasureScan();

		MeasureScan(MeasureScan&& rhs);
		MeasureScan& operator=(MeasureScan&& rhs);
		MeasureScan(const MeasureScan& rhs);
		MeasureScan& operator=(const MeasureScan& rhs);

	public:
		virtual bool buildPattern(bool hidden=false);

	protected:
		virtual void buildPatternPoint(void);
		virtual void buildPatternLine(bool isVert, int lineCount = 0);
		virtual void buildPatternCircle(void);
		virtual void buildPatternCross(void);
		virtual void buildPatternRadial(void);
		virtual void buildPatternRaster(bool isVert);
		virtual void buildPatternCube(bool isVert);

	private:
		struct MeasureScanImpl;
		std::unique_ptr<MeasureScanImpl> d_ptr;
		MeasureScanImpl& getImpl(void) const;
	};
}
