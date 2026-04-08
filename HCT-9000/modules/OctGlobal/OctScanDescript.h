#pragma once

#include "OctGlobalDef.h"
#include "GlobalPatternDef.h"
#include "GlobalMeasureDef.h"
#include "PatternHelper.h"

#include "OctRoute.h"
#include "OctPoint.h"
#include "OctScanResult.h"

#include <cstdint>
#include <string>
#include <sstream>


namespace OctGlobal
{
	struct OctScanOffset
	{
	public:
		float _offsetX = 0.0f;
		float _offsetY = 0.0f;
		float _angle = 0.0f;
		float _scaleX = 1.0f;
		float _scaleY = 1.0f;
		bool _mmAsUnit = true;

	public:
		OctScanOffset() {
			init();
		}

		OctScanOffset(float offsetX, float offsetY, float angle = 0.0f,
						float scaleX = 1.0f, float scaleY = 1.0f) {
			setup(offsetX, offsetY, angle, scaleX, scaleY, true);
		}

		void set(float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
				float scaleX = 1.0f, float scaleY = 1.0f, bool mmAsUnit = true) {
			setup(offsetX, offsetY, angle, scaleX, scaleY, mmAsUnit);
		}

		void setup(float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
					float scaleX = 1.0f, float scaleY = 1.0f, bool mmAsUnit = true) {
			_offsetX = offsetX;
			_offsetY = offsetY;
			_angle = angle;
			_scaleX = scaleX;
			_scaleY = scaleY;
			_mmAsUnit = mmAsUnit;
		}

		void init(void) {
			setup();
		}

		float getScanAngle(void) const {
			return _angle;
		}

		float getScanOffsetX(void) const {
			return _offsetX;
		}

		float getScanOffsetY(void) const {
			return _offsetY;
		}

		float getScanScaleX(void) const {
			return _scaleX;
		}

		float getScanScaleY(void) const {
			return _scaleY;
		}
	};


	struct OctScanPattern
	{
	public:
		PatternName _name = PatternName::Macular3D;
		PatternDomain _domain = PatternDomain::Macular;
		PatternType _type = PatternType::HorzCube;
		PatternType _previewType = PatternType::HorzLine;

		int _numPoints = 512;
		int _numLines = 96;
		float _rangeX = 6.0f;
		float _rangeY = 6.0f;
		int _overlaps = 1;
		int _direction = 0;
		float _lineSpace = 0.0f;

		ScanSpeed _speed = ScanSpeed::Fastest;
		OctScanOffset _offset;

	public:
		OctScanPattern() {
			init();
		}

		void setup(PatternName name, int numPoints, int numLines,
					float rangeX, float rangeY, int direction = 0, int overlaps = 1,
					float lineSpace = 0.0f, ScanSpeed speed = ScanSpeed::Fastest) {
			_name = name;

			auto code = PatternHelper::getPatternCodeFromName(_name);
			_domain = PatternHelper::getPatternDomainFromCode(code);
			_type = PatternHelper::getPatternTypeFromCode(code, direction);

			_numPoints = numPoints;
			_numLines = numLines;
			_rangeX = rangeX;
			_rangeY = rangeY;
			_direction = direction;
			_overlaps = overlaps;
			_lineSpace = lineSpace;
			_speed = speed;
			return;
		}

		void setup(PatternDomain domain, PatternType type, int numPoints, int numLines,
			float rangeX, float rangeY, int direction = 0, int overlaps = 1,
			float lineSpace = 0.0f, ScanSpeed speed = ScanSpeed::Fastest) {
			_domain = domain;
			_type = type;
			_numPoints = numPoints;
			_numLines = numLines;
			_rangeX = rangeX;
			_rangeY = rangeY;
			_direction = direction;
			_overlaps = overlaps;
			_lineSpace = lineSpace;
			_speed = speed;
			return;
		}

		void init(void) {
			_name = PatternName::Unknown;
			_domain = PatternDomain::Unknown;
			_type = PatternType::Unknown;

			_numPoints = 0;
			_numLines = 0;
			_rangeX = 0.0f;
			_rangeY = 0.0f;
			_overlaps = 1;
			_direction = 0;
			_lineSpace = 0.0f;

			_speed = ScanSpeed::Fastest;
			_offset.init();
			return;
		}

