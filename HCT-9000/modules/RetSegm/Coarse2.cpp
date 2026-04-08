#include "stdafx.h"
#include "Coarse2.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "DataFitt.h"
#include "Feature2.h"
#include "SegmImage.h"
#include "SegmLayer.h"
#include "Logger.h"

#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetSegm;
using namespace cv;
using namespace std;


Coarse2::Coarse2()
{
}


Coarse2::~Coarse2()
{
}


bool RetSegm::Coarse2::makeBoundaryLinesOfRetina(const SegmImage * imgSrc, const SegmImage * imgAsc, const SegmImage* imgDes, SegmLayer * layerInn, SegmLayer * layerOut, float rangeX, bool isDisc, int& discX1, int& discX2, int& retiX1, int& retiX2)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> ideal;
	vector<int> idealInn;
	vector<int> idealOut;

	if (isDisc) { // isDisc || rangeX > 6.0f) {
		locateBoundaryEdgesOfRetina(imgSrc, imgAsc, inner, outer);

		removeBoundaryOuterOutliers(inner, outer, outer);
		removeBoundaryInnerOutliers(inner, outer, inner);
	}
	else {
		locateBoundaryEdgesOfRetina2(imgSrc, imgAsc, inner, outer, isDisc);

		removeBoundaryOuterOutliers(inner, outer, outer);
		removeBoundaryInnerOutliers(inner, outer, inner);

		correctFalseInnerPoints(imgSrc, imgAsc, inner, outer);
	}

	if (!checkBoundaryPointsIsValid(inner, 0.45f)) {
		return false;
	}
	else {
		if (!checkBoundaryPointsIsValid(outer, 0.25f)) {
			locateOuterBoundaryOfBackside(imgSrc, imgDes, inner, outer);
			Feature2::makeupIdealFittingBoundary(inner, inner);
			Feature2::makeupIdealFittingBoundary(outer, outer);
			layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
			layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
			retiX1 = 0; retiX2 = (int)inner.size() - 1;
			return true;
		}
	}

	if (!replaceFalseBoundaryPoints(inner, outer, ideal, rangeX, isDisc)) {
		return false;
	}

	expandOuterBoundaryPoints(imgAsc, inner, outer, ideal, inner, outer);

	discX1 = discX2 = -1;
	if (isDisc) {
		Feature2::estimateOpticDiscMargin(imgAsc, inner, outer, rangeX, discX1, discX2);
		// Feature2::adjustOpticDiscBounds(inner, outer, rangeX, discX1, discX2);
	}

	// layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
	// layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
	// return true;

	retiX1 = retiX2 = -1;
	if (!Feature2::detectBoundaryPointsRegion(inner, outer, rangeX, retiX1, retiX2)) {
		return false;
	}

	Feature2::makeupIdealOuterBoundary(inner, outer, rangeX, discX1, discX2, idealOut);
	Feature2::makeupIdealInnerBoundary(inner, idealOut, rangeX, discX1, discX2, idealInn);

	inner = idealInn;
	outer = idealOut;
	layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
	layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
	return true;
}


