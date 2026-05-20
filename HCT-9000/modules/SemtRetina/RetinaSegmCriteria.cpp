#include "stdafx.h"
#include "RetinaSegmCriteria.h"
#include "RetinaSegmenter.h"
#include "RetinaSegmentModel.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

#include <algorithm>
#include <vector>

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct RetinaSegmCriteria::RetinaSegmCriteriaImpl
{
	const RetinaSegmenter* segm = nullptr;

	int sampleW = 0;
	int sampleH = 0;
	int sourceW = 0;
	int sourceH = 0;

	float sampleSpaceX = 0.0f;
	float sampleSpaceY = 0.0f;
	float sourceSpaceX = 0.0f;
	float sourceSpaceY = 0.0f;
	float sampleScaleX = 0.0f;
	float sampleScaleY = 0.0f;

	float scanRangeX = 6.0f;

	bool useSampleDim = false; 

	RetinaSegmCriteriaImpl()
	{
		initializeRetinaSegmCriteriaImpl();
	}

	void initializeRetinaSegmCriteriaImpl()
	{
	}
};



RetinaSegmCriteria::RetinaSegmCriteria(RetinaSegmenter* segm) :
	d_ptr(make_unique<RetinaSegmCriteriaImpl>())
{
	impl().segm = segm;
}


SemtRetina::RetinaSegmCriteria::~RetinaSegmCriteria() = default;
SemtRetina::RetinaSegmCriteria::RetinaSegmCriteria(RetinaSegmCriteria&& rhs) = default;
RetinaSegmCriteria& SemtRetina::RetinaSegmCriteria::operator=(RetinaSegmCriteria&& rhs) = default;

float SemtRetina::RetinaSegmCriteria::sampleScaleX(void) const
{
	return impl().sampleScaleX;
}

float SemtRetina::RetinaSegmCriteria::sampleScaleY(void) const
{
	return impl().sampleScaleY;
}

float SemtRetina::RetinaSegmCriteria::sampleSpaceX(void) const
{
	return impl().sampleSpaceX;
}

float SemtRetina::RetinaSegmCriteria::sampleSpaceY(void) const
{
	return impl().sampleSpaceY;
}

float SemtRetina::RetinaSegmCriteria::sourceScaleX(void) const
{
	return 1.0f;
}

float SemtRetina::RetinaSegmCriteria::sourceScaleY(void) const
{
	return 1.0f;
}

float SemtRetina::RetinaSegmCriteria::sourceSpaceX(void) const
{
	return impl().sourceSpaceX;
}

float SemtRetina::RetinaSegmCriteria::sourceSpaceY(void) const
{
	return impl().sourceSpaceY;
}

float SemtRetina::RetinaSegmCriteria::imageScaleX(void) const
{
	float scale = (fromSampleDimensions() ? sampleScaleX() : sourceScaleX());
	return scale;
}

float SemtRetina::RetinaSegmCriteria::imageScaleY(void) const
{
	float scale = (fromSampleDimensions() ? sampleScaleY() : sourceScaleY());
	return scale;
}

float SemtRetina::RetinaSegmCriteria::pixelSpaceX(void) const
{
	float space = (fromSampleDimensions() ? sampleSpaceX() : sourceSpaceX());
	return space;
}

float SemtRetina::RetinaSegmCriteria::pixelSpaceY(void) const
{
	float space = (fromSampleDimensions() ? sampleSpaceY() : sourceSpaceY());
	return space;
}

float SemtRetina::RetinaSegmCriteria::imageScanRangeX(void) const
{
	float range = impl().scanRangeX;
	return range;
}

int SemtRetina::RetinaSegmCriteria::deltaByPixelSpaceX(float factor, int minval, int maxval) const
{
	// 3 mm / 512 = 0.0058, 3 mm / 384 = 0.0078 => 70% 4.04 50% 2.9, 40% 2.3, 30% 1.74, 25% 1.45, 150% 8.7
	// 6 mm / 512 = 0.0117, 6 mm / 384 = 0.0156 => 70% 8.21 50% 5.8, 40% 4.7, 30% 3.51, 25% 2.92, 150% 17.4
	// 9 mm / 512 = 0.0175, 9 mm / 384 = 0.0234 => 70% 12.25 50% 8.7, 40% 7, 30% 5.25, 25% 4.25, 150% 26.1
	// 12 mm / 512 = 0.0234, 12 mm / 384 = 0.0312 => 70% 16.3 50% 11.7, 40% 9.36, 30% 7.02, 25% 5.75, 150% 34.8
	float space = pixelSpaceX();
	int delta = (int)(space * 1000.0f * factor);
	delta += (delta % 2 ? 0 : 1);
	delta = min(max(minval, delta), maxval);
	return delta;
}

