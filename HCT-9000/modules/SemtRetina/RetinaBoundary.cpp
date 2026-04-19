#include "stdafx.h"
#include "RetinaBoundary.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct RetinaBoundary::RetinaBoundaryImpl
{
	const RetinaSegmenter* segm = nullptr;

	vector<int> uppers;
	vector<int> lowers;
	vector<int> deltas;
	vector<int> minPath;

	std::vector<int> sampleYs;
	std::vector<int> sourceYs;

	Mat matEdge;
	Mat matCost;
	Mat matProb;

	RetinaBoundaryImpl()
	{
	}
};


RetinaBoundary::RetinaBoundary(RetinaSegmenter* segm) :
	d_ptr(make_unique<RetinaBoundaryImpl>())
{
	impl().segm = segm;
}


SemtRetina::RetinaBoundary::~RetinaBoundary() = default;
SemtRetina::RetinaBoundary::RetinaBoundary(RetinaBoundary&& rhs) = default;
RetinaBoundary& SemtRetina::RetinaBoundary::operator=(RetinaBoundary&& rhs) = default;


std::vector<int>& SemtRetina::RetinaBoundary::sampleYs(void) const
{
	return impl().sampleYs;
}

std::vector<int>& SemtRetina::RetinaBoundary::sourceYs(void) const
{
	return impl().sourceYs;
}

const RetinaSegmenter* SemtRetina::RetinaBoundary::retinaSegmenter(void) const
{
	return impl().segm;
}

bool SemtRetina::RetinaBoundary::searchPathMinCost()
{
	const auto matCost = impl().matCost;
	auto minPath = vector<int>(matCost.cols, -1);

	const auto& uppers = impl().uppers;
	const auto& lowers = impl().lowers;
	const auto& deltas = impl().deltas;

	// Starting from the right most column. 
	int lastIdx = -1, nextIdx = -1;
	float minCost = PATH_COST_MAX;
	int r, c;

	Mat matAccm;
	matCost.copyTo(matAccm);

	for (int c = 1; c < matCost.cols; c++) {
		auto y1 = uppers[c];
		auto y2 = lowers[c];
		for (int r = y1; r <= y2; r++) {
			minCost = PATH_COST_MAX;
			auto r1 = r - deltas[c - 1];
			auto r2 = r + deltas[c - 1];
			for (int k = r1; k <= r2; k++) {
				auto b1 = uppers[c - 1];
				auto b2 = lowers[c - 1];
				if (k >= b1 && k <= b2) {
					auto val = matAccm.at<float>(k, c - 1);
					if (val < minCost) {
						minCost = val;
					}
				}
			}
			matAccm.at<float>(r, c) += minCost;
		}
	}

	minCost = PATH_COST_MAX;
	c = matAccm.cols - 1;
	for (r = uppers[c]; r <= lowers[c]; r++) {
		if (matAccm.at<float>(r, c) < minCost) {
			minCost = matAccm.at<float>(r, c);
			lastIdx = r;
		}
	}
	minPath[c] = (lastIdx < 0 ? ((uppers[c] + lowers[c]) / 2) : lastIdx);
	lastIdx = minPath[c];

	// Follow the minimum cost path in reverse order. 
	// Next point should be within the allowed vertical distance. 
	int r1, r2;
	for (c = matAccm.cols - 2; c >= 0; c--) {
		r1 = max(lastIdx - deltas[c], uppers[c]);
		r2 = min(lastIdx + deltas[c], lowers[c]);

		if (r1 > lowers[c] || r2 < uppers[c]) {
			minPath[c] = (r1 > lowers[c] ? lowers[c] : uppers[c]);
		}
		else {
			// Note that if any element with cost calculated from intensities (less than maximum at default) 
			// is not found at this column, the last index would go down toward the bottom of map. 
			minCost = PATH_COST_MAX;
			nextIdx = -1;
			for (r = r1; r <= r2; r++) {
				if (matAccm.at<float>(r, c) < minCost) {
					minCost = matAccm.at<float>(r, c);
					nextIdx = r;
				}
			}
			minPath[c] = (nextIdx < 0 ? ((uppers[c] + lowers[c]) / 2) : nextIdx);
		}
		lastIdx = minPath[c];
	}
	impl().minPath = minPath;
	return true;
}


