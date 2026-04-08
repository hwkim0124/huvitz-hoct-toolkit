#include "stdafx.h"
#include "TrackingRecords.h"
#include "TrackingObject.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;

struct TrackingRecords::TrackingRecordsImpl
{
	std::mutex singleMutex_;
    std::vector<int> track_objs;
    std::vector<float> track_pos_x;
    std::vector<float> track_pos_y;
    std::vector<float> track_score;
    std::vector<float> track_dists;
    std::vector<float> track_deltx;
    std::vector<float> track_delty;
    std::vector<float> track_motix;
    std::vector<float> track_motiy;

    float refer_pos_x;
    float refer_pos_y;
    bool is_reference;

    bool recording;

    bool occluded;
    bool dispaced_x;
    bool dispaced_y;
    bool displaced;
    bool unmatched;
	bool outranged;


    TrackingRecordsImpl() {
        initializeTrackingRecordsImpl();
    };

    void initializeTrackingRecordsImpl(void)
    {
        track_objs.clear();
        track_pos_x.clear();
        track_pos_y.clear();
        track_score.clear();
        track_dists.clear();
        track_deltx.clear();
        track_delty.clear();
        track_motix.clear();
        track_motiy.clear();

        refer_pos_x = 0.0f;
        refer_pos_y = 0.0f;
		is_reference = false;

        occluded = false;
        dispaced_x = false;
        dispaced_y = false;
        displaced = false;
        unmatched = false;
		outranged = false;

        recording = false;
    }
};


TrackingRecords::TrackingRecords()
    : d_ptr(make_unique<TrackingRecordsImpl>())
{
}

TrackingRecords::~TrackingRecords()
{
}

RetFocus::TrackingRecords::TrackingRecords(TrackingRecords&& rhs) = default;
TrackingRecords& RetFocus::TrackingRecords::operator=(TrackingRecords&& rhs) = default;

void RetFocus::TrackingRecords::beginSession(bool refer, float x, float y)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    clearRecords();
    if (refer) {
        setReferencePosition(x, y);
    }
    impl().recording = true;
    LogD() << "Tracking records, begin session, refer: " << refer << ", pos: " << x << ", " << y;
    return;
}

void RetFocus::TrackingRecords::resetSession(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    clearRecords();
    return;
}

void RetFocus::TrackingRecords::resumeSession(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    impl().recording = true;
    LogD() << "Tracking records, resume session";
    return;
}

void RetFocus::TrackingRecords::closeSession(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    impl().recording = false;
    LogD() << "Tracking records, close session";
    return;
}

void RetFocus::TrackingRecords::insertRecord(const RetFocus::TrackingObject& obj)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    if (isRecording()) {
        addTrackingObject(obj);
    }
    return;
}

void RetFocus::TrackingRecords::insertRecord(float x, float y, float score)
{
	lock_guard<mutex> lock(impl().singleMutex_);
	if (isRecording()) {
		auto obj = RetFocus::TrackingObject();
		obj.setTargetMatched(x, y, score);
		addTrackingObject(obj);
	}
	return;
}

int RetFocus::TrackingRecords::getTrackingCount(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    int count = getScoreCount();
    return count;
}


bool RetFocus::TrackingRecords::isReferenceTarget(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = impl().is_reference;
    return result;
}

void RetFocus::TrackingRecords::registerReferenceTarget(float x, float y)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    setReferencePosition(x, y);
    return;
}


bool RetFocus::TrackingRecords::checkTargetStabilized(int count)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    if (impl().track_score.size() < count) {
        return false;
    }

    float score_min = std::numeric_limits<float>::max();
    float x_min = std::numeric_limits<float>::max();
    float y_min = std::numeric_limits<float>::max();
    float x_max = std::numeric_limits<float>::min();
    float y_max = std::numeric_limits<float>::min();

    auto size = impl().track_score.size();
    for (int i = 1; i <= count; i++) {
        int index = (int)(size - i);
        score_min = std::min(score_min, impl().track_score[index]);
        float x = impl().track_pos_x[index];
        float y = impl().track_pos_y[index];
        x_min = std::min(x_min, x);
        y_min = std::min(y_min, y);
        x_max = std::max(x_max, x);
        y_max = std::max(y_max, y);
        LogD() << "Target record, index: " << index << ", score: " << score_min << ", x: " << x << ", y: " << y << ", x_min: " << x_min << ", y_min: " << y_min << ", x_max: " << x_max << ", y_max: " << y_max;
    }

    float dx = x_max - x_min;
    float dy = y_max - y_min;
    float dist = sqrt(dx*dx + dy*dy);

    // if (score_min >= RETINA_TRACK_TARGET_SCORE_MIN && dx < 1.0f && dy < 1.0f) 
	if (score_min >= RETINA_TRACK_TARGET_SCORE_MIN && dx <= RETINA_TRACK_TARGET_MOVED_X2 && dy <= RETINA_TRACK_TARGET_MOVED_Y2) {
        LogD() << "Target is stabilized, score_min: " << score_min << ", dx: " << dx << ", dy: " << dy << ", dist: " << dist;
        return true;
    }
    else {
        LogD() << "Target is not stabilized, score_min: " << score_min << ", dx: " << dx << ", dy: " << dy << ", dist: " << dist;
    }
    return false;
}

