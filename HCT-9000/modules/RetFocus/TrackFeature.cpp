#include "stdafx.h"
#include "TrackFeature.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;


struct TrackFeature::TrackFeatureImpl
{
	CvImage frame;
	CvImage feats;
	
	float entropy;
	float moved_x;
	float moved_y;
	float sim_val;
	float match_val;
	float match_cx;
	float match_cy;

	float frame_mean;
	float frame_stdev;
	float feat_mean;
	float feat_stdev;
	float feat_covar;

	float upp_mean;
	float low_mean;
	bool valid;
	bool covered;

	bool is_disc;
	bool is_od;


	TrackFeatureImpl() {
		initializeTrackFeatureImpl();
	};

	void initializeTrackFeatureImpl(void)
	{
		entropy = 0.0f;
		moved_x = 0.0f;
		moved_y = 0.0f;
		sim_val = 0.0f;

		match_val = 0.0f;
		match_cx = 0.0f;
		match_cy = 0.0f;

		frame_mean = 0.0f;
		frame_stdev = 0.0f;
		feat_mean = 0.0f;
		feat_stdev = 0.0f;
		feat_covar = 0.0f;

		upp_mean = 0.0f;
		low_mean = 0.0f;
		valid = false;
		covered = false;

		is_disc = false;
		is_od = false;
	}
};


TrackFeature::TrackFeature()
	: d_ptr(make_unique<TrackFeatureImpl>())
{
}


TrackFeature::~TrackFeature()
{
}


RetFocus::TrackFeature::TrackFeature(TrackFeature&& rhs) = default;
TrackFeature& RetFocus::TrackFeature::operator=(TrackFeature&& rhs) = default;


bool RetFocus::TrackFeature::setupFrameImage(bool isOD, bool isDisc, const CppUtil::CvImage& image)
{
	if (image.isEmpty()) {
		return false;
	}

	// Frame image.
	if (image.getWidth() != RET_TRACK_CAMERA_FRAME_WIDTH || image.getHeight() != RET_TRACK_CAMERA_FRAME_HEIGHT) {
		CvImage sample;
		image.resizeTo(&sample, RET_TRACK_CAMERA_FRAME_WIDTH, RET_TRACK_CAMERA_FRAME_HEIGHT);
		impl().frame = sample;
	}
	else {
		impl().frame = image;
	}

	impl().is_disc = isDisc;
	impl().is_od = isOD;

	impl().entropy = 0.0f;
	impl().match_val = 0.0f;
	impl().match_cx = 0.0f;
	impl().match_cy = 0.0f;

	impl().moved_x = 0.0f;
	impl().moved_y = 0.0f;
	impl().sim_val = 0.0f;
	impl().valid = false;
	impl().covered = false;

	calculateFeature();
	// calculateEntropy();

	if (checkUpperRegionIsCovered() || checkLowerRegionIsCovered()) {
		impl().covered = true;
		// LogD() << "Retina feature, upper/lower region is covered";
	}

	// LogD() << "Retina frame, feat_mean: " << impl().feat_mean << ", covar: " << impl().feat_covar << ", upp_mean: " << impl().upp_mean << ", low_mean: " << impl().low_mean;
	return true;
}

bool RetFocus::TrackFeature::evaluateValidity(void) const
{
	float mean, stdev, covar;
	mean = impl().feat_mean;
	stdev = impl().feat_stdev;
	covar = impl().feat_covar;

	bool valid = true;
	if (mean < RETT_TRACK_FEATURE_MEAN_MIN || mean > RETT_TRACK_FEATURE_MEAN_MAX) {
		// LogD() << "Retina feature is not valid, mean = " << mean;
		valid = false;
	}
	else if (covar < RETT_TRACK_FEATURE_COEFF_VARS_MIN || covar > RETT_TRACK_FEATURE_COEFF_VARS_MAX) {
		// LogD() << "Retina feature is not valid, covar = " << covar;
		valid = false;
	}
	else if (impl().covered) {
		valid = false;
	}
	
	impl().valid = valid;
	return valid;
}

