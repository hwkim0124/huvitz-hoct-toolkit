#include "stdafx.h"
#include "TrackingObject.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;


struct TrackingObject::TrackingObjectImpl
{
    CvImage frame;
    CvImage ret_patch;
    CvImage roi_patch;
    CvImage obj_patch;
    CvImage tpl_patch;
    CvImage upside;
    CvImage downside;

    bool is_eye_od;
    bool is_disc_scan;

    float ret_mean;
    float ret_stdev;
    float roi_mean;
    float roi_stdev;
    float obj_mean;
    float obj_stdev;
    float upside_mean;
    float upside_stdev;
    float downside_mean;
    float downside_stdev;

    float obj_cx;
    float obj_cy;

    float match_score;
    float match_cx;
    float match_cy;

    TrackingObjectImpl() {
        initializeTrackingObjectImpl();
    }

    void initializeTrackingObjectImpl(void) {
        is_eye_od = false;
        is_disc_scan = false;

        ret_mean = 0.0f;
        ret_stdev = 0.0f;
        roi_mean = 0.0f;
        roi_stdev = 0.0f;
        obj_mean = 0.0f;
        obj_stdev = 0.0f;
        upside_mean = 0.0f;
        upside_stdev = 0.0f;
        downside_mean = 0.0f;
        downside_stdev = 0.0f;

        obj_cx = 0.0f;
        obj_cy = 0.0f;

        match_score = 0.0f;
        match_cx = 0.0f;
        match_cy = 0.0f;
    }
};

TrackingObject::TrackingObject()
    : d_ptr(make_unique<TrackingObjectImpl>())
{
    initializeTrackingObject();
}

TrackingObject::~TrackingObject()
{

}


RetFocus::TrackingObject::TrackingObject(TrackingObject && rhs) = default;
TrackingObject & RetFocus::TrackingObject::operator=(TrackingObject && rhs) = default;


void RetFocus::TrackingObject::initializeTrackingObject(void)
{
    impl().initializeTrackingObjectImpl();
}


bool RetFocus::TrackingObject::assignTrackingFrame(bool isOD, bool isDisc, const CppUtil::CvImage& frame)
{
    if (frame.isEmpty()) {
        return false;
    }

    initializeTrackingObject();

    if (frame.getWidth() != RETINA_TRACK_FRAME_WIDTH || frame.getHeight() != RETINA_TRACK_FRAME_HEIGHT) {
        CvImage sample;
        frame.resizeTo(&sample, RETINA_TRACK_FRAME_WIDTH, RETINA_TRACK_FRAME_HEIGHT);
        impl().frame = sample;
    }
    else {
        impl().frame = frame;
    }

    // Denoise mildly. or Gaussian with sigma = 1.0
    // impl().frame.applyGuidedFilter();

    impl().is_eye_od = isOD;
    impl().is_disc_scan = isDisc;
    return true;
}

bool RetFocus::TrackingObject::assessTrackingFrame(void) const
{
    estimateRetinaRegion();
    estimateObjectRegion();
    return true;
}

