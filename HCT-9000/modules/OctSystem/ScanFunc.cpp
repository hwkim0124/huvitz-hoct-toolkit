#include "stdafx.h"
#include "ScanFunc.h"

#include <iostream>
#include <fstream>

#include "OctPattern2.h"
#include "OctDevice2.h"
#include "OctGrab2.h"
#include "SigChain2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctPattern;
using namespace OctDevice;
using namespace OctGrab;
using namespace SigChain;
using namespace std;
using namespace CppUtil;


struct ScanFunc::ScanFuncImpl
{
	MainBoard* board;
	FrameGrabber* grabber;
	Usb3Grabber* usb3Grab;
	Galvanometer* galvano;

	bool initiated;

	ScanFuncImpl() : board(nullptr), grabber(nullptr), usb3Grab(nullptr), galvano(nullptr)
	{
	}
};


ScanFunc::ScanFunc()
{
}


ScanFunc::~ScanFunc()
{
}


bool OctSystem::ScanFunc::initScanFunc(OctDevice::MainBoard * board, OctGrab::FrameGrabber * grabber)
{
	getImpl().board = board;
	getImpl().galvano = board->getGalvanometer();
	getImpl().grabber = grabber;
	getImpl().initiated = true;
	return true;
}


bool OctSystem::ScanFunc::initScanFunc(OctDevice::MainBoard * board, OctGrab::Usb3Grabber * grabber)
{
	getImpl().board = board;
	getImpl().galvano = board->getGalvanometer();
	getImpl().usb3Grab = grabber;
	getImpl().initiated = true;
	return true;
}


bool OctSystem::ScanFunc::isInitiated(void)
{
	return getImpl().initiated;
}


void OctSystem::ScanFunc::registerUsb3Grabber(OctGrab::Usb3Grabber * grabber)
{
	getImpl().usb3Grab = grabber;
	return;
}


bool OctSystem::ScanFunc::prepareChainSetup(PatternDomain domain, PatternName name, ScanSpeed speed, bool measure)
{
	if (!measure) {
		ChainSetup::setPatternDomain(domain);
		ChainSetup::setPatternSpeed(speed);
		ChainSetup::setPatternName(name);
		ChainSetup::updateSpectrometerParameters();
		ChainSetup::updateDispersionParameters();
		ChainSetup::clearBackgroundSpectrum();
		ChainSetup::resetDynamicDispersionCorrection();

		// if (d_ptr->scanMeasure.getPattern()._name == PatternName::AnteriorLine) {
		ChainSetup::useACALinePatternPreview(true, true);
		// }
	}
	else {
		ChainSetup::useACALinePatternPreview(true, false);
	}
	return true;
}


bool OctSystem::ScanFunc::updateLineTrace(OctPattern::LineTrace* line, ScanSpeed speed, bool foreDist)
{
	if (!line) {
		return false;
	}

	auto forePadds = getGalvanometer()->triggerForePaddings(speed);
	auto postPadds = getGalvanometer()->triggerPostPaddings(speed);
	auto scanPoints = line->getNumberOfScanPoints();
	auto timeStep = getGalvanometer()->getTriggerTimeStep();

	if (foreDist) {
		forePadds = line->getForePaddings();
	}
	else {
		scanPoints = 0;
	}

	line->setPaddings(forePadds, postPadds, scanPoints);
	line->setScanSpeed(speed);
	return true;
}


