#include "stdafx.h"
#include "SystemSettings.h"


using namespace OctConfig;
using namespace std;


struct SystemSettings::SystemSettingsImpl
{
	int checkMainboard;
	int modelType;
	int octGrabberType;
	int lineCameraMode;
	int sldFaultDetection;
	int sldFaultTimer;
	int sldFaultThreshold;
	int retinaTrackingSpeed;

	int triggerForePadd[3];
	int triggerPostPadd[3];

	std::string serialNo;

	SystemSettingsImpl() {
		initializeSystemSettingsImpl();
	}

	void initializeSystemSettingsImpl() {
		serialNo.clear();
		modelType = 0;
		octGrabberType = 0;
		lineCameraMode = 0;
		sldFaultDetection = 0;
		retinaTrackingSpeed = 0;
		sldFaultTimer = 5000;
		sldFaultThreshold = 10;
		checkMainboard = 0;

		triggerForePadd[0] = TRIGGER_FORE_PADDING_POINTS_AT_FASTEST;
		triggerForePadd[1] = TRIGGER_FORE_PADDING_POINTS_AT_FASTER;
		triggerForePadd[2] = TRIGGER_FORE_PADDING_POINTS_AT_NORMAL;
		triggerPostPadd[0] = TRIGGER_POST_PADDING_POINTS_AT_FASTEST;
		triggerPostPadd[1] = TRIGGER_POST_PADDING_POINTS_AT_FASTER;
		triggerPostPadd[2] = TRIGGER_POST_PADDING_POINTS_AT_NORMAL;
	}
};



SystemSettings::SystemSettings() :
	d_ptr(make_unique<SystemSettingsImpl>())
{
	initialize();
}


OctConfig::SystemSettings::~SystemSettings() = default;
OctConfig::SystemSettings::SystemSettings(SystemSettings && rhs) = default;
SystemSettings & OctConfig::SystemSettings::operator=(SystemSettings && rhs) = default;


OctConfig::SystemSettings::SystemSettings(const SystemSettings & rhs)
	: d_ptr(make_unique<SystemSettingsImpl>(*rhs.d_ptr))
{
}


SystemSettings & OctConfig::SystemSettings::operator=(const SystemSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::SystemSettings::initialize(void)
{
	resetToDefaultValues();
	return;
}


void OctConfig::SystemSettings::resetToDefaultValues(void)
{
	getImpl().initializeSystemSettingsImpl();
	return;
}

//bool OctConfig::SystemSettings::useOctSoloVersion(bool isset, bool flag)
//{
//	if (isset) {
//		getImpl().octSoloVersion = flag;
//	}
//	return getImpl().octSoloVersion;
//}

int OctConfig::SystemSettings::getModelType(void)
{
	 return getImpl().modelType;
}

int OctConfig::SystemSettings::getOctGrabberType(void)
{
	return getImpl().octGrabberType;
}

int OctConfig::SystemSettings::getOctLineCameraMode(void)
{
	return getImpl().lineCameraMode;
}

int OctConfig::SystemSettings::getSldFaultDetection(void)
{
	return getImpl().sldFaultDetection;
}

int OctConfig::SystemSettings::getSldFaultTimer(void)
{
	return getImpl().sldFaultTimer;
}

int OctConfig::SystemSettings::getSldFaultThreshold(void)
{
	return getImpl().sldFaultThreshold;
}

int OctConfig::SystemSettings::getRetinaTrackingSpeed(void)
{
	return getImpl().retinaTrackingSpeed;
}

int OctConfig::SystemSettings::getTriggerForePadd(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().triggerForePadd[index];
	}
	else {
		return (index == 2 ? TRIGGER_FORE_PADDING_POINTS_AT_NORMAL : (index == 1 ? TRIGGER_FORE_PADDING_POINTS_AT_FASTER : TRIGGER_FORE_PADDING_POINTS_AT_FASTEST));
	}
}

int OctConfig::SystemSettings::getTriggerPostPadd(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().triggerPostPadd[index];
	}
	else {
		return (index == 2 ? TRIGGER_POST_PADDING_POINTS_AT_NORMAL : (index == 1 ? TRIGGER_POST_PADDING_POINTS_AT_FASTER : TRIGGER_POST_PADDING_POINTS_AT_FASTEST));;
	}
}

