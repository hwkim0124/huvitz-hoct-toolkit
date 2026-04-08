#include "stdafx.h"
#include "Galvanometer.h"
#include "MainBoard.h"
#include "UsbComm.h"
#include "TraceProfile.h"

#include <atomic>
#include <functional>
#include <vector>


#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct Galvanometer::GalvanometerImpl
{
	MainBoard *board;
	bool initiated;

	double scaleX;
	double scaleY;
	int offsetX;
	int offsetY;
	float timeStep;
	std::uint32_t timeDelay;

	std::uint32_t forePadds[3];
	std::uint32_t postPadds[3];

	double retinaPositionFunctionCoeffsX[GALVANO_POSITION_FUNCTION_DEGREE];
	double corneaPositionFunctionCoeffsX[GALVANO_POSITION_FUNCTION_DEGREE];
	double retinaPositionFunctionCoeffsY[GALVANO_POSITION_FUNCTION_DEGREE];
	double corneaPositionFunctionCoeffsY[GALVANO_POSITION_FUNCTION_DEGREE];

	vector<TraceProfile> traceProfiles;

	GalvanometerImpl() : board(nullptr), initiated(false),
		scaleX(1.0), scaleY(1.0), offsetX(0), offsetY(0), 
		timeStep(0.0f), timeDelay(0)
	{
		traceProfiles = vector<TraceProfile>(TRAJECT_NUMBER_OF_PROFILES);

		forePadds[0] = TRIGGER_FORE_PADDING_POINTS_AT_NORMAL;
		forePadds[1] = TRIGGER_FORE_PADDING_POINTS_AT_FASTER;
		forePadds[2] = TRIGGER_FORE_PADDING_POINTS_AT_FASTEST;

		postPadds[0] = TRIGGER_POST_PADDING_POINTS_AT_NORMAL;
		postPadds[1] = TRIGGER_POST_PADDING_POINTS_AT_FASTER;
		postPadds[2] = TRIGGER_POST_PADDING_POINTS_AT_FASTEST;
	}
};


Galvanometer::Galvanometer(MainBoard* board) :
	d_ptr(make_unique<GalvanometerImpl>())
{
	getImpl().board = board;
}


Galvanometer::~Galvanometer()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::Galvanometer::Galvanometer(Galvanometer && rhs) = default;
Galvanometer & OctDevice::Galvanometer::operator=(Galvanometer && rhs) = default;


void OctDevice::Galvanometer::initialize(void)
{
	getImpl().initiated = true;

	/*
	double xRetinaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		1.4410187436827788e-06, -9.7133441484331052e-20, -0.00011985701023880836, 6.4674054049174615e-18, 0.0028552466717464307, -1.0430290493911516e-16, 0.66944698562503657, 1.8456898150141549e-16
	};

	double yRetinaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		1.4623770940168971e-06, -6.9165776219763975e-20, -0.00012335554136967943, 3.6829185051551137e-18, 0.0028881275879500323, -5.9318454642637446e-17, 0.68738835283477329, 8.4979040753390628e-17
	};

	double xCorneaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		2.8877598008545472e-07, -5.2958064497304126e-20, 1.733657564647471e-05, -9.3165194010950477e-19, 0.0017106442380195397, 2.9964659245129259e-17, 0.53901998508231752, -1.8046256185991158e-17
	};

	double yCorneaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		3.9484961372912463e-07, 3.8841787253834272e-19, 1.3405609824851101e-05, -1.6076132093176448e-17, 0.0017142318572031768, 1.5137119801673921e-16, 0.52497030846982484, -3.6192091564353036e-17
	};
	*/

	double xRetinaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		-4e-05, -2e-05, 0.0007, 0.6045, 0.0016
	};

	double yRetinaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		2e-05, -0.0001, -0.0006, 0.6033, -0.0002
	};

	double xCorneaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		5e-6, 0.0003, -0.0008, 0.4676, 0.0097
	};

	double yCorneaCoeffs[GALVANO_POSITION_FUNCTION_DEGREE] = {
		5e-06, 0.0003, -3e-05, 0.4647, -0.0048
	};

	setRetinaPositionFunctionCoefficientsX(xRetinaCoeffs);
	setRetinaPositionFunctionCoefficientsY(yRetinaCoeffs);
	setCorneaPositionFunctionCoefficientsX(xCorneaCoeffs);
	setCorneaPositionFunctionCoefficientsY(yCorneaCoeffs);
	return;
}


