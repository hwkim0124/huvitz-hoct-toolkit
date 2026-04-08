#include "stdafx.h"
#include "GlobalRegister.h"
#include "GlobalScanDef.h"

#include <mutex>

using namespace OctGlobal;
using namespace std;


struct GlobalRegister::GlobalRegisterImpl
{
	PreviewImageCompletedEvent* cbPreviewImageCompleted;
	MeasureImageCompletedEvent* cbMeasureImageCompleted;
	EnfaceImageCompletedEvent* cbEnfaceImageCompleted;

	AutoOptimizeCompletedCallback* cbAutoOptimizeCompleted;
	AutoPositionCompletedCallback* cbAutoPositionCompleted;
	AutoMeasureCompletedCallback* cbAutoMeasureCompleted;
	AutoCorneaFocusCompletedCallback* cbAutoCorneaCompleted;
	AutoRetinaFocusCompletedCallback* cbAutoRetinaCompleted;
	AutoLensFrontFocusCompletedCallback* cbAutoFrontCompleted;
	AutoLensBackFocusCompletedCallback* cbAutoBackCompleted;

	FundusImageAcquiredCallback* cbFundusImageAcquired;
	FundusImageCompletedCallback* cbFundusImageCompleted;
	ScanPatternAcquiredCallback* cbScanPatternAcquired;
	ScanPatternCompletedCallback* cbScanPatternCompleted;

