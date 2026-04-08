#pragma once

#include "OctGlobalDef.h"
#include "GlobalPatternDef.h"
#include "GlobalMeasureDef.h"

#include <cstdint>

namespace OctGlobal
{
	struct OctScanImage
	{
	public:
		const std::uint8_t* data = nullptr;
		int width = 0;
		int height = 0;
		float quality = 0.0f;
		float sigRatio = 0.0f;
		int refPoint = 0;

	public:
		void set(const std::uint8_t* data_, int width_, int height_, float quality_, float sigRatio_, int refPoint_) {
			data = data_;
			width = width_;
			height = height_;
			quality = quality_;
			sigRatio = sigRatio_;
			refPoint = refPoint_;
		}

		const std::uint8_t* getData(void) const {
			return data;
		}

		int getWidth(void) const {
			return width;
		}

		int getHeight(void) {
			return height;
		}

		float getQuality(void) const {
			return quality;
		}

		float getSignalRatio(void) const {
			return sigRatio;
		}

		int getReferencePoint(void) const {
			return refPoint;
		}
	};


	struct OctScanSection
	{
	public:
		OctRoute route;

	public:
		void set(OctRoute route_) {
			route = route_;
		}
	};


	struct OctScanOffset
	{
	public:
		float offsetX = 0.0f;
		float offsetY = 0.0f;
		float angle = 0.0f;
		float scaleX = 1.0f;
		float scaleY = 1.0f;

	public:
		void set(float offsetX_ = 0.0f, float offsetY_ = 0.0f, float angle_ = 0.0f,
					float scaleX_ = 1.0f, float scaleY_ = 1.0f) {
			offsetX = offsetX_;
			offsetY = offsetY_;
			angle = angle_;
			scaleX = scaleX_;
			scaleY = scaleY_;
		}

		void init(void) {
			offsetX = 0.0f;
			offsetY = 0.0f;
			angle = 0.0f;
			scaleX = 1.0f;
			scaleY = 1.0f;
		}

		float getScanAngle(void) const {
			return angle;
		}

		float getScanOffsetX(void) const {
			return offsetX;
		}

		float getScanOffsetY(void) const {
			return offsetY;
		}

		float getScanScaleX(void) const {
			return scaleX;
		}

		float getScanScaleY(void) const {
			return scaleY;
		}
	};


	struct OctScanPattern
	{
	public:
		OctGlobal::EyeSide side = EyeSide::UNKNOWN;
		PatternName name = PatternName::Unknown;
		PatternDomain domain = PatternDomain::Unknown;
		PatternType type = PatternType::Unknown;

		int numPoints = 0;
		int numLines = 0;
		float rangeX = 0.0f;
		float rangeY = 0.0f;
		int overlaps = 0;
		int direction = 0;
		float lineSpace = 0.0f;

		ScanSpeed speed = ScanSpeed::FASTEST;
		OctScanOffset scanOffset;

	public:
		void set(EyeSide side_, PatternName name_, int numPoints_, int numLines_,
				float rangeX_, float rangeY_, int direction_ = 0, int overlaps_ = 1, 
				float lineSpace_ = 0.0f, ScanSpeed speed_ = ScanSpeed::FASTEST) {
			side = side_;
			name = name_;
			numPoints = numPoints_;
			numLines = numLines_;
			rangeX = rangeX_;
			rangeY = rangeY_;
			direction = direction_;
			overlaps = overlaps_;
			lineSpace = lineSpace_;
			speed = speed_;
		}

		void set(EyeSide side_, PatternDomain domain_, PatternType type_, int numPoints_, int numLines_,
				float rangeX_, float rangeY_, int direction_ = 0, int overlaps_ = 1, 
				float lineSpace_ = 0.0f, ScanSpeed speed_ = ScanSpeed::FASTEST) {
			side = side_;
			domain = domain_;
			type = type_;
			numPoints = numPoints_;
			numLines = numLines_;
			rangeX = rangeX_;
			rangeY = rangeY_;
			direction = direction_;
			overlaps = overlaps_;
			lineSpace = lineSpace_;
			speed = speed_;
		}

