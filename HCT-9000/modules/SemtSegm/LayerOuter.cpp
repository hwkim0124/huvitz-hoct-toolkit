#include "stdafx.h"
#include "LayerOuter.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>

struct LayerOuter::LayerOuterImpl
{
	int raised_bump_x1 = -1;
	int raised_bump_x2 = -1;;

	LayerOuterImpl()
	{
	}
};



LayerOuter::LayerOuter(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerOuterImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerOuter::~LayerOuter() = default;
SemtSegm::LayerOuter::LayerOuter(LayerOuter && rhs) = default;
LayerOuter & SemtSegm::LayerOuter::operator=(LayerOuter && rhs) = default;

bool SemtSegm::LayerOuter::buildOutlinePath()
{
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
	return true;
}

bool SemtSegm::LayerOuter::buildBoundaryPath()
{
	if (!designBorderConstraints()) {
		return false;
	}
	if (!createBorderCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}

	smoothBorderPath();
	return true;
}

bool SemtSegm::LayerOuter::alterBoundaryPath(bool withDisc)
{
	if (!designBorderConstraints2()) {
		return false;
	}
	if (!createBorderCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}

	if (!withDisc) {
		smoothBorderPath();
	}
	else {
		smoothBorderPathWithDisc();
	}
	return true;
}


bool SemtSegm::LayerOuter::designOutlineConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_bound = pSegm->getBodering()->getInnerBound();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();

	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	const auto& out_border = pSegm->getBodering()->getOuterBorder();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	const int UPPER_SPAN = 45;
	const int LOWER_SPAN = -9;
	const int MOVE_DELTA = 5;

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, height - 1);
	deltas = vector<int>(width, MOVE_DELTA);

	Mat weight = Mat::ones(height, width, CV_32F);
	Mat srcMat = pImage->getCvMatConst();

	for (int c = 0; c < width; c++) {
		uppers[c] = inn_layer[c] + (out_border[c] - inn_layer[c]) / 4;
		lowers[c] = max(out_border[c] + LOWER_SPAN, uppers[c]);
	}

	pWeight->getCvMat() = weight;
	return true;
}