bool RetFocus::TrackFeature::calculateFeature(void) const
{
	// ROI feature image. 
	int x, y, w, h;
	getFeatureRect(x, y, w, h);

	CvImage roi;
	impl().frame.copyTo(&roi, x, y, w, h);

	float mean, stdev;
	roi.getMeanStddev(&mean, &stdev);
	Mat mat = roi.getCvMatConst();

	// Note that as the average pixel value increases, the contrast of the image
	// (the coefficient of variation) decreases.
	
	/*
	// Closer to optic disc area. 
	float gsum = 0.0f;
	float gcnt = 0.0f;
	float mean2 = 0.0f;

	for (int r = 0; r < mat.rows; r++) {
		auto* ptr = mat.ptr(r);
		for (int c = 0; c < mat.cols; c++) {
			if (ptr[c] >= mean) {
				gsum += mean;
				gcnt += 1;
			}
		}
	}

	if (gcnt > 0) {
		mean2 = gsum / gcnt;
	}
	*/

	impl().feat_mean = mean;
	impl().feat_stdev = stdev;
	impl().feat_covar = (stdev / (mean + 0.1f));
	impl().feats = std::move(roi);
	return true;
}

bool RetFocus::TrackFeature::calculateEntropy(void) const
{
	cv::Mat feat = impl().feats.getCvMat();
	cv::Mat grad_x, grad_y, magn;

	cv::Sobel(feat, grad_x, CV_32F, 1, 0);
	cv::Sobel(feat, grad_y, CV_32F, 0, 1);
	cv::magnitude(grad_x, grad_y, magn);

	int histSize = 256; // Number of bins
	float range[] = { 0, 256 }; // Range of pixel values
	const float* histRange = { range };
	cv::Mat hist;

	cv::calcHist(&feat, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

	// Normalize histogram
	hist /= (double)feat.total();

	// Calculate entropy
	double value = 0.0;
	for (int i = 0; i < histSize; ++i) {
		float p = hist.at<float>(i);
		if (p > 0) {
			value -= p * std::log2(p);
		}
	}

	impl().entropy = (float)value;
	return true;
}

bool RetFocus::TrackFeature::calculateSimilarity(const CppUtil::CvImage& image) const
{
	if (image.isEmpty()) {
		return false;
	}

	int x, y, w, h;
	getFeatureRect(x, y, w, h);

	CvImage fnew;
	image.copyTo(&fnew, x, y, w, h);

	Mat result;
	matchTemplate(impl().feats.getCvMat(), fnew.getCvMat(), result, TM_CCORR_NORMED);

	// Localizing the best match with minMaxLoc
	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc); // , cv::Mat());
	matchLoc = maxLoc;

	impl().match_val = (float)maxVal;
	impl().match_cx = (float)matchLoc.x + w / 2;
	impl().match_cy = (float)matchLoc.y + h / 2;
	LogD() << "Tracking feature, template match: " << impl().match_val << ", cx: " << impl().match_cx << ", cy: " << impl().match_cy;
	return true;
}

bool RetFocus::TrackFeature::calculateSimilarityOnFrame(const CppUtil::CvImage & image) const
{
	if (image.isEmpty()) {
		return false;
	}

	int x, y, w, h;
	getFeatureRect(x, y, w, h);

	Mat frame = image.getCvMat();
	Mat templ = impl().feats.getCvMat();

	Mat result;
	int resRows = frame.rows - templ.rows + 1;
	int resCols = frame.cols - templ.cols + 1;
	result.create(resCols, resRows, CV_32FC1);
	
	matchTemplate(frame, templ, result, TM_CCORR_NORMED);

	// Localizing the best match with minMaxLoc
	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc); // , cv::Mat());
	matchLoc = maxLoc;

	impl().match_val = (float)maxVal;
	impl().match_cx = (float)matchLoc.x + templ.cols / 2;
	impl().match_cy = (float)matchLoc.y + templ.rows / 2;
	LogD() << "Tracking feature, template match on frame: " << impl().match_val << ", cx: " << impl().match_cx << ", cy: " << impl().match_cy;
	return true;
}