bool RetSegm::Coarse2::locateBoundaryEdgesOfRetina(const SegmImage * srcImg, const SegmImage* ascImg, std::vector<int>& inner, std::vector<int>& outer)
{
	Mat srcMat = srcImg->getCvMat();
	Mat ascMat = ascImg->getCvMat();

	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);

	auto maxLocsInt = srcImg->getColumMaxLocs();
	auto maxValsInt = srcImg->getColumMaxVals();
	auto maxLocsAsc = ascImg->getColumMaxLocs();
	auto maxValsAsc = ascImg->getColumMaxVals();

	int baseThresh = (int)(ascImg->getMean() + ascImg->getStddev() * 0.0f);
	baseThresh = max(baseThresh, 7);
	int edgeThresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 0.5f);
	edgeThresh = max(edgeThresh, 15);
	int peakThresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 1.0f);
	peakThresh = max(peakThresh, 25);

	int r, c;
	for (c = 0; c < srcMat.cols; c++) {
		int maxIdx = maxLocsAsc[c];
		int maxVal = maxValsAsc[c];
		int curVal = 0;
		int preVal = 0;
		int innIdx = -1;
		int outIdx = -1;
		int thresh = 0;

		if (maxVal < peakThresh) {
			continue;
		}

		int uppBound = max(maxIdx - 120, 0);
		int lowBound = min(maxIdx + 120, srcMat.rows - 1);
		int uppVal = 0, lowVal = 0;
		int uppIdx = -1, lowIdx = -1;
		int subIdx = -1, subVal = 0;
		bool found = false;

		// Avascular region supposed.
		preVal = maxVal;
		for (r = (maxIdx - 1); r >= uppBound; r--) {
			curVal = ascMat.at<uchar>(r, c);
			if (curVal > preVal && preVal < baseThresh) {
				found = true;
				break;
			}
			preVal = curVal;
		}

		if (found) {
			subVal = 0, subIdx = r;
			for (; r >= uppBound; r--) {
				curVal = ascMat.at<uchar>(r, c);
				if (curVal >= subVal) {
					subVal = curVal;
					subIdx = r;
				}
			}

			thresh = edgeThresh;

			if (subVal >= thresh) {
				uppVal = subVal;
				uppIdx = subIdx;
			}

			// Upper retina boundary.
			if (uppIdx > 0) {
				for (; r >= uppBound; r--) {
					curVal = ascMat.at<uchar>(r, c);
					if (curVal <= uppVal) {
						uppVal = curVal;
						uppIdx = r;
					}
					else if (curVal < baseThresh) {
						break;
					}
				}
			}
		}

		// Beneath choriods or hole between photoreceptors escalated and bruch's membrane. 
		found = false;
		preVal = maxVal;
		for (r = (maxIdx + 1); r <= lowBound; r++) {
			curVal = ascMat.at<uchar>(r, c);
			if (curVal > preVal && preVal < baseThresh) {
				found = true;
				break;
			}
			preVal = curVal;
		}

		if (found) {
			subVal = 0, subIdx = r;
			for (; r <= lowBound; r++) {
				curVal = ascMat.at<uchar>(r, c);
				if (curVal >= subVal) {
					subVal = curVal;
					subIdx = r;
				}
			}

			thresh = max((int)(edgeThresh * 1.5f), (int)(maxVal * 0.35f));
			// thresh = edgeThresh;
			
			if (subVal >= thresh) {
				lowIdx = subIdx;
				lowVal = subVal;
			}

			if (lowIdx > 0) {
				for (; r <= lowBound; r++) {
					curVal = ascMat.at<uchar>(r, c);
					if (curVal <= lowVal) {
						lowVal = curVal;
						lowIdx = r;
					}
					else if (curVal < baseThresh) {
						break;
					}
				}
			}

			if (subVal > uppVal) {
				uppIdx = -1;
			}
		}

		if (uppIdx >= 0 && lowIdx >= 0) {
			if (uppVal >= lowVal) {
				innIdx = uppIdx;
				outIdx = maxIdx;
			}
			else {
				innIdx = maxIdx;
				outIdx = lowIdx;
			}
		}
		else if (uppIdx >= 0) {
			innIdx = uppIdx;
			outIdx = maxIdx;
		}
		else if (lowIdx >= 0) {
			innIdx = maxIdx;
			outIdx = lowIdx;
		}
		else {
			innIdx = maxIdx;
		}

		if (innIdx >= 0) {
			int limit = max(innIdx - 7, 0);
			for (r = innIdx; r >= limit; r--) {
				int val = ascMat.at<uchar>(r, c);
				if (val < peakThresh) {
					break;
				}
				else {
					innIdx = r;
				}
			}
		}

		if (outIdx >= 0) {
			int limit = max(outIdx - 7, innIdx);
			for (r = outIdx; r >= limit; r--) {
				int val = ascMat.at<uchar>(r, c);
				if (val < peakThresh) {
					break;
				}
				else {
					outIdx = r;
				}
			}
		}

		inner[c] = innIdx;
		outer[c] = outIdx;
	}

	return true;
}


