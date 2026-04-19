#include "stdafx.h"
#include "ONHMorphometrics.h"
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


struct ONHMorphometrics::ONHMorphometricsImpl
{
	const RetinaSegmenter* segm = nullptr;

	int discMarginMinX;
	int discMarginMaxX;
	int cupOpeningMinX;
	int cupOpeningMaxX;

	int discRimCount;
	int discCupCount;

	ONHMorphometricsImpl()
	{
		initializeONHMorphometricsImpl();
	}

	void initializeONHMorphometricsImpl()
	{
		discMarginMinX = -1;
		discMarginMaxX = -1;
		cupOpeningMinX = -1;
		cupOpeningMaxX = -1;

		discRimCount = -1;
		discCupCount = -1;
	}
};



ONHMorphometrics::ONHMorphometrics(RetinaSegmenter* segm) :
	d_ptr(make_unique<ONHMorphometricsImpl>())
{
	impl().segm = segm;
}


SemtRetina::ONHMorphometrics::~ONHMorphometrics() = default;
SemtRetina::ONHMorphometrics::ONHMorphometrics(ONHMorphometrics&& rhs) = default;
ONHMorphometrics& SemtRetina::ONHMorphometrics::operator=(ONHMorphometrics&& rhs) = default;

bool SemtRetina::ONHMorphometrics::accumulateDiscRimVoxels(void)
{
	auto* segm = impl().segm;
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSource();
	auto sidx = resa->sampleIndex();

	auto* bilm = segm->boundaryILM();
	auto* brpe = segm->boundaryRPE();

	auto width = image->getWidth();
	auto dsc_x1 = band->opticDiscMinX();
	auto dsc_x2 = band->opticDiscMaxX();
	auto dsize = (dsc_x2 - dsc_x1 + 1);
	auto count = 0;

	auto ilms = bilm->sourceYs();
	auto rpes = brpe->sourceYs();
	if (ilms.size() != width || rpes.size() != width) {
		return false;
	}
	if (dsize <= 0 || dsc_x1 >= width || dsc_x2 >= width) {
		return false;
	}

	auto out_y1 = rpes[dsc_x1];
	auto out_y2 = rpes[dsc_x2];
	if (out_y1 < 0 || out_y2 < 0) {
		return false;
	}

	const float FEAT_DISK_CUP_LINE_DEPTH = 150.0f;
	const float axialResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int top_y1 = (int)(out_y1 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	int top_y2 = (int)(out_y2 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	top_y1 = max(top_y1, 0);
	top_y2 = max(top_y2, 0);

	float slope = (float)(top_y2 - top_y1) / (float)(dsc_x2 - dsc_x1);
	int tpos;

	for (int x = dsc_x1, k = 0; x <= dsc_x2; x++, k++) {
		tpos = (int)(top_y1 + k * slope);
		if (ilms[x] < tpos) {
			count += (tpos - ilms[x]);
		}
	}

	LogD() << "ONH disc margin, x1: " << dsc_x1 << ", x2: " << dsc_x2 << ", rim voxels: " << count << ", imageIdx: " << sidx;
	impl().discMarginMinX = dsc_x1;
	impl().discMarginMaxX = dsc_x2;
	impl().discRimCount = count;
	return true;
}

bool SemtRetina::ONHMorphometrics::accumulateDiscCupVoxels(void)
{
	auto* segm = impl().segm;
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSource();
	auto sidx = resa->sampleIndex();

	auto* bilm = segm->boundaryILM();
	auto* brpe = segm->boundaryRPE();

	auto width = image->getWidth();
	auto dsc_x1 = band->opticDiscMinX();
	auto dsc_x2 = band->opticDiscMaxX();
	auto dsize = (dsc_x2 - dsc_x1 + 1);

	auto ilms = bilm->sourceYs();
	auto rpes = brpe->sourceYs();
	if (ilms.size() != width || rpes.size() != width) {
		return false;
	}
	if (dsize <= 0 || dsc_x1 >= width || dsc_x2 >= width) {
		return false;
	}

	auto out_y1 = rpes[dsc_x1];
	auto out_y2 = rpes[dsc_x2];
	if (out_y1 < 0 || out_y2 < 0) {
		return false;
	}

	const float FEAT_DISK_CUP_LINE_DEPTH = 150.0f;
	const float axialResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int top_y1 = (int)(out_y1 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	int top_y2 = (int)(out_y2 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	top_y1 = max(top_y1, 0);
	top_y2 = max(top_y2, 0);

	float slope = (float)(top_y2 - top_y1) / (float)(dsc_x2 - dsc_x1);
	int tpos;
	int cup_x1 = -1;
	int cup_x2 = -1;

	for (int x = dsc_x1, k = 0; x <= dsc_x2; x++, k++) {
		tpos = max((int)(top_y1 + k * slope), 0);
		if (ilms[k] >= tpos) {
			cup_x1 = x;
			break;
		}
	}

	if (cup_x1 >= 0) {
		for (int x = dsc_x2, k = 0; x >= dsc_x1; x--, k++) {
			tpos = max((int)(top_y2 - k * slope), 0);
			if (ilms[k] >= tpos) {
				cup_x2 = x;
				break;
			}
		}
	}

	if (cup_x1 < 0 || cup_x2 < 0 || cup_x1 == cup_x2) {
		return false;
	}

	int count = 0;
	for (int x = cup_x1, k = (cup_x1 - dsc_x1); x <= cup_x2; x++, k++) {
		tpos = max((int)(top_y1 + k * slope), 0);
		if (ilms[k] >= tpos) {
			count += (ilms[k] - tpos + 1);
		}
	}

	LogD() << "ONH cup opening, x1: " << cup_x1 << ", x2: " << cup_x2 << ", cup voxels: " << count << ", imageIdx: " << sidx;
	impl().cupOpeningMinX = cup_x1;
	impl().cupOpeningMaxX = cup_x2;
	impl().discRimCount = count;
	return false;
}

int SemtRetina::ONHMorphometrics::discMarginMinX(void) const
{
	auto v = impl().discMarginMinX;
	return v;
}

int SemtRetina::ONHMorphometrics::discMarginMaxX(void) const
{
	auto v = impl().discMarginMaxX;
	return v;
}

int SemtRetina::ONHMorphometrics::cupOpeningMinX(void) const
{
	auto v = impl().cupOpeningMinX;
	return v;
}

int SemtRetina::ONHMorphometrics::cupOpeningMaxX(void) const
{
	auto v = impl().cupOpeningMaxX;
	return v;
}

int SemtRetina::ONHMorphometrics::discRimVoxelCount(void) const
{
	auto v = impl().discRimCount;
	return v;
}

int SemtRetina::ONHMorphometrics::discCupVoxelCount(void) const
{
	auto v = impl().discCupCount;
	return v;
}

bool SemtRetina::ONHMorphometrics::isDiscMarginRangeValid(void) const
{
	auto x1 = impl().discMarginMinX;
	auto x2 = impl().discMarginMaxX;
	auto b = (x1 >= 0 && x2 >= 0 && x1 < x2);
	return b;
}

bool SemtRetina::ONHMorphometrics::isCupOpeningRangeValid(void) const
{
	auto x1 = impl().cupOpeningMinX;
	auto x2 = impl().cupOpeningMaxX;
	auto b = (x1 >= 0 && x2 >= 0 && x1 < x2);
	return b;
}

bool SemtRetina::ONHMorphometrics::hasDiscRimMeasurements(void) const
{
	auto b = impl().discRimCount >= 0;
	return b;
}

bool SemtRetina::ONHMorphometrics::hasDiscCupMeasurements(void) const
{
	auto b = impl().discCupCount >= 0;
	return b;
}

ONHMorphometrics::ONHMorphometricsImpl& SemtRetina::ONHMorphometrics::impl(void) const
{
	return *d_ptr;
}
