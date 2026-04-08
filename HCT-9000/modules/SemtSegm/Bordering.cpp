#include "stdafx.h"
#include "Bordering.h"
#include "Sampling.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace CppUtil;
using namespace SemtSegm;
using namespace std;
using namespace cv;

#include <algorithm>

struct Bordering::BorderingImpl
{
	const BscanSegmentator* pSegm = nullptr;
	const Sampling* sampling = nullptr;

	vector<int> outerEdges;
	vector<int> innerEdges;
	vector<int> outerBound;
	vector<int> innerBound;
	vector<int> centerBound;

	vector<int> outerWeights;
	vector<int> innerWeights;
	vector<int> outerBorder;
	vector<int> innerBorder;

	int discX1 = -1;
	int discX2 = -1;
	int holeX1 = -1;
	int holeX2 = -1;
	int reversedX1 = -1;
	int reversedX2 = -1;

	bool isDiscOpening = false;
	int discBaseX = -1;
	int discBaseY = -1;

	BorderingImpl()
	{
	}
};


Bordering::Bordering(BscanSegmentator* pSegm) :
	d_ptr(make_unique<BorderingImpl>())
{
	getImpl().pSegm = pSegm;
}


SemtSegm::Bordering::~Bordering() = default;
SemtSegm::Bordering::Bordering(Bordering && rhs) = default;
Bordering & SemtSegm::Bordering::operator=(Bordering && rhs) = default;


bool SemtSegm::Bordering::establishOcularBound()
{
	auto* pSegm = getImpl().pSegm;
	auto* pSample = pSegm->getSampling();
	const auto* pImage = pSegm->getSampling()->average();

	Mat avgMat = pImage->getCvMatConst();

	int cols = avgMat.cols;
	int rows = avgMat.rows;

	auto& inn_bound = getInnerBound();
	auto& out_bound = getOuterBound();

	auto maxLocsAvg = pImage->getColumMaxLocs();
	auto maxLocsVal = pImage->getColumMaxVals();

	float mean = pImage->getMean();
	float stdd = pImage->getStddev();
	
	int inn_thresh = (int)(mean * 1.0f + stdd * 0.5f);
	int out_thresh = (int)(mean * 1.0f + stdd * 0.0f);

	const int UPPER_SPAN_MAX = 210;
	const int UPPER_SPAN_MIN = 120;
	const int LOWER_MARGIN = 16;
	const int UPPER_MARGIN = 48;
	const int FILTER_SIZE = 27;
	const int CURVE_DEGREE = 1;

	inn_bound.resize(cols, -1);
	out_bound.resize(cols, -1);

	int r, c;
	for (c = 0; c < cols; c++) {
		int spos = -1;
		int epos = -1;

		// Shadowed region.
		if (maxLocsVal[c] < inn_thresh) {
			continue;
		}

		// Find the bottom of ocular body. 
		for (r = rows - 1; r >= 0; r--) {
			if (avgMat.at<uchar>(r, c) >= out_thresh) {
				epos = r;
				break;
			}
		}

		if (epos >= 0) {
			epos = max(epos, UPPER_SPAN_MIN);

			int y1 = max(epos - UPPER_SPAN_MAX, 0);
			int y2 = epos; // max(epos - UPPER_SPAN_MIN, 0);

			// Find the top of ocular body. 
			for (r = y1; r <= y2; r++) {
				if (avgMat.at<uchar>(r, c) >= inn_thresh) {
					spos = r;
					break;
				}
			}

			if (spos < 0) {
				spos = max(epos - UPPER_SPAN_MIN, 0);
			}
		}
		else {
			continue;
		}

		inn_bound[c] = spos;
		out_bound[c] = epos;
	}

	int last_inn = -1;
	int last_out = -1;
	if (inn_bound[0] < 0) {
		for (int c = 1; c < cols; c++) {
			if (inn_bound[c] > 0) {
				last_inn = inn_bound[c];
				last_out = out_bound[c];
				for (int k = 0; k < c; k++) {
					inn_bound[k] = last_inn;
					out_bound[k] = last_out;
				}
				break;
			}
		}
	}
	else {
		last_inn = inn_bound[0];
		last_out = out_bound[0];
	}

	if (inn_bound[cols-1] < 0) {
		for (int c = cols-2; c >= 0; c--) {
			if (inn_bound[c] > 0) {
				for (int k = c+1; k < cols; k++) {
					inn_bound[k] = inn_bound[c];
					out_bound[k] = out_bound[c];
				}
				break;
			}
		}
	}

	for (int c = 0; c < cols; c++) {
		if (inn_bound[c] < 0) {
			for (int k = c + 1; k < cols; k++) {
				if (inn_bound[k] >= 0) {
					last_inn = min(last_inn, inn_bound[k]);
					last_out = max(last_out, out_bound[k]);
					for (int j = c + 1; j < k; j++) {
						inn_bound[j] = last_inn;
						out_bound[j] = last_out;
					}
					c = k;
					break;
				}
			}
		}
		else {
			last_inn = inn_bound[c];
			last_out = out_bound[c];
		}
	}

	auto inn_curve = inn_bound;

	inn_bound = CppUtil::SgFilter::smoothInts(inn_bound, FILTER_SIZE, CURVE_DEGREE);
	out_bound = CppUtil::SgFilter::smoothInts(out_bound, FILTER_SIZE, CURVE_DEGREE);

	std::transform(begin(inn_bound), end(inn_bound), begin(inn_bound), [=](int elem) { return min(max(elem - UPPER_MARGIN, 0), rows - 1); });
	std::transform(begin(out_bound), end(out_bound), begin(out_bound), [=](int elem) { return min(max(elem + LOWER_MARGIN, 0), rows - 1); });

	int count = 0;
	int limit = UPPER_MARGIN / 2;
	for (int c = 0; c < cols; c++) {
		if (inn_bound[c] >= (inn_curve[c] - limit)) {
			inn_bound[c] -= UPPER_MARGIN;
			count++;
		}
	}

	if (count > 0) {
		inn_bound = CppUtil::SgFilter::smoothInts(inn_bound, FILTER_SIZE/2, CURVE_DEGREE);
		std::transform(begin(inn_bound), end(inn_bound), begin(inn_bound), [=](int elem) { return min(max(elem, 0), rows - 1); });
	}
	return true;
}

