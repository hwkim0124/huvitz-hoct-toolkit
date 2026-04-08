#include "stdafx.h"
#include "RetinaFocus.h"

#include <mutex>

#include "OctDevice2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace std;


struct RetinaFocus::RetinaFocusImpl
{
	bool initiated;
	MainBoard* board;

	IrCameraFrameCallback cbFrameFunc;
	RetinaCameraImageCallback* cbImageFunc;
	mutex mutexLock;

	RetinaFocusImpl() : initiated(false), board(nullptr), cbFrameFunc(nullptr), cbImageFunc(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<RetinaFocus::RetinaFocusImpl> RetinaFocus::d_ptr(new RetinaFocusImpl());


RetinaFocus::RetinaFocus()
{
}


RetinaFocus::~RetinaFocus()
{
}


bool OctSystem::RetinaFocus::initialize(OctDevice::MainBoard * board)
{
	getImpl().board = board;
	getImpl().initiated = true;

	// The type of a pointer to static member function looks like a pointer to non-member function.
	// It doesn't require this argument since static functions are bound to class not to the object. 
	getImpl().cbFrameFunc = std::bind(&RetinaFocus::callbackIrCameraFrame, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	board->getRetinaIrCamera()->setCallback(&getImpl().cbFrameFunc);
	getImpl().cbImageFunc = nullptr;
	return true;
}


void OctSystem::RetinaFocus::setCameraImageCallback(RetinaCameraImageCallback * callback)
{
	getImpl().cbImageFunc = callback;
	return;
}


RetinaFocus::RetinaFocusImpl & OctSystem::RetinaFocus::getImpl(void)
{
	return *d_ptr;
}


void OctSystem::RetinaFocus::callbackIrCameraFrame(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	// Pass the retina ir image with focus information. 
	unique_lock<mutex> lock(getImpl().mutexLock);
	if (getImpl().cbImageFunc != nullptr) {
		(*getImpl().cbImageFunc)(data, width, height);
	}
	return;
}
