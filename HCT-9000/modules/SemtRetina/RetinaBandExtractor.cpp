#include "stdafx.h"
#include "RetinaBandExtractor.h"
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


struct RetinaBandExtractor::RetinaBandExtractorImpl
{
	const RetinaSegmenter* segm = nullptr;

	int retinaBeginX;
	int retinaEndX;
	int retinaSpanX;
	int retinaSpanY;

	int opticDiscMinX;
	int opticDiscMaxX;
	bool isDiscCupShaped;

	vector<int> coordXs;
	vector<int> innerYs;
	vector<int> outerYs;
	vector<int> innerYsFull;
	vector<int> outerYsFull;

	RetinaBandExtractorImpl()
	{
		initializeRetinaBandExtractorImpl();
	}

	void initializeRetinaBandExtractorImpl()
	{
		retinaBeginX = -1;
		retinaEndX = -1;
		retinaSpanX = 0;
		retinaSpanY = 0;

		opticDiscMinX = -1;
		opticDiscMaxX = -1;
		isDiscCupShaped = false;

		coordXs.clear();
		innerYs.clear();
		outerYs.clear();
		innerYsFull.clear();
		outerYsFull.clear();
	}
};


RetinaBandExtractor::RetinaBandExtractor(RetinaSegmenter* segm) :
	d_ptr(make_unique<RetinaBandExtractorImpl>())
{
	impl().segm = segm;
}


SemtRetina::RetinaBandExtractor::~RetinaBandExtractor() = default;
SemtRetina::RetinaBandExtractor::RetinaBandExtractor(RetinaBandExtractor&& rhs) = default;
RetinaBandExtractor& SemtRetina::RetinaBandExtractor::operator=(RetinaBandExtractor&& rhs) = default;


bool SemtRetina::RetinaBandExtractor::estimateHorizontalBounds(void)
{
	auto* segm = impl().segm;
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();

	auto* image = resa->imageCoarse();
	auto img_w = image->getWidth();
	auto img_h = image->getHeight();

	auto snrs = image->columnSnRatios();
	auto size = (int)snrs.size();

	const float OBJ_SNR_MIN = COLUMN_OBJ_SN_RATIO_MIN;
	const int WINDOW_SIZE = crta->getColumnSnRatiosSmoothWindow();
	const int DEGREE = 1;
	auto filts = CppUtil::SgFilter::smoothFloats(snrs, WINDOW_SIZE, DEGREE);

	int x1 = -1;
	int x2 = -1;
	for (int i = 0; i < size; i++) {
		if (filts[i] >= OBJ_SNR_MIN) {
			x1 = i;
			break;
		}
	}
	for (int i = size - 1; i > x1; i--) {
		if (filts[i] >= OBJ_SNR_MIN) {
			x2 = i;
			break;
		}
	}

	int width = x2 - x1 + 1;
	int height = img_h;
	if (width <= 0) {
		return false;
	}

	vector<int> xs;
	for (int x = x1; x <= x2; x++) {
		xs.push_back(x);
	}

	if (img_w != width) {
		LogD() << "Sampling index: " << resa->sampleIndex() << ", retina x1: " << x1 << ", x2: " << x2 << ", size: " << xs.size() << ", img width: " << img_w;
	}

	impl().retinaBeginX = x1;
	impl().retinaEndX = x2;
	impl().retinaSpanX = width;
	impl().retinaSpanY = height;
	impl().coordXs = xs;
	return true;
}