bool OctDevice::Galvanometer::isInitiated(void) const
{
	return getImpl().initiated;
}


short OctDevice::Galvanometer::getStepXat(float mm, bool cornea)
{
	if (cornea) {
		return getStepXatCornea(mm);
	}
	else {
		return getStepXatRetina(mm);
	}
}


short OctDevice::Galvanometer::getStepXatRetina(float mm)
{
	double pos;
	double angle = 0.0;
	int degree = GALVANO_POSITION_FUNCTION_DEGREE;

	pos = mm * getImpl().scaleX;
	for (int i = 0; i < degree; i++) {
		angle += (pow(pos, i) * getImpl().retinaPositionFunctionCoeffsX[degree - i - 1]);
	}

	//angle = pos;
	int nStep = (int)(floor(((angle / 5.0) * GALVANO_STEP_RANGE_HALF) + 0.5));
	nStep += getImpl().offsetX;

	short sStep = (short)(max(min(nStep, GALVANO_STEP_RANGE_MAX), GALVANO_STEP_RANGE_MIN));
	return sStep;
}


short OctDevice::Galvanometer::getStepXatCornea(float mm)
{
	double pos;
	double angle = 0.0;
	int degree = GALVANO_POSITION_FUNCTION_DEGREE;

	pos = mm * getImpl().scaleX;
	for (int i = 0; i < degree; i++) {
		angle += (pow(pos, i) * getImpl().corneaPositionFunctionCoeffsX[degree - i - 1]);
	}

	int nStep = (int)(floor(((angle / 5.0) * GALVANO_STEP_RANGE_HALF) + 0.5));
	nStep += getImpl().offsetX;

	short sStep = (short)(max(min(nStep, GALVANO_STEP_RANGE_MAX), GALVANO_STEP_RANGE_MIN));
	return sStep;
}


short OctDevice::Galvanometer::getStepYat(float mm, bool cornea)
{
	if (cornea) {
		return getStepYatCornea(mm);
	}
	else {
		return getStepYatRetina(mm);
	}
}


short OctDevice::Galvanometer::getStepYatRetina(float mm)
{
	double pos;
	double angle = 0.0;
	int degree = GALVANO_POSITION_FUNCTION_DEGREE;

	pos = mm * getImpl().scaleY;
	for (int i = 0; i < degree; i++) {
		angle += (pow(pos, i) * getImpl().retinaPositionFunctionCoeffsY[degree - i - 1]);
	}

	//angle = pos;
	int nStep = (int)(floor(((angle / 5.0) * GALVANO_STEP_RANGE_HALF) + 0.5));
	nStep += getImpl().offsetY;

	short sStep = (short)(max(min(nStep, GALVANO_STEP_RANGE_MAX), GALVANO_STEP_RANGE_MIN));
	return sStep;
}


short OctDevice::Galvanometer::getStepYatCornea(float mm)
{
	double pos;
	double angle = 0.0;
	int degree = GALVANO_POSITION_FUNCTION_DEGREE;

	pos = mm * getImpl().scaleY;
	for (int i = 0; i < degree; i++) {
		angle += (pow(pos, i) * getImpl().corneaPositionFunctionCoeffsY[degree - i - 1]);
	}

	int nStep = (int)(floor(((angle / 5.0) * GALVANO_STEP_RANGE_HALF) + 0.5));
	nStep += getImpl().offsetY;

	short sStep = (short)(max(min(nStep, GALVANO_STEP_RANGE_MAX), GALVANO_STEP_RANGE_MIN));
	return sStep;
}


void OctDevice::Galvanometer::setScaleX(double scale)
{
	getImpl().scaleX = scale;
	return;
}


void OctDevice::Galvanometer::setScaleY(double scale)
{
	getImpl().scaleY = scale;
	return;
}


void OctDevice::Galvanometer::setTriggerTimeStep(float timeStep)
{
	getImpl().timeStep = timeStep;
	return;
}


void OctDevice::Galvanometer::setTriggerTimeDelay(std::uint32_t timeDelay)
{
	getImpl().timeDelay = timeDelay;
	return;
}


void OctDevice::Galvanometer::triggerForePaddings(ScanSpeed speed, std::uint32_t padds)
{
	int index = (speed == ScanSpeed::Normal ? 0 : (speed == ScanSpeed::Faster ? 1 : 2));
	getImpl().forePadds[index] = padds;
	return;
}


