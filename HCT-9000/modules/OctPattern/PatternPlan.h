#pragma once

#include "OctPatternDef.h"

#include <memory>


namespace OctPattern
{
	class PreviewScan;
	class MeasureScan;
	class EnfaceScan;

	class OCTPATTERN_DLL_API PatternPlan
	{
	public:
		PatternPlan();
		virtual ~PatternPlan();

		PatternPlan(PatternPlan&& rhs);
		PatternPlan& operator=(PatternPlan&& rhs);
		PatternPlan(const PatternPlan& rhs);
		PatternPlan& operator=(const PatternPlan& rhs);

	public:
		bool buildMeasure(PatternDomain domain, PatternType type,
			int numPoints = 1024, int numLines = 1, float rangeX = 6.0f, float rangeY = 0.0f,
			int overlaps = 1, float lineSpace = 0.0f, bool useEnface = false,
			bool usePattern = false, bool useFaster = false);
		bool buildMeasure(OctScanMeasure& measure);

		void setupEnface(int numPoints = PATTERN_ENFACE_ASCAN_POINTS,
			int numLines = PATTERN_ENFACE_BSCAN_LINES,
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
		void clear(void);

		bool isCornea(void) const;
		bool isEnfaceScan(void) const;

		void exportGalvanoPositions(std::string name = "gavl_points.txt");

		PreviewScan& getPreviewScan(void) const;
		MeasureScan& getMeasureScan(void) const;
		EnfaceScan& getEnfaceScan(void) const;

	protected:
		bool buildPreviewPattern(OctScanMeasure& measure);
		bool buildMeasurePattern(OctScanMeasure& measure);
		bool buildEnfacePattern(OctScanMeasure& measure);

		void updatePatternScale(const OctScanPattern& pattern, float& scaleX, float& scaleY);
		void updatePatternOffset(const OctScanPattern& pattern, float& offsetX, float& offsetY);

	private:
		struct PatternPlanImpl;
		std::unique_ptr<PatternPlanImpl> d_ptr;
		PatternPlanImpl& getImpl(void) const;
	};
}
