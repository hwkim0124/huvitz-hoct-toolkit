#pragma once

#include "RetParamDef.h"

#include <memory>
#include <vector>


namespace RetSegm {
	class SegmImage;
	class SegmLayer;
}


namespace CppUtil {
	class CvImage;
}


namespace RetParam
{
	class RETPARAM_DLL_API EnfaceData
	{
	public:
		EnfaceData();
		virtual ~EnfaceData();

		EnfaceData(EnfaceData&& rhs);
		EnfaceData& operator=(EnfaceData&& rhs);
		EnfaceData(const EnfaceData& rhs);
		EnfaceData& operator=(const EnfaceData& rhs);

	public:
		static const int RADIAL_DATA_IMAGE_SIZE = 256;

	public:
		bool setupData(const OctScanPattern &pattern,
			OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
			const std::vector<std::vector<std::uint8_t>>& data,
			const std::vector<bool>& flags);
		bool setupData(const OctScanPattern &pattern,
			OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
			const std::vector<std::vector<float>>& lines,
			const std::vector<bool>& flags);

		bool isEmpty(void) const;
		bool isIdentical(OcularLayerType upper, OcularLayerType lower, float upperOffset=0.0f, float lowerOffset=0.0f) const;
		bool isIdentical(float width, float height, int numLines, int lineSize,
			OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f) const;

		float rangeX(void) const;
		float rangeY(void) const;
		int numLines(void) const;
		int lineSize(void) const;
		bool isVertical(void) const;
		
		OcularLayerType upperLayer(void) const;
		OcularLayerType lowerLayer(void) const;
		float upperLayerOffset(void) const;
		float lowerLayerOffset(void) const;

		int getWidth(void) const;
		int getHeight(void) const;
		float getPixelXperMM(void) const;
		float getPixelYperMM(void) const;

		int getPositionX(float xmm) const;
		int getPositionY(float ymm) const;
		float getPositionXf(float xmm) const;
		float getPositionYf(float ymm) const;

		float getPixelSizeX(float xmm) const;
		float getPixelSizeY(float ymm) const;

		float getAverageOnHorzLine(float diam = 3.0f) const;
		float getAverageOnVertLine(float diam = 3.0f) const;
		float getAverageOnCenter(float diam = 3.0f) const;
		float getAverageOnCircle(float diam, int samples) const;
				
	protected:
		CppUtil::CvImage* getDataImage(void) const;
		bool buildDataImage(const std::vector<std::vector<std::uint8_t>>& lines, const std::vector<bool>& flags);
		bool buildDataImage(const std::vector<std::vector<float>>& lines, const std::vector<bool>& flags);

	private:
		struct EnfaceDataImpl;
		std::unique_ptr<EnfaceDataImpl> d_ptr;
		EnfaceDataImpl& getImpl(void) const;

		friend class SectorChart;
		friend class ClockChart;
		friend class DonutChart;
	};
}