bool SemtSegm::LayerOuter::designBorderConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();
	const auto& inn_border = pSegm->getBodering()->getInnerBorder();
	const auto& out_border = pSegm->getBodering()->getOuterBorder();
	const auto& out_layer = getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	const int MOVE_DELTA = 5;

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, height - 1);
	deltas = vector<int>(width, MOVE_DELTA);

	Mat weight = Mat::ones(height, width, CV_32F);
	bool is_disc = pSegm->getBodering()->isOpticDiscRegion();
	int disc_x1 = pSegm->getBodering()->getOpticDiscX1();
	int disc_x2 = pSegm->getBodering()->getOpticDiscX2();

	for (int c = 0; c < width; c++) {
		uppers[c] = min(inn_border[c], out_border[c]);
		lowers[c] = max(out_bound[c], uppers[c]);

		if (out_edges[c] < 0) {
			int y1 = uppers[c];
			int y2 = lowers[c];

			for (int r = y1; r <= y2; r++) {
				float ratio = max(1.0f - 0.05f * abs(r - out_border[c]), 0.001f);
				weight.at<float>(r, c) = ratio;
			}
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}


bool SemtSegm::LayerOuter::designBorderConstraints2()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();
	const auto& inn_border = pSegm->getBodering()->getInnerBorder();
	const auto& out_border = pSegm->getBodering()->getOuterBorder();
	const auto& out_layer = getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	const int MOVE_DELTA = 5;

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, height - 1);
	deltas = vector<int>(width, MOVE_DELTA);

	Mat weight = Mat::ones(height, width, CV_32F);
	bool is_disc = pSegm->getBodering()->isOpticDiscRegion();
	int disc_x1 = pSegm->getBodering()->getOpticDiscX1();
	int disc_x2 = pSegm->getBodering()->getOpticDiscX2();

	for (int c = 0; c < width; c++) {
		uppers[c] = min(inn_border[c], out_border[c]);
		lowers[c] = max(out_bound[c], uppers[c]);

		if (out_edges[c] < 0) {
			int y1 = uppers[c];
			int y2 = lowers[c];

			if (!is_disc || c < disc_x1 || c > disc_x2) {
				for (int r = y1; r <= y2; r++) {
					float ratio = max(1.0f - 0.075f * abs(r - out_border[c]), 0.0001f);
					weight.at<float>(r, c) = ratio;
				}
			}
			else {
				y1 = max(y1, out_border[c] - 5);
				y2 = max(min(y2, out_border[c] + 5), y1);

				for (int r = y1; r <= y2; r++) {
					float ratio = max(1.0f - 0.15f * abs(r - out_border[c]), 0.00001f);
					weight.at<float>(r, c) = ratio;
				}

				uppers[c] = y1;
				lowers[c] = y2;
			}
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}


bool SemtSegm::LayerOuter::createOutlineCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->average();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	const int KERNEL_ROWS = 13;
	const int KERNEL_COLS = 5; // 7;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

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


bool SemtSegm::LayerOuter::createBorderCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->sample();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	const int KERNEL_ROWS = 13;
	const int KERNEL_COLS = 5; // 7;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);
	
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

bool SemtSegm::LayerOuter::smoothOutlnePath()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_bound = pSegm->getBodering()->getInnerBound();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();

	const int FILTER_SIZE = 7; // 17;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int rows = pImage->getHeight();

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerOuter::smoothBorderPath()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_bound = pSegm->getBodering()->getInnerBound();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();

	const int FILTER_SIZE = 7; // 17;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int rows = pImage->getHeight();

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerOuter::smoothBorderPathWithDisc()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->ascent();
	auto* pWeight = getWeightMap();
	auto* pBorder = pSegm->getBodering();

	const auto& out_edges = pBorder->getOuterEdges();
	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	
	const int FILTER_SIZE = 7; // 17;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int cols = pImage->getWidth();
	int rows = pImage->getHeight();

	if (pBorder->isOpticDiscOpening()) {
		int base_x = pBorder->getOpticDiscBaseX();
		int base_y = pBorder->getOpticDiscBaseY();

		for (int x = base_x - 1; x >= 0; x--) {
			if (out_edges[x] >= 0) {
				int open_y = out_edges[x];
				float rate = (float)(base_y - open_y) / (float)(base_x - x);
				int dist = 0;
				for (; x < base_x; x++) {
					path[x] = (int)(open_y + dist * rate);
					dist += 1;
				}
				break;
			}
		}

		for (int x = base_x + 1; x < cols; x++) {
			if (out_edges[x] >= 0) {
				int open_y = out_edges[x];
				float rate = (float)(base_y - open_y) / (float)(x - base_x);
				int dist = 0;
				for (; x > base_x; x--) {
					path[x] = (int)(open_y + dist * rate);
					dist += 1;
				}
				break;
			}
		}
	}

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerOuter::makeupOutlineInterpolation(std::vector<int>& result)
{
	auto* pSegm = getSegmentator();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();

	int cols = pSegm->getSampling()->sample()->getWidth();
	int rows = pSegm->getSampling()->sample()->getHeight();

	const int FILTER_SIZE = 17;
	const int CURVE_DEGREE = 1;

	const int BASE_LOBE = 7;
	const int DEPTH_MIN = 9;
	const int DEPTH_MAX = 15;
	const int WIDTH_MIN = 45;
	const int WIDTH_MAX = 180;
	const int PEAK_OFFSET = 5;

	auto fitt = out_edges;
	auto data = out_edges;
	auto line = out_edges;

	int size = (int) out_edges.size();
	int spos = max((int)(size * 0.10f), 0);
	int epos = min((int)(size * 0.90f), size-1);

	for (int i = 0; i < 20; i++) {
		if (!interpolateByLinearFitt(data, fitt, true)) {
			return false;
		}

		line = CppUtil::SgFilter::smoothInts(fitt, FILTER_SIZE, CURVE_DEGREE);
		// break;

		int cnt = 0;
		for (int c = spos; c <= epos; c++) {
			if (data[c] >= 0) {
				int dist = (data[c] - line[c]);
				if (dist < -5) {
					data[c] = -1;
					cnt++;
				}
			}
		}

		if (cnt > 0) {
			continue;
		}
		else {
			break;
		}

		int last_x = 0;

		// Remove outliers.
		int count = 0;
		int width, depth;
		for (int c = spos; c <= epos; c++) {
			int peak_x1 = c;
			int peak_x2 = c;
			int peak_y1 = line[c];
			int peak_y2 = line[c];
			int peak_cy = line[c];
			int peak_cx = c;
			int peak_d1 = 0;
			int peak_d2 = 0;

			for (int k = c - 1; k >= last_x; k--) {
				if (line[k] >= peak_y1) {
					peak_x1 = k;
					peak_y1 = line[k];
				}
				else {
					if (line[k] < peak_cy) {
						peak_cx = k;
						peak_cy = line[k];
					}
					if ((peak_y1 - line[k]) > BASE_LOBE) {
						break;
					}
				}
			}
			
			peak_d1 = peak_y1 - peak_cy;
			if (peak_d1 < DEPTH_MIN || peak_cx < peak_x1 || peak_cx < spos) {
				last_x = peak_x1;
				continue;
			}

			for (int k = c + 1; k < size; k++) {
				if (line[k] >= peak_y2) {
					peak_x2 = k;
					peak_y2 = line[k];
				}
				else {
					if (line[k] < peak_cy) {
						peak_cx = k;
						peak_cy = line[k];
					}
					if ((peak_y2 - line[k]) > BASE_LOBE) {
						break;
					}
				}
			}

			peak_d2 = peak_y2 - peak_cy;
			if (peak_d2 < DEPTH_MIN || peak_cx > peak_x2 || peak_cx > epos) {
				last_x = peak_x1;
				continue;
			}

			width = peak_x2 - peak_x1 + 1;
			depth = max(peak_d1, peak_d2);

			if (width > WIDTH_MIN && width < WIDTH_MAX && depth > DEPTH_MAX) {
				getImpl().raised_bump_x1 = peak_x1;
				getImpl().raised_bump_x2 = peak_x2;
				for (int k = peak_x1; k <= peak_x2; k++) {
					if (data[k] >= 0) {
						int dist = line[k] - data[k];
						if (dist > PEAK_OFFSET || peak_cy > data[k]) {
							data[k] = -1;
							count += 1;
						}
					}
				}
					
				LogD() << "Index: " << pSegm->getImageIndex() << ", peak width: " << width << ", depth: " << depth << ", count: " << count << ", x1: " << peak_x1 << ", x2: " << peak_x2 << ", cx: " << peak_cx << ", cy: " << peak_cy << ", y1: " << peak_y1 << ", y2: " << peak_y2;
			}

			last_x = peak_x2;
		}

		for (int c = spos; c < epos; c++) {
			if (data[c] >= 0) {
				int depth = line[c] - inn_layer[c];
				if (depth > DEPTH_MAX) {
					int dist1 = data[c] - inn_layer[c];
					int dist2 = line[c] - data[c];
					if (dist1 < (dist2 + 1)) {
						data[c] = -1;
						count += 1;
					}
				}
			}
		}

		if (count == 0) {
			break;
		}
	}

	// pSegm->getBodering()->getOuterBound() = data;
	// transform(begin(line), end(line), begin(inn_layer), begin(line), [](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(line), end(line), begin(line), [=](int elem) { return min(max(elem, 0), rows - 1); });
	result = line;
	return true;
}

LayerOuter::LayerOuterImpl & SemtSegm::LayerOuter::getImpl(void) const
{
	return *d_ptr;
}