bool SemtSegm::Bordering::locateOutlinePoints()
{
	auto* pSegm = getImpl().pSegm;
	auto* pSample = pSegm->getSampling();

	const auto* pImage = pSegm->getSampling()->sample();
	const auto* pAverage = pSegm->getSampling()->average();
	const auto* pAscent = pSegm->getSampling()->ascent();

	Mat srcMat = pImage->getCvMatConst();
	Mat ascMat = pAscent->getCvMatConst();
	Mat avgMat = pAverage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	auto& inn_edges = getInnerEdges();
	auto& out_edges = getOuterEdges();
	const auto& inn_bound = getInnerBound();
	const auto& out_bound = getOuterBound();

	inn_edges.resize(cols, -1);
	out_edges.resize(cols, -1);

	float mean = pAscent->getMean();
	float stdd = pAscent->getStddev();
	float mean2 = pAverage->getMean();
	float stdd2 = pAverage->getStddev();

	const int SURF_SIZE_MIN = 3;
	const int SURF_THRESH_MIN = 25;
	const int SURF_THRESH_MAX = 65;
	const int PEAK_DIST_MIN = 35;
	const float PEAK_RATIO = 0.65f; 
	const int FILTER_SIZE = 13;
	const int CURVE_DEGREE = 1;

	int peak_thresh = (int)(mean * 1.0f + stdd * 2.0f);
	int base_thresh = (int)(mean * 0.5f + stdd * 0.5f);
	int subo_thresh = (int)(mean * 1.0f + stdd * 1.0f);

	int surf_thresh = (int)(mean2 * 1.0f + stdd2 * 0.5f);
	surf_thresh = min(max(surf_thresh, SURF_THRESH_MIN), SURF_THRESH_MAX);

	int base_thresh2 = (int)(mean2 * 1.0f + stdd2 * 0.0f);

	auto maxLocsAsc = pAscent->getColumMaxLocs();
	auto maxValsAsc = pAscent->getColumMaxVals();
	auto maxLocsAvg = pAverage->getColumMaxLocs();
	auto maxValsAvg = pAverage->getColumMaxVals();

	int r, c;
	for (c = 0; c < cols; c++) {
		int reti_spos = -1; 
		int reti_epos = -1;  
		int base_spos = -1;
		int base_epos = -1;
		int base_dist = 0;

		int avg_peak_pos = maxLocsAvg[c];
		int avg_peak_val = maxValsAvg[c];

		if (avg_peak_val < surf_thresh || avg_peak_pos < inn_bound[c] || avg_peak_pos > out_bound[c]) {
			continue;
		}

		int upp_peak_pos = -1;
		int low_peak_pos = -1;
		int upp_peak_val = 0;
		int low_peak_val = 0;
		int gval = 0;
		int grad = 0;
		int gcnt = 0;

		for (r = inn_bound[c]; r <= out_bound[c]; r++) {
			gval = avgMat.at<uchar>(r, c);
			if (gval >= surf_thresh) {
				if (reti_spos < 0) {
					reti_spos = r;
				}
				if (++gcnt > SURF_SIZE_MIN) {
					break;
				}
			}
			else {
				reti_spos = -1;
				gcnt = 0;
			}
		}

		if (reti_spos < 0) {
			continue;
		}
		else {
			upp_peak_pos = reti_spos;
			upp_peak_val = gval;
		}

		/*
		for (r = reti_spos; r <= out_bound[c]; r++) {
			gval = avgMat.at<uchar>(r, c);
			if (gval >= upp_peak_val) {
				upp_peak_pos = r;
				upp_peak_val = gval;
			}
			else {
				break;
			}
		}
		*/

		for (r = upp_peak_pos; r <= out_bound[c]; r++) {
			grad = ascMat.at<uchar>(r, c);
			if (grad <= base_thresh) {
				if (base_spos < 0) {
					base_spos = r;
				}
				base_epos = r;
			}
			else {
				if (base_spos >= 0) {
					break;
				}
			}
		}

		if (base_spos >= 0) {
			for (r = base_epos; r <= out_bound[c]; r++) {
				grad = ascMat.at<uchar>(r, c);
				if (grad >= low_peak_val) {
					low_peak_val = grad;
					low_peak_pos = r;
				}
			}

			// No secondary peak or the two peaks might be adhered. 
			if (low_peak_val < peak_thresh) {
				low_peak_pos = -1;
				low_peak_val = 0;

				// First peak value. 
				for (r = upp_peak_pos; r <= out_bound[c]; r++) {
					grad = ascMat.at<uchar>(r, c);
					if (grad >= upp_peak_val) {
						upp_peak_val = grad;
					}
					else {
						break;
					}
				}

				// To find the valley between peaks. 
				int mid_thresh = (int)(upp_peak_val * 0.65f);
				int mid_spos = -1;
				for (; r <= out_bound[c]; r++) {
					grad = ascMat.at<uchar>(r, c);
					if (grad < mid_thresh) {
						mid_spos = r;
						break;
					}
				}

				if (mid_spos >= 0) {
					// The following peak value. 
					low_peak_val = mid_thresh;
					for (; r <= out_bound[c]; r++) {
						grad = ascMat.at<uchar>(r, c);
						if (grad > low_peak_val) {
							low_peak_val = grad;
							low_peak_pos = r;
						}
					}
					if (low_peak_val < peak_thresh) {
						low_peak_pos = -1;
					}
				}
			}
			else {
				for (r = low_peak_pos; r <= out_bound[c]; r++) {
					grad = ascMat.at<uchar>(r, c);
					if (grad <= base_thresh) {
						base_epos = r;
						break;
					}
				}

				if (base_epos >= 0) {
					int sub_peak_val = 0;
					int sub_peak_pos = -1;

					for (r = base_epos; r <= out_bound[c]; r++) {
						grad = ascMat.at<uchar>(r, c);
						if (grad >= sub_peak_val) {
							sub_peak_val = grad;
							sub_peak_pos = r;
						}
					}

					if (sub_peak_pos > 0) {
						int dist1 = low_peak_pos - upp_peak_pos;
						int dist2 = sub_peak_pos - low_peak_pos;
						int thresh = max((int)(low_peak_val * PEAK_RATIO), peak_thresh);

						if (/*(dist1 < PEAK_DIST_MIN && dist2 > dist1) &&*/ sub_peak_val >= thresh) {
							low_peak_val = sub_peak_val;
							low_peak_pos = sub_peak_pos;
						}
					}
				}
			}
		}

		inn_edges[c] = upp_peak_pos;
		out_edges[c] = low_peak_pos;
	}

	if (!reclaimOutlinePoints()) {
		return false;
	}
	return true;
}

bool SemtSegm::Bordering::reclaimOutlinePoints()
{
	auto* pSegm = getImpl().pSegm;
	auto* pSample = pSegm->getSampling();

	Mat srcMat = pSample->sample()->getCvMatConst();
	Mat ascMat = pSample->ascent()->getCvMatConst();
	Mat avgMat = pSample->average()->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	auto& inn_edges = getInnerEdges();
	auto& out_edges = getOuterEdges();
	const auto& inn_bound = getInnerBound();
	const auto& out_bound = getOuterBound();

	inn_edges.resize(cols, -1);
	out_edges.resize(cols, -1);

	float mean = pSample->ascent()->getMean();
	float stdd = pSample->ascent()->getStddev();
	float mean2 = pSample->average()->getMean();
	float stdd2 = pSample->average()->getStddev();

	const int SURF_SIZE_MIN = 3;
	const int SURF_THRESH_MIN = 25;
	const int SURF_THRESH_MAX = 65;
	const int PEAK_DIST_MIN = 35;
	const float PEAK_RATIO = 0.55f;
	const int FILTER_SIZE = 13;
	const int CURVE_DEGREE = 1;

	int peak_thresh = (int)(mean * 1.0f + stdd * 2.0f);
	int base_thresh = (int)(mean * 0.5f + stdd * 0.5f);
	int subo_thresh = (int)(mean * 1.0f + stdd * 1.0f);

	int surf_thresh = (int)(mean2 * 1.0f + stdd2 * 0.5f);
	surf_thresh = min(max(surf_thresh, SURF_THRESH_MIN), SURF_THRESH_MAX);

	int base_thresh2 = (int)(mean2 * 1.0f + stdd2 * 0.0f);

	auto maxLocsAsc = pSample->ascent()->getColumMaxLocs();
	auto maxValsAsc = pSample->ascent()->getColumMaxVals();
	auto maxLocsAvg = pSample->average()->getColumMaxLocs();
	auto maxValsAvg = pSample->average()->getColumMaxVals();

	vector<int> outers;
	if (!interpolateOuterPoints(outers)) {
		return false;
	}

	int r, c;
	for (c = 0; c < cols; c++) {
		int reti_spos = -1;
		int reti_epos = -1;
		int base_spos = -1;
		int base_epos = -1;
		int base_dist = 0;

		int avg_peak_pos = maxLocsAvg[c];
		int avg_peak_val = maxValsAvg[c];

		int upp_peak_pos = -1;
		int low_peak_pos = -1;
		int upp_peak_val = 0;
		int low_peak_val = 0;
		int gval = 0;
		int grad = 0;
		int gcnt = 0;

		if (inn_edges[c] < 0) {
			continue;
		}

		for (r = inn_edges[c]; r <= outers[c]; r++) {
			gval = avgMat.at<uchar>(r, c);
			if (gval <= base_thresh2) {
				base_spos = r;
				base_epos = r;
				break;
			}
		}

		if (base_spos < 0) {
			continue;
		}

		for (r = base_spos; r <= out_bound[c]; r++) {
			gval = avgMat.at<uchar>(r, c);
			if (gval <= base_thresh2) {
				base_epos = r;
			}
			else {
				break;
			}
		}

		base_dist = base_epos - base_spos;
		if (base_dist < PEAK_DIST_MIN) {
			continue;
		}

		for (r = base_epos; r <= out_bound[c]; r++) {
			gval = avgMat.at<uchar>(r, c);
			if (gval <= base_thresh2) {
				base_epos = r;
			}
			else {
				break;
			}
		}

		for (r = base_epos; r <= out_bound[c]; r++) {
			gval = avgMat.at<uchar>(r, c);
			if (gval >= surf_thresh) {
				if (reti_spos < 0) {
					reti_spos = r;
				}
				if (++gcnt > SURF_SIZE_MIN) {
					break;
				}
			}
			else {
				reti_spos = -1;
				gcnt = 0;
			}
		}

		if (reti_spos < 0) {
			continue;
		}
		else {
			upp_peak_pos = reti_spos;
		}

		base_spos = -1;
		base_epos = -1;
		for (r = upp_peak_pos; r <= out_bound[c]; r++) {
			grad = ascMat.at<uchar>(r, c);
			if (grad <= base_thresh) {
				if (base_spos < 0) {
					base_spos = r;
				}
				base_epos = r;
			}
			else {
				if (base_spos >= 0) {
					break;
				}
			}
		}

		if (base_spos >= 0) {
			for (r = base_epos; r <= out_bound[c]; r++) {
				grad = ascMat.at<uchar>(r, c);
				if (grad >= low_peak_val) {
					low_peak_val = grad;
					low_peak_pos = r;
				}
			}

			if (low_peak_val < peak_thresh) {
				low_peak_pos = -1;
				low_peak_val = 0;
			}
		}

		inn_edges[c] = upp_peak_pos;
		out_edges[c] = low_peak_pos;
	}
	return true;
}

