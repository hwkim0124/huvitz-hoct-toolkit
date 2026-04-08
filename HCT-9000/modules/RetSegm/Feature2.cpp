#include "stdafx.h"
#include "Feature2.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "SegmImage.h"
#include "SegmLayer.h"

#include "RetSegm2.h"
#include "CppUtil2.h"

#include <vector>
#include <numeric>
#include <algorithm>

using namespace RetSegm;
using namespace CppUtil;
using namespace cv;
using namespace std;


Feature2::Feature2()
{
}


Feature2::~Feature2()
{
}



bool RetSegm::Feature2::detectBoundaryPointsRegion(const std::vector<int>& inner, const std::vector<int>& outer, float rangeX, int & regX1, int & regX2)
{
	auto size = (int)outer.size();

	const int edgeSizeMin = (int)((size / rangeX) * 0.25f);
	const int emptSizeMax = (int)((size / rangeX) * 0.25f);

	int empty = 0, count = 0, posit = -1;

	for (int i = 0; i < size; i++) {
		if (inner[i] >= 0 || outer[i] >= 0) {
			posit = (posit < 0 ? i : posit);
			empty = 0;
			if (++count >= edgeSizeMin) {
				break;
			}
		}
		else {
			if (++empty >= emptSizeMax) {
				posit = -1; count = 0;
			}
		}
	}

	regX1 = posit;
	empty = 0, count = 0, posit = -1;

	for (int i = size - 1; i >= 0; i--) {
		if (inner[i] >= 0 || outer[i] >= 0) {
			posit = (posit < 0 ? i : posit);
			empty = 0;
			if (++count >= edgeSizeMin) {
				break;
			}
		}
		else {
			if (++empty >= emptSizeMax) {
				posit = -1; count = 0;
			}
		}
	}

	regX2 = posit;

	return (regX1 >= 0 && regX2 >= 0);
}


bool RetSegm::Feature2::detectOpticDiscRegion(const std::vector<int>& outer, float rangeX, int & discX1, int & discX2)
{
	auto size = outer.size();
	auto input = outer;

	const int holeSizeMin = (int)((size / rangeX) * 0.35f);
	const int discSizeMin = (int)((size / rangeX) * 0.85f);
	const int edgeSizeMin = (int)((size / rangeX) * 0.25f);
	const int emptSizeMax = (int)((size / rangeX) * 0.25f);

	int pts_x1 = -1, pts_x2 = -1;
	for (int i = 0; i < size; i++) {
		if (input[i] >= 0) {
			pts_x1 = (pts_x1 < 0 ? i : pts_x1);
			pts_x2 = i;
		}
	}

	auto holes = vector<int>(size, 0);
	for (int i = pts_x1, cnt = 0; i <= pts_x2; i++) {
		if (input[i] < 0) {
			holes[i] = ++cnt;
		}
		else {
			cnt = 0;
		}
	}

	auto hmax = max_element(holes.begin(), holes.end());
	if (*hmax >= holeSizeMin) {
		int center = (int)(hmax - holes.begin());
		bool edge1 = false, edge2 = false;
		int end_y1 = -1, end_y2 = -1;

		for (int k = center, count = 0, empty = 0; k >= pts_x1; k--) {
			if (input[k] < 0) {
				if (count > 0) {
					if (++empty > emptSizeMax) {
						count = 0; discX1 = k;
					}
				}
				else {
					discX1 = k;
				}
			}
			else {
				if (count > 0 && abs(end_y1 - input[k]) > 9) {
					count = 0;
				}
				end_y1 = input[k];
				empty = 0;
				if (++count >= edgeSizeMin) {
edge1 = true;
break;
				}
			}
		}
		for (int k = center, count = 0, empty = 0; k <= pts_x2; k++) {
			if (input[k] < 0) {
				if (count > 0) {
					if (++empty > emptSizeMax) {
						count = 0; discX2 = k;
					}
				}
				else {
					discX2 = k;
				}
			}
			else {
				if (count > 0 && abs(end_y2 - input[k]) > 9) {
					count = 0;
				}
				end_y2 = input[k];
				empty = 0;
				if (++count >= edgeSizeMin) {
					edge2 = true;
					break;
				}
			}
		}

		if ((discX2 - discX1) > discSizeMin) {
			discX1 = max(discX1, pts_x1);
			discX2 = min(discX2, pts_x2);
			return true;
		}
	}

	discX1 = discX2 = -1;
	return false;
}


