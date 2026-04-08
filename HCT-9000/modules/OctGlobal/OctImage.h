#pragma once

#include "OctGlobalDef.h"

#include <cstdint>
#include <string>


namespace OctGlobal
{
	struct OctFrameImage
	{
		std::uint8_t* _data;
		int _width = 0;
		int _height = 0;

	public:
		OctFrameImage() {
		}

		OctFrameImage(std::uint8_t* data, int width, int height) {
			setup(data, width, height);
		}

		void setup(std::uint8_t* data, int width, int height) {
			_data = data;
			_width = width;
			_height = height;
		}

		std::uint8_t* getBuffer(void) const {
			return _data;
		}

		int getWidth(void) const {
			return _width;
		}

		int getHeight(void) const {
			return _height;
		}
	};


	struct OctRetinaImage : public OctFrameImage
	{
	public:
		OctRetinaImage() {
		}

		OctRetinaImage(std::uint8_t* data, int width, int height) 
			: OctFrameImage(data, width, height)
		{
		}
	};


	struct OctCorneaImage : public OctFrameImage
	{
	public:
		OctCorneaImage() {
		}

		OctCorneaImage(std::uint8_t* data, int width, int height)
			: OctFrameImage(data, width, height)
		{
		}
	};


	struct OctEnfaceImage : public OctFrameImage
	{

	public:
		OctEnfaceImage() {
		}

		OctEnfaceImage(std::uint8_t* data, int width, int height)
			: OctFrameImage(data, width, height)
		{
		}
	};
}