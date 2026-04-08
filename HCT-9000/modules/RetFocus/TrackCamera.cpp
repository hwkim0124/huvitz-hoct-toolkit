#include "stdafx.h"
#include "TrackCamera.h"
#include "TrackFeature.h"

#include "CppUtil2.h"
#include "OctDevice2.h"
#include "SigChain2.h"


using namespace RetFocus;
using namespace OctDevice;
using namespace SigChain;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct TrackCamera::TrackCameraImpl
{
	MainBoard* mainboard;
	TrackFeature* p_frame;

	int adjustFrameCount;
	int ignoreFrameCount;
	float frameMean;

	float oldAgain;
	float oldDgain;
	int oldLevel;

	TrackCameraImpl() {
		initializeTrackCameraImpl();
	};

	void initializeTrackCameraImpl(void)
	{
		mainboard = nullptr;
		p_frame = nullptr;
		frameMean = 0.0f;
	}
};



TrackCamera::TrackCamera()
	: d_ptr(make_unique<TrackCameraImpl>())
{
}


TrackCamera::~TrackCamera()
{
}


RetFocus::TrackCamera::TrackCamera(TrackCamera&& rhs) = default;
TrackCamera& RetFocus::TrackCamera::operator=(TrackCamera&& rhs) = default;

void RetFocus::TrackCamera::setupTrackCamera(OctDevice::MainBoard* mainboard)
{
	impl().mainboard = mainboard;
	impl().adjustFrameCount = 0;
	impl().ignoreFrameCount = 0;
	impl().frameMean = 0.0f;
}

void RetFocus::TrackCamera::adjustCameraIntensity(RetFocus::TrackFeature* frame)
{
	const int IGNORE_COUNT1 = 4; // 33 x 4 = 132 ms
	const int IGNORE_COUNT2 = 2; // 66 x 2 = 132 ms
	const int FRAME_AVG_SIZE1 = 4; // 33 x 4 = 132 ms;
	const int FRAME_AVG_SIZE2 = 2; // 66 x 2 = 132 ms;
	static int acc_mean = 0;

	impl().p_frame = frame;
	if (frame) {
		if (frame->isFeatureCovered()) {
			int ignore = isHighSpeedCamera() ? IGNORE_COUNT1 : IGNORE_COUNT2;
			impl().ignoreFrameCount = ignore;
			impl().adjustFrameCount = 0;
			return;
		}

		if (impl().ignoreFrameCount > 0) {
			impl().ignoreFrameCount -= 1;
			return;
		}

		int count = impl().adjustFrameCount;
		if (count == 0) {
			acc_mean = (int)frame->getFeatureMean();
		}
		else {
			acc_mean += (int)frame->getFeatureMean();
		}
		count += 1;

		int avg_size = isHighSpeedCamera() ? FRAME_AVG_SIZE1 : FRAME_AVG_SIZE2;
		if (count >= avg_size) {
			auto fmean = acc_mean / avg_size;
			impl().frameMean = fmean;
			count = 0;

			if (isFrameLevelOptimal()) {
			}
			else if (isFrameLevelBelowRange()) {
				increaseCameraIntensity(false);
			}
			else if (isFrameLevelAboveRange()) {
				decreaseCameraIntensity(false);
			}
			else if (isFrameLevelTooLow()) {
				increaseCameraIntensity(true);
			}
			else if (isFrameLevelTooHigh()) {
				decreaseCameraIntensity(true);
			}
		}
		
		impl().ignoreFrameCount = 0;
		impl().adjustFrameCount = count;
	}
	return;
}


bool RetFocus::TrackCamera::initiateCameraControl(void)
{
	const float INIT_AGAIN = 10.0f;
	const float INIT_DGAIN = 2.0f;
	const int INIT_LEVEL = 16; // 7; // 15;

	auto board = impl().mainboard;
	if (board) {
		auto icam = board->getRetinaIrCamera();
		impl().oldAgain = icam->getAnalogGain();
		impl().oldDgain = icam->getDigitalGain();

		auto iled = board->getRetinaIrLed();
		impl().oldLevel = iled->getIntensity();

		if (true) { 
			iled->setIntensity(INIT_LEVEL);
		}
		if (true) {
			icam->setDigitalGain(INIT_DGAIN);
		}
		if (true) {
			icam->setAnalogGain(INIT_AGAIN);
		}

		// Increase retina camera frames to high speed (30 fps) to support the realtime tracking. 
		bool flag = GlobalSettings::isRetinaTracking30fps();
		icam->setExposureTime(flag);
		LogD() << "Retina tracking camera initiated, dgain: " << INIT_DGAIN << ", again: " << INIT_AGAIN << ", level: " << INIT_LEVEL << ", high speed: " << flag;
	}
	return false;
}

