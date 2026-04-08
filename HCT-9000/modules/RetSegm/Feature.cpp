#include "stdafx.h"
#include "Feature.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "SegmImage.h"
#include "SegmLayer.h"

#include "RetSegm2.h"
#include "CppUtil2.h"

#include <vector>

using namespace RetSegm;
using namespace CppUtil;
using namespace cv;
using namespace std;


Feature::Feature()
{
}


Feature::~Feature()
{
}



bool RetSegm::Feature::estimateOpticDiskMargin(const SegmImage * imgSrc, const SegmImage * imgAsc, const std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& ideal, int & disc1, int & disc2, float rangeX)
{
	auto size = outer.size();
	int edgeSize = 5; // (int)((FEAT_DISK_EDGE_SIZE_MM / rangeX) * size) + 1;
	int holeSize = (int)((FEAT_DISK_HOLE_SIZE_MM / rangeX) * size) + 1;
	int discSize = (int)((FEAT_DISK_HORZ_SIZE_MM / rangeX) * size) + 1;
	int distSize = (int)((FEAT_DISC_DIST_SIZE_MM / rangeX) * size) + 1;

	int edges = 0;
	int sidx = -1, eidx = -1;
	for (int i = 0; i < size; i++) {
		if (outer[i] >= 0) {
			if (++edges > edgeSize) {
				sidx = i - edgeSize ;
				break;
			}
		}
	}

	if (sidx < 0) {
		return false;
	}

	edges = 0;
	for (int i = (int)(size - 1); i > sidx; i--) {
		if (outer[i] >= 0) {
			if (++edges > edgeSize) {
				eidx = i + edgeSize;
				break;
			}
		}
	}

	if (eidx >= size) {
		return false;
	}

	int none_cnt = 0, none_sum = 0;
	int hole_cnt = 0, hole_sum = 0;
	for (int i = sidx; i <= eidx; i++) {
		if (outer[i] < 0) {
			none_sum += i;
			none_cnt++;
			if (none_cnt > hole_cnt) {
				hole_cnt = none_cnt;
				hole_sum = none_sum;
			}
		}
		else {
			none_sum = none_cnt = 0;
		}
	}

	if (hole_cnt < holeSize) {
		return false;
	}

	int hole_cx = hole_sum / hole_cnt;
	int edge_x1 = 0, edge_x2 = 0;
	int edge_y1 = 0, edge_y2 = 0;

	for (int i = hole_cx; i >= sidx; i--) {
		if (outer[i] >= 0) {
			edge_x1 = i;
			edge_y1 = outer[i];
			break;
		}
	}

	for (int i = hole_cx; i <= eidx; i++) {
		if (outer[i] >= 0) {
			edge_x2 = i;
			edge_y2 = outer[i];
			break;
		}
	}

	// LogD() << "edge_x1: " << edge_x1 << ", edge_x2: " << edge_x2 << ", size: " << (edge_x2 - edge_x1) << ", holeSize: " << holeSize;


	int disc_x1 = edge_x1;
	int disc_x2 = edge_x2;
	int edge_cnt = 0;
	int prev_y1 = edge_y1;
	int prev_y2 = edge_y2;

	for (int i = edge_x1; i >= 0; i--) {
		if (outer[i] >= 0) {
			int diff = outer[i] - prev_y1;
			if (diff >= FEAT_DISK_EDGE_OFFSET_Y1 && diff <= FEAT_DISK_EDGE_OFFSET_Y2) {
				if (++edge_cnt >= edgeSize) {
					break;
				}
			}
			else {
				disc_x1 = i;
				edge_y1 = outer[i];
				edge_cnt = 0;
			}
			prev_y1 = outer[i];
		}
	}

	edge_cnt = 0;
	for (int i = edge_x2; i < size; i++) {
		if (outer[i] >= 0) {
			int diff = outer[i] - prev_y2;
			if (diff >= FEAT_DISK_EDGE_OFFSET_Y1 && diff <= FEAT_DISK_EDGE_OFFSET_Y2) {
				if (++edge_cnt >= edgeSize) {
					break;
				}
			}
			else {
				disc_x2 = i;
				edge_y2 = outer[i];
				edge_cnt = 0;
			}
			prev_y2 = outer[i];
		}
	}

	int threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 4.0f);
	auto maxLocs = imgSrc->getColumMaxLocs();
	auto maxVals = imgSrc->getColumMaxVals();
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMat();
	Mat ascMat = imgAsc->getCvMat();

	auto odata = outer;
	for (int i = disc_x1 + 1; i < disc_x2; i++) {
		odata[i] = -1;
	}

	Coarse::interpolateBoundaryByLinearFitting(odata, ideal, true);

	if (disc_x1 > 0) {
		edge_x1 = disc_x1;

		int x1 = max(edge_x1 - 12, 0);
		int x2 = (hole_cx + edge_x1) / 2;
		for (int c = x1; c < x2; c++) {
			if (ideal[c] > inner[c]) {
				int y1 = max(ideal[c], 0);
				int y2 = min(ideal[c] + 12, height - 1);
				int thresh = max((int)(maxVals[c] * 0.90f), threshMin);
				int gcur = 0, gmax = 0, mpos = 0;

				if (maxLocs[c] >= y1 && maxLocs[c] <= y2) {
					edge_x1 = c;
					edge_y1 = max(maxLocs[c] - 3, 0);
					odata[c] = edge_y1;
				}
				else {
					for (int r = y1; r <= y2; r++) {
						if (srcMat.at<uchar>(r, c) >= thresh) {
							for (; r <= y2; r++) {
								gcur = srcMat.at<uchar>(r, c);
								if (gcur > gmax) {
									gmax = gcur;
									mpos = r;
								}
							}
						}
					}

					if (mpos > 0) {
						gmax = mpos = 0;
						for (int r = y1; r <= mpos; r++) {
							gcur = ascMat.at<uchar>(r, c);
							if (gcur > gmax) {
								gmax = gcur;
								mpos = r;
							}
						}

						if (mpos > 0) {
							edge_x1 = c;
							edge_y1 = mpos;
							odata[c] = edge_y1;
						}
					}
				}
			}
			else {
				break;
			}
		}

		edge_x1 = disc_x1;
	}

	if (disc_x2 > 0) {
		edge_x2 = disc_x2;

		int x1 = (hole_cx + edge_x2) / 2;
		int x2 = min(edge_x2 + 12, (int)size - 1);

		for (int c = x2; c > x1; c--) {
			if (ideal[c] > inner[c]) {
				int y1 = max(ideal[c], 0);
				int y2 = min(ideal[c] + 12, height - 1);
				int thresh = max((int)(maxVals[c] * 0.90f), threshMin);
				int gcur = 0, gmax = 0, mpos = 0;

				if (maxLocs[c] >= y1 && maxLocs[c] <= y2) {
					edge_x2 = c;
					edge_y2 = max(maxLocs[c] - 3, 0);
					odata[c] = edge_y2;
				}
				else {
					for (int r = y1; r <= y2; r++) {
						if (srcMat.at<uchar>(r, c) >= thresh) {
							for (; r <= y2; r++) {
								gcur = srcMat.at<uchar>(r, c);
								if (gcur > gmax) {
									gmax = gcur;
									mpos = r;
								}
							}
						}
					}

					if (mpos > 0) {
						gmax = mpos = 0;
						for (int r = y1; r <= mpos; r++) {
							gcur = ascMat.at<uchar>(r, c);
							if (gcur > gmax) {
								gmax = gcur;
								mpos = r;
							}
						}

						if (mpos > 0) {
							edge_x2 = c;
							edge_y2 = mpos;
							odata[c] = edge_y2;
						}
					}
				}
			}
			else {
				break;
			}
		}

		disc_x2 = edge_x2;
	}
	
	float dx = (float)(disc_x2 - disc_x1);
	float dy = (float)(edge_y2 - edge_y1);
	int dist = (int)sqrt(dx*dx + dy*dy);
	int diff = disc_x2 - disc_x1;

	// if ((disc_x2 - disc_x1) >= discSize) {
	LogD() << "disc: " << disc_x1 << ", " << disc_x2 << ", size: " << (disc_x2 - disc_x1) << ", dist: " << dist;

	disc1 = disc_x1;
	disc2 = disc_x2;

	if (diff >= discSize || dist >= distSize) {
		disc1 = disc_x1;
		disc2 = disc_x2;
		outer = odata;
		return true;
	}
	return false;
}


