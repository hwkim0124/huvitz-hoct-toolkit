#include "stdafx.h"
#include "StepMotorTest.h"

#include <mutex>
#include <atomic>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>

#include "OctDevice2.h"
#include "OctConfig2.h"
#include "CppUtil2.h"

using namespace OctDevice;
using namespace CppUtil;
using namespace OctConfig;
using namespace std;


struct StepMotorTest::StepMotorTestImpl
{
	MainBoard* board;
	bool initiated;

	int motorNo;
	int repeats;
	int present;
	int stage;
	int count;
	int target;

	atomic<bool> running;
	atomic<bool> stopping;
	thread threadWork;

	StepMotorMovingCallback* clbMoving;
	StepMotorCompleteCallback* clbComplete;

	StepMotorTestImpl() : initiated(false), board(nullptr), clbMoving(nullptr), clbComplete(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<StepMotorTest::StepMotorTestImpl> StepMotorTest::d_ptr(new StepMotorTestImpl());


StepMotorTest::StepMotorTest()
{
}


StepMotorTest::~StepMotorTest()
{
}


bool StepMotorTest::initialize(OctDevice::MainBoard * board)
{
	d_ptr->board = board;
	d_ptr->initiated = true;
	resetStatus();
	return true;
}


bool StepMotorTest::isInitialized(void)
{
	return d_ptr->initiated;
}


bool StepMotorTest::start(int motorNo, int repeats, StepMotorMovingCallback* clbMoving, StepMotorCompleteCallback* clbComplete)
{
	if (!isInitialized()) {
		return false;
	}

	cancel(true);
	resetStatus();

	d_ptr->motorNo = motorNo;
	d_ptr->repeats = repeats;
	d_ptr->present = 1;
	d_ptr->stage = 0;
	d_ptr->count = 0;
	d_ptr->target = 0;

	d_ptr->clbMoving = clbMoving;
	d_ptr->clbComplete = clbComplete;

	// Thread object should be joined before being reused. 
	if (getImpl().threadWork.joinable()) {
		getImpl().threadWork.join();
	}
	getImpl().threadWork = thread{ &StepMotorTest::threadFunction };
	return true;
}


void StepMotorTest::cancel(bool wait)
{
	if (isRunning() && !isCancelling()) {
		d_ptr->stopping = true;

		if (wait) {
			// Thread object should be joined before being reused. 
			if (getImpl().threadWork.joinable()) {
				getImpl().threadWork.join();
			}
		}
	}
	return;
}


bool StepMotorTest::isRunning(void)
{
	return d_ptr->running;
}


bool StepMotorTest::isCancelling(void)
{
	return d_ptr->stopping;
}


void StepMotorTest::resetStatus(void)
{
	d_ptr->running = false;
	d_ptr->stopping = false;
	return;
}


void StepMotorTest::threadFunction(void)
{
	d_ptr->running = true;

	while (true)
	{
		if (checkIfCountOver()) {
			callbackStepMotorComplete(true);
			break;
		}

		if (d_ptr->motorNo == static_cast<int>(StageMotorType::STAGE_Y)) {
			if (!phaseStepAutoMotorMove()) {
				break;
			}
		}
		else {
			if (!phaseStepMotorMove()) {
				break;
			}
		}

		if (isCancelling()) {
			callbackStepMotorComplete(false);
			break;
		}
	}

	d_ptr->running = false;
	return;
}


bool StepMotorTest::checkIfCountOver(void)
{
	if (d_ptr->present > d_ptr->repeats) {
		return true;
	}
	return false;
}


bool StepMotorTest::phaseStepMotorMove(void)
{
	StepMotorType type = static_cast<StepMotorType>(d_ptr->motorNo);
	StepMotor* motor = getMainboard()->getStepMotor(type);
	if (!motor) {
		return false;
	}

	switch (d_ptr->stage) {
	case 0:
		// d_ptr->target = motor->getRangeMax();
		d_ptr->target = getMotorRangeMax(type);
		callbackStepMotorMoving(false);
		if (!motor->controlMove(d_ptr->target)) {
			callbackStepMotorComplete(false);
			return false;
		}
		callbackStepMotorMoving(true);
		this_thread::sleep_for(std::chrono::milliseconds(500));
		d_ptr->stage = 1;
		if (d_ptr->count > 0) {
			d_ptr->present++;
		}
		break;

	case 1:
		// d_ptr->target = motor->getRangeMin();
		d_ptr->target = getMotorRangeMin(type);
		callbackStepMotorMoving(false);
		if (!motor->controlMove(d_ptr->target)) {
			callbackStepMotorComplete(false);
			return false;
		}
		callbackStepMotorMoving(true);
		this_thread::sleep_for(std::chrono::milliseconds(500));
		d_ptr->stage = 0;
		d_ptr->count++;
		break;
	}
	return true;
}


bool StepMotorTest::phaseStepAutoMotorMove(void)
{
	StageMotor* motor = getMainboard()->getStageYMotor();

	switch (d_ptr->stage) {
	case 0:
		// d_ptr->target = motor->getRangeMax();
		d_ptr->target = 48000;
		callbackStepMotorMoving(false);
		if (!motor->controlMove(d_ptr->target)) {
			callbackStepMotorComplete(false);
			return false;
		}
		callbackStepMotorMoving(true);
		this_thread::sleep_for(std::chrono::milliseconds(6000));
		d_ptr->stage = 1;
		if (d_ptr->count > 0) {
			d_ptr->present++;
		}
		break;

	case 1:
		// d_ptr->target = motor->getRangeMin();
		d_ptr->target = -10;
		callbackStepMotorMoving(false);
		if (!motor->controlMove(d_ptr->target)) {
			callbackStepMotorComplete(false);
			return false;
		}
		callbackStepMotorMoving(true);
		this_thread::sleep_for(std::chrono::milliseconds(6000));
		d_ptr->stage = 0;
		d_ptr->count++;
		break;
	}
	return true;
}


int StepMotorTest::getMotorRangeMin(StepMotorType type)
{
	int value = 0;

	switch (type) {
	case StepMotorType::OCT_SAMPLE:
		value = SystemConfig::quickReturnMirrorPos(true);
		break;
	case StepMotorType::SPLIT_FOCUS:
		value = SystemConfig::splitFocusMirrorPos(true);
		break;
	case StepMotorType::FUNDUS_DIOPTER:
		value = getMainboard()->getStepMotor(type)->getRangeMin();
		break;
	case StepMotorType::PUPIL_MASK:
		// value = getMainboard()->getStepMotor(type)->getRangeMin();
		value = 0;
		break;
	case StepMotorType::OCT_DIOPTER:
		value = -10;
		break;
	default:
		value = getMainboard()->getStepMotor(type)->getRangeMin();
		break;
	}
	return value;
}


int StepMotorTest::getMotorRangeMax(StepMotorType type)
{
	int value = 0;

	switch (type) {
	case StepMotorType::OCT_SAMPLE:
		value = SystemConfig::quickReturnMirrorPos(false);
		break;
	case StepMotorType::SPLIT_FOCUS:
		value = SystemConfig::splitFocusMirrorPos(false);
		break;
	case StepMotorType::FUNDUS_DIOPTER:
		value = SystemConfig::minusLensPosToFundus();
		break;
	case StepMotorType::PUPIL_MASK:
		value = SystemConfig::pupilMaskPos();
		break;
	case StepMotorType::OCT_DIOPTER:
		value = ((OctDiopterMotor*)getMainboard()->getStepMotor(type))->getPositionOfPlusLens();
		break;
	default:
		value = getMainboard()->getStepMotor(type)->getRangeMax();
		break;
	}
	return value;
}


void StepMotorTest::callbackStepMotorMoving(bool isError)
{
	if (d_ptr->clbMoving) {
		(*d_ptr->clbMoving)(d_ptr->stage, d_ptr->present, d_ptr->target, isError);
	}
	return;
}


void StepMotorTest::callbackStepMotorComplete(bool success)
{
	if (d_ptr->clbComplete) {
		(*d_ptr->clbComplete)(success);
	}
	return;
}


StepMotorTest::StepMotorTestImpl & StepMotorTest::getImpl(void)
{
	return *d_ptr;
}


OctDevice::MainBoard * StepMotorTest::getMainboard(void)
{
	return d_ptr->board;
}