bool SemtRetina::RetinaBoundary::searchPathMinCostInRange(void)
{
	const auto matCost = impl().matCost;
	auto minPath = vector<int>(matCost.cols, -1);

	const auto& uppers = impl().uppers;
	const auto& lowers = impl().lowers;
	const auto& deltas = impl().deltas;

	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	const int col_beg = band->retinaBeginX();
	const int col_end = band->retinaEndX();

	// Starting from the right most column. 
	int lastIdx = -1, nextIdx = -1;
	float minCost = PATH_COST_MAX;
	int r, c;

	Mat matAccm;
	matCost.copyTo(matAccm);

	for (int c = col_beg + 1; c <= col_end; c++) {
		auto y1 = uppers[c];
		auto y2 = lowers[c];
		for (int r = y1; r <= y2; r++) {
			minCost = PATH_COST_MAX;
			auto r1 = r - deltas[c - 1];
			auto r2 = r + deltas[c - 1];
			for (int k = r1; k <= r2; k++) {
				auto b1 = uppers[c - 1];
				auto b2 = lowers[c - 1];
				if (k >= b1 && k <= b2) {
					auto val = matAccm.at<float>(k, c - 1);
					if (val < minCost) {
						minCost = val;
					}
				}
			}
			matAccm.at<float>(r, c) += minCost;
		}
	}

	minCost = PATH_COST_MAX;
	c = col_end;
	for (r = uppers[c]; r <= lowers[c]; r++) {
		if (matAccm.at<float>(r, c) < minCost) {
			minCost = matAccm.at<float>(r, c);
			lastIdx = r;
		}
	}
	minPath[c] = (lastIdx < 0 ? ((uppers[c] + lowers[c]) / 2) : lastIdx);
	lastIdx = minPath[c];

	// Follow the minimum cost path in reverse order. 
	// Next point should be within the allowed vertical distance. 
	int r1, r2;
	for (c = col_end - 1; c >= col_beg; c--) {
		r1 = max(lastIdx - deltas[c], uppers[c]);
		r2 = min(lastIdx + deltas[c], lowers[c]);

		if (r1 > lowers[c] || r2 < uppers[c]) {
			minPath[c] = (r1 > lowers[c] ? lowers[c] : uppers[c]);
		}
		else {
			// Note that if any element with cost calculated from intensities (less than maximum at default) 
			// is not found at this column, the last index would go down toward the bottom of map. 
			minCost = PATH_COST_MAX;
			nextIdx = -1;
			for (r = r1; r <= r2; r++) {
				if (matAccm.at<float>(r, c) < minCost) {
					minCost = matAccm.at<float>(r, c);
					nextIdx = r;
				}
			}
			minPath[c] = (nextIdx < 0 ? ((uppers[c] + lowers[c]) / 2) : nextIdx);
		}
		lastIdx = minPath[c];
	}

	const int SLOPE_SIZE = crta->getPathSideMarginSlopeWidth();
	const int roi_size = (col_end - col_beg + 1);

	if (col_beg > 0) {
		if (roi_size > SLOPE_SIZE) {
			int dsum = 0;
			int dcnt = 0;
			for (int i = col_beg + 1; i < (col_beg + SLOPE_SIZE); i++) {
				dsum += (minPath[i-1] - minPath[i]);
				dcnt += 1;
			}
			float rate = ((float)dsum / (float)dcnt);
			for (int i = col_beg - 1, k = 1; i >= 0; i--, k++) {
				int y = (int)(minPath[col_beg] + rate * k);
				minPath[i] = y;
			}
			/*
			auto x_dat = std::vector<double>();
			auto y_dat = std::vector<double>();
			for (int i = col_beg + 1; i < (col_beg + SLOPE_SIZE); i++) {
				x_dat.push_back((double)i);
				y_dat.push_back((double)minPath[i]);
			}
			auto x_inp = std::vector<double>();
			auto y_inp = std::vector<double>();
			for (int i = 0; i < col_beg; i++) {
				x_inp.push_back((double)i);
			}
			CubicSpline spline;
			spline.setPoints(x_dat, y_dat);
			y_inp = spline.interpolate(x_inp);
			for (int i = 0; i < col_beg; i++) {
				minPath[i] = (int)y_inp[i];
			}
			*/
		}
		else {
			for (int i = 0; i < col_beg; i++) {
				minPath[i] = minPath[col_beg];
			}
		}
	}

	if (col_end < (matCost.cols - 1)) {
		if (roi_size > SLOPE_SIZE) {
			int dsum = 0;
			int dcnt = 0;
			for (int i = col_end - 1; i > (col_end - SLOPE_SIZE); i--) {
				dsum += (minPath[i+1] - minPath[i]);
				dcnt += 1;
			}
			float rate = ((float)dsum / (float)dcnt);
			for (int i = col_end + 1, k = 1; i < matCost.cols; i++, k++) {
				int y = (int)(minPath[col_end] + rate * k);
				minPath[i] = y;
			}
		}
		else {
			for (int i = col_end + 1; i < matCost.cols; i++) {
				minPath[i] = minPath[col_end];
			}
		}
	}

	impl().minPath = minPath;
	return true;
}


