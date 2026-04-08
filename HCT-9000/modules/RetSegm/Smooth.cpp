#include "stdafx.h"
#include "Smooth.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "SegmImage.h"
#include "SegmLayer.h"

#include "CppUtil2.h"


using namespace RetSegm;

#include <opencv2/core/core.hpp>
using namespace cv;

#include <iostream>
#include <iomanip>
using namespace std;


Smooth::Smooth()
{
}


Smooth::~Smooth()
{
}


bool RetSegm::Smooth::smoothLayerEPI(const std::vector<int>& path, int width, int height, float rangeX, SegmLayer * layerEPI)
{
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	float multi = 1.0f; // 16.0f / rangeX;
	int size = (int)(ratio * multi * LAYER_EPI_SMOOTH_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_EPI_SMOOTH_CURVE_DEGREE);

	/*
	for (int i = 0; i < outs.size(); i++) {
		outs[i] = max(outs[i], 0);
		outs[i] = min(outs[i], height - 1);
	}
	*/

	layerEPI->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerEND(const std::vector<int>& path, int width, int height, float rangeX, SegmLayer * layerEPI, SegmLayer * layerEND)
{
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	float multi = 1.0f; // 16.0f / rangeX;
	int size = (int)(ratio * multi * LAYER_END_SMOOTH_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_END_SMOOTH_CURVE_DEGREE);

	/*
	for (int i = 0; i < outs.size(); i++) {
		outs[i] = max(outs[i], 0);
		outs[i] = min(outs[i], height - 1);
	}
	*/

	auto inner = layerEPI->getYs();
	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });

	layerEND->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerBOW(const std::vector<int>& path, int width, int height, float rangeX, SegmLayer * layerEPI, SegmLayer * layerEND, SegmLayer * layerBOW)
{
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	float multi = 1.0f; // 16.0f / rangeX;
	int size = (int)(ratio * LAYER_BOW_SMOOTH_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_BOW_SMOOTH_CURVE_DEGREE);

	/*
	for (int i = 0; i < outs.size(); i++) {
		outs[i] = max(outs[i], 0);
		outs[i] = min(outs[i], height - 1);
	}
	*/

	auto inner = layerEPI->getYs();
	auto outer = layerEND->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });

	layerBOW->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerILM(const std::vector<int>& path, int width, int height, SegmLayer* layerNFL, SegmLayer * layerILM, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_ILM_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_ILM_CURVE_DEGREE);

	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerILM->initialize(outs, width, height);
	*/

	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * LAYER_ILM_CURVE_SIZE);
	auto outer = layerNFL->getYs();

	if (!found) {
		transform(begin(path), end(path), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, filter, LAYER_ILM_CURVE_DEGREE);
	}
	else {
		outs = CppUtil::SgFilter::smoothInts(path, LAYER_ILM_CURVE_SIZE_DISC, LAYER_ILM_CURVE_DEGREE_DISC);
		auto subs = CppUtil::SgFilter::smoothInts(path, filter, LAYER_ILM_CURVE_DEGREE);

		int knees = (int)(size * 0.050f);
		for (int i = (discX1 - knees); i >= 0; i--) {
			if (subs[i] == outs[i]) {
				copy(subs.begin(), subs.begin() + i, outs.begin());
				break;
			}
		}
		for (int i = (discX2 + knees); i < size; i++) {
			if (subs[i] == outs[i]) {
				copy(subs.begin() + i, subs.end(), outs.begin() + i);
				break;
			}
		}

		for (int i = 0; i < discX1; i++) {
			outs[i] = min(outs[i], outer[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = min(outs[i], outer[i]);
		}
	}

	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerILM->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerNFL(const std::vector<int>& path, int width, int height, SegmLayer * layerILM, SegmLayer * layerOPL, SegmLayer * layerNFL, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_NFL_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_NFL_CURVE_DEGREE);
	auto inner = layerILM->getYs();
	auto outer = layerOPL->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerNFL->initialize(outs, width, height);
	*/

	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();
	auto inner = layerILM->getYs();
	auto outer = layerOPL->getYs();

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * LAYER_NFL_CURVE_SIZE);

	if (true) { // !found) {
		outs = CppUtil::SgFilter::smoothInts(path, filter, LAYER_NFL_CURVE_DEGREE);
	}
	else {
		outs = CppUtil::SgFilter::smoothInts(path, LAYER_NFL_CURVE_SIZE_DISC, LAYER_NFL_CURVE_DEGREE_DISC);
		auto subs = CppUtil::SgFilter::smoothInts(path, filter, LAYER_NFL_CURVE_DEGREE);

		int knees = (int)(size * 0.050f);
		for (int i = (discX1 - knees); i >= 0; i--) {
			if (subs[i] == outs[i]) {
				copy(subs.begin(), subs.begin() + i, outs.begin());
				break;
			}
		}
		for (int i = (discX2 + knees); i < size; i++) {
			if (subs[i] == outs[i]) {
				copy(subs.begin() + i, subs.end(), outs.begin() + i);
				break;
			}
		}
	}

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerNFL->initialize(outs, width, height);
	return true;
}



