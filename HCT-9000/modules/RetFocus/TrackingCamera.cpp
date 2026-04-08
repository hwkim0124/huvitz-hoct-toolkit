#include "stdafx.h"
#include "TrackingCamera.h"
#include "TrackingObject.h"

#include "CppUtil2.h"
#include "OctDevice2.h"
#include "SigChain2.h"

using namespace RetFocus;
using namespace OctDevice;
using namespace SigChain;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct TrackingCamera::TrackingCameraImpl
{
    MainBoard* mainboard;
    TrackingObject* p_object;

    int adjustCount;
    int ignoreCount;
    float frameMean;

    float old_dgain;
    float old_again;
    int old_level;

    TrackingCameraImpl() {
        initializeTrackingCameraImpl();
    }

    void initializeTrackingCameraImpl(void)
    {
        mainboard = nullptr;
        p_object = nullptr;

        old_dgain = 0.0f;
        old_again = 0.0f;
        old_level = 0;

        adjustCount = 0;
        ignoreCount = 0;
        frameMean = 0.0f;
    }
};


TrackingCamera::TrackingCamera() 
    : d_ptr(make_unique<TrackingCameraImpl>())
{

}


TrackingCamera::~TrackingCamera()
{
}


RetFocus::TrackingCamera::TrackingCamera(TrackingCamera&& rhs) = default;
TrackingCamera& RetFocus::TrackingCamera::operator=(TrackingCamera&& rhs) = default;


void RetFocus::TrackingCamera::initializeTrackingCamera(OctDevice::MainBoard* mainboard)
{
	impl().initializeTrackingCameraImpl();
    impl().mainboard = mainboard;
    return;
}


void RetFocus::TrackingCamera::initiateAdjustment(void)
{
    const float INIT_AGAIN = 8.0f;
	const float INIT_DGAIN = 2.0f;
	const int INIT_LEVEL = 16; // 7; // 15;

    auto board = impl().mainboard;
    if (board) {
        auto icam = board->getRetinaIrCamera();
        auto again = icam->getAnalogGain();
        auto dgain = icam->getDigitalGain();
        auto level = board->getRetinaIrLed()->getIntensity();
        impl().old_again = again;
        impl().old_dgain = dgain;
        impl().old_level = level;

        if (true) {
            board->getRetinaIrLed()->setIntensity(INIT_LEVEL);
        }
        if (true) {
            icam->setDigitalGain(INIT_DGAIN);
        }
        if (true) {
            icam->setAnalogGain(INIT_AGAIN);
        }

        // Increase retina camera frames to high speed (30 fps) to support the realtime tracking. 
        bool flag = false; // GlobalSettings::isRetinaTracking30fps();
        icam->setExposureTime(flag);
        LogD() << "Retina tracking camera adjustment initiated, dgain: " << INIT_DGAIN << ", again: " << INIT_AGAIN << ", level: " << INIT_LEVEL << ", high speed: " << flag;
    }

    impl().adjustCount = 0;
    impl().ignoreCount = 0;
    impl().frameMean = 0.0f;
    return;
}

void RetFocus::TrackingCamera::completeAdjustment(void)
{
    auto board = impl().mainboard;
    if (board) {
        board->getSplitFocusLed()->control(false);
        board->getWorkingDotsLed()->control(false);
        board->getWorkingDot2Led()->control(false);
        board->getSplitFocusMotor()->updatePositionToMirrorOut();

        auto icam = board->getRetinaIrCamera();
        bool flag = GlobalSettings::isRetinaTracking30fps();
        icam->setExposureTime(flag);
        LogD() << "Retina tracking camera adjustment completed, high speed: " << flag;
    }
}

void RetFocus::TrackingCamera::releaseAdjustment(void)
{
    auto board = impl().mainboard;
    if (board) {
        auto icam = board->getRetinaIrCamera();
        icam->setAnalogGain(impl().old_again);
        icam->setDigitalGain(impl().old_dgain);

        auto iled = board->getRetinaIrLed();
        iled->setIntensity(impl().old_level);

        // Return retina camera frames to normal speed (15 fps).
        icam->setExposureTime(false);
        LogD() << "Retina tracking camera adjustment released, dgain: " << impl().old_dgain << ", again: " << impl().old_again << ", level: " << impl().old_level;
    }
    return;
}


void RetFocus::TrackingCamera::adjustFrameIntensity(RetFocus::TrackingObject* object)
{
    const int FRAME_AVG_SIZE1 = 4; // 33 x 4 = 132 ms
    const int FRAME_AVG_SIZE2 = 2; // 66 x 2 = 132 ms
    static int acc_mean = 0;
    static int acc_count = 0;

    if (!object) {
        return;
    }

    float mean = object->retinaPatchMean();
    if (object->checkEyelidCovered()) {
        acc_count = 0;
        return;
    }

    if (acc_count == 0) {
        acc_mean = (int)mean;
    }
    else {
        acc_mean += (int)mean;
    }
    acc_count += 1;

    int avg_size = isHighSpeedCamera() ? FRAME_AVG_SIZE1 : FRAME_AVG_SIZE2;
    if (acc_count >= avg_size) {
        auto fmean = acc_mean / avg_size;
        impl().frameMean = fmean;
        acc_count = 0;

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

        impl().adjustCount += 1;
        impl().ignoreCount = 0;
    }
    return;
}