bool RetSegm::Coarse2::locateBoundaryEdgesOfRetina2(const SegmImage * srcImg, const SegmImage * ascImg, 
										std::vector<int>& inner, std::vector<int>& outer, bool isDisc)
{
	Mat srcMat = srcImg->getCvMat();
	Mat ascMat = ascImg->getCvMat();

	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);

	auto maxLocsInt = srcImg->getColumMaxLocs();
	auto maxValsInt = srcImg->getColumMaxVals();
	auto maxLocsAsc = ascImg->getColumMaxLocs();
	auto maxValsAsc = ascImg->getColumMaxVals();

	int base_thresh = (int)(ascImg->getMean() + ascImg->getStddev() * 0.0f);
	int edge_thresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 0.5f);
	int peak_thresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 2.5f);
	int subo_thresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 1.5f);
	int grey_thresh = (int)(srcImg->getMean() + srcImg->getStddev() * 1.5f);

	// LogD() << base_thresh << ", " << edge_thresh << ", " << peak_thresh;

	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int r, c;
	for (c = 0; c < img_w; c++) {
		int max_idx = maxLocsAsc[c];
		int max_val = maxValsAsc[c];
		if (max_val < peak_thresh) {
			continue;
		}

		// If the reflective band in retina is considerably narrower than kernel, 
		// the convolutional peak might be arbitrarily assigned within kernel size
		// affected by noise signal nearby.
		// In the worst case scenario, the kernel peak could be positioned outside retina 
		// maximally kerel size distant, so the gradient peak should be searched within 
		// kernel range around.
		int y1 = (int)(floor(img_h * 0.12f));
		int y2 = (int)(floor(img_h * 0.88f));
		int ker_size = (int)(floor(img_h * 0.24f));
		int conv_peak = -1;
		int conv_cent = -1;
		int grad_max = -1;
		int grad_pos = -1;
		for (r = y1; r < y2; r += 4) {
			int wsum = 0;
			int ysum = 0;
			int grad = 0;
			int k1 = r - ker_size / 2;
			int k2 = r + ker_size / 2;
			int gmax = 0;
			int gpos = 0;

			for (int k = k1; k < k2; k += 2) {
				grad = min((int)(ascMat.at<uchar>(k, c)), subo_thresh);
				wsum += grad;
				ysum += (grad * k);
				if (grad > gmax) {
					gmax = grad;
					gpos = k;
				}
			}
			if (wsum > 0 && wsum > conv_peak) {
				conv_peak = wsum;
				conv_cent = (int)(ysum / wsum);
				grad_max = gmax;
				grad_pos = gpos;
			}
		}

		int ret_size = (int)(img_h * 0.24f);
		int upp_limit = max(conv_cent - ret_size, 0);
		int low_limit = min(conv_cent + ret_size, img_h - 1);
		int gap_size = (int)(img_h * 0.045f);
		int top_size = (int)(img_h * 0.15f);
		int sup_size = (int)(img_h * 0.12f);
		int inf_size = (int)(img_h * 0.18f);

		if (max_idx < upp_limit || max_idx > low_limit) {
			if (grad_max < peak_thresh) {
				continue;
			}
			max_idx = grad_pos;
			max_val = grad_max;
		}

		int sup_peak_val = -1;
		int sup_peak_idx = -1;
		int sup_base_idx = -1;
		for (r = max_idx; r >= upp_limit; r--) {
			int grad = ascMat.at<uchar>(r, c);
			if (grad < edge_thresh) {
				sup_base_idx = r;
				for (; r >= upp_limit; r--) {
					grad = ascMat.at<uchar>(r, c);
					if (sup_peak_val < grad) {
						sup_peak_val = grad;
						sup_peak_idx = r;
					}
				}
				break;
			}
		}

		// Inner retina is stuck to the top of image.  
		if (sup_peak_idx < 0) {
			if (upp_limit == 0) {
				sup_base_idx = max_idx;
				sup_peak_idx = max_idx;
				sup_peak_val = max_val;
			}
		}

		int inf_peak_val = -1;
		int inf_peak_idx = -1;
		int inf_base_idx = -1;
		for (r = max_idx; r <= low_limit; r++) {
			int grad = ascMat.at<uchar>(r, c);
			if (grad < edge_thresh) {
				inf_base_idx = r;
				for (; r <= low_limit; r++) {
					grad = ascMat.at<uchar>(r, c);
					if (inf_peak_val < grad) {
						inf_peak_val = grad;
						inf_peak_idx = r;
					}
					if (inf_peak_val >= subo_thresh) {
						if (abs(r - sup_peak_idx) > ret_size) {
							break;
						}
					}
				}
				break;
			}
		}

		if (sup_peak_idx < 0 || inf_peak_idx < 0) {
			continue;
		}

		int inn_peak_idx = -1;
		int out_peak_idx = -1;
		int inn_edge_idx = -1;
		int out_edge_idx = -1;
		int mid_base_idx = -1;

		bool one_peak = false;
		int outs_thresh = min((int)(max_val * 0.75f), peak_thresh);
		int rpes_thresh = max((int)(max_val * 0.65f), subo_thresh);

		// Inner surface is distinguished as the first of mostly reflective regions, 
		// which are identified by the distinctly increasing phase of gradients. 
		if (inn_peak_idx < 0 && out_peak_idx < 0) {
			if (sup_peak_val >= subo_thresh && sup_peak_idx != max_idx) {
				inn_peak_idx = sup_peak_idx;
				out_peak_idx = max_idx;
			}
		}

		// Otherwise, outer surface should be seperately beneath the maximum of gradients,
		// is required to meet the higher criterion to avoid the choroidal region. 
		if (inn_peak_idx < 0 && out_peak_idx < 0) {
			if (inf_peak_val >= outs_thresh && inf_peak_idx != max_idx) {
				inn_peak_idx = max_idx;
				out_peak_idx = inf_peak_idx;
			}
		}

		// In case both surfaces are adhered to each other. As a combined signal, 
		// it should be formed as multimodal gaussian shape, typically consisting of 
		// two identifiable peaks in each subdivision. 
		if (inn_peak_idx < 0 && out_peak_idx < 0) {
			if (abs(inf_base_idx - sup_base_idx) > 24 && inf_peak_val < outs_thresh) {
				mid_base_idx = (sup_base_idx + inf_base_idx) / 2;
				sup_peak_val = -1;
				for (r = sup_base_idx; r <= mid_base_idx; r++) {
					int grad = ascMat.at<uchar>(r, c);
					if (sup_peak_val < grad) {
						sup_peak_val = grad;
						sup_peak_idx = r;
					}
				}
				inf_peak_val = -1;
				for (r = mid_base_idx; r <= inf_base_idx; r++) {
					int grad = ascMat.at<uchar>(r, c);
					if (inf_peak_val < grad) {
						inf_peak_val = grad;
						inf_peak_idx = r;
					}
				}

				if (abs(inf_peak_idx - sup_peak_idx) > 6) {
					inn_peak_idx = sup_peak_idx;
					out_peak_idx = inf_peak_idx;
				}
				else {
					inn_peak_idx = sup_peak_idx;
					// out_peak_idx = inf_peak_idx;
				}

				one_peak = true;
			}
		}

		// Retina is clinging to the top of image. 
		if (inn_peak_idx < 0 && out_peak_idx < 0) {
			if (max_idx < top_size) {
				if (sup_peak_idx != max_idx && sup_peak_idx > subo_thresh) {
					inn_peak_idx = sup_peak_idx;
					out_peak_idx = max_idx;
				}
				else if (inf_peak_val >= outs_thresh) {
					inn_peak_idx = max_idx;
					out_peak_idx = inf_peak_idx;
				}
				else {
					for (r = 0; r <= max_idx; r++) {
						int ival = srcMat.at<uchar>(r, c);
						if (ival >= grey_thresh) {
							if (abs(max_idx - r) > 8) {
								inn_peak_idx = r;
								out_peak_idx = max_idx;
							}
							break;
						}
					}
				}
			}
		}

		if (inn_peak_idx < 0 && out_peak_idx < 0) {
			if (sup_peak_val >= edge_thresh) {
				inn_peak_idx = sup_peak_idx;
				if (sup_peak_idx == max_idx) {
					if (inf_peak_val >= subo_thresh) {
						out_peak_idx = inf_peak_idx;
					}
				}
				else {
					if (inf_peak_val >= subo_thresh) {
						inn_peak_idx = max_idx;
						out_peak_idx = inf_peak_idx;
					}
					else {
						out_peak_idx = max_idx;
					}
				}
			}
			else {
				inn_peak_idx = max_idx;
				if (inf_peak_val >= subo_thresh) {
					out_peak_idx = inf_peak_idx;
				}
			}
		}

		if (inn_peak_idx >= 0) {
			for (r = inn_peak_idx; r >= upp_limit; r--) {
				int grad = ascMat.at<uchar>(r, c);
				if (grad >= subo_thresh) {
					inn_peak_idx = r;
				}
			}

			inn_edge_idx = inn_peak_idx;
			for (r = inn_peak_idx; r >= upp_limit; r--) {
				int grad = ascMat.at<uchar>(r, c);
				if (grad < edge_thresh) {
					break;
				}
				inn_edge_idx = r;
			}
			inner[c] = inn_edge_idx;
		}

		if (out_peak_idx >= 0) {
			if (one_peak) {
				out_edge_idx = out_peak_idx;
				int out_edge_val = inf_peak_val;

				for (r = out_peak_idx; r >= mid_base_idx; r--) {
					int grad = ascMat.at<uchar>(r, c);
					if (grad > out_edge_val) {
						break;
					}
					out_edge_val = grad;
					out_edge_idx = r;
				}
			}
			else {
				if (abs(out_peak_idx - inn_peak_idx) < sup_size) {
					int out_limit = min((inn_peak_idx + ret_size), img_h - 1);
					for (r = out_peak_idx; r <= out_limit; r++) {
						int grad = ascMat.at<uchar>(r, c);
						if (grad >= rpes_thresh) {
							out_peak_idx = r;
						}
					}
				}

				out_edge_idx = out_peak_idx;
				for (r = out_peak_idx; r >= 0; r--) {
					int grad = ascMat.at<uchar>(r, c);
					if (grad < edge_thresh) {
						break;
					}
					out_edge_idx = r;
				}
			}
			outer[c] = out_edge_idx;
		}

		// inner[c] = sup_base_idx;
		// outer[c] = conv_cent;
	}
	return true;
}