bool RetSegm::Smooth::smoothLayerIPL(const std::vector<int>& path, int width, int height, SegmLayer * layerNFL, SegmLayer * layerOPL, SegmLayer * layerIPL, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_IPL_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_IPL_CURVE_DEGREE);
	auto inner = layerNFL->getYs();
	auto outer = layerRPE->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerIPL->initialize(outs, width, height);
	*/

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * LAYER_IPL_CURVE_SIZE);
	int size = (int)path.size();

	auto inner = layerNFL->getYs();
	auto outer = layerOPL->getYs();
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, LAYER_IPL_CURVE_DEGREE);
	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerIPL->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerOPL(const std::vector<int>& path, int width, int height, SegmLayer * layerIPL, SegmLayer * layerRPE, SegmLayer * layerOPL, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_OPL_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_OPL_CURVE_DEGREE);
	auto inner = layerIPL->getYs();
	auto outer = layerRPE->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerOPL->initialize(outs, width, height);
	*/

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * LAYER_OPL_CURVE_SIZE);
	int size = (int)path.size();

	auto inner = layerIPL->getYs();
	auto outer = layerRPE->getYs();
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, LAYER_OPL_CURVE_DEGREE);
	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerOPL->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerIOS(const std::vector<int>& path, int width, int height, SegmLayer * layerOPL, SegmLayer * layerRPE, SegmLayer * layerIOS, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_IOS_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_IOS_CURVE_DEGREE);
	auto inner = layerOPL->getYs();
	auto outer = layerRPE->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerIOS->initialize(outs, width, height);
	*/

	int curvSize = isDisc ? LAYER_IOS_CURVE_SIZE_DISC : LAYER_IOS_CURVE_SIZE;
	int curvDegree = isDisc ? LAYER_IOS_CURVE_DEGREE_DISC : LAYER_IOS_CURVE_DEGREE;

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * curvSize);
	int size = (int)path.size();

	auto inner = layerOPL->getYs();
	auto outer = layerRPE->getYs();
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, curvDegree);
	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerIOS->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerBRM(const std::vector<int>& path, int width, int height, SegmLayer * layerRPE, SegmLayer * layerBRM, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_BRM_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_BRM_CURVE_DEGREE);
	auto inner = layerRPE->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerBRM->initialize(outs, width, height);
	*/

	int curvSize = isDisc ? LAYER_BRM_CURVE_SIZE_DISC : LAYER_BRM_CURVE_SIZE;
	int curvDegree = isDisc ? LAYER_BRM_CURVE_DEGREE_DISC : LAYER_BRM_CURVE_DEGREE;

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * curvSize);
	int size = (int)path.size();

	auto inner = layerRPE->getYs();
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, curvDegree);
	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerBRM->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerBRM2(SegmLayer * layerOut, SegmLayer * layerBRM)
{
	int size = (int)layerBRM->getYs().size();
	int width = size;
	int height = 768;

	int curvSize = (int)(size * 0.025f);
	int curvDegree = LAYER_BRM_CURVE_DEGREE;

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * curvSize);

	auto outs = layerBRM->getYs();

	outs = CppUtil::SgFilter::smoothInts(outs, filter, curvDegree);
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerOut->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothLayerRPE(const std::vector<int>& path, int width, int height, SegmLayer * layerNFL, SegmLayer * layerBRM, SegmLayer * layerRPE, bool isDisc, int discX1, int discX2)
{
	/*
	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int size = (int)(ratio * LAYER_RPE_CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, size, LAYER_RPE_CURVE_DEGREE);
	auto inner = layerIOS->getYs();
	auto outer = layerBRM->getYs();

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerRPE->initialize(outs, width, height);
	*/

	int curvSize = isDisc ? LAYER_RPE_CURVE_SIZE_DISC : LAYER_RPE_CURVE_SIZE;
	int curvDegree = isDisc ? LAYER_RPE_CURVE_DEGREE_DISC : LAYER_RPE_CURVE_DEGREE;

	float ratio = (float)width / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * curvSize);
	int size = (int)path.size();

	auto inner = layerNFL->getYs();
	auto outer = layerBRM->getYs();
	auto layer = layerRPE->getYs();
	auto outs = path;

	transform(begin(inner), end(inner), begin(outer), begin(layer), [](int elem1, int elem2) { return (int)(elem1 + (elem2 - elem1) * 0.66f); });

	outs = CppUtil::SgFilter::smoothInts(outs, filter, curvDegree);
	transform(begin(outs), end(outs), begin(layer), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerRPE->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveILM(const std::vector<int>& path, int width, int height, SegmLayer * layerILM, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		outs = CppUtil::SgFilter::smoothInts(path, PATH_ILM_CURVE_SIZE, PATH_ILM_CURVE_DEGREE);
	}
	else {
		outs = CppUtil::SgFilter::smoothInts(path, PATH_ILM_CURVE_SIZE_DISC, PATH_ILM_CURVE_DEGREE_DISC);
		auto subs = CppUtil::SgFilter::smoothInts(path, PATH_ILM_CURVE_SIZE, PATH_ILM_CURVE_DEGREE);
		
		int knees = (int)(size * 0.035f);
		for (int i = (discX1 - knees); i >= 0; i--) {
			if (subs[i] == outs[i]) {
				copy(subs.begin(), subs.begin() + i, outs.begin());
				break;
			}
		}
		for (int i = (discX2 + knees); i < size; i++) {
			if (subs[i] == outs[i]) {
				copy(subs.begin() + i, subs.end(), outs.begin() + i);
				break;
			}
		}
	}

	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerILM->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveOPR(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, SegmLayer * layerOPR, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_OPR_CURVE_SIZE, PATH_OPR_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_OPR_CURVE_SIZE_DISC, PATH_OPR_CURVE_DEGREE_DISC);
	}

	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerOPR->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveIOS(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer * layerIOS, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_IOS_CURVE_SIZE, PATH_IOS_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_IOS_CURVE_SIZE_DISC, PATH_IOS_CURVE_DEGREE_DISC);
	}

	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2 - 12); });
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerIOS->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveBRM(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, SegmLayer * layerCHR, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_BRM_CURVE_SIZE, PATH_BRM_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_BRM_CURVE_SIZE_DISC, PATH_BRM_CURVE_DEGREE_DISC);
	}

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerCHR->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveRPE(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer * layerRPE, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();
	// auto base = inner;

	// transform(begin(inner), end(inner), begin(outer), begin(base), [](int elem1, int elem2) { return (int)(elem1 + (elem2 - elem1) * 0.66f); });

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_RPE_CURVE_SIZE, PATH_RPE_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_RPE_CURVE_SIZE_DISC, PATH_RPE_CURVE_DEGREE_DISC);
	}

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });
	layerRPE->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveOPL(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer * layerOPL, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, 17, PATH_OPL_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, 17, PATH_IOS_CURVE_DEGREE_DISC);
	}

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerOPL->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveIPL(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer * layerIPL, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_IPL_CURVE_SIZE, PATH_IPL_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, 17, PATH_IPL_CURVE_DEGREE);
	}

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerIPL->initialize(outs, width, height);
	return true;
}