bool OctSystem::ScanFunc::updatePatternPositions(EyeSide eyeSide, const PatternPlan & pattern, ScanSpeed speed)
{
	if (!updatePreviewPositions(eyeSide, pattern, speed)) {
		return false;
	}
	if (!updateMeasurePositions(eyeSide, pattern, speed)) {
		return false;
	}
	if (!updateEnfacePositions(eyeSide, pattern, speed)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanFunc::updatePreviewPositions(EyeSide eyeSide, const PatternPlan & pattern, ScanSpeed speed)
{
	if (!isInitiated()) {
		return false;
	}

	bool cornea = pattern.isCornea();
	PatternFrameVect& frames = pattern.getPreviewScan().getPatternFrames();
	size_t fsize = frames.size();

	for (int i = 0; i < fsize; i++) {
		LineTraceVect& lines = frames[i].getLineTraceList();
		size_t lsize = lines.size();
		for (int j = 0; j < lsize; j++) {
			LineTrace* pLine = &lines[j];
			updateLineTrace(pLine, speed);
			if (!updateTracePositions(eyeSide, pattern, *pLine, cornea)) {
				LogE() << "Update trace positions in preview failed!, frame: " << i << ", line: " << j;
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::ScanFunc::updateMeasurePositions(EyeSide eyeSide, const PatternPlan & pattern, ScanSpeed speed)
{
	if (!isInitiated()) {
		return false;
	}

	bool cornea = pattern.isCornea();
	PatternFrameVect& frames = pattern.getMeasureScan().getPatternFrames();
	size_t fsize = frames.size();

	for (int i = 0; i < fsize; i++) {
		LineTraceVect& lines = frames[i].getLineTraceList();
		size_t lsize = lines.size();
		for (int j = 0; j < lsize; j++) {
			LineTrace* pLine = &lines[j];
			updateLineTrace(pLine, speed, pattern.getMeasureScan().useForeDistance());
			if (!updateTracePositions(eyeSide, pattern, *pLine, cornea)) {
				LogE() << "Update trace positions in measure failed!, frame: " << i << ", line: " << j;
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::ScanFunc::updateEnfacePositions(EyeSide eyeSide, const PatternPlan & pattern, ScanSpeed speed)
{
	if (!isInitiated()) {
		return false;
	}

	bool cornea = pattern.isCornea();
	PatternFrameVect& frames = pattern.getEnfaceScan().getPatternFrames();
	size_t fsize = frames.size();

	for (int i = 0; i < fsize; i++) {
		LineTraceVect& lines = frames[i].getLineTraceList();
		size_t lsize = lines.size();
		for (int j = 0; j < lsize; j++) {
			LineTrace* pLine = &lines[j];
			updateLineTrace(pLine, speed);
			if (!updateTracePositions(eyeSide, pattern, *pLine, cornea)) {
				LogE() << "Update trace positions in enface failed!, frame: " << i << ", line: " << j;
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::ScanFunc::updateTracePositions(EyeSide eyeSide, const PatternPlan& pattern, OctPattern::LineTrace & line, bool cornea)
{
	Galvanometer* galvano = getGalvanometer();

	short forePadds = line.getForePaddings();
	short numPoints = line.getNumberOfScanPoints();
	short postPadds = line.getPostPaddings();

	float startX = line.getStartX();
	float closeX = line.getCloseX();
	float startY = line.getStartY();
	float closeY = line.getCloseY();
	bool circle = line.isCircle();

	short buffer[GALVANO_TRACE_POSITION_BUFFER_SIZE];
	short count;

	// Reverse direction of x, y coordinate
	startX *= -1.0f;
	closeX *= -1.0f;
	startY *= -1.0f;
	closeY *= -1.0f;

	/*
	if (!galvano->buildTracePositionsX(forePadds, numPoints, postPadds, startX, closeX, cornea, circle, buffer, &count)) {
	return false;
	}
	*/
	if (!galvano->buildTracePositionsX(eyeSide, forePadds, numPoints, postPadds, startY, closeY, cornea, circle, buffer, &count)) {
		return false;
	}

	if (line.isHidden()) {
		for (int i = 0; i < count; i++) {
			buffer[i] = GALVANO_STEP_RANGE_MIN;
		}
	}
	else {
		// Apply scan offset with amount of galvano positions.
		for (int i = 0; i < count; i++) {
			buffer[i] += (short)pattern.getMeasureScan().getScanMoveY();
		}
	}

	line.setGalvanoPositionsX(buffer, count);

	/*
	if (!galvano->buildTracePositionsY(forePadds, numPoints, postPadds, startY, closeY, cornea, circle, buffer, &count)) {
	return false;
	}
	*/
	if (!galvano->buildTracePositionsY(eyeSide, forePadds, numPoints, postPadds, startX, closeX, cornea, circle, buffer, &count)) {
		return false;
	}

	if (line.isHidden()) {
		for (int i = 0; i < count; i++) {
			buffer[i] = GALVANO_STEP_RANGE_MIN;
		}
	}
	else {
		// Apply scan offset with amount of galvano positions.
		for (int i = 0; i < count; i++) {
			buffer[i] += (short)pattern.getMeasureScan().getScanMoveX();
		}
	}

	line.setGalvanoPositionsY(buffer, count);
	return true;
}


bool OctSystem::ScanFunc::exportPatternPositions(const OctPattern::PatternPlan & pattern, std::string filename)
{
	ofstream file(filename);
	if (file.is_open()) {
		file << "[Preview Pattern]\n";
		auto frame = pattern.getPreviewScan().getPatternFrame(0);
		if (frame != nullptr) {
			auto line = frame->getLineTrace(0);
			if (line != nullptr) {
				auto xpos = line->getGalvanoPositionsX();
				auto xcnt = line->getCountOfPositionsX();
				file << "Galvano Position Xs, size: " << xcnt << "\n";
				for (int i = 0; i < xcnt; i++) {
					file << i << " : " << xpos[i] << "\n";
				}
				auto ypos = line->getGalvanoPositionsY();
				auto ycnt = line->getCountOfPositionsY();
				file << "Galvano Position Ys, size: " << xcnt << "\n";
				for (int i = 0; i < ycnt; i++) {
					file << i << " : " << ypos[i] << "\n";
				}
			}
		}
		file.close();
	}
	else {
		return false;
	}
	return true;
}


bool OctSystem::ScanFunc::uploadPatternProfiles(const PatternPlan & pattern, bool measure)
{
	if (!measure) {
		// Upload the first lines of preview frame with trace profile index into mainboard.
		if (!uploadPreviewProfiles(pattern)) {
			return false;
		}

		if (!uploadEnfaceProfiles(pattern)) {
			return false;
		}
	}
	else {
		// Upload the first lines of measure frame with trace profile index into mainboard. 
		// The following lines within frame are operated through control command parameters
		// of scan ACQ with number of scan lines and x, y offsets. 

		// Note that in case the following line trajectory within frame can't be covered 
		// via x, y offsets from the previous line's, then it should be added as next frame. 
		if (!uploadMeasureProfiles(pattern)) {
			return false;
		}
	}
	return true;
}


bool OctSystem::ScanFunc::uploadPreviewProfiles(const PatternPlan & pattern)
{
	if (!isInitiated()) {
		return false;
	}

	PatternFrameVect& frames = pattern.getPreviewScan().getPatternFrames();
	size_t fsize = frames.size();

	for (int i = 0; i < fsize; i++) {
		LineTrace* pLine = frames[i].getLineTraceFirst();
		if (pLine != nullptr) {
			if (uploadTraceProfile(*pLine)) {
				LogD() << "Trace profile uploaded for preview, frame: " << i << ", traceId: " << pLine->getTraceId();
			}
			else {
				LogE() << "Upload trace profile for preview failed!, frame: " << i << ", traceId: " << pLine->getTraceId();
				return false;
			}
		}
		else {
			LogE() << "Invalid trace profile to preview!, frame: " << i << ", traceId: " << pLine->getTraceId();
			return false;
		}
	}
	return true;
}


bool OctSystem::ScanFunc::uploadMeasureProfiles(const PatternPlan & pattern, int startIdx, int maxLines)
{
	if (!isInitiated()) {
		return false;
	}

	PatternFrameVect& frames = pattern.getMeasureScan().getPatternFrames();
	size_t fsize = frames.size();

	int count = 0;
	for (int i = startIdx; i < fsize; i++) {
		LineTrace* pLine = frames[i].getLineTraceFirst();
		if (pLine != nullptr) {
			if (uploadTraceProfile(*pLine)) {
				LogD() << "Trace profile uploaded for measure, frame: " << i << ", traceId: " << pLine->getTraceId();
			}
			else {
				LogE() << "Upload trace profile for measure failed!, frame: " << i << ", traceId: " << pLine->getTraceId();
				return false;
			}
		}
		else {
			LogE() << "Invalid trace profile for measure!, frame: " << i << ", traceId: " << pLine->getTraceId();
			return false;
		}
		if (++count >= maxLines) {
			break;
		}
	}
	return true;
}


bool OctSystem::ScanFunc::uploadEnfaceProfiles(const PatternPlan & pattern)
{
	if (!isInitiated()) {
		return false;
	}

	PatternFrameVect& frames = pattern.getEnfaceScan().getPatternFrames();
	size_t fsize = frames.size();

	for (int i = 0; i < fsize; i++) {
		LineTrace* pLine = frames[i].getLineTraceFirst();
		if (pLine != nullptr) {
			if (uploadTraceProfile(*pLine)) {
				LogD() << "Trace profile uploaded for enface, frame: " << i << ", traceId: " << pLine->getTraceId();
			}
			else {
				LogE() << "Upload trace profile for enface failed!, frame: " << i << ", traceId: " << pLine->getTraceId();
				return false;
			}
		}
		else {
			LogE() << "Invalid trace profile for enface!, frame: " << i << ", traceId: " << pLine->getTraceId();
			return false;
		}
	}
	return true;
}


bool OctSystem::ScanFunc::uploadTraceProfile(OctPattern::LineTrace & line)
{
	Galvanometer* galvano = getGalvanometer();

	int traceId = line.getTraceId();
	short forePadds = line.getForePaddings();
	short numPoints = line.getNumberOfScanPoints();
	short postPadds = line.getPostPaddings();
	short numRepeats = line.getNumberOfRepeats();

	float startX = line.getStartX();
	float closeX = line.getCloseX();
	float startY = line.getStartY();
	float closeY = line.getCloseY();

	short* bufferX = nullptr;
	short* bufferY = nullptr;
	int countX = 0;
	int countY = 0;

	float timeStep = galvano->getTriggerTimeStep();
	uint32_t timeDelay = galvano->getTriggerTimeDelay();


	// Transmit a trace profile with id into mainboard. 
	if (!galvano->setTraceProfile(traceId, forePadds, numPoints, postPadds, numRepeats, timeStep, timeDelay)) {
		return false;
	}

	// Transmit x coordinates along the line. 
	bufferX = line.getGalvanoPositionsX();
	countX = line.getCountOfPositionsX();
	if (!galvano->setTracePositionsX(traceId, bufferX, countX)) {
		return false;
	}
	/*
	if (!galvano->setTracePositionsY(traceId, bufferX, countX)) {
	return false;
	}
	*/

	// Transmit y coordinates along the line. 
	bufferY = line.getGalvanoPositionsY();
	countY = line.getCountOfPositionsY();
	if (!galvano->setTracePositionsY(traceId, bufferY, countY)) {
		return false;
	}
	/*
	if (!galvano->setTracePositionsX(traceId, bufferY, countY)) {
	return false;
	}
	*/

	LogD() << "Trace id: " << traceId << ", posX: " << bufferX[0] << ", " << bufferX[countX - 1] << ", posY: " << bufferY[0] << ", " << bufferY[countY - 1];
	LogD() << "forePadd: " << forePadds << ", numPoints: " << numPoints << ", postPadd: " << postPadds << ", numRepeats: " << numRepeats;
	LogD() << "xRange: " << startX << ", " << closeX << ", yRange: " << startY << ", " << closeY;

	if (line.isRasterFastX() || line.isRasterFastY() || line.isRasterX() || line.isRasterY())
	{
		// Transmit a trace profile with id into mainboard. 
		if (!galvano->setTraceProfile(traceId + 1, forePadds, numPoints, postPadds, numRepeats, timeStep, timeDelay)) {
			return false;
		}

		// Transmit x coordinates along the line in reversed direction.
		bufferX = line.getGalvanoPositionsX(true);
		countX = line.getCountOfPositionsX();
		if (!galvano->setTracePositionsX(traceId + 1, bufferX, countX)) {
			return false;
		}
		/*
		if (!galvano->setTracePositionsY(traceId + 1, bufferX, countX)) {
		return false;
		}
		*/

		// Transmit y coordinates along the line in reversed direction.
		bufferY = line.getGalvanoPositionsY(true);
		countY = line.getCountOfPositionsY();
		if (!galvano->setTracePositionsY(traceId + 1, bufferY, countY)) {
			return false;
		}
		/*
		if (!galvano->setTracePositionsX(traceId + 1, bufferY, countY)) {
		return false;
		}
		*/
	}
	return true;
}


bool OctSystem::ScanFunc::updatePatternBuffers(const PatternPlan & pattern, bool measure, bool reset)
{
	if (reset) {
		releaseGrabberBuffers();
	}

	if (!measure) {
		if (!updatePreviewBuffers(pattern)) {
			return false;
		}
		if (!updateEnfaceBuffers(pattern)) {
			return false;
		}
	}
	else {
		if (!updateMeasureBuffers(pattern)) {
			return false;
		}
	}
	return true;
}


bool OctSystem::ScanFunc::updatePreviewBuffers(const PatternPlan & pattern)
{
	if (!isInitiated()) {
		return false;
	}

	// vector<int> sizes = pattern.getPreviewScan().getLateralSizeListOfFrame(0, false);
	
	PatternFrameVect& frames = pattern.getPreviewScan().getPatternFrames();
	vector<int> sizes;

	for (PatternFrame& frame : frames) {
		auto line = frame.getLineTraceFirst();
		if (!line->isHidden()) {
			vector<int> fsize = frame.getLateralSizeList(true);
			sizes.insert(end(sizes), begin(fsize), end(fsize));
		}
	}

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->prepareBuffersToPreview(sizes)) {
			getUsb3Grab()->releaseBuffersToPreview();
			return false;
		}
	}
	else {
		// Create MIL buffers corresponding to the frist frame data. 
		if (!getGrabber()->prepareBuffersToPreview(sizes)) {
			getGrabber()->releaseBuffersToPreview();
			return false;
		}
	}

	return true;
}


bool OctSystem::ScanFunc::updateMeasureBuffers(const PatternPlan & pattern)
{
	if (!isInitiated()) {
		return false;
	}

	PatternFrameVect& frames = pattern.getMeasureScan().getPatternFrames();
	vector<int> sizes;

	int count = 0;
	int depth = 0;
	for (PatternFrame& frame : frames) {
		vector<int> fsize = frame.getLateralSizeList(true);
		if (depth == 0 && pattern.getMeasureScan().isScan3D()) {
			depth = (int)fsize.size();
			LogD() << "Pattern depth: " << depth;
		}
		sizes.insert(end(sizes), begin(fsize), end(fsize));
	}

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->prepareBuffersToMeasure(sizes, depth)) {
			getUsb3Grab()->releaseBuffersToMeasure();
			return false;
		}
	}
	else {
		// Create MIL buffers corresponding to the frist frame data. 
		if (!getGrabber()->prepareBuffersToMeasure(sizes, depth)) {
			getGrabber()->releaseBuffersToMeasure();
			return false;
		}
	}

	return true;
}


bool OctSystem::ScanFunc::updateEnfaceBuffers(const PatternPlan & pattern)
{
	if (!isInitiated()) {
		return false;
	}

	vector<int> sizes = pattern.getEnfaceScan().getLateralSizeListOfFrame(0, false);

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->prepareBuffersToEnface(sizes)) {
			getUsb3Grab()->releaseBuffersToEnface();
			return false;
		}
	}
	else {
		// Create MIL buffers corresponding to the frist frame data. 
		if (!getGrabber()->prepareBuffersToEnface(sizes)) {
			getGrabber()->releaseBuffersToEnface();
			return false;
		}
	}

	return true;
}


void OctSystem::ScanFunc::releaseGrabberBuffers(void)
{
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getUsb3Grab()->releaseBuffersToPattern();
	}
	else {
		getGrabber()->releaseBuffersToPattern();
	}
	return;
}


OctGrab::FrameGrabber * OctSystem::ScanFunc::getGrabber(void)
{
	return getImpl().grabber;
}

OctGrab::Usb3Grabber * OctSystem::ScanFunc::getUsb3Grab(void)
{
	return getImpl().usb3Grab;
}

OctDevice::MainBoard * OctSystem::ScanFunc::getMainboard(void)
{
	return getImpl().board;
}


OctDevice::Galvanometer * OctSystem::ScanFunc::getGalvanometer(void)
{
	return getImpl().galvano;
}


// Direct initialization of static smart pointer.
std::unique_ptr<ScanFunc::ScanFuncImpl> ScanFunc::d_ptr(new ScanFuncImpl());


ScanFunc::ScanFuncImpl & OctSystem::ScanFunc::getImpl(void)
{
	return *d_ptr;
}
