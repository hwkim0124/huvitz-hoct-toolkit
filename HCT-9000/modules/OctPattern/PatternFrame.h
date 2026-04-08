#pragma once

#include "OctPatternDef.h"

#include <memory>
#include <vector>


namespace OctPattern
{
	class LineTrace;

	class OCTPATTERN_DLL_API PatternFrame
	{
	public:
		PatternFrame();
		virtual ~PatternFrame();

		PatternFrame(PatternFrame&& rhs);
		PatternFrame& operator=(PatternFrame&& rhs);
		PatternFrame(const PatternFrame& rhs);
		PatternFrame& operator=(const PatternFrame& rhs);

	public:
		void addLineTrace(LineTrace& trace);
		std::vector<LineTrace>& getLineTraceList(void) const;
		int getLineTraceListSize(void) const;
		void clearLineTraceList(void);

		LineTrace* getLineTraceFirst(void) const;
		LineTrace* getLineTrace(int index) const;

		int getLateralSizeFirst(void) const;
		std::vector<int> getLateralSizeList(bool repeat) const;

		int getNumberOfLineTraces(bool repeat) const;

		int getPositionOffsetX(int index1 = 0, int index2 = 1, int xPos = 0);
		int getPositionOffsetY(int index1 = 0, int index2 = 1, int yPos = 0);

	private:
		struct PatternFrameImpl;
		std::unique_ptr<PatternFrameImpl> d_ptr;
		PatternFrameImpl& getImpl(void) const;
	};

	typedef std::vector<PatternFrame> PatternFrameVect;
}