bool RetSegm::Feature2::estimateOpticDiscMargin(const SegmImage * ascImg, const std::vector<int>& inner, std::vector<int>& outer, float rangeX, int & discX1, int & discX2)
{
	Mat ascMat = ascImg->getCvMat();
	auto size = (int) outer.size();

	const int kPixelsPerMM = (int)((size / rangeX));
	const int holeSizeMin = (int)((size / rangeX) * 0.55f);
	const int discSizeMin = (int)((size / rangeX) * 0.75f);
	const int edgeSizeMin = (int)((size / rangeX) * 0.50f);
	const int emptSizeMax = 3; // (int)((size / rangeX) * 0.25f);

	int edgeThresh = (int)(ascImg->getMean() * 0.0f + ascImg->getStddev() * 1.0f);
	edgeThresh = max(edgeThresh, 15);

	auto inn_curv = inner;
	auto out_curv = outer;
	DataFitt::interpolateBoundaryByLinearFitting(inner, inn_curv, true);
	DataFitt::smoothBoundaryLine(inn_curv, inn_curv, 0.05f);
	DataFitt::interpolateBoundaryByLinearFitting(outer, out_curv, true);
	DataFitt::smoothBoundaryLine(out_curv, out_curv, 0.08f);

	auto depth = vector<int>(size, 0);
	std::transform(cbegin(inn_curv), cend(inn_curv), cbegin(out_curv),
		begin(depth), [](int e1, int e2) {
		return e2 - e1;
	});

	// Assumes that optic nerve head can be identified by the fallen inner boundary 
	// below retinal pigmented epithelium.  
	int inn_base_pos = (int)(max_element(cbegin(inn_curv), cend(inn_curv)) - cbegin(inn_curv));
	int inn_side_pos1 = max(inn_base_pos - kPixelsPerMM, 0);
	int inn_side_pos2 = min(inn_base_pos + kPixelsPerMM, size - 1);
	int disc_inn1 = -1;
	int disc_inn2 = -1;

	auto sunk = count_if(cbegin(depth) + inn_side_pos1, cbegin(depth) + inn_side_pos2, [](int e) { return e < 0; });
	if (sunk > 0) {
		int avg_depth = (int)accumulate(cbegin(depth), cend(depth), 0.0f) / size;
		int d_thresh = (int)(avg_depth * 0.75f);

		int disc_out1 = inn_side_pos1;
		int disc_out2 = inn_side_pos2;
		disc_inn1 = disc_out1;
		disc_inn2 = disc_out2;

		auto iter1 = std::find_if(crbegin(depth) + (size - inn_base_pos - 1), crbegin(depth) + (size - disc_out1 - 1),
			[=](int e) { return e > d_thresh; });
		if (iter1 != crend(depth)) {
			disc_inn1 = (int)distance(iter1, crend(depth)) - 1;
		}

		auto iter2 = std::find_if(cbegin(depth) + inn_base_pos, cbegin(depth) + disc_out2,
			[=](int e) { return e > d_thresh; });
		if (iter2 != cend(depth)) {
			disc_inn2 = (int)distance(cbegin(depth), iter2);
		}

		fill_n(begin(outer) + disc_inn1, (disc_inn2 - disc_inn1 + 1), 0);
	}


	transform(outer.begin(), outer.end(), inn_curv.begin(), outer.begin(), [](int elem1, int elem2) { 
		return (elem1 <= elem2 ? -1 : elem1); 
	});

	int retiX1, retiX2;
	if (!detectBoundaryPointsRegion(inner, outer, rangeX, retiX1, retiX2)) {
		return false;
	}

	auto holes = vector<int>(size, 0);
	int width = 0;
	transform(holes.begin() + retiX1, holes.begin() + retiX2, outer.begin() + retiX1, holes.begin() + retiX1, [&](int elem1, int elem2) {
		width = (elem2 < 0 ? width + 1 : 0);
		return width;
	});

	auto hmax = max_element(holes.begin(), holes.end());
	width = *hmax;
	if (width < holeSizeMin) {
		return false;
	}

	int center = (int)(hmax - holes.begin());
	int end_x1 = -1, end_x2 = -1;
	int end_y1 = -1, end_y2 = -1;
	int disc_x1 = -1, disc_x2 = -1;

	for (int i = center; i >= retiX1; i--) {
		if (outer[i] >= 0) {
			end_x1 = i; end_y1 = outer[i];
			break;
		}
		disc_x1 = i;
	}
	for (int i = center; i <= retiX2; i++) {
		if (outer[i] >= 0) {
			end_x2 = i; end_y2 = outer[i];
			break;
		}
		disc_x2 = i;
	}

	disc_x1 = (disc_x1 < 0 ? 0 : disc_x1);
	disc_x2 = (disc_x2 >= size ? size - 1 : disc_x2);
	center = (disc_x1 + disc_x2) / 2;

	int thresh1 = 0, thresh2 = 0;
	int gsum = 0, gcnt = 0;
	int bound1 = retiX1; // (discX1 - (discX1 - retiX1) / 3);
	int bound2 = retiX2; // (discX2 + (retiX2 - discX2) / 3);

	for (int c = disc_x1; c >= bound1; c--) {
		if (outer[c] >= 0) {
			int r = outer[c];
			gsum += ascMat.at<uchar>(r, c);
			gcnt += 1;
		}
	}
	thresh1 = (gcnt > 0 ? (int)((gsum / gcnt) * 0.55f) : 0);
	thresh1 = max(thresh1, edgeThresh);

	gsum = 0, gcnt = 0;
	for (int c = disc_x2; c <= bound2; c++) {
		if (outer[c] >= 0) {
			int r = outer[c];
			gsum += ascMat.at<uchar>(r, c);
			gcnt += 1;
		}
	}
	thresh2 = (gcnt > 0 ? (int)((gsum / gcnt) * 0.55f) : 0);
	thresh2 = max(thresh2, edgeThresh);

	int count = 0, empty = 0;
	int end_y = 0, end_x = 0, beg_x = 0;
	bool found = false;
	width = 0;
	for (int c = (disc_x1 - 1); c >= retiX1; c--) {
		if (outer[c] >= 0) {
			int r = outer[c];
			int curVal = ascMat.at<uchar>(r, c);
			if (curVal >= thresh1) {
				if (count > 0) {
					if (abs(end_y - outer[c]) > 5) {
						count = 0;
					}
				}
				width++;
				if (++count >= edgeSizeMin) {
					found = true;
					break;
				}
				beg_x = (width == 1 ? c : beg_x);
				end_x = (count == 1 ? c : end_x);
				end_y = outer[c];
				empty = 0;
			}
		}
		else {
			if (++empty >= emptSizeMax) {
				count = 0;
			}
		}
	}

	if (!found && width > 0) {
		found = true;
		end_x = beg_x;
	}

	if (!found) {
		count = 0, empty = 0;
		for (int c = retiX1; c < center; c++) {
			if (inner[c] >= 0) {
				int r = inner[c];
				int y1 = min((count > 0 ? max(end_y - 3, inner[c] + 1) : r + 25), ascMat.rows - 1);
				int y2 = min((count > 0 ? end_y + 3 : r + 125), ascMat.rows - 1);
				int curVal = 0, maxVal = 0, maxIdx = -1;

				for (int k = y1; k <= y2; k++) {
					curVal = ascMat.at<uchar>(k, c);
					if (curVal > maxVal) {
						maxVal = curVal;
						maxIdx = k;
					}
				}

				int thresh = edgeThresh; 
				if (maxVal >= edgeThresh) {
					outer[c] = maxIdx;
					end_x = c;
					end_y = maxIdx;
					count++;
					empty = 0;
				}
				else {
					if (++empty >= emptSizeMax * 2) {
						break;
					}
				}
			}
			else {
				if (++empty >= emptSizeMax * 2) {
					break;
				}
			}
		}
		
		if (count == 0) {
			disc_x1 = retiX1;
		}
		else {
			disc_x1 = end_x + 1;
		}
	}
	else {
		for (int c = end_x + 1; c < center; c++) {
			outer[c] = -1;
		}

		disc_inn1 = (disc_inn1 < 0 ? center : disc_inn1);

		empty = 0;
		for (int c = end_x; c < disc_inn1; c++) {
			int r = outer[c];
			int y1 = max(r - 5, 0);
			int y2 = min(r + 5, ascMat.rows - 1);
			int curVal = 0, maxVal = 0, maxIdx = -1;
			int next_c = c + 1;

			for (int k = y1; k <= y2; k++) {
				curVal = ascMat.at<uchar>(k, next_c);
				if (curVal > maxVal) {
					maxVal = curVal;
					maxIdx = k;
				}
			}

			if (maxIdx <= inn_curv[next_c]) {
				break;
			}

			int thresh = max(edgeThresh, thresh1);
			if (maxVal >= thresh) {
				outer[next_c] = maxIdx;
				end_x = next_c;
				empty = 0;
			}
			else {
				if (++empty > emptSizeMax) {
					break;
				}
			}
		}
		disc_x1 = min(end_x + 1, ascMat.cols-1);
	}

	count = 0, empty = 0;
	end_y = 0, end_x = 0, beg_x = 0;
	found = false;
	width = 0;
	for (int c = (disc_x2 + 1); c <= retiX2; c++) {
		if (outer[c] >= 0) {
			int r = outer[c];
			int curVal = ascMat.at<uchar>(r, c);
			if (curVal >= thresh2) {
				if (count > 0) {
					if (abs(end_y - outer[c]) > 5) {
						count = 0;
					}
				}
				width++;
				if (++count >= edgeSizeMin) {
					found = true;
					break;
				}
				beg_x = (width == 1 ? c : beg_x);
				end_x = (count == 1 ? c : end_x);
				end_y = outer[c];
				empty = 0;
			}
		}
		else {
			if (++empty >= emptSizeMax) {
				count = 0;
			}
		}
	}

	if (!found && width > 0) {
		found = true;
		end_x = beg_x;
	}

	if (!found) {
		count = 0, empty = 0;
		for (int c = retiX2; c > center; c--) {
			if (inner[c] >= 0) {
				int r = inner[c];
				int y1 = min((count > 0 ? max(end_y - 3, inner[c] + 1) : r + 25), ascMat.rows - 1);
				int y2 = min((count > 0 ? end_y + 3 : r + 125), ascMat.rows - 1);
				int curVal = 0, maxVal = 0, maxIdx = -1;

				for (int k = y1; k <= y2; k++) {
					curVal = ascMat.at<uchar>(k, c);
					if (curVal > maxVal) {
						maxVal = curVal;
						maxIdx = k;
					}
				}

				int thresh = edgeThresh;
				if (maxVal >= edgeThresh) {
					outer[c] = maxIdx;
					end_x = c;
					end_y = maxIdx;
					count++;
					empty = 0;
				}
				else {
					if (++empty >= emptSizeMax * 2) {
						break;
					}
				}
			}
			else {
				if (++empty >= emptSizeMax * 2) {
					break;
				}
			}
		}

		if (count == 0) {
			disc_x2 = retiX2;
		}
		else {
			disc_x2 = max(end_x - 1, 0);
		}
	}
	else {
		for (int c = end_x - 1; c > center; c--) {
			outer[c] = -1;
		}

		disc_inn2 = (disc_inn2 < 0 ? center : disc_inn2);

		empty = 0;
		for (int c = end_x; c > disc_inn2; c--) {
			int r = outer[c];
			int y1 = max(r - 5, 0);
			int y2 = min(r + 5, ascMat.rows - 1);
			int curVal = 0, maxVal = 0, maxIdx = -1;
			int next_c = c - 1;

			for (int k = y1; k <= y2; k++) {
				curVal = ascMat.at<uchar>(k, next_c);
				if (curVal > maxVal) {
					maxVal = curVal;
					maxIdx = k;
				}
			}

			if (maxIdx <= inn_curv[next_c]) {
				break;
			}

			int thresh = max(edgeThresh, thresh2);
			if (maxVal >= thresh) {
				outer[next_c] = maxIdx;
				end_x = next_c;
				empty = 0;
			}
			else {
				if (++empty > emptSizeMax) {
					break;
				}
			}
		}
		disc_x2 = end_x - 1;
	}

	// LogD() << "Disc range: " << disc_x1 << ", " << disc_x2;

	if ((disc_x2 - disc_x1) < discSizeMin) {
		discX1 = discX2 = -1;
	}
	else {
		discX1 = disc_x1;
		discX2 = disc_x2;
	}
	return true;
}