bool SemtRetina::RetinaBandExtractor::detectInnerRetinaBoundary(void)
{
	auto* segm = impl().segm;
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto* pipe = segm->retinaInferPipeline();
	auto* indice = pipe->classIndices();
	
	auto x_beg = impl().retinaBeginX;
	auto x_end = impl().retinaEndX;
	auto x_span = impl().retinaSpanX;
	auto y_span = impl().retinaSpanY;

	auto width = image->getWidth();
	auto height = image->getHeight();

	const int SEARCH_BAND = crta->getVitreousSizeToTriggerMidpoint();

	vector<int> y_locs;
	for (int x = x_beg; x <= x_end; x++) {
		vector<int> vits;
		for (int y = 0; y < y_span; y++) {
			auto val = indice[y * width + x];
			if (val == CLASS_VITREOUS) {
				vits.push_back(y);
			}
		}

		int row_start = 0;
		int vits_size = static_cast<int>(vits.size());
		if (vits_size > SEARCH_BAND) {
			row_start = vits[vits_size / 2];

			// Search downward from row_start
			for (int y = row_start; y < height; ++y) {
				auto val = indice[y * width + x];
				if (val != CLASS_VITREOUS) {
					row_start = y;
					break;
				}
			}

			// Count values below threshold before row_start
			int not_count = 0;
			for (int y = 0; y < row_start; ++y) {
				auto val = indice[y * width + x];
				if (val != CLASS_VITREOUS) {
					not_count++;
				}
			}

			if (not_count > (vits_size / 2)) {
				row_start = 0;
			}
		}
		else {
			row_start = 0;
		}

		if (row_start == 0) {
			// Find first row where value < 0.5
			for (int y = 0; y < height; ++y) {
				auto val = indice[y * width + x];
				if (val != CLASS_VITREOUS) {
					row_start = y;
					break;
				}
			}
		}
		y_locs.push_back(row_start);
	}

	const int INNER_MARGIN = crta->getUpwardOffsetToInnerBound();
	for (int i = 0; i < static_cast<int>(y_locs.size()); ++i) {
		y_locs[i] = std::max(0, y_locs[i] - INNER_MARGIN);
	}

	const int WINDOW_SIZE = crta->getSmoothWindowToInnerBound();
	const int DEGREE = 1;
	auto filts = CppUtil::SgFilter::smoothInts(y_locs, WINDOW_SIZE, DEGREE);

	for (int i = 0; i < static_cast<int>(filts.size()); ++i) {
		y_locs[i] = std::max(0, std::min(filts[i], height-1));
	}

	auto y_exts = vector<int>(width, -1);
	for (int i = 0; i < x_beg; i++) {
		y_exts[i] = y_locs[0];
	}
	for (int i = x_beg, k = 0; i <= x_end; i++, k++) {
		y_exts[i] = y_locs[k];
	}
	for (int i = x_end + 1; i < width; i++) {
		y_exts[i] = y_locs.back();
	}

	impl().innerYs = y_locs;
	impl().innerYsFull = y_exts;
	/*
	// Check if the detected inner boundary is out of range. 
	for (int i = 0; i < static_cast<int>(y_exts.size()); ++i) {
		if (y_exts[i] < 0 || y_exts[i] >= height - 1) {
			LogD() << "Sampling index: " << resa->sampleIndex() << ", retina x: " << (x_beg + i) << ", inner y: " << y_locs[i] << ", height: " << height;
			return false;
		}
	}
	*/
	return true;
}