void OctDevice::Galvanometer::triggerPostPaddings(ScanSpeed speed, std::uint32_t padds)
{
	int index = (speed == ScanSpeed::Normal ? 0 : (speed == ScanSpeed::Faster ? 1 : 2));
	getImpl().postPadds[index] = padds;
	return;
}


float OctDevice::Galvanometer::getTriggerTimeStep(void)
{
	return getImpl().timeStep;
}


std::uint32_t OctDevice::Galvanometer::getTriggerTimeDelay(void)
{
	return getImpl().timeDelay;
}


std::uint32_t OctDevice::Galvanometer::triggerForePaddings(ScanSpeed speed)
{
	int index = (speed == ScanSpeed::Normal ? 0 : (speed == ScanSpeed::Faster ? 1 : 2));
	return getImpl().forePadds[index];
}


std::uint32_t OctDevice::Galvanometer::triggerPostPaddings(ScanSpeed speed)
{
	int index = (speed == ScanSpeed::Normal ? 0 : (speed == ScanSpeed::Faster ? 1 : 2));
	return getImpl().postPadds[index];
}


void OctDevice::Galvanometer::setRetinaPositionFunctionCoefficientsX(double coeffs[GALVANO_POSITION_FUNCTION_DEGREE])
{
	for (int i = 0; i < GALVANO_POSITION_FUNCTION_DEGREE; i++) {
		getImpl().retinaPositionFunctionCoeffsX[i] = coeffs[i];
	}
	return;
}


void OctDevice::Galvanometer::setCorneaPositionFunctionCoefficientsX(double coeffs[GALVANO_POSITION_FUNCTION_DEGREE])
{
	for (int i = 0; i < GALVANO_POSITION_FUNCTION_DEGREE; i++) {
		getImpl().corneaPositionFunctionCoeffsX[i] = coeffs[i];
	}
	return;
}


void OctDevice::Galvanometer::setRetinaPositionFunctionCoefficientsY(double coeffs[GALVANO_POSITION_FUNCTION_DEGREE])
{
	for (int i = 0; i < GALVANO_POSITION_FUNCTION_DEGREE; i++) {
		getImpl().retinaPositionFunctionCoeffsY[i] = coeffs[i];
	}
	return;
}


void OctDevice::Galvanometer::setCorneaPositionFunctionCoefficientsY(double coeffs[GALVANO_POSITION_FUNCTION_DEGREE])
{
	for (int i = 0; i < GALVANO_POSITION_FUNCTION_DEGREE; i++) {
		getImpl().corneaPositionFunctionCoeffsY[i] = coeffs[i];
	}
	return;
}


TraceProfile * OctDevice::Galvanometer::getTraceProfile(int traceId)
{
	if (traceId < 0 || traceId >= TRAJECT_NUMBER_OF_PROFILES) {
		return nullptr;
	}
	return &getImpl().traceProfiles[traceId];
}


bool OctDevice::Galvanometer::setTraceProfile(int traceId, short forePadd, short numPoints, short postPadd, short numRepeats)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	int totalCount = forePadd + numPoints + postPadd;

	profile->setNumberOfRepeats(numRepeats);
	profile->setSampleSize(totalCount);
	profile->setTriggerStartIndex(forePadd);
	profile->setTriggerCount(numPoints);

	TrajectoryProfileParams params;
	profile->getTrajectoryProfileParams(params);

	MainBoard* board = getMainBoard();
	UsbComm& usbComm = board->getUsbComm();

	uint32_t baseAddr = board->getBaseAddressOfTrajectoryProfile();
	if (usbComm.TrajectoryParams(traceId, baseAddr, &params)) {
		return true;
	}
	DebugOut2() << "Galvanometer::setTraceProfile() failed!";
	return false;
}


