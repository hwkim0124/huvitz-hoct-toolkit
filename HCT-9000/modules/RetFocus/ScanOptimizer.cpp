#include "stdafx.h"
#include "ScanOptimizer.h"

#include "OctDevice2.h"
#include "SigChain2.h"
#include "CppUtil2.h"

using namespace RetFocus;
using namespace OctDevice;
using namespace CppUtil;
using namespace SigChain;
using namespace std;


struct ScanOptimizer::ScanOptimizerImpl
{
	unsigned long splitCount;

	ScanOptimizerImpl() : splitCount(0) {
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanOptimizer::ScanOptimizerImpl> ScanOptimizer::d_ptr(new ScanOptimizerImpl());


ScanOptimizer::ScanOptimizer()
{
}


ScanOptimizer::~ScanOptimizer()
{
}


bool RetFocus::ScanOptimizer::obtainQualityIndexFromPreview(float& qidx, float& sig_ratio, bool next)
{
	unsigned long prevCount, nextCount;
	float prevIndex, nextIndex;
	float prevRatio, nextRatio;

	// Ignore the preview image right after motor moving.
	ChainOutput::getPreviewImageResult(&prevIndex, &prevRatio, nullptr, &prevCount);
	if (!next) {
		qidx = prevIndex;
		sig_ratio = prevRatio;
		return true;
	}

	int waits = 0;

	while (true) {
		this_thread::sleep_for(chrono::milliseconds(OBTAIN_NEXT_PREVIEW_WAIT_DELAY));
		if (++waits > OBTAIN_NEXT_PREVIEW_RETRY_MAX) {
			break;
		}

		ChainOutput::getPreviewImageResult(&nextIndex, &nextRatio, nullptr, &nextCount);
		if (prevCount != nextCount) {
			qidx = nextIndex;
			sig_ratio = nextRatio;
			return true;
		}
	}
	return false;
}


bool RetFocus::ScanOptimizer::obtainReferencePointFromPreview(int& refPoint, bool next)
{
	unsigned long prevCount, nextCount;
	int prevPoint, nextPoint;

	// Ignore the preview image right after motor moving.
	ChainOutput::getPreviewImageResult(nullptr, nullptr, &prevPoint, &prevCount);
	if (!next) {
		refPoint = prevPoint;
		return true;
	}

	int waits = 0;

	while (true) {
		this_thread::sleep_for(chrono::milliseconds(OBTAIN_NEXT_PREVIEW_WAIT_DELAY));
		if (++waits > OBTAIN_NEXT_PREVIEW_RETRY_MAX) {
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


bool RetFocus::ScanOptimizer::obtainSplitFocusFromRetina(float & offset, bool next)
{
	unsigned long count;
	int waits = 0;

	while (true) {
		GlobalRecord::readSplitFocusOffset(offset, count);
		if (d_ptr->splitCount < count) {
			d_ptr->splitCount = count;
			return true;
		}

		this_thread::sleep_for(chrono::milliseconds(OBTAIN_NEXT_SPLIT_FOCUS_WAIT_DELAY));
		if (++waits > OBTAIN_NEXT_SPLIT_FOCUS_RETRAY_MAX) {
			break;
		}
	}
	return false;
}


bool RetFocus::ScanOptimizer::isQualityToSignal(float qidx)
{
	return (qidx >= SCAN_AUTO_QINDEX_TO_SIGNAL ? true : false);
}


bool RetFocus::ScanOptimizer::isQualityToComplete(float qidx)
{
	return (qidx >= SCAN_AUTO_QINDEX_TO_COMPLETE ? true : false);
}


bool RetFocus::ScanOptimizer::isQualityToTarget(float qidx, bool isLensBack)
{
	if (isLensBack) {
		return (qidx >= SCAN_AUTO_QINDEX_TO_TARGET_LENS_BACK ? true : false);
	}
	else {
		return (qidx >= SCAN_AUTO_QINDEX_TO_TARGET ? true : false);
	}
}


bool RetFocus::ScanOptimizer::isQualityToConfirm(float qidx)
{
	return (qidx >= SCAN_AUTO_QINDEX_TO_CONFIRM ? true : false);
}


void RetFocus::ScanOptimizer::onScanOptimizingStarted(void) 
{
	ChainSetup::pauseDynamicDispersionCorrection(true);
}


void RetFocus::ScanOptimizer::onScanOptimizingClosed(void)
{
	ChainSetup::pauseDynamicDispersionCorrection(false);
}


ScanOptimizer::ScanOptimizerImpl & RetFocus::ScanOptimizer::getImpl(void)
{
	return *d_ptr;
}
