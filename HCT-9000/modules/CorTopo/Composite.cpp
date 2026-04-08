#include "stdafx.h"
#include "Composite.h"
#include "CppUtil2.h"
#include "RetSegm2.h"
#include "SegmScan2.h"

using namespace CorTopo;
using namespace CppUtil;
using namespace RetSegm;
using namespace SegmScan;
using namespace cv;
using namespace std;


Composite::Composite()
{
}


Composite::~Composite()
{
}



bool CorTopo::Composite::detectTrunkLinesOnCornea(const RetSegm::SegmImage * srcImg, const std::vector<int>& antes, const std::vector<int>& posts, std::vector<int>& line1, std::vector<int>& line2)
{
	Mat srcMat = srcImg->getCvMatConst();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	if (antes.empty() || posts.empty()) {
		return false;
	}

	line1 = std::vector<int>(img_w, -1);
	line2 = std::vector<int>(img_w, -1);
	auto edge1 = line1;
	auto edge2 = line2;

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 1.0f);
	int depth = 48;
	int limit = depth / 3;
	int band_y1 = img_h - 1 - depth;
	int band_y2 = img_h - 1;
	int size1 = 0;
	int size2 = 0;

	for (int r = band_y1; r <= band_y2; r++) {
		int band_x1 = -1;
		int band_x2 = -1;
		int out_x = 0;
		for (int c = 0; c < img_w / 2; c++) {
			if (antes[c] <= r) {
				band_x1 = max(c - 10, 0);
				out_x = c;
				for (; c < img_w / 2; c++) {
					if (posts[c] <= r) {
						band_x2 = out_x + (c - out_x)/2;
						break;
					}
				}
				break;
			}
		}
		if (band_x1 >= 0 && band_x2 >= 0) {
			for (int c = band_x1; c <= band_x2; c++) {
				int val = srcMat.at<uchar>(r, c);
				if (val > edge_thresh) {
					edge1[c] = r;
					size1 += 1;
					break;
				}
			}
		}

		band_x1 = -1;
		band_x2 = -1;
		for (int c = img_w - 1; c > img_w / 2; c--) {
			if (antes[c] <= r) {
				band_x2 = min(c + 10, img_w - 1);
				out_x = c;
				for (; c > img_w / 2; c--) {
					if (posts[c] <= r) {
						band_x1 = c + (out_x - c) / 2;
						break;
					}
				}
				break;
			}
		}
		if (band_x1 >= 0 && band_x2 >= 0) {
			for (int c = band_x2; c >= band_x1; c--) {
				int val = srcMat.at<uchar>(r, c);
				if (val > edge_thresh) {
					edge2[c] = r;
					size2 += 1;
					break;
				}
			}
		}
	}

	if (size1 >= limit) {
		auto size = img_w;
		auto dataX = vector<int>(size, -1);
		auto dataY = vector<int>(size, -1);
		auto fittY = vector<int>(size, -1);

		for (int i = 0; i < img_w/2; i++) {
			dataX[i] = i;
			dataY[i] = (edge1[i] < 0 ? 0 : edge1[i]);
		}

		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
			line1 = fittY;
		}
	}
	else {
		line1.clear();
	}

	if (size2 >= limit) {
		auto size = img_w;
		auto dataX = vector<int>(size, -1);
		auto dataY = vector<int>(size, -1);
		auto fittY = vector<int>(size, -1);

		for (int i = img_w-1; i > img_w / 2; i--) {
			dataX[i] = i;
			dataY[i] = (edge2[i] < 0 ? 0 : edge2[i]);
		}

		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
			line2 = fittY;
		}
	}
	else {
		line2.clear();
	}

	return true;
}


