#include "stdafx.h"
#include "EnfaceData.h"


#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;



struct EnfaceData::EnfaceDataImpl
{
	CvImage image;

	OctScanPattern pattern;
	OcularLayerType upper;
	OcularLayerType lower;
	float upperOffset = 0.0f;
	float lowerOffset = 0.0f;

	EnfaceDataImpl() : upper(OcularLayerType::NONE), lower(OcularLayerType::NONE)
	{
		pattern.init();
	}
};


EnfaceData::EnfaceData() :
	d_ptr(make_unique<EnfaceDataImpl>())
{
}


RetParam::EnfaceData::~EnfaceData() = default;
RetParam::EnfaceData::EnfaceData(EnfaceData && rhs) = default;
EnfaceData & RetParam::EnfaceData::operator=(EnfaceData && rhs) = default;


RetParam::EnfaceData::EnfaceData(const EnfaceData & rhs)
	: d_ptr(make_unique<EnfaceDataImpl>(*rhs.d_ptr))
{
}


EnfaceData & RetParam::EnfaceData::operator=(const EnfaceData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool RetParam::EnfaceData::setupData(const OctScanPattern &pattern,
	OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
	const std::vector<std::vector<std::uint8_t>>& lines,
	const std::vector<bool>& flags)
{
	d_ptr->pattern = pattern;
	d_ptr->upper = upper;
	d_ptr->lower = lower;
	d_ptr->upperOffset = upperOffset;
	d_ptr->lowerOffset = lowerOffset;

	bool ret = buildDataImage(lines, flags);
	return ret;
}


bool RetParam::EnfaceData::setupData(const OctScanPattern& pattern,
	OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
	const std::vector<std::vector<float>>& lines,
	const std::vector<bool>& flags)
{
	d_ptr->pattern = pattern;
	d_ptr->upper = upper;
	d_ptr->lower = lower;
	d_ptr->upperOffset = upperOffset;
	d_ptr->lowerOffset = lowerOffset;

	bool ret = buildDataImage(lines, flags);
	return ret;
}


bool RetParam::EnfaceData::isEmpty(void) const
{
	return getDataImage()->isEmpty();
}


bool RetParam::EnfaceData::isIdentical(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset) const
{
	if (this->upperLayer() == upper &&
		this->lowerLayer() == lower && 
		this->upperLayerOffset() == upperOffset && 
		this->lowerLayerOffset() == lowerOffset) {
		return true;
	}
	return false;
}


bool RetParam::EnfaceData::isIdentical(float rangeX, float rangeY, int numLines, int lineSize, OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset) const
{
	if (this->rangeX() == rangeX &&
		this->rangeY() == rangeY &&
		this->numLines() == numLines &&
		this->lineSize() == lineSize &&
		this->upperLayer() == upper &&
		this->lowerLayer() == lower &&
		this->upperLayerOffset() == upperOffset &&
		this->lowerLayerOffset() == lowerOffset) {
		return true;
	}
	return false;
}


float RetParam::EnfaceData::rangeX(void) const
{
	return d_ptr->pattern.getScanRangeX();
}


float RetParam::EnfaceData::rangeY(void) const
{
	return d_ptr->pattern.getScanRangeY();
}


int RetParam::EnfaceData::numLines(void) const
{
	return d_ptr->pattern.getNumberOfScanLines();
}


int RetParam::EnfaceData::lineSize(void) const
{
	return d_ptr->pattern.getNumberOfScanPoints();
}

bool RetParam::EnfaceData::isVertical(void) const
{
	return d_ptr->pattern.isVertial();
}


OcularLayerType RetParam::EnfaceData::upperLayer(void) const
{
	return d_ptr->upper;
}


OcularLayerType RetParam::EnfaceData::lowerLayer(void) const
{
	return d_ptr->lower;
}

float RetParam::EnfaceData::upperLayerOffset(void) const
{
	return d_ptr->upperOffset;
}

float RetParam::EnfaceData::lowerLayerOffset(void) const
{
	return d_ptr->lowerOffset;
}


int RetParam::EnfaceData::getWidth(void) const
{
	return d_ptr->image.getWidth();
}


int RetParam::EnfaceData::getHeight(void) const
{
	return d_ptr->image.getHeight();
}


float RetParam::EnfaceData::getPixelXperMM(void) const
{
	return (rangeX() <= 0.0f ? 0.0f : getWidth() / rangeX());
}


float RetParam::EnfaceData::getPixelYperMM(void) const
{
	return (rangeY() <= 0.0f ? 0.0f : getHeight() / rangeY());
}


int RetParam::EnfaceData::getPositionX(float xmm) const
{
	int posX = (int)getPositionXf(xmm);
	return min(max(posX, 0), getWidth() - 1);
}


int RetParam::EnfaceData::getPositionY(float ymm) const
{
	int posY = (int)getPositionYf(ymm);
	return min(max(posY, 0), getHeight() - 1);
}


float RetParam::EnfaceData::getPositionXf(float xmm) const
{
	float posX = (getWidth() / 2 + getPixelSizeX(xmm));
	return posX;
}


float RetParam::EnfaceData::getPositionYf(float ymm) const
{
	float posY = (getHeight() / 2 + getPixelSizeY(ymm));
	return posY;
}


float RetParam::EnfaceData::getPixelSizeX(float width) const
{
	return getPixelXperMM() * width;
}


float RetParam::EnfaceData::getPixelSizeY(float height) const
{
	return getPixelYperMM() * height;
}


float RetParam::EnfaceData::getAverageOnHorzLine(float diam) const
{
	if (isEmpty()) {
		return 0.0f;
	}

	float radius = diam / 2.0f;
	float cent_x, cent_y;
	float dsum, dcnt, davg;
	int pos_x1, pos_x2, pos_y1;

	cent_x = cent_y = 0.0f;
	pos_x1 = (int)getPositionXf(cent_x - radius);
	pos_x2 = (int)getPositionXf(cent_x + radius);
	pos_y1 = (int)getPositionYf(cent_y);

	Mat mat = d_ptr->image.getCvMat();

	dsum = dcnt = davg = 0.0f;
	for (int xpos = pos_x1; xpos <= pos_x2; xpos += 1) {
		dsum += mat.at<float>(pos_y1, xpos);
		dcnt += 1.0f;
	}

	if (dcnt < 1.0f) {
		return 0.0f;
	}

	davg = dsum / dcnt;
	return davg;
}


float RetParam::EnfaceData::getAverageOnVertLine(float diam) const
{
	if (isEmpty()) {
		return 0.0f;
	}

	float radius = diam / 2.0f;
	float cent_x, cent_y;
	float dsum, dcnt, davg;
	int pos_x1, pos_y2, pos_y1;

	cent_x = cent_y = 0.0f;
	pos_y1 = (int)getPositionYf(cent_y - radius);
	pos_y2 = (int)getPositionYf(cent_y + radius);
	pos_x1 = (int)getPositionXf(cent_x);

	Mat mat = d_ptr->image.getCvMat();

	dsum = dcnt = davg = 0.0f;
	for (int ypos = pos_y1; ypos <= pos_y2; ypos += 1) {
		dsum += mat.at<float>(ypos, pos_x1);
		dcnt += 1.0f;
	}

	if (dcnt < 1.0f) {
		return 0.0f;
	}

	davg = dsum / dcnt;
	return davg;
}


float RetParam::EnfaceData::getAverageOnCenter(float diam) const
{
	if (isEmpty()) {
		return 0.0f;
	}

	Mat image = d_ptr->image.getCvMatConst();
	Mat mask1 = Mat::zeros(image.rows, image.cols, CV_8UC1);

	float cent_x, cent_y;
	cent_x = cent_y = 0.0f;

	Point center;
	center.x = getPositionX(cent_x);
	center.y = getPositionY(cent_y);

	float radius = diam / 2.0f;
	Size size = Size((int)(radius*getPixelXperMM()), (int)(radius*getPixelYperMM()));
	cv::ellipse(mask1, center, size, 0.0, 0.0, 360.0, 255, -1);

	float dmean = (float)cv::mean(image, mask1)(0);

	return dmean;
}


float RetParam::EnfaceData::getAverageOnCircle(float diam, int samples) const
{
	if (isEmpty()) {
		return 0.0f;
	}

	Mat mat = d_ptr->image.getCvMat();

	float radian;
	float radius = diam / 2.0f;
	float a_step = 360.0f / samples;
	float curr_x, curr_y;
	float mpos_x, mpos_y;

	float itp_x1, itp_x2, itp_y1, itp_y2;
	float sum_x1, sum_x2, sum_y1, sum_y2;
	float data_x, data_y, data_sum, data_avg;
	int pos_x1, pos_x2, pos_y1, pos_y2;

	data_sum = 0.0f;
	for (int i = 0; i < samples; i++) {
		radian = (float)NumericFunc::degreeToRadian(i * a_step);
		curr_x = (float)radius * cos(radian);
		curr_y = (float)radius * sin(radian);
		mpos_x = getPositionXf(curr_x);
		mpos_y = getPositionYf(curr_y);
		itp_x1 = floor(mpos_x);
		itp_x2 = ceil(mpos_x);
		itp_y1 = floor(mpos_y);
		itp_y2 = ceil(mpos_y);

		pos_x1 = min(max(int(itp_x1), 0), mat.cols - 1);
		pos_x2 = min(max(int(itp_x2), 0), mat.cols - 1);
		pos_y1 = min(max(int(itp_y1), 0), mat.rows - 1);
		pos_y2 = min(max(int(itp_y2), 0), mat.rows - 1);

		sum_x1 = (mat.at<float>(pos_y1, pos_x1) + mat.at<float>(pos_y2, pos_x1)) * 0.5f;
		sum_x2 = (mat.at<float>(pos_y1, pos_x2) + mat.at<float>(pos_y2, pos_x2)) * 0.5f;
		sum_y1 = (mat.at<float>(pos_y1, pos_x1) + mat.at<float>(pos_y1, pos_x2)) * 0.5f;
		sum_y2 = (mat.at<float>(pos_y2, pos_x1) + mat.at<float>(pos_y2, pos_x2)) * 0.5f;

		if (itp_x1 < itp_x2) {
			data_x = sum_x1 * (itp_x2 - mpos_x) + sum_x2 * (mpos_x - itp_x1);
		}
		else {
			data_x = (sum_x1 + sum_x2) * 0.5f;
		}

		if (itp_y1 < itp_y2) {
			data_y = sum_y1 * (itp_y2 - mpos_y) + sum_y2 * (mpos_y - itp_y1);
		}
		else {
			data_y = (sum_y1 + sum_y2) * 0.5f;
		}

		data_sum += (data_x + data_y) * 0.5f;
	}

	data_avg = (data_sum / samples);
	return data_avg;
}


CppUtil::CvImage * RetParam::EnfaceData::getDataImage(void) const
{
	return &d_ptr->image;
}


bool RetParam::EnfaceData::buildDataImage(const std::vector<std::vector<std::uint8_t>>& lines, const std::vector<bool>& flags)
{
	int rows, cols;

	if (d_ptr->pattern.isHorizontal()) {
		rows = numLines();
		cols = lineSize();
	}
	else {
		rows = lineSize();
		cols = numLines();
	}

	Mat mat = Mat::zeros(rows, cols, CV_8UC1);

	if (d_ptr->pattern.isHorizontal()) {
		if (rows != lines.size() || lines.size() <= 0) {
			LogW() << "Enface lines are insufficient, rows: " << rows << ", lines: " << lines.size();
			return false;
		}

		for (int r = 0; r < rows; r++) {
			auto size = lines[r].size();
			if (cols != size) {
				LogW() << "Enface line size is not matched, row: " << r << ", cols: " << cols << ", size: " << lines[r].size();
				return false;
			}

			auto data = lines[r].data();
			auto iptr = (unsigned char*)mat.ptr(r);
			memcpy(iptr, data, size*sizeof(uint8_t));
		}
	}
	else {
		if (cols != lines.size() || lines.size() <= 0) {
			LogW() << "Enface lines are insufficient, cols: " << cols << ", lines: " << lines.size();
			return false;
		}

		for (int c = 0; c < cols; c++) {
			if (rows != lines[c].size()) {
				LogW() << "Enface line size is not matched, col: " << c << ", rows: " << rows << ", size: " << lines[c].size();
				return false;
			}
			auto data = lines[c].data();
			for (int r = 0; r < rows; r++) {
				mat.at<unsigned char>(r, c) = data[r];
			}
		}
	}

	/*
	auto w = lineSize();
	auto h = numLines();
	Mat mat = Mat::zeros(h, w, CV_8UC1);

	if (h > lines.size()) {
		LogW() << "Enface lines are insufficient, height: " << h << ", data size: " << lines.size();
		return false;
	}

	for (int r = 0; r < mat.rows; r++) {
		auto line = lines[r];
		auto p = (unsigned char*)mat.ptr(r);

		if (w != line.size()) {
			LogW() << "Enface line size is not matched, row: " << r << ", size: " << line.size() << ", width: " << w;
			return false;
		}
		memcpy(p, line.data(), line.size()*sizeof(uint8_t));
	}
	*/

	d_ptr->image.getCvMat() = mat;
	return true;
}


bool RetParam::EnfaceData::buildDataImage(const std::vector<std::vector<float>>& lines, const std::vector<bool>& flags)
{
	int rows, cols;

	if (d_ptr->pattern.isRadialScan()) {
		rows = RADIAL_DATA_IMAGE_SIZE; // lineSize();
		cols = RADIAL_DATA_IMAGE_SIZE; // lineSize();
	}
	else if (d_ptr->pattern.isHorizontal()) {
		rows = numLines();
		cols = lineSize();
	}
	else {
		rows = lineSize();
		cols = numLines();
	}

	Mat mat = Mat::zeros(rows, cols, CV_32FC1);

	if (d_ptr->pattern.isRadialScan()) 
	{
		int radius = rows / 2;
		int cent_x = cols / 2;
		int cent_y = rows / 2;
		int n_line = (int)lines.size();

		int count = 0;
		for (int k = 0; k < n_line; k++) {
			if (flags[k]) {
				count++;
			}
		}
		if (count < (n_line / 2) || n_line <= 0) {
			LogD() << "Radial scan segments not enough, count: " << count << ", n_line: " << n_line;
		}
		else {
			int dx, dy, idx1, idx2, sel1, sel2, pos1, pos2;
			int len1, len2;
			float angle, radsqr, r2;
			float r2deg, a_step;
			float ofs1, ofs2, deg1, deg2;

			a_step = 180.0f / n_line;
			r2deg = (180.0f / 3.141592f);
			radsqr = (float) radius; // (float)(radius * radius);

			for (int y = 0; y < rows; y++) {
				for (int x = 0; x < cols; x++) {
					// x = 118; y = 236;
					dx = (cent_x - x);
					dy = (cent_y - y);
					r2 = (float)sqrt((dx * dx + dy * dy));
					if (r2 > radsqr) {
						continue;
					}

					angle = (float)(atan2(dy, dx) * r2deg);
					angle += (angle < 0.0f ? 360.0f : 0.0f);

					idx1 = (int)(angle / a_step);
					idx2 = idx1;

					while (true) {
						sel1 = (idx1 >= n_line ? (idx1 - n_line) : idx1);
						if (flags[sel1]) {
							break;
						}
						idx1 = (--idx1 < 0 ? (2 * n_line - 1) : idx1);
					}

					while (true) {
						idx2 = (++idx2 >= (2 * n_line) ? 0 : idx2);
						sel2 = (idx2 >= n_line ? (idx2 - n_line) : idx2);
						if (flags[sel2]) {
							break;
						}
					}

					/*
					while (true) {
						sel1 = ((idx1 / n_line + 1) * n_line - idx1) % n_line;
						if (flags[sel1]) {
							break;
						}
						idx1 = (--idx1 < 0 ? (2 * n_line - 1) : idx1);
					}

					while (true) {
						idx2 = (++idx2 >= (2 * n_line) ? 0 : idx2);
						sel2 = ((idx2 / n_line + 1) * n_line - idx2) % n_line;
						if (flags[sel2]) {
							break;
						}
					}
					*/

					if (flags[sel1] && flags[sel2])
					{
						deg1 = idx1 * a_step;
						deg2 = idx2 * a_step;

						if (deg2 > deg1) {
							ofs2 = deg2 - angle;
							ofs1 = angle - deg1;
						}
						else {
							ofs2 = (angle < deg2 ? (deg2 - angle) : (deg2 + (360.0f - angle)));
							ofs1 = (angle < deg2 ? (angle + (360.0f - deg1)) : (angle - deg1));
						}

						len1 = (int)lines[sel1].size();
						len2 = (int)lines[sel2].size();
						pos1 = (int)(idx1 < n_line ? (len1 * 0.5f * (1.0f - r2 / radsqr)) : (len1 * 0.5f * (1.0f + (r2 / radsqr))));
						pos1 = (pos1 >= len1 ? len1 - 1 : pos1);
						pos1 = (pos1 < 0 ? 0 : pos1);
						pos2 = (int)(idx2 < n_line ? (len2 * 0.5f * (1.0f - r2 / radsqr)) : (len2 * 0.5f * (1.0f + (r2 / radsqr))));
						pos2 = (pos2 >= len2 ? len2 - 1 : pos2);
						pos2 = (pos2 < 0 ? 0 : pos2);

						mat.at<float>(y, x) = ((lines[sel1][pos1] * ofs2 + lines[sel2][pos2] * ofs1) / (ofs1 + ofs2));
						// DebugOut() << "x: " << x << ", y: " << y << ", " << mat.at<float>(y, x) << " : " << angle << " : " << sel1 << ", " << pos1 << ", " << lines[sel1][pos1] << ", " << sel2 << ", " << pos2 << ", " << lines[sel2][pos2];
						
						// if (y == rows / 2) {
						// 	LogD() << "y: " << y << ", x: " << x << ", pos1: " << pos1 << ", pos2: " << pos2 << ", dx: " << dx << ", dy: " << dy << ", r2: " << r2 << ", rsqr: " << radsqr << ", ratio: " << (r2 / radsqr);
						// }
					}
				}
			}

			/*
			cv::Mat dst;
			cv::flip(mat, dst, 1);
			mat = dst;
			*/
		}

		/*
		for (int y = 0; y < rows; y++) {
			CString test, temp;
			for (int x = 0; x < cols; x++) {
				temp.Format(_T("%03d "), (int)mat.at<float>(y, x));
				test += temp;
			}
			test += _T("\n");
			LogD() << wtoa(test);
		}
		*/
	}
	else if (d_ptr->pattern.isHorizontal()) {
		if (rows != lines.size() || lines.size() <= 0) {
			LogW() << "Enface lines are insufficient, rows: " << rows << ", lines: " << lines.size();
			return false;
		}

		for (int r = 0; r < rows; r++) {
			auto size = lines[r].size();
			if (cols != size) {
				LogW() << "Enface line size is not matched, row: " << r << ", cols: " << cols << ", size: " << lines[r].size();
				return false;
			}

			if (flags[r]) {
				memcpy((unsigned char*)mat.ptr(r), lines[r].data(), size*sizeof(float));
			}
			else {
				int uidx = -1, didx = -1;
				for (int k = r - 1; k >= 0; k--) {
					if (flags[k]) {
						uidx = k;
						break;
					}
				}

				for (int k = r + 1; k < rows; k++) {
					if (flags[k]) {
						didx = k;
						break;
					}
				}

				if (uidx >= 0 && didx >= 0) {
					float uofs = (float)(r - uidx) / (float)(didx - uidx);
					float dofs = (float)(didx - r) / (float)(didx - uidx);
					for (int c = 0; c < cols; c++) {
						mat.at<float>(r, c) = lines[uidx][c] * dofs + lines[didx][c] * uofs;
					}
				}
				else if (uidx >= 0) {
					memcpy((unsigned char*)mat.ptr(r), lines[uidx].data(), lines[uidx].size()*sizeof(float));
				}
				else if (didx >= 0) {
					memcpy((unsigned char*)mat.ptr(r), lines[didx].data(), lines[didx].size()*sizeof(float));
				}
				else {
				}
			}
		}
	}
	else {
		if (cols != lines.size() || lines.size() <= 0) {
			LogW() << "Enface lines are insufficient, cols: " << cols << ", lines: " << lines.size();
			return false;
		}

		for (int c = 0; c < cols; c++) {
			auto size = lines[c].size();
			if (rows != size) {
				LogW() << "Enface line size is not matched, col: " << c << ", rows: " << rows << ", size: " << lines[c].size();
				return false;
			}

			if (flags[c]) {
				auto data = lines[c].data();
				for (int r = 0; r < rows; r++) {
					mat.at<float>(r, c) = data[r];
				}
			}
			else {
				int uidx = -1, didx = -1;
				for (int k = c - 1; k >= 0; k--) {
					if (flags[k]) {
						uidx = k;
						break;
					}
				}

				for (int k = c + 1; k < cols; k++) {
					if (flags[k]) {
						didx = k;
						break;
					}
				}

				if (uidx >= 0 && didx >= 0) {
					float uofs = (float)(c - uidx) / (float)(didx - uidx);
					float dofs = (float)(didx - c) / (float)(didx - uidx);
					for (int r = 0; r < rows; r++) {
						mat.at<float>(r, c) = lines[uidx][r] * dofs + lines[didx][r] * uofs;
					}
				}
				else if (uidx >= 0) {
					auto data = lines[uidx].data();
					for (int r = 0; r < rows; r++) {
						mat.at<float>(r, c) = data[r];
					}
				}
				else if (didx >= 0) {
					auto data = lines[didx].data();
					for (int r = 0; r < rows; r++) {
						mat.at<float>(r, c) = data[r];
					}
				}
				else {
				}
			}
		}
	}

	d_ptr->image.getCvMat() = mat;
	return true;
}


EnfaceData::EnfaceDataImpl & RetParam::EnfaceData::getImpl(void) const
{
	return *d_ptr;
}