void RetSegm::Coarse2::correctFalseInnerPoints(const SegmImage * srcImg, const SegmImage * ascImg, std::vector<int>& inner, std::vector<int>& outer)
{
	Mat srcMat = srcImg->getCvMat();
	Mat ascMat = ascImg->getCvMat();

	int edgeThresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 0.5f);
	edgeThresh = max(edgeThresh, 15);

	auto inn_curv = inner;
	auto out_curv = outer;

	int filter = (int)(inner.size() * 0.05f);
	int degree = 1;

	for (int retry = 0; retry < 3; retry++) {
		DataFitt::interpolateBoundaryByLinearFitting(inner, inn_curv, true);
		DataFitt::interpolateBoundaryByLinearFitting(outer, out_curv, true);

		inn_curv = CppUtil::SgFilter::smoothInts(inn_curv, filter, degree);
		out_curv = CppUtil::SgFilter::smoothInts(out_curv, filter, degree);

		int c;
		int count = 0;
		for (c = 0; c < srcMat.cols; c++) {
			if (inner[c] > 0) {
				if (abs(inn_curv[c] - inner[c]) > abs(out_curv[c] - inner[c])) {
					inner[c] = -1;
					outer[c] = -1;
					count++;
				}
			}

			if (outer[c] > 0) {
				if (abs(inn_curv[c] - outer[c]) < abs(out_curv[c] - outer[c])) {
					inner[c] = -1;
					outer[c] = -1;
					count++;
				}
			}
		}

		if (count == 0) {
			break;
		}
	}

	int limit = (int)(outer.size() * 0.075f);
	for (int c = 0; c < limit; c++) {
		if (outer[c] < 0) {
			outer[c] = out_curv[c];
		}
		else {
			break;
		}
	}

	for (int c = srcMat.cols-1; c > srcMat.cols-limit; c--) {
		if (outer[c] < 0) {
			outer[c] = out_curv[c];
		}
		else {
			break;
		}
	}

	/*
	for (int retry = 0; retry < 3; retry++) {
		DataFitt::interpolateBoundaryByLinearFitting(inner, inn_curv, true);
		DataFitt::interpolateBoundaryByLinearFitting(outer, out_curv, true);

		inn_curv = CppUtil::SgFilter::smoothInts(inner, filter, degree);
		out_curv = CppUtil::SgFilter::smoothInts(outer, filter, degree);

		int r, c;
		int count = 0;
		for (c = 0; c < srcMat.cols; c++) {
			if (abs(inn_curv[c] - inner[c]) < abs(out_curv[c] - inner[c])) {
				continue;
			}

			int bound = max(inn_curv[c] - 15, 0);
			int cur_val = 0;
			int upp_idx = -1;

			for (r = inner[c]; r >= bound; r--) {
				cur_val = ascMat.at<uchar>(r, c);
				if (cur_val >= edgeThresh) {
					upp_idx = r;
				}
			}

			if (upp_idx > 0) {
				outer[c] = inner[c];
				inner[c] = upp_idx;
			}
			else {
				outer[c] = inner[c];
				inner[c] = -1;
			}
			count++;
		}

		if (count == 0) {
			break;
		}
	}

	for (int retry = 0; retry < 1; retry++) {
		inn_curv = CppUtil::SgFilter::smoothInts(inner, filter, degree);
		out_curv = CppUtil::SgFilter::smoothInts(outer, filter, degree);

		int r, c;
		int count = 0;
		for (c = 0; c < srcMat.cols; c++) {
			if (abs(inn_curv[c] - outer[c]) > abs(out_curv[c] - outer[c])) {
				continue;
			}

			inner[c] = -1;
			outer[c] = -1;
			count++;
		}

		if (count == 0) {
			break;
		}
	}
	*/

	// inner = inn_curv;
	// outer = out_curv;
	return;
}



