#include "stdafx.h"
#include "TrackRecords.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;


struct TrackRecords::TrackRecordsImpl
{
	std::mutex singleMutex_;

	float init_x;
	float init_y;
	float last_x;
	float last_y;

	float drift_x_min;
	float drift_x_max;
	float drift_y_min;
	float drift_y_max;
	
	float mean_min;
	float mean_max;
	float covar_min;
	float covar_max;
	float stdev_min;
	float stdev_max;

	float simval_min;
	float simval_max;

	bool is_covered;
	bool is_valid;
	bool is_recording;

	int recordCount;

	TrackRecordsImpl() {
		initializeTrackRecordsImpl();
	};

	void initializeTrackRecordsImpl(void)
	{
		is_recording = false;
		is_valid = true;
		is_covered = false;
	}
};


TrackRecords::TrackRecords()
	: d_ptr(make_unique<TrackRecordsImpl>())
{
}


TrackRecords::~TrackRecords()
{
}


RetFocus::TrackRecords::TrackRecords(TrackRecords&& rhs) = default;
TrackRecords& RetFocus::TrackRecords::operator=(TrackRecords&& rhs) = default;

void RetFocus::TrackRecords::insertMovement(bool valid, bool covered, float mean, float stdev, float simval, float dx, float dy)
{
	lock_guard<mutex> lock(impl().singleMutex_);

	if (!impl().is_recording) {
		// LogD() << "Tracker not recording";
		return;
	}

	impl().is_valid = (valid ? impl().is_valid : valid);
	impl().is_covered = (covered ? covered : impl().is_covered);

	if (impl().recordCount == 0) {
		impl().mean_min = mean;
		impl().mean_max = mean;
		impl().stdev_min = stdev;
		impl().stdev_max = stdev;
		impl().simval_max = simval;
		impl().simval_min = simval;
		impl().drift_x_max = dx;
		impl().drift_x_min = dx;
		impl().drift_y_max = dy;
		impl().drift_y_min = dy;
	}
	else {
		if (mean > impl().mean_max)
			impl().mean_max = mean;
		if (mean < impl().mean_min)
			impl().mean_min = mean;
		if (stdev > impl().stdev_max)
			impl().stdev_max = stdev;
		if (stdev < impl().stdev_min)
			impl().stdev_min = stdev;

		if (simval > impl().simval_max)
			impl().simval_max = simval;
		if (simval < impl().simval_min)
			impl().simval_min = simval;

		if (dx > impl().drift_x_max)
			impl().drift_x_max = dx;
		if (dx < impl().drift_x_min)
			impl().drift_x_min = dx;

		if (dy > impl().drift_y_max)
			impl().drift_y_max = dy;
		if (dy < impl().drift_y_min)
			impl().drift_y_min = dy;
	}

	impl().last_x = dx;
	impl().last_y = dy;
	impl().recordCount += 1;
	LogD() << "Tracker records, insert valid: " << valid << ", covered: " << covered << ", mean: " << mean << ", simval: " << simval << ", last: " << dx << ", " << dy << ", init: " << impl().init_x << ", " << impl().init_y << ", count: " << impl().recordCount;
	return;
}

void RetFocus::TrackRecords::clearRecords(void)
{
	lock_guard<mutex> lock(impl().singleMutex_);

	impl().init_x = 0.0f;
	impl().init_y = 0.0f;
	impl().last_x = 0.0f;
	impl().last_y = 0.0f;

	impl().drift_x_max = 0.0f;
	impl().drift_x_min = 0.0f;
	impl().drift_y_max = 0.0f;
	impl().drift_y_min = 0.0f;

	impl().mean_max = 0.0f;
	impl().mean_min = 9999.0f;
	impl().stdev_max = 0.0f;
	impl().stdev_min = 9999.0f;

	impl().simval_max = 0.0f;
	impl().simval_min = 1.0f;

	impl().is_covered = false;
	impl().is_valid = true;	
	impl().recordCount = 0;
	// LogD() << "Tracker records, cleared";
	return;
}

void RetFocus::TrackRecords::setIsRecording(bool flag)
{
	lock_guard<mutex> lock(impl().singleMutex_);
	impl().is_recording = flag;
	// LogD() << "Tracking records, set: " << flag;
}

void RetFocus::TrackRecords::setInitialPosition(float x, float y)
{
	lock_guard<mutex> lock(impl().singleMutex_);
	impl().init_x = x;
	impl().init_y = y;
	LogD() << "Tracker records, initial pos: " << x << ", " << y;
}

void RetFocus::TrackRecords::getLastPosition(float& x_pos, float& y_pos)
{
	lock_guard<mutex> lock(impl().singleMutex_);
	x_pos = impl().last_x;
	y_pos = impl().last_y;
}