bool SemtRetina::RetinaBoundary::resizeBoundaryPath(std::vector<int> path, int src_w, int src_h, int targ_w, int targ_h, std::vector<int>& outs)
{
	if (path.size() != src_w || path.empty()) {
		return false;
	}

	if ((src_w == targ_w) && (src_h == targ_h)) {
		outs = path;
	}
	else {
		float horz_rate = (float)src_w / (float)targ_w;
		float vert_rate = (float)targ_h / (float)src_h;

		outs = std::vector<int>(targ_w, -1);
		for (int i = 0; i < targ_w; i++) {
			auto idx = (int)(i * horz_rate);
			auto val = path[idx];
			if (val >= 0) {
				auto dst = (int)(val * vert_rate);
				outs[i] = dst;
			}
		}
	}
	return true;
}

std::vector<int> SemtRetina::RetinaBoundary::smoothOptimalPath(int filt_size, int degree, bool nerve_head)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2) && nerve_head) 
	{
		auto l_marg = std::vector<int>();
		auto r_marg = std::vector<int>();
		if (band->isRetinaOnNerveHeadMarginLeft()) {
			for (auto i = 0; i < head_x1; i++) {
				l_marg.push_back(path[i]);
			}
			if (l_marg.size() > (filt_size * 2 + 1)) {
				auto filt = CppUtil::SgFilter::smoothInts(l_marg, filt_size, degree);
				l_marg = filt;
			}
			for (auto i = 0, k = 0; i < head_x1; i++, k++) {
				path[i] = l_marg[k];
			}
		}
		if (band->isRetinaOnNerveHeadMarginRight()) {
			for (auto i = head_x2 + 1; i < (width - 1); i++) {
				r_marg.push_back(path[i]);
			}
			if (r_marg.size() > (filt_size * 2 + 1)) {
				auto filt = CppUtil::SgFilter::smoothInts(r_marg, filt_size, degree);
				r_marg = filt;
			}
			for (auto i = (head_x2 + 1), k = 0; i < (width - 1); i++, k++) {
				path[i] = r_marg[k];
			}
		}
		if (band->isRetinaOnNerveHeadMarginBoth()) {
			auto y1 = l_marg.back();
			auto x1 = head_x1 - 1;
			auto y2 = r_marg.front();
			auto x2 = head_x2 + 1;
			auto slope = (float)(y2 - y1) / (float)(x2 - x1);
			for (auto i = (x1 + 1), k = 1; i <= (x2 - 1); i++, k++) {
				path[i] = (int)(y1 + k * slope);
			}
		}
		else if (band->isRetinaOnNerveHeadMarginLeft()) {
			for (auto i = head_x1; i < width; i++) {
				path[i] = l_marg.back();
			}
		}
		else if (band->isRetinaOnNerveHeadMarginRight()) {
			for (auto i = head_x2; i >= 0; i--) {
				path[i] = r_marg.front();
			}
		}
	}
	else {
		auto filt = CppUtil::SgFilter::smoothInts(path, filt_size, degree);
		path = filt;
	}
	return path;
}

std::vector<int>& SemtRetina::RetinaBoundary::upperYs(void) const
{
	return impl().uppers;
}

std::vector<int>& SemtRetina::RetinaBoundary::lowerYs(void) const
{
	return impl().lowers;
}

std::vector<int>& SemtRetina::RetinaBoundary::deltaYs(void) const
{
	return impl().deltas;
}

std::vector<int>& SemtRetina::RetinaBoundary::optimalPath(void) const
{
	return impl().minPath;
}

cv::Mat& SemtRetina::RetinaBoundary::pathEdgeMat(void) const
{
	return impl().matEdge;
}

cv::Mat& SemtRetina::RetinaBoundary::pathProbMat(void) const
{
	return impl().matProb;
}

cv::Mat& SemtRetina::RetinaBoundary::pathCostMat(void) const
{
	return impl().matCost;
}


RetinaBoundary::RetinaBoundaryImpl& SemtRetina::RetinaBoundary::impl(void) const
{
	return *d_ptr;
}