void SemtSegm::Bordering::computeOuterWeights(const std::vector<int>& edges)
{
	// const auto& out_edges = getOuterEdges();
	const auto& inn_edges = getInnerEdges();
	auto& weights = getOuterWeights();

	int size = (int)edges.size();
	const int NONE_MAX = (int)(size * 0.050f); // (int)(size * 0.035f);
	const int SEGM_MAX = (int)(size * 0.50f);
	const int SEGM_SPAN = (int)(9); // (int)(15);

	weights = vector<int>(size, 0);

	int value = 0;
	int none = 0;
	int x1;
	for (int c = 0; c < size; c++) {
		if (edges[c] >= 0) {
			if (value == 0) {
				x1 = max(c - NONE_MAX, 0);
				for (int k = c; k >= x1; k--) {
					if (edges[k] >= 0) {
						if (abs(edges[k] - edges[c]) < SEGM_SPAN) {
							value = weights[k];
						}
						for (int j = k; j < c; j++) {
							weights[j] = value;
						}
						break;
					}
				}
			}
			value += 1;
			weights[c] = value;
			none = 0;
		}
		else {
			value = 0;
			none += 1;
		}
	}

	value = 0;
	for (int c = size - 1; c >= 0; c--) {
		if (weights[c] > 0) {
			value = max(weights[c], value);
			weights[c] = min(value, SEGM_MAX);
		}
		else {
			value = 0;
		}
	}

	return;
}

void SemtSegm::Bordering::computeInnerWeights()
{
	auto& edges = getInnerEdges();
	auto& weights = getInnerWeights();

	int size = (int)edges.size();
	const int NONE_MAX = (int)(size * 0.035f); // (int)(size * 0.035f);
	const int SEGM_MAX = (int)(size * 0.35f);
	const int SEGM_SPAN = (int)(15);

	weights = vector<int>(size, 0);

	int value = 0;
	int none = 0;
	int x1;
	for (int c = 0; c < size; c++) {
		if (edges[c] >= 0) {
			if (value == 0) {
				x1 = max(c - NONE_MAX, 0);
				for (int k = c; k >= x1; k--) {
					if (edges[k] >= 0) {
						if (abs(edges[k] - edges[c]) < SEGM_SPAN) {
							value = weights[k];
						}
						for (int j = k; j < c; j++) {
							weights[j] = value;
						}
						break;
					}
				}
			}
			value += 1;
			weights[c] = value;
			none = 0;
		}
		else {
			value = 0;
			none += 1;
		}
	}

	value = 0;
	for (int c = size - 1; c >= 0; c--) {
		if (weights[c] > 0) {
			value = max(weights[c], value);
			weights[c] = min(value, SEGM_MAX);
		}
		else {
			value = 0;
		}
	}

	return;
}

void SemtSegm::Bordering::removeOuterOutliers(bool debris)
{
	auto& inn_edges = getInnerEdges();
	auto& out_edges = getOuterEdges();
	auto output = out_edges;

	int size = (int)out_edges.size();
	int cent = (int)(size * 0.50f);
	int spos = (int)(size * 0.25f);
	int epos = (int)(size * 0.75f);

	const int SEGM_MIN = (int)(size * 0.035f); // (int)(size * 0.050f);
	const int NONE_MAX = (int)(size * 0.090f); // (int)(size * 0.035f);
	const int Y_RANGE = (int)(9);
	const int SEGM_MIN2 = (int)(size * 0.100f); // (int)(size * 0.050f);
	const int NONE_MAX2 = (int)(size * 0.050f); // (int)(size * 0.035f);
	const int Y_RANGE2 = (int)(13);

	const int SEGM_CONN_SPAN = 5;
	const int SEGM_SIZE_MIN = 5;

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	int slen = 0, none = 0;
	int x_span1 = -1;
	int x_span2 = -1;
	int y_lend1 = -1;
	int y_lend2 = -1;

	int segmMin = SEGM_MIN;
	int noneMax = NONE_MAX;
	int yRange = Y_RANGE;

	int c;
	for (c = 0; c < size; c++) {
		float dist = ((float)abs(c - cent) / cent);
		segmMin = (int)(SEGM_MIN + size * (0.065f * (1.0f - dist)));
		noneMax = (int)(NONE_MAX - size * (0.065f * dist));
		yRange = (int)(Y_RANGE + 9.0f * dist);

		if (out_edges[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				// Initial point of line segment. 
				x1 = x2 = c;
				y1 = y2 = out_edges[c];
				slen = 1;
			}
			else {
				if ((abs(out_edges[c] - y2) <= yRange)) {
					// Subsequent point is within extenable range. 
					x2 = c;
					y2 = out_edges[c];
					slen++;
				}
				else {
					if (slen < segmMin) {
						x_span1 = max(x1 - noneMax, 0);
						x_span2 = min(x2 + noneMax, size - 1);
						y_lend1 = y1;
						y_lend2 = y2;

						for (int k = c + 1; k <= x_span2; k++) {
							if (out_edges[k] >= 0) {
								if (abs(out_edges[k] - y_lend2) <= yRange) {
									y_lend2 = out_edges[k];
									if (++slen >= segmMin) {
										break;
									}
								}
							}
							else if (inn_edges[k] >= 0) {
								if (abs(inn_edges[k] - y_lend2) <= yRange) {
									y_lend2 = inn_edges[k];
									if (++slen >= segmMin) {
										break;
									}
								}
							}
						}

						if (slen < segmMin) {
							for (int k = x1 - 1; k >= x_span1; k--) {
								if (out_edges[k] >= 0) {
									if (abs(out_edges[k] - y_lend1) <= yRange) {
										y_lend1 = out_edges[k];
										if (++slen >= segmMin) {
											break;
										}
									}
								}
								else if (inn_edges[k] >= 0) {
									if (abs(inn_edges[k] - y_lend1) <= yRange) {
										y_lend1 = inn_edges[k];
										if (++slen >= segmMin) {
											break;
										}
									}
								}
							}
						}
						if (slen < segmMin) {
							for (int k = x1; k <= x2; k++) {
								output[k] = -1;
							}
						}
					}

					// Starting the next line segment. 
					x1 = x2 = c;
					y1 = y2 = out_edges[c];
					slen = 1;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > noneMax) {
					if (slen < segmMin) {
						x_span1 = max(x1 - noneMax, 0);
						y_lend1 = y1;
						for (int k = x1 - 1; k >= x_span1; k--) {
							if (out_edges[k] >= 0) {
								if (abs(out_edges[k] - y_lend1) <= yRange) {
									y_lend1 = out_edges[k];
									if (++slen >= segmMin) {
										break;
									}
								}
							}
							else if (inn_edges[k] >= 0) {
								if (abs(inn_edges[k] - y_lend1) <= yRange) {
									y_lend1 = inn_edges[k];
									if (++slen >= segmMin) {
										break;
									}
								}
							}
						}
						if (slen < segmMin) {
							for (int k = x1; k <= x2; k++) {
								output[k] = -1;
							}
						}
					}
					slen = 0;
				}
				else {
					if (inn_edges[c] >= 0) {
						if (abs(inn_edges[c] - y2) <= yRange) {
							x2 = c;
							y2 = inn_edges[c];
							slen++;
						}
					}
				}
			}
		}
	}

	if (slen > 0) {
		if (slen < segmMin) {
			x_span1 = max(x1 - noneMax, 0);
			y_lend1 = y1;
			for (int k = x1 - 1; k >= x_span1; k--) {
				for (int k = x1 - 1; k >= x_span1; k--) {
					if (out_edges[k] >= 0) {
						if (abs(out_edges[k] - y_lend1) <= yRange) {
							y_lend1 = out_edges[k];
							if (++slen >= segmMin) {
								break;
							}
						}
					}
					else if (inn_edges[k] >= 0) {
						if (abs(inn_edges[k] - y_lend1) <= yRange) {
							y_lend1 = inn_edges[k];
							if (++slen >= segmMin) {
								break;
							}
						}
					}
				}
			}
			if (slen < segmMin) {
				for (int k = x1; k <= x2; k++) {
					output[k] = -1;
				}
			}
		}
	}

	if (debris) {
		// Remove remaining debris. 
		x1 = y1 = x2 = y2;
		slen = none = 0;
		for (int c = 0; c < size; c++) {
			if (output[c] >= 0) {
				none = 0;
				if (slen == 0) {
					x1 = x2 = c;
					y1 = output[c];
					y2 = output[c];
					slen = 1;
				}
				else {
					if (abs(output[c] - y2) < SEGM_CONN_SPAN) {
						x2 = c;
						y2 = output[c];
						slen += 1;
					}
					else {
						if (slen < SEGM_SIZE_MIN) {
							if (x1 >= spos && x2 <= epos) {
								for (int k = x1; k <= x2; k++) {
									output[k] = -1;
								}
							}
						}
						x1 = x2 = c;
						y1 = output[c];
						y2 = output[c];
						slen = 1;
					}
				}
			}
			else {
				if (slen > 0) {
					if (slen < SEGM_SIZE_MIN) {
						if (x1 >= spos && x2 <= epos) {
							for (int k = x1; k <= x2; k++) {
								output[k] = -1;
							}
						}
					}
					slen = 0;
				}
			}
		}
	}

	out_edges = output;
	return ;
}