bool CorTopo::Composite::detectTrunkLinesOnChamber(const RetSegm::SegmImage * srcImg, std::vector<int>& line1, std::vector<int>& line2, float rangeX)
{
	Mat srcMat = srcImg->getCvMatConst();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	line1 = std::vector<int>(img_w, -1);
	line2 = std::vector<int>(img_w, -1);
	auto edge1 = line1;
	auto edge2 = line2;

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 1.0f);
	int range = 72;
	int depth = 48;
	int limit = (int)(depth * 0.35f);
	int space = 5;
	int size1 = 0;
	int size2 = 0;

	for (int r = 0; r < depth; r++) {
		for (int c = 0; c < range; c++) {
			int val = srcMat.at<uchar>(r, c);
			if (val > edge_thresh) {
				if (c > space) {
					edge1[c] = r;
					size1++;
				}
				break;
			}
		}
	}

	if (size1 >= limit) {
		auto size = img_w;
		auto dataX = vector<int>(size, -1);
		auto dataY = vector<int>(size, -1);
		auto fittY = vector<int>(size, -1);

		for (int i = 0; i < img_w/2; i++) {
			dataX[i] = i;
			dataY[i] = (edge1[i] < 0 ? 0 : edge1[i]);
		}

		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
			line1 = fittY;
		}
	}
	else {
		line1.clear();
	}

	for (int r = 0; r < depth; r++) {
		for (int c = img_w - 1; c >= (img_w - range); c--) {
			int val = srcMat.at<uchar>(r, c);
			if (val > edge_thresh) {
				if (c < img_w - space) {
					edge2[c] = r;
					size2++;
				}
				break;
			}
		}
	}

	if (size2 >= limit) {
		auto size = img_w;
		auto dataX = vector<int>(size, -1);
		auto dataY = vector<int>(size, -1);
		auto fittY = vector<int>(size, -1);

		for (int c = img_w - 1; c > img_w/2; c--) {
			dataX[c] = c;
			dataY[c] = (edge2[c] < 0 ? 0 : edge2[c]);
		}

		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
			line2 = fittY;
		}
	}
	else {
		line2.clear();
	}

	if (line1.empty() && line2.empty()) {
		return false;
	}
	return true;
}


bool CorTopo::Composite::searchCorneaChamberOffsets(const RetSegm::SegmImage * cornea, const RetSegm::SegmImage * chamber, const std::vector<float>& line1, const std::vector<float>& line2, int & offsetX, int & offsetY)
{
	Mat corMat = cornea->getCvMatConst();
	Mat chaMat = chamber->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;

	offsetX = 0;
	offsetY = 0;

	int ofs_x = 0;
	int ofs_y = 0;
	int ofs_xmin = -25;
	int ofs_xmax = +25;
	int ofs_ymax = +18;
	int y_lookup = 15;
	int ksize = 3;
	int khalf = ksize / 2;

	float max_score = 0.0f;
	int max_ofs_x = 0;
	int max_ofs_y = 0;

	if (line1.empty() && line2.empty()) {
		return true;
	}

	for (int dy = 0; dy <= ofs_ymax; dy++) {
		for (int dx = ofs_xmin; dx <= ofs_xmax; dx++) {
			float score = 0.0f;
			int ndata = 0;

			if (!line1.empty()) {
				float a = line1[0];
				float b = line1[1];
				int r, c;

				for (int y = 1, x = 0; y < y_lookup; y++) {
					x = (int)((-1 * y - b) / a);
					r = img_h - (y + dy);
					c = x + dx;
					r = min(max(r, 0), img_h - 1);
					c = min(max(c, ksize), img_w - ksize);

					int gsum = 0;
					for (int k = -khalf; k < 0; k++) {
						gsum -= corMat.at<uchar>(r, c + k);
					}
					for (int k = khalf; k > 0; k--) {
						gsum += corMat.at<uchar>(r, c + k);
					}
					if (gsum > 0) {
						score += ((float)gsum / ksize);
						ndata += 1;
					}
				}
			}
			
			if (!line2.empty()) {
				float a = line2[0];
				float b = line2[1];
				int r, c;

				for (int y = 1, x = 0; y < y_lookup; y++) {
					x = (int)((-1 * y - b) / a);
					r = img_h - (y + dy);
					c = x + dx;
					r = min(max(r, 0), img_h - 1);
					c = min(max(c, ksize), img_w - ksize);

					int gsum = 0;
					for (int k = -khalf; k < 0; k++) {
						gsum += corMat.at<uchar>(r, c + k);
					}
					for (int k = khalf; k > 0; k--) {
						gsum -= corMat.at<uchar>(r, c + k);
					}
					if (gsum > 0) {
						score += ((float)gsum / ksize);
						ndata += 1;
					}
				}
			}

			if (score > max_score) {
				max_score = score;
				max_ofs_x = dx;
				max_ofs_y = dy;
			}
		}
	}

	offsetX = max_ofs_x;
	offsetY = max_ofs_y;
	return true;
}