		void setupAsPreviewDefault(void) {
			init();

			_name = PatternName::MacularLine;
			_domain = PatternDomain::Macular;
			_type = PatternType::HorzLine;

			_numPoints = PATTERN_PREVIEW_ASCAN_POINTS;
			_numLines = 1;
			_rangeX = PATTERN_ENFACE_RANGE_X;
			_rangeY = PATTERN_ENFACE_RANGE_Y;
			return;
		}

		void setupAsEnfaceDefault(void) {
			init();

			_name = PatternName::Macular3D;
			_domain = PatternDomain::Macular;
			_type = PatternType::HorzCube;

			_numPoints = PATTERN_ENFACE_ASCAN_POINTS;
			_numLines = PATTERN_ENFACE_BSCAN_LINES;
			_rangeX = PATTERN_ENFACE_RANGE_X;
			_rangeY = PATTERN_ENFACE_RANGE_Y;
			return;
		}

		PatternType getPreviewType(void) {
			return _previewType;
		}

		void setPreviewType(PatternType type) {
			_previewType = type;
		}


		bool isPreviewCross(void) {
			return _previewType == PatternType::Cross;
		}


		bool isPreviewVertLine(void) {
			return _previewType == PatternType::VertLine;
		}


		void setDomain(PatternDomain domain) {
			_domain = domain;
		}

		void setPatternName(PatternName name) {
			_name = name;
		}

		void setScanOffset(float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
						float scaleX = 1.0f, float scaleY = 1.0f, bool mmAsUnit = true) {
			_offset.setup(offsetX, offsetY, angle, scaleX, scaleY, mmAsUnit);
			return;
		}

		void setScanOffset(const OctScanOffset offset) {
			_offset = offset;
			return;
		}

		void setScanSpeed(ScanSpeed speed) {
			_speed = speed;
		}

		bool isNormalSpeed(void) const {
			return (_speed == ScanSpeed::Normal);
		}

		bool isFasterSpeed(void) const {
			return (_speed == ScanSpeed::Faster);
		}

		bool isFastestSpeed(void) const {
			return (_speed == ScanSpeed::Fastest);
		}

		bool isCustomSpeed(void) const {
			return (_speed == ScanSpeed::Custom);
		}

		ScanSpeed getScanSpeed(void) const {
			return _speed;
		}

		OctScanOffset& getScanOffset(void) {
			return _offset;
		}

		PatternName getPatternName(void) const {
			return _name;
		}

		const char* getPatternNameString(void) const {
			return PatternHelper::getPatternNameString(_name);
		}

		PatternDomain getPatternDomain(void) const {
			return _domain;
		}

		PatternType getPatternType(void) const {
			return _type;
		}

		EyeRegion getEyeRegion(void) const {
			return PatternHelper::getEyeRegionFromDomain(_domain);
		}

		bool isCorneaScan(void) const {
			return (getEyeRegion() == EyeRegion::Cornea);
		}

		bool isMacularScan(void) const {
			return (getEyeRegion() == EyeRegion::Macular);
		}

		bool isDiscScan(void) const {
			return (getEyeRegion() == EyeRegion::OpticDisc);
		}

		bool isMeasureScan(void) const {
			if (_name == PatternName::LensThickness ||
				_name == PatternName::AxialLength) {
				return true;
			}
			return false;
		}

		bool isLensThicknessScan(void) const {
			return (_name == PatternName::LensThickness);
		}

		bool isAxialLengthScan(void) const {
			return (_name == PatternName::AxialLength);
		}

		bool isAngioScan(void) const {
			if (_name == PatternName::MacularAngio ||
				_name == PatternName::DiscAngio ||
				_name == PatternName::AnteriorAngio) {
				return true;
			}
			return false;
		}

		bool isCrossScan(void) const {
			switch (_name) {
			case PatternName::MacularCross:
				return true;
			}
			return false;
		}

