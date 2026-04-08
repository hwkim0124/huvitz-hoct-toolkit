#include "stdafx.h"
#include "Smooth2.h"
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


Smooth2::Smooth2()
{
}


Smooth2::~Smooth2()
{
}

bool RetSegm::Smooth2::smoothBoundaryINN(const std::vector<int> path, std::vector<int>& layer)
{
	const int CURVE_SIZE = 21;
	const int CURVE_DEGREE = 1;

	auto size = path.size();
	float ratio = (float)size / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, filter, CURVE_DEGREE);

	layer = outs;
	return true;
}

bool RetSegm::Smooth2::smoothBoundaryOUT(const std::vector<int> path, const std::vector<int> inner, std::vector<int>& layer)
{
	const int CURVE_SIZE = 5;
	const int CURVE_DEGREE = 1;

	auto size = path.size();
	float ratio = (float)size / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, filter, CURVE_DEGREE);

	for (int i = 0; i < size; i++) {
		if (outs[i] < inner[i]) {
			outs[i] = inner[i];
		}
	}
	layer = outs;
	return true;
}

bool RetSegm::Smooth2::smoothLayerEPI(const std::vector<int>& path, std::vector<int>& layer)
{
	const int CURVE_SIZE = 9;
	const int CURVE_DEGREE = 1;

	auto size = path.size();
	float ratio = (float)size / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, filter, CURVE_DEGREE);

	layer = outs;
	return true;
}

bool RetSegm::Smooth2::smoothLayerEND(const std::vector<int>& path, SegmLayer * layerEPI, std::vector<int>& layer)
{
	const int CURVE_SIZE = 9;
	const int CURVE_DEGREE = 1;

	auto size = path.size();
	float ratio = (float)size / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, filter, CURVE_DEGREE);

	auto inner = layerEPI->getYs();
	for (int i = 0; i < size; i++) {
		if (outs[i] < inner[i]) {
			outs[i] = inner[i];
		}
	}

	layer = outs;
	return true;
}

bool RetSegm::Smooth2::smoothLayerEND2(const std::vector<int>& path, SegmLayer * layerEPI, std::vector<int>& layer)
{
	const int CURVE_SIZE = 9;
	const int CURVE_DEGREE = 1;

	auto size = path.size();
	float ratio = (float)size / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, filter, CURVE_DEGREE);

	auto inner = layerEPI->getYs();
	for (int i = 0; i < size; i++) {
		if (outs[i] < inner[i]) {
			outs[i] = inner[i];
		}
	}

	layer = outs;
	return true;
}

bool RetSegm::Smooth2::smoothLayerBOW(const std::vector<int>& path, SegmLayer * layerEPI, SegmLayer * layerEND, std::vector<int>& layer)
{
	const int CURVE_SIZE = 13;
	const int CURVE_DEGREE = 1;

	auto size = path.size();
	float ratio = (float)size / SAMPLE_IMAGE_WIDTH;
	int filter = (int)(ratio * CURVE_SIZE);

	auto outs = CppUtil::SgFilter::smoothInts(path, filter, CURVE_DEGREE);

	auto inner = layerEPI->getYs();
	auto outer = layerEND->getYs();
	for (int i = 0; i < size; i++) {
		outs[i] = min(max(outs[i], inner[i]), outer[i]);
	}

	layer = outs;
	return true;
}


