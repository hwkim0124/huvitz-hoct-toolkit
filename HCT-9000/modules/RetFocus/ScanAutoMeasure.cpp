#include "stdafx.h"
#include "ScanAutoMeasure.h"

#include <mutex>
#include <atomic>

#include "OctDevice2.h"
#include "CppUtil2.h"
#include "SigChain2.h"

using namespace RetFocus;
using namespace OctDevice;
using namespace CppUtil;
using namespace SigChain;
using namespace std;


struct ScanAutoMeasure::ScanAutoMeasureImpl
{
	MainBoard* board;
	bool initiated;
	
	int referRetina = -1;
	int referCornea = -1;
	int referLensFront = -1;
	int referLensBack = -1;

	float dioptRetina = 0.0f;
	float dioptCornea = 0.0f;
	float dioptLensFront = 0.0f;
	float dioptLensBack = 0.0f;

	float polarRetina = 0.0f;
	float polarCornea = 0.0f;
	float polarLensFront = 0.0f;
	float polarLensBack = 0.0f;

	bool compenLens;
	float axialLength;
	float quality;

	FocusTarget target = FocusTarget::RETINA;

	ScanAutoMeasureImpl() : initiated(false), board(nullptr), referRetina(0), referCornea(0), axialLength(0.0f)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanAutoMeasure::ScanAutoMeasureImpl> ScanAutoMeasure::d_ptr(new ScanAutoMeasureImpl());


ScanAutoMeasure::ScanAutoMeasure()
{
}


ScanAutoMeasure::~ScanAutoMeasure()
{
}


bool RetFocus::ScanAutoMeasure::initiateAxialLength(OctDevice::MainBoard * board, bool reset)
{
	d_ptr->board = board;
	d_ptr->initiated = true;

	if (reset) {
		d_ptr->referCornea = -1;
		d_ptr->referRetina = -1;
		d_ptr->referLensFront = -1;
		d_ptr->referLensBack = -1;
	}

	d_ptr->axialLength = 0.0f;
	return true;
}


bool RetFocus::ScanAutoMeasure::initialize(OctDevice::MainBoard * board, FocusTarget target, bool compLens)
{
	d_ptr->board = board;
	d_ptr->initiated = true;
	d_ptr->compenLens = compLens;
	d_ptr->target = target;

	if (target == FocusTarget::CORNEA) {
		// d_ptr->referCornea = -1;
	}
	else if (target == FocusTarget::RETINA) {
		// d_ptr->referRetina = -1;
	}
	else if (target == FocusTarget::LENS_FRONT) {
		// d_ptr->referLensFront = -1;
	}
	else if (target == FocusTarget::LENS_BACK) {
		// d_ptr->referLensBack = -1;
	}
	else if (target == FocusTarget::AXIAL_LENGTH) {
		d_ptr->referCornea = -1;
		d_ptr->referRetina = -1;
	}
	else if (target == FocusTarget::LENS_THICKNESS) {
		d_ptr->referCornea = -1;
		d_ptr->referLensFront = -1;
		d_ptr->referLensBack = -1;
	}
	return true;
}


bool RetFocus::ScanAutoMeasure::isInitialized(void)
{
	return d_ptr->initiated;
}


void RetFocus::ScanAutoMeasure::clearResult(void)
{
	d_ptr->referRetina = -1;
	d_ptr->referCornea = -1;
	d_ptr->referLensFront = -1;
	d_ptr->referLensBack = -1;

	d_ptr->dioptRetina = 0.0f;
	d_ptr->dioptCornea = 0.0f;
	d_ptr->dioptLensFront = 0.0f;
	d_ptr->dioptLensBack = 0.0f;

	d_ptr->polarRetina = 0.0f;
	d_ptr->polarCornea = 0.0f;
	d_ptr->polarLensFront = 0.0f;
	d_ptr->polarLensBack = 0.0f;
	return;
}


bool RetFocus::ScanAutoMeasure::isCompensationLensMode(void)
{
	return d_ptr->compenLens;
}


bool RetFocus::ScanAutoMeasure::recordPositionOfRetina(bool confirm)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	auto currRefer = motor->getPosition();
	auto currDiopt = getMainBoard()->getOctFocusMotor()->getCurrentDiopter();
	auto currPolar = getMainBoard()->getPolarizationMotor()->getCurrentDegree();
	bool result = false;

