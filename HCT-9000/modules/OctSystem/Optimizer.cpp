#include "stdafx.h"
#include "Optimizer.h"
#include "Scanner.h"
#include "RetinaAlign.h"

#include "OctDevice2.h"
#include "SigChain2.h"
#include "CppUtil2.h"
#include "RetFocus2.h"

#include <mutex>
#include <atomic>

using namespace OctSystem;
using namespace OctDevice;
using namespace RetFocus;
using namespace SigChain;
using namespace CppUtil;
using namespace std;


struct Optimizer::OptimizerImpl
{
	bool initiated;
	bool noFundus;
	bool measureRetina;
	bool measureCornea;
	bool dioptFocus;
	bool topoMode;

	int measureCount;
	bool keepOnError;
	bool referFixed;

	MainBoard* mainboard;
	Scanner* scanner;
	RetinaAlign* retAlign;

	float afQuality;
	float afDiopter;
	float afDioptStep;
	int afDirection;

	int afRetryCnt;
	int afForwardCnt;
	int afRetraceCnt;

	float apQuality;
	float apDegree;
	float apMoveStep;
	int apDirection;

	int apRetryCnt;
	int apForwardCnt;
	int apRetraceCnt;

	float arQuality;
	int arRefPoint;
	int arPosition;
	int arMoveStep;
	int arDirection;

	int arRetryCnt;
	int arForwardCnt;
	int arRetraceCnt;
	int arSuccessCnt;


	AutoFocusCompleteCallback* cbAutoFocus;
	AutoPolarizationCompleteCallback* cbAutoPolar;
	AutoReferenceCompleteCallback* cbAutoRefer;

	AutoFocusPhase afPhase;
	AutoReferPhase arPhase;
	AutoPolarPhase apPhase;

	atomic<bool> stopping;
	atomic<bool> running;
	atomic<bool> completed;
	condition_variable condVarWork;
	thread threadWork;
	mutex mutexWork;

	OptimizerImpl() : initiated(false), mainboard(nullptr), scanner(nullptr), retAlign(nullptr),
		running(false), stopping(false), completed(false), measureRetina(false), measureCornea(false),
		cbAutoFocus(nullptr), cbAutoPolar(nullptr), cbAutoRefer(nullptr), 
		referFixed(false), dioptFocus(false), topoMode(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Optimizer::OptimizerImpl> Optimizer::d_ptr(new OptimizerImpl());


Optimizer::Optimizer()
{
}


Optimizer::~Optimizer()
{
}


bool OctSystem::Optimizer::initOptimizer(OctDevice::MainBoard * board, Scanner* scanner, RetinaAlign* retAlign)
{
	getImpl().mainboard = board;
	getImpl().scanner = scanner;
	getImpl().retAlign = retAlign;
	getImpl().initiated = true;
	return true;
}


bool OctSystem::Optimizer::initOptimizer(OctDevice::MainBoard * board, RetinaAlign* retAlign)
{
	getImpl().mainboard = board;
	getImpl().retAlign = retAlign;
	getImpl().initiated = true;
	return true;
}


bool OctSystem::Optimizer::isInitiated(void)
{
	return getImpl().initiated;
}


void OctSystem::Optimizer::clearAutoMeasureResult(void)
{
	ScanAutoMeasure::clearResult();
	return;
}


bool OctSystem::Optimizer::startAutoOptimize(bool noFundus, bool dioptFocus, bool topogrphyMode)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	d_ptr->noFundus = noFundus;
	d_ptr->dioptFocus = dioptFocus;
	d_ptr->topoMode = topogrphyMode;
	d_ptr->completed = false;
	getImpl().threadWork = thread{ &Optimizer::threadAutoOptimizeFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoPosition(void)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	d_ptr->completed = false;
	getImpl().threadWork = thread{ &Optimizer::threadAutoPositionFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoMeasure(bool macular, bool cornea)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initiateAxialLength(getMainBoard())) {
		return false;
	}

	d_ptr->noFundus = true;
	d_ptr->completed = false;
	d_ptr->measureRetina = macular;
	d_ptr->measureCornea = cornea;
	getImpl().threadWork = thread{ &Optimizer::threadAutoMeasureFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoLensBackFocus(int frontReferPos, float frontDioptPos)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::LENS_BACK, false)) {
		return false;

	}

	ScanAutoMeasure::setLensFrontPosition(frontReferPos);
	ScanAutoMeasure::setLensFrontDiopter(frontDioptPos);

	d_ptr->noFundus = true;
	d_ptr->completed = false;
	getImpl().threadWork = thread{ &Optimizer::threadAutoLensBackFocusFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoLensThickness(int measureCount, bool keepOnError)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::LENS_THICKNESS, false)) {
		return false;
	}
	d_ptr->noFundus = true;
	d_ptr->completed = false;
	d_ptr->measureCount = measureCount;
	d_ptr->keepOnError = keepOnError;

	getImpl().threadWork = thread{ &Optimizer::threadAutoLensThicknessFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoLensThickness2(int measureCount, bool keepOnError, bool referFixed)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::LENS_THICKNESS, false)) {
		return false;
	}
	d_ptr->noFundus = true;
	d_ptr->completed = false;
	d_ptr->measureCount = measureCount;
	d_ptr->keepOnError = keepOnError;
	d_ptr->referFixed = referFixed;

	getImpl().threadWork = thread{ &Optimizer::threadAutoLensThicknessFunction2 };
	return true;
}


bool OctSystem::Optimizer::startAutoCorneaFocus(bool compenLens)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::CORNEA, compenLens)) {
		return false;
	}

	d_ptr->noFundus = true;
	d_ptr->completed = false;

	getImpl().threadWork = thread{ &Optimizer::threadAutoCorneaFocusFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoRetinaFocus(bool compenLens)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::RETINA, compenLens)) {
		return false;
	}

	d_ptr->noFundus = true;
	d_ptr->completed = false;

	getImpl().threadWork = thread{ &Optimizer::threadAutoRetinaFocusFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoAxialLength(int measureCount, bool keepOnError)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::AXIAL_LENGTH, true)) {
		return false;

	}
	d_ptr->noFundus = true;
	d_ptr->completed = false;
	d_ptr->measureCount = measureCount;
	d_ptr->keepOnError = keepOnError;

	getImpl().threadWork = thread{ &Optimizer::threadAutoAxialLengthFunction };
	return true;
}


bool OctSystem::Optimizer::startAutoLensFrontFocus(int corneaReferPos, float corneaDioptPos)
{
	if (!isInitiated()) {
		return false;
	}
	cancel();

	if (!ScanAutoMeasure::initialize(getMainBoard(), ScanAutoMeasure::LENS_FRONT, false)) {
		return false;
	}

	ScanAutoMeasure::setCorneaPosition(corneaReferPos);
	ScanAutoMeasure::setCorneaDiopter(corneaDioptPos);

	d_ptr->noFundus = true;
	d_ptr->completed = false;
	getImpl().threadWork = thread{ &Optimizer::threadAutoLensFrontFocusFunction };
	return true;
}