bool CorTopo::Composite::updateCorneaChamberOffsets(const RetSegm::SegmImage * cornea, const std::vector<int>& corneaHinge1, const std::vector<int>& corneaHinge2,
													const std::vector<int>& chamberHinge1, const std::vector<int>& chamberHinge2, int & offsetX, int & offsetY)
{
	Mat corMat = cornea->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;

	if ((corneaHinge1.empty() && corneaHinge2.empty()) ||
		(chamberHinge1.empty() && chamberHinge2.empty())) {
		offsetX = 0;
		offsetY = 0;
		return false;
	}

	int cor_out_x1 = -1, cor_out_x2 = -1;
	int cha_out_x1 = -1, cha_out_x2 = -1;

	if (!corneaHinge1.empty()) {
		for (int i = 0; i < img_w/2; i++) {
			if (corneaHinge1[i] < img_h) {
				cor_out_x1 = i - offsetX;
				break;
			}
		}
	}

	if (!corneaHinge2.empty()) {
		for (int i = img_w-1; i > img_w / 2; i--) {
			if (corneaHinge2[i] < img_h) {
				cor_out_x2 = i - offsetX;
				break;
			}
		}
	}

	if (!chamberHinge1.empty()) {
		for (int i = 0; i < img_w / 2; i++) {
			if (chamberHinge1[i] < img_h) {
				cha_out_x1 = i;
				break;
			}
		}
	}

	if (!chamberHinge2.empty()) {
		for (int i = img_w - 1; i > img_w / 2; i--) {
			if (chamberHinge2[i] < img_h) {
				cha_out_x2 = i;
				break;
			}
		}
	}

	bool out_joint1 = (cor_out_x1 >= 0 && cha_out_x1 >= 0) && (cha_out_x1 > cor_out_x1);
	bool out_joint2 = (cor_out_x2 >= 0 && cha_out_x2 >= 0) && (cha_out_x2 < cor_out_x2);
	offsetY = 0;

	if (out_joint1 || out_joint2) {
		int shift = 0;
		int limit = 24;

		for (int k = 1; k < limit; k++) {
			bool check = false;
			if (out_joint1) {
				cor_out_x1 = corneaHinge1[cor_out_x1 + k] - offsetX;
				if (cor_out_x1 <= cha_out_x1) {
					check = true;
				}
			}

			if (out_joint2) {
				cor_out_x2 = corneaHinge2[cor_out_x2 - k] - offsetX;
				if (cor_out_x2 >= cha_out_x2) {
					check = true;
				}
			}

			if (check) {
				offsetY = k * -1;
				break;
			}
		}
	}

	// LogD() << "Updated offsets: " << offsetX << ", " << offsetY;

	return true;
}