void SemtSegm::Bordering::removeInnerOutliers()
{
	auto* inner = getImpl().pSegm->getLayerInner();

	auto& inn_edges = getInnerEdges();
	auto& out_edges = getOuterEdges();
	auto output = inn_edges;

	int size = (int)inn_edges.size();
	const int SEGM_MIN = (int)(size * 0.025f);		// 6
	const int NONE_MAX = (int)(size * 0.050f);		// 12 
	const int Y_RANGE = (int)(15);

	const int SEGM_CONN_SPAN = 5;
	const int SEGM_SIZE_MIN = 5;

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	int slen = 0, none = 0;
	int x_span1 = -1;
	int x_span2 = -1;
	int y_lend1 = -1;
	int y_lend2 = -1;

	int c;
	for (c = 0; c < size; c++) {
		if (inn_edges[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				// Initial point of line segment. 
				x1 = x2 = c;
				y1 = y2 = inn_edges[c];
				slen = 1;
			}
			else {
				if ((abs(inn_edges[c] - y2) <= Y_RANGE)) {
					// Subsequent point is within extenable range. 
					x2 = c;
					y2 = inn_edges[c];
					slen++;
				}
				else {
					if (slen < SEGM_MIN) {
						x_span1 = max(x1 - NONE_MAX, 0);
						x_span2 = min(x2 + NONE_MAX, size - 1);
						y_lend1 = y1;
						y_lend2 = y2;

						for (int k = c + 1; k <= x_span2; k++) {
							if (inn_edges[k] >= 0) {
								if (abs(inn_edges[k] - y_lend2) <= Y_RANGE) {
									y_lend2 = inn_edges[k];
									if (++slen >= SEGM_MIN) {
										break;
									}
									continue;
								}
							}
							if (out_edges[k] >= 0) {
								if (abs(out_edges[k] - y_lend2) <= Y_RANGE) {
									y_lend2 = out_edges[k];
									if (++slen >= SEGM_MIN) {
										break;
									}
								}
							}
						}

						if (slen < SEGM_MIN) {
							for (int k = x1 - 1; k >= x_span1; k--) {
								if (inn_edges[k] >= 0) {
									if (abs(inn_edges[k] - y_lend1) <= Y_RANGE) {
										y_lend1 = inn_edges[k];
										if (++slen >= SEGM_MIN) {
											break;
										}
										continue;
									}
								}
								if (out_edges[k] >= 0) {
									if (abs(out_edges[k] - y_lend1) <= Y_RANGE) {
										y_lend1 = out_edges[k];
										if (++slen >= SEGM_MIN) {
											break;
										}
									}
								}
							}
						}
						if (slen < SEGM_MIN) {
							for (int k = x1; k <= x2; k++) {
								output[k] = -1;
							}
						}
					}

					// Starting the next line segment. 
					x1 = x2 = c;
					y1 = y2 = inn_edges[c];
					slen = 1;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > NONE_MAX) {
					if (slen < SEGM_MIN) {
						x_span1 = max(x1 - NONE_MAX, 0);
						y_lend1 = y1;
						for (int k = x1 - 1; k >= x_span1; k--) {
							if (inn_edges[k] >= 0) {
								if (abs(inn_edges[k] - y_lend1) <= Y_RANGE) {
									y_lend1 = inn_edges[k];
									if (++slen >= SEGM_MIN) {
										break;
									}
									continue;
								}
							}
							if (out_edges[k] >= 0) {
								if (abs(out_edges[k] - y_lend1) <= Y_RANGE) {
									y_lend1 = out_edges[k];
									if (++slen >= SEGM_MIN) {
										break;
									}
								}
							}
						}
						if (slen < SEGM_MIN) {
							for (int k = x1; k <= x2; k++) {
								output[k] = -1;
							}
						}
					}
					slen = 0;
				}
			}
		}
	}

	if (slen > 0) {
		if (slen < SEGM_MIN) {
			x_span1 = max(x1 - NONE_MAX, 0);
			y_lend1 = y1;
			for (int k = x1 - 1; k >= x_span1; k--) {
				for (int k = x1 - 1; k >= x_span1; k--) {
					if (inn_edges[k] >= 0) {
						if (abs(inn_edges[k] - y_lend1) <= Y_RANGE) {
							y_lend1 = inn_edges[k];
							if (++slen >= SEGM_MIN) {
								break;
							}
							continue;
						}
					}
					if (out_edges[k] >= 0) {
						if (abs(out_edges[k] - y_lend1) <= Y_RANGE) {
							y_lend1 = out_edges[k];
							if (++slen >= SEGM_MIN) {
								break;
							}
						}
					}
				}
			}
			if (slen < SEGM_MIN) {
				for (int k = x1; k <= x2; k++) {
					output[k] = -1;
				}
			}
		}
	}

	/*
	if (true) {
		// Remove remaining debris. 
		x1 = y1 = x2 = y2;
		slen = none = 0;
		for (int c = 0; c < size; c++) {
			if (output[c] >= 0) {
				none = 0;
				if (slen == 0) {
					x1 = x2 = c;
					y1 = output[c];
					y2 = output[c];
					slen = 1;
				}
				else {
					if (abs(output[c] - y2) < SEGM_CONN_SPAN) {
						x2 = c;
						y2 = output[c];
						slen += 1;
					}
					else {
						if (slen < SEGM_SIZE_MIN) {
							for (int k = x1; k <= x2; k++) {
								output[k] = -1;
							}
						}
						x1 = x2 = c;
						y1 = output[c];
						y2 = output[c];
						slen = 1;
					}
				}
			}
			else {
				if (slen > 0) {
					if (slen < SEGM_SIZE_MIN) {
						for (int k = x1; k <= x2; k++) {
							output[k] = -1;
						}
					}
					slen = 0;
				}
			}
		}
	}
	*/

	inn_edges = output;
	return;
}

bool SemtSegm::Bordering::estimateInnerBorder()
{
	auto* pSegm = getImpl().pSegm;
	Mat srcMat = sampling()->sample()->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	auto& inn_edges = getInnerEdges();
	// auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	// auto& out_border = getOuterBorder();

	auto& weights = getInnerWeights();
	auto output = vector<int>(cols, 0);
	auto inners = vector<int>(cols, 0);

	const int FILTER_SIZE = 7;
	const int CURVE_DEGREE = 1;

	/*
	const int BORDER_SPAN = 3;

	for (int c = 0; c < cols; c++) {
		if (inn_edges[c] >= 0) {
			int d1 = inn_edges[c] - inn_layer[c];
			int d2 = out_border[c] - inn_edges[c];
			if (d2 >= 0 && d2 < d1) {
				inn_edges[c] = -1;
			}
		}
	}
	*/

	computeInnerWeights();

	if (!interpolateByWeightFitt(inn_edges, weights, output, true)) {
		return false;
	}

	inners = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE, CURVE_DEGREE);
	std::transform(begin(inners), end(inners), begin(inners), [=](int elem) { return min(max(elem, 0), rows - 1); });

	getInnerBorder() = inners;
	return true;
}