bool OctSystem::Optimizer::startAutoDiopterFocus(AutoFocusCompleteCallback* callback)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}

	if (!ScanAutoFocus::initialize(getMainBoard(), callback)) {
		return false;
	}

	if (!ScanAutoFocus::startOptimizing()) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::startAutoPolarization(AutoPolarizationCompleteCallback* callback)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}

	if (!ScanAutoPolar::initialize(getMainBoard(), callback)) {
		return false;
	}

	if (!ScanAutoPolar::startOptimizing()) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::startAutoReference(AutoReferenceCompleteCallback* callback, bool captureCornea, 
												bool narrowTarget, bool smallMoveStep, bool autoCenter, bool measureMode, bool upperTarget)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}

	if (!ScanAutoRefer::initialize(getMainBoard(), callback)) {
		return false;
	}

	bool isAnterior = getScanner()->isAnteriorMode();
	isAnterior = (captureCornea ? true : isAnterior);
	bool motorInRange = (captureCornea ? false : true);
	if (!ScanAutoRefer::startOptimizing(isAnterior, autoCenter, motorInRange, narrowTarget, smallMoveStep, measureMode, upperTarget)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::startAutoSplitFocus(void)
{
	if (!isInitiated()) {
		return false;
	}

	if (!SplitAutoFocus::initialize(getMainBoard())) {
		return false;
	}

	if (!SplitAutoFocus::start()) {
		return false;
	}

	return true;
}


void OctSystem::Optimizer::cancelAutoReference(bool wait)
{
	ScanAutoRefer::cancelOptimizing(wait);
	return;
}


void OctSystem::Optimizer::cancelAutoDiopterFocus(bool wait)
{
	ScanAutoFocus::cancelOptimizing(wait);
	return;
}


void OctSystem::Optimizer::cancelAutoPolarization(bool wait)
{
	ScanAutoPolar::cancelOptimizing(wait);
	return;
}


void OctSystem::Optimizer::cancelAutoSplitFocus(bool wait)
{
	SplitAutoFocus::cancel(wait);
	return;
}


void OctSystem::Optimizer::cancelAutoMeasure(bool wait)
{
}


bool OctSystem::Optimizer::isAutoDiopterFocusing(void)
{
	return ScanAutoFocus::isRunning();
}


bool OctSystem::Optimizer::isAutoPolarizing(void)
{
	return ScanAutoPolar::isRunning();
}


bool OctSystem::Optimizer::isAutoReferencing(void)
{
	return ScanAutoRefer::isRunning();
}


bool OctSystem::Optimizer::isAutoSplitFocusing(void)
{
	return SplitAutoFocus::isRunning();
}


bool OctSystem::Optimizer::isOptimizing(void)
{
	return (getImpl().running || ScanAutoFocus::isRunning() || ScanAutoPolar::isRunning() || ScanAutoRefer::isRunning());
}


bool OctSystem::Optimizer::isCancelling(void)
{
	return (getImpl().stopping || ScanAutoFocus::isCancelling() || ScanAutoPolar::isCancelling() || ScanAutoRefer::isCancelling());
}


bool OctSystem::Optimizer::isCompleted(void)
{
	return (getImpl().completed || ScanAutoFocus::isCompleted() || ScanAutoPolar::isCompleted() || ScanAutoRefer::isCompleted());
}


void OctSystem::Optimizer::cancel(void)
{
	/*
	if (!isOptimizing()) {
		return;
	}
	*/

	getImpl().stopping = true;
	d_ptr->measureRetina = false;
	d_ptr->measureCornea = false;
	/*
	// Caution:
	// If this routine owning the lock falls into waiting status by calling join, 
	// the worker thread can't wake up until the lock released. 
	// However, the lock should be held to be the waiting status of the worker thread 
	// before notification, RAII fashion within a bracket scope could resolve this problem.
	{
		unique_lock<mutex> lock(getImpl().mutexWork);
		getImpl().condVarWork.notify_all();
	}
	*/

	// Thread object should be joined before being reused. 
	if (getImpl().threadWork.joinable()) {
		getImpl().threadWork.join();
	}

	getImpl().stopping = false;
	return;
}


bool OctSystem::Optimizer::isMeasuring(void)
{
	return (isMeasuringRetina() || isMeasuringCornea());
}


bool OctSystem::Optimizer::isMeasuringRetina(void)
{
	return d_ptr->measureRetina;
}


bool OctSystem::Optimizer::isMeasuringCornea(void)
{
	return d_ptr->measureCornea;
}


Optimizer::OptimizerImpl & OctSystem::Optimizer::getImpl(void)
{
	return *d_ptr;
}


OctSystem::Scanner * OctSystem::Optimizer::getScanner(void)
{
	return getImpl().scanner;
}


OctDevice::MainBoard * OctSystem::Optimizer::getMainBoard(void)
{
	return getImpl().mainboard;
}


void OctSystem::Optimizer::threadAutoOptimizeFunction(void)
{
	bool result = false;
	d_ptr->running = true;

	bool is_grabbing = getScanner()->isGrabbing();
	bool is_anterior = getScanner()->isAnteriorMode();
	bool not_fundus = getImpl().noFundus;
	bool diopt_focus = getImpl().dioptFocus;  // full range anterior mode.

	bool isAnimalEye = false;

	if (isAnimalEye) {
		if (startAutoDiopterFocus())
		{
			LogD() << "Auto diopter focus started";
			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoFocus::isRunning()) {
					result = ScanAutoFocus::isCompleted();
					LogD() << "Auto diopter focus completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoFocus::cancelOptimizing(true);
					LogD() << "Auto diopter focus canceled";
					goto exit_optim;
				}
			}
		}
		goto exit_optim;
	}

	if ((!is_anterior && !not_fundus && !diopt_focus) ||
		(!is_grabbing && !not_fundus)) {
		if (startAutoSplitFocus())
		{
			LogD() << "Auto split focus started";

			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!SplitAutoFocus::isRunning()) {
					result = SplitAutoFocus::isCompleted();
					LogD() << "Auto split focus completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					SplitAutoFocus::cancel(true);
					LogD() << "Auto split focus canceled";
					goto exit_optim;
				}
			}
		}
	}

	if (!is_anterior && !diopt_focus) {
		if (startAutoReference())
		{
			LogD() << "Auto reference started";

			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoRefer::isRunning()) {
					result = ScanAutoRefer::isCompleted();
					LogD() << "Auto reference completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoRefer::cancelOptimizing(true);
					LogD() << "Auto reference canceled";
					goto exit_optim;
				}
			}
		}
	}

	// if (!d_ptr->topoMode) {
		if (startAutoDiopterFocus())
		{
			LogD() << "Auto diopter focus started";
			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoFocus::isRunning()) {
					result = ScanAutoFocus::isCompleted();
					LogD() << "Auto diopter focus completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoFocus::cancelOptimizing(true);
					LogD() << "Auto diopter focus canceled";
					goto exit_optim;
				}
			}
		}
	// }

	// if (!d_ptr->dioptFocus) {
		if (startAutoPolarization()) 
		{
			LogD() << "Auto polarization started";
			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoPolar::isRunning()) {
					result = ScanAutoPolar::isCompleted();
					LogD() << "Auto polarization completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoPolar::cancelOptimizing(true);
					LogD() << "Auto polarization canceled";
					goto exit_optim;
				}
			}
		}
	// }
	
exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;

	LogD() << "Auto optimize completed, result: " << result ;
	GlobalRegister::runAutoOptimizeCompletedCallback(result);
	return;
}


void OctSystem::Optimizer::threadAutoPositionFunction(void)
{
	/*
	threadAutoReferFunction();
	
	bool result = isCompleted();
	GlobalRegister::runAutoPositionCompletedCallback(result);
	*/

	bool result = false;
	d_ptr->running = true;

	if (startAutoReference())
	{
		while (true)
		{
			this_thread::sleep_for(chrono::milliseconds(50));

			if (!ScanAutoRefer::isRunning()) {
				result = isCompleted();
				break;
			}

			if (isCancelling()) {
				ScanAutoRefer::cancelOptimizing(true);
				break;
			}
		}
	}

	d_ptr->completed = result;
	d_ptr->running = false;

	GlobalRegister::runAutoPositionCompletedCallback(result);
	return;
}


void OctSystem::Optimizer::threadAutoMeasureFunction(void)
{
	bool result = false;
	d_ptr->running = true;

	if (isMeasuringRetina()) {
		if (startAutoReference(nullptr, false, true, true))
		{
			LogD() << "Auto reference started";

			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoRefer::isRunning()) {
					result = ScanAutoRefer::isCompleted();
					LogD() << "Auto reference completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoRefer::cancelOptimizing(true);
					LogD() << "Auto reference canceled";
					goto exit_optim;
				}
			}
		}

		if (startAutoDiopterFocus())
		{
			LogD() << "Auto diopter focus started";
			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoFocus::isRunning()) {
					result = ScanAutoFocus::isCompleted();
					LogD() << "Auto diopter focus completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoFocus::cancelOptimizing(true);
					LogD() << "Auto diopter focus canceled";
					goto exit_optim;
				}
			}
		}

		if (startAutoPolarization())
		{
			LogD() << "Auto polarization started";
			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoPolar::isRunning()) {
					result = ScanAutoPolar::isCompleted();
					LogD() << "Auto polarization completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoPolar::cancelOptimizing(true);
					LogD() << "Auto polarization canceled";
					goto exit_optim;
				}
			}
		}

		result = ScanAutoMeasure::recordPositionOfRetina();
		if (result && isMeasuringCornea()) {
		}
		else {
			goto exit_optim;
		}
	}

	if (isMeasuringCornea())
	{
		result = false;
		if (!ScanAutoMeasure::moveToStartCorneaCapture()) {
			result = false;
			goto exit_optim;
		}

		if (startAutoReference(nullptr, true, true, true))
		{
			LogD() << "Auto reference started to capture cornea";

			while (true)
			{
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoRefer::isRunning()) {
					result = ScanAutoRefer::isCompleted();
					LogD() << "Auto reference completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoRefer::cancelOptimizing(true);
					LogD() << "Auto reference canceled";
					goto exit_optim;
				}
			}
		}

		result = ScanAutoMeasure::recordPositionOfCornea();
		if (!result) {
			if (!ScanAutoMeasure::moveToStartCorneaCapture()) {
				result = false;
				goto exit_optim;
			}
		}
	}

	if (result) {
		if (isMeasuringRetina() && isMeasuringCornea()) {
			ScanAutoMeasure::returnToRetinaPosition();
		}
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;

	int posMacular = ScanAutoMeasure::getRetinaPosition();
	int posCornea = ScanAutoMeasure::getCorneaPosition();
	float axialLength = ScanAutoMeasure::getAxialLength();

	LogD() << "Auto measure completed, result: " << result << ", macular pos: " << posMacular << ", cornea pos: " << posCornea << ", axialLength: " << axialLength;
	GlobalRegister::runAutoMeasureCompletedCallback(result, 
					posMacular, 
					posCornea, 
					axialLength);
	return;
}


void OctSystem::Optimizer::threadAutoCorneaFocusFunction(void)
{
	bool result = true;
	bool found = false;

	int referCornea = -1;
	float dioptCornea = 0.0f;

	d_ptr->running = true;
	LogD() << "Auto focus of cornea started";

	// Transferred to cornea origin or upper end.
	if (!ScanAutoMeasure::moveToStartCorneaCapture(true, false)) {
		result = false;
		goto exit_cornea;
	}
	else {
		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_cornea;
			}
		}
	}

	if (ScanAutoMeasure::checkIfTargetExist()) {
		found = true;
	}
	else {
		found = false;
	}

	// Allow to search cornea. 
	bool measure = ScanAutoMeasure::isCompensationLensMode();
	if (startAutoReference(nullptr, true, true, true, found, measure)) {
		LogD() << "Auto reference started to capture cornea";
		while (true) {
			this_thread::sleep_for(chrono::milliseconds(50));
			if (!ScanAutoRefer::isRunning()) {
				result = ScanAutoRefer::isCompleted();
				LogD() << "Auto reference completed, result: " << result;
				break;
			}
			if (isCancelling()) {
				ScanAutoRefer::cancelOptimizing(true);
				LogD() << "Auto reference cancelled";
				goto exit_cornea;
			}
		}
	}

	found = ScanAutoMeasure::checkIfTargetExist();
	if (found && !ScanAutoMeasure::isTargetQuality()) {
		if (startAutoDiopterFocus()) {
			LogD() << "Auto diopter focus started";
			while (true) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoFocus::isRunning()) {
					result = ScanAutoFocus::isCompleted();
					LogD() << "Auto diopter focus completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoFocus::cancelOptimizing(true);
					LogD() << "Auto diopter focus cancelled";
					goto exit_cornea;
				}
			}
		}

		if (!ScanAutoMeasure::isTargetCorneaFound()) {
			if (startAutoPolarization()) {
				LogD() << "Auto polarization started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoPolar::isRunning()) {
						result = ScanAutoPolar::isCompleted();
						LogD() << "Auto polarization completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoPolar::cancelOptimizing(true);
						LogD() << "Auto polarization canceled";
						goto exit_cornea;
					}
				}
			}
		}
	}

	for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
		this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
		if (isCancelling()) {
			goto exit_cornea;
		}
	}

