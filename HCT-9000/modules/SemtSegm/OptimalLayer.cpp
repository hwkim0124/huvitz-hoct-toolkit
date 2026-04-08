#include "stdafx.h"
#include "OptimalLayer.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace CppUtil;
using namespace SemtSegm;
using namespace std;
using namespace cv;


struct OptimalLayer::OptimalLayerImpl
{
	const BscanSegmentator* pSegm = nullptr;

	vector<int> uppers;
	vector<int> lowers;
	vector<int> deltas;
	vector<int> minPath;

	OcularImage imgCost;
	OcularImage imgWeight;
	OcularImage imgBiases;

	int regionWidth = 256;
	int regionHeight = 384;

	OptimalLayerImpl()
	{
		minPath = vector<int>(regionWidth, -1);
	}
};



OptimalLayer::OptimalLayer(BscanSegmentator* pSegm) :
	d_ptr(make_unique<OptimalLayerImpl>())
{
	getImpl().pSegm = pSegm;
}


SemtSegm::OptimalLayer::~OptimalLayer() = default;
SemtSegm::OptimalLayer::OptimalLayer(OptimalLayer && rhs) = default;
OptimalLayer & SemtSegm::OptimalLayer::operator=(OptimalLayer && rhs) = default;


bool SemtSegm::OptimalLayer::searchPathMinCost()
{
	const auto matCost = getCostMap()->getCvMatConst();
	auto minPath = vector<int>(matCost.cols, -1);

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	// Starting from the right most column. 
	int lastIdx = -1, nextIdx = -1;
	float minCost = PATH_COST_MAX;
	int r, c;

	c = matCost.cols - 1;
	for (r = uppers[c]; r <= lowers[c]; r++) {
		if (matCost.at<float>(r, c) <= minCost) {
			minCost = matCost.at<float>(r, c);
			lastIdx = r;
		}
	}
	minPath[c] = (lastIdx < 0 ? ((uppers[c] + lowers[c]) / 2) : lastIdx);
	lastIdx = minPath[c];

	// Follow the minimum cost path in reverse order. 
	// Next point should be within the allowed vertical distance. 
	int rBeg, rEnd;
	for (c = matCost.cols - 2; c >= 0; c--) {
		rBeg = max(lastIdx - deltas[c], uppers[c]);
		rEnd = min(lastIdx + deltas[c], lowers[c]);

		if (rBeg > lowers[c] || rEnd < uppers[c]) {
			minPath[c] = (rBeg > lowers[c] ? lowers[c] : uppers[c]);
		}
		else {
			// Note that if any element with cost calculated from intensities (less than maximum at default) 
			// is not found at this column, the last index would go down toward the bottom of map. 
			minCost = PATH_COST_MAX;
			nextIdx = -1;
			for (r = rBeg; r <= rEnd; r++) {
				if (matCost.at<float>(r, c) <= minCost) {
					minCost = matCost.at<float>(r, c);
					nextIdx = r;
				}
			}
			minPath[c] = (nextIdx < 0 ? ((uppers[c] + lowers[c]) / 2) : nextIdx);
		}
		lastIdx = minPath[c];
	}

	getImpl().minPath = minPath;
	return true;
}