void OctConfig::SystemSettings::setModelType(int type)
{
	getImpl().modelType = type;
	return;
}

void OctConfig::SystemSettings::setOctGrabberType(int type)
{
	if (type == (int)OctGrabberType::USB_LINE_CAMERA || type == (int)OctGrabberType::MIL_FRAME_GRABBER) {
		getImpl().octGrabberType = type;
	}
	else {
		getImpl().octGrabberType = (int)OctGrabberType::MIL_FRAME_GRABBER;
	}
	return;
}

void OctConfig::SystemSettings::setOctLineCameraMode(int mode)
{
	if (mode >= (int)OctLineCameraMode::LINE_CAMERA_68K && mode <= (int)OctLineCameraMode::LINE_CAMERA_120K) {
		getImpl().lineCameraMode = mode;
	}
	else {
		getImpl().lineCameraMode = (int)OctLineCameraMode::LINE_CAMERA_68K;
	}
	return;
}

void OctConfig::SystemSettings::setSldFaultDetection(int value)
{
	if (value >= 0 && value <= 1) {
		getImpl().sldFaultDetection = value;
	}
	else {
		getImpl().sldFaultDetection = 0;
	}
	return;
}

void OctConfig::SystemSettings::setSldFaultTimer(int value)
{
	if (value >= 0 && value <= 99999) {
		getImpl().sldFaultTimer = value;
	}
	else {
		getImpl().sldFaultTimer = 5000;
	}
	return;
}

void OctConfig::SystemSettings::setSldFaultThreshold(int value)
{
	if (value >= 0 && value <= 99999) {
		getImpl().sldFaultThreshold = value;
	}
	else {
		getImpl().sldFaultThreshold = 10;
	}
	return;
}


void OctConfig::SystemSettings::setRetinaTrackingSpeed(int speed)
{
	if (speed >= (int)RetinaTrackingSpeed::TRACK_CAMERA_15FPS && speed <= (int)RetinaTrackingSpeed::TRACK_CAMERA_30FPS) {
		getImpl().retinaTrackingSpeed = speed;
	}
	else {
		getImpl().retinaTrackingSpeed = (int)RetinaTrackingSpeed::TRACK_CAMERA_30FPS;
	}
	return;
}

void OctConfig::SystemSettings::setTriggerForePadd(int index, int value)
{
	if (index >= 0 && index < 3) {
		if (value >= 0 && value <= 199) {
			getImpl().triggerForePadd[index] = value;
		}
		else {
			getImpl().triggerForePadd[index] = (index == 2 ? TRIGGER_FORE_PADDING_POINTS_AT_NORMAL : (index  == 1 ? TRIGGER_FORE_PADDING_POINTS_AT_FASTER : TRIGGER_FORE_PADDING_POINTS_AT_FASTEST));
		}
	}
	return;
}

void OctConfig::SystemSettings::setTriggerPostPadd(int index, int value)
{
	if (index >= 0 && index < 3) {
		if (value >= 0 && value <= 199) {
			getImpl().triggerPostPadd[index] = value;
		}
		else {
			getImpl().triggerPostPadd[index] = (index == 2 ? TRIGGER_POST_PADDING_POINTS_AT_NORMAL : (index == 1 ? TRIGGER_POST_PADDING_POINTS_AT_FASTER : TRIGGER_POST_PADDING_POINTS_AT_FASTEST));
		}
	}
	return;
}

int OctConfig::SystemSettings::checkMainboardAtStartup(bool isset, int value)
{
	if (isset) {
		getImpl().checkMainboard = value;
	}
	return getImpl().checkMainboard;
}


std::string & OctConfig::SystemSettings::serialNumber(void)
{
	return getImpl().serialNo;
}


void OctConfig::SystemSettings::setSerialNumber(std::string serialNo)
{
	std::string sstr;
	if (serialNo.size() > SERIAL_NUMBER_SIZE_MAX) {
		sstr = serialNo.substr(0, SERIAL_NUMBER_SIZE_MAX);
	}
	else {
		sstr = serialNo;
	}
	d_ptr->serialNo = sstr;
	return;
}


SystemSettings::SystemSettingsImpl & OctConfig::SystemSettings::getImpl(void) const
{
	return *d_ptr;
}