exit_cornea:
	result = ((isCancelling()) ? false : true);
	if (result) {
		// Confirm whether target exists at the focused position. If not, refer position set to -1.
		found = ScanAutoMeasure::recordPositionOfCornea(true);
		referCornea = (found ? ScanAutoMeasure::getCorneaPosition() : -1);
		dioptCornea = ScanAutoMeasure::getCorneaDiopter();
		result = ScanAutoMeasure::isTargetCorneaFound();
	}

	LogD() << "Auto cornea focus completed, result: " << result << ", refer: " << referCornea << ", diopt: " << dioptCornea ;
	GlobalRegister::runAutoCorneaFocusCompletedCallback(result, referCornea, dioptCornea);
	if (!result /*&& !d_ptr->keepOnError*/) {
		goto exit_optim;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;
	LogD() << "Auto focus of cornea completed, result: " << result;
	return;
}


void OctSystem::Optimizer::threadAutoRetinaFocusFunction(void)
{
	bool result = true;
	bool found = false;

	int referRetina = -1;
	float dioptRetina = 0.0f;

	d_ptr->running = true;
	LogD() << "Auto focus of retina started";

	if (!ScanAutoMeasure::moveToStartRetinaCapture(true, false)) {
		result = false;
		goto exit_retina;
	}
	else {
		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_retina;
			}
		}
	}

	if (ScanAutoMeasure::checkIfTargetExist()) {
		found = true;
	}
	else {
		found = false;
	}

	bool measure = ScanAutoMeasure::isCompensationLensMode();
	if (startAutoReference(nullptr, false, true, true, found, measure)) {
		LogD() << "Auto reference started to capture retina";
		while (true) {
			this_thread::sleep_for(chrono::milliseconds(50));
			if (!ScanAutoRefer::isRunning()) {
				result = ScanAutoRefer::isCompleted();
				LogD() << "Auto reference completed, result: " << result;
				break;
			}
			if (isCancelling()) {
				ScanAutoRefer::cancelOptimizing(true);
				LogD() << "Auto reference cancelled";
				goto exit_retina;
			}
		}
	}

	found = ScanAutoMeasure::checkIfTargetExist();
	if (found && !ScanAutoMeasure::isTargetQuality()) {
		if (startAutoDiopterFocus()) {
			LogD() << "Auto diopter focus started";
			while (true) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoFocus::isRunning()) {
					result = ScanAutoFocus::isCompleted();
					LogD() << "Auto diopter focus completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoFocus::cancelOptimizing(true);
					LogD() << "Auto diopter focus cancelled";
					goto exit_retina;
				}
			}
		}

		if (!ScanAutoMeasure::isTargetRetinaFound()) {
			if (startAutoPolarization()) {
				LogD() << "Auto polarization started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoPolar::isRunning()) {
						result = ScanAutoPolar::isCompleted();
						LogD() << "Auto polarization completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoPolar::cancelOptimizing(true);
						LogD() << "Auto polarization canceled";
						goto exit_retina;
					}
				}
			}
		}
	}

	for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
		this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
		if (isCancelling()) {
			goto exit_retina;
		}
	}

exit_retina:
	result = ((isCancelling()) ? false : true);
	if (result) {
		found = ScanAutoMeasure::recordPositionOfRetina(true);
		referRetina = (found ? ScanAutoMeasure::getRetinaPosition() : -1);
		dioptRetina = ScanAutoMeasure::getRetinaDiopter();
	}

	LogD() << "Auto retina focus completed, result: " << result << ", refer: " << referRetina << ", diopt: " << dioptRetina;
	GlobalRegister::runAutoRetinaFocusCompletedCallback(result, referRetina, dioptRetina);
	if (!result /* && !d_ptr->keepOnError */) {
		goto exit_optim;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;
	LogD() << "Auto focus of retina completed, result: " << result;
	return;
}


void OctSystem::Optimizer::threadAutoLensFrontFocusFunction(void)
{
	bool result = true;
	bool found = false;

	int referFront = -1;
	float dioptFront = 0.0f;

	d_ptr->running = true;
	LogD() << "Auto focus of lens front started";

	if (!ScanAutoMeasure::moveToStartLensFrontCapture(true)) {
		result = false;
		goto exit_front;
	}
	else {
		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_front;
			}
		}
	}

	if (ScanAutoMeasure::checkIfTargetExist()) {
		found = true;
	}
	else {
		found = false;
	}

	if (true) { // found) {
		if (!ScanAutoMeasure::isTargetQuality()) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_front;
					}
				}
			}

			if (!ScanAutoMeasure::isTargetLensFrontFound()) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization canceled";
							goto exit_front;
						}
					}
				}
			}
		}

		if (ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
			if (startAutoReference(nullptr, true, true, true, found, false)) {
				LogD() << "Auto reference started to capture retina";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoRefer::isRunning()) {
						result = ScanAutoRefer::isCompleted();
						LogD() << "Auto reference completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoRefer::cancelOptimizing(true);
						LogD() << "Auto reference cancelled";
						goto exit_front;
					}
				}
			}
		}
	}

	for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
		this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
		if (isCancelling()) {
			goto exit_front;
		}
	}

exit_front:
	result = (isCancelling() ? false : true);
	if (result) {
		found = ScanAutoMeasure::recordPositionOfLensFront(true);
		referFront = (found ? ScanAutoMeasure::getLensFrontPosition() : -1); 
		dioptFront = ScanAutoMeasure::getLensFrontDiopter();
	}

	LogD() << "Auto lens front focus completed, result: " << result << ", refer: " << referFront << ", diopt: " << dioptFront;
	GlobalRegister::runAutoLensFrontFocusCompletedCallback(result, referFront, dioptFront);
	if (!result /* && !d_ptr->keepOnError*/) {
		goto exit_optim;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;
	LogD() << "Auto focus of lens front completed, result: " << result;
	return;
}


void OctSystem::Optimizer::threadAutoLensBackFocusFunction(void)
{
	bool result = true;
	bool found = false;

	int referBack = -1;
	float dioptBack = 0.0f;

	d_ptr->running = true;
	LogD() << "Auto focus of lens back started";

	if (!ScanAutoMeasure::moveToStartLensBackCapture(true)) {
		result = false;
		goto exit_back;
	}
	else {
		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_back;
			}
		}
	}

	if (ScanAutoMeasure::checkIfTargetExist()) {
		found = true;
	}
	else {
		found = false;
	}

	if (true) { // found) {
		if (!ScanAutoMeasure::isTargetQuality()) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_back;
					}
				}
			}

			if (!ScanAutoMeasure::isTargetLensBackFound()) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization canceled";
							goto exit_back;
						}
					}
				}
			}
		}

		if (ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
			if (startAutoReference(nullptr, true, true, true, found, false, true)) {
				LogD() << "Auto reference started to capture retina";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoRefer::isRunning()) {
						result = ScanAutoRefer::isCompleted();
						LogD() << "Auto reference completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoRefer::cancelOptimizing(true);
						LogD() << "Auto reference cancelled";
						goto exit_back;
					}
				}
			}
		}
	}

	for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
		this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
		if (isCancelling()) {
			goto exit_back;
		}
	}

exit_back:
	result = (isCancelling() ? false : true);
	if (result) {
		found = ScanAutoMeasure::recordPositionOfLensBack(true);
		referBack = (found ? ScanAutoMeasure::getLensBackPosition() : -1);
		dioptBack = ScanAutoMeasure::getLensBackDiopter();
	}

	LogD() << "Auto lens back focus completed, result: " << result << ", refer: " << referBack << ", diopt: " << dioptBack;
	GlobalRegister::runAutoLensBackFocusCompletedCallback(result, referBack, dioptBack);
	if (!result /* && !d_ptr->keepOnError*/) {
		goto exit_optim;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;
	LogD() << "Auto focus of lens back completed, result: " << result;
	return;
}


void OctSystem::Optimizer::threadAutoAxialLengthFunction(void)
{
	bool result = false;
	bool first = true; 
	bool found = false;
	bool error = false;
	bool reset = false;

	int referRetina = -1, referCornea = -1;
	float dioptRetina = 0.0f, dioptCornea = 0.0f;

	d_ptr->running = true;
	LogD() << "Auto measure of axial length started, measureCnt: " << d_ptr->measureCount;

	if (!ScanAutoMeasure::checkIfTargetExist()) {
		reset = true;
	}

	for (int i = 0; i < d_ptr->measureCount; i++) {
		if (i > 0 || reset) {
			if (!ScanAutoMeasure::moveToStartRetinaCapture(true, false)) {
				error = true;
				goto exit_retina;
			}
			else {
				for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
					this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
					if (isCancelling()) {
						goto exit_retina;
					}
				}
			}
			reset = false;
		}

		if (ScanAutoMeasure::isTargetRetinaFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (startAutoReference(nullptr, false, true, true, found, true)) {
			LogD() << "Auto reference started to capture retina";
			while (true) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoRefer::isRunning()) {
					result = ScanAutoRefer::isCompleted();
					LogD() << "Auto reference completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoRefer::cancelOptimizing(true);
					LogD() << "Auto reference cancelled";
					goto exit_retina;
				}
			}
		}

		found = ScanAutoMeasure::checkIfTargetExist();
		if (found && (/*first ||*/ !ScanAutoMeasure::isTargetQuality())) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_retina;
					}
				}
			}

			if (first) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization cancelled";
							goto exit_retina;
						}
					}
				}
			}
		}

		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_retina;
			}
		}