bool SemtRetina::RetinaBandExtractor::detectOuterRetinaBoundary(void)
{
	auto* segm = impl().segm;
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();
	auto img_mat = image->getCvMatConst();

	auto* pipe = segm->retinaInferPipeline();
	auto* indice = pipe->classIndices();

	auto x_beg = impl().retinaBeginX;
	auto x_end = impl().retinaEndX;
	auto x_span = impl().retinaSpanX;
	auto y_span = impl().retinaSpanY;

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto col_means = image->columnMeans();
	auto img_stdev = image->imageStdev();
	
	vector<int> y_inns = impl().innerYs;
	vector<int> y_locs;

	for (int x = x_beg, k = 0; x <= x_end; x++, k++) {
		auto thresh = col_means[x] + img_stdev * 2.0f;

		int row_start = y_inns[k];
		for (int y = row_start; y < height; y++) {
			auto val = indice[y * width + x];
			if (val == CLASS_SCLERA) {
				row_start = y;
				break;
			}
		}

		for (int y = height - 1; y >= row_start; y--) {
			auto val = img_mat.at<uchar>(y, x);
			if (val >= thresh) {
				row_start = y;
				break;
			}
		}
		y_locs.push_back(row_start);
	}

	const int OUTER_MARGIN = crta->getDownwardOffsetToOuterBound();
	for (int i = 0; i < static_cast<int>(y_locs.size()); ++i) {
		y_locs[i] = std::min(height-1, y_locs[i] + OUTER_MARGIN);
	}

	const int WINDOW_SIZE = crta->getSmoothWindowToOuterBound();
	const int DEGREE = 1;
	auto filts = CppUtil::SgFilter::smoothInts(y_locs, WINDOW_SIZE, DEGREE);

	for (int i = 0; i < static_cast<int>(filts.size()); ++i) {
		y_locs[i] = std::max(y_inns[i], std::min(filts[i], height - 1));
	}

	auto y_exts = vector<int>(width, -1);
	for (int i = 0; i < x_beg; i++) {
		y_exts[i] = y_locs[0];
	}
	for (int i = x_beg, k = 0; i <= x_end; i++, k++) {
		y_exts[i] = y_locs[k];
	}
	for (int i = x_end + 1; i < width; i++) {
		y_exts[i] = y_locs.back();
	}

	impl().outerYs = y_locs;
	impl().outerYsFull = y_exts;
	/*
	// Check if the detected inner boundary is out of range. 
	for (int i = 0; i < static_cast<int>(y_exts.size()); ++i) {
		if (y_exts[i] < 0 || y_exts[i] >= height - 1) {
			LogD() << "Sampling index: " << resa->sampleIndex() << ", retina x: " << (x_beg + i) << ", inner y: " << y_locs[i] << ", height: " << height;
			return false;
		}
	}
	*/
	return true;
}


bool SemtRetina::RetinaBandExtractor::detectOpticNerveHeadRegion(void)
{
	auto* segm = impl().segm;
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto* pipe = segm->retinaInferPipeline();
	auto* indice = pipe->classIndices();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto ret_x1 = retinaBeginX();
	auto ret_x2 = retinaEndX();
	auto inns = innerYsFull();
	auto outs = outerYsFull();

	const float PROB_THRESH = 0.5f;
	const int HEAD_WIDTH_MIN = crta->getOpticDiscHeadWidthMin(); 
	const int PERI_WIDTH_MIN = crta->getOpticDiscSideWidthMin(); 
	const int HEAD_DEPTH_MIN = crta->getOpticDiscHeadDepthMin(); 
	const int HEAD_MERGE_MAX = crta->getOpticDiscHeadMergeDist();

	auto xsum = 0;
	auto ysum = 0;
	auto wsum = 0;
	auto dlen = 0;
	auto head_list = std::vector<int>(width, 0);
	auto rnfl_list = std::vector<int>(width, 0);

	for (int x = 0; x < width; x++) {
		if (x < ret_x1 || x > ret_x2) {
			continue;
		}
		auto y1 = inns[x];
		auto y2 = outs[x];
		auto xacc = 0;
		auto yacc = 0;
		auto zcnt = 0;
		auto rcnt = 0;
		for (int y = y1; y <= y2; y++) {
			auto idx = y * width + x;
			auto cls = indice[idx];
			if (cls == CLASS_DISC_HEAD) {
				xacc += x;
				yacc += y;
				zcnt++;
			}
			if (cls == CLASS_RNFL) {
				rcnt++;
			}
			if (cls == CLASS_SCLERA) {
				break;
			}
		}
		if (zcnt >= HEAD_DEPTH_MIN) {
			xsum += xacc;
			ysum += yacc;
			wsum += zcnt;
			head_list[x] = zcnt;
			rnfl_list[x] = rcnt;
			dlen++;
		}
	}

	if (dlen >= HEAD_WIDTH_MIN) {
		auto xcen = (int)(xsum / wsum + 0.5f);
		auto ycen = (int)(ysum / wsum + 0.5f);
		auto disc_x1 = xcen;
		auto disc_x2 = xcen;
		for (int x = xcen, offs = 0; x >= ret_x1; x--) {
			if (head_list[x] >= HEAD_DEPTH_MIN) {
				disc_x1 = x;
				offs = 0;
			}
			else {
				if (++offs >= HEAD_MERGE_MAX) {
					break;
				}
			}
		}
		for (int x = xcen, offs = 0; x <= ret_x2; x++) {
			if (head_list[x] >= HEAD_DEPTH_MIN) {
				disc_x2 = x;
				offs = 0;
			}
			else {
				if (++offs >= HEAD_MERGE_MAX) {
					break;
				}
			}
		}

		auto disc_w = disc_x2 - disc_x1 + 1;
		if (disc_w >= HEAD_WIDTH_MIN) {
			auto rnfl_w = 0;
			for (int x = disc_x1; x <= disc_x2; x++) {
				if (rnfl_list[x] > 0) {
					rnfl_w++;
				}
			}
			if (rnfl_w >= (disc_w * 0.9f)) {
				impl().isDiscCupShaped = false;
			}
			else {
				impl().isDiscCupShaped = true;
			}
			if (disc_x1 < PERI_WIDTH_MIN) {
				disc_x1 = 0;
			}
			if (disc_x2 >= (width - PERI_WIDTH_MIN)) {
				disc_x2 = width - 1;
			}
			setNerveHeadRangeX(disc_x1, disc_x2);
			LogD() << "Optic Nerve Head region detected, x1: " << disc_x1 << ", x2: " << disc_x2 << ", width: " << disc_w << ", cup shaped: " << impl().isDiscCupShaped << ", index: " << resa->sampleIndex();
		}
	}
	return true;
}