bool CorTopo::Composite::updateCorneaChamberOffsets2(const RetSegm::SegmImage* cornea, const std::vector<int>& corenaAntes, const std::vector<int>& chamberHinge1, const std::vector<int>& chamberHinge2, int & offsetX, int & offsetY)
{
	Mat corMat = cornea->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;

	if ((corenaAntes.empty()) ||
		(chamberHinge1.empty() && chamberHinge2.empty())) {
		offsetX = 0;
		offsetY = 0;
		return false;
	}

	int cor_out_x1 = -1, cor_out_x2 = -1;
	int cha_out_x1 = -1, cha_out_x2 = -1;

	if (!chamberHinge1.empty()) {
		for (int i = 0; i < img_w / 2; i++) {
			if (chamberHinge1[i] >= 0) {
				cha_out_x1 = i;
			}
			else {
				break;
			}
		}
	}

	if (!chamberHinge2.empty()) {
		for (int i = img_w - 1; i > img_w / 2; i--) {
			if (chamberHinge2[i] >= 0) {
				cha_out_x2 = i;
			}
			else {
				break;
			}
		}
	}

	if (cha_out_x1 < 0 && cha_out_x2 < 0) {
		offsetX = 0;
		offsetY = 0;
		return false;
	}

	int limit = 48;
	offsetY = 0;

	for (int k = 0; k < limit; k++) {
		int cur_y = img_h - k;
		int cor_out_x1 = -1, cor_out_x2 = -1;

		if (!corenaAntes.empty()) {
			for (int i = 0; i < img_w / 2; i++) {
				if (corenaAntes[i] < cur_y) {
					cor_out_x1 = i - offsetX;
					break;
				}
			}
		}

		if (!corenaAntes.empty()) {
			for (int i = img_w - 1; i > img_w / 2; i--) {
				if (corenaAntes[i] < cur_y) {
					cor_out_x2 = i - offsetX;
					break;
				}
			}
		}

		int count = 0;
		int check = 0;

		if (cha_out_x1 >= 0 && cor_out_x1 >= 0) {
			count += 1;
			if ((cor_out_x1 - offsetX) >= cha_out_x1) {
				check += 1;
			}
		}
		if (cha_out_x2 >= 0 && cor_out_x2 >= 0) {
			count += 1;
			if ((cor_out_x2 - offsetX) <= cha_out_x2) {
				check += 1;
			}
		}

		offsetY = k * -1;
		if (check > 0) {
			break;
		}
	}

	// LogD() << "Updated offsets: " << offsetX << ", " << offsetY;

	return true;
}


bool CorTopo::Composite::stitchCorneaChamberImages(const RetSegm::SegmImage * cornea, const RetSegm::SegmImage * chamber, int offsetX, int offsetY, RetSegm::SegmImage * result)
{
	Mat corMat = cornea->getCvMatConst();
	Mat chaMat = chamber->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;

	Mat resMat = Mat::zeros(img_h*2, img_w, CV_8UC1);
	chaMat.copyTo(resMat(cv::Rect(0, img_h, img_w, img_h)));

	for (int r = 0; r < (img_h - offsetY); r++) {
		for (int c = 0; c < img_w; c++) {
			int x = c + offsetX;
			int y = r + offsetY;
			if (x >= 0 && x < img_w && y >= 0 && y < img_h) {
				if (r < img_h) {
					resMat.at<uchar>(r, c) = corMat.at<uchar>(y, x);
				}
				else {
					resMat.at<uchar>(r, c) = (resMat.at<uchar>(r, c) + corMat.at<uchar>(y, x)) / 2;
				}
			}
		}
	}
	
	// result->getCvMat() = resMat;
	Size size(img_w, img_h);
	cv::resize(resMat, result->getCvMat(), size, INTER_LANCZOS4);
	return true;
}