void SemtRetina::RetinaSegmCriteria::setImageScanRangeX(float range)
{
	impl().scanRangeX = range;
	return;
}

void SemtRetina::RetinaSegmCriteria::setSampleScaleFactors(float scaleX, float scaleY)
{
	impl().sampleScaleX = scaleX;
	impl().sampleScaleY = scaleY;
	return;
}

void SemtRetina::RetinaSegmCriteria::setSampleDimensions(int width, int height, float spaceX, float spaceY)
{
	impl().sampleW = width;
	impl().sampleH = height;
	impl().sampleSpaceX = spaceX;
	impl().sampleSpaceY = spaceY;
	return;
}


void SemtRetina::RetinaSegmCriteria::setSourceDimensions(int width, int height, float spaceX, float spaceY)
{
	impl().sourceW = width;
	impl().sourceH = height;
	impl().sourceSpaceX = spaceX;
	impl().sourceSpaceY = spaceY;
	return;
}

void SemtRetina::RetinaSegmCriteria::enableSampleDimensions(void)
{
	impl().useSampleDim = true;
	return;
}

void SemtRetina::RetinaSegmCriteria::enableSourceDimensions(void)
{
	impl().useSampleDim = false;
	return;
}

bool SemtRetina::RetinaSegmCriteria::fromSampleDimensions(void) const
{
	return impl().useSampleDim;
}