bool SemtRetina::RetinaBandExtractor::adjustOpticNerveDiscMargins(void)
{
	auto* segm = impl().segm;
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* pipe = segm->retinaInferPipeline();
	auto* indice = pipe->classIndices();

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();

	if (isNerveHeadRangeValid()) {
		auto disc_x1 = impl().opticDiscMinX;
		auto disc_x2 = impl().opticDiscMaxX;
		auto disc_cx = (disc_x1 + disc_x2) / 2;
		auto new_x1 = disc_x1;
		auto new_x2 = disc_x2;

		for (int x = disc_x1; x < disc_cx; x++) {
			auto y1 = onls[x];
			auto idx1 = y1 * width + x;
			if (indice[idx1] == CLASS_DISC_HEAD) {
				break;
			}

			for (int y = y1; y < height; y++) {
				auto idx = y * width + x;
				if (indice[idx] == CLASS_ONL) {
					continue;
				}
				else {
					if (indice[idx] == CLASS_RPE) {
						new_x1 = x + 1;
					}
					break;
				}
			}
		}

		for (int x = disc_x2; x > disc_cx; x--) {
			auto y1 = onls[x];
			auto idx1 = y1 * width + x;
			if (indice[idx1] == CLASS_DISC_HEAD) {
				break;
			}

			for (int y = y1; y < height; y++) {
				auto idx = y * width + x;
				if (indice[idx] == CLASS_ONL) {
					continue;
				}
				else {
					if (indice[idx] == CLASS_RPE) {
						new_x2 = x - 1;
					}
					break;
				}
			}
		}

		int ilm_y1 = ilms[new_x1];
		int ilm_y2 = ilms[new_x2];
		int ilm_cy = (ilm_y1 + ilm_y2) / 2;
		int CUP_MIN = crta->getOpticDiscCupDepthMin();

		for (int x = new_x1; x <= new_x2; x++) {
			int pass = ilms[x] - ilm_cy;
			if (pass >= CUP_MIN) {
				impl().isDiscCupShaped = true;
				break;
			}
		}

		/*
		for (int x = new_x1; x <= new_x2; x++) {
			if (ilms[x] >= onls[x]) {
				impl().isDiscCupShaped = true;
				break;
			}
		}
		*/
		impl().opticDiscMinX = new_x1;
		impl().opticDiscMaxX = new_x2;
		LogD() << "Optic Nerve Head margins adjusted, old x1: " << disc_x1 << ", x2: " << disc_x2 << ", new x1: " << new_x1 << ", new x2: " << new_x2 << ", cup shaped: " << impl().isDiscCupShaped << ", index: " << resa->sampleIndex();
	}
	return true;
}