bool RetSegm::Feature::estimateOpticDiskMargin(const std::vector<int>& input, const std::vector<int>& ideal, int & disc1, int & disc2)
{
	auto size = input.size();

	int hx1, hx2;
	bool hole = false;

	disc1 = disc2 = 0;
	for (int i = 0; i < size; i++) {
		if (input[i] < 0 || input[i] > (ideal[i] + FEAT_DISK_OUTER_IDEAL_OFFSET)) {
			if (!hole) {
				hx1 = hx2 = i;
				hole = true;
			}
			else {
				hx2 = i;
			}
		}
		else {
			if (hole) {
				if ((hx2 - hx1) > (disc2 - disc1)) {
					disc1 = hx1;
					disc2 = hx2;
				}
				hole = false;
			}
		}
	}

	for (int i = disc1; i >= 0; i--) {
		if (input[i] >= 0) {
			if (input[i] <= (ideal[i] + FEAT_DISK_MARGIN_IDEAL_OFFSET)) {
				disc1 = i;
				break;
			}
		}
	}

	for (int i = disc2; i < size; i++) {
		if (input[i] >= 0) {
			if (input[i] <= (ideal[i] + FEAT_DISK_MARGIN_IDEAL_OFFSET)) {
				disc2 = i;
				break;
			}
		}
	}

	int minDisk = (int)(size * FEAT_DISK_SIZE_MIN);
	if ((disc2 - disc1 + 1) >= minDisk) {
		return true;
	}
	return false;
}