void RetSegm::Coarse2::removeBoundaryInnerOutliers(const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& result)
{
	int size = (int)outer.size();
	int minSegm = (int)(size * 0.035f);
	int maxEmpt = (int)(size * 0.050f);
	int yRange = (int)(13);

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	int slen = 0, none = 0;
	result = inner;

	for (int c = 0; c < size; c++) {
		if (inner[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				x1 = x2 = c;
				y1 = y2 = inner[c];
				slen = 1;
			}
			else {
				if ((abs(inner[c] - y2) <= yRange)) {
					x2 = c;
					y2 = inner[c];
					slen++;
				}
				else {
					if (slen < minSegm) {
						int boundX1 = max(x1 - maxEmpt, 0);
						int boundX2 = min(x2 + maxEmpt, size - 1);
						int ext_y1 = y1, ext_y2 = y2;
						for (int k = c + 1; k <= boundX2; k++) {
							if (inner[k] >= 0 && (abs(inner[k] - ext_y2) <= yRange)) {
								ext_y2 = inner[k];
								// boundX2 = min(k + maxEmpt, size - 1);
								if (++slen >= minSegm) {
									break;
								}
							}
							else if (outer[k] >= 0 && (abs(outer[k] - ext_y2) <= yRange)) {
								ext_y2 = outer[k];
								// boundX2 = min(k + maxEmpt, size - 1);
								if (++slen >= minSegm) {
									break;
								}
							}
						}
						if (slen < minSegm) {
							for (int k = x1 - 1; k >= boundX1; k--) {
								if (inner[k] >= 0 && (abs(inner[k] - ext_y1) <= yRange)) {
									ext_y1 = inner[k];
									// boundX1 = max(k - maxEmpt, 0);
									if (++slen >= minSegm) {
										break;
									}
								}
								else if (outer[k] >= 0 && (abs(outer[k] - ext_y1) <= yRange)) {
									ext_y1 = outer[k];
									// boundX1 = min(k + maxEmpt, size - 1);
									if (++slen >= minSegm) {
										break;
									}
								}
							}
						}
						if (slen < minSegm) {
							for (int k = x1; k <= x2; k++) {
								result[k] = -1;
							}
						}
					}
					x1 = x2 = c;
					y1 = y2 = inner[c];
					slen = 1;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > maxEmpt) {
					if (slen < minSegm) {
						int boundX1 = max(x1 - maxEmpt, 0);
						int ext_y1 = y1;
						for (int k = x1 - 1; k >= boundX1; k--) {
							if (inner[k] >= 0 && (abs(inner[k] - ext_y1) <= yRange)) {
								ext_y1 = inner[k];
								// boundX1 = max(k - maxEmpt, 0);
								if (++slen >= minSegm) {
									break;
								}
							}
							else if (outer[k] >= 0 && (abs(outer[k] - ext_y1) <= yRange)) {
								ext_y1 = outer[k];
								// boundX1 = min(k + maxEmpt, size - 1);
								if (++slen >= minSegm) {
									break;
								}
							}
						}
						if (slen < minSegm) {
							for (int k = x1; k <= x2; k++) {
								result[k] = -1;
							}
						}
					}
					slen = 0;
				}
			}
		}
	}

	if (slen > 0) {
		if (slen < minSegm) {
			int boundX1 = max(x1 - maxEmpt, 0);
			int ext_y1 = y1;
			for (int k = x1 - 1; k >= boundX1; k--) {
				if (inner[k] >= 0 && (abs(inner[k] - ext_y1) <= yRange)) {
					ext_y1 = inner[k];
					// boundX1 = max(k - maxEmpt, 0);
					if (++slen >= minSegm) {
						break;
					}
				}
				else if (outer[k] >= 0 && (abs(outer[k] - ext_y1) <= yRange)) {
					ext_y1 = outer[k];
					// boundX1 = min(k + maxEmpt, size - 1);
					if (++slen >= minSegm) {
						break;
					}
				}
			}
			if (slen < minSegm) {
				for (int k = x1; k <= x2; k++) {
					result[k] = -1;
				}
			}
		}
	}

	return;
}