void RetFocus::TrackRecords::getLastDrifts(float& dx, float& dy)
{
	lock_guard<mutex> lock(impl().singleMutex_);
	dx = impl().drift_x_max - impl().drift_x_min;
	dy = impl().drift_y_max - impl().drift_y_min;
	return;
}

bool RetFocus::TrackRecords::isAvailable(int size) const
{
	lock_guard<mutex> lock(impl().singleMutex_);
	bool result = impl().recordCount >= size;
	return result;
}

bool RetFocus::TrackRecords::isConsistent(bool align) const
{
	lock_guard<mutex> lock(impl().singleMutex_);
	if (isUnspotted() || isCovered()) {
		LogD() << "Track records is invalid/covered";
		return false;
	}

	if (isOutOfRange()) {
		LogD() << "Track records is out of moving range";
		return false;
	}

	if (isMismatched()) {
		LogD() << "Track records is mismatched";
		return false;
	}

	if (isDrifted()) {
		LogD() << "Track records is drifted";
		return false;
	}

	if (align) {
		if (!isTargetAligned()) {
			LogD() << "Track records is not aligned";
			return false;
		}
	}

	LogD() << "Track records is consistent";
	return true;
}

bool RetFocus::TrackRecords::isIdentified(void) const
{
	lock_guard<mutex> lock(impl().singleMutex_);
	if (isUnspotted() || isCovered()) {
		LogD() << "Track records is invalid/covered";
		return false;
	}

	if (isOutOfRange()) {
		LogD() << "Track records is out of moving range";
		return false;
	}

	if (isMismatched()) {
		LogD() << "Track records is mismatched";
		return false;
	}

	LogD() << "Track records is identified";
	return true;
}

bool RetFocus::TrackRecords::isDisplaced(void) const
{
	lock_guard<mutex> lock(impl().singleMutex_);
	if (isDrifted()) {
		LogD() << "Track records is drifted";
		return true;
	}
	if (!isTargetAligned()) {
		LogD() << "Track records is not aligned";
		return true;
	}
	return false;
}

bool RetFocus::TrackRecords::isOutOfRange(void) const
{
	const float DRIFT_X_MAX = 9.0f;
	const float DRIFT_Y_MAX = 9.0f;

	auto dx = max(abs(impl().drift_x_max), abs(impl().drift_x_min));
	auto dy = max(abs(impl().drift_y_max), abs(impl().drift_y_min));

	if (dx >= DRIFT_X_MAX || dy >= DRIFT_Y_MAX) {
		return true;
	}
	return false;
}

bool RetFocus::TrackRecords::isUnspotted(void) const
{
	bool flag = !impl().is_valid;
	return flag;
}

bool RetFocus::TrackRecords::isCovered(void) const
{
	return impl().is_covered;
}

bool RetFocus::TrackRecords::isDrifted(void) const
{
	const float DRIFT_X_MIN = 0.5f;
	const float DRIFT_Y_MIN = 0.5f;

	auto ox = fabs(impl().drift_x_max - impl().drift_x_min);
	auto oy = fabs(impl().drift_y_max - impl().drift_y_min);
	if (ox >= DRIFT_X_MIN || oy >= DRIFT_Y_MIN) {
		return true;
	}
	return false;
}

bool RetFocus::TrackRecords::isMismatched(void) const
{
	const float SIMILARITY_MIN = RETT_FEAT_THRESH_TO_COMPARE;

	auto simval = impl().simval_min;
	if (simval < SIMILARITY_MIN) {
		return true;
	}
	return false;
}


bool RetFocus::TrackRecords::isTargetAligned(void) const
{
	const float ALIGN_X_MAX = 0.5f;
	const float ALIGN_Y_MAX = 0.5f;

	auto dx = fabs(impl().init_x - impl().last_x);
	auto dy = fabs(impl().init_y - impl().last_y);

	if (dx < ALIGN_X_MAX && dy < ALIGN_Y_MAX) {
		return true;
	}
	return false;
}


bool RetFocus::TrackRecords::isCenterAligned(void) const
{
	const float CENTER_X_MAX = 1.0f;
	const float CENTER_Y_MAX = 1.0f;

	auto dx = max(abs(impl().drift_x_max), abs(impl().drift_x_min));
	auto dy = max(abs(impl().drift_y_max), abs(impl().drift_y_min));

	if (dx < CENTER_X_MAX && dy < CENTER_Y_MAX) {
		return true;
	}
	return false;
}


TrackRecords::TrackRecordsImpl& RetFocus::TrackRecords::impl(void) const
{
	return *d_ptr;
}