bool RetFocus::TrackCamera::releaseCameraControl(void)
{
	auto board = impl().mainboard;
	if (board) {
		auto icam = board->getRetinaIrCamera();
		icam->setAnalogGain(impl().oldAgain);
		icam->setDigitalGain(impl().oldDgain);

		auto iled = board->getRetinaIrLed();
		iled->setIntensity(impl().oldLevel);

		// Return retina camera frames to normal speed (15 fps).
		icam->setExposureTime(false);
		LogD() << "Retina tracking camera released, dgain: " << impl().oldDgain << ", again: " << impl().oldAgain << ", level: " << impl().oldLevel;
	}
	return false;
}

bool RetFocus::TrackCamera::increaseCameraIntensity(bool bigStep)
{
	const int LEVEL_STEP_1 = 1;
	const int LEVEL_STEP_2 = 3;
	const int LEVEL_MAX[] = { 19, 40, 61, 82 };  // { 16, 34, 52, 70 }; //  
	const float DGAIN_SET[] = { 1.0f, 2.0f, 3.0f, 4.0f };

	auto board = impl().mainboard;
	if (board) {
		auto led = board->getRetinaIrLed();
		auto ints = (int)led->getIntensity();
		auto icam = board->getRetinaIrCamera();
		auto d_gain = icam->getDigitalGain();

		auto inew = ints + (bigStep ? LEVEL_STEP_2 : LEVEL_STEP_1) - (ints % LEVEL_STEP_1);
		auto mean = getFrameLevelMean();

		inew = min(inew, LEVEL_MAX[3]);
		if (inew > ints) {
			if (inew > LEVEL_MAX[2] && ints <= LEVEL_MAX[2]) {
				icam->setDigitalGain(DGAIN_SET[3], true);
			}
			else if (inew > LEVEL_MAX[1] && ints <= LEVEL_MAX[1]) {
				icam->setDigitalGain(DGAIN_SET[2], true);
			}
			else if (inew > LEVEL_MAX[0] && ints <= LEVEL_MAX[0]) {
				icam->setDigitalGain(DGAIN_SET[1], true);
			}

			led->setIntensity(inew);
			LogD() << "Retina intensity: " << ints << " => " << inew << ", mean: " << mean << ", dgain: " << d_gain;
			return true;
		}
	}
	return false;
}

bool RetFocus::TrackCamera::decreaseCameraIntensity(bool bigStep)
{
	const int LEVEL_STEP_1 = 1;
	const int LEVEL_STEP_2 = 3;
	const int LEVEL_MIN = 1;
	const int LEVEL_MAX[] = { 19, 40, 61, 82 }; // { 16, 34, 52, 70 }; //
	const float DGAIN_SET[] = { 1.0f, 2.0f, 3.0f, 4.0f };

	auto board = impl().mainboard;
	if (board) {
		auto led = board->getRetinaIrLed();
		auto ints = (int)led->getIntensity();
		auto icam = board->getRetinaIrCamera();
		auto d_gain = icam->getDigitalGain();

		auto inew = ints - (bigStep ? LEVEL_STEP_2 : LEVEL_STEP_1) - (ints % LEVEL_STEP_1);
		auto mean = getFrameLevelMean();

		inew = max(inew, LEVEL_MIN);
		if (inew < ints) {
			if (inew <= LEVEL_MAX[1] && ints > LEVEL_MAX[1]) {
				icam->setDigitalGain(DGAIN_SET[2], true);
			}
			else if (inew <= LEVEL_MAX[0] && ints > LEVEL_MAX[0]) {
				icam->setDigitalGain(DGAIN_SET[1], true);
			}

			led->setIntensity(inew);
			LogD() << "Retina intensity: " << ints << " => " << inew << ", mean: " << mean << ", dgain: " << d_gain;
			return true;
		}
	}
	return false;
}

bool RetFocus::TrackCamera::stepUpCameraGain(void)
{
	const int LEVEL_DOWN_AT_HIGH_GAIN = 0; // 10;
	const int LEVEL_MIN = 3;
	const int LEVEL_MAX = 70;
	const float DGAIN_MAX1 = 4.0f; // 7.5f;
	const float DGAIN_MAX2 = 7.0f; 
	const float DGAIN_STEP = 0.25f;

	auto board = impl().mainboard;
	if (board) {
		auto icam = board->getRetinaIrCamera();
		auto a_gain = icam->getAnalogGain();
		auto d_gain = icam->getDigitalGain();
		auto a_next = (a_gain >= 8.0f ? 10.0f : (a_gain >= 4.0f) ? 8.0f : (a_gain >= 2.0f) ? 4.0f : 2.0f);

		auto led = board->getRetinaIrLed();
		auto ints = (int)led->getIntensity();
		auto mean = getFrameLevelMean();

		// At first, increase the analog gain to maximum, 
		// then step up the digital gain before increasing led power, 
		// from the led power maximum, step up the digital gain to the top. 
		if (false) { //a_next > a_gain) {
			icam->setAnalogGain(a_next, true);
			LogD() << "Retina a-gain: " << a_gain << " => " << a_next << ", d-gain: " << d_gain << ", ints: " << ints << ", mean: " << mean;
			return true;
		}
		else {
			// auto d_next = min(d_gain + DGAIN_STEP, (ints < LEVEL_MAX ? DGAIN_MAX1 : DGAIN_MAX2));
			auto d_next = min(d_gain + DGAIN_STEP, DGAIN_MAX1);
			if (d_next > d_gain) {
				icam->setDigitalGain(d_next, true);
				LogD() << "Retina d-gain: " << d_gain << " => " << d_next << ", a-gain: " << a_gain << ", ints: " << ints << ", mean: " << mean;
				return true;
			}
		}
	}
	return false;
}