void RetSegm::Coarse2::removeBoundaryOuterOutliers(const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& result)
{
	int size = (int)outer.size();
	int minSegm = (int)(size * 0.025f);
	int maxEmpt = (int)(size * 0.050f);
	int yRange = (int)(7);

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	int slen = 0, none = 0;
	result = outer;

	for (int c = 0; c < size; c++) {
		if (outer[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				x1 = x2 = c;
				y1 = y2 = outer[c];
				slen = 1;
			}
			else {
				if ((abs(outer[c] - y2) <= yRange)) {
					x2 = c;
					y2 = outer[c];
					slen++;
				}
				else {
					if (slen < minSegm) {
						int boundX1 = max(x1 - maxEmpt, 0);
						int boundX2 = min(x2 + maxEmpt, size - 1);
						int ext_y1 = y1, ext_y2 = y2;
						for (int k = c + 1; k <= boundX2; k++) {
							if (outer[k] >= 0 && (abs(outer[k] - ext_y2) <= yRange)) {
								ext_y2 = outer[k];
								// boundX2 = min(k + maxEmpt, size - 1);
								if (++slen >= minSegm) {
									break;
								}
							}
							else if (inner[k] >= 0 && (abs(inner[k] - ext_y2) <= yRange)) {
								ext_y2 = inner[k];
								// boundX2 = min(k + maxEmpt, size - 1);
								if (++slen >= minSegm) {
									break;
								}
							}
						}
						if (slen < minSegm) {
							for (int k = x1 - 1; k >= boundX1; k--) {
								if (outer[k] >= 0 && (abs(outer[k] - ext_y1) <= yRange)) {
									ext_y1 = outer[k];
									// boundX1 = max(k - maxEmpt, 0);
									if (++slen >= minSegm) {
										break;
									}
								}
								else if (inner[k] >= 0 && (abs(inner[k] - ext_y1) <= yRange)) {
									ext_y1 = inner[k];
									// boundX1 = min(k + maxEmpt, size - 1);
									if (++slen >= minSegm) {
										break;
									}
								}
							}
						}
						if (slen < minSegm) {
							for (int k = x1; k <= x2; k++) {
								result[k] = -1;
							}
						}
					}
					x1 = x2 = c;
					y1 = y2 = outer[c];
					slen = 1;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > maxEmpt) {
					if (slen < minSegm) {
						int boundX1 = max(x1 - maxEmpt, 0);
						int ext_y1 = y1;
						for (int k = x1 - 1; k >= boundX1; k--) {
							if (outer[k] >= 0 && (abs(outer[k] - ext_y1) <= yRange)) {
								ext_y1 = outer[k];
								// boundX1 = max(k - maxEmpt, 0);
								if (++slen >= minSegm) {
									break;
								}
							}
							else if (inner[k] >= 0 && (abs(inner[k] - ext_y1) <= yRange)) {
								ext_y1 = inner[k];
								// boundX1 = min(k + maxEmpt, size - 1);
								if (++slen >= minSegm) {
									break;
								}
							}
						}
						if (slen < minSegm) {
							for (int k = x1; k <= x2; k++) {
								result[k] = -1;
							}
						}
					}
					slen = 0;
				}
			}
		}
	}

	if (slen > 0) {
		if (slen < minSegm) {
			int boundX1 = max(x1 - maxEmpt, 0);
			int ext_y1 = y1;
			for (int k = x1 - 1; k >= boundX1; k--) {
				if (outer[k] >= 0 && (abs(outer[k] - ext_y1) <= yRange)) {
					ext_y1 = outer[k];
					// boundX1 = max(k - maxEmpt, 0);
					if (++slen >= minSegm) {
						break;
					}
				}
				else if (inner[k] >= 0 && (abs(inner[k] - ext_y1) <= yRange)) {
					ext_y1 = inner[k];
					// boundX1 = min(k + maxEmpt, size - 1);
					if (++slen >= minSegm) {
						break;
					}
				}
			}
			if (slen < minSegm) {
				for (int k = x1; k <= x2; k++) {
					result[k] = -1;
				}
			}
		}
	}

	return;
}