		bool isRasterScan(void) const {
			switch (_name) {
			case PatternName::MacularRaster:
			case PatternName::DiscRaster:
				return true;
			}
			return false;
		}

		bool isRadialScan(void) const {
			switch (_name) {
			case PatternName::MacularRadial:
			case PatternName::DiscRadial:
			case PatternName::AnteriorRadial:
			case PatternName::Topography:
				return true;
			}
			return false;
		}

		bool isCubeScan(void) const {
			return isEnfaceScan();
		}

		bool isEnfaceScan(void) const {
			switch (_name) {
			case PatternName::Macular3D:
			case PatternName::MacularAngio:
			case PatternName::Disc3D:
			case PatternName::DiscAngio:
			case PatternName::Anterior3D:
			case PatternName::AnteriorAngio:
			case PatternName::MacularDisc:
				return true;
			}
			return false;
		}

		bool isCircleScan(void) const {
			switch (_name) {
			case PatternName::DiscCircle:
			case PatternName::MacularCircle:
				return true;
			}
			return false;
		}

		int getDirection(void) const {
			return _direction;
		}

		bool isHorizontal(void) const {
			return (_direction == 0);
		}

		bool isVertial(void) const {
			return !isHorizontal();
		}

		int getNumberOfScanPoints(void) const {
			return _numPoints;
		}

		int getNumberOfScanLines(void) const {
			return _numLines;
		}

		float getScanRangeX(void) const {
			return _rangeX;
		}

		float getScanRangeY(void) const {
			return _rangeY;
		}

		float getScanDistance(void) const {
			return (isHorizontal() ? getScanRangeX() : getScanRangeY());
		}

		float getScanLength(void) const {
			if (isCircleScan()) {
				return getScanRangeX() * 3.14159265358979f; 
			}
			else {
				return getScanDistance();
			}
		}

		float getStartX(bool offset = true) const {
			float pos = (_rangeX / 2.0f) * -1.0f + (offset ? _offset._offsetX : 0.0f);
			return pos;
		}

		float getCloseX(bool offset = true) const {
			float pos = (_rangeX / 2.0f) * +1.0f + (offset ? _offset._offsetX : 0.0f);
			return pos;
		}

		float getStartY(bool offset = true) const {
			float pos = (_rangeY / 2.0f) * -1.0f + (offset ? _offset._offsetY : 0.0f);
			return pos;
		}

		float getCloseY(bool offset = true) const {
			float pos = (_rangeY / 2.0f) * +1.0f + (offset ? _offset._offsetY : 0.0f);
			return pos;
		}

		OctPoint startPoint(bool offset = true) const {
			return OctPoint(getStartX(offset), getStartY(offset));
		}

		OctPoint closePoint(bool offset = true) const {
			return OctPoint(getCloseX(offset), getCloseY(offset));
		}

		OctPoint centerPoint(bool offset = true) const {
			return (offset ? OctPoint(_offset._offsetX, _offset._offsetY) : OctPoint());
		}

		float startX(bool offset = true) const {
			return getStartX(offset);
		}

		float closeX(bool offset = true) const {
			return getCloseX(offset);
		}

		float startY(bool offset = true) const {
			return getStartY(offset);
		}

		float closeY(bool offset = true) const {
			return getCloseY(offset);
		}

		float centerX(bool offset = true) const {
			return centerPoint(offset)._x;
		}

		float centerY(bool offset = true) const {
			return centerPoint(offset)._y;
		}

		float width(void) const {
			return (float)fabs(closeX() - startX());
		}

		float height(void) const {
			return (float)fabs(closeY() - startY());
		}

		float radiusX(void) const {
			return width() * 2.0f;
		}

		float radiusY(void) const {
			return height() * 2.0f;
		}

		OctRange getScanRange(bool offset = true) const {
			float x1 = getStartX(offset);
			float y1 = getStartY(offset);
			float x2 = getCloseX(offset);
			float y2 = getCloseY(offset);
			return OctRange(x1, y1, x2, y2);
		}

		int getScanOverlaps(void) const {
			return _overlaps;
		}

