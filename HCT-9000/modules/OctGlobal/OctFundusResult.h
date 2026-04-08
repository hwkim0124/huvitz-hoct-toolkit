#pragma once

#include "OctGlobalDef.h"
#include "GlobalFundusDef.h"
#include "GlobalMeasureDef.h"

#include <cstdint>

namespace OctGlobal
{
	struct OctFundusSection
	{
		int _index;

	public:
		OctFundusSection() {
			init();
		}

		OctFundusSection(int index) {
			_index = index;
		}

		void init(void) {
			_index = 0;
		}

		void set(int index) {
			_index = index;
		}

		int getIndex(void) const {
			return _index;
		}
	};


	struct OctFundusImage
	{
		const std::uint8_t* _data;
		int _width;
		int _height;

	public:
		OctFundusImage() {
			init();
		}

		OctFundusImage(const std::uint8_t* data, int width, int height) {
			setup(data, width, height);
		}

		void init(void) {
			_data = nullptr;
			_width = 0;
			_height = 0;
		}

		void setup(const std::uint8_t* data, int width, int height) {
			_data = data;
			_width = width;
			_height = height;
		}

		const std::uint8_t* getBuffer(void) const {
			return _data;
		}

		int getWidth(void) const {
			return _width;
		}

		int getHeight(void) const {
			return _height;
		}
	};


	struct OctFundusFrame : public OctFundusImage
	{
	public:
		OctFundusFrame() : OctFundusImage() {
		}

		OctFundusFrame(const std::uint8_t* data, int width, int height)
			: OctFundusImage(data, width, height)
		{
		}
	};
}