	GlobalRegisterImpl() 
		: cbPreviewImageCompleted(nullptr), cbMeasureImageCompleted(nullptr), cbEnfaceImageCompleted(nullptr),
			cbAutoOptimizeCompleted(nullptr), cbAutoPositionCompleted(nullptr), 
			cbFundusImageAcquired(nullptr), cbFundusImageCompleted(nullptr), 
			cbScanPatternAcquired(nullptr), cbScanPatternCompleted(nullptr), cbAutoMeasureCompleted(nullptr), 
			cbAutoCorneaCompleted(nullptr), cbAutoRetinaCompleted(nullptr), 
			cbAutoFrontCompleted(nullptr), cbAutoBackCompleted(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<GlobalRegister::GlobalRegisterImpl> GlobalRegister::d_ptr(new GlobalRegisterImpl());


GlobalRegister::GlobalRegister()
{
}


GlobalRegister::~GlobalRegister()
{
}


void OctGlobal::GlobalRegister::setPreviewImageCompletedEvent(PreviewImageCompletedEvent * callback)
{
	d_ptr->cbPreviewImageCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::setEnfaceImageCompletedEvent(EnfaceImageCompletedEvent * callback)
{
	d_ptr->cbEnfaceImageCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::setMeasureImageCompletedEvent(MeasureImageCompletedEvent * callback)
{
	d_ptr->cbMeasureImageCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::triggerPreviewImageCompleted(unsigned char * data, unsigned int width, unsigned height, float qindex, float sigRatio, int refPoint, int idxImage)
{
	auto callback = d_ptr->cbPreviewImageCompleted;
	if (callback != nullptr) {
		(*callback)(data, width, height, qindex, sigRatio, refPoint, idxImage);
	}
	return;
}


void OctGlobal::GlobalRegister::triggerMeasureImageCompleted(unsigned char * data, unsigned int width, unsigned height, float qindex, float sigRatio, int refPoint, int idxImage, unsigned char* lateral)
{
	auto callback = d_ptr->cbMeasureImageCompleted;
	if (callback != nullptr) {
		(*callback)(data, width, height, qindex, sigRatio, refPoint, idxImage, lateral);
	}
	return;
}


void OctGlobal::GlobalRegister::triggerEnfaceImageCompleted(unsigned char * data, unsigned int width, unsigned height)
{
	auto callback = d_ptr->cbEnfaceImageCompleted;
	if (callback != nullptr) {
		(*callback)(data, width, height);
	}
	return;
}


void OctGlobal::GlobalRegister::setAutoOptimizeCompletedCallback(AutoOptimizeCompletedCallback * callback)
{
	d_ptr->cbAutoOptimizeCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::setAutoPositionCompletedCallback(AutoPositionCompletedCallback * callback)
{
	d_ptr->cbAutoPositionCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::setAutoMeasureCompletedCallback(AutoMeasureCompletedCallback * callback)
{
	d_ptr->cbAutoMeasureCompleted = callback;
	return;
}

void OctGlobal::GlobalRegister::setAutoCorneaFocusCompletedCallback(AutoCorneaFocusCompletedCallback * callback)
{
	d_ptr->cbAutoCorneaCompleted = callback;
	return;
}

void OctGlobal::GlobalRegister::setAutoRetinaFocusCompletedCallback(AutoRetinaFocusCompletedCallback * callback)
{
	d_ptr->cbAutoRetinaCompleted = callback;
	return;
}

void OctGlobal::GlobalRegister::setAutoLensFrontFocusCompletedCallback(AutoLensFrontFocusCompletedCallback * callback)
{
	d_ptr->cbAutoFrontCompleted = callback;
	return;
}

void OctGlobal::GlobalRegister::setAutoLensBackFocusCompletedCallback(AutoLensBackFocusCompletedCallback * callback)
{
	d_ptr->cbAutoBackCompleted = callback;
	return;
}

void OctGlobal::GlobalRegister::runAutoOptimizeCompletedCallback(bool result)
{
	auto callback = d_ptr->cbAutoOptimizeCompleted;
	if (callback != nullptr) {
		(*callback)(result);
	}
	return;
}


void OctGlobal::GlobalRegister::runAutoPositionCompletedCallback(bool result)
{
	auto callback = d_ptr->cbAutoPositionCompleted;
	if (callback != nullptr) {
		(*callback)(result);
	}
	return;
}


void OctGlobal::GlobalRegister::runAutoMeasureCompletedCallback(bool result, int posMacular, int posCornea, float length)
{
	auto callback = d_ptr->cbAutoMeasureCompleted;
	if (callback != nullptr) {
		(*callback)(result, posMacular, posCornea, length);
	}
	return;
}

void OctGlobal::GlobalRegister::runAutoCorneaFocusCompletedCallback(bool result, int referPos, float dioptPos)
{
	auto callback = d_ptr->cbAutoCorneaCompleted;
	if (callback != nullptr) {
		(*callback)(result, referPos, dioptPos);
	}
	return;
}

void OctGlobal::GlobalRegister::runAutoRetinaFocusCompletedCallback(bool result, int referPos, float dioptPos)
{
	auto callback = d_ptr->cbAutoRetinaCompleted;
	if (callback != nullptr) {
		(*callback)(result, referPos, dioptPos);
	}
	return;
}

void OctGlobal::GlobalRegister::runAutoLensFrontFocusCompletedCallback(bool result, int referPos, float dioptPos)
{
	auto callback = d_ptr->cbAutoFrontCompleted;
	if (callback) {
		(*callback)(result, referPos, dioptPos);
	}
}

void OctGlobal::GlobalRegister::runAutoLensBackFocusCompletedCallback(bool result, int referPos, float dioptPos)
{
	auto callback = d_ptr->cbAutoBackCompleted;
	if (callback) {
		(*callback)(result, referPos, dioptPos);
	}
}


bool OctGlobal::GlobalRegister::isAutoLensFrontFocusCompletedCallback(void)
{
	return (d_ptr->cbAutoFrontCompleted != nullptr);
}


void OctGlobal::GlobalRegister::setFundusImageAcquiredCallback(FundusImageAcquiredCallback * callback)
{
	d_ptr->cbFundusImageAcquired = callback;
	return;
}


void OctGlobal::GlobalRegister::setFundusImageCompletedCallback(FundusImageCompletedCallback * callback)
{
	d_ptr->cbFundusImageCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::setScanPatternAcquiredCallback(ScanPatternAcquiredCallback * callback)
{
	d_ptr->cbScanPatternAcquired = callback;
	return;
}


void OctGlobal::GlobalRegister::setScanPatternCompletedCallback(ScanPatternCompletedCallback * callback)
{
	d_ptr->cbScanPatternCompleted = callback;
	return;
}


void OctGlobal::GlobalRegister::runFundusImageAcquiredCallback(bool result)
{
	auto callback = d_ptr->cbFundusImageAcquired;
	if (callback != nullptr) {
		(*callback)(result);
	}
	return;
}


void OctGlobal::GlobalRegister::runFundusImageCompletedCallback(bool result)
{
	auto callback = d_ptr->cbFundusImageCompleted;
	if (callback != nullptr) {
		(*callback)(result);
	}
	return;
}


void OctGlobal::GlobalRegister::runScanPatternAcquiredCallback(bool result)
{
	auto callback = d_ptr->cbScanPatternAcquired;
	if (callback != nullptr) {
		(*callback)(result);
	}
	return;
}


void OctGlobal::GlobalRegister::runScanPatternCompletedCallback(bool result)
{
	auto callback = d_ptr->cbScanPatternCompleted;
	if (callback != nullptr) {
		(*callback)(result);
	}
	return;
}


GlobalRegister::GlobalRegisterImpl & OctGlobal::GlobalRegister::getImpl(void)
{
	return *d_ptr;
}