bool RetSegm::Feature2::estimateOpticDiscBounds(const SegmImage * ascImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX, int & discX1, int & discX2)
{
	// Gradients map. 
	Mat ascMat = ascImg->getCvMat();
	auto size = (int)outer.size();

	const int kPixelsPerMM = (int)((size / rangeX));
	const int kHoleSizeMin = (int)((size / rangeX) * 0.55f);
	const int kDiscSizeMin = (int)((size / rangeX) * 0.75f);
	const int kEdgeSizeMin = (int)((size / rangeX) * 0.50f);
	const int kEmptySizeMax = 3; // (int)((size / rangeX) * 0.25f);
	
	auto inn_curv = inner;
	auto out_curv = outer;
	DataFitt::interpolateBoundaryByLinearFitting(inner, inn_curv, true);
	DataFitt::smoothBoundaryLine(inn_curv, inn_curv, 0.05f);
	DataFitt::interpolateBoundaryByLinearFitting(outer, out_curv, true);
	DataFitt::smoothBoundaryLine(out_curv, out_curv, 0.08f);

	auto depth = vector<int>(size, 0);
	std::transform(cbegin(inn_curv), cend(inn_curv), cbegin(out_curv), 
		begin(depth), [](int e1, int e2) {
		return e2 - e1;
	});

	// Assumes that optic nerve head can be identified by the fallen inner boundary 
	// below retinal pigmented epithelium.  
	int inn_base_pos = (int)(max_element(cbegin(inn_curv), cend(inn_curv)) - cbegin(inn_curv));
	int inn_side_pos1 = max(inn_base_pos - kPixelsPerMM, 0);
	int inn_side_pos2 = min(inn_base_pos + kPixelsPerMM, size - 1);

	auto sunk = count_if(cbegin(depth)+inn_side_pos1, cbegin(depth)+inn_side_pos2, [](int e) { return e < 0; });
	if (sunk > 0) {
		int avg_depth = (int)accumulate(cbegin(depth), cend(depth), 0.0f) / size;
		int d_thresh = (int)(avg_depth * 0.75f);

		int disc_out1 = inn_side_pos1;
		int disc_out2 = inn_side_pos2;
		int disc_inn1 = disc_out1;
		int disc_inn2 = disc_out2;

		auto iter1 = std::find_if(crbegin(depth) + (size - inn_base_pos - 1), crbegin(depth) + (size - disc_out1 - 1),
			[=](int e) { return e > d_thresh; });
		if (iter1 != crend(depth)) {
			disc_inn1 = (int) distance(iter1, crend(depth)) - 1;
		}

		auto iter2 = std::find_if(cbegin(depth) + inn_base_pos, cbegin(depth) + disc_out2,
			[=](int e) { return e > d_thresh; });
		if (iter2 != cend(depth)) {
			disc_inn2 = (int) distance(cbegin(depth), iter2);
		}

		fill_n(begin(outer) + disc_inn1, (disc_inn2 - disc_inn1 + 1), +1);
	}

	// Assign horizontal span in which retina layers can be discerned.
	int reti_x1 = -1, reti_x2 = -1;
	{
		auto iter1 = find_if(cbegin(outer), cend(outer), [](int e) { return e >= 0; });
		if (iter1 != cend(outer)) {
			reti_x1 = (int)distance(cbegin(outer), iter1);
		}
		auto iter2 = find_if(crbegin(outer), crend(outer), [](int e) { return e >= 0; });
		if (iter2 != crend(outer)) {
			reti_x2 = (int)distance(iter2, crend(outer)) - 1;
		}
		if (reti_x1 < 0 || reti_x2 < 0) {
			return false;
		}
	}

	auto holes = vector<int>(size, 0);
	int width = 0;
	transform(holes.begin() + reti_x1, holes.begin() + reti_x2, outer.begin() + reti_x1, holes.begin() + reti_x1, [&](int elem1, int elem2) {
		width = (elem2 < 0 ? width + 1 : 0);
		return width;
	});

	auto hmax_iter = max_element(holes.begin(), holes.end());
	auto hmax_pos = (int)(hmax_iter - holes.begin());
	width = *hmax_iter;
	if (width < kHoleSizeMin) {
		return false;
	}

	int disc_cent = hmax_pos - width / 2;
	int disc_pos1 = -1, disc_pos2 = -1;
	{
		auto iter1 = find_if(crbegin(outer) + (size - disc_cent - 1), crend(outer), [](int e) { return e >= 0; });
		if (iter1 != crend(outer)) {
			disc_pos1 = (int)distance(iter1, crend(outer)) - 1;
		}
		auto iter2 = find_if(cbegin(outer) + disc_cent, cend(outer), [](int e) { return e >= 0; });
		if (iter2 != cend(outer)) {
			disc_pos2 = (int)distance(cbegin(outer), iter2);
		}

		fill_n(begin(outer) + disc_pos1, (disc_pos2 - disc_pos1 + 1), +5);

		discX1 = disc_pos1;
		discX2 = disc_pos2;
	}

	// inner = inn_curv;
	// outer = out_curv;
	return true;
}


