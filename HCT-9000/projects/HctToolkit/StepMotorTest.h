#pragma once


#include <functional>
#include <string>

#include "OctSystem2.h"

using namespace OctSystem;

typedef std::function<void(int, int, int, bool)> StepMotorMovingCallback;
typedef std::function<void(bool)> StepMotorCompleteCallback;

class StepMotorTest
{
public:
	StepMotorTest();
	virtual ~StepMotorTest();

public:
	static bool initialize(OctDevice::MainBoard * board);
	static bool isInitialized(void);

	static bool start(int motorNo, int repeats, StepMotorMovingCallback* clbMoving, StepMotorCompleteCallback* clbComplete);
	static void cancel(bool wait);
	static bool isRunning(void);
	static bool isCancelling(void);

protected:
	static void resetStatus(void);
	static void threadFunction(void);
	static bool checkIfCountOver(void);
	static bool phaseStepMotorMove(void);
	static bool phaseStepAutoMotorMove(void);
	static int getMotorRangeMin(StepMotorType type);
	static int getMotorRangeMax(StepMotorType type);

	static void callbackStepMotorMoving(bool isError);
	static void callbackStepMotorComplete(bool success);

private:
	struct StepMotorTestImpl;
	static std::unique_ptr<StepMotorTestImpl> d_ptr;
	static StepMotorTestImpl& getImpl(void);

	static OctDevice::MainBoard* getMainboard(void);
};

