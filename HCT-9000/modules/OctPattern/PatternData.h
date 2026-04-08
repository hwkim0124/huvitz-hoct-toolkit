#pragma once

#include "OctPatternDef.h"

#include <memory>


namespace OctPattern
{
	class PreviewScan;
	class MeasureScan;
	class EnfaceScan;

	class OCTPATTERN_DLL_API PatternData
	{
	public:
		PatternData();
		virtual ~PatternData();

		PatternData(PatternData&& rhs);
		PatternData& operator=(PatternData&& rhs);
		PatternData(const PatternData& rhs);
		PatternData& operator=(const PatternData& rhs);

	public:
		bool setupPattern(PatternDomain domain, PatternType type,
							int numPoints = 1024, int numLines = 1, float rangeX = 6.0f, float rangeY = 0.0f,
							int overlaps = 1, float lineSpace = 0.0f, bool useEnface = false,
							bool usePattern = false, bool useFaster = false);

		void setupEnface(int numPoints = PATTERN_ENFACE_NUM_ASCAN_POINTS, 
						 int numLines = PATTERN_ENFACE_NUM_BSCAN_LINES,
						 float rangeX = PATTERN_ENFACE_RANGE_X, 
						 float rangeY = PATTERN_ENFACE_RANGE_Y);

		void setupDisplacement(float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
							   float scaleX = 1.0f, float scaleY = 1.0f);

		int getPreviewScanPoints(void) const;
		int getMeasureScanPoints(void) const;
		int getEnfaceScanPoints(void) const;
		
		int getPreviewScanLines(void) const;
		int getMeasureScanLines(void) const;
		int getEnfaceScanLines(void) const;

		bool isInitiated(void) const;
		bool isEnfaceScan(void) const;
		void clear(void);

		bool isCornea(void);

		PreviewScan& getPreviewScan(void) const;
		MeasureScan& getMeasureScan(void) const;
		EnfaceScan& getEnfaceScan(void) const;

	private:
		struct PatternDataImpl;
		std::unique_ptr<PatternDataImpl> d_ptr;
		PatternDataImpl& getImpl(void) const;
	};
}