		float getScanLineSpace(void) const {
			return _lineSpace;
		}

		OctRoute getRouteOfScanLine(int index) const 
		{
			OctRoute route;

			switch (_name) {
			case PatternName::Macular3D:
			case PatternName::Disc3D:
			case PatternName::Anterior3D:
			case PatternName::MacularDisc:
				return getRouteOfCube(index);
			case PatternName::MacularRadial:
			case PatternName::DiscRadial:
			case PatternName::AnteriorRadial:
			case PatternName::Topography:
				return getRouteOfRadial(index);
			case PatternName::MacularRaster:
			case PatternName::DiscRaster:
				return getRouteOfRaster(index);
			case PatternName::MacularCross:
				return getRouteOfCross(index);
			case PatternName::MacularCircle:
			case PatternName::DiscCircle:
				return getRouteOfCircle();
			case PatternName::MacularLine:
			case PatternName::DiscLine:
			case PatternName::AnteriorLine:
			case PatternName::AnteriorFull:
			case PatternName::AxialLength:
			case PatternName::LensThickness:
			default:
				return getRouteOfLine();
			}
		}

		OctScanSection makeScanSection(int index) const
		{
			OctScanSection sect(index, getRouteOfScanLine(index));
			return sect;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<ScanPattern> " : "");
			int code = PatternHelper::getPatternCodeFromName(this->_name);
			ss << "name: " << PatternHelper::getPatternNameStringFromCode(code);
			ss << ", points: " << this->_numPoints << ", lines: " << this->_numLines << "\n";
			ss << "rangeX: " << this->_rangeX << ", rangeY: " << this->_rangeY << ", overlaps: " << this->_overlaps;
			ss << ", direction: " << this->_direction << ", linespace: " << this->_lineSpace << "\n";
			ss << "start: " << startPoint().text(false) << ", close: " << closePoint().text(false);
			return ss.str();
		}

	protected:

		OctRoute getRouteOfLine(void) const {
			if (isHorizontal()) {
				return OctRoute(startX(), centerY(), closeX(), centerY());
			}
			else {
				return OctRoute(centerX(), startY(), centerX(), closeY());
			}
		}

		OctRoute getRouteOfCircle(void) const {
			return OctRoute(startPoint(), closePoint(), true);
		}

		OctRoute getRouteOfCross(int index) const {
			int half = getNumberOfScanLines() / 2;
			float space = getScanLineSpace();
			if (index < half) {
				float y = centerY() - space * (index - half / 2);
				return OctRoute(startX(), y, closeX(), y);
			}
			else {
				float x = centerX() - space * ((index - half)  - half / 2);
				return OctRoute(x, startY(), x, closeY());
			}
		}

		OctRoute getRouteOfRadial(int index) const {
			int lines = getNumberOfScanLines();
			double angle = 180.0 / lines;
			double degree = angle * index;
			double radian = ((degree * 3.14159265358979323) / 180.0); // NumericFunc::degreeToRadian(degree);

			float x1 = (float)(radiusX() * cos(radian) * -1.0 + _offset.getScanOffsetX());
			float y1 = (float)(radiusY() * sin(radian) * -1.0 + _offset.getScanOffsetY());
			float x2 = (float)(radiusX() * cos(radian) * +1.0 + _offset.getScanOffsetX());
			float y2 = (float)(radiusY() * sin(radian) * +1.0 + _offset.getScanOffsetY());

			return OctRoute(x1, y1, x2, y2);
		}

		OctRoute getRouteOfRaster(int index) const {
			if (isHorizontal()) {
				float space = height() / (getNumberOfScanLines() - 1);
				float y = startY() + space * index;
				return OctRoute(startX(), y, closeX(), y);
			}
			else {
				float space = width() / (getNumberOfScanLines() - 1);
				float x = startX() + space * index;
				return OctRoute(x, startY(), x, closeY());
			}
		}