bool CorTopo::Composite::removeReversedCorneaOnChamber(const RetSegm::SegmImage * cornea, const RetSegm::SegmImage * chamber)
{
	Mat corMat = cornea->getCvMatConst();
	Mat chaMat = chamber->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;

	Mat revMat = Mat::zeros(img_h, img_w, CV_8UC1);
	flip(corMat, revMat, 0);

	int img_hw = img_w / 2;
	int img_hh = img_h / 2;
	int rect_w = img_w / 8;
	int rect_h = img_h / 8;
	int rect_hw = rect_w / 2;
	int rect_hh = rect_h / 2;

	int sizePerMM = img_w / 16;
	int band_x1 = (int)(-(sizePerMM * 0.25));
	int band_x2 = (int)(sizePerMM * 0.25);
	int band_y1 = -(rect_hh);
	int band_y2 = 0;

	int max_corr = 0;
	int max_ofs_y = 0;
	int max_ofs_x = 0;

	for (int ofs_y = band_y1; ofs_y <= band_y2; ofs_y++) {
		for (int ofs_x = band_x1; ofs_x <= band_x2; ofs_x++) {
			int corr = 0;
			for (int r = 0; r < rect_h; r++) {
				for (int c = 0; c < rect_w; c++) {
					int x1 = img_hw - rect_hw + c;
					int y1 = img_h - rect_h + r;
					int x2 = x1 + ofs_x;
					int y2 = y1 + ofs_y;

					int val1 = revMat.at<uchar>(y1, x1);
					int val2 = chaMat.at<uchar>(y2, x2);
					corr += (val1 * val2);
				}
			}
			if (corr > max_corr) {
				max_corr = corr;
				max_ofs_x = ofs_x;
				max_ofs_y = ofs_y;
			}
			// LogD() << ofs_x << ", " << ofs_y << " : " << corr;
		}
	}

	for (int y = 0; y < img_h; y++) {
		for (int x = 0; x < img_w; x++) {
			int x1 = x + max_ofs_x;
			int y1 = y + max_ofs_y;

			if (x1 >= 0 && x1 < img_w && y1 >= 0 && y1 < img_h) {
				int val1 = chaMat.at<uchar>(y, x);
				int val2 = revMat.at<uchar>(y1, x1);
				int vout = val1 - val2;
				chaMat.at<uchar>(y, x) = max(vout, 0);
			}
		}
	}

	// chaMat = chaMat - revMat;

	// chamber->getCvMat() = revMat;
	return true;
}