bool RetFocus::TrackFeature::estimateMovement(const CppUtil::CvImage& image)
{
	if (image.isEmpty()) {
		return false;
	}

	int x, y, w, h;	
	getFeatureRect(x, y, w, h);

	CvImage fnew;
	image.copyTo(&fnew, x, y, w, h);

	Mat matWarp = Mat::eye(2, 3, CV_32F);
	const int motionType = MOTION_TRANSLATION;
	const int numIters = 500;
	const double termEps = 1e-4;
	const float CC_THRESHOLD = RETT_FEAT_THRESH_TO_COMPARE;

	TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, numIters, termEps);

	try {
		double cc = findTransformECC(impl().feats.getCvMat(), fnew.getCvMat(), matWarp, motionType, criteria);
		float dx = matWarp.at<float>(0, 2);
		float dy = matWarp.at<float>(1, 2);

		impl().sim_val = cc;
		impl().moved_x = dx;
		impl().moved_y = dy;
		LogD() << "Tracking feature, transform cc: " << cc << ", dx: " << dx << ", dy: " << dy;
		/*
		if (cc < CC_THRESHOLD) {
			return false;
		}
		*/
		return true;
	}
	catch (...) {
		impl().sim_val = 0.0f;
		impl().moved_x = 0.0f;
		impl().moved_y = 0.0f;
		// LogD() << ex.what() << std::endl;
		LogD() << "Tracking feature, estimating movements failed";
		return false;
	}
}

bool RetFocus::TrackFeature::estimateMovementOnFrame(const CppUtil::CvImage & image) const
{
	if (image.isEmpty()) {
		return false;
	}

	int x, y, w, h;
	getFeatureRect(x, y, w, h);

	int mx = (int)impl().match_cx;
	int my = (int)impl().match_cy;

	CvImage fnew;
	image.copyTo(&fnew, mx - w / 2, my - h / 2, w, h);

	Mat matWarp = Mat::eye(2, 3, CV_32F);
	const int motionType = MOTION_TRANSLATION;
	const int numIters = 500;
	const double termEps = 1e-4;
	const float CC_THRESHOLD = RETT_FEAT_THRESH_TO_COMPARE;

	TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, numIters, termEps);

	try {
		double cc = findTransformECC(impl().feats.getCvMat(), fnew.getCvMat(), matWarp, motionType, criteria);
		float dx = matWarp.at<float>(0, 2);
		float dy = matWarp.at<float>(1, 2);
		float kx = mx - (x + w / 2) + dx;
		float ky = my - (y + h / 2) + dy;

		impl().sim_val = cc;
		impl().moved_x = kx;
		impl().moved_y = ky;
		LogD() << "Tracking feature, transform cc: " << cc << ", dx: " << dx << ", dy: " << dy << ", mx: " << mx << ", my: " << my << " => " << kx << ", " << ky;
		
		if (cc < CC_THRESHOLD) {
			return false;
		}
		return true;
	}
	catch (...) {
		// LogD() << ex.what() << std::endl;
		LogD() << "Tracking feature, estimating movements failed";
		impl().sim_val = 0.0f;
		impl().moved_x = 0.0f;
		impl().moved_y = 0.0f;
		return false;
	}
}

float RetFocus::TrackFeature::getMovedX(void) const
{
	return impl().moved_x;
}

float RetFocus::TrackFeature::getMovedY(void) const
{
	return impl().moved_y;
}

float RetFocus::TrackFeature::getFeatureMean(void) const
{
	return impl().feat_mean;
}

float RetFocus::TrackFeature::getFeatureStdev(void) const
{
	return impl().feat_stdev;
}

float RetFocus::TrackFeature::getSimilarity(void) const
{
	return impl().sim_val;
}

float RetFocus::TrackFeature::getEntropy(void) const
{
	return impl().entropy;
}

