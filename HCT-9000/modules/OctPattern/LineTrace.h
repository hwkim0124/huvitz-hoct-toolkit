#pragma once

#include "OctPatternDef.h"


#include <memory>
#include <vector>
#include <string>


namespace OctGlobal {
	struct OctPoint;
}


namespace OctPattern
{
	class OCTPATTERN_DLL_API LineTrace
	{
	public:
		LineTrace();
		virtual ~LineTrace();

		LineTrace(LineTrace&& rhs);
		LineTrace& operator=(LineTrace&& rhs);
		LineTrace(const LineTrace& rhs);
		LineTrace& operator=(const LineTrace& rhs);

	public:
		void initLine(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats = 1, int forePadds = TRIGGER_FORE_PADDING_POINTS, int postPadds = TRIGGER_POST_PADDING_POINTS);
		void initCircle(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats = 1, int forePadds = TRIGGER_FORE_PADDING_POINTS, int postPadds = TRIGGER_POST_PADDING_POINTS);
		void initHorzRaster(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats = 1, int forePadds = TRIGGER_FORE_PADDING_POINTS, int postPadds = TRIGGER_POST_PADDING_POINTS);
		void initVertRaster(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats = 1, int forePadds = TRIGGER_FORE_PADDING_POINTS, int postPadds = TRIGGER_POST_PADDING_POINTS);
		void initHorzRasterFast(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats = 1, int forePadds = TRIGGER_FORE_PADDING_POINTS, int postPadds = TRIGGER_POST_PADDING_POINTS);
		void initVertRasterFast(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats = 1, int forePadds = TRIGGER_FORE_PADDING_POINTS, int postPadds = TRIGGER_POST_PADDING_POINTS);
		void setPaddings(int forePadds, int postPadds, int numPoints = 0);

		OctRoute getRouteOfScan(void) const;
		LineTraceType getTraceType(void) const;
		std::string getTraceTypeStr(void) const;

		int getTraceId(void) const;
		int getForePaddings(void) const;
		int getPostPaddings(void) const;
		int getNumberOfScanPoints(bool repeats = false) const;
		int getNumberOfRepeats(void) const;
	
		void setScanSpeed(ScanSpeed speed);
		ScanSpeed getScanSpeed(void);

		float getStartX(void) const;
		float getStartY(void) const;
		float getCloseX(void) const;
		float getCloseY(void) const;

		short getPositionX(int index) const;
		short getPositionY(int index) const;

		short* getGalvanoPositionsX(bool reversed = false) const;
		short* getGalvanoPositionsY(bool reversed = false) const;
		void setGalvanoPositionsX(short* posXs, short count);
		void setGalvanoPositionsY(short* posYs, short count);
		int getCountOfPositionsX(void) const;
		int getCountOfPositionsY(void) const;

		bool isLine(void) const;
		bool isCircle(void) const;
		bool isRasterX(void) const;
		bool isRasterY(void) const;
		bool isRasterFastX(void) const;
		bool isRasterFastY(void) const;
		bool isHidden(void) const;
		bool isLineHD(void) const;

		void setHidden(bool flag);
		void setLineHD(bool flag);

	private:
		struct LineTraceImpl;
		std::unique_ptr<LineTraceImpl> d_ptr;
		LineTraceImpl& getImpl(void) const;
	};

	typedef std::vector<LineTrace> LineTraceVect;
}