bool RetFocus::TrackingRecords::getLastPosition(float& x, float& y, float& score) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    if (impl().track_score.empty()) {
        return false;
    }
    x = impl().track_pos_x.back();
    y = impl().track_pos_y.back();
    score = impl().track_score.back();
    return true;
}

bool RetFocus::TrackingRecords::getLastDistance(float from_x, float from_y, float& xpos, float& ypos, float& dx, float& dy, float& dist, float& score) const
{
    float x, y, s;
    if (getLastPosition(x, y, s)) {
        dx = x - from_x;
        dy = y - from_y;
        dist = sqrt(dx*dx + dy*dy);

		xpos = x;
		ypos = y;
		score = s;
        return true;
    }
    return false;
}

bool RetFocus::TrackingRecords::hasTargetValidated(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    if (impl().track_score.empty()) {
        return false;
    }
    float score = impl().track_score.back();
    if (score < RETINA_TRACK_TARGET_SCORE_MIN) {
        return false;
    }

    if (impl().is_reference) {
        float tx = impl().track_pos_x.back();
        float ty = impl().track_pos_y.back();
        float rx = impl().refer_pos_x;
        float ry = impl().refer_pos_y;
        float dx = tx - rx;
        float dy = ty - ry;
        float dist = sqrt(dx*dx + dy*dy);
        dist = sqrt(dx*dx + dy*dy);
        if (fabs(dx) <= RETINA_TRACK_TARGET_MOVED_X2 && fabs(dy) <= RETINA_TRACK_TARGET_MOVED_Y2) {
            return true;
        }
    }
    return false;
}


bool RetFocus::TrackingRecords::hasTargetMotionFreed(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = isTargetConsistent();
    return result;
}

bool RetFocus::TrackingRecords::hasTargetOccluded(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool occluding = false;
    bool result = isTargetOccluded(occluding);
    return result;
}


void RetFocus::TrackingRecords::clearRecords(void)
{
    impl().initializeTrackingRecordsImpl();
    return;
}


bool RetFocus::TrackingRecords::isRecording(void) const
{
    bool result = impl().recording;
    return result;
}

bool RetFocus::TrackingRecords::isEmpty(void) const
{
    bool result = getScoreCount() == 0;
    return result;
}


int RetFocus::TrackingRecords::getScoreCount(void) const
{
    int count = (int)impl().track_score.size();
    return count;
}

void RetFocus::TrackingRecords::setReferencePosition(float x, float y)
{
    impl().refer_pos_x = x;
    impl().refer_pos_y = y;
    impl().is_reference = true;

	LogD() << "Tracking records, reference pos: " << x << ", " << y << ", count: " << impl().track_pos_x.size();
    return;
}


bool RetFocus::TrackingRecords::getLastPosition(float& x, float& y, float& score, float& dist) const 
{
    if (impl().track_score.empty()) {
        return false;
    }
    x = impl().track_pos_x.back();
    y = impl().track_pos_y.back();
    score = impl().track_score.back();
    dist = impl().track_dists.back();
    return true;
}


bool RetFocus::TrackingRecords::isTargetConsistent(void)
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    bool occluded = impl().occluded;
    bool displaced_x = impl().dispaced_x;
    bool displaced_y = impl().dispaced_y;
    bool outranged = impl().outranged;
    bool unmatched = impl().unmatched;

    if (occluded || displaced_x || displaced_y || outranged || unmatched) {
        LogD() << "Target is not stabilized, occluded: " << occluded << ", displaced_x: " << displaced_x << ", displaced_y: " << displaced_y << ", outranged: " << outranged << ", unmatched: " << unmatched;
        return false;
    }
    return true;
}