bool SemtSegm::Bordering::estimateOuterBorder()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->ascent();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();

	auto& out_edges = getOuterEdges();
	auto& weights = getOuterWeights();
	auto output = vector<int>(cols, 0);
	auto outers = vector<int>(cols, 0);

	const int FILTER_SIZE1 = 25; // 25;
	const int FILTER_SIZE2 = 45;
	const int CURVE_DEGREE = 1;
	const int LAYERS_DIST_MIN = 15;
	const int POINTS_OFFSET_MAX = 13;
	const int POINTS_OFFSET_MIN = -25;
	const int SEGM_END_OFFSET = -5;
	const int SEGM_END_DIST_MAX = 35;
	const int SEGM_CURVE_DIFF = 35;

	inflateOuterPointsInSideEdges();

	// Remove edge points closer to the inner layer. 
	for (int i = 0; i < 10; i++) {
		computeOuterWeights(out_edges);

		if (!interpolateByWeightFitt(out_edges, weights, output, true)) {
			return false;
		}

		outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE1, CURVE_DEGREE);
		std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });

		int count = 0;
		for (int c = 0; c < cols; c++) {
			if (out_edges[c] >= 0) {
				int offs = outers[c] - inn_layer[c];
				int d1 = out_edges[c] - inn_layer[c];
				int d2 = outers[c] - out_edges[c];
				if (offs > LAYERS_DIST_MIN && d2 > d1) {
					out_edges[c] = -1;
					count++;
				}
			}

		}

		if (count == 0) {
			break;
		}
	}
	

	// Remove the edge points above the estimated curve. 
	for (int i = 0; i < 10; i++) {
		computeOuterWeights(out_edges);

		if (!interpolateByWeightFitt(out_edges, weights, output, true)) {
			return false;
		}

		outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE1, CURVE_DEGREE);
		std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });

		int count = 0;
		for (int c = 0; c < cols; c++) {
			if (out_edges[c] >= 0) {
				int diff = outers[c] - out_edges[c];
				if (diff > POINTS_OFFSET_MAX) {
					int t0 = diff;
					int t1 = 0, d1 = 0;
					int t2 = 0, d2 = 0;
					int x1 = 0, x2 = 0;
					int last_y = out_edges[c];
					int last_x = c;

					// Left end of segment over curve. 
					for (int j = c - 1, nots = 0; j >= 0; j--) {
						if (out_edges[j] >= 0) {
							int offs = outers[j] - out_edges[j];
							if (offs < SEGM_END_OFFSET) {
								t1 = (out_edges[j] - last_y);
								d1 = abs(j - last_x);
								x1 = last_x;
								break;
							}
							else {
								// Points segment over curve.
								if (offs > 0) {
									last_y = out_edges[j];
									last_x = j;
								}
								if (offs <= 0) {
									if (++nots > SEGM_END_DIST_MAX) {
										break;
									}
								}
							}
						}
					}

					// Right end of segment over curve. 
					for (int j = c + 1, nots = 0; j < cols; j++) {
						if (out_edges[j] >= 0) {
							int offs = outers[j] - out_edges[j];
							if (offs < SEGM_END_OFFSET) {
								t2 = (out_edges[j] - last_y);
								d2 = abs(j - last_x);
								x2 = last_x;
								break;
							}
							else {
								// Points segment over curve.
								if (offs > 0) {
									last_y = out_edges[j];
									last_x = j;
								}
								if (offs <= 0) {
									if (++nots > SEGM_END_DIST_MAX) {
										break;
									}
								}
							}
						}
					}

					if (t1 > 0 && t2 > 0) {
						int diff = 0;
						if (t1 > 0 && t2 > 0) {
							diff = (d1 < d2 ? t1 : t2);
						}
						else {
							diff = (t1 > 0 ? t1 : t2);
						}

						if (diff > SEGM_CURVE_DIFF) {
							for (int k = x1; k <= x2; k++) {
								out_edges[k] = -1;
							}
							count++;
						}
					}
				}
			}
		}

		if (count == 0) {
			break;
		}
	}

	for (int i = 0; i < 10; i++) {
		auto data = out_edges;

		computeOuterWeights(out_edges);

		if (!interpolateByWeightFitt(out_edges, weights, output, true)) {
			return false;
		}

		outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE2, CURVE_DEGREE);
		std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });

		// Exclude the edge points underneath estimated curve. 
		for (int c = 0; c < cols; c++) {
			if (out_edges[c] >= 0) {
				if (inn_layer[c] >= outers[c]) {
					out_edges[c] = -1;
				}
				int offs = outers[c] - out_edges[c];
				if (offs < POINTS_OFFSET_MIN) {
					out_edges[c] = -1;
				}
			}
		}

		// Make the estimated curve from the filtered edge points. 
		computeOuterWeights(out_edges);

		if (!interpolateByLinearFitt(out_edges, output, true)) {
			return false;
		}

		outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE1, CURVE_DEGREE);
		std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });

		out_edges = data;

		// Not loop. 
		break;
	}

	getOuterBorder() = outers;
	return true;
}

bool SemtSegm::Bordering::reclaimOuterBorder()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->ascent();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOuter()->getOptimalPath();

	auto& out_edges = getOuterEdges();
	auto& weights = getOuterWeights();
	auto output = vector<int>(cols, 0);
	auto outers = vector<int>(cols, 0);

	const int FILTER_SIZE = 25;
	const int CURVE_DEGREE = 1;
	const int LAYER_DIST_MIN = 15;
	const int EDGE_PEAK_SPAN = 2;
	const int EDGE_DIST_MAX = 5;
	const int EDGE_LAYER_DIST_MAX = 5;
	const int EDGE_OUTER_DIST_MAX = 13;
	const int EDGE_INNER_DIST_MIN = 13;

	// To extend outer edges along the estimated curve. 
	int thresh = (int)(pImage->getMean() + pImage->getStddev()*2.0f);
	
	for (int i = 0; i < 1; i++) {
		int count = 0;
		for (int c = 0; c < cols; c++) {
			// Replace with outer edge points along the boundary line.
			int dist = out_layer[c] - inn_layer[c];
			if (dist > LAYER_DIST_MIN) {
				int y1 = max(out_layer[c] - EDGE_PEAK_SPAN, 0);
				int y2 = min(out_layer[c] + EDGE_PEAK_SPAN, rows - 1);
				int grad = 0, gmax = 0, gpos = 0;
				for (int r = y1; r <= y2; r++) {
					grad = srcMat.at<uchar>(r, c);
					if (grad >= gmax) {
						gmax = grad;
						gpos = r;
					}
				}
				if (gmax > thresh) {
					out_edges[c] = gpos;
				}
			}

			// Excludes the points too close to the inner boundary. 
			if (out_edges[c] >= 0) {
				int dist1 = abs(out_edges[c] - out_layer[c]);
				int dist2 = abs(out_edges[c] - inn_layer[c]);
				if (dist1 > EDGE_LAYER_DIST_MAX || dist2 < EDGE_INNER_DIST_MIN) {
					out_edges[c] = -1;
					count++;
				}
			}

			/*
			if (out_edges[c] < 0) {
				if ((out_layer[c] - inn_layer[c]) > LAYER_DIST_MIN) {
					int y1 = max(out_layer[c] - EDGE_PEAK_SPAN, 0);
					int y2 = min(out_layer[c] + EDGE_PEAK_SPAN, rows-1);
					int grad = 0, gmax = 0, gpos = 0;
					for (int r = y1; r <= y2; r++) {
						grad = srcMat.at<uchar>(r, c);
						if (grad >= gmax) {
							gmax = grad;
							gpos = r;
						}
					}
					if (gmax > thresh) {
						out_edges[c] = gpos;
					}
				}
			}
			else {
				if (out_edges[c] >= 0) {
					if (abs(out_edges[c] - out_layer[c]) > EDGE_DIST_MAX) {
						out_edges[c] = -1;
						count++;
					}
				}
			}
			*/
		}

		removeOuterOutliers(true);

		auto out_points = out_edges;

		if (isReversedRegion()) {
			int rx1 = getImpl().reversedX1;
			int rx2 = getImpl().reversedX2;

			if (rx1 >= 0) {
				for (int c = 0; c <= rx1; c++) {
					out_points[c] = out_layer[c];
				}
			}
			if (rx2 >= 0) {
				for (int c = rx2; c < cols; c++) {
					out_points[c] = out_layer[c];
				}
			}
		}

		// Rebuild the outer border line to filter the outliers. 
		computeOuterWeights(out_points);

		if (!interpolateByWeightFitt(out_points, weights, output, true)) {
			return false;
		}

		outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE, CURVE_DEGREE);
		std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });

		if (removeOuterPointsInOpticDisc()) {
			computeOuterWeights(out_points);

			if (!interpolateByWeightFitt(out_points, weights, output, true)) {
				return false;
			}

			outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE, CURVE_DEGREE);
			std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });
		}

		// Remove outer points too further from curve. 
		count = 0;
		for (int c = 0; c < cols; c++) {
			if (out_edges[c] >= 0) {
				int dist1 = abs(out_edges[c] - outers[c]);
				int dist2 = abs(out_edges[c] - inn_layer[c]);
				if (dist1 > EDGE_OUTER_DIST_MAX || dist2 < EDGE_INNER_DIST_MIN) {
					out_edges[c] = -1;
					count++;
				}
			}
		}

		// Extend outer points with the nearest peaks. 
		for (int t = 0; t < 99; t++) {
			count = 0;
			for (int c = 1; c < cols - 1; c++) {
				if (out_edges[c] >= 0) {
					if (out_edges[c - 1] < 0) {
						int grad = 0, gmax = 0, gpos = 0;
						int x1 = c - 1;
						int y1 = max(out_edges[c] - EDGE_PEAK_SPAN, inn_layer[x1] + EDGE_DIST_MAX);
						int y2 = min(out_edges[c] + EDGE_PEAK_SPAN, rows - 1);

						for (int r = y1; r <= y2; r++) {
							grad = srcMat.at<uchar>(r, x1);
							if (grad >= gmax) {
								gmax = grad;
								gpos = r;
							}
						}
						if (gmax > thresh) {
							out_edges[x1] = gpos;
							count++;
						}
					}

					if (out_edges[c + 1] < 0) {
						int grad = 0, gmax = 0, gpos = 0;
						int x2 = c + 1;
						int y1 = max(out_edges[c] - EDGE_PEAK_SPAN, inn_layer[x2] + EDGE_DIST_MAX);
						int y2 = min(out_edges[c] + EDGE_PEAK_SPAN, rows - 1);

						for (int r = y1; r <= y2; r++) {
							grad = srcMat.at<uchar>(r, x2);
							if (grad >= gmax) {
								gmax = grad;
								gpos = r;
							}
						}
						if (gmax > thresh) {
							out_edges[x2] = gpos;
							count++;
						}
					}
				}
			}
			if (count <= 0) {
				break;
			}
		}

		// Remove outer points too further from curve. 
		count = 0;
		for (int c = 0; c < cols; c++) {
			if (out_edges[c] >= 0) {
				int dist1 = abs(out_edges[c] - outers[c]);
				int dist2 = abs(out_edges[c] - inn_layer[c]);
				if (dist1 > EDGE_OUTER_DIST_MAX || dist2 < EDGE_INNER_DIST_MIN) {
					out_edges[c] = -1;
					count++;
				}
			}
		}

		// Rebuild the outer border line without weighting. 
		removeOuterOutliers(true);

		if (!interpolateByLinearFitt(out_edges, output, true)) {
			return false;
		}

		outers = CppUtil::SgFilter::smoothInts(output, FILTER_SIZE, CURVE_DEGREE);
		std::transform(begin(outers), end(outers), begin(outers), [=](int elem) { return min(max(elem, 0), rows - 1); });
	}

	getOuterBorder() = outers;
	return true;
}