bool RetSegm::Coarse2::replaceFalseBoundaryPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& ideal, float rangeX, bool isDisc)
{
	auto size = (int)outer.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);
	auto dataY1 = vector<int>(size, -1);
	auto fittY1 = vector<int>(size, -1);
	auto dataY2 = vector<int>(size, -1);
	auto fittY2 = vector<int>(size, -1);
	auto dataY3 = vector<int>(size, -1);
	auto fittY3 = vector<int>(size, -1);
	auto dataY4 = vector<int>(size, -1);
	auto fittY4 = vector<int>(size, -1);

	auto inns = inner;
	auto outs = outer;

	int discX1 = -1, discX2 = -1;
	int halfX1 = -1, halfX2 = -1;
	int retiX1 = -1, retiX2 = -1;
	int outsX1 = -1, outsX2 = -1;
	bool found = false;
	bool failed = false;
	
	for (int retry = 0; retry < 5; retry++) {
		int corrects = 0;

		dataX = vector<int>(size, -1);
		dataY = vector<int>(size, -1);
		fittY = vector<int>(size, -1);

		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			dataY[i] = outs[i];
		}

		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (abs(outs[i] - fittY[i]) > 35) {
					outs[i] = -1;
				}
			}
		}
		else {
			failed = true;
			break; //  return false;
		}

		dataX = vector<int>(size, -1);
		dataY = vector<int>(size, -1);
		fittY = vector<int>(size, -1);

		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			dataY[i] = outs[i];
		}

		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (abs(outs[i] - fittY[i]) > 25) {
					outs[i] = -1;
				}
			}
		}
		else {
			failed = true;
			break; // return false;
		}

		discX1 = -1; discX2 = -1;
		halfX1 = -1; halfX2 = -1;
		retiX1 = -1; retiX2 = -1;
		outsX1 = -1; outsX2 = -1;

		if (!Feature2::detectBoundaryPointsRegion(inns, outs, rangeX, retiX1, retiX2)) {
			// return false;
			failed = true;
			break;
		}
		else {
			if (!isDisc || (found = Feature2::detectOpticDiscRegion(outs, rangeX, discX1, discX2)) == false) {
				discX1 = discX2 = -1;
			}
			halfX1 = (retiX1 + retiX2) / 2;
			halfX2 = size - 1;
			outsX1 = retiX1 + (retiX2 - retiX1) / 6;
			outsX2 = retiX2 - (retiX2 - retiX1) / 6;
		}

		dataX = vector<int>(size, -1);
		dataY1 = vector<int>(size, -1);
		fittY1 = vector<int>(size, -1);
		dataY2 = vector<int>(size, -1);
		fittY2 = vector<int>(size, -1);

		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= 0 && i < halfX1) {
				dataY1[i] = outs[i];
				dataY2[i] = inns[i];
				if (i >= discX1 && i <= discX2) {
					dataY1[i] = -1;
					dataY2[i] = -1;
				}
			}
		}

		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY1, 2, true, fittY1, coeffs) && 
			CppUtil::Regressor::chevyPolyCurve(dataX, dataY2, 2, true, fittY2, coeffs)) {
			for (int i = 0; i < halfX1; i++) {
				if (inns[i] >= 0) {
					if (i >= discX1 && i <= discX2) {
					}
					else if (i <= outsX1 || i >= outsX2) {
						if ((inns[i] - fittY1[i]) >= -5 || abs(inner[i] - fittY1[i]) < abs(inner[i] - fittY2[i])) {
							outs[i] = inns[i];
							inns[i] = -1;
							corrects++;
						}
					}
				}
				if (outs[i] >= 0) {
					if (i >= discX1 && i <= discX2) {
						outs[i] = -1;
						corrects++;
					}
					else { // if (i > outsX1 && i < outsX2) { 
						if ((outs[i] - fittY1[i]) > 15 || (fittY1[i] - outs[i]) > 15) {
							outs[i] = -1;
							corrects++;
						}
					}
				}
			}
		}
		else {
			failed = true;
			break;
			// return false;
		}

		dataX = vector<int>(size, -1);
		dataY3 = vector<int>(size, -1);
		fittY3 = vector<int>(size, -1);
		dataY4 = vector<int>(size, -1);
		fittY4 = vector<int>(size, -1);

		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= halfX1 && i < size) {
				dataY3[i] = outs[i];
				dataY4[i] = inns[i];
				if (i >= discX1 && i <= discX2) {
					dataY3[i] = -1;
					dataY4[i] = -1;
				}
			}
		}

		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY3, 2, true, fittY3, coeffs) &&
			CppUtil::Regressor::chevyPolyCurve(dataX, dataY4, 2, true, fittY4, coeffs)) {
			for (int i = halfX1; i < size; i++) {
				if (inns[i] >= 0) {
					if (i >= discX1 && i <= discX2) {
					}
					else if (i <= outsX1 || i >= outsX2) {
						if ((inns[i] - fittY3[i]) >= -5 || abs(inner[i] - fittY3[i]) < abs(inner[i] - fittY4[i])) {
							outs[i] = inns[i];
							inns[i] = -1;
							corrects++;
						}
					}
				}
				if (outs[i] >= 0) {
					if (i >= discX1 && i <= discX2) {
						outs[i] = -1;
						corrects++;
					}
					else { // if (i > outsX1 && i < outsX2) {
						if ((outs[i] - fittY3[i]) > 15 || (fittY3[i] - outs[i]) > 15) {
							outs[i] = -1;
							corrects++;
						}
					}
				}
			}
		}
		else {
			failed = true;
			break;
		}

		if (corrects <= 0) {
			break;
		}
	}

	if (!failed && halfX1 > 0 && halfX2 > 0 && (halfX1 < halfX2)) {
		inner = inns;
		outer = outs;
		ideal = outs;

		for (int i = 0; i < halfX1; i++) {
			ideal[i] = fittY1[i];
		}
		for (int i = halfX1; i < size; i++) {
			ideal[i] = fittY3[i];
		}
		if (isDisc && found) {
			for (int i = discX1; i <= discX2; i++) {
				ideal[i] = -1;
			}
		}
		return true;
	}
	else {
		ideal = outer;
		return true;
	}
}