bool RetFocus::TrackingObject::detectTargetInFrame(const CppUtil::CvImage& target) const
{
    if (target.isEmpty() || impl().frame.isEmpty()) {
        return false;
    }
 
    bool is_od = impl().is_eye_od;
    bool is_disc = impl().is_disc_scan;

    CvImage templ;
    target.copyTo(&templ);
    
    Mat result;
    matchTemplate(impl().frame.getCvMat(), templ.getCvMat(), result, TM_CCORR_NORMED);

    double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    
	int targ_w = target.getWidth();
	int targ_h = target.getHeight();

    float score = (float)maxVal;
    float cx = (float)(maxLoc.x + targ_w / 2.0f);
    float cy = (float)(maxLoc.y + targ_h / 2.0f);

    CvImage patch;
    impl().frame.copyTo(&patch, maxLoc.x, maxLoc.y, targ_w, targ_h);

	LogD() << "Target detected, match score: " << score << ", cx: " << cx << ", cy: " << cy << ", targ_w: " << targ_w << ", targ_h: " << targ_h;


    cv::Mat a = templ.getCvMat().clone();
    cv::Mat b = patch.getCvMat().clone();
    a.convertTo(a, CV_32F);
    b.convertTo(b, CV_32F);

    /*
    cv::Mat hann;
    if (hann.empty() || hann.size() != patch.getCvMat().size()) {
        cv::createHanningWindow(hann, patch.getCvMat().size(), CV_32F);
    }
    */

    double reponse;
    auto shift = cv::phaseCorrelate(a, b, cv::noArray(), &reponse);

    if (fabs(shift.x) < 0.3f && fabs(shift.y) < 0.3f && reponse > 0.85f) {
        cx = cx + (float)shift.x;
        cy = cy + (float)shift.y;
    }

	int roi_x1, roi_y1, roi_w, roi_h;
	getRoiRegion(is_od, is_disc, roi_x1, roi_y1, roi_w, roi_h);
	int roi_x2 = roi_x1 + roi_w;
	int roi_y2 = roi_y1 + roi_h;

	float x1 = cx - targ_w / 2.0f;
	float x2 = cx + targ_w / 2.0f;
	float y1 = cy - targ_h / 2.0f;
	float y2 = cy + targ_h / 2.0f;

	if (cx < roi_x1 || cx > roi_x2 || cy < roi_y1 || cy > roi_y2) {
		score = 0.0f;
		LogD() << "Out of roi region, x1: " << roi_x1 << ", x2: " << roi_x2 << ", y1: " << roi_y1 << ", y2: " << roi_y2;
	}

    impl().tpl_patch = patch;
    impl().match_score = score;
    impl().match_cx = cx;
    impl().match_cy = cy;
	LogD() << "Target correlated, response: " << reponse << ", shift: " << shift.x << ", " << shift.y << " => cx: " << cx << ", cy : " << cy << ", score: " << score;
    return true;
}


bool RetFocus::TrackingObject::checkEyelidCovered(void) const
{
    auto upside_mean = impl().upside_mean;
    auto downside_mean = impl().downside_mean;

    if (upside_mean >= RETINA_TRACK_UPSIDE_COVERED_MEAN || downside_mean >= RETINA_TRACK_DOWNSIDE_COVERED_MEAN) {
        LogD() << "Retina upside or downside covered, mean: " << upside_mean << ", " << downside_mean;
        return true;
    }
    return false;
}

bool RetFocus::TrackingObject::verifyObjectWithinBox(void) const
{
    if (impl().obj_patch.isEmpty()) {
        LogD() << "Retina object patch is empty";
        return false;
    }

    auto mean = impl().obj_mean;
    auto stdev = impl().obj_stdev;

    if (mean >= RETINA_TRACK_OBJECT_MEAN_MIN && mean <= RETINA_TRACK_OBJECT_MEAN_MAX && stdev >= RETINA_TRACK_OBJECT_STDEV_MIN) {
        LogD() << "Retina object verified, mean: " << mean << ", stdev: " << stdev;
        return true;
    }

    LogD() << "Retina target not verified, mean: " << mean << ", stdev: " << stdev;
    return false;
}

CppUtil::CvImage RetFocus::TrackingObject::getObjectPatch(void) const
{
    return impl().obj_patch;
}

float RetFocus::TrackingObject::objectCenterX(void) const
{
    return impl().obj_cx;
}

float RetFocus::TrackingObject::objectCenterY(void) const
{
    return impl().obj_cy;
}

CppUtil::CvImage RetFocus::TrackingObject::getTargetPatch(void) const
{
    return impl().tpl_patch;
}

float RetFocus::TrackingObject::targetMatchScore(void) const
{
    return impl().match_score;
}

float RetFocus::TrackingObject::targetMatchedX(void) const
{
    return impl().match_cx;
}

float RetFocus::TrackingObject::targetMatchedY(void) const
{
    return impl().match_cy;
}

void RetFocus::TrackingObject::setTargetMatched(float x, float y, float score)
{
	impl().match_cx = x;
	impl().match_cy = y;
	impl().match_score = score;
	return;
}


