#include "stdafx.h"
#include "DiscSession.h"

#include "CppUtil2.h"
#include "OctData2.h"
#include "SegmScan2.h"
#include "SemtRetina2.h"
/*
#ifdef __USE_SEMT_SEGM__
#include "SemtSegm2.h"
#endif
*/

#include "opencv.hpp"

using namespace SegmProc;
/*
#ifdef __USE_SEMT_SEGM__
using namespace SemtSegm;
#endif
*/
using namespace SegmScan;
using namespace OctData;
using namespace CppUtil;
using namespace std;
using namespace cv;

struct DiscSession::DiscSessionImpl
{
	Mat enfaceBMO;
	vector<Point> discPoint;
	vector<Point> smoothContour;

	int enfaceHeight, enfaceWidth;

	DiscSessionImpl() {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<DiscSession::DiscSessionImpl> DiscSession::d_ptr(new DiscSessionImpl());


DiscSession::DiscSession()
{
}


DiscSession::~DiscSession()
{
}


bool SegmProc::DiscSession::initialize(void)
{
	int overlaps = getScanData()->getDescript().getScanOverlaps();

	clearOcularBsegms();
	for (int i = 0; i < SegmSession::getScanData()->countBscanData(); i++) {
		auto bscan = getScanData()->getBscanData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th bscan data is empty!";
			return false;
		}

		auto bsegm = make_unique<DiscBsegm>();
		// if (!(i % overlaps)) {
			bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		// }
		bsegm->setPatternDescript(getScanData()->getDescript());
		addOcularBsegm(std::move(bsegm));
	}
	LogD() << "DiscSession measure bscans: " << getScanData()->countBscanData();

	clearPreviewBsegms();
	for (int i = 0; i < SegmSession::getScanData()->countPreviewData(); i++) {
		auto bscan = getScanData()->getPreviewData(i);
		if (bscan == nullptr || bscan->isEmpty()) {
			LogW() << i << "th preview data is empty!";
			return false;
		}

		auto bsegm = make_unique<DiscBsegm>();
		bsegm->loadSource(bscan->getImageBuffer(), bscan->getImageWidth(), bscan->getImageHeight());
		bsegm->setPatternDescript(getScanData()->getDescript());
		addPreviewBsegm(std::move(bsegm));
	}
	LogD() << "DiscSession preview bscans: " << getScanData()->countPreviewData();

	try {
		if (!SemtRetina::RetinaSegmentModel::isInitialized()) {
			SemtRetina::RetinaSegmentModel::initializeNetwork();
		}
	}
	catch (exception e) {
	}
/*
#ifdef __USE_SEMT_SEGM__
	SemtSegm::InferenceModel::initializeNetwork();
#endif
*/

	return true;
}


bool SegmProc::DiscSession::rectify(void)
{
	determineDiscRange();
	return true;
}


void SegmProc::DiscSession::determineDiscRange(void)
{
	if (filterOpticDiscSet()) {

	}

	auto bsegm = getOcularBsegm(0);
	auto pattern = bsegm->getPatternDescript();
	auto patternName = pattern.getPatternName();
	
	// if ((patternName == PatternName::Disc3D || patternName == PatternName::MacularDisc) && pattern.getScanRangeX() < 12.0f)
	{
		d_ptr->enfaceHeight = 512;
		d_ptr->enfaceWidth = 512;

		if (!makeBRMImage(pattern)) {
			return;
		}
		elaborateDiscRange(pattern);
	}
	return;
}

bool SegmProc::DiscSession::makeBRMImage(OctScanPattern pattern) 
{
	auto lines = vector<vector<uint8_t>>();
	auto flags = vector<bool>();
	auto pixels = vector<uint8_t>();
	auto discVector = vector<Point>();

	PatternName patternName = pattern.getPatternName();

	bool output;
	int rows, cols;

	if (pattern.isHorizontal()) {
		rows = pattern.getNumberOfScanLines();
		cols = pattern.getNumberOfScanPoints();
	}
	else {
		rows = pattern.getNumberOfScanPoints();
		cols = pattern.getNumberOfScanLines();
	}

	auto size = countOcularBsegms();
	Mat enface = Mat::zeros(rows, cols, CV_8UC1);

	for (int i = 0; i < size; i++) {
		auto bsegm = getOcularBsegm(i);
		if (bsegm != nullptr) {
			int x1, x2;
			Point disc1, disc2;

			output = bsegm->getMaxValueLine(OcularLayerType::IOS, OcularLayerType::BASE, pixels);
			lines.push_back(std::move(pixels));
			flags.push_back(output);

			if (bsegm->getOpticNerveDiscRange(x1, x2)) {
				disc1.x = ((float)x1 / pattern.getNumberOfScanPoints() * d_ptr->enfaceWidth + 0.5f);
				disc1.y = ((float)i / (pattern.getNumberOfScanLines() - 1) * d_ptr->enfaceHeight + 0.5f);
				disc2.x = ((float)x2 / pattern.getNumberOfScanPoints() *d_ptr->enfaceWidth + 0.5f);
				disc2.y = ((float)i / (pattern.getNumberOfScanLines() - 1) * d_ptr->enfaceHeight + 0.5f);

				if (pattern.getDirection() == 1) {
					std::swap(disc1.x, disc1.y);
					std::swap(disc2.x, disc2.y);
				}
				if (x1 > 1 && x2 > 1 && i >= size / 3 && i <= size / 3 * 2) {
					discVector.insert(discVector.begin(), disc1);
					discVector.insert(discVector.end(), disc2);
				}
			}
		}
		else {
			return false;
		}
	}

	if (pattern.isHorizontal()) {
		if (rows != lines.size() || lines.size() <= 0) {
			LogW() << "BMO Enface lines are insufficient, rows: " << rows << ", lines: " << lines.size();
			return false;
		}

		for (int r = 0; r < rows; r++) {
			auto size = lines[r].size();
			if (cols != size) {
				LogW() << "BMO Enface line size is not matched, row: " << r << ", cols: " << cols << ", size: " << lines[r].size();
				return false;
			}

			auto data = lines[r].data();
			auto iptr = (unsigned char*)enface.ptr(r);
			memcpy(iptr, data, size * sizeof(uint8_t));
		}
	}
	else {
		if (cols != lines.size() || lines.size() <= 0) {
			LogW() << "BMO Enface lines are insufficient, cols: " << cols << ", lines: " << lines.size();
			return false;
		}

		for (int c = 0; c < cols; c++) {

			if (rows != lines[c].size()) {
				LogW() << "BMO Enface line size is not matched, col: " << c << ", rows: " << rows << ", size: " << lines[c].size();
				return false;
			}

			auto data = lines[c].data();
			for (int r = 0; r < rows; r++) {
				enface.at<unsigned char>(r, c) = data[r];
			}
		}
	}
	cv::resize(enface, enface, Size(d_ptr->enfaceHeight, d_ptr->enfaceWidth), INTER_CUBIC);

	getImpl().enfaceBMO = enface;
	getImpl().discPoint = discVector;

	return true;
}


bool SegmProc::DiscSession::elaborateDiscRange(OctScanPattern pattern)
{
	Mat enfaceImg;
	int rows, cols;
	bool horizontal;

	if (pattern.isHorizontal()) {
		rows = pattern.getNumberOfScanLines();
		cols = pattern.getNumberOfScanPoints();
		horizontal = true;
	}
	else {
		rows = pattern.getNumberOfScanPoints();
		cols = pattern.getNumberOfScanLines();
		horizontal = false;
	}
	getImpl().enfaceBMO.copyTo(enfaceImg);

	preprocessingEnface(enfaceImg); //Thresholding and Ellipse Mask
	findSmoothContour(enfaceImg);	

	vector<Point> resizePoint;
	Mat resizeDisc = Mat::zeros(rows, cols, CV_8U);		//make image for detecting disc point 
	
	for (auto iter : getImpl().smoothContour) {
		float ratioX = (float)cols / (float)enfaceImg.cols;
		float ratioY = (float)rows / (float)enfaceImg.rows;
		cv::Point disc;

		disc.x = (float)iter.x * ratioX;
		disc.y = (float)iter.y * ratioY;

		resizePoint.push_back(disc);
	}
	cv::fillConvexPoly(resizeDisc, resizePoint, cv::Scalar(255, 255, 255)); 

	modifyDiscPoint(resizeDisc, rows, cols, horizontal);
	
	return true;
}

bool SegmProc::DiscSession::findSmoothContour(Mat& enfaceImg)
{
	Mat distanceImg;
	vector<vector<Point> > contourPoint;
	vector<vector<Point> > contourDistanceImg;

	findContours(enfaceImg, contourPoint, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	Mat contourImg = Mat::zeros(enfaceImg.size(), CV_8U);
	for (size_t i = 0; i < contourPoint.size(); i++)
	{
		drawContours(contourImg, contourPoint, static_cast<int>(i), Scalar(static_cast<int>(i) + 1), -1);
	}

	cv::threshold(contourImg, contourImg, 0, 255, CV_THRESH_BINARY);
	distanceTransform(contourImg, distanceImg, DIST_L2, 3);

	if (getImpl().discPoint.size() <= 6) {
		double radius;
		int maxIdx[2];
		minMaxIdx(distanceImg, NULL, &radius, NULL, maxIdx);

		Point center(maxIdx[1], maxIdx[0]);
		cv::Mat elipseMask = cv::Mat::ones(enfaceImg.size(), CV_8UC1);

		cv::ellipse(elipseMask, center, Size(radius, radius * 1.3), 180, 0, 360, Scalar(0, 0, 0), -1);
		distanceImg.setTo(0, elipseMask);
	}

	normalize(distanceImg, distanceImg, 0, 1.0, NORM_MINMAX);
	threshold(distanceImg, distanceImg, 0.4, 255, CV_THRESH_BINARY);
	distanceImg.convertTo(distanceImg, CV_8UC1);

	vector<Vec4i> hierarchy;
	findContours(distanceImg, contourDistanceImg, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0));

	if (contourDistanceImg.size() < 1) {
		return false;
	}

	vector<Point> longContour = contourDistanceImg[0];
	for (auto iter : contourDistanceImg) {
		if (iter.size() > longContour.size()) {
			longContour = iter;
		}
	}

	vector<Point> scaleContour;
	cv::Rect rct = cv::boundingRect(longContour);

	std::vector<cv::Point> dc_contour;
	cv::Point rct_offset(-rct.tl().x, -rct.tl().y);

	dc_contour.clear();
	dc_contour.resize(longContour.size());
	for (int j = 0; j < longContour.size(); j++)
		dc_contour[j] = longContour[j] + rct_offset;

	std::vector<cv::Point> dc_contour_scale(dc_contour.size());

	float scale = 1.71;
	for (int i = 0; i < dc_contour.size(); i++)
		dc_contour_scale[i] = dc_contour[i] * scale;

	cv::Rect rct_scale = cv::boundingRect(dc_contour_scale);

	cv::Point offset((rct.width - rct_scale.width) / 2, (rct.height - rct_scale.height) / 2);
	offset -= rct_offset;
	scaleContour.clear();
	scaleContour.resize(dc_contour_scale.size());

	for (int i = 0; i < dc_contour_scale.size(); i++)
	{
		scaleContour[i] = dc_contour_scale[i] + offset;
	}

	int k = 65;
	cv::Mat roughDisc;
	roughDisc = cv::Mat(scaleContour);
	vector<vector<Point> > smoothDisc(1);
	roughDisc.copyTo(smoothDisc[0]);

	cv::copyMakeBorder(roughDisc, roughDisc, (k - 1) / 2, (k - 1) / 2, 0, 0, cv::BORDER_WRAP);
	cv::blur(roughDisc, roughDisc, cv::Size(1, k), cv::Point(-1, -1));
	roughDisc.rowRange(cv::Range((k - 1) / 2, 1 + roughDisc.rows - (k - 1) / 2)).copyTo(smoothDisc[0]);

	contourImg = Scalar(0);
	for (size_t i = 0; i < smoothDisc.size(); i++)
	{
		drawContours(contourImg, smoothDisc, static_cast<int>(i), Scalar(255, 255, 255), -1);
	}

	getImpl().smoothContour = smoothDisc[0];

	return true;
}

bool SegmProc::DiscSession::preprocessingEnface(Mat& enfaceImg)
{
	equalizeHist(enfaceImg, enfaceImg);
	auto clahe = cv::createCLAHE(2.0, cv::Size(11, 11));
	clahe->apply(enfaceImg, enfaceImg);

	GaussianBlur(enfaceImg, enfaceImg, Size(33, 33), 0);
	cv::adaptiveThreshold(enfaceImg, enfaceImg, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 57, 0);

	cv::Mat elipseMask = cv::Mat::ones(enfaceImg.size(), CV_8UC1);
	int majorAxis, minorAxis;
	if (getImpl().discPoint.size() > 6) {
		auto rect = fitEllipseDirect(getImpl().discPoint);
		majorAxis = rect.size.height / 2;
		minorAxis = rect.size.width / 2;
		/*
#ifdef __USE_SEMT_SEGM__
		if (isSegmentVersion2()) {
			majorAxis *= 1.1f;
			minorAxis *= 1.15f;
		}
		else 
#endif
		*/
		if (SemtRetina::RetinaSegmentModel::isInitialized()) {
			majorAxis *= 1.1f;
			minorAxis *= 1.15f;
		}
		else 
		{
			majorAxis *= 1.25f;
			minorAxis *= 1.3f;
		}
		
		cv::ellipse(elipseMask, rect.center, Size(minorAxis, majorAxis), rect.angle, 0, 360, Scalar(0, 0, 0), -1);
		enfaceImg.setTo(0, elipseMask);
	}
	else {
		circle(elipseMask, Point(enfaceImg.rows / 2, enfaceImg.cols / 2), enfaceImg.cols / 4, Scalar(0, 0, 0), -1);
		enfaceImg.setTo(0, elipseMask);
	}

	Mat ker = getStructuringElement(MORPH_ELLIPSE, Size(11, 11), Point(1, 1));
	cv::morphologyEx(enfaceImg, enfaceImg, MORPH_CLOSE, ker, Point(-1, -1), 1);

	return true;
}

bool SegmProc::DiscSession::modifyDiscPoint(Mat& resizeDisc, int rows, int cols, bool horizontal)
{
	if (horizontal)
	{
		for (int i = 0; i < rows; i++) {
			cv::Point pt1, pt2;
			cv::Point disc1 = -1;
			cv::Point disc2 = -1;

			bool found = FALSE;
			bool fin = FALSE;

			pt1.x = 0;
			pt1.y = i;

			pt2.x = cols - 1;
			pt2.y = i;

			LineIterator lineItDisc(resizeDisc, pt1, pt2, 4);

			for (int k = 0; k < lineItDisc.count; k++, ++lineItDisc)
			{
				if (resizeDisc.at<uchar>(lineItDisc.pos()) > 0 && found == FALSE)
				{
					disc1 = lineItDisc.pos();
					found = TRUE;
					fin = FALSE;
				}
				if (resizeDisc.at<uchar>(lineItDisc.pos()) <= 0 && found == TRUE)
				{
					disc2 = lineItDisc.pos();
					found = FALSE;
					fin = TRUE;
					break;
				}
			}

			auto bsegm = getOcularBsegm(i);

			bsegm->setOpticNerveDiscRange(disc1.x, disc2.x);
			bsegm->elaborateParams(bsegm->getRetinaLayers()->getILM(), bsegm->getRetinaLayers()->getBRM());
		}
	}
	else
	{
		for (int i = 0; i < cols; i++) {
			cv::Point pt1, pt2;
			cv::Point disc1 = -1;
			cv::Point disc2 = -1;

			bool found = FALSE;
			bool fin = FALSE;

			pt1.x = i;
			pt1.y = 0;

			pt2.x = i;
			pt2.y = rows - 1;

			LineIterator lineItDisc(resizeDisc, pt1, pt2, 4);

			for (int k = 0; k < lineItDisc.count; k++, ++lineItDisc)
			{
				if (resizeDisc.at<uchar>(lineItDisc.pos()) > 0 && found == FALSE)
				{
					disc1 = lineItDisc.pos();
					found = TRUE;
					fin = FALSE;
				}
				if (resizeDisc.at<uchar>(lineItDisc.pos()) <= 0 && found == TRUE)
				{
					disc2 = lineItDisc.pos();
					found = FALSE;
					fin = TRUE;
					break;
				}
			}

			auto bsegm = getOcularBsegm(i);

			bsegm->setOpticNerveDiscRange(disc1.y, disc2.y);
			bsegm->elaborateParams(bsegm->getRetinaLayers()->getILM(), bsegm->getRetinaLayers()->getBRM());
		}
	}

	return true;
}


bool SegmProc::DiscSession::adjustOpticDiscSet(void)
{
	return true;
}

bool SegmProc::DiscSession::isSegmentVersion2(void)
{
	/*
#ifdef __USE_SEMT_SEGM__
	auto bsegm = getOcularBsegm(0);
	auto pattern = bsegm->getPatternDescript();
	auto patternName = pattern.getPatternName();

	if (pattern.isDiscScan()){ // && pattern.getScanRangeX() < 9.0f) {
		if (InferenceModel::isInitialized()) {
			return true;
		}
	}
#endif
	*/
	return false;
}


bool SegmProc::DiscSession::isSemanticVersion2(void)
{
	if (SemtRetina::RetinaSegmentModel::isInitialized()) {
		return true;
	}
	return false;
}


bool SegmProc::DiscSession::filterOpticDiscSet(void)
{
	auto list = std::vector<int>();
	int size = countOcularBsegms();
	for (int i = 0; i < size; i++) {
		auto p = getOcularBsegm(i);
		if (p) {
			if (p->isOpticNerveDisc()) {
				list.push_back(i);
			}
		}
	}

	float rangeY = getOcularBsegm(0)->getPatternDescript().getScanRangeY();
	int linePerMM = (int)ceil(size / rangeY);
	int emptyLinesMax = min(max((int)(linePerMM * 0.5f), 3), 9);
	// int discLinesMin = min(max((int)(linePerMM * 0.35f), 3), 9);

	// const int EMPTY_DISC_LINES_MAX = 5; //  3;
	const int VALID_DISC_LINES_MIN = 3; // 5;

	auto founds = list.size();
	if (founds > 0)
	{
		int cidx = list[founds / 2];
		int none = 0;
		for (int k = cidx; k >= 0; k--) {
			auto p = getOcularBsegm(k);
			if (p && p->isOpticNerveDisc()) {
				none = 0;
			}
			else {
				if (++none > emptyLinesMax) {
					for (; k >= 0; k--) {
						getOcularBsegm(k)->clearOpticNerveRange();
					}
				}
			}
		}

		none = 0;
		for (int k = cidx; k < size; k++) {
			auto p = getOcularBsegm(k);
			if (p && p->isOpticNerveDisc()) {
				none = 0;
			}
			else {
				if (++none > emptyLinesMax) {
					for (; k < size; k++) {
						getOcularBsegm(k)->clearOpticNerveRange();
					}
				}
			}
		}

		int count = 0;
		for (int i = 0; i < size; i++) {
			auto p = getOcularBsegm(i);
			if (p && p->isOpticNerveDisc()) {
				count++;
			}
		}

		if (count < VALID_DISC_LINES_MIN) {
			for (int i = 0; i < size; i++) {
				auto p = getOcularBsegm(i);
				if (p && p->isOpticNerveDisc()) {
					getOcularBsegm(i)->clearOpticNerveRange();
				}
			}
			return false;
		}
		return true;
	}
	return false;
}


DiscSession::DiscSessionImpl & SegmProc::DiscSession::getImpl(void)
{
	return *d_ptr;
}