void SemtRetina::RetinaBandExtractor::upscaleToSourceDimensions(void)
{
	auto* segm = impl().segm;
	auto* resa = segm->bscanResampler();

	float scaleX = 1.0f / resa->sampleScaleRatioX();
	float scaleY = 1.0f / resa->sampleScaleRatioY();

	auto ret_begx = (int)(impl().retinaBeginX * scaleX + 0.5f);
	auto ret_endx = (int)(impl().retinaEndX * scaleX + 0.5f);
	auto ret_spanx = (int)(impl().retinaSpanX * scaleX + 0.5f);
	auto ret_spany = (int)(impl().retinaSpanY * scaleY + 0.5f);
	auto disc_minx = (int)(impl().opticDiscMinX * scaleX + 0.5f);
	auto disc_maxx = (int)(impl().opticDiscMaxX * scaleX + 0.5f);

	impl().retinaBeginX = ret_begx;
	impl().retinaEndX = ret_endx;
	impl().retinaSpanX = ret_spanx;
	impl().retinaSpanY = ret_spany;
	impl().opticDiscMinX = disc_minx;
	impl().opticDiscMaxX = disc_maxx;
	return;
}


void SemtRetina::RetinaBandExtractor::setNerveHeadRangeX(int x1, int x2) const
{
	impl().opticDiscMinX = x1;
	impl().opticDiscMaxX = x2;
	return;
}

bool SemtRetina::RetinaBandExtractor::getNerveHeadRangeX(int& x1, int& x2) const
{
	x1 = impl().opticDiscMinX;
	x2 = impl().opticDiscMaxX;
	bool flag = isNerveHeadRangeValid();
	return flag;
}

bool SemtRetina::RetinaBandExtractor::isNerveHeadRangeValid(void) const
{
	auto x1 = impl().opticDiscMinX;
	auto x2 = impl().opticDiscMaxX;
	bool flag = (x1 >= 0 && x2 >= 0 && x2 > x1);
	return flag;
}

bool SemtRetina::RetinaBandExtractor::isRetinaOnNerveHeadMarginLeft(void) const
{
	if (isNerveHeadRangeValid()) {
		auto x = impl().opticDiscMinX;
		auto flag = x > 0;
		return flag;
	}
	return false;
}

bool SemtRetina::RetinaBandExtractor::isRetinaOnNerveHeadMarginRight(void) const
{
	if (isNerveHeadRangeValid()) {
		auto x = impl().opticDiscMaxX;
		auto w = impl().retinaSpanX;
		auto flag = x < (w - 1);
		return flag;
	}
	return false;
}

bool SemtRetina::RetinaBandExtractor::isRetinaOnNerveHeadMarginBoth(void) const
{
	auto f1 = isRetinaOnNerveHeadMarginLeft();
	auto f2 = isRetinaOnNerveHeadMarginRight();
	return (f1 && f2);
}

bool SemtRetina::RetinaBandExtractor::isNerveHeadDiscCupShaped(void) const
{
	return impl().isDiscCupShaped;
}

std::vector<int> SemtRetina::RetinaBandExtractor::innerYs(void) const
{
	return impl().innerYs;
}

std::vector<int> SemtRetina::RetinaBandExtractor::outerYs(void) const
{
	return impl().outerYs;
}

std::vector<int> SemtRetina::RetinaBandExtractor::innerYsFull(void) const
{
	return impl().innerYsFull;
}

std::vector<int> SemtRetina::RetinaBandExtractor::outerYsFull(void) const
{
	return impl().outerYsFull;
}

int SemtRetina::RetinaBandExtractor::retinaBeginX(void) const
{
	auto x = impl().retinaBeginX;
	return x;
}

int SemtRetina::RetinaBandExtractor::retinaEndX(void) const
{
	auto x = impl().retinaEndX;
	return x;
}

int SemtRetina::RetinaBandExtractor::opticDiscMinX(void) const
{
	auto x = impl().opticDiscMinX;
	return x;
}

int SemtRetina::RetinaBandExtractor::opticDiscMaxX(void) const
{
	auto x = impl().opticDiscMaxX;
	return x;
}


RetinaBandExtractor::RetinaBandExtractorImpl& SemtRetina::RetinaBandExtractor::impl(void) const
{
	return *d_ptr;
}