bool RetFocus::TrackingObject::estimateRetinaRegion(void) const
{
    bool is_od = impl().is_eye_od;
    bool is_disc = impl().is_disc_scan;

    int ret_x, ret_y, ret_w, ret_h;
    getRetinaRegion(is_od, is_disc, ret_x, ret_y, ret_w, ret_h);

    CvImage patch;
    impl().frame.copyTo(&patch, ret_x, ret_y, ret_w, ret_h);
    impl().ret_patch = patch;

    float mean, stdev;
    patch.getMeanStddev(&mean, &stdev);
    impl().ret_mean = mean;
    impl().ret_stdev = stdev;
    // LogD() << "Retina patch, mean: " << mean << ", stdev: " << stdev << ", region: " << ret_x << ", " << ret_y << ", " << ret_w << ", " << ret_h;
    
    int upside_x, upside_y, upside_w, upside_h;
    getUpsideRegion(upside_x, upside_y, upside_w, upside_h);

    CvImage upside;
    impl().frame.copyTo(&upside, upside_x, upside_y, upside_w, upside_h);
    impl().upside = upside;

    upside.getMeanStddev(&mean, &stdev);
    impl().upside_mean = mean;
    impl().upside_stdev = stdev;
    //LogD() << "Retina upside, mean: " << mean << ", stdev: " << stdev << ", region: " << x << ", " << y << ", " << w << ", " << h;
    
    int downside_x, downside_y, downside_w, downside_h;
    getDownsideRegion(downside_x, downside_y, downside_w, downside_h);

    CvImage downside;
    impl().frame.copyTo(&downside, downside_x, downside_y, downside_w, downside_h);
    impl().downside = downside;

    downside.getMeanStddev(&mean, &stdev);
    impl().downside_mean = mean;
    impl().downside_stdev = stdev;
    //LogD() << "Retina downside, mean: " << mean << ", stdev: " << stdev << ", region: " << x << ", " << y << ", " << w << ", " << h;
    return true;
}