bool RetSegm::Feature2::adjustOpticDiscBounds(const std::vector<int>& inner, std::vector<int>& outer, float rangeX, int & discX1, int & discX2)
{
	auto size = (int)outer.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	const int edgeSizeMin = (int)((size / rangeX) * 0.50f);
	const int emptSizeMax = (int)((size / rangeX) * 0.25f);

	if (discX1 < 0 || discX2 < 0) {
		return false;
	}

	auto inn_curv = inner;
	DataFitt::interpolateBoundaryByLinearFitting(inner, inn_curv, true);
	DataFitt::smoothBoundaryLine(inn_curv, inn_curv, 0.05f);

	int retiX1, retiX2;
	if (!detectBoundaryPointsRegion(inner, outer, rangeX, retiX1, retiX2)) {
		return false;
	}

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = outer[i];
		if (i < retiX1 || i > discX1) {
			dataY[i] = -1;
		}
	}

	bool correct = false;
	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		int sideX1 = discX1 - (discX1 - retiX1) / 3;
		int sideX2 = discX1;
		for (int i = sideX1; i < sideX2; i++) {
			if ((outer[i] - fittY[i]) > 15) {
				outer[i] = -1;
				correct = true;
			}
		}
	}

	if (correct) {
		int count = 0;
		int end_y = 0, end_x = 0;
		for (int i = discX1; i >= retiX1; i--) {
			if (outer[i] >= 0) {
				if (count == 0) {
					end_y = outer[i];
					end_x = i;
				}
				else {
					if (abs(end_y - outer[i]) > 3) {
						end_y = outer[i];
						end_x = i;
						count = 0;
					}
				}
				if (++count >= edgeSizeMin) {
					break;
				}
			}
			else {
				count = 0;
			}
		}
		discX1 = (count > 0 ? min(end_x + 1, size - 1) : retiX1);
	}

	dataX = vector<int>(size, -1);
	dataY = vector<int>(size, -1);
	fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = outer[i];
		if (i < discX2 || i > retiX2) {
			dataY[i] = -1;
		}
	}

	correct = false;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		int sideX1 = discX2;
		int sideX2 = discX2 + (retiX2 - discX2) / 3;
		for (int i = sideX1; i < sideX2; i++) {
			if ((outer[i] - fittY[i]) > 15) {
				outer[i] = -1;
				correct = true;
			}
		}
	}

	if (correct) {
		int count = 0;
		int end_y = 0, end_x = 0;
		for (int i = discX2; i <= retiX2; i++) {
			if (outer[i] >= 0) {
				if (count == 0) {
					end_y = outer[i];
					end_x = i;
				}
				else {
					if (abs(end_y - outer[i]) > 3) {
						end_y = outer[i];
						end_x = i;
						count = 0;
					}
				}
				if (++count >= edgeSizeMin) {
					break;
				}
			}
			else {
				count = 0;
			}
		}
		discX2 = (count > 0 ? max(end_x - 1, 0) : retiX2);
	}

	for (int i = discX1; i <= discX2; i++) {
		outer[i] = -1;
	}
	return true;
}


