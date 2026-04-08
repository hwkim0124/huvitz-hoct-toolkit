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
	class CORTOPO_DLL_API LensMeasure
	{
	public:
		LensMeasure();
		virtual ~LensMeasure();

		LensMeasure(LensMeasure&& rhs);
		LensMeasure& operator=(LensMeasure&& rhs);
		LensMeasure(const LensMeasure& rhs) = delete;
		LensMeasure& operator=(const LensMeasure& rhs) = delete;

	public:
		bool loadCorneaImage(const std::wstring& path, int motorPos);
		bool loadCorneaSegmSource(const unsigned char* bits, int width, int height, int motorPos);
		bool loadLensFrontImage(const std::wstring& path, int motorPos);
		bool loadLensBackImage(const std::wstring& path, int motorPos);

		bool isCorneaImageLoaded(void);
		bool isLensFrontImageLoaded(void);
		bool isLensBackImageLoaded(void);

		bool performCalculation(void);
		bool performCalculation2(void);
		bool isResult(void);

		float getLensThickness(void);
		float getLensThickness(int frontReferPos, int frontLinePos, int backReferPos, int backLinePos);
		float getAnteriorChamberDistance(void);
		float getAnteriorChamberDistance(int corneaReferPos, int corneaLinePos, int frontReferPos, int frontLinePos);
		float getCorneaCenterThickness(void);

		int getCorneaLinePosition(void);
		int getLensFrontLinePosition(void);
		int getLensBackLinePosition(void);

		std::vector<int> getCorneaAnteriorPoints(void);
		std::vector<int> getCorneaPosteriorPoints(void);

	private:
		struct LensMeasureImpl;
		std::unique_ptr<LensMeasureImpl> d_ptr;
		LensMeasureImpl& getImpl(void) const;
	};
}