exit_retina:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfRetina(true);
			referRetina = (found ? ScanAutoMeasure::getRetinaPosition() : -1);
			dioptRetina = ScanAutoMeasure::getRetinaDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetRetinaFound();
			}
		}

		LogD() << "Auto retina focus completed, result: " << result << ", refer: " << referRetina << ", diopt: " << dioptRetina << ", measureCnt: " << i;
		GlobalRegister::runAutoRetinaFocusCompletedCallback(result, referRetina, dioptRetina);
		if (!result /* && !d_ptr->keepOnError */) {
			goto exit_optim;
		}

		if (!ScanAutoMeasure::moveToStartCorneaCapture(true)) {
			error = true;
			goto exit_cornea;
		}
		else {
			for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
				this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
				if (isCancelling()) {
					goto exit_cornea;
				}
			}
		}

		if (ScanAutoMeasure::isTargetCorneaFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (startAutoReference(nullptr, true, true, true, found, true)) {
			LogD() << "Auto reference started to capture cornea";
			while (true) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoRefer::isRunning()) {
					result = ScanAutoRefer::isCompleted();
					LogD() << "Auto reference completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoRefer::cancelOptimizing(true);
					LogD() << "Auto reference cancelled";
					goto exit_cornea;
				}
			}
		}

		found = ScanAutoMeasure::checkIfTargetExist();
		if (found && (/*first || */!ScanAutoMeasure::isTargetQuality())) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_cornea;
					}
				}
			}

			if (first) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization canceled";
							goto exit_cornea;
						}
					}
				}
			}
		}
		
		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_cornea;
			}
		}

exit_cornea:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfCornea(true);
			referCornea = (found ? ScanAutoMeasure::getCorneaPosition() : -1);
			dioptCornea = ScanAutoMeasure::getCorneaDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetCorneaFound();
			}
		}

		LogD() << "Auto cornea focus completed, result: " << result << ", refer: " << referCornea << ", diopt: " << dioptCornea << ", measureCnt: " << i;
		GlobalRegister::runAutoCorneaFocusCompletedCallback(result, referCornea, dioptCornea);
		if (!result /*&& !d_ptr->keepOnError*/) {
			goto exit_optim;
		}
		
		first = false;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;

	LogD() << "Auto measure of axial length completed, result: " << result;
	return;
}


void OctSystem::Optimizer::threadAutoLensThicknessFunction(void)
{
	bool result = false;
	bool first = true;
	bool found = false;
	bool error = false;

	int referFront = -1, referBack = -1, referCornea = -1;
	float dioptFront = 0.0f, dioptBack = -1, dioptCornea = 0.0f;

	d_ptr->running = true;
	LogD() << "Auto measure of lens thickness started, measureCnt: " << d_ptr->measureCount;

	for (int i = 0; i < d_ptr->measureCount; i++) {
		if (!ScanAutoMeasure::moveToStartCorneaCapture(true, false)) {
			error = true;
			goto exit_cornea;
		}
		else {
			for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
				this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
				if (isCancelling()) {
					goto exit_cornea;
				}
			}
		}

		if (ScanAutoMeasure::isTargetCorneaFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (found) {
			if (startAutoReference(nullptr, true, true, true, found, false)) {
				LogD() << "Auto reference started to capture cornea";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoRefer::isRunning()) {
						result = ScanAutoRefer::isCompleted();
						LogD() << "Auto reference completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoRefer::cancelOptimizing(true);
						LogD() << "Auto reference cancelled";
						goto exit_cornea;
					}
				}
			}
		}

		found = ScanAutoMeasure::checkIfTargetExist();
		if (found && (/*first ||*/ !ScanAutoMeasure::isTargetQuality())) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_cornea;
					}
				}
			}

			if (first) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization cancelled";
							goto exit_cornea;
						}
					}
				}
			}
		}

		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_cornea;
			}
		}

exit_cornea:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfCornea(true);
			referCornea = (found ? ScanAutoMeasure::getCorneaPosition() : -1);
			dioptCornea = ScanAutoMeasure::getCorneaDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetCorneaFound();
			}
		}

		LogD() << "Auto cornea focus completed, result: " << result << ", refer: " << referCornea << ", diopt: " << dioptCornea << ", measureCnt: " << i;
		GlobalRegister::runAutoCorneaFocusCompletedCallback(result, referCornea, dioptCornea);
		if (!result /* && !d_ptr->keepOnError */) {
			goto exit_optim;
		}

		if (!ScanAutoMeasure::moveToStartLensFrontCapture(first)) {
			error = true;
			goto exit_front;
		}
		else {
			for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
				this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
				if (isCancelling()) {
					goto exit_front;
				}
			}
		}

		found = true; // ScanAutoMeasure::checkIfTargetExist();
		if (found && (first || !ScanAutoMeasure::isTargetQuality(true))) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_front;
					}
				}
			}

			if (first) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization cancelled";
							goto exit_front;
						}
					}
				}
			}
		}

		if (ScanAutoMeasure::isTargetLensFrontFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (found) {
			if (startAutoReference(nullptr, true, true, true, found, false)) {
				LogD() << "Auto reference started to capture lens front";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoRefer::isRunning()) {
						result = ScanAutoRefer::isCompleted();
						LogD() << "Auto reference completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoRefer::cancelOptimizing(true);
						LogD() << "Auto reference cancelled";
						goto exit_front;
					}
				}
			}
		}

		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_front;
			}
		}

exit_front:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfLensFront(true);
			referFront = (found ? ScanAutoMeasure::getLensFrontPosition() : -1);
			dioptFront = ScanAutoMeasure::getLensFrontDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetLensFrontFound();
			}
		}

		LogD() << "Auto lens front focus completed, result: " << result << ", refer: " << referFront << ", diopt: " << dioptFront << ", measureCnt: " << i;
		GlobalRegister::runAutoLensFrontFocusCompletedCallback(result, referFront, dioptFront);
		if (!result /* && !d_ptr->keepOnError*/) {
			goto exit_optim;
		}

		if (!ScanAutoMeasure::moveToStartLensBackCapture(first)) {
			error = true;
			goto exit_back;
		}
		else {
			for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
				this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
				if (isCancelling()) {
					goto exit_back;
				}
			}
		}
	
		found = true; // ScanAutoMeasure::checkIfTargetExist();
		if (found && (first || !ScanAutoMeasure::isTargetQuality(true))) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_back;
					}
				}
			}

			if (first) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization cancelled";
							goto exit_back;
						}
					}
				}
			}
		}

		if (ScanAutoMeasure::isTargetLensBackFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (found) {
			if (startAutoReference(nullptr, true, true, true, found, false, true)) {
				LogD() << "Auto reference started to capture lens back";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoRefer::isRunning()) {
						result = ScanAutoRefer::isCompleted();
						LogD() << "Auto reference completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoRefer::cancelOptimizing(true);
						LogD() << "Auto reference cancelled";
						goto exit_back;
					}
				}
			}
		}

		for (int i = 0; i < AUTO_SCAN_FOCUS_DELAY_COUNT; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_back;
			}
		}

exit_back:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfLensBack(true);
			referBack = (found ? ScanAutoMeasure::getLensBackPosition() : -1);
			dioptBack = ScanAutoMeasure::getLensBackDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetLensBackFound();
			}
		}

		LogD() << "Auto lens back focus completed, result: " << result << ", refer: " << referBack << ", diopt: " << dioptBack << ", measureCnt: " << i;
		GlobalRegister::runAutoLensBackFocusCompletedCallback(result, referBack, dioptBack);
		if (!result /* && !d_ptr->keepOnError*/) {
			goto exit_optim;
		}

		first = false;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;

	LogD() << "Auto measure of lens thickness completed, result: " << result;
	return;
}


void OctSystem::Optimizer::threadAutoLensThicknessFunction2(void)
{
	bool result = false;
	bool first = true;
	bool found = false;
	bool error = false;

	int referFront = -1, referBack = -1, referCornea = -1;
	float dioptFront = 0.0f, dioptBack = -1, dioptCornea = 0.0f;

	bool corneaMode = !GlobalRegister::isAutoLensFrontFocusCompletedCallback();
	int delayCount = AUTO_SCAN_FOCUS_DELAY_COUNT * 2;

	d_ptr->running = true;
	LogD() << "Auto measure of lens thickness2 started, measureCnt: " << d_ptr->measureCount;

	for (int i = 0; i < d_ptr->measureCount; i++) {
		if (d_ptr->referFixed) {
		}
		else {
			if (!ScanAutoMeasure::moveToStartCorneaCapture(true, false)) {
				error = true;
				goto exit_cornea;
			}
			else {
				for (int i = 0; i < delayCount; i++) {
					this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
					if (isCancelling()) {
						goto exit_cornea;
					}
				}
			}
		}

		if (ScanAutoMeasure::isTargetCorneaFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (d_ptr->referFixed) {
		}
		else {
			if (first || !(corneaMode && found)) {
				if (startAutoReference(nullptr, true, true, true, found, false)) {
					LogD() << "Auto reference started to capture cornea";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoRefer::isRunning()) {
							result = ScanAutoRefer::isCompleted();
							LogD() << "Auto reference completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoRefer::cancelOptimizing(true);
							LogD() << "Auto reference cancelled";
							goto exit_cornea;
						}
					}
				}
			}
		}

		found = ScanAutoMeasure::checkIfTargetExist();
		if (found && (first || !ScanAutoMeasure::isTargetQuality())) {
			if (startAutoDiopterFocus()) {
				LogD() << "Auto diopter focus started";
				while (true) {
					this_thread::sleep_for(chrono::milliseconds(50));
					if (!ScanAutoFocus::isRunning()) {
						result = ScanAutoFocus::isCompleted();
						LogD() << "Auto diopter focus completed, result: " << result;
						break;
					}
					if (isCancelling()) {
						ScanAutoFocus::cancelOptimizing(true);
						LogD() << "Auto diopter focus cancelled";
						goto exit_cornea;
					}
				}
			}

			if (first) {
				if (startAutoPolarization()) {
					LogD() << "Auto polarization started";
					while (true) {
						this_thread::sleep_for(chrono::milliseconds(50));
						if (!ScanAutoPolar::isRunning()) {
							result = ScanAutoPolar::isCompleted();
							LogD() << "Auto polarization completed, result: " << result;
							break;
						}
						if (isCancelling()) {
							ScanAutoPolar::cancelOptimizing(true);
							LogD() << "Auto polarization cancelled";
							goto exit_cornea;
						}
					}
				}
			}
		}

		for (int i = 0; i < delayCount; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_cornea;
			}
		}

	exit_cornea:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfCornea(true);
			referCornea = (found ? ScanAutoMeasure::getCorneaPosition() : -1);
			dioptCornea = ScanAutoMeasure::getCorneaDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetCorneaFound();
			}
		}

		LogD() << "Auto cornea focus completed, result: " << result << ", refer: " << referCornea << ", diopt: " << dioptCornea << ", measureCnt: " << i;
		GlobalRegister::runAutoCorneaFocusCompletedCallback(result, referCornea, dioptCornea);
		if (!result /* && !d_ptr->keepOnError */) {
			goto exit_optim;
		}

		if (corneaMode) {
			goto next_loop;
		}

		if (!ScanAutoMeasure::moveToStartLensFrontCapture(first, true)) {
			error = true;
			goto exit_front;
		}
		else {
			for (int i = 0; i < delayCount; i++) {
				this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
				if (isCancelling()) {
					goto exit_front;
				}
			}
		}
		
		if (ScanAutoMeasure::isTargetLensFrontFound() || ScanAutoMeasure::checkIfTargetExist()) {
			found = true;
		}
		else {
			found = false;
		}

		if (startAutoReference(nullptr, true, true, true, found, false)) {
			LogD() << "Auto reference started to capture lens front";
			while (true) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (!ScanAutoRefer::isRunning()) {
					result = ScanAutoRefer::isCompleted();
					LogD() << "Auto reference completed, result: " << result;
					break;
				}
				if (isCancelling()) {
					ScanAutoRefer::cancelOptimizing(true);
					LogD() << "Auto reference cancelled";
					goto exit_front;
				}
			}
		}

		for (int i = 0; i < delayCount; i++) {
			this_thread::sleep_for(chrono::milliseconds(AUTO_SCAN_FOCUS_DELAY_TIME));
			if (isCancelling()) {
				goto exit_front;
			}
		}

	exit_front:
		result = ((isCancelling() || error) ? false : true);
		if (result) {
			found = ScanAutoMeasure::recordPositionOfLensFront(true);
			referFront = (found ? ScanAutoMeasure::getLensFrontPosition() : -1);
			dioptFront = ScanAutoMeasure::getLensFrontDiopter();
			if (first) {
				result = ScanAutoMeasure::isTargetLensFrontFound();
			}
		}

		LogD() << "Auto lens front focus completed, result: " << result << ", refer: " << referFront << ", diopt: " << dioptFront << ", measureCnt: " << i;
		GlobalRegister::runAutoLensFrontFocusCompletedCallback(result, referFront, dioptFront);
		if (!result /* && !d_ptr->keepOnError*/) {
			goto exit_optim;
		}

	next_loop:
		first = false;
	}