bool RetSegm::Feature2::makeupIdealFittingBoundary(const std::vector<int>& line, std::vector<int>& ideal, float fittSize)
{
	auto outs = line;

	DataFitt::interpolateBoundaryByLinearFitting(line, outs, true);
	DataFitt::smoothBoundaryLine(outs, outs, fittSize);
	ideal = outs;
	return true;
}


bool RetSegm::Feature2::makeupIdealOuterBoundary(const std::vector<int>& inner, const std::vector<int>& outer, float rangeX, int discX1, int discX2, std::vector<int>& ideal)
{
	auto size = outer.size();
	auto outs = outer; 

	int retiX1, retiX2;
	if (!detectBoundaryPointsRegion(inner, outer, rangeX, retiX1, retiX2)) {
		return false;
	}

	bool isDisc = (discX2 - discX1) > 1;
	if (isDisc) {
		auto out_fitt = outer;
		DataFitt::interpolateBoundaryByLinearFitting(outer, out_fitt, false);
		copy(out_fitt.begin() + discX1, out_fitt.begin() + discX2, outs.begin() + discX1);
	}

	int center = (retiX2 + retiX1) / 2;
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);
	
	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i >= 0 && i <= center) {
			dataY[i] = outs[i];
		}
	}

	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		copy(fittY.begin(), fittY.begin() + retiX1, outs.begin());
	}

	dataY = vector<int>(size, -1);
	fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i >= center && i < size) {
			dataY[i] = outs[i];
		}
	}

	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		copy(fittY.begin()+retiX2, fittY.end(), outs.begin()+retiX2);
	}

	auto out_fitt = outs;
	auto out_curv = outs;
	DataFitt::interpolateBoundaryByLinearFitting(outs, out_fitt, true);
	DataFitt::smoothBoundaryLine(out_fitt, out_curv, 0.05f);
	ideal = out_curv;
	return true;
}