bool RetFocus::TrackingObject::estimateObjectRegion(void) const
{
    bool is_od = impl().is_eye_od;
    bool is_disc = impl().is_disc_scan; 

    int roi_x1, roi_y1, roi_w, roi_h;
    getRoiRegion(is_od, is_disc, roi_x1, roi_y1, roi_w, roi_h);

    int roi_x2 = roi_x1 + roi_w;
    int roi_y2 = roi_y1 + roi_h;

    CvImage roi;
    impl().frame.copyTo(&roi, roi_x1, roi_y1, roi_w, roi_h);
    impl().roi_patch = roi;
 
    float mean, stdev;
    roi.getMeanStddev(&mean, &stdev);
    impl().roi_mean = mean;
    impl().roi_stdev = stdev; 

    // LogD() << "Retina roi, mean: " << mean << ", stdev: " << stdev << ", region: " << roi_x << ", " << roi_y << ", " << roi_w << ", " << roi_h;

    cv::Mat roiMat = roi.getCvMatConst();
    cv::Mat roiFloat;
    if (roiMat.type() != CV_32F) {
        roiMat.convertTo(roiFloat, CV_32F);
    } else {
        roiFloat = roiMat;
    }

    const int KERNEL_SIZE = 9;
    cv::Mat kernel = cv::Mat::ones(KERNEL_SIZE, KERNEL_SIZE, CV_32F);
    cv::Mat filtered;
    cv::filter2D(roiFloat, filtered, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(filtered, nullptr, &maxVal, nullptr, &maxLoc);

    auto peak_x = maxLoc.x + roi_x1;
    auto peak_y = maxLoc.y + roi_y1;

    int bbox_w = RETINA_TRACK_BBOX_WIDTH;
    int bbox_h = RETINA_TRACK_BBOX_HEIGHT;
    int bbox_x1 = peak_x - (bbox_w / 2);
    int bbox_y1 = peak_y - (bbox_h / 2);
    int bbox_x2 = peak_x + (bbox_w / 2);
    int bbox_y2 = peak_y + (bbox_h / 2);

    // Clip the target region to the ROI region
    if (bbox_x1 < roi_x1) {
        bbox_x1 = roi_x1;
        bbox_x2 = roi_x1 + bbox_w;
    }
    else if (bbox_x2 > roi_x2) {
        bbox_x1 = roi_x2 - bbox_w;
        bbox_x2 = roi_x2;
    }

    if (bbox_y1 < roi_y1) {
        bbox_y1 = roi_y1;
        bbox_y2 = roi_y1 + bbox_h;
    }
    else if (bbox_y2 > roi_y2) {
        bbox_y1 = roi_y2 - bbox_h;
        bbox_y2 = roi_y2;
    }

    float bbox_cx = bbox_x1 + (bbox_w / 2.0f);
    float bbox_cy = bbox_y1 + (bbox_h / 2.0f);

    CvImage obj;
    impl().frame.copyTo(&obj, bbox_x1, bbox_y1, bbox_w, bbox_h);
    impl().obj_patch = obj;
    impl().obj_cx = bbox_cx;
    impl().obj_cy = bbox_cy;

    obj.getMeanStddev(&mean, &stdev);
    impl().obj_mean = mean;
    impl().obj_stdev = stdev;
    LogD() << "Retina target, mean: " << mean << ", stdev: " << stdev << ", region: " << bbox_x1 << ", " << bbox_y1 << ", " << bbox_w << ", " << bbox_h;
    return true;
}


float RetFocus::TrackingObject::retinaPatchMean(void) const
{
    return impl().ret_mean;
}

float RetFocus::TrackingObject::retinaPatchStdev(void) const
{
    return impl().ret_stdev;
}

float RetFocus::TrackingObject::objectPatchMean(void) const
{
    return impl().obj_mean;
}

float RetFocus::TrackingObject::objectPatchStdev(void) const
{
    return impl().obj_stdev;
}

float RetFocus::TrackingObject::retinaUpsideMean(void) const
{
    return impl().upside_mean;
}

float RetFocus::TrackingObject::retinaUpsideStdev(void) const
{
    return impl().upside_stdev;
}

float RetFocus::TrackingObject::retinaDownsideMean(void) const
{
    return impl().downside_mean;
}

float RetFocus::TrackingObject::retinaDownsideStdev(void) const
{
    return impl().downside_stdev;
}   


void RetFocus::TrackingObject::getRetinaRegion(bool isOD, bool isDisc, int& x, int& y, int& w, int& h) const
{
    w = RETINA_TRACK_PATCH_WIDTH;
    h = RETINA_TRACK_PATCH_HEIGHT;
    x = RETINA_TRACK_FRAME_CENT_X;
    y = RETINA_TRACK_FRAME_CENT_Y - RETINA_TRACK_PATCH_Y_OFFSET;
 
    if (isOD) {
        x += RETINA_TRACK_PATCH_X_OFFSET;
    }
    else {
        x -= (RETINA_TRACK_PATCH_X_OFFSET + RETINA_TRACK_PATCH_WIDTH);
    }
    return;
}

void RetFocus::TrackingObject::getRoiRegion(bool isOD, bool isDisc, int& x, int& y, int& w, int& h) const
{
    x = RETINA_TRACK_FRAME_CENT_X;
    y = RETINA_TRACK_FRAME_CENT_Y - RETINA_TRACK_ROI_Y_OFFSET;
    w = RETINA_TRACK_ROI_WIDTH;
    h = RETINA_TRACK_ROI_HEIGHT;

	if (isDisc) {
        w = RETINA_TRACK_ROI_WIDTH_DISC;
		x = x - w / 2;
	}
	else {
		if (isOD) {
			x += RETINA_TRACK_ROI_X_OFFSET;
		}
		else {
			x -= (RETINA_TRACK_ROI_X_OFFSET + RETINA_TRACK_ROI_WIDTH);
		}
	}
    return;
}

void RetFocus::TrackingObject::getUpsideRegion(int& x, int& y, int& w, int& h) const
{
    w = RETINA_TRACK_UPSIDE_X_SIZE;
    h = RETINA_TRACK_UPSIDE_Y_SIZE;
    x = RETINA_TRACK_FRAME_CENT_X;
    y = RETINA_TRACK_FRAME_CENT_Y - (RETINA_TRACK_UPSIDE_Y_OFFSET + RETINA_TRACK_UPSIDE_Y_SIZE);
    return;
}

void RetFocus::TrackingObject::getDownsideRegion(int& x, int& y, int& w, int& h) const
{
    w = RETINA_TRACK_DOWNSIDE_X_SIZE;
    h = RETINA_TRACK_DOWNSIDE_Y_SIZE;
    x = RETINA_TRACK_FRAME_CENT_X;
    y = RETINA_TRACK_FRAME_CENT_Y + RETINA_TRACK_DOWNSIDE_Y_OFFSET;
    return;
}

TrackingObject::TrackingObjectImpl& TrackingObject::impl(void) const
{
    return *d_ptr;
}