bool RetFocus::TrackCamera::stepDownCameraGain(void)
{
	const int LEVEL_UP_AT_LOWER_GAIN = 0; // 10;
	const int LEVEL_MIN = 3;
	const int LEVEL_MAX = 70; //  90;
	const float DGAIN_MIN = 1.0f;
	const float DGAIN_STEP = 0.25f;

	auto board = impl().mainboard;
	if (board) {
		auto icam = board->getRetinaIrCamera();
		auto a_gain = icam->getAnalogGain();
		auto d_gain = icam->getDigitalGain();
		auto a_next = (a_gain >= 10.0f ? 8.0f : (a_gain >= 8.0f) ? 4.0f : (a_gain >= 4.0f) ? 2.0f : 1.0f);

		auto led = board->getRetinaIrLed();
		auto ints = (int)led->getIntensity();
		auto mean = getFrameLevelMean();

		// At first, try to step down the digital gain to the default level. 
		// then decrease the led power to minimum, 
		// lastly, step down the analog gain to the bottom.  
		auto d_next = max(d_gain - DGAIN_STEP, DGAIN_MIN);
		if (d_next < d_gain) {
			icam->setDigitalGain(d_next, true);
			LogD() << "Retina d-gain: " << d_gain << " => " << d_next << ", a-gain: " << a_gain << ", ints: " << ints << ", mean: " << mean;
			return true;
		}
		else {
			/*
			if (ints <= LEVEL_MIN) {
				if (a_next < a_gain) {
					icam->setAnalogGain(a_next, true);
					LogD() << "Retina a-gain: " << a_gain << " => " << a_next << ", d-gain: " << d_gain << ", ints: " << ints << ", mean: " << mean;
					return true;
				}
			}
			*/
		}
	}
	return false;
}

float RetFocus::TrackCamera::getFrameLevelMean(void) const
{
	return impl().frameMean;
}


bool RetFocus::TrackCamera::isFrameLevelTooLow(void) const
{
	auto mean = getFrameLevelMean();
	auto level = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_DARK_1 : RET_TRACK_FRAME_LEVEL_DARK_2;
	if (mean < level) {
		return true;
	}
	return false;
}

bool RetFocus::TrackCamera::isFrameLevelTooHigh(void) const
{
	auto mean = getFrameLevelMean();
	auto level = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_SHINY_1 : RET_TRACK_FRAME_LEVEL_SHINY_2;
	if (mean > level) {
		return true;
	}
	return false;
}


bool RetFocus::TrackCamera::isHighSpeedCamera(void) const
{
	if (impl().mainboard) {
		auto icam = impl().mainboard->getRetinaIrCamera();
		return icam->isHighSpeedMode();
	}
	return false;
}

bool RetFocus::TrackCamera::isFrameLevelOptimal(void) const
{
	auto mean = getFrameLevelMean();
	auto level1 = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_LOW_1 : RET_TRACK_FRAME_LEVEL_LOW_2;
	auto level2 = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_HIGH_1 : RET_TRACK_FRAME_LEVEL_HIGH_2;
	if (mean >= level1 && mean <= level2) {
		return true;
	}
	return false;
}

bool RetFocus::TrackCamera::isFrameLevelBelowRange(void) const
{
	auto mean = getFrameLevelMean();
	auto level1 = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_DARK_1 : RET_TRACK_FRAME_LEVEL_DARK_2;
	auto level2 = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_LOW_1 : RET_TRACK_FRAME_LEVEL_LOW_2;
	if (mean >= level1 && mean <= level2) {
		return true;
	}
	return false;
}

bool RetFocus::TrackCamera::isFrameLevelAboveRange(void) const
{
	auto mean = getFrameLevelMean();
	auto level1 = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_HIGH_1 : RET_TRACK_FRAME_LEVEL_HIGH_2;
	auto level2 = isHighSpeedCamera() ? RET_TRACK_FRAME_LEVEL_SHINY_1 : RET_TRACK_FRAME_LEVEL_SHINY_2;
	if (mean >= level1 && mean <= level2) {
		return true;
	}
	return false;
}

TrackCamera::TrackCameraImpl& RetFocus::TrackCamera::impl(void) const
{
	return *d_ptr;
}