bool RetSegm::Coarse2::expandOuterBoundaryPoints(const SegmImage * ascImg, const std::vector<int>& inner, const std::vector<int>& outer, const std::vector<int>& ideal, std::vector<int>& result1, std::vector<int>& result2)
{
	Mat ascMat = ascImg->getCvMat();
	int img_w = ascMat.cols;
	int img_h = ascMat.rows;

	int edgeThresh = (int)(ascImg->getMean() * 0.0f + ascImg->getStddev() * 1.0f);
	edgeThresh = max(edgeThresh, 15);
	
	auto inns = inner;
	auto outs = outer;
	auto size = (int)outer.size();
	int gsum = 0, gcnt = 0;
	int thresh = 0;

	int r, c;
	for (c = 0; c < ascMat.cols; c++) {
		if (outer[c] >= 0 && outer[c] < img_h) {
			r = outer[c];
			gsum += ascMat.at<uchar>(r, c);
			gcnt += 1;
		}
	}
	thresh = (gcnt > 0 ? (int)((gsum / gcnt) * 0.75f) : 0);
	thresh = max(thresh, edgeThresh);

	auto inn_curv = inner;
	DataFitt::interpolateBoundaryByLinearFitting(inner, inn_curv, true);
	DataFitt::smoothBoundaryLine(inn_curv, inn_curv, 0.05f);

	auto out_curv = ideal;
	DataFitt::interpolateBoundaryByLinearFitting(ideal, out_curv, true);
	DataFitt::smoothBoundaryLine(out_curv, out_curv, 0.05f);

	for (c = 0; c < ascMat.cols; c++) {
		if (outer[c] < 0 && ideal[c] >= 0) {
			if (inner[c] < 0 /*|| (out_curv[c] - inn_curv[c]) > 5*/) {
				r = out_curv[c];
				int y1 = max(r - 9, 0);
				int y2 = min(r + 9, ascMat.rows - 1);
				int curVal = 0, maxVal = 0, maxIdx = -1;

				for (r = y1; r <= y2; r++) {
					curVal = ascMat.at<uchar>(r, c);
					if (curVal > maxVal) {
						maxVal = curVal;
						maxIdx = r;
					}
				}

				if (maxVal >= thresh) {
					outs[c] = maxIdx;
				}
			}
		}
	}

	for (c = ascMat.cols - 1; c >= 0; c--) {
		if (outer[c] < 0 && ideal[c] >= 0) {
			if (inner[c] < 0 /*|| (out_curv[c] - inn_curv[c]) > 5*/) {
				r = out_curv[c];
				int y1 = max(r - 9, 0);
				int y2 = min(r + 9, ascMat.rows - 1);
				int curVal = 0, maxVal = 0, maxIdx = -1;

				for (r = y1; r <= y2; r++) {
					curVal = ascMat.at<uchar>(r, c);
					if (curVal > maxVal) {
						maxVal = curVal;
						maxIdx = r;
					}
				}

				if (maxVal >= thresh) {
					outs[c] = maxIdx;
				}
			}
		}
	}

	result1 = inns;
	result2 = outs;
	return true;
}


bool RetSegm::Coarse2::checkBoundaryPointsIsValid(const std::vector<int>& points, float minSize)
{
	int count = (int)count_if(cbegin(points), cend(points), [](int e) { return e >= 0; });
	int limit = (int)(points.size() * minSize);

	if (count < limit) {
		return false;
	}
	return true;
}


void RetSegm::Coarse2::locateOuterBoundaryOfBackside(const SegmImage * srcImg, const SegmImage * desImg, std::vector<int>& inner, std::vector<int>& outer)
{
	Mat desMat = desImg->getCvMat();
	outer = vector<int>(desMat.cols, -1);

	int r, c;
	for (c = 0; c < desMat.cols; c++) {
		if (inner[c] < 0) {
			continue;
		}

		int maxVal = -1, maxPos = -1;
		int curVal;
		for (r = inner[c]; r < desMat.rows; r++) {
			curVal = desMat.at<uchar>(r, c);
			if (curVal >= maxVal) {
				maxVal = curVal;
				maxPos = r;
			}
		}

		outer[c] = maxPos;
	}
	return;
}