float RetFocus::TrackFeature::getCoefficientOfVariation(void) const
{
	return impl().feat_covar;
}

bool RetFocus::TrackFeature::isEmpty(void) const
{
	return impl().frame.isEmpty();
}

bool RetFocus::TrackFeature::isFeatureCovered(void) const
{
	return impl().covered;
}

bool RetFocus::TrackFeature::isValidFeature(void) const
{
	return impl().valid;
}

void RetFocus::TrackFeature::getFeatureRect(int& sx, int& sy, int& width, int& height) const
{
	int w = RET_TRACK_FEATURE_X_SIZE;
	int h = RET_TRACK_FEATURE_Y_SIZE;
	int x = RET_TRACK_CAMERA_FRAME_CENT_X;
	int y = RET_TRACK_CAMERA_FRAME_CENT_Y - RET_TRACK_FEATURE_Y_SIZE / 2;

	bool isOD = impl().is_od;
	bool isDisc = impl().is_disc;

	if (isDisc) {
		w = RET_TRACK_FEATURE_X_SIZE_DISC;
		h = RET_TRACK_FEATURE_Y_SIZE_DISC;
		x = RET_TRACK_CAMERA_FRAME_CENT_X - RET_TRACK_FEATURE_X_SIZE_DISC / 2;
		y = RET_TRACK_CAMERA_FRAME_CENT_Y - RET_TRACK_FEATURE_Y_SIZE_DISC / 2;
	}
	else {
		if (isOD) {
			x = RET_TRACK_CAMERA_FRAME_CENT_X + (RET_TRACK_FEATURE_X_OFFSET);
		}
		else {
			x = RET_TRACK_CAMERA_FRAME_CENT_X - (RET_TRACK_FEATURE_X_OFFSET + RET_TRACK_FEATURE_X_SIZE);
		}
		y = (RET_TRACK_CAMERA_FRAME_CENT_Y + RET_TRACK_FEATURE_Y_OFFSET) - (RET_TRACK_FEATURE_Y_SIZE / 2);
	}

	sx = x; sy = y; width = w; height = h;
	return;
}

bool RetFocus::TrackFeature::checkUpperRegionIsCovered(void) const
{
	int w = RET_TRACK_UPSIDE_X_SIZE;
	int h = RET_TRACK_UPSIDE_Y_SIZE;
	int x = (RET_TRACK_CAMERA_FRAME_CENT_X + RET_TRACK_UPSIDE_X_OFFSET) - w / 2;
	int y = (RET_TRACK_CAMERA_FRAME_CENT_Y - RET_TRACK_UPSIDE_Y_OFFSET) - h;

	CvImage roi;
	impl().frame.copyTo(&roi, x, y, w, h);

	float mean, stdev;
	roi.getMeanStddev(&mean, &stdev);

	impl().upp_mean = mean;
	if (mean > RETT_TRACK_EYELID_COVERED_MEAN) {
		return true;
	}
	return false;
}

bool RetFocus::TrackFeature::checkLowerRegionIsCovered(void) const
{
	int w = RET_TRACK_DOWNSIDE_X_SIZE;
	int h = RET_TRACK_DOWNSIDE_Y_SIZE;
	int x = (RET_TRACK_CAMERA_FRAME_CENT_X + RET_TRACK_DOWNSIDE_X_OFFSET) - w / 2;
	int y = (RET_TRACK_CAMERA_FRAME_CENT_Y + RET_TRACK_DOWNSIDE_Y_OFFSET);

	CvImage roi;
	impl().frame.copyTo(&roi, x, y, w, h);

	float mean, stdev;
	roi.getMeanStddev(&mean, &stdev);

	impl().low_mean = mean;
	if (mean > RETT_TRACK_EYELID_COVERED_MEAN) {
		return true;
	}
	return false;
}


TrackFeature::TrackFeatureImpl& RetFocus::TrackFeature::impl(void) const
{
	return *d_ptr;
}