bool RetSegm::Smooth::smoothCurveNFL(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer * layerNFL, bool isDisc, int discX1, int discX2)
{
	if (path.empty()) {
		return false;
	}

	bool found = (isDisc && (discX2 - discX1) > 1);
	auto outs = path;
	auto size = (int)path.size();

	if (!found) {
		transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, PATH_NFL_CURVE_SIZE, PATH_NFL_CURVE_DEGREE);
	}
	else {
		for (int i = 0; i < discX1; i++) {
			outs[i] = max(path[i], inner[i]);
		}
		for (int i = (discX2 + 1); i < size; i++) {
			outs[i] = max(path[i], inner[i]);
		}

		// transform(begin(path), end(path), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
		outs = CppUtil::SgFilter::smoothInts(outs, 17, 1);
	}

	transform(begin(outs), end(outs), begin(inner), begin(outs), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outer), begin(outs), [](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return min(max(elem, 0), height - 1); });
	layerNFL->initialize(outs, width, height);
	return true;
}


int RetSegm::Smooth::selectPointsNFL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL, SegmLayer * layerNFL)
{
	Mat srcMat = imgSrc->getCvMat();

	auto ilms = layerILM->getYs();
	auto nfls = layerNFL->getYs();
	auto ipls = layerIPL->getYs();
	auto tops = imgSrc->getColumMaxVals();

	int move = 0;

	int mean = (int)(imgSrc->getMean() * 4.0f);
	int base;
	int gmax, gcnt;
	for (int c = 0; c < srcMat.cols; c++) {
		gmax = gcnt = 0;
		for (int r = nfls[c]; r <= ipls[c]; r++) {
			gmax = max((int)(srcMat.at<uint8_t>(r, c)), gmax);
			gcnt++;
		}

		base = (int)(tops[c] * 0.75f);
		if (gmax > base && gmax > mean && gcnt > 5) {
			nfls[c] = ipls[c];
			move++;
		}
	}

	if (move > 0) {
		layerNFL->initialize(nfls, srcMat.cols, srcMat.rows);
	}
	return move;
}