bool CorTopo::Composite::removeReversedCorneaOnChamber2(const RetSegm::SegmImage * cornea, const RetSegm::SegmImage * chamber, 
	const std::vector<int>& corneaAntes, const std::vector<int>& corneaPosts, const std::vector<int>& corneaSides, 
	const std::vector<int>& chamberHinge1, const std::vector<int>& chamberHinge2, int centerX, int centerY, int offsetX, int offsetY)
{
	Mat corMat = cornea->getCvMatConst();
	Mat chaMat = chamber->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;
	float sizePerMM = img_w / 16.0f;

	if (corneaAntes.empty() || corneaPosts.empty() || corneaSides.empty() || centerX < 0) {
		return false;
	}
	int size = (int)corneaAntes.size();

	Mat revMat = Mat::zeros(img_h, img_w, CV_8UC1);
	flip(corMat, revMat, 0);
	cv::GaussianBlur(revMat, revMat, cv::Size(27, 27), 3.5);

	auto antes = std::vector<int>(img_w);
	auto posts = std::vector<int>(img_w);
	auto sides = std::vector<int>(img_w);
	auto rates = std::vector<float>(img_w);

	for (int i = 0; i < img_w; i++) {
		antes[i] = img_h - 1 - corneaAntes[i] - offsetY;
		posts[i] = img_h - 1 - corneaPosts[i] - offsetY;
		sides[i] = img_h - 1 - corneaSides[i] - offsetY;

		float dist = (i - centerX) / sizePerMM;
		float rate = (0.045f*dist*dist + 1.0f);
		rates[i] = rate;
	}

	const int kHorzShiftToCenter = 45;
	const float kHorzPreserveRange = 60.0f;

	for (int y = 0; y < img_h; y++) {
		int side_x1 = -1;
		int side_x2 = -1;
		int cent_x1 = -1;
		int cent_x2 = -1;
		
		if (!chamberHinge1.empty()) {
			for (int i = 0; i < centerX; i++) {
				if (chamberHinge1[i] <= y && chamberHinge1[i] >= 0) {
					cent_x1 = i + kHorzShiftToCenter;
					break;
				}
			}
		}
		else {
			for (int i = 0; i < centerX; i++) {
				if ((corneaPosts[i] - img_h) <= y) {
					side_x1 = i + offsetX; 
					cent_x1 = side_x1 - kHorzShiftToCenter;
					break;
				}
			}
		}

		if (!chamberHinge2.empty()) {
			for (int i = img_w - 1; i > centerX; i--) {
				if (chamberHinge2[i] <= y && chamberHinge2[i] >= 0) {
					cent_x2 = i - kHorzShiftToCenter;
					break;
				}
			}
		}
		else {
			for (int i = img_w - 1; i > centerX; i--) {
				if ((corneaPosts[i] - img_h) <= y) {
					side_x2 = i + offsetX;
					cent_x2 = side_x2 + kHorzShiftToCenter;
					break;
				}
			}
		}

		for (int x = 0; x < img_w; x++) {
			int x1 = x + offsetX;
			int y1 = y + offsetY;

			float side_rate = 1.0f;
			int dist = 0;
			if (x < centerX) {
				if (cent_x1 >= 0) {
					dist = abs(x - cent_x1);
					side_rate = min((float)dist / kHorzPreserveRange, 1.0f);
				}

			}
			else {
				if (cent_x2 >= 0) {
					dist = abs(x - cent_x2);
					side_rate = min((float)dist / kHorzPreserveRange, 1.0f);
				}
			}

			if (x1 >= 0 && x1 < img_w) {
				if (y1 >= 0 && y1 < img_h) {
					if (y <= antes[x1] && y >= posts[x1]) {
						int val1 = chaMat.at<uchar>(y, x);
						int val2 = revMat.at<uchar>(y1, x1);
						// float corr = (val1*val2) / (0.5f*(val1*val1 + val2*val2));
						// int vout = (int)(val1 - val2 * rates[x] * (val1 / 128.0f));
						int vout = (int)(val1 - val2 * rates[x] * side_rate);
						chaMat.at<uchar>(y, x) = max(vout, 0);
					}
					else {
						int val1 = chaMat.at<uchar>(y, x);
						int val2 = revMat.at<uchar>(y1, x1);
						// float corr = (val1*val2) / (0.5f*(val1*val1 + val2*val2));
						// int vout = (int)(val1 - val2 * rates[x] * (val1 / 128.0f));
						int vout = (int)(val1 - val2 * rates[x] * side_rate);
						chaMat.at<uchar>(y, x) = max(vout, 0);
					}
				}
				else {
					y1 = img_h - 1;
					int val1 = chaMat.at<uchar>(y, x);
					int val2 = revMat.at<uchar>(y1, x1);
					// float corr = (val1*val2) / (0.5f*(val1*val1 + val2*val2));
					// int vout = (int)(val1 - val2 * rates[x] * (val1 / 128.0f));
					int vout = (int)(val1 - val2 * rates[x]);
					chaMat.at<uchar>(y, x) = max(vout, 0);
				}
			}
		}
	}
	return true;
}