		void init(void) {
			side = EyeSide::UNKNOWN;
			name = PatternName::Unknown;
			domain = PatternDomain::Unknown;
			type = PatternType::Unknown;

			numPoints = 0;
			numLines = 0;
			rangeX = 0.0f;
			rangeY = 0.0f;
			overlaps = 0;
			direction = 0;
			lineSpace = 0.0f;

			speed = ScanSpeed::FASTEST;
			scanOffset.init();
			return;
		}

		void initAsEnfaceDefault(void) {
			init();

			domain = PatternDomain::Macular;
			type = PatternType::HorzCube;

			numPoints = PATTERN_ENFACE_NUM_ASCAN_POINTS;
			numLines = PATTERN_ENFACE_NUM_BSCAN_LINES;
			rangeX = PATTERN_ENFACE_RANGE_X;
			rangeY = PATTERN_ENFACE_RANGE_Y;
			return;
		}

		void setScanOffset(float offsetX_ = 0.0f, float offsetY_ = 0.0f, float angle_ = 0.0f,
							float scaleX_ = 1.0f, float scaleY_ = 1.0f) {
			scanOffset.offsetX = offsetX_;
			scanOffset.offsetY = offsetY_;
			scanOffset.angle = angle_;
			scanOffset.scaleX = scaleX_;
			scanOffset.scaleY = scaleY_;
		}

		void setScanOffset(OctScanOffset offset_) {
			scanOffset = offset_;
		}

		void setScanSpeed(ScanSpeed speed_) {
			speed = speed_;
		}

		bool isNormalSpeed(void) {
			return (speed == ScanSpeed::NORMAL);
		}

		EyeSide getEyeSide(void) const {
			return side;
		}

		PatternName getPatternName(void) const {
			return name;
		}

		PatternDomain getPatternDomain(void) const {
			return domain;
		}

		PatternType getPatternType(void) const {
			return type;
		}

		int getNumberOfScanPoints(void) const {
			return numPoints;
		}

		int getNumberOfScanLines(void) const {
			return numLines;
		}

		float getScanRangeX(void) const {
			return rangeX;
		}

		float getScanRangeY(void) const {
			return rangeY;
		}

		int getScanOverlaps(void) const {
			return overlaps;
		}

		float getScanLineSpace(void) const {
			return lineSpace;
		}
	};


	struct OctScanMeasure
	{
	public:
		OctScanPattern pattern;
		OctScanPattern enface;

		bool phaseEnface = false;
		bool fastRaster = false;
		bool previewPattern = false;

	public:
		void set(OctScanPattern pattern_,
			bool phaseEnface_ = false, bool fastRaster_ = false, bool previewPattern_ = false) {
			setPattern(pattern_);
			getEnface().initAsEnfaceDefault();

			usePhaseEnface(true, phaseEnface_);
			useFastRaster(true, fastRaster_);
			usePreviewPattern(true, previewPattern_);
		}


		void set(OctScanPattern pattern_, OctScanPattern enface_,
			bool phaseEnface_ = false, bool fastRaster_ = false, bool previewPattern_ = false) {
			setPattern(pattern_);
			setEnface(enface_);

			usePhaseEnface(true, phaseEnface_);
			useFastRaster(true, fastRaster_);
			usePreviewPattern(true, previewPattern_);
		}


		void setPattern(OctScanPattern patt_) {
			pattern = patt_;
		}


		void setEnface(OctScanPattern patt_) {
			enface = patt_;
		}


		OctScanPattern& getPattern(void) {
			return pattern;
		}


		OctScanPattern& getEnface(void) {
			return enface;
		}


		bool usePhaseEnface(bool set_ = false, bool flag = false) {
			if (set_) {
				phaseEnface = flag;
			}
			return phaseEnface;
		}


		bool useFastRaster(bool set_ = false, bool flag = false) {
			if (set_) {
				fastRaster = flag;
			}
			return fastRaster;
		}


		bool usePreviewPattern(bool set_ = false, bool flag = false) {
			if (set_) {
				previewPattern = flag;
			}
			return previewPattern;
		}


		void applyScanOffset(OctScanOffset offset) {
			pattern.scanOffset = offset;
			enface.scanOffset = offset;
			return;
		}
	};
}