void SemtSegm::Bordering::inflateOuterPointsInSideEdges()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->ascent();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& inn_edges = getInnerEdges();
	auto& out_edges = getOuterEdges();

	const int SEGM_SIZE_MIN = (int)(cols * 0.075f);

	int spos = (int)(cols * 0.25f);
	int epos = (int)(cols * 0.75f);
	int spos2 = (int)(cols * 0.35f);
	int epos2 = (int)(cols * 0.65f);

	/*
	if (pSegm->getImageIndex() == 55) {
		int i = 0; 
		i = 3;
	}
	*/

	// Count outer points on left side. 
	int size1 = 0;
	for (int c = 0; c <= spos; c++) {
		if (out_edges[c] >= 0) {
			size1 += 1;
		}
	}

	// If it's too small, fill the outer points with inner points. 
	if (size1 < SEGM_SIZE_MIN) {
		for (int c = 0; c <= spos2; c++) {
			if (out_edges[c] < 0) {
				if (inn_edges[c] >= 0) {
					out_edges[c] = inn_edges[c];
				}
			}
			else {
				if (c > spos) {
					break;
				}
			}
		}
	}

	// Count outer points on right side. 
	int size2 = 0;
	for (int c = cols-1; c >= epos; c--) {
		if (out_edges[c] >= 0) {
			size2 += 1;
		}
	}

	// If it's too small, fill the outer points with inner points. 
	if (size2 < SEGM_SIZE_MIN) {
		for (int c = cols-1; c >= epos2; c--) {
			if (out_edges[c] < 0) {
				if (inn_edges[c] >= 0) {
					out_edges[c] = inn_edges[c];
				}
			}
			else {
				if (c < epos) {
					break;
				}
			}
		}
	}
	return;
}

bool SemtSegm::Bordering::removeOuterPointsInOpticDisc()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->ascent();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& out_layer = pSegm->getLayerOuter()->getOptimalPath();
	const auto& out_border = getOuterBorder();
	auto& out_edges = getOuterEdges();

	const int SPROUT_DEPTH_MIN = 3;
	const int SPROUT_RANGE_MAX = 75;
	const int OUTER_DIFF_MIN = 1;

	auto output = vector<int>(cols, 0);
	int spos = (int)(cols * 0.25f);
	int epos = (int)(cols * 0.75f);

	std::transform(out_border.cbegin(), out_border.cend(), out_layer.cbegin(), output.begin(), [=](int elem1, int elem2) { return elem1 - elem2; });
	auto max_pos = max_element(output.cbegin() + spos, output.cbegin() + epos);
	auto max_val = *max_pos;
	auto max_idx = (int)(max_pos - output.cbegin());

	if (max_val < SPROUT_DEPTH_MIN) {
		return false;
	}

	int over_x1 = -1;
	int over_x2 = -1;

	for (int c = max_idx; c >= 0; c--) {
		int offs = out_border[c] - out_layer[c];
		if (offs <= 0) {
			for (int j = c; j >= 0; j--) {
				offs = out_border[j] - out_layer[j];
				if (offs < 0) {
					over_x1 = c;
					break;
				}
				if (offs > 0) {
					break;
				}
			}
			break;
		}
	}

	for (int c = max_idx; c < cols; c++) {
		int offs = out_border[c] - out_layer[c];
		if (offs <= 0) {
			for (int j = c; j < cols; j++) {
				offs = out_border[j] - out_layer[j];
				if (offs < 0) {
					over_x2 = c;
					break;
				}
				if (offs > 0) {
					break;
				}
			}
			break;
		}
	}

	if (over_x1 < 0 || over_x2 < 0 || (over_x2 - over_x1) > SPROUT_RANGE_MAX) {
		return false;
	}

	int over_cx = (over_x2 + over_x1) / 2;
	bool is_gap = false;
	int count = 0;

	for (int c = over_x1; c <= over_cx; c++) {
		if (out_edges[c] < 0) {
			is_gap = true;
		}
		else {
			if (is_gap) {
				int offs = out_border[c] - out_edges[c];
				if (offs > OUTER_DIFF_MIN) {
					out_edges[c] = -1;
					count += 1;
				}
			}
		}
	}

	is_gap = false;
	for (int c = over_x2; c >= over_cx; c--) {
		if (out_edges[c] < 0) {
			is_gap = true;
		}
		else {
			if (is_gap) {
				int offs = out_border[c] - out_edges[c];
				if (offs > OUTER_DIFF_MIN) {
					out_edges[c] = -1;
					count += 1;
				}
			}
		}
	}

	return (count >= 1);
}

bool SemtSegm::Bordering::detectReversedRegion()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->ascent();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOuter()->getOptimalPath();
	const int TOP_MARGIN = 15;

	int top_x1 = -1;
	int top_x2 = -1;
	bool is_top = any_of(inn_layer.cbegin(), inn_layer.cend(), [=](int elem) { return elem <= 0; });

	if (is_top) {
		for (int c = 0; c <= cols / 2; c++) {
			if (inn_layer[c] <= TOP_MARGIN) {
				top_x1 = c;
			}
		}
		for (int c = cols - 1; c >= cols / 2; c--) {
			if (inn_layer[c] <= TOP_MARGIN) {
				top_x2 = c;
			}
		}
	}

	getImpl().reversedX1 = top_x1;
	getImpl().reversedX2 = top_x2;
	return true;
}


bool SemtSegm::Bordering::removeOuterPointsAtDiscBase()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->average();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	const auto& out_layer = getOuterBorder();
	auto& out_edges = getOuterEdges();

	const int IMAGE_W = 256;
	const float RANGE_X = 4.5f;
	const int DISC_WIND_SIZE = 64; // 48;
	const int DISC_DEPTH = 45;
	const int DISC_DEPTH_MIN = (DISC_DEPTH / 2);
	const float DISC_SINK_TOP = 0.75f;
	const float DISC_SINK_BASE = 0.45f;
	const float DISC_SINK_RATIO = 0.45f;

	const float rangeX = pSegm->getImageRangeX();
	const float ratioX = ((float)IMAGE_W / cols) * (RANGE_X / rangeX);
	const int windSize = (int)(DISC_WIND_SIZE * ratioX);
	const int windHalf = windSize / 2;

	int index = pSegm->getImageIndex();
	int xpos1 = 0;
	int xpos2 = cols - 1;

	if (isReversedRegion()) {
		xpos1 = (getImpl().reversedX1 >= 0 ? getImpl().reversedX1 : xpos1);
		xpos2 = (getImpl().reversedX2 >= 0 ? getImpl().reversedX2 : xpos2);
	}

	int peri_x1 = max(xpos1, (int)(cols * 0.20f));
	int peri_x2 = min(xpos2, (int)(cols * 0.80f));

	auto max_pos = (int)(max_element(inn_layer.cbegin(), inn_layer.cend()) - inn_layer.cbegin());
	if (max_pos < peri_x1 || max_pos > peri_x2) {
		return false;
	}

	int wind_x1 = max(max_pos - windHalf, 0);
	int wind_x2 = min(max_pos + windHalf, cols-1);
	int depth1 = 0;
	int depth2 = 0;

	for (int i = max_pos; i >= wind_x1; i--) {
		depth1 = max(depth1, out_layer[i] - inn_layer[i]);
	}
	for (int i = max_pos; i <= wind_x2; i++) {
		depth2 = max(depth2, out_layer[i] - inn_layer[i]);
	}

	if (depth1 < DISC_DEPTH && depth2 < DISC_DEPTH) {
		return false;
	}
	if (depth1 < DISC_DEPTH_MIN || depth2 < DISC_DEPTH_MIN) {
		return false;
	}

	int dbase = max((out_layer[max_pos] - inn_layer[max_pos]), 0);
	float dsink = (float)dbase / (float)min(depth1, depth2);
	if (dsink > DISC_SINK_RATIO) {
		return false;
	}

	int count = 0;
	int top_1 = (int)(depth1 * DISC_SINK_TOP);
	int base_1 = (int)(depth1 * DISC_SINK_BASE);
	for (int i = max_pos; i >= wind_x1; i--) {
		int deep = out_layer[i] - inn_layer[i];
		if (deep < base_1) {
			if (out_edges[i] >= 0) {
				out_edges[i] = -1;
				count++;
			}
		}
		if (deep > top_1) {
			break;
		}
	}

	int top_2 = (int)(depth2 * DISC_SINK_TOP);
	int base_2 = (int)(depth2 * DISC_SINK_BASE);
	for (int i = max_pos; i <= wind_x2; i++) {
		int deep = out_layer[i] - inn_layer[i];
		if (deep < base_2) {
			if (out_edges[i] >= 0) {
				out_edges[i] = -1;
				count++;
			}
		}
		if (deep > top_2) {
			break;
		}
	}

	if (count > 0) {
		LogD() << "Disc base points " << count << " removed, index: " << index;
	}
	return true;
}