		OctRoute getRouteOfCube(int index) const {
			if (isHorizontal()) {
				float space = height() / (getNumberOfScanLines() - 1);
				float y = startY() + space * index;
				return OctRoute(startX(), y, closeX(), y);
			}
			else {
				float space = width() / (getNumberOfScanLines() - 1);
				float x = startX() + space * index;
				return OctRoute(x, startY(), x, closeY());
			}
		}
	};


	struct OctScanMeasure
	{
	public:
		EyeSide _side = EyeSide::OD;
		float _diopter = 0.0f;

		OctScanPattern _preview;
		OctScanPattern _pattern;
		OctScanPattern _enface;

		bool _phaseEnface = false;
		bool _fastRaster = false;
		bool _previewPattern = false;
		bool _noImageGrab = false;
		bool _targetFundus = false;
		int _angioPanorama = 0;
		int _anteriorWideScanPattern = 0;
		int _anteriorALMeasureTimes = 0;
		int _anteriorALMeasureType = 0;
		int _anteriorLTLensType = 0;
		bool _anteriorReadyPosition = true;
		int _anteriorWideMeasureType = 0;
		int _bioWithTopoType = 0;

	public:
		OctScanMeasure() {
			init();
		}


		void init(void) {
			_side = EyeSide::OD;

			_pattern.init();
			_preview.setupAsPreviewDefault();
			_enface.setupAsEnfaceDefault();

			_phaseEnface = false;
			_fastRaster = false;
			_previewPattern = false;
			_noImageGrab = false;
			return;
		}


		void setup(EyeSide side, OctScanPattern pattern) {
			init();

			_side = side;
			_pattern = pattern;
			return;
		}


		void setup(EyeSide side, OctScanPattern pattern, OctScanPattern enface) {
			_side = side;
			_pattern = pattern;
			_enface = enface;

			usePhaseEnface(true, true);
			return;
		}

		std::string getLabel(void) const {
			std::string label;
			label = (isOD() ? "OD" : "OS");
			label += "_";
			label += _pattern.getPatternNameString();
			return label;
		}

		bool isOD(void) const {
			return EyeSide::OD == _side;
		}


		bool isOS(void) const {
			return EyeSide::OS == _side;
		}

		EyeSide getEyeSide(void) const {
			return _side;
		}

		void setEyeSide(EyeSide side) {
			_side = side;
		}

		float getDiopter(void) const {
			return _diopter;
		}

		void setPattern(OctScanPattern pattern) {
			_pattern = pattern;
		}

		void setEnface(OctScanPattern enface) {
			_enface = enface;
		}

		void setPreview(OctScanPattern preview) {
			_preview = preview;
		}

		const OctScanPattern& pattern(void) const {
			return _pattern;
		}

		const OctScanPattern& enface(void) const {
			return _enface;
		}

		const OctScanPattern& preview(void) const {
			return _preview;
		}

		OctScanPattern& getPattern(void) {
			return _pattern;
		}

		OctScanPattern& getEnface(void) {
			return _enface;
		}

		OctScanPattern& getPreview(void) {
			return _preview;
		}


		bool usePhaseEnface(bool isset = false, bool flag = false) {
			if (isset) {
				_phaseEnface = flag;
			}
			return _phaseEnface;
		}


		bool useFastRaster(bool isset = false, bool flag = false) {
			if (isset) {
				_fastRaster = flag;
			}
			return _fastRaster;
		}


		bool usePreviewPattern(bool isset = false, bool flag = false) {
			if (isset) {
				_previewPattern = flag;
			}
			return _previewPattern;
		}

		bool useNoImageGrab(bool isset = false, bool flag = false) {
			if (isset) {
				_noImageGrab = flag;
			}
			return _noImageGrab;
		}

		bool useTargetFundus(bool isset = false, bool flag = false) {
			if (isset) {
				_targetFundus = flag;
			}
			return _targetFundus;
		}

		void setAnteriorReadyPosition(bool flag = false) {
			_anteriorReadyPosition = flag;
			return;
		}

		bool isAnteriorReadyPosition(void) const {
			return _anteriorReadyPosition;
		}

		void applyScanOffset(OctScanOffset offset) {
			_pattern._offset = offset;
			_enface._offset = offset;
			return;
		}
	};
}