bool RetSegm::Feature::calculateDiscSidePixels(const std::vector<int>& inner, const std::vector<int>& outer, int & disc1, int & disc2, int & pixels)
{
	int size = (int)inner.size();

	if (disc1 < 0 || disc2 < 0 || disc1 >= size || disc2 >= size || disc1 == disc2) {
		return false;
	}

	int out_x1 = disc1;
	int out_x2 = disc2;
	int out_y1 = outer[out_x1];
	int out_y2 = outer[out_x2];

	if (out_y1 < 0 || out_y2 < 0 || out_x1 == out_x2) {
		return false;
	}

	/*
	const int DISC_MARGIN_EXTEND_SPAN = 5;

	int init_x = max(disc1 - 1, 0);
	int init_y = outer[init_x];

	for (int i = out_x1; i <= out_x2; i++) {
		if (abs(outer[i] - init_y) >= DISC_MARGIN_EXTEND_SPAN) {
			disc1 = i;
			break;
		}
	}

	init_x = min(disc2 + 1, size - 1);
	init_y = outer[init_x];

	for (int i = out_x2; i >= disc1; i--) {
		if (abs(outer[i] - init_y) >= DISC_MARGIN_EXTEND_SPAN) {
			disc2 = i;
			break;
		}
	}
	*/

	out_x1 = disc1;
	out_x2 = disc2;
	out_y1 = outer[out_x1];
	out_y2 = outer[out_x2];

	float axialResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int upp_y1 = (int)(out_y1 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	int upp_y2 = (int)(out_y2 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	upp_y1 = max(upp_y1, 0);
	upp_y2 = max(upp_y2, 0);

	float slope = (float)(upp_y2 - upp_y1) / (float)(out_x2 - out_x1);
	int line;

	int count = 0;
	for (int k = out_x1, dist = 0; k <= out_x2; k++, dist++) {
		line = (int)(upp_y1 + dist * slope);
		if (inner[k] < line) {
			count += (line - inner[k]);
		}
	}

	pixels = count;
	return true;
}


bool RetSegm::Feature::calculateCupDepthPixels(const std::vector<int>& inner, const std::vector<int>& outer, int disc1, int disc2, int & cup1, int & cup2, int & pixels)
{
	int size = (int)inner.size();

	if (disc1 < 0 || disc2 < 0 || disc1 >= size || disc2 >= size || disc1 == disc2) {
		return false;
	}

	int out_x1 = disc1;
	int out_x2 = disc2;
	int out_y1 = outer[out_x1];
	int out_y2 = outer[out_x2];

	if (out_y1 < 0 || out_y2 < 0 || out_x1 == out_x2) {
		return false;
	}

	float axialResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int upp_y1 = (int)(out_y1 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	int upp_y2 = (int)(out_y2 - FEAT_DISK_CUP_LINE_DEPTH / axialResol);
	upp_y1 = max(upp_y1, 0);
	upp_y2 = max(upp_y2, 0);

	float slope = (float)(upp_y2 - upp_y1) / (float)(out_x2 - out_x1);
	int line;

	int cup_x1 = -1;
	int cup_x2 = -1;

	for (int k = out_x1, dist = 0; k <= out_x2; k++, dist++) {
		line = max((int)(upp_y1 + dist * slope), 0);
		if (inner[k] >= line) {
			cup_x1 = k;
			break;
		}
	}

	if (cup_x1 >= 0)
	{
		for (int k = out_x2, dist = 0; k >= out_x1; k--, dist++) {
			line = max((int)(upp_y2 - dist * slope), 0);
			if (inner[k] >= line) {
				cup_x2 = k;
				break;
			}
		}
	}

	if (cup_x1 < 0 || cup_x2 < 0 || cup_x1 == cup_x2) {
		return false;
	}

	int count = 0;
	for (int k = cup_x1, dist = (cup_x1 - out_x1); k <= cup_x2; k++, dist++) {
		line = max((int)(upp_y1 + dist * slope), 0);
		if (inner[k] >= line) {
			count += (inner[k] - line + 1);
		}
	}

	cup1 = cup_x1;
	cup2 = cup_x2;
	pixels = count;
	return true;
}