bool RetFocus::TrackingRecords::isTargetOccluded(bool& occluding)
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    bool occluded = impl().occluded;
    occluding = false; 

    if (occluded) {
        if (!impl().track_score.empty()) {
            float score = impl().track_score.back(); 
            if (score < RETINA_TRACK_TARGET_SCORE_MIN) {
                occluding = true;
            }
        }
        LogD() << "Target is eyelid covered, occluded: " << occluded << ", occluding: " << occluding;
        return true;
    }
    return false;
}

bool RetFocus::TrackingRecords::isTargetDisplaced(void)
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    bool displaced_x = impl().dispaced_x;
    bool displaced_y = impl().dispaced_y;
    
    if (displaced_x || displaced_y) {
        LogD() << "Target is displaced, displaced_x: " << displaced_x << ", displaced_y: " << displaced_y;
        return true;
    }
    return false;
}

bool RetFocus::TrackingRecords::isTargetOutranged(void)
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    bool outranged = impl().outranged;

    if (outranged) {
        float max_dist = 0.0f;
        if (!impl().track_dists.empty()) {
            max_dist = *std::max_element(impl().track_dists.begin(), impl().track_dists.end());
        }
        LogD() << "Target is outranged, max_dist: " << max_dist;
        return true;
    }
    return false;
}

bool RetFocus::TrackingRecords::isTargetUnmatched(void)
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    bool unmatched = impl().unmatched;

    if (unmatched) {
        float min_score = 0.0f;
        if (!impl().track_score.empty()) {
            min_score = *std::min_element(impl().track_score.begin(), impl().track_score.end());
        }
        LogD() << "Target is unmatched, min_score: " << min_score;  
        return true;
    }
    return false;
}


void RetFocus::TrackingRecords::addTrackingObject(const RetFocus::TrackingObject& obj)
{
    // impl().trackingObjects.push_back(obj);
    float tx = obj.targetMatchedX();
    float ty = obj.targetMatchedY();
    float score = obj.targetMatchScore();
	float dist = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	float ix = 0.0f;
	float iy = 0.0f;
	float mx = 0.0f;
	float my = 0.0f;

	bool displaced_x = false;
	bool displaced_y = false;
	bool outranged = false;
	bool unmatched = false;
	bool occluded = false;

	// Distance between target and reference position
	if (impl().is_reference) {
		rx = impl().refer_pos_x;
		ry = impl().refer_pos_y;
		dx = tx - rx;
		dy = ty - ry;
		dist = sqrt(dx*dx + dy*dy);
	}
	/*
	else {
	if (!impl().track_pos_x.empty()) {
	rx = impl().track_pos_x[0];
	ry = impl().track_pos_y[0];
	dx = tx - rx;
	dy = ty - ry;
	dist = sqrt(dx*dx + dy*dy);
	}
	}
	*/

	// Motion from the first frame
	if (!impl().track_pos_x.empty()) {
		ix = impl().track_pos_x[0];
		iy = impl().track_pos_y[0];
		mx = tx - ix;
		my = ty - iy;
	}

	if (fabs(mx) > RETINA_TRACK_TARGET_MOVED_X) {
		displaced_x = true;
	}
	if (fabs(my) > RETINA_TRACK_TARGET_MOVED_Y) {
		displaced_y = true;
	}
	if (dist > RETINA_TRACK_TARGET_DIST_MAX) {
		outranged = true;
	}

	if (obj.checkEyelidCovered()) {
		score = 0.0f;
		occluded = true;
	}
	if (score < RETINA_TRACK_TARGET_SCORE_MIN) {
		unmatched = true;
	}

	impl().track_pos_x.push_back(tx);
	impl().track_pos_y.push_back(ty);
	impl().track_score.push_back(score);
	impl().track_dists.push_back(dist);
	impl().track_deltx.push_back(dx);
	impl().track_delty.push_back(dy);
	impl().track_motix.push_back(mx);
	impl().track_motiy.push_back(my);

	impl().occluded = occluded;
	impl().unmatched = unmatched;
	impl().dispaced_x = displaced_x;
	impl().dispaced_y = displaced_y;
	impl().outranged = outranged;

	auto count = impl().track_pos_x.size();
	LogD() << "Target record, count: " << count << ", pos: " << tx << ", " << ty << ", score: " << score << ", dist: " << dist << ", init: " << rx << ", " << ry << ", motion: " << mx << ", " << my;
	LogD() << "Target record, occluded: " << occluded << ", unmatched: " << unmatched << ", displaced: " << displaced_x << ", " << displaced_y << ", outranged: " << outranged;
	return;
}

TrackingRecords::TrackingRecordsImpl& RetFocus::TrackingRecords::impl(void) const
{
    return *d_ptr;
}
