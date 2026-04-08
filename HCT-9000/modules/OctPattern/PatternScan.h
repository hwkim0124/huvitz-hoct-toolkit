#pragma once

#include "OctPatternDef.h"

#include <memory>
#include <vector>


namespace OctPattern
{
	class LineTrace;
	class PatternFrame;

	class OCTPATTERN_DLL_API PatternScan
	{
	public:
		PatternScan();
		virtual ~PatternScan();

		PatternScan(PatternScan&& rhs);
		PatternScan& operator=(PatternScan&& rhs);
		PatternScan(const PatternScan& rhs);
		PatternScan& operator=(const PatternScan& rhs);

	public:
		virtual void initialize(PatternDomain domain, PatternType type,
						int numPoints = 1024, int numLines = 1,
						float rangeX = 6.0f, float rangeY = 0.0f, int overlaps = 1, float lineSpace = 0.0f,
						float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
						float scaleX = 0.0f, float scaleY = 0.0f, 
						float posMoveX = 0.0f, float posMoveY = 0.0f);

		virtual bool buildPattern(bool hidden=false);

		bool usePreviewPattern(bool isset = false, bool flag = false);
		bool useFastRasters(bool isset = false, bool flag = false);
		bool useForeDistance(bool isset = false, bool flag = false);

		void setScanRange(float rangeX, float rangeY);
		void setScanAngle(float angle);
		void setScanOffset(float offsetX, float offsetY);
		void setScanScale(float scaleX, float scaleY);
		void setScanOverlaps(int overlaps);
		void setScanLineSpace(float space);
		void setNumberOfScanPoints(int numPoints);
		void setNumberOfScanLines(int numLines);

		float getScanRangeX(void) const;
		float getScanRangeY(void) const;
		float getScanLineSpace(void) const;
		float getScanAngle(void) const;
		float getScanOffsetX(bool valid = true) const;
		float getScanOffsetY(bool valid = true) const;
		float getScanScaleX(void) const;
		float getScanScaleY(void) const;
		float getScanMoveX(void) const;
		float getScanMoveY(void) const;

		int getNumberOfScanPoints(void) const;
		int getNumberOfScanLines(void) const;
		int getNumberOfScanOverlaps(void) const;

		int getFrameCapacityFromPoints(int numPoints, int numLines, int numOverlaps, bool enface = false) const;
		int getNumberOfFramesFromPoints(int numPoints, int numLines, int numOverlaps, bool enface = false) const;

		bool isVertical(void) const;
		bool isCornea(void) const;
		bool isScan3D(void) const;
		bool isRaster(void) const;
		PatternType getPatternType(void) const;

		std::vector<PatternFrame>& getPatternFrames(void) const;
		void clearPatternFrames(void);
		int resizePatternFrames(int size);
		int getNumberOfPatternFrames(void) const;
		PatternFrame* getPatternFrame(int index) const;
		void addPatternFrame(PatternFrame& frame);

		std::vector<int> getLateralSizeListOfFrame(int index, bool repeat) const;
		LineTrace* getLineTraceFromImageIndex(int index, bool repeat, bool preview = false) const;
		int getLineIndexFromImageIndex(int index, bool repeat, bool preview = false) const;
		int getOverlapIndexFromImageIndex(int index, bool repeat, bool preview = false) const;

 	protected:
		virtual void buildPatternPoint(void);
		virtual void buildPatternLine(bool isVert, int lineCount=0);
		virtual void buildPatternCircle(void);
		virtual void buildPatternCross(void);
		virtual void buildPatternRadial(void);
		virtual void buildPatternRaster(bool isVert);
		virtual void buildPatternCube(bool isVert);

	private:
		struct PatternScanImpl;
		std::unique_ptr<PatternScanImpl> d_ptr;
		PatternScanImpl& getImpl(void) const;
	};

	typedef std::vector<PatternFrame> PatternFrameVect;
}