int SemtRetina::RetinaSegmCriteria::getColumnSnRatiosSmoothWindow(void) const
{
	const float SIZE = 0.09f;  // 5.8 pixs in angio 384.
	int size = (int)(SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getVitreousSizeToTriggerMidpoint(void) const
{
	const int SIZE = 64;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getUpwardOffsetToInnerBound(void) const
{
	const int OFFSET = 48; // 32;
	int offset = (int)(OFFSET * imageScaleY() + 0.5f);
	return offset;
}

int SemtRetina::RetinaSegmCriteria::getSmoothWindowToInnerBound(void) const
{
	const float SIZE = 0.36f; // 23 pixs in angio 384.
	int size = (int)(SIZE / pixelSpaceX());
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getUpwardFilterSizeToOuterBound(void) const
{
	const int SIZE = 12; // 32;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getDownwardOffsetToOuterBound(void) const
{
	const int OFFSET = 12;
	int offset = (int)(OFFSET * imageScaleY() + 0.5f);
	return offset;
}

int SemtRetina::RetinaSegmCriteria::getSmoothWindowToOuterBound(void) const
{
	// const float SIZE = 0.54f;  // 35 pixs in angio 384
	const float SIZE = 0.72f;  // 48 pixs in angio 384
	int size = (int)(SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getOpticDiscHeadWidthMin(void) const
{
	const float HEAD_WDITH = 0.42f;  // 27 pixs in angio 384
	int width = (int)(HEAD_WDITH / pixelSpaceX() + 0.5f);
	return width;
}

int SemtRetina::RetinaSegmCriteria::getOpticDiscSideWidthMin(void) const
{
	const float SIDE_WIDTH = 0.24f;  // 15 pixs in angio 384
	int width = (int)(SIDE_WIDTH / pixelSpaceX() + 0.5f);
	return width;
}

int SemtRetina::RetinaSegmCriteria::getOpticDiscHeadDepthMin(void) const
{
	const float HEAD_DEPTH = 36;
	int depth = (int)(HEAD_DEPTH * imageScaleY() + 0.5f);
	return depth;
}

int SemtRetina::RetinaSegmCriteria::getOpticDiscHeadMergeDist(void) const
{
	// const float SPACE = 0.72f; // 46 pixs in angio 384
	const float SPACE = 1.08f; // 69 pixs in angio 384
	int width = (int)(SPACE / pixelSpaceX() + 0.5f);
	return width;
}

int SemtRetina::RetinaSegmCriteria::getOpticDiscCupDepthMin(void) const
{
	const float HEAD_DEPTH = 36;
	int depth = (int)(HEAD_DEPTH * imageScaleY() + 0.5f);
	return depth;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsILM(void) const
{
	const int SIZE = 15;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsILM(void) const
{
	const int SIZE = 5;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaILM(void) const
{
	// int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	int delta = deltaByPixelSpaceX(0.5f, 5, 11);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaILM(void) const
{
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDownwardMarginILM(void) const
{
	const int MARGIN = 12; // 24;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathTopOverMarginILM(void) const
{
	const int MARGIN = 4;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowILM(void) const
{
	const float SIZE = 0.09f;  // 5.8 pixs in angio 384.
	int size = (int)(SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowILM(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE1 = 0.18f; // 0.18f;  // 11.6 pixs in angio 384.
	const float SIZE2 = 0.27f; // 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getlayerUpperSpaceMinILM(void) const
{
	const int SIZE = 24;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathSideMarginSlopeWidth(void) const
{
	const float SIZE = 0.54f;  // 35 pixs in angio 384
	int width = (int)(SIZE / pixelSpaceX() + 0.5f);
	return width;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaONL(void) const
{
	int delta = deltaByPixelSpaceX(0.5f, 5, 11);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaONL(void) const
{
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowONL(void) const
{
	const float WINDOW_SIZE = 0.09f;  // 5.8 pixs in angio 384.
	int size = (int)(WINDOW_SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowONL(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE2 = 0.18f;  // 11.6 pixs in angio 384.
	// const float SIZE2 = 0.27f;  // 17.2 pixs in angio 384.
	const float SIZE2 = 0.36f;  // 23.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsNFL(void) const
{
	const int SIZE = 15;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsNFL(void) const
{
	const int SIZE = 5; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaNFL(void) const
{
	// const int DELTA = 5;
	// int delta = (int)(DELTA * imageScaleY() + 0.5f);
	int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaNFL(void) const
{
	// const int DELTA = 11;
	// int delta = (int)(DELTA * imageScaleY() + 0.5f);
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowNFL(void) const
{
	const float WINDOW_SIZE = 0.09f;  // 5.76 pixs in angio 384.
	int size = (int)(WINDOW_SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowNFL(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE1 = 0.18f; // 0.18f;  // 11.6 pixs in angio 384.
	// const float SIZE2 = 0.36f; // 0.27f;  // 23.2 pixs in angio 384.
	const float SIZE2 = 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerUpperSpaceMinNFL(void) const
{
	const int SIZE = 3;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsOPL(void) const
{
	const int SIZE = 15; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsOPL(void) const
{
	const int SIZE = 5; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaOPL(void) const
{
	int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaOPL(void) const
{
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscUpwardMarginOPL(void) const
{
	const int MARGIN = 18;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowOPL(void) const
{
	const float WINDOW_SIZE = 0.09f;  // 5.76 pixs in angio 384.
	int size = (int)(WINDOW_SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowOPL(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE1 = 0.18f; // 0.18f;  // 11.6 pixs in angio 384.
	const float SIZE2 = 0.27f; // 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerUpperSpaceMinOPL(void) const
{
	const int SIZE = 12;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerLowerSpaceMaxOPL(void) const
{
	const int SIZE = 7; // 24;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsIPL(void) const
{
	const int SIZE = 15; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsIPL(void) const
{
	const int SIZE = 5; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaIPL(void) const
{
	int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaIPL(void) const
{
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowIPL(void) const
{
	const float WINDOW_SIZE = 0.09f;  // 5.76 pixs in angio 384.
	int size = (int)(WINDOW_SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowIPL(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE1 = 0.18f; // 0.18f;  // 11.6 pixs in angio 384.
	const float SIZE2 = 0.27f; // 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}


int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsIOS(void) const
{
	const int SIZE = 9; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsIOS(void) const
{
	const int SIZE = 5; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaIOS(void) const
{
	// const int DELTA = 5;
	// int delta = (int)(DELTA * imageScaleY() + 0.5f);
	int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaIOS(void) const
{
	// int delta = deltaByPixelSpaceX(0.5f, 5, 13);
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscUpperOffsetMinIOS(void) const
{
	const int MARGIN = 12;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscUpperOffsetMaxIOS(void) const
{
	const int MARGIN = 64;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathTopOverOffsetIOS(void) const
{
	const int MARGIN = 24;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowIOS(void) const
{
	const float WINDOW_SIZE = 0.09f;  // 5.76 pixs in angio 384.
	int size = (int)(WINDOW_SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerUpperSpaceMaxIOS(void) const
{
	const int MARGIN = 16;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getLayerRefiningRangeIOS(void) const
{
	const int SIZE = 3;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscOffsetMinIOS(void) const
{
	const int MARGIN = 5;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscOffsetMaxIOS(void) const
{
	const int MARGIN = 12;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowIOS(bool local) const
{
	const float SIZE1 = 0.09f; // 0.18f;  // 11.6 pixs in angio 384.
	const float SIZE2 = 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscEdgeGapIOS(void) const
{
	const int SIZE = 11;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsRPE(void) const
{
	const int SIZE = 11; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsRPE(void) const
{
	const int SIZE = 5; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaRPE(void) const
{
	int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaRPE(void) const
{
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathUpperOffsetMinRPE(void) const
{
	const int SIZE = 5;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathUpperOffsetMaxRPE(void) const
{
	const int SIZE = 36;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerLowerOffsetMinRPE(void) const
{
	const int SIZE = 3;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerLowerOffsetMaxRPE(void) const
{
	const int SIZE = 16;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerRefiningRangeRPE(void) const
{
	const int SIZE = 5;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowRPE(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE1 = 0.18f; // 0.18f;  // 11.6 pixs in angio 384.
	// const float SIZE2 = 0.36f; // 0.27f;  // 17.2 pixs in angio 384.
	const float SIZE2 = 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscEdgeGapRPE(void) const
{
	const int SIZE = 11;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelRowsBRM(void) const
{
	const int SIZE = 11; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getGradientKernelColsBRM(void) const
{
	const int SIZE = 5; 
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getPathCostRangeDeltaBRM(void) const
{
	int delta = deltaByPixelSpaceX(0.5f, 3, 9);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscRangeDeltaBRM(void) const
{
	int delta = deltaByPixelSpaceX(1.5f, 9, 27);
	delta = (int)(delta * imageScaleY() + 0.5f);
	return delta;
}

int SemtRetina::RetinaSegmCriteria::getPathUpperOffsetMinBRM(void) const
{
	const int MARGIN = 5; // 36;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscUpperOffsetMinBRM(void) const
{
	const int MARGIN = 8; // 36;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathDiscUpperOffsetMaxBRM(void) const
{
	const int MARGIN = 32; // 72;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getPathSmoothWindowBRM(void) const
{
	const float WINDOW_SIZE = 0.09f;  // 5.76 pixs in angio 384.
	int size = (int)(WINDOW_SIZE / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerLowerSpaceMaxBRM(void) const
{
	const int SIZE = 7;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerRefiningUpperSpaceBRM(void) const
{
	const int SIZE = 3;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerRefiningLowerSpaceBRM(void) const
{
	const int SIZE = 3;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscUpperOffsetMinBRM(void) const
{
	const int MARGIN = 5;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscUpperOffsetMaxBRM(void) const
{
	const int MARGIN = 12;
	int margin = (int)(MARGIN * imageScaleY() + 0.5f);
	return margin;
}

int SemtRetina::RetinaSegmCriteria::getLayerSmoothWindowBRM(bool local) const
{
	const float SIZE1 = 0.09f;  // 5.8 pixs in angio 384.
	// const float SIZE1 = 0.18f;  // 11.6 pixs in angio 384.
	// const float SIZE2 = 0.36f; //  0.27f;  // 17.2 pixs in angio 384.
	const float SIZE2 = 0.27f;  // 17.2 pixs in angio 384.
	int size = (int)((local ? SIZE1 : SIZE2) / pixelSpaceX() + 0.5f);
	size = (size % 2 == 0) ? (size + 1) : size;
	return size;
}

int SemtRetina::RetinaSegmCriteria::getLayerDiscEdgeGapBRM(void) const
{
	const int SIZE = 11;
	int size = (int)(SIZE * imageScaleY() + 0.5f);
	return size;
}


RetinaSegmCriteria::RetinaSegmCriteriaImpl& SemtRetina::RetinaSegmCriteria::impl(void) const
{
	return *d_ptr;
}