	if (confirm) {
		result = checkIfTargetExist();
	}
	else {
		result = true;
	}

	if (result) {
		d_ptr->referRetina = currRefer;
		d_ptr->dioptRetina = currDiopt;
		d_ptr->polarRetina = currPolar;
	}
	LogD() << "Auto Measure: record position of retina, refer: " << currRefer << ", diopt: " << currDiopt << ", polar: " << currPolar;
	return result;
}


bool RetFocus::ScanAutoMeasure::recordPositionOfCornea(bool confirm)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	auto currRefer = motor->getPosition();
	auto currDiopt = getMainBoard()->getOctFocusMotor()->getCurrentDiopter();
	auto currPolar = getMainBoard()->getPolarizationMotor()->getCurrentDegree();
	bool result = false;

	if (confirm) {
		result = checkIfTargetExist();
	}
	else {
		result = true;
	}

	if (result) {
		d_ptr->referCornea = currRefer;
		d_ptr->dioptCornea = currDiopt;
		d_ptr->polarCornea = currPolar;
	}
	LogD() << "Auto Measure: record position of cornea, refer: " << currRefer << ", diopt: " << currDiopt << ", polar: " << currPolar;
	return result;
}


bool RetFocus::ScanAutoMeasure::recordPositionOfLensFront(bool confirm)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	auto currRefer = motor->getPosition();
	auto currDiopt = getMainBoard()->getOctFocusMotor()->getCurrentDiopter();
	auto currPolar = getMainBoard()->getPolarizationMotor()->getCurrentDegree();

	int numRetry = 9;
	int numSuccess = 3;
	int countRetry = 0;
	int countOkay = 0;
	bool result = false;

	if (confirm) {
		while (true) {
			if (!renewQualityIndex()) {
				return false;
			}
			if (!isTargetFound()) {
				if (countRetry++ >= numRetry) {
					LogD() << "Auto Measure: lens frontside not found!";
					break;
				}
				countOkay = 0;
			}
			else {
				if (countOkay++ >= numSuccess) {
					result = true;
					break;
				}
				countRetry = 0;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}
	else {
		result = true;
	}

	if (result) {
		d_ptr->referLensFront = currRefer;
		d_ptr->dioptLensFront = currDiopt;
		d_ptr->polarLensFront = currPolar;
	}
	LogD() << "Auto Measure: record position of lens frontside, refer: " << currRefer << ", diopt: " << currDiopt << ", polar: " << currPolar;
	return true;
}


bool RetFocus::ScanAutoMeasure::recordPositionOfLensBack(bool confirm)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	auto currRefer = motor->getPosition();
	auto currDiopt = getMainBoard()->getOctFocusMotor()->getCurrentDiopter();
	auto currPolar = getMainBoard()->getPolarizationMotor()->getCurrentDegree();

	int numRetry = 9;
	int numSuccess = 3;
	int countRetry = 0;
	int countOkay = 0;
	bool result = false;

	if (confirm) {
		while (true) {
			if (!renewQualityIndex()) {
				return false;
			}
			if (!isTargetFound()) {
				if (countRetry++ >= numRetry) {
					LogD() << "Auto Measure: lens backside not found!";
					break;
				}
				countOkay = 0;
			}
			else {
				if (countOkay++ >= numSuccess) {
					result = true;
					break;
				}
				countRetry = 0;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}
	else {
		result = true;
	}

	if (result) {
		d_ptr->referLensBack = currRefer;
		d_ptr->dioptLensBack = currDiopt;
		d_ptr->polarLensBack = currPolar;
	}
	LogD() << "Auto Measure: record position of lens backside, refer: " << currRefer << ", diopt: " << currDiopt << ", polar: " << currPolar;
	return true;
}


bool RetFocus::ScanAutoMeasure::makeDelayToProceed(int count)
{
	for (int i = 0; i < count; i++) {
		if (!renewQualityIndex()) {
			return false;
		}
		this_thread::sleep_for(chrono::milliseconds(50));
	}
	return true;
}


bool RetFocus::ScanAutoMeasure::checkIfTargetExist(void)
{
	int countMax = AUTO_REFER_RETRY_MAX_CAPTURE;
	bool found = false;

	if (ChainSetup::isSpeedFaster()) {
		countMax *= 2;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax *= 1;
	}
	else {
		countMax *= 2;
	}

	for (int i = 0; i < countMax; i++) {
		if (!renewQualityIndex()) {
			return false;
		}

		if (isTargetFound()) {
			found = true;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(50));
	}
	return found;
}


bool RetFocus::ScanAutoMeasure::moveToStartCorneaCapture(bool first, bool check)
{
	auto compenMotor = getMainBoard()->getOctDiopterMotor();
	auto referMotor = getMainBoard()->getReferenceMotor();
	auto polarMotor = getMainBoard()->getPolarizationMotor();
	auto dioptMotor = getMainBoard()->getOctFocusMotor();
	bool result = true;

	if (isCompensationLensMode()) {
		if (!compenMotor->isPlusLensMode()) {
			compenMotor->updatePositionToPlusLens();
		}

		if (isTargetCorneaFound()) {
			referMotor->updatePosition(d_ptr->referCornea);
			dioptMotor->updateDiopter(d_ptr->dioptCornea);
			result = polarMotor->updateDegree(d_ptr->polarCornea);
		}
		else {
			dioptMotor->updateDiopter(0.0f);
			if (check && checkIfTargetExist()) {
				result = true;
			}
			else {
				if (first) {
					result = referMotor->updatePositionToUpperEnd();
				}
			}
		}
	}
	else {
		if (!compenMotor->isZeroLensMode()) {
			compenMotor->updatePositionToZeroLens();
		}

		if (isTargetCorneaFound()) {
			referMotor->updatePosition(d_ptr->referCornea);
			dioptMotor->updateDiopter(d_ptr->dioptCornea);
			result = polarMotor->updateDegree(d_ptr->polarCornea);
		}
		else {
			dioptMotor->updateDiopter(0.0f);
			if (check && checkIfTargetExist()) {
				result = true;
			}
			else {
				if (first) {
					result = referMotor->updatePositionToCorneaOrigin();
				}
			}
		}
	}

	auto currPos = referMotor->getPosition();
	auto currDpt = dioptMotor->getCurrentDiopter();
	LogD() << "Auto Measure: move to start cornea focus, refer: " << currPos << ", diopt: " << currDpt << ", compenLens: " << d_ptr->compenLens;
	return result;
}


bool RetFocus::ScanAutoMeasure::moveToStartRetinaCapture(bool first, bool check)
{
	auto compenMotor = getMainBoard()->getOctDiopterMotor();
	auto referMotor = getMainBoard()->getReferenceMotor();
	auto polarMotor = getMainBoard()->getPolarizationMotor();
	auto dioptMotor = getMainBoard()->getOctFocusMotor();
	bool result = true;

	if (!compenMotor->isZeroLensMode()) {
		compenMotor->updatePositionToZeroLens();
	}

	if (isTargetRetinaFound()) {
		referMotor->updatePosition(d_ptr->referRetina);
		dioptMotor->updateDiopter(d_ptr->dioptRetina);
		result = polarMotor->updateDegree(d_ptr->polarRetina);
	}
	else {
		dioptMotor->updateDiopter(0.0f);
		if (check && checkIfTargetExist()) {
			result = true;
		}
		else {
			if (first) {
				result = referMotor->updatePositionToRetinaOrigin();
			}
		}
	}
	
	auto currPos = referMotor->getPosition();
	auto currDpt = dioptMotor->getCurrentDiopter();
	LogD() << "Auto Measure: move to start retina focus, refer: " << currPos << ", diopt: " << currDpt << ", compenLens: " << d_ptr->compenLens;
	return result;
}


bool RetFocus::ScanAutoMeasure::moveToStartLensFrontCapture(bool first, bool meye)
{
	auto compenMotor = getMainBoard()->getOctDiopterMotor();
	auto referMotor = getMainBoard()->getReferenceMotor();
	auto polarMotor = getMainBoard()->getPolarizationMotor();
	auto dioptMotor = getMainBoard()->getOctFocusMotor();
	bool result = false;

	if (isTargetLensFrontFound()) {
		referMotor->updatePosition(d_ptr->referLensFront);
		dioptMotor->updateDiopter(d_ptr->dioptLensFront);
		result = polarMotor->updateDegree(d_ptr->polarLensFront);
	}
	else {
		int position = 0;
		float diopter = 0.0f;

		if (meye) {
			result = referMotor->updatePositionToLowerEnd();
		}
		else {
			if (isTargetCorneaFound()) {
				position = getCorneaPosition() + AUTO_LENS_FRONT_REFER_ADD;
				diopter = getCorneaDiopter() + AUTO_LENS_FRONT_DIOPT_ADD;
			}
			else {
				position = referMotor->getPositionOfCorneaOrigin() + AUTO_LENS_FRONT_REFER_ADD;
				diopter = AUTO_LENS_FRONT_DIOPT_MAX;
			}
			referMotor->updatePosition(position);
			result = dioptMotor->updateDiopter(diopter);
		}
	}

	auto currPos = referMotor->getPosition();
	auto currDpt = dioptMotor->getCurrentDiopter();
	LogD() << "Auto Measure: move to start lens front focus, refer: " << currPos << ", diopt: " << currDpt ;
	return result;
}


bool RetFocus::ScanAutoMeasure::moveToStartLensBackCapture(bool first)
{
	auto compenMotor = getMainBoard()->getOctDiopterMotor();
	auto referMotor = getMainBoard()->getReferenceMotor();
	auto polarMotor = getMainBoard()->getPolarizationMotor();
	auto dioptMotor = getMainBoard()->getOctFocusMotor();
	bool result = false;

	if (isTargetLensBackFound()) {
		referMotor->updatePosition(d_ptr->referLensBack);
		dioptMotor->updateDiopter(d_ptr->dioptLensBack);
		result = polarMotor->updateDegree(d_ptr->polarLensBack);
	}
	else {
		int position = 0;
		float diopter = 0.0f;

		if (isTargetLensFrontFound()) {
			position = getLensFrontPosition() + AUTO_LENS_BACK_REFER_ADD;
			diopter = getLensFrontDiopter() + AUTO_LENS_BACK_DIOPT_ADD;
		}
		else {
			position = referMotor->getPositionOfCorneaOrigin() + AUTO_LENS_FRONT_REFER_ADD + AUTO_LENS_FRONT_REFER_ADD;
			diopter = AUTO_LENS_BACK_DIOPT_MAX;
		}

		referMotor->updatePosition(position);
		result = dioptMotor->updateDiopter(diopter);
	}

	auto currPos = referMotor->getPosition();
	auto currDpt = dioptMotor->getCurrentDiopter();
	LogD() << "Auto Measure: move to start lens back focus, refer: " << currPos << ", diopt: " << currDpt;
	return result;
}


bool RetFocus::ScanAutoMeasure::returnToRetinaPosition(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	auto nextPos = d_ptr->referRetina;

	LogD() << "Auto Measure: return to retina position: " << nextPos;
	return motor->controlMove(nextPos);
}


bool RetFocus::ScanAutoMeasure::isTargetRetinaFound(void)
{
	return (d_ptr->referRetina >= 0);
}


bool RetFocus::ScanAutoMeasure::isTargetCorneaFound(void)
{
	return (d_ptr->referCornea >= 0);
}


bool RetFocus::ScanAutoMeasure::isTargetLensFrontFound(void)
{
	return (d_ptr->referLensFront >= 0);
}


bool RetFocus::ScanAutoMeasure::isTargetLensBackFound(void)
{
	return (d_ptr->referLensBack >= 0);
}


int RetFocus::ScanAutoMeasure::getRetinaPosition(void)
{
	return d_ptr->referRetina;
}


int RetFocus::ScanAutoMeasure::getCorneaPosition(void)
{
	return d_ptr->referCornea;
}

int RetFocus::ScanAutoMeasure::getLensFrontPosition(void)
{
	return d_ptr->referLensFront;
}

int RetFocus::ScanAutoMeasure::getLensBackPosition(void)
{
	return d_ptr->referLensBack;
}

void RetFocus::ScanAutoMeasure::setCorneaPosition(int pos)
{
	d_ptr->referCornea = pos;
	return;
}

void RetFocus::ScanAutoMeasure::setLensFrontPosition(int pos)
{
	d_ptr->referLensFront = pos;
	return;
}

void RetFocus::ScanAutoMeasure::setLensBackPosition(int pos)
{
	d_ptr->referLensBack = pos;
	return;
}

float RetFocus::ScanAutoMeasure::getRetinaDiopter(void)
{
	return d_ptr->dioptRetina;
}

float RetFocus::ScanAutoMeasure::getCorneaDiopter(void)
{
	return d_ptr->dioptCornea;
}

float RetFocus::ScanAutoMeasure::getLensFrontDiopter(void)
{
	return d_ptr->dioptLensFront;
}

float RetFocus::ScanAutoMeasure::getLensBackDiopter(void)
{
	return d_ptr->dioptLensBack;
}

void RetFocus::ScanAutoMeasure::setCorneaDiopter(float diopt)
{
	d_ptr->dioptCornea = diopt;
	return;
}

void RetFocus::ScanAutoMeasure::setLensFrontDiopter(float diopt)
{
	d_ptr->dioptLensFront = diopt;
	return;
}

void RetFocus::ScanAutoMeasure::setLensBackDiopter(float diopt)
{
	d_ptr->dioptLensBack = diopt;
	return;
}


float RetFocus::ScanAutoMeasure::getAxialLength(void)
{
	auto posRetina = d_ptr->referRetina;
	auto posCornea = d_ptr->referCornea;

	auto length = (abs(posRetina - posCornea) * 1.25f) / (1000.0f * 1.35f);
	LogD() << "Auto Measure: retina pos: " << posRetina << ", cornea pos: " << posCornea << ", length: " << length;

	if (posRetina < 0 || posCornea < 0) {
		length = 0.0f;
	}
	d_ptr->axialLength = length;
	return length;
}


bool RetFocus::ScanAutoMeasure::isTargetFound(void)
{
	return isQualityToSignal(d_ptr->quality);
}


bool RetFocus::ScanAutoMeasure::isTargetQuality(bool isLensBack)
{
	return isQualityToTarget(d_ptr->quality, isLensBack);
}


bool RetFocus::ScanAutoMeasure::isTargetConfirmed(void)
{
	return isQualityToConfirm(d_ptr->quality);;
}


bool RetFocus::ScanAutoMeasure::renewQualityIndex(bool next)
{
	float qidx, ratio;
	if (!obtainQualityIndexFromPreview(qidx, ratio, next)) {
		return false;
	}
	getImpl().quality = qidx;
	return true;
}


ScanAutoMeasure::ScanAutoMeasureImpl & RetFocus::ScanAutoMeasure::getImpl(void)
{
	return *d_ptr;
}


OctDevice::MainBoard * RetFocus::ScanAutoMeasure::getMainBoard(void)
{
	return d_ptr->board;
}