bool OctDevice::Galvanometer::setTraceProfile(int traceId, short forePadd, short numPoints, short postPadd, short numRepeats, float timeStep, uint32_t timeDelay)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	int totalCount = forePadd + numPoints + postPadd;
	int startIndex = forePadd + (int)(GALVANO_RESP_TIME_IN_US / timeStep);

	profile->setNumberOfRepeats(numRepeats);
	profile->setSampleSize(totalCount);
	profile->setTriggerStartIndex(startIndex);
	profile->setTriggerCount(numPoints);
	profile->setTimeStepInUs(timeStep);
	profile->setTriggerDelay(timeDelay);

	TrajectoryProfileParams params;
	profile->getTrajectoryProfileParams(params);

	MainBoard* board = getMainBoard();
	UsbComm& usbComm = board->getUsbComm();

	uint32_t baseAddr = board->getBaseAddressOfTrajectoryProfile();
	if (usbComm.TrajectoryParams(traceId, baseAddr, &params)) {
		return true;
	}
	DebugOut2() << "Galvanometer::setTraceProfile() failed!";
	return false;
}


bool OctDevice::Galvanometer::setTracePositionsX(int traceId, short* posXs, short count)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	if (count <= 0) {
		return false;
	}

	short* buff = profile->getPositionsX();
	memcpy(buff, posXs, sizeof(short)*count);

	MainBoard* board = getMainBoard();
	UsbComm& usbComm = board->getUsbComm();

	uint32_t baseAddr = board->getBaseAddressOfTrajectoryProfile();
	if (usbComm.TrajectoryPositionsX(traceId, baseAddr, buff, count)) {
		return true;
	}
	DebugOut2() << "Galvanometer::setTracePositionsX() failed!";
	return false;
}


bool OctDevice::Galvanometer::setTracePositionsY(int traceId, short* posYs, short count)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	if (count <= 0) {
		return false;
	}

	short* buff = profile->getPositionsY();
	memcpy(buff, posYs, sizeof(short)*count);

	MainBoard* board = getMainBoard();
	UsbComm& usbComm = board->getUsbComm();

	uint32_t baseAddr = board->getBaseAddressOfTrajectoryProfile();
	if (usbComm.TrajectoryPositionsY(traceId, baseAddr, buff, count)) {
		return true;
	}
	DebugOut2() << "Galvanometer::setTracePositionsY() failed!";
	return false;
}


bool OctDevice::Galvanometer::buildTracePositionsX(EyeSide eyeSide, short forePadd, short numPoints, short postPadd,
									float start, float close, bool cornea, bool circle, short * buffer, short* count)
{
	if (!isInitiated()) {
		return false;
	}

	int totalCount = forePadd + numPoints + postPadd;
	int startIndex = forePadd;

	TraceProfile profile;
	profile.setSampleSize(totalCount);
	profile.setTriggerStartIndex(startIndex);
	profile.setTriggerCount(numPoints);

	short index = 0;

	if (!circle)
	{
		double interval = double(close - start) / (profile.getTriggerCount() - 1);
		int startIdx = profile.getTriggerStartIndex() * -1;
		int closeIdx = profile.getSampleSize() + startIdx;

		for (int i = startIdx; i < closeIdx; i++) {
			buffer[index] = getStepXat(start + (float)(interval * i), cornea);
			index++;
			// DebugOut2() << "x" << i << " : " << buff[count - 1];
		}
	}
	else
	{
		double radius = fabs(close - start) * 0.5;
		double center = (close + start) * 0.5;
		double angle = 360.0 / numPoints;

		int startIdx = profile.getTriggerStartIndex() * -1;
		int closeIdx = profile.getSampleSize() + startIdx;

		float position;
		double degree, radian;
		int direction = 1; //  (eyeSide == EyeSide::OD ? +1 : -1);
		for (int i = startIdx; i < closeIdx; i++) {
			degree = i * angle;
			radian = NumericFunc::degreeToRadian(degree);
			position = (float)((radius * sin(radian) * direction) + center);
			buffer[index] = getStepXat(position, cornea);
			index++;
		}
	}
	*count = index;
	return true;
}


