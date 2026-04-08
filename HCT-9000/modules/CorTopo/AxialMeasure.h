#pragma once


#include "CorTopoDef.h"

#include <string>
#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace RetSegm {
	class SegmImage;
}


namespace CorTopo
{
	class CORTOPO_DLL_API AxialMeasure
	{
	public:
		AxialMeasure();
		virtual ~AxialMeasure();

		AxialMeasure(AxialMeasure&& rhs);
		AxialMeasure& operator=(AxialMeasure&& rhs);
		AxialMeasure(const AxialMeasure& rhs) = delete;
		AxialMeasure& operator=(const AxialMeasure& rhs) = delete;

	public:
		bool loadRetinaImage(const std::wstring& path, int referPos);
		bool loadCorneaImage(const std::wstring& path, int referPos);
		bool loadCorneaImage2(const std::wstring& path, int referPos);
		bool isRetinaImageLoaded(void);
		bool isCorneaImageLoaded(void);
		bool isCorneaImageLoaded2(void);

		bool performCalculation();
		bool performCalculation2();
		bool performCalculation3();
		bool isResult(void);

		float getAxialLength(float refIndex = REFRACTIVE_INDEX_MEASURE);
		float getAxialLength(int retinaReferPos, int retinaLinePos, int corneaReferPos, int corneaLinePos, float refIndex = REFRACTIVE_INDEX_MEASURE);
		float getAxialLength2(float refIndex = REFRACTIVE_INDEX_MEASURE);
		float getAxialLength2(int retinaReferPos, int retinaLinePos, int corneaReferPos, int corneaLinePos, float refIndex = REFRACTIVE_INDEX_MEASURE);
		float getAxialLength3(float refIndex = REFRACTIVE_INDEX_MEASURE);

		int getRetinaLinePosition(void);
		int getCorneaLinePosition(void);
		std::vector<int> getRetinaLinePoints(void);
		std::vector<int> getCorneaLinePoints(void);

	protected:
		bool loadRetinaSegmSource(const unsigned char* bits, int width, int height, int referPos);
		bool loadCorneaSegmSource(const unsigned char* bits, int width, int height, int referPos);

	private:
		struct AxialMeasureImpl;
		std::unique_ptr<AxialMeasureImpl> d_ptr;
		AxialMeasureImpl& getImpl(void) const;
	};
}