bool SemtSegm::Bordering::detectOpticDiscRegion()
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->average();
	Mat srcMat = pImage->getCvMatConst();

	int cols = srcMat.cols;
	int rows = srcMat.rows;

	const auto& inn_layer = pSegm->getLayerInner()->getOptimalPath();
	// const auto& out_layer = pSegm->getLayerOuter()->getOptimalPath();
	const auto& out_layer = getOuterBorder();
	auto& out_edges = getOuterEdges();
	auto& inn_edges = getInnerEdges();

	const int IMAGE_W = 256;
	const float RANGE_X = 4.5f;
	const int DISC_WIND_SIZE = 48;
	const int DISC_EDGE_SIZE = 11;

	const float HOLE_DIMEN_MIN = 1.0f;
	const float HOLE_RATIO_MIN = 0.80f;
	const float PEAK_RATIO_MIN = 0.35f; // 0.50f;
	const int INNER_PASS_MIN = 3;
	const int TOP_MARGIN = 15;

	const float rangeX = pSegm->getImageRangeX();
	const float ratioX = ((float)IMAGE_W / cols) * (RANGE_X / rangeX);
	const float xPixelMM = rangeX / cols;
	const float yPixelMM = (float)GlobalSettings::getRetinaScanAxialResolution() / 1000.0f;

	const int windSize = (int)(DISC_WIND_SIZE * ratioX);
	const int windHalf = windSize / 2;
	const int edgeSize = (int)(DISC_EDGE_SIZE * ratioX);

	int hole_x1 = -1;
	int hole_x2 = -1;
	int hole_cx = -1;

	int xpos1 = 0;
	int xpos2 = cols - 1;

	int index = pSegm->getImageIndex();
	if (index == 41) {
		int i = 0; 
		i = 23;
	}

	if (isReversedRegion()) {
		xpos1 = (getImpl().reversedX1 >= 0 ? getImpl().reversedX1 : xpos1);
		xpos2 = (getImpl().reversedX2 >= 0 ? getImpl().reversedX2 : xpos2);
	}

	removeOuterPointsAtDiscBase();

	// LogD() << "Image: " << pSegm->getImageIndex() << ", xpos: " << xpos1 << ", " << xpos2;
	// inn_edges = vector<int>(cols, -1);

	// Inner layer passed over outer layer in optic disc region. 
	auto is_sunk = false;
	for (int i = xpos1; i <= xpos2; i++) {
		int deep = (inn_layer[i] - out_layer[i]);
		if (deep > INNER_PASS_MIN) {
			hole_x1 = (hole_x1 < 0 ? i : hole_x1);
			hole_x2 = (hole_x1 >= 0 ? i : -1);
			is_sunk = true;
		}
	}

	if (is_sunk) {
		hole_cx = hole_x1 + (hole_x2 - hole_x1) / 2;
		hole_x1 = hole_cx - windHalf;
		hole_x2 = hole_cx + windSize;
		hole_x1 = max(hole_x1, xpos1);
		hole_x2 = min(hole_x2, xpos2);

		auto max_pos = (int)(max_element(inn_layer.cbegin(), inn_layer.cend()) - inn_layer.cbegin());
		auto max_val = inn_layer[max_pos];

		if (hole_x1 > max_pos || hole_x2 < max_pos) {
			hole_x1 = hole_x2 = -1;
			is_sunk = false;
		}
		else {
			getImpl().discBaseX = max_pos;
			getImpl().discBaseY = max_val;
			getImpl().isDiscOpening = true;
		}
	}
	
	if (hole_x1 < 0 || hole_x2 < 0) {
		int x1 = windHalf + 1;
		int x2 = cols - x1;
		auto holes = vector<float>(cols, 0.0f);
		auto dists = vector<float>(cols, 0.0f);

		int end_x1 = -1;
		int end_x2 = -1;
		int end_y1 = -1;
		int end_y2 = -1;
		for (int i = x1; i <= x2; i++) {
			int count = 0;
			for (int k = -windHalf; k <= windHalf; k++) {
				int pos = i + k;
				if (out_edges[pos] >= 0) {
					if (count == 0) {
						end_x1 = pos;
						end_y1 = out_edges[pos];
					}
					else {
						end_x2 = pos;
						end_y2 = out_edges[pos];
					}
					count++;
				}
			}
			holes[i] = (float)(windSize - count) / windSize;

			/*
			if (end_x1 >= 0 && end_x2 >= 0) {
				float dx = (abs(end_x2 - end_x1) + 1.0f) * xPixelMM;
				float dy = (abs(end_y1 - end_y2) + 1.0f) * yPixelMM;
				float dlen = sqrt((dx*dx) + (dy*dy));
				dists[i] = dlen;
			}
			*/
		}

		auto max_pos = (int)(max_element(cbegin(holes), cend(holes)) - cbegin(holes));
		auto max_val = (float)(holes[max_pos]);
		auto max_pos2 = (int)(max_element(cbegin(dists), cend(dists)) - cbegin(dists));
		auto max_val2 = (float)(dists[max_pos2]);

		if (max_val > HOLE_RATIO_MIN || max_val2 > HOLE_DIMEN_MIN) {
			hole_x1 = max_pos - windHalf;
			hole_x2 = max_pos + windHalf;
		}
	}

	if (hole_x1 < 0 || hole_x2 < 0) {
		return false;
	}

	hole_x1 = min(max(hole_x1, xpos1), xpos2);
	hole_x2 = min(max(hole_x2, xpos1), xpos2);
	hole_cx = hole_x1 + (hole_x2 - hole_x1) / 2;

	int count = 0;
	for (int c = hole_cx; c >= 0; c--) {
		if (out_edges[c] >= 0) {
			if (++count > edgeSize) {
				break;
			}
		}
		else {
			count = 0;
			hole_x1 = c;
		}
	}
	for (int c = hole_cx; c < cols; c++) {
		if (out_edges[c] >= 0) {
			if (++count > edgeSize) {
				break;
			}
		}
		else {
			count = 0;
			hole_x2 = c;
		}
	}

	if (hole_x1 >= 0 && hole_x2 >= 0) {
		auto min_out = *min_element(out_layer.cbegin(), out_layer.cend());
		auto max_out = *max_element(out_layer.cbegin(), out_layer.cend());
		auto max_y = max_out - max((int)((max_out - min_out)*0.35f), 45);

		float ratio = 0.0f;
		bool is_disc = any_of(out_layer.cbegin() + hole_x1, out_layer.cbegin() + hole_x2, [=](int elem) { return elem >= max_y; });

		if (hole_x1 < xpos1 || hole_x2 > xpos2) {
			is_disc = false;
		}
		else {
			/*
			if (!is_sunk) {
				auto max_pos = (int)(max_element(inn_layer.cbegin(), inn_layer.cend()) - inn_layer.cbegin());
				auto max_val = inn_layer[max_pos];
				auto max_pos2 = (int)(max_element(out_layer.cbegin(), out_layer.cend()) - out_layer.cbegin());
				auto max_val2 = out_layer[max_pos2];

				int hx1 = max((hole_x1 - 15), 0);
				int hx2 = max((hole_x2 + 15), cols - 1);

				if ((hx1 <= max_pos && max_pos <= hx2) &&
				    (hx1 <= max_pos2 && max_pos2 <= hx2)) {
					is_sunk = true;
				}
			}
			*/

			if (!is_sunk) {
				int thresh = (int)(pImage->getMean() * 1.0f + pImage->getStddev() * 1.0f);
				int count = 0;

				auto max_vals = pImage->getColumMaxVals();
				for (int c = hole_x1; c <= hole_x2; c++) {
					if (max_vals[c] > thresh) {
						count++;
					}
				}
				ratio = count / (float)(hole_x2 - hole_x1 + 1);
				if (ratio < PEAK_RATIO_MIN) {
					is_disc = false;
				}
			}

			if (is_disc) {
				for (int i = hole_x1; i <= hole_x2; i++) {
					if (inn_layer[i] <= TOP_MARGIN) {
						is_disc = false;
						break;
					}
				}
			}
		}

		// LogD() << "Image: " << pSegm->getImageIndex() << ", hole: " << hole_x1 << ", " << hole_x2 << ", ratio: " << ratio;

		if (is_disc) {
			getImpl().discX1 = hole_x1;
			getImpl().discX2 = hole_x2;

			// Clear all edge points within optic disc hole. 
			for (int i = hole_x1; i <= hole_x2; i++) {
				out_edges[i] = -1;
				// inn_edges[i] = 96;
			}
		}
	}
	return true;
}


