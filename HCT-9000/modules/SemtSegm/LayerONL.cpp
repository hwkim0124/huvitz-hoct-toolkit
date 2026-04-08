#include "stdafx.h"
#include "LayerONL.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerONL::LayerONLImpl
{

	LayerONLImpl()
	{
	}
};


LayerONL::LayerONL(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerONLImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerONL::~LayerONL() = default;
SemtSegm::LayerONL::LayerONL(LayerONL && rhs) = default;
LayerONL & SemtSegm::LayerONL::operator=(LayerONL && rhs) = default;


bool SemtSegm::LayerONL::buildOutlinePath()
{
	/*
	if (!designOutlineConstraints()) {
	return false;
	}
	if (!createOutlineCostMap()) {
	return false;
	}
	if (!searchPathMinCost()) {
	return false;
	}

	smoothOutlnePath();
	*/
	filterOuterPoints();
	return true;
}


bool SemtSegm::LayerONL::designOutlineConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_bound = pSegm->getBodering()->getInnerBound();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();
	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	const int UPPER_SPAN = 35;
	const int LOWER_SPAN = 15;
	const int MOVE_DELTA = 3;
	const int FILTER_SIZE = 17;
	const int CURVE_DEGREE = 1;

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, height - 1);
	deltas = vector<int>(width, MOVE_DELTA);

	auto fitt = out_edges;
	auto line = out_edges;

	if (!interpolateByLinearFitt(out_edges, fitt, true)) {
		return false;
	}

	line = CppUtil::SgFilter::smoothInts(fitt, FILTER_SIZE, CURVE_DEGREE);
	transform(begin(fitt), end(fitt), begin(inn_layer), begin(fitt), [](int elem1, int elem2) { return max(elem1, elem2); });

	Mat weight = Mat::ones(height, width, CV_32F);

	for (int c = 0; c < width; c++) {
		uppers[c] = inn_layer[c]; // innBound[c];
		lowers[c] = out_bound[c];

		if (out_edges[c] >= 0) {
			/*
			for (int r = uppers[c]; r <= lowers[c]; r++) {
			auto dist = abs(r - out_edges[c]);
			weight.at<float>(r, c) = 1.0f / (dist + 1.0f);
			}
			*/
		}
		else {
			/*
			for (int r = uppers[c]; r <= lowers[c]; r++) {
			auto dist = abs(r - out_bound[c]);
			weight.at<float>(r, c) = 1.0f / (dist + 1.0f);
			}
			*/
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}

bool SemtSegm::LayerONL::createOutlineCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->average();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pImage->getMean()*0.0f + pImage->getStddev()*0.0f);
	int gray_max = (int)(pImage->getMean() + pImage->getStddev()*1.0f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 13; // 13;
	const int KERNEL_COLS = 13; // 7;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	outMat = outMat.mul(pWeight->getCvMatConst());

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		for (int r = uppers[c]; r <= lowers[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - deltas[c - 1]; k <= r + deltas[c - 1]; k++) {
				if (k >= uppers[c - 1] && k <= lowers[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	getCostMap()->getCvMat() = outMat;
	return true;
}

bool SemtSegm::LayerONL::smoothOutlnePath()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_bound = pSegm->getBodering()->getInnerBound();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();

	const int FILTER_SIZE = 27;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);
	return true;

	auto fitt = out_edges;
	interpolateByLinearFitt(out_edges, fitt, true);
	path = fitt;

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);
	return true;
}

bool SemtSegm::LayerONL::filterOuterPoints()
{
	auto* pSegm = getSegmentator();
	auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();

	const int FILTER_SIZE = 17;
	const int CURVE_DEGREE = 1;
	const int UPPER_SPAN = 35;

	auto& path = getOptimalPath();
	auto fitt = out_edges;
	auto line = out_edges;

	int size = (int)out_edges.size();
	int spos = (int)(size * 0.15f);
	int epos = (int)(size * 0.85f);

	for (int i = 0; i < 10; i++) {
		if (!interpolateByLinearFitt(out_edges, fitt, true)) {
			return false;
		}

		transform(begin(fitt), end(fitt), begin(inn_layer), begin(fitt), [](int elem1, int elem2) { return max(elem1, elem2); });
		line = CppUtil::SgFilter::smoothInts(fitt, FILTER_SIZE, CURVE_DEGREE);
		break;

		int count = 0;
		for (int c = 0; c < size; c++) {
			if (out_edges[c] >= 0) {
				if (c >= spos && c <= epos) {
					int dist = line[c] - out_edges[c];
					if (dist > UPPER_SPAN) {
						out_edges[c] = -1;
						count += 1;
						continue;
					}
				}

				int dist1 = abs(inn_layer[c] - out_edges[c]);
				int dist2 = abs(line[c] - out_edges[c]);
				if (dist2 > dist1) {
					out_edges[c] = -1;
					count += 1;
				}
			}
		}

		if (count == 0) {
			break;
		}
	}

	path = line;
	return true;
}

LayerONL::LayerONLImpl & SemtSegm::LayerONL::getImpl(void) const
{
	return *d_ptr;
}