bool OctDevice::Galvanometer::buildTracePositionsY(EyeSide eyeSide, short forePadd, short numPoints, short postPadd,
									float start, float close, bool cornea, bool circle, short * buffer, short* count)
{
	if (!isInitiated()) {
		return false;
	}

	int totalCount = forePadd + numPoints + postPadd;
	int startIndex = forePadd;

	TraceProfile profile;
	profile.setSampleSize(totalCount);
	profile.setTriggerStartIndex(startIndex);
	profile.setTriggerCount(numPoints);

	short index = 0;

	if (!circle)
	{
		double interval = double(close - start) / (profile.getTriggerCount() - 1);
		int startIdx = profile.getTriggerStartIndex() * -1;
		int closeIdx = profile.getSampleSize() + startIdx;

		for (int i = startIdx; i < closeIdx; i++) {
			buffer[index] = getStepYat(start + (float)(interval * i), cornea);
			index++;
			// DebugOut2() << "y" << i << " : " << buff[count - 1];
		}
	}
	else {
		double radius = fabs(close - start) * 0.5;
		double center = (close + start) * 0.5;
		double angle = 360.0 / numPoints;

		int startIdx = profile.getTriggerStartIndex() * -1;
		int closeIdx = profile.getSampleSize() + startIdx;

		float position;
		double degree, radian;
		int direction = (eyeSide == EyeSide::OD ? +1 : -1);
		for (int i = startIdx; i < closeIdx; i++) {
			degree = i * angle;
			radian = NumericFunc::degreeToRadian(degree);
			position = (float)((radius * cos(radian) * direction) + center);
			buffer[index] = getStepYat(position, cornea);
			index++;
		}
	}
	*count = index;
	return true;
}


bool OctDevice::Galvanometer::scanMoveXY(int traceId)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoScanXY(traceId, traceId)) {
		return true;
	}
	DebugOut2() << "Galvanometer::scanMoveXY() failed!";
	return false;
}


bool OctDevice::Galvanometer::scanRepeatX(int traceId, int repeats, short offsetX, short offsetY)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoRasterX(traceId, repeats, offsetX, offsetY)) {
	// if (usbComm.GalvanoRasterXY(traceId, repeats, offsetX, offsetY)) {
		return true;
	}
	return false;
}


bool OctDevice::Galvanometer::scanRepeatY(int traceId, int repeats, short offsetX, short offsetY)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoRasterY(traceId, repeats, offsetX, offsetY)) {
	// if (usbComm.GalvanoRasterXY(traceId, repeats, offsetX, offsetY)) {
		return true;
	}
	return false;
}


bool OctDevice::Galvanometer::scanRepeatFastX(int traceId, int repeats, short offsetX, short offsetY)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoRasterFastX(traceId, repeats, offsetX, offsetY)) {
		// if (usbComm.GalvanoRasterXY(traceId, repeats, offsetX, offsetY)) {
		return true;
	}
	return false;
}


bool OctDevice::Galvanometer::scanRepeatFastY(int traceId, int repeats, short offsetX, short offsetY)
{
	if (!isInitiated()) {
		return false;
	}

	TraceProfile* profile = getTraceProfile(traceId);
	if (profile == nullptr) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoRasterFastY(traceId, repeats, offsetX, offsetY)) {
		// if (usbComm.GalvanoRasterXY(traceId, repeats, offsetX, offsetY)) {
		return true;
	}
	return false;
}


bool OctDevice::Galvanometer::slewMoveXY(float xpos, float ypos, bool cornea)
{
	if (!isInitiated()) {
		return false;
	}

	short xStep = getStepXat(xpos, cornea);
	short yStep = getStepYat(ypos, cornea);

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoSlewXY(xStep, yStep)) {
		return true;
	}
	return false;
}


bool OctDevice::Galvanometer::slewMoveXY(short xpos, short ypos)
{
	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.GalvanoSlewXY(xpos, ypos)) {
		return true;
	}
	return false;
}

bool OctDevice::Galvanometer::setAutoSlew(uint16_t flag)
{
	UsbComm& usbComm = getMainBoard()->getUsbComm();
	uint32_t baseAddr = getMainBoard()->getBaseAddressOfSysConfiguration();
	if (usbComm.GalvanoAutoSlew(baseAddr, flag)) {
		return true;
	}
	return false;
}

bool OctDevice::Galvanometer::setSlewRate(uint32_t rate)
{
	UsbComm& usbComm = getMainBoard()->getUsbComm();
	uint32_t baseAddr = getMainBoard()->getBaseAddressOfSysConfiguration();
	if (usbComm.GalvanoSlewRate(baseAddr, rate)) {
		LogD() << "Galvano slew rate: " << rate;
		return true;
	}
	return false;
}


Galvanometer::GalvanometerImpl & OctDevice::Galvanometer::getImpl(void) const
{
	return *d_ptr;
}


MainBoard * OctDevice::Galvanometer::getMainBoard(void) const
{
	return getImpl().board;
}



