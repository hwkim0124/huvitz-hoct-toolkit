#pragma once

#include "OctGlobalDef.h"
#include "GlobalPatternDef.h"
#include "GlobalMeasureDef.h"

#include "PatternHelper.h"

#include <cstdint>
#include <string>

namespace OctGlobal
{
	struct OctScanEnface
	{
	public:
		const std::uint8_t* data = nullptr;
		int width = 0;
		int height = 0;
		OctRange range;

	public:
		OctScanEnface() {
			init();
		}

		OctScanEnface(const std::uint8_t* data_, int width_, int height_) {
			set(data_, width_, height_);
		}

		OctScanEnface(const std::uint8_t* data_, int width_, int height_, OctRange range_) {
			set(data_, width_, height_, range_);
		}

		void init(void) {
			data = nullptr;
			width = 0;
			height = 0;
		}


		void set(const std::uint8_t* data_, int width_, int height_) {
			data = data_;
			width = width_;
			height = height_;
		}

		void set(const std::uint8_t* data_, int width_, int height_, OctRange range_) {
			data = data_;
			width = width_;
			height = height_;
			range = range_;
		}

		const std::uint8_t* getData(void) const {
			return data;
		}

		int getWidth(void) const {
			return width;
		}

		int getHeight(void) const {
			return height;
		}

		OctRange getRange(void) const {
			return range;
		}
	};


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
		OctScanImage() {
			init();
		}

		OctScanImage(const std::uint8_t* data_, int width_, int height_, float quality_, float sigRatio_, int refPoint_) {
			set(data_, width_, height_, quality_, sigRatio_, refPoint_);
		}

		void init(void) {
			data = nullptr;
			width = 0;
			height = 0;
			quality = 0.0f;
			sigRatio = 0.0f;
			refPoint = 0;
		}

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

		int getHeight(void) const {
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
		int index;
		OctRoute route;

	public:
		OctScanSection() {
			init();
		}

		OctScanSection(int index_, OctRoute route_) {
			index = index_;
			route = route_;
		}

		void init(void) {
			index = 0;
		}

		void set(int index_, OctRoute route_) {
			index = index_;
			route = route_;
		}

		int getIndex(void) const {
			return index;
		}

		const OctRoute& getRoute(void) const {
			return route;
		}

	};

}