bool RetFocus::TrackingCamera::increaseCameraIntensity(bool bigStep)
{
    const int LEVEL_STEP_1 = 1;
    const int LEVEL_STEP_2 = 3;
    const int LEVEL_MAX[] = { 19, 40, 61, 82 }; // { 16, 34, 52, 70 }; //
    const float DGAIN_SET[] = { 1.0f, 2.0f, 3.0f, 4.0f };

    auto board = impl().mainboard;
    if (board) {
        auto icam = board->getRetinaIrCamera();
        auto dgain = icam->getDigitalGain();
        auto iled = board->getRetinaIrLed();
        auto ints = (int)iled->getIntensity();

        auto inew = ints + (bigStep ? LEVEL_STEP_2 : LEVEL_STEP_1) - (ints % LEVEL_STEP_1);
        auto mean = impl().frameMean;

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
            iled->setIntensity(inew);
            //LogD() << "Retina intensity: " << ints << " => " << inew << ", mean: " << mean << ", dgain: " << dgain;
            return true;
        }
    }
    return false;
}

bool RetFocus::TrackingCamera::decreaseCameraIntensity(bool bigStep)
{
    const int LEVEL_STEP_1 = 1;
    const int LEVEL_STEP_2 = 3;
    const int LEVEL_MIN = 1;
    const int LEVEL_MAX[] = { 19, 40, 61, 82 }; // { 16, 34, 52, 70 }; //
    const float DGAIN_SET[] = { 1.0f, 2.0f, 3.0f, 4.0f };

    auto board = impl().mainboard;
    if (board) {
        auto icam = board->getRetinaIrCamera();
        auto dgain = icam->getDigitalGain();
        auto iled = board->getRetinaIrLed();
        auto ints = (int)iled->getIntensity();

        auto inew = ints - (bigStep ? LEVEL_STEP_2 : LEVEL_STEP_1) - (ints % LEVEL_STEP_1);
        auto mean = impl().frameMean;

        inew = max(inew, LEVEL_MIN);
        if (inew < ints) {
            if (inew <= LEVEL_MAX[1] && ints > LEVEL_MAX[1]) {
                icam->setDigitalGain(DGAIN_SET[2], true);
            }
            else if (inew <= LEVEL_MAX[0] && ints > LEVEL_MAX[0]) {
                icam->setDigitalGain(DGAIN_SET[1], true);
            }
            iled->setIntensity(inew);
            //LogD() << "Retina intensity: " << ints << " => " << inew << ", mean: " << mean << ", dgain: " << dgain;
            return true;
        }
    }
    return false;
}


bool RetFocus::TrackingCamera::isFrameLevelOptimal(void) const
{
    auto mean = impl().frameMean;
    if (mean >= RETINA_TRACK_FRAME_LEVEL_LOW && mean <= RETINA_TRACK_FRAME_LEVEL_HIGH) {
        return true;
    }
    return false;
}

bool RetFocus::TrackingCamera::isFrameLevelBelowRange(void) const
{
    auto mean = impl().frameMean;
    if (mean >= RETINA_TRACK_FRAME_LEVEL_DARK && mean <= RETINA_TRACK_FRAME_LEVEL_LOW) {
        return true;
    }
    return false;
}

bool RetFocus::TrackingCamera::isFrameLevelAboveRange(void) const
{
    auto mean = impl().frameMean;
    if (mean >= RETINA_TRACK_FRAME_LEVEL_HIGH && mean <= RETINA_TRACK_FRAME_LEVEL_SHINY) {
        return true;
    }
    return false;
}

bool RetFocus::TrackingCamera::isFrameLevelTooLow(void) const
{
    auto mean = impl().frameMean;
    if (mean < RETINA_TRACK_FRAME_LEVEL_DARK) {
        return true;
    }
    return false;
}

bool RetFocus::TrackingCamera::isFrameLevelTooHigh(void) const
{
    auto mean = impl().frameMean;
    if (mean > RETINA_TRACK_FRAME_LEVEL_SHINY) {
        return true;
    }
    return false;
}


bool RetFocus::TrackingCamera::isHighSpeedCamera(void) const
{
    if (impl().mainboard) {
        auto icam = impl().mainboard->getRetinaIrCamera();
        auto mode = icam->isHighSpeedMode();
        return mode;
    }
    return false;
}

TrackingCamera::TrackingCameraImpl& TrackingCamera::impl(void) const
{
    return *d_ptr;
}

