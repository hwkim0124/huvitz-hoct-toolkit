#pragma once

#include "SegmScanDef.h"

#include <memory>


namespace SegmScan
{
	struct SEGMSCAN_DLL_API ScanPoint
	{
	public:
		ScanPoint() :
			x(0.0f), y(0.0f)
		{
		}
		ScanPoint(float xPos, float yPos) {
			x = xPos; y = yPos;
		}

	public:
		float x;
		float y;
	};


	class SEGMSCAN_DLL_API ScanRange
	{
	public:
		ScanRange();
		virtual ~ScanRange();

		ScanRange(ScanRange&& rhs);
		ScanRange& operator=(ScanRange&& rhs);
		ScanRange(const ScanRange& rhs);
		ScanRange& operator=(const ScanRange& rhs);

		ScanRange(const ScanPoint& start, const ScanPoint& end);
		ScanRange(float startX, float startY, float endX, float endY);
		ScanRange(float centerX, float centerY, float radius);

	private:
		struct ScanRangeImpl;
		std::unique_ptr<ScanRangeImpl> d_ptr;

	public:
		void set(const ScanPoint& start, const ScanPoint& end);
		void set(const ScanPoint& start, const ScanPoint& end, float degree);
		void set(const ScanPoint& center, float radius);

		ScanPoint& getStart(void) const;
		ScanPoint& getEnd(void) const;
		ScanPoint& getCenter(void) const;
		float getWidth(void) const;
		float getHeight(void) const;
		float getDegree(void) const;
		float getRadius(void) const;
		
		bool isHorizontalLine(void) const;
		bool isVerticalLine(void) const;
		bool isCircle(void) const;
		bool hitTest(float xPos, float yPos, float offset = LINE_HITTEST_OFFSET) const;
	};

}