bool SemtSegm::Bordering::inflateOpticDiscRegion()
{
	auto* pSegm = getImpl().pSegm;
	auto* pSample = pSegm->getSampling();

	if (isOpticDiscRegion()) {
		float ratio = 1.0f / pSample->getSampleWidthRatio();
		int disc_x1 = getImpl().discX1;
		int disc_x2 = getImpl().discX2;
		int hole_x1 = getImpl().holeX1;
		int hole_x2 = getImpl().holeX2;

		getImpl().discX1 = (int)(disc_x1 * ratio);
		getImpl().discX2 = (int)(disc_x2 * ratio);
		getImpl().holeX1 = (int)(hole_x1 * ratio);
		getImpl().holeX2 = (int)(hole_x2 * ratio);
	}
	return true;
}

bool SemtSegm::Bordering::buildInnerBoundary()
{
	auto* pSegm = getImpl().pSegm;
	auto* inner = getImpl().pSegm->getLayerInner();
	auto* outer = getImpl().pSegm->getLayerOuter();

	removeInnerOutliers();

	if (!inner->buildOutlinePath()) {
		return false;
	}

	if (!inner->buildBoundaryPath()) {
		return false;
	}
	return true;
}

bool SemtSegm::Bordering::buildOuterBoundary()
{
	auto* pSegm = getImpl().pSegm;
	auto* inner = getImpl().pSegm->getLayerInner();
	auto* outer = getImpl().pSegm->getLayerOuter();

	removeOuterOutliers();
	estimateOuterBorder();

	if (!outer->buildOutlinePath()) {
		return false;
	}

	getInnerBorder() = outer->getOptimalPath();

	if (!outer->buildBoundaryPath()) {
		return false;
	}

	detectReversedRegion();
	reclaimOuterBorder();

	if (!outer->alterBoundaryPath(false)) {
		return false;
	}

	detectOpticDiscRegion();

	// Outer boundary with optic disc. 
	if (!outer->alterBoundaryPath(true)) {
		return false;
	}
	return true;
}


void SemtSegm::Bordering::setSampling(Sampling * sampling)
{
	getImpl().sampling = sampling;
}

bool SemtSegm::Bordering::isReversedRegion() const
{
	return (getImpl().reversedX1 >= 0 || getImpl().reversedX2 >= 0);
}

bool SemtSegm::Bordering::isOpticDiscRegion() const
{
	return (getImpl().discX1 >= 0 && getImpl().discX2 >= 0);
}

bool SemtSegm::Bordering::isOpticDiscOpening() const
{
	return getImpl().isDiscOpening;
}

int SemtSegm::Bordering::getOpticDiscX1() const
{
	return getImpl().discX1;
}

int SemtSegm::Bordering::getOpticDiscX2() const
{
	return getImpl().discX2;
}

int SemtSegm::Bordering::getOpticDiscBaseX() const
{
	return getImpl().discBaseX;
}

int SemtSegm::Bordering::getOpticDiscBaseY() const
{
	return getImpl().discBaseY;
}

const Sampling * SemtSegm::Bordering::sampling()
{
	return getImpl().sampling;
}


bool SemtSegm::Bordering::interpolateOuterPoints(std::vector<int>& points)
{
	auto* pSegm = getImpl().pSegm;
	const auto* pImage = pSegm->getSampling()->sample();

	Mat srcMat = pImage->getCvMatConst();
	int cols = srcMat.cols;
	int rows = srcMat.rows;
	
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();

	const int FILTER_SIZE = 17;
	const int CURVE_DEGREE = 1;

	auto fitt = out_edges;
	auto data = out_edges;
	auto line = out_edges;

	int size = (int)out_edges.size();
	int spos = max((int)(size * 0.10f), 0);
	int epos = min((int)(size * 0.90f), size - 1);

	if (!interpolateByLinearFitt(data, fitt, true)) {
		return false;
	}

	line = CppUtil::SgFilter::smoothInts(fitt, FILTER_SIZE, CURVE_DEGREE);

	std::transform(begin(line), end(line), begin(line), [=](int elem) { return min(max(elem, 0), rows - 1); });
	points = line;
	return true;
}

bool SemtSegm::Bordering::interpolateByWeightFitt(const std::vector<int>& input, const std::vector<int>& weights, 
	std::vector<int>& output, bool sideFitt, float sideData)
{
	const int fittSize = (int)(sideData * input.size());

	float slope = 0.0f;
	int count = 0;
	int size = (int)input.size();
	int spos = (int)(size * 0.20f);
	int epos = (int)(size * 0.80f);
	output = input;

	int x1 = -1, x2 = -1;
	int y1 = -1, y2 = -1;

	if (input[0] < 0) {
		for (int i = 0; i < size; i++) {
			if (input[i] >= 0) {
				if (!sideFitt || i >= spos) {
					int ys1 = 0;
					int cnt1 = 0;
					for (int j = i; j < size; j++) {
						if (input[j] >= 0) {
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (int j = 0; j < i; j++) {
						// output[j] = input[i];
						output[j] = y1;
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
						int y_pred = 0;
						for (int k = (x1 - 1), dist = -1; k >= 0; k--, dist--) {
							y_pred = (int)(y1 + dist * slope);
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
				if (!sideFitt || i <= epos) {
					int ys1 = 0;
					int cnt1 = 0;
					for (int j = i - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (int j = i + 1; j < size; j++) {
						// output[j] = input[i];
						output[j] = y1;
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
						int y_pred = 0;
						for (int k = (x2 + 1), dist = 1; k < size; k++, dist++) {
							y_pred = (int)(y2 + dist * slope);
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

					for (int k = (x1 + 1); k < x2; k++) {
						int d1 = k - x1;
						int d2 = x2 - k;
						int w1 = weights[x1] * y1 * d2;
						int w2 = weights[x2] * y2 * d1;
						int wsum = weights[x1] * d2 + weights[x2] * d1;
						int val = (w1 + w2) / wsum;
						output[k] = val;
					}
					i = x2;
					break;
				}
			}
		}
	}
	return true;
}

bool SemtSegm::Bordering::interpolateByLinearFitt(const std::vector<int>& input, std::vector<int>& output, bool sideFitt, int moveSpan, float sideData)
{
	const int fittSize = (int)(sideData * input.size());

	float slope = 0.0f;
	int count = 0;
	int size = (int)input.size();
	int spos = (int)(size * 0.20f);
	int epos = (int)(size * 0.80f);
	output = input;

	int x1 = -1, x2 = -1;
	int y1 = -1, y2 = -1;

	if (input[0] < 0) {
		for (int i = 0; i < size; i++) {
			if (input[i] >= 0) {
				if (!sideFitt || i >= spos) {
					int ys1 = 0;
					int cnt1 = 0;
					for (int j = i; j < size; j++) {
						if (input[j] >= 0) {
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (int j = 0; j < i; j++) {
						// output[j] = input[i];
						output[j] = y1;
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
				if (!sideFitt || i <= epos) {
					int ys1 = 0;
					int cnt1 = 0;
					for (int j = i - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (int j = i + 1; j < size; j++) {
						// output[j] = input[i];
						output[j] = y1;
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

std::vector<int>& SemtSegm::Bordering::getOuterEdges() const
{
	return getImpl().outerEdges;
}

std::vector<int>& SemtSegm::Bordering::getInnerEdges() const
{
	return getImpl().innerEdges;
}

std::vector<int>& SemtSegm::Bordering::getInnerBound() const
{
	return getImpl().innerBound;
}

std::vector<int>& SemtSegm::Bordering::getOuterBound() const
{
	return getImpl().outerBound;
}

std::vector<int>& SemtSegm::Bordering::getCenterBound() const
{
	return getImpl().centerBound;
}

std::vector<int>& SemtSegm::Bordering::getOuterBorder() const
{
	return getImpl().outerBorder;
}

std::vector<int>& SemtSegm::Bordering::getInnerBorder() const
{
	return getImpl().innerBorder;
}

std::vector<int>& SemtSegm::Bordering::getOuterWeights() const
{
	return getImpl().outerWeights;
}

std::vector<int>& SemtSegm::Bordering::getInnerWeights() const
{
	return getImpl().innerWeights;
}


Bordering::BorderingImpl & SemtSegm::Bordering::getImpl(void) const
{
	return *d_ptr;
}