bool SemtSegm::OptimalLayer::interpolateByLinearFitt(const std::vector<int>& input, std::vector<int>& output, 
								bool sideFitt, int moveSpan, float sideData)
{
	const int fittSize = (int)(sideData * input.size());

	float slope = 0.0f;
	int count = 0;
	int size = (int)input.size();
	output = input;

	int x1 = -1, x2 = -1;
	int y1 = -1, y2 = -1;

	if (input[0] < 0) {
		for (int i = 0; i < size; i++) {
			if (input[i] >= 0) {
				if (!sideFitt) {
					for (int j = 0; j < i; j++) {
						output[j] = input[i];
					}
				}
				else {
					int xs1 = 0, xs2 = 0;
					int ys1 = 0, ys2 = 0;
					int cnt1 = 0, cnt2 = 0;
					int j;

					for (j = i; j < size; j++) {
						if (input[j] >= 0) {
							xs1 += j;
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								x1 = xs1 / cnt1;
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (j = j + 1; j < size; j++) {
						if (input[j] >= 0) {
							xs2 += j;
							ys2 += input[j];
							if (++cnt2 >= fittSize) {
								x2 = xs2 / cnt2;
								y2 = ys2 / cnt2;
								break;
							}
						}
					}

					if (x1 >= 0 && x2 >= 0 && x2 > x1) {
						slope = (float)(y2 - y1) / (float)(x2 - x1);
						int y_last = input[x1];
						int y_pred = 0;
						for (int k = (x1 - 1), dist = -1; k >= 0; k--, dist--) {
							y_pred = (int)(y1 + dist * slope);
							// y_last = min(y_last + moveSpan, max(y_last - moveSpan, y_pred));
							// output[k] = y_last;
							output[k] = y_pred;
						}
					}
				}
				break;
			}
		}
	}

	if (input[size - 1] < 0) {
		for (int i = size - 1; i >= 0; i--) {
			if (input[i] >= 0) {
				if (!sideFitt) {
					for (int j = i + 1; j < size; j++) {
						output[j] = input[i];
					}
				}
				else {
					int xs1 = 0, xs2 = 0;
					int ys1 = 0, ys2 = 0;
					int cnt1 = 0, cnt2 = 0;
					int j;

					for (j = i - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							xs2 += j;
							ys2 += input[j];
							if (++cnt2 >= fittSize) {
								x2 = xs2 / cnt2;
								y2 = ys2 / cnt2;
								break;
							}
						}
					}

					for (j = j - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							xs1 += j;
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								x1 = xs1 / cnt1;
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					if (x1 >= 0 && x2 >= 0 && x2 > x1) {
						slope = (float)(y2 - y1) / (float)(x2 - x1);
						int y_last = input[x2];
						int y_pred = 0;
						for (int k = (x2 + 1), dist = 1; k < size; k++, dist++) {
							y_pred = (int)(y2 + dist * slope);
							// y_last = min(y_last + moveSpan, max(y_last - moveSpan, y_pred));
							// output[k] = y_last;
							output[k] = y_pred;
						}
					}
				}
				break;
			}
		}
	}

	if (output[0] < 0 || output[size - 1] < 0) {
	// 	return false;
	}

	for (int i = 0; i < size; i++)
	{
		if (output[i] >= 0) {
			x1 = i;
			y1 = output[i];
		}
		else
		{
			for (int j = i + 1; j < size; j++) {
				if (output[j] >= 0) {
					x2 = j;
					y2 = output[j];

					slope = (float)(y2 - y1) / (float)(x2 - x1);
					int y_last = y1;
					int y_pred = 0;

					for (int k = (x1 + 1), dist = 1; k < x2; k++, dist++) {
						y_pred = (int)(y1 + dist * slope);
						// y_last = min(y_last + moveSpan, max(y_last - moveSpan, y_pred));
						// output[k] = max(y_last, max(y1, y2));
						output[k] = y_pred;
					}
					i = x2;
					break;
				}
			}
		}
	}
	return true;
}

OcularImage * SemtSegm::OptimalLayer::getCostMap() const
{
	return &getImpl().imgCost;
}

OcularImage * SemtSegm::OptimalLayer::getWeightMap() const
{
	return &getImpl().imgWeight;
}

OcularImage * SemtSegm::OptimalLayer::getBiasesMap() const
{
	return &getImpl().imgBiases;
}

std::vector<int>& SemtSegm::OptimalLayer::getUppers() const
{
	return getImpl().uppers;
}

std::vector<int>& SemtSegm::OptimalLayer::getLowers() const
{
	return getImpl().lowers;
}

std::vector<int>& SemtSegm::OptimalLayer::getDeltas() const
{
	return getImpl().deltas;
}

std::vector<int> SemtSegm::OptimalLayer::getPoints(int shift) const
{
	auto points = getOptimalPath();

	if (shift != 0) {
		int width = getImpl().regionWidth;
		int height = getImpl().regionHeight;

		transform(begin(points), end(points), begin(points), [=](int elem) { return elem + shift; });
		transform(begin(points), end(points), begin(points), [=](int elem) { return min(max(elem, 0), height - 1); });
	}

	return points;
}

std::vector<int>& SemtSegm::OptimalLayer::getOptimalPath() const
{
	return getImpl().minPath;
}

void SemtSegm::OptimalLayer::setRegionDimension(int width, int height, bool stretchY)
{
	auto& path = getOptimalPath();
	int reg_w = getImpl().regionWidth;
	int reg_h = getImpl().regionHeight;

	if (width <= 0 || height <= 0) {
		return;
	}

	if (reg_w != width || reg_h != height) {
		// Layer points within region should be updated. 
		if (!path.empty()) {
			float w_ratio = (float)reg_w / (float)width;
			float h_ratio = (float)height / (float)reg_h;
			auto dest = vector<int>(width, -1);

			if (!stretchY) {
				h_ratio = 1.0f;
			}

			for (int i = 0; i < width; i++) {
				int src = path[(int)(i*w_ratio)];
				// Only valid point transformed. 
				if (src >= 0) {
					dest[i] = (int)(src*h_ratio);
				}
			}

			path = dest;
		}
		getImpl().regionWidth = width;
		getImpl().regionHeight = height;
	}
	else {
		// Same dimension, nothing changed. 
	}
	return;
}

void SemtSegm::OptimalLayer::applyVeritcalShifts(std::vector<int> shifts, bool inverse)
{
	auto& path = getOptimalPath();
	int reg_w = getImpl().regionWidth;
	int reg_h = getImpl().regionHeight;

	int dsize = (int)shifts.size();
	if (dsize != path.size() || dsize != reg_w) {
		return;
	}

	int direct = (inverse ? -1 : +1);

	std::transform(path.cbegin(), path.cend(), shifts.cbegin(), path.begin(), [=](int elem1, int elem2) { return elem1 + elem2 * direct; });
	// std::transform(path.cbegin(), path.cend(), path.begin(), [=](int elem) { return min(max(elem, 0), reg_h - 1); });
	return;
}

const BscanSegmentator * SemtSegm::OptimalLayer::getSegmentator() const
{
	return getImpl().pSegm;
}

OptimalLayer::OptimalLayerImpl & SemtSegm::OptimalLayer::getImpl(void) const
{
	return *d_ptr;
}