bool RetSegm::Feature2::makeupIdealOuterBoundary2(const std::vector<int>& outer, float rangeX, std::vector<int>& ideal)
{
	auto size = outer.size();
	auto outs = outer;

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		//if (i < size / 4) { // || i >= size * 2 / 3) {
			dataY[i] = outs[i];
		//}
	}

	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		copy(fittY.begin(), fittY.begin() + size, outs.begin());
	}

	ideal = outs;
	return true;
}


bool RetSegm::Feature2::makeupIdealInnerBoundary(const std::vector<int>& inner, const std::vector<int>& outer, float rangeX, int discX1, int discX2, std::vector<int>& ideal)
{
	auto size = (int)inner.size();
	auto inn_fitt = inner;
	auto inn_curv = inner;

	DataFitt::interpolateBoundaryByLinearFitting(inner, inn_fitt, true);
	
	for (int i = 0; i < size; i++) {
		if (i >= discX1 && i <= discX2) {
		}
		else {
			inn_fitt[i] = min(inn_fitt[i], outer[i]);
		}
	}

	DataFitt::smoothBoundaryLine(inn_fitt, inn_curv, 0.03f);

	for (int i = 0; i < size; i++) {
		if (i >= discX1 && i <= discX2) {
		}
		else {
			inn_curv[i] = min(inn_curv[i], outer[i]);
		}
	}

	ideal = inn_curv;
	return true;
}