bool CorTopo::Composite::removeBackgroundOnCornea(const RetSegm::SegmImage * cornea, int centerX, int centerY, const std::vector<int> antes, const std::vector<int> posts)
{
	Mat corMat = cornea->getCvMatConst();
	float thresh = 64.0f;

	int img_w = corMat.cols;
	int img_h = corMat.rows;
	float sizePerMM = img_w / 9.0f;

	if (antes.empty() || posts.empty() || centerX < 0) {
		return false;
	}
	int size = (int)antes.size();
	auto rates = std::vector<float>(img_w);

	Mat revMat = Mat::zeros(img_h, img_w, CV_8UC1);
	cv::GaussianBlur(corMat, revMat, cv::Size(27, 27), 3.5);

	for (int i = 0; i < img_w; i++) {
		float dist = (i - centerX) / sizePerMM;
		float rate = max((1.0f - 0.01f*dist*dist), 0.0f);
		rates[i] = rate;
	}

	for (int y = 0; y < img_h; y++) {
		for (int x = 0; x < img_w; x++) {
			int dist = min(abs(y - antes[x]), abs(y - posts[x]));
			if (y < antes[x] || y > posts[x]) {
				int val1 = corMat.at<uchar>(y, x);
				int val2 = revMat.at<uchar>(y, x);
				float rate2 = min(((float)dist / 20.0f), 1.0f);
				int vout = (int)(val1 - val2 * rates[x] * rate2);
				corMat.at<uchar>(y, x) = max(vout, 0);
			}
			else {
				int val1 = corMat.at<uchar>(y, x);
				int val2 = revMat.at<uchar>(y, x);
				float rate2 = max((1.0f - (float)dist * 0.05f), 0.0f);
				float rate3 = max((1.0f - (float)val1 / thresh), 0.0f);
				int vout = (int)(val1 - val2 * rates[x] * rate2 * rate3);
				corMat.at<uchar>(y, x) = max(vout, 0);
			}
		}
	}
	return true;
}


bool CorTopo::Composite::removeBackgroundOnAnterior(const RetSegm::SegmImage * anterior)
{
	if (!anterior || anterior->isEmpty()) {
		return false;
	}

	Mat corMat = anterior->getCvMatConst();
	int img_w = corMat.cols;
	int img_h = corMat.rows;
	float sizePerMM = img_w / 16.0f;

	CorneaBsegm bsegm;
	if (bsegm.loadSource(anterior->getBitsData(), anterior->getWidth(), anterior->getHeight())) {
		OctScanPattern desc;
		desc.setup(PatternName::AnteriorLine, anterior->getWidth(), 1, 16.0f, 0.0f);
		bsegm.setPatternDescript(desc);
		if (bsegm.performAnalysis()) {
			auto layer1 = bsegm.getCorneaLayers()->getEPI()->getYs();
			auto layer2 = bsegm.getCorneaLayers()->getEND()->getYs();
			float xRes = bsegm.getResolutionX();
			float yRes = (float)GlobalSettings::getCorneaScanAxialResolution();

			Mat revMat = Mat::zeros(img_h, img_w, CV_8UC1);
			cv::GaussianBlur(corMat, revMat, cv::Size(0, 0), 1.0);

			for (int y = 0; y < img_h; y++) {
				for (int x = 0; x < img_w; x++) {
					if (y > layer2[x]) {
						int val1 = corMat.at<uchar>(y, x);
						int val2 = revMat.at<uchar>(y, x);
						float dist = abs(x - img_w / 2) / sizePerMM;
						float rate1 = min(0.05f * dist * dist, 1.0f);
						float rate2 = max(1.0f - val1 / 255.0f, 0.0f);
						int vout = (int)(val1 - val2 * rate1 * rate2);
						corMat.at<uchar>(y, x) = max(vout, 0);
					}
				}
			}
			return true;
		}
	}

	return false;
}


bool CorTopo::Composite::applyDewarpingOnAnterior(RetSegm::SegmImage * anterior)
{
	if (!anterior || anterior->isEmpty()) {
		return false;
	}

	CorneaBsegm bsegm;
	if (bsegm.loadSource(anterior->getBitsData(), anterior->getWidth(), anterior->getHeight())) {
		OctScanPattern desc;
		desc.setup(PatternName::AnteriorLine, anterior->getWidth(), 1, 16.0f, 0.0f);
		bsegm.setPatternDescript(desc);
		if (bsegm.performAnalysis()) {
			auto layer1 = bsegm.getCorneaLayers()->getEPI()->getYs();
			auto layer2 = bsegm.getCorneaLayers()->getEND()->getYs();
			float xRes = bsegm.getResolutionX();
			float yRes = (float)GlobalSettings::getCorneaScanAxialResolution();
			anterior->applyDewarping(layer1, xRes, yRes, CORNEA_REFLECTIVE_INDEX);
			return true;
		}
	}

	return false;
}
