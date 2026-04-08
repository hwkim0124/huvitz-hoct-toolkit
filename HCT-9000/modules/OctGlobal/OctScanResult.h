#pragma once

#include "OctGlobalDef.h"
#include "GlobalPatternDef.h"
#include "GlobalMeasureDef.h"

#include "PatternHelper.h"

#include <cstdint>
#include <string>
#include <sstream>


namespace OctGlobal
{
	struct OctScanSection
	{
	public:
		int _index = 0;
		OctRoute _route = OctRoute();

	public:
		OctScanSection() {
			init();
		}

		OctScanSection(int index, OctRoute route) {
			_index = index;
			_route = route;
		}

		void init(void) {
			setup(0, OctRoute());
		}

		void setup(int index, OctRoute route) {
			_index = index;
			_route = route;
		}

		int getIndex(void) const {
			return _index;
		}

		const OctRoute& getRoute(void) const {
			return _route;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<ScanSection> " : "");
			ss << "index: " << this->_index << ", route: " << this->_route.text(false);
			return ss.str();
		}
	};


	struct OctScanImage
	{
	public:
		const std::uint8_t* _data = nullptr;
		int _width = 0;
		int _height = 0;
		float _quality = 0.0f;
		float _sigRatio = 0.0f;
		int _refPoint = 0;
		bool _reversed = false;
		int _index = 0;
		float _rangeX = 6.0f;

	public:
		OctScanImage() {
			init();
		}

		OctScanImage(const std::uint8_t* data, int width, int height, float quality = 0.0f, float sigRatio = 0.0f, int refPoint = 0) {
			setup(data, width, height, quality, sigRatio, refPoint);
		}

		void init(void) {
			_data = nullptr;
			_width = 0;
			_height = 0;
			_quality = 0.0f;
			_sigRatio = 0.0f;
			_refPoint = 0;
			_reversed = false;
			_index = 0;
			return;
		}

		void setup(const std::uint8_t* data, int width, int height, float quality=0.0f, float sigRatio=0.0f, int refPoint=0) {
			_data = data;
			_width = width;
			_height = height;
			_quality = quality;
			_sigRatio = sigRatio;
			_refPoint = refPoint;
			return;
		}

		const std::uint8_t* getData(void) const {
			return _data;
		}

		int getWidth(void) const {
			return _width;
		}

		int getHeight(void) const {
			return _height;
		}

		void setReversed(bool flag) {
			_reversed = flag;
			return;
		}

		void setIndex(int index) {
			_index = index;
		}

		int getIndex(void) const {
			return _index;
		}

		bool isEmpty(void) const {
			return _data == nullptr || _width <= 0 || _height <= 0;
		}

		bool isReversed(void) const {
			return _reversed;
		}

		void setSize(int width, int height) {
			_width = width;
			_height = height;
			return;
		}

		void setRangeX(float range) {
			_rangeX = range;
		}

		float getRangeX() {
			return _rangeX;
		}

		float getQuality(void) const {
			return _quality;
		}

		float getSignalRatio(void) const {
			return _sigRatio;
		}

		int getReferencePoint(void) const {
			return _refPoint;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<ScanImage> " : "");
			ss << "width: " << this->_width << ", height: " << this->_height;
			ss << ", quality: " << this->_quality << ", sigRatio: " << this->_sigRatio;
			ss << ", refPoint: " << this->_refPoint;
			return ss.str();
		}
	};


	struct OctScanEnface
	{
	public:
		const std::uint8_t* _data = nullptr;
		int _width = 0;
		int _height = 0;
		OctRange _range = OctRange();

	public:
		OctScanEnface() {
			init();
		}

		OctScanEnface(const std::uint8_t* data, int width, int height) {
			setup(data, width, height);
		}

		OctScanEnface(const std::uint8_t* data, int width, int height, OctRange range) {
			setup(data, width, height, range);
		}

		void init(void) {
			setup(nullptr, 0, 0, OctRange());
		}

		void setup(const std::uint8_t* data, int width, int height) {
			_data = data;
			_width = width;
			_height = height;
			return;
		}

		void setup(const std::uint8_t* data, int width, int height, OctRange range) {
			_data = data;
			_width = width;
			_height = height;
			_range = range;
			return;
		}

		const std::uint8_t* getData(void) const {
			return _data;
		}

		int getWidth(void) const {
			return _width;
		}

		int getHeight(void) const {
			return _height;
		}

		const OctRange& getRange(void) const {
			return _range;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<ScanEnface> " : "");
			ss << "width: " << this->_width << ", height: " << this->_height;
			ss << ", range: " << this->_range.text(false);
			return ss.str();
		}
	};



}