exit_optim:
	d_ptr->completed = result;
	d_ptr->running = false;

	LogD() << "Auto measure of lens thickness completed, result: " << result;
	return;
}




void OctSystem::Optimizer::threadAutoFocusFunction(void)
{
	getImpl().running = true;
	getImpl().completed = false;

	while (true)
	{
		if (!phaseAutoFocusProcess()) {
			DebugOut2() << "Auto focus cancelled with error!";
			break;
		}
		if (isCompleted()) {
			DebugOut2() << "Auto focus completed!";
			break;
		}
		if (isCancelling()) {
			DebugOut2() << "Auto focus cancelled!";
			break;
		}
	}

	getImpl().running = false;
	return;
}


void OctSystem::Optimizer::threadAutoPolarFunction(void)
{
	getImpl().running = true;
	getImpl().completed = false;

	while (true)
	{
		if (!phaseAutoPolarProcess()) {
			DebugOut2() << "Auto polarizaton cancelled with error!";
			break;
		}
		if (isCompleted()) {
			DebugOut2() << "Auto polarizaton completed!";
			break;
		}
		if (isCancelling()) {
			DebugOut2() << "Auto polarizaton cancelled!";
			break;
		}
	}

	getImpl().running = false;
	return;
}


void OctSystem::Optimizer::threadAutoReferFunction(void)
{
	getImpl().running = true;
	getImpl().completed = false;

	while (true)
	{
		if (!phaseAutoReferProcess()) {
			DebugOut2() << "Auto reference cancelled with error!";
			break;
		}
		if (isCompleted()) {
			DebugOut2() << "Auto reference completed!";
			break;
		}
		if (isCancelling()) {
			DebugOut2() << "Auto reference cancelled!";
			break;
		}
	}

	getImpl().running = false;
	return;
}


bool OctSystem::Optimizer::phaseAutoFocusProcess(void)
{
	bool res = false;

	switch (getImpl().afPhase) {
	case AutoFocusPhase::INIT:
		res = phaseAutoFocus_Initiate();
		break;
	case AutoFocusPhase::NO_SIGNAL:
	case AutoFocusPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoFocus_NoSignal();
		break;
	case AutoFocusPhase::FIRST_STEP:
		res = phaseAutoFocus_FirstStep();
		break;
	case AutoFocusPhase::FORWARD:
		res = phaseAutoFocus_Forward();
		break;
	case AutoFocusPhase::RETRACE:
		res = phaseAutoFocus_Retrace();
		break;
	}

	if (res) {
		if (getImpl().afPhase == AutoFocusPhase::COMPLETE) {
			phaseAutoFocus_Complete();
		}
		if (getImpl().afPhase == AutoFocusPhase::CANCELED) {
			phaseAutoFocus_Canceled();
		}
		return true;
	}
	else {
		phaseAutoFocus_Canceled();
		return false;
	}
}


bool OctSystem::Optimizer::phaseAutoFocus_Initiate(void)
{
	if (!renewFocusQualityFromPreview(false)) {
		return false;
	}

	if (isTargetCompleteOfFocus()) {
		getImpl().afPhase = AutoFocusPhase::COMPLETE;
	}
	else if (isTargetFoundOfFocus()) {
		getImpl().afPhase = AutoFocusPhase::FIRST_STEP;
	}
	else {
		getImpl().afPhase = AutoFocusPhase::NO_SIGNAL;
	}

	getImpl().afDirection = AUTO_FOCUS_MINUS_DIRECTION; // (isAtPlusSideOfFocusDiopter() ? AUTO_FOCUS_MINUS_DIRECTION : AUTO_FOCUS_PLUS_DIRECTION);

	updateFocusMoveStep();
	updateFocusDiopter();
	resetRetryCountToFocus();
	return true;
}


bool OctSystem::Optimizer::phaseAutoFocus_NoSignal(void)
{
	if (!moveMotorPositionOfFocus()) {
		return false;
	}

	if (!renewFocusQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfFocus())
	{
		updateFocusMoveStep();
		updateFocusDiopter();

		getImpl().afPhase = AutoFocusPhase::FORWARD;
		getImpl().afForwardCnt = 0;
	}
	else
	{
		// Reach an end of diopter range in the moving direction. 
		if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter())
		{
			if (getImpl().afPhase == AutoFocusPhase::NO_SIGNAL_REVERSE) {
				getImpl().afPhase = AutoFocusPhase::CANCELED;
			}
			else {
				getImpl().afPhase = AutoFocusPhase::NO_SIGNAL_REVERSE;
				getImpl().afDirection *= -1;
			}

			// Return to the starting position. 
			if (returnToFocusDiopter()) {
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoFocus_FirstStep(void)
{
	// Move forward to strengthen the signal.
	if (!moveMotorPositionOfFocus()) {
		return false;
	}

	float prev = getImpl().afQuality;
	if (!renewFocusQualityFromPreview(true)) {
		return false;
	}
	float curr = getImpl().afQuality;

	// If quality got worse, reverse the moving direction. 
	if (curr < prev || !isTargetFoundOfFocus())
	{
		getImpl().afDirection *= -1;
		// Return to the starting position. 
		if (!returnToFocusDiopter()) {
			return false;
		}
	}
	else if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter())
	{
		getImpl().afDirection *= -1;
		if (!returnToFocusDiopter()) {
			return false;
		}
	}

	getImpl().afPhase = AutoFocusPhase::FORWARD;
	getImpl().afForwardCnt = 0;

	updateFocusMoveStep();
	updateFocusDiopter();
	return true;
}


bool OctSystem::Optimizer::phaseAutoFocus_Forward(void)
{
	if (!isRetryCountToFocus()) {
		if (!moveMotorPositionOfFocus()) {
			return false;
		}

		if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter()) {
			getImpl().afPhase = AutoFocusPhase::COMPLETE;
		}
	}

	float prev = getImpl().afQuality;
	if (!renewFocusQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfFocus())
	{
		if (true) // getImpl().afForwardCnt++ > 0) 
		{
			float curr = getImpl().afQuality;
			if (curr >= prev)
			{
			}
			else {
				// Keep moving foward until quality getting worse enough to retrace.
				if (fabs(curr - prev) >= AUTO_FOCUS_QINDEX_DIFF_TO_RETRACE) {
					// Return to the currently best position.
					if (!returnToFocusDiopter()) {
						return false;
					}

					// Retrace with the minimum step.
					getImpl().afPhase = AutoFocusPhase::RETRACE;
					getImpl().afDirection *= -1;
					getImpl().afRetraceCnt = 0;
				}
			}
		}
		updateFocusMoveStep();
		updateFocusDiopter();
		resetRetryCountToFocus();
	}
	else
	{
		getImpl().arForwardCnt = 0;
		if (checkRetryOverToReference()) {
			getImpl().afPhase = AutoFocusPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoFocus_Retrace(void)
{
	if (!isRetryCountToFocus()) {
		if (!moveMotorPositionOfFocus()) {
			return false;
		}

		if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter()) {
			getImpl().afPhase = AutoFocusPhase::COMPLETE;
		}
	}

	float prev = getImpl().afQuality;
	if (!renewFocusQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfFocus())
	{
		if (true) // getImpl().afRetraceCnt++ > 0)
		{
			float curr = getImpl().afQuality;
			if (curr >= prev)
			{
				// Keep moving foward until quality getting worse enough to retrace.
			}
			else {
				// Return to the currently best position.
				if (!returnToFocusDiopter()) {
					return false;
				}

				getImpl().afPhase = AutoFocusPhase::COMPLETE;
			}
		}

		updateFocusMoveStep();
		updateFocusDiopter();
		resetRetryCountToFocus();
	}
	else
	{
		getImpl().arRetraceCnt = 0;
		if (checkRetryOverToReference()) {
			getImpl().afPhase = AutoFocusPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoFocus_Complete(void)
{
	if (getImpl().cbAutoFocus != nullptr) {
		(*getImpl().cbAutoFocus)(true, getImpl().afQuality, getImpl().afDiopter);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::Optimizer::phaseAutoFocus_Canceled(void)
{
	if (getImpl().cbAutoFocus != nullptr) {
		(*getImpl().cbAutoFocus)(false, getImpl().afQuality, getImpl().afDiopter);
	}

	// Completed by getting nowhere.
	getImpl().completed = true;
	return true;
}


bool OctSystem::Optimizer::renewFocusQualityFromPreview(bool next)
{
	float qidx;
	if (!obtainQualityIndexOfPreview(qidx, next)) {
		return false;
	}
	getImpl().afQuality = qidx;
	return true;
}


float OctSystem::Optimizer::getMotorStepToFocus(void)
{
	float dstep = AUTO_FOCUS_DIOPT_STEP_MAX;
	if (getImpl().afPhase == AutoFocusPhase::FIRST_STEP) {
		dstep = AUTO_FOCUS_DIOPT_STEP_FIRST;
	}
	else if (getImpl().afPhase == AutoFocusPhase::RETRACE) {
		dstep = AUTO_FOCUS_DIOPT_STEP_MIN;
	}
	else {
		float qidx = getImpl().afQuality;
		for (int i = 0; i < AUTO_FOCUS_DIOPT_STEP_SIZE; i++) {
			if (qidx >= AutoFocusQualitySteps[i]) {
				dstep = AutoFocusDioptSteps[i];
				break;
			}
		}
	}
	return dstep;
}


float OctSystem::Optimizer::getMotorOffsetToFocus(void)
{
	return getImpl().afDioptStep * getImpl().afDirection;
}


bool OctSystem::Optimizer::moveMotorPositionOfFocus(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	float offset = getMotorOffsetToFocus();

	// Move forward/reverse to find a signal. 
	if (!motor->updateDiopterByOffset(offset)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::updateFocusMoveStep(void)
{
	getImpl().afDioptStep = getMotorStepToFocus();
	return true;
}


bool OctSystem::Optimizer::updateFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	getImpl().afDiopter = motor->getCurrentDiopter();
	return true;
}


bool OctSystem::Optimizer::returnToFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	if (!motor->updateDiopter(getImpl().afDiopter)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::isHeadingForPlusEndOfFocusDiopter(void)
{
	return (getImpl().afDirection == AUTO_FOCUS_PLUS_DIRECTION);
}


bool OctSystem::Optimizer::isHeadingForMinusEndOfFocusDiopter(void)
{
	return (getImpl().afDirection == AUTO_FOCUS_MINUS_DIRECTION);
}


bool OctSystem::Optimizer::isAtPlusSideOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return (motor->getCurrentDiopter() >= 0);
}


bool OctSystem::Optimizer::isAtMinusSideOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return (motor->getCurrentDiopter() <= 0);
}


bool OctSystem::Optimizer::isAtPlusEndOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	// return (motor->isEndOfPlusDiopter() && isHeadingForPlusEndOfFocusDiopter());
	return (motor->isEndOfLowerPosition() && isHeadingForPlusEndOfFocusDiopter());
}


bool OctSystem::Optimizer::isAtMinusEndOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	// return (motor->isEndOfMinusDiopter() && isHeadingForMinusEndOfFocusDiopter());
	return (motor->isEndOfUpperPosition() && isHeadingForMinusEndOfFocusDiopter());
}


bool OctSystem::Optimizer::isTargetFoundOfFocus(void)
{
	return isQualityToSignal(getImpl().afQuality);
}


bool OctSystem::Optimizer::isTargetCompleteOfFocus(void)
{
	return isQualityToComplete(getImpl().afQuality);
}


void OctSystem::Optimizer::resetRetryCountToFocus(void)
{
	getImpl().afRetryCnt = 0;
	return;
}


bool OctSystem::Optimizer::checkRetryOverToFocus(void)
{
	getImpl().afRetryCnt++;
	if (getImpl().afRetryCnt > 3) {
		resetRetryCountToFocus();
		return true;
	}
	return false;
}


bool OctSystem::Optimizer::isRetryCountToFocus(void)
{
	return (getImpl().afRetryCnt > 0);
}


bool OctSystem::Optimizer::phaseAutoPolarProcess(void)
{
	bool res = false;

	switch (getImpl().apPhase) {
	case AutoPolarPhase::INIT:
		res = phaseAutoPolar_Initiate();
		break;
	case AutoPolarPhase::NO_SIGNAL:
	case AutoPolarPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoPolar_NoSignal();
		break;
	case AutoPolarPhase::FIRST_STEP:
		res = phaseAutoPolar_FirstStep();
		break;
	case AutoPolarPhase::FORWARD:
		res = phaseAutoPolar_Forward();
		break;
	case AutoPolarPhase::RETRACE:
		res = phaseAutoPolar_Retrace();
		break;
	}

	if (res) {
		if (getImpl().apPhase == AutoPolarPhase::COMPLETE) {
			phaseAutoPolar_Complete();
		}
		if (getImpl().apPhase == AutoPolarPhase::CANCELED) {
			phaseAutoPolar_Canceled();
		}
		return true;
	}
	else {
		phaseAutoPolar_Canceled();
		return false;
	}
}


bool OctSystem::Optimizer::phaseAutoPolar_Initiate(void)
{
	if (!renewPolarQualityFromPreview(false)) {
		return false;
	}

	if (isTargetCompleteOfPolar()) {
		getImpl().apPhase = AutoPolarPhase::COMPLETE;
	}
	else if (isTargetFoundOfPolar()) {
		getImpl().apPhase = AutoPolarPhase::FIRST_STEP;
	}
	else {
		getImpl().apPhase = AutoPolarPhase::NO_SIGNAL;
	}

	getImpl().apDirection = (isAtUpperSideOfPolarDegree() ? AUTO_POLAR_LOWER_DIRECTION : AUTO_POLAR_UPPER_DIRECTION);

	updatePolarMoveStep();
	updatePolarDegree();
	resetRetryCountToPolar();
	return true;
}


bool OctSystem::Optimizer::phaseAutoPolar_NoSignal(void)
{
	if (!moveMotorPositionOfPolar()) {
		return false;
	}

	if (!renewFocusQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfPolar())
	{
		updatePolarMoveStep();
		updatePolarDegree();

		getImpl().apPhase = AutoPolarPhase::FORWARD;
		getImpl().apForwardCnt = 0;
	}
	else {
		// Reach an end of diopter range in the moving direction. 
		if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree())
		{
			if (getImpl().apPhase == AutoPolarPhase::NO_SIGNAL_REVERSE) {
				getImpl().apPhase = AutoPolarPhase::CANCELED;
			}
			else {
				getImpl().apPhase = AutoPolarPhase::NO_SIGNAL_REVERSE;
				getImpl().apDirection *= -1;
			}

			// Return to the starting position. 
			if (returnToPolarDegree()) {
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoPolar_FirstStep(void)
{
	// Move forward to strengthen the signal.
	if (!moveMotorPositionOfPolar()) {
		return false;
	}

	float prev = getImpl().apQuality;
	if (!renewPolarQualityFromPreview(true)) {
		return false;
	}
	float curr = getImpl().apQuality;

	// If quality got worse, reverse the moving direction. 
	if (curr < prev || !isTargetFoundOfPolar())
	{
		getImpl().apDirection *= -1;
		// Return to the starting position. 
		if (!returnToPolarDegree()) {
			return false;
		}
	}
	else if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree())
	{
		getImpl().apDirection *= -1;
		if (!returnToPolarDegree()) {
			return false;
		}
	}

	getImpl().apPhase = AutoPolarPhase::FORWARD;
	getImpl().apForwardCnt = 0;

	updatePolarMoveStep();
	updatePolarDegree();
	return true;
}


bool OctSystem::Optimizer::phaseAutoPolar_Forward(void)
{
	if (!isRetryCountToPolar()) {
		if (!moveMotorPositionOfPolar()) {
			return false;
		}

		if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree()) {
			getImpl().apPhase = AutoPolarPhase::COMPLETE;
		}
	}

	float prev = getImpl().apQuality;
	if (!renewPolarQualityFromPreview(true)) {
		return false;
	}


	if (isTargetFoundOfFocus())
	{
		if (true) // getImpl().apForwardCnt++ > 0) 
		{
			float curr = getImpl().apQuality;
			if (curr >= prev)
			{
			}
			else {
				// Keep moving foward until quality getting worse enough to retrace.
				if (fabs(curr - prev) >= AUTO_POLAR_QINDEX_DIFF_TO_RETRACE) {
					// Return to the currently best position.
					if (!returnToPolarDegree()) {
						return false;
					}

					// Retrace with the minimum step.
					getImpl().apPhase = AutoPolarPhase::RETRACE;
					getImpl().apDirection *= -1;
					getImpl().apRetraceCnt = 0;
				}
			}
		}
		updatePolarMoveStep();
		updatePolarDegree();
		resetRetryCountToPolar();
	}
	else
	{
		getImpl().apForwardCnt = 0;
		if (checkRetryOverToPolar()) {
			getImpl().apPhase = AutoPolarPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoPolar_Retrace(void)
{
	if (!isRetryCountToPolar()) {
		if (!moveMotorPositionOfPolar()) {
			return false;
		}

		if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree()) {
			getImpl().apPhase = AutoPolarPhase::COMPLETE;
		}
	}

	float prev = getImpl().apQuality;
	if (!renewPolarQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfPolar())
	{
		if (true) // getImpl().apRetraceCnt++ > 0)
		{
			float curr = getImpl().apQuality;
			if (curr >= prev)
			{
				// Keep moving foward until quality getting worse enough to retrace.
			}
			else {
				// Return to the currently best position.
				if (!returnToPolarDegree()) {
					return false;
				}

				getImpl().apPhase = AutoPolarPhase::COMPLETE;
			}
		}

		updatePolarMoveStep();
		updatePolarDegree();
		resetRetryCountToPolar();
	}
	else
	{
		getImpl().apRetraceCnt = 0;
		if (checkRetryOverToPolar()) {
			getImpl().apPhase = AutoPolarPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoPolar_Complete(void)
{
	if (getImpl().cbAutoPolar != nullptr) {
		(*getImpl().cbAutoPolar)(true, getImpl().apQuality, getImpl().apDegree);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::Optimizer::phaseAutoPolar_Canceled(void)
{
	if (getImpl().cbAutoPolar != nullptr) {
		(*getImpl().cbAutoPolar)(false, getImpl().apQuality, getImpl().apDegree);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::Optimizer::renewPolarQualityFromPreview(bool next)
{
	float qidx;
	if (!obtainQualityIndexOfPreview(qidx, next)) {
		return false;
	}
	getImpl().apQuality = qidx;
	return true;
}


float OctSystem::Optimizer::getMotorStepToPolar(void)
{
	float qidx = getImpl().apQuality;

	if (isQualityToComplete(qidx)) {
		return AUTO_POLAR_MOVE_STEP_MIN;
	}
	else if (isQualityToSignal(qidx)) {
		return AUTO_POLAR_MOVE_STEP_FIRST;
	}
	else {
		return AUTO_POLAR_MOVE_STEP_MAX;
	}
}


float OctSystem::Optimizer::getMotorOffsetToPolar(void)
{
	return getImpl().apMoveStep * getImpl().apDirection;
}


bool OctSystem::Optimizer::moveMotorPositionOfPolar(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	float offset = getMotorOffsetToPolar();

	// Move forward/reverse to find a signal. 
	if (!motor->updateDegreeByOffset(offset)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::updatePolarMoveStep(void)
{
	getImpl().apMoveStep = getMotorStepToPolar();
	return true;
}


bool OctSystem::Optimizer::updatePolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	getImpl().apDegree = motor->getCurrentDegree();
	return true;
}


bool OctSystem::Optimizer::returnToPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	if (!motor->updateDegree(getImpl().apDegree)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::isHeadingForUpperEndOfPolarDegree(void)
{
	return (getImpl().apDirection == AUTO_POLAR_UPPER_DIRECTION);
}


bool OctSystem::Optimizer::isHeadingForLowerEndOfPolarDegree(void)
{
	return (getImpl().apDirection == AUTO_POLAR_LOWER_DIRECTION);
}


bool OctSystem::Optimizer::isAtUpperSideOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isAtUpperSideOfPosition());
}


bool OctSystem::Optimizer::isAtLowerSideOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isAtLowerSideOfPosition());
}


bool OctSystem::Optimizer::isAtUpperEndOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isEndOfUpperPosition() && isHeadingForUpperEndOfPolarDegree());
}


bool OctSystem::Optimizer::isAtLowerEndOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isEndOfLowerPosition() && isHeadingForLowerEndOfPolarDegree());
}


bool OctSystem::Optimizer::isTargetFoundOfPolar(void)
{
	return isQualityToSignal(getImpl().apQuality);
}


bool OctSystem::Optimizer::isTargetCompleteOfPolar(void)
{
	return isQualityToComplete(getImpl().apQuality);
}


void OctSystem::Optimizer::resetRetryCountToPolar(void)
{
	getImpl().apRetryCnt = 0;
	return;
}


bool OctSystem::Optimizer::checkRetryOverToPolar(void)
{
	getImpl().apRetryCnt++;
	if (getImpl().apRetryCnt > 3) {
		resetRetryCountToPolar();
		return true;
	}
	return false;
}


bool OctSystem::Optimizer::isRetryCountToPolar(void)
{
	return (getImpl().apRetryCnt > 0);
}


bool OctSystem::Optimizer::phaseAutoReferProcess(void)
{
	bool res = false;

	switch (getImpl().arPhase) {
	case AutoReferPhase::INIT:
		res = phaseAutoRefer_Initiate();
		break;
	case AutoReferPhase::NO_SIGNAL:
	case AutoReferPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoRefer_NoSignal();
		break;
	case AutoReferPhase::FORWARD:
		res = phaseAutoRefer_Forward();
		break;
	case AutoReferPhase::RETRACE:
		res = phaseAutoRefer_Retrace();
		break;
	case AutoReferPhase::CENTER:
		res = phaseAutoRefer_Center();
		break;
	}

	if (res) {
		if (getImpl().arPhase == AutoReferPhase::COMPLETE) {
			phaseAutoRefer_Complete();
		}
		if (getImpl().arPhase == AutoReferPhase::CANCELED) {
			phaseAutoRefer_Canceled();
		}
		return true;
	}
	else {
		phaseAutoRefer_Canceled();
		return false;
	}
}


bool OctSystem::Optimizer::phaseAutoRefer_Initiate(void)
{
	if (!renewReferenceQualityFromPreview(false)) {
		return false;
	}

	if (!renewReferencePointFromPreview(false)) {
		return false;
	}

	DebugOut2() << "Start auto reference, qidx: " << getImpl().arQuality;

	if (isTargetFoundOfReference()) {
		if (isTargetAtReferenceCenter()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Center completed";
		}
		else {
			getImpl().arPhase = AutoReferPhase::FORWARD;
			getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			getImpl().arForwardCnt = 0;
			DebugOut2() << "Target found to forward";
		}
	}
	else {
		getImpl().arPhase = AutoReferPhase::NO_SIGNAL;
		// getImpl().arDirection = (isAtLowerSideOfReference() ? AUTO_REFER_UPPER_DIRECTION : AUTO_REFER_LOWER_DIRECTION);
		getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
		DebugOut2() << "Target not found to no signal";
	}

	updateReferenceMoveStep();
	updateReferencePosition();
	resetRetryCountToReference();
	return true;
}


bool OctSystem::Optimizer::phaseAutoRefer_NoSignal(void)
{
	if (!moveMotorPositionOfReference()) {
		return false;
	}

	if (!renewReferenceQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfReference()) 
	{
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}

		updateReferenceMoveStep();
		updateReferencePosition();

		if (isHeadingForLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::FORWARD;
			getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION; // (isTargetAboveReferenceCenter() ? AUTO_REFER_UPPER_DIRECTION : AUTO_REFER_LOWER_DIRECTION);
			getImpl().arRetraceCnt = 0;
			DebugOut2() << "Target found to retrace, qidx: " << getImpl().arQuality << ", refPoint: " << getImpl().arRefPoint << ", step: " << getImpl().arMoveStep << ", mpos: " << getImpl().arPosition;
		}
		else {
			getImpl().arPhase = AutoReferPhase::FORWARD;
			getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			getImpl().arForwardCnt = 0;
			DebugOut2() << "Target found to forward, qidx: " << getImpl().arQuality << ", refPoint: " << getImpl().arRefPoint << ", step: " << getImpl().arMoveStep << ", mpos: " << getImpl().arPosition;
		}
	}
	else {
		// Reach an end of diopter range in the moving direction. 
		if (isAtUpperEndOfReference() || isAtLowerEndOfReference())
		{
			if (getImpl().arPhase == AutoReferPhase::NO_SIGNAL_REVERSE) {
				getImpl().arPhase = AutoReferPhase::CANCELED;
				DebugOut2() << "Target not found to canceled, return to mpos: " << getImpl().arPosition;
			}
			else {
				getImpl().arPhase = AutoReferPhase::NO_SIGNAL_REVERSE;
				getImpl().arDirection *= -1;		// Reverse the direction.
				DebugOut2() << "Target not found to reverse, return to mpos: " << getImpl().arPosition;
			}

			// Return to the starting position. 
			if (!returnToReferencePosition()) {
				DebugOut2() << "Return to initial position failed";
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoRefer_Forward(void)
{
	if (!isRetryCountToReference()) {
		if (!moveMotorPositionOfReference()) {
			return false;
		}

		if (isAtUpperEndOfReference() || isAtLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Retrace completed at end of reference";
		}
	}

	if (!renewReferenceQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfReference())
	{
		int prev = getImpl().arRefPoint;
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}
		int curr = getImpl().arRefPoint;

		if (getImpl().arForwardCnt++ > 0) {
			if (curr > prev) {
				getImpl().arPhase = AutoReferPhase::RETRACE;
				getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			}
		}

		updateReferenceMoveStep();
		updateReferencePosition();
		resetRetryCountToReference();
		DebugOut2() << "Forwarding target, qidx: " << getImpl().arQuality << ", refPoint: " << getImpl().arRefPoint << ", mpos: " << getImpl().arPosition;
	}
	else {
		getImpl().arForwardCnt = 0;

		if (checkRetryOverToReference()) {
			// getImpl().arPhase = AutoReferPhase::CANCELED;
			// DebugOut2() << "Target not found in forward, canceled";
			getImpl().arPhase = AutoReferPhase::RETRACE;
			getImpl().arDirection = AUTO_REFER_LOWER_DIRECTION;
			resetRetryCountToReference();
			DebugOut2() << "Target not found in forward, retrace";
		}
		else {
			DebugOut2() << "Target not found in forward, retrying...";
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoRefer_Retrace(void)
{
	// if (!isRetryCountToReference()) {
		if (!moveMotorPositionOfReference()) {
			return false;
		}

		if (isAtUpperEndOfReference() || isAtLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Retrace canceled at end of reference";
		}
	// }

	if (!renewReferenceQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfReference())
	{
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}

		if (isHeadingForUpperEndOfReference())
		{
			if (isTargetBelowReferenceCenter()) {
				getImpl().arPhase = AutoReferPhase::CENTER;
				DebugOut2() << "Retrace completed at below, to center";
			}
		}
		else
		{
			if (isTargetAboveReferenceCenter()) {
				getImpl().arPhase = AutoReferPhase::CENTER;
				DebugOut2() << "Retrace completed at above, to center";
			}
		}

		updateReferenceMoveStep();
		updateReferencePosition();
		resetRetryCountToReference();
	}
	else
	{
		if (checkRetryOverToReference()) {
			getImpl().arPhase = AutoReferPhase::CANCELED;
			DebugOut2() << "Target not found in retrace, canceled";
		}
		else {
			DebugOut2() << "Target not found in retrace, retrying";
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoRefer_Center(void)
{
	if (!isRetryCountToReference()) {
		if (!moveMotorPositionOfReference()) {
			return false;
		}

		if (isAtUpperEndOfReference() || isAtLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Center canceled at end of reference";
		}
	}

	if (!renewReferenceQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfReference())
	{
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}

		if (isTargetAtReferenceCenter()) {
			if (++getImpl().arSuccessCnt > 1) {
				getImpl().arPhase = AutoReferPhase::COMPLETE;
				DebugOut2() << "Center completed";
			}
		}
		else
		{
			getImpl().arSuccessCnt = 0;
			if (isTargetBelowReferenceCenter())
			{
				getImpl().arDirection = AUTO_REFER_LOWER_DIRECTION;
			}
			else
			{
				getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			}
		}

		updateReferenceMoveStep();
		updateReferencePosition();
		resetRetryCountToReference();

		DebugOut2() << "Reference point: " << getImpl().arRefPoint << ", step: " << getImpl().arMoveStep << ", dir: " << getImpl().arDirection << ", dist: " << getTargetDistanceFromReferCenter();
	}
	else
	{
		if (checkRetryOverToReference()) {
			getImpl().arPhase = AutoReferPhase::CANCELED;
			DebugOut2() << "Target not found in center, canceled";
		}
		else {
			DebugOut2() << "Target not found in center, retrying";
		}
	}
	return true;
}


bool OctSystem::Optimizer::phaseAutoRefer_Complete(void)
{
	if (getImpl().cbAutoRefer != nullptr) {
		(*getImpl().cbAutoRefer)(true, getImpl().arRefPoint, getImpl().arPosition);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::Optimizer::phaseAutoRefer_Canceled(void)
{
	if (getImpl().cbAutoRefer != nullptr) {
		(*getImpl().cbAutoRefer)(false, getImpl().arRefPoint, getImpl().arPosition);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::Optimizer::obtainQualityIndexOfPreview(float & qidx, bool next)
{
	unsigned long prevCount, nextCount;
	float prevIndex, nextIndex;

	// Ignore the preview image right after the motor moving.
	ChainOutput::getPreviewImageResult(&prevIndex, nullptr, nullptr, &prevCount);
	if (!next) {
		qidx = prevIndex;
		return true;
	}

	int waits = 0;
	const int waitsMax = 50;

	while (true) {
		// wait for 250ms to obtain the next preview image.
		this_thread::sleep_for(chrono::milliseconds(5));
		if (++waits > waitsMax) {
			break;
		}

		ChainOutput::getPreviewImageResult(&nextIndex, nullptr, nullptr, &nextCount);
		if (prevCount != nextCount) {
			qidx = nextIndex;
			return true;
		}
	}
	return false;
}


bool OctSystem::Optimizer::obtainReferencePointOfPreview(int & refPoint, bool next)
{
	unsigned long prevCount, nextCount;
	int prevPoint, nextPoint;

	// Ignore the preview image right after the motor moving.
	ChainOutput::getPreviewImageResult(nullptr, nullptr, &prevPoint, &prevCount);
	if (!next) {
		refPoint = prevPoint;
		return true;
	}

	int waits = 0;
	const int waitsMax = 50;

	while (true) {
		// wait for 250ms to obtain the next preview image.
		this_thread::sleep_for(chrono::milliseconds(5));
		if (++waits > waitsMax) {
			break;
		}

		ChainOutput::getPreviewImageResult(nullptr, nullptr, &nextPoint, &nextCount);
		if (prevCount != nextCount) {
			refPoint = nextPoint;
			return true;
		}
	}
	return false;
}


float OctSystem::Optimizer::getFocusDiopterStep(float qidx, bool init)
{
	float dstep = AUTO_FOCUS_DIOPT_STEP_MAX;
	for (int i = 0; i < AUTO_FOCUS_DIOPT_STEP_SIZE; i++) {
		if (qidx >= AutoFocusQualitySteps[i]) {
			dstep = AutoFocusDioptSteps[i];
			break;
		}
	}
	return dstep;
}


float OctSystem::Optimizer::getFocusDiopterOffset(void)
{
	float offset = getImpl().afDioptStep * getImpl().afDirection;
	return offset;
}


float OctSystem::Optimizer::getPolarDegreeStep(float qidx)
{
	if (isQualityToComplete(qidx)) {
		return AUTO_POLAR_MOVE_STEP_MIN;
	}
	else if (isQualityToSignal(qidx)) {
		return AUTO_POLAR_MOVE_STEP_FIRST;
	}
	else {
		return AUTO_POLAR_MOVE_STEP_MAX;
	}
}


bool OctSystem::Optimizer::renewReferenceQualityFromPreview(bool next)
{
	float qidx;
	if (!obtainQualityIndexOfPreview(qidx, next)) {
		return false;
	}
	getImpl().arQuality = qidx;
	return true;
}


bool OctSystem::Optimizer::renewReferencePointFromPreview(bool next)
{
	int refPoint;
	if (!obtainReferencePointOfPreview(refPoint, next)) {
		return false;
	}

	getImpl().arRefPoint = refPoint;
	return true;
}


int OctSystem::Optimizer::getMotorStepToReference(void)
{
	int mstep = AUTO_REFER_MOVE_STEP_MAX;
	if (getImpl().arPhase == AutoReferPhase::NO_SIGNAL ||
		getImpl().arPhase == AutoReferPhase::NO_SIGNAL_REVERSE) {
		mstep = AUTO_REFER_MOVE_STEP_FIND;
	}
	else if (getImpl().arPhase == AutoReferPhase::FORWARD) {
		mstep = AUTO_REFER_MOVE_STEP_MAX;
	}
	else {
		int dist = getTargetDistanceFromReferCenter();
		for (int i = 0; i < AUTO_REFER_MOVE_STEP_SIZE; i++) {
			if (dist <= AutoReferPointSteps[i]) {
				mstep = AutoReferMoveSteps[i];
				break;
			}
		}
	}
	return mstep;
}


int OctSystem::Optimizer::getMotorOffsetToReference(void)
{
	return getImpl().arMoveStep * getImpl().arDirection;
}


bool OctSystem::Optimizer::moveMotorPositionOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	int offset = getMotorOffsetToReference();

	// Move forward/reverse to find a signal. 
	if (!motor->updatePositionByOffset(offset)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::updateReferenceMoveStep(void)
{
	getImpl().arMoveStep = getMotorStepToReference();
	return true;
}


bool OctSystem::Optimizer::updateReferencePosition(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	getImpl().arPosition = motor->getPosition();
	return true;
}


bool OctSystem::Optimizer::returnToReferencePosition(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	if (!motor->updatePosition(getImpl().arPosition)) {
		return false;
	}
	return true;
}


bool OctSystem::Optimizer::isTargetFoundOfReference(void)
{
	return isQualityToSignal(getImpl().arQuality);
}


bool OctSystem::Optimizer::isHeadingForUpperEndOfReference(void)
{
	return (getImpl().arDirection == AUTO_REFER_UPPER_DIRECTION);
}


bool OctSystem::Optimizer::isHeadingForLowerEndOfReference(void)
{
	return (getImpl().arDirection == AUTO_REFER_LOWER_DIRECTION);
}


bool OctSystem::Optimizer::isAtUpperSideOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return motor->isAtUpperSide();
}


bool OctSystem::Optimizer::isAtLowerSideOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return motor->isAtLowerSide();
}


bool OctSystem::Optimizer::isAtUpperEndOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return (motor->isEndOfUpperPosition() && isHeadingForUpperEndOfReference());
}


bool OctSystem::Optimizer::isAtLowerEndOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return (motor->isEndOfLowerPosition() && isHeadingForLowerEndOfReference());
}


int OctSystem::Optimizer::getTargetDistanceFromReferCenter(void)
{
	return abs(getTargetOffsetFromReferCenter());
}


int OctSystem::Optimizer::getTargetOffsetFromReferCenter(void)
{
	int offset = (getImpl().arRefPoint - AUTO_REFER_CENTER_POINT);
	return offset;
}


bool OctSystem::Optimizer::isTargetBelowReferenceCenter(void)
{
	return (getTargetOffsetFromReferCenter() >= 0);
}


bool OctSystem::Optimizer::isTargetAboveReferenceCenter(void)
{
	return (getTargetOffsetFromReferCenter() <= 0);
}


bool OctSystem::Optimizer::isTargetAtReferenceCenter(void)
{
	/*
	if (getTargetDistanceFromReferCenter() <= AUTO_REFER_CENTER_OFFSET) {
		return true;
	}
	*/

	int offset = abs(getTargetOffsetFromReferCenter());
	DebugOut2() << "Reference point: " << getImpl().arRefPoint << ", offset: " << offset;
	if (offset >= 0 && offset <= AUTO_REFER_CENTER_OFFSET) {
		return true;
	}
	return false;
}


void OctSystem::Optimizer::resetRetryCountToReference(void)
{
	getImpl().arRetryCnt = 0;
	return;
}


bool OctSystem::Optimizer::checkRetryOverToReference(void)
{
	getImpl().arRetryCnt++;
	if (getImpl().arRetryCnt > 3) {
		resetRetryCountToReference();
		return true;
	}
	return false;
}


bool OctSystem::Optimizer::isRetryCountToReference(void)
{
	return (getImpl().arRetryCnt > 0);
}


bool OctSystem::Optimizer::isQualityToComplete(float qidx)
{
	return (qidx >= AUTO_FOCUS_QINDEX_TO_COMPLETE ? true : false);
}


bool OctSystem::Optimizer::isQualityToSignal(float qidx)
{
	return (qidx >= AUTO_FOCUS_QINDEX_TO_SIGNAL ? true : false);
}


