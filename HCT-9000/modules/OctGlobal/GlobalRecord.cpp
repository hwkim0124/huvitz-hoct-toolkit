#include "stdafx.h"
#include "GlobalRecord.h"

#include <mutex>

using namespace OctGlobal;
using namespace std;


struct GlobalRecord::GlobalRecordImpl
{
	unsigned long splitFocusCount;
	float splitFocusOffset;
	float flashPupilExtent;
	int flashPupilWidth;
	float splitFocusSize;
	float splitFocusInt;
	bool retinaTargetDisplaced;
	bool retinaTargetNotFound;
	bool retinaEyelidCovered;

	vector<int> corneaAnteriorCurves[2];
	bool corneaTargetFounds[2] = { false };
	float corneaCenterX[2];
	float corneaCenterY[2];

	float octImageSignalLevel;
	float octImageBackgroundLevel;
	float octImageNoiseLevel;

	mutex mutexAccess;

	GlobalRecordImpl() : splitFocusCount(0), splitFocusOffset(0.0f), flashPupilExtent(1.0f), 
		splitFocusInt(0.0f), splitFocusSize(0.0f), flashPupilWidth(0), 
		retinaTargetDisplaced(false), retinaTargetNotFound(false), retinaEyelidCovered(false), 
		octImageSignalLevel(0.0f), octImageBackgroundLevel(0.0f), octImageNoiseLevel(0.0f)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<GlobalRecord::GlobalRecordImpl> GlobalRecord::d_ptr(new GlobalRecordImpl());


GlobalRecord::GlobalRecord()
{
}


GlobalRecord::~GlobalRecord()
{
}


void OctGlobal::GlobalRecord::setOctImageSignalLevel(float level)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->octImageSignalLevel = level;
	return;
}

float OctGlobal::GlobalRecord::getOctImageSignalLevel(void)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	float level = d_ptr->octImageSignalLevel;
	return level;
}

void OctGlobal::GlobalRecord::setOctImageBackgroundLevel(float level)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->octImageBackgroundLevel = level;
	return;
}

float OctGlobal::GlobalRecord::getOctImageBackgroundLevel(void)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	float level = d_ptr->octImageBackgroundLevel;
	return level;
}

void OctGlobal::GlobalRecord::setOctImageNoiseLevel(float level)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->octImageNoiseLevel = level;
	return;
}

float OctGlobal::GlobalRecord::getOctImageNoiseLevel(void)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	float level = d_ptr->octImageNoiseLevel;
	return level;
}

void OctGlobal::GlobalRecord::writeSplitFocusOffset(float offset)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->splitFocusCount += 1;
	d_ptr->splitFocusOffset = offset;
	return;
}

void OctGlobal::GlobalRecord::writeSplitFocusSize(float size, float intensity)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->splitFocusSize = size;
	d_ptr->splitFocusInt = intensity;
	return;
}

void OctGlobal::GlobalRecord::writeFlashPupilExtent(float extent)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->flashPupilExtent = extent;
	return;
}


void OctGlobal::GlobalRecord::writeFlashPupilWidth(int width)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->flashPupilWidth = width;
	return;
}


void OctGlobal::GlobalRecord::readSplitFocusOffset(float & offset, unsigned long & count)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	offset = d_ptr->splitFocusOffset;
	count = d_ptr->splitFocusCount;
	return;
}


void OctGlobal::GlobalRecord::readSplitFocusSize(float & size, float & intensity)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	size = d_ptr->splitFocusSize;
	intensity = d_ptr->splitFocusInt;
	return;
}


void OctGlobal::GlobalRecord::readFlashPupilExtent(float & extent)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	extent = d_ptr->flashPupilExtent;
	return;
}


void OctGlobal::GlobalRecord::readFlashPupilWidth(int & width)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	width = d_ptr->flashPupilWidth;
	return;
}

void OctGlobal::GlobalRecord::writeCorneaTrackTargetFound(bool flag, int lineIdx)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	int idx = lineIdx != 0 ? 1 : lineIdx;
	getImpl().corneaTargetFounds[idx] = flag;
}

void OctGlobal::GlobalRecord::writeCorneaTrackAnteriorCurve(std::vector<int> curve, int lineIdx)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	int idx = lineIdx != 0 ? 1 : lineIdx;
	getImpl().corneaAnteriorCurves[idx] = curve;
}

void OctGlobal::GlobalRecord::writeCorneaTrackAnteriorCenter(float xpos, float ypos, int lineIdx)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	int idx = lineIdx != 0 ? 1 : lineIdx;
	getImpl().corneaCenterX[idx] = xpos;
	getImpl().corneaCenterY[idx] = ypos;
}

void OctGlobal::GlobalRecord::readCorneaTrackAnteriorCurve(std::vector<int>& curve, int lineIdx)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	int idx = lineIdx != 0 ? 1 : lineIdx;
	curve = getImpl().corneaAnteriorCurves[idx];
}

void OctGlobal::GlobalRecord::readCorneaTrackAnteriorCenter(float & xpos, float & ypos, int lineIdx)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	int idx = lineIdx != 0 ? 1 : lineIdx;
	xpos = getImpl().corneaCenterX[idx];
	ypos = getImpl().corneaCenterY[idx];
	return;
}

bool OctGlobal::GlobalRecord::checkIfCorneaTrackTargetFound(bool isset, bool flag, int lineIdx)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	int idx = lineIdx != 0 ? 1 : lineIdx;
	bool found = getImpl().corneaTargetFounds[idx];
	if (isset) {
		getImpl().corneaTargetFounds[idx] = flag;
	}
	return found;
}


void OctGlobal::GlobalRecord::writeRetinaTrackTargetDisplaced(bool flag)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->retinaTargetDisplaced = flag;
	return;
}


void OctGlobal::GlobalRecord::writeRetinaTrackTargetNotFound(bool flag)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->retinaTargetNotFound = flag;
	return;
}


void OctGlobal::GlobalRecord::writeRetinaTrackEyelidCovered(bool flag)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	d_ptr->retinaEyelidCovered = flag;
	return;
}


bool OctGlobal::GlobalRecord::checkIfRetinaTrackTargetDisplaced(bool isset, bool flag)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	bool moved = d_ptr->retinaTargetDisplaced;
	if (isset) {
		d_ptr->retinaTargetDisplaced = flag;
	}
	return moved;
}


bool OctGlobal::GlobalRecord::checkIfRetinaTrackTargetNotFound(bool isset, bool flag)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	bool result = d_ptr->retinaTargetNotFound;
	if (isset) {
		d_ptr->retinaTargetNotFound = flag;
	}
	return result;
}


bool OctGlobal::GlobalRecord::checkIfRetinaTrackEyelidCovered(bool isset, bool flag)
{
	unique_lock<mutex> lock(getImpl().mutexAccess);
	bool moved = d_ptr->retinaEyelidCovered;
	if (isset) {
		d_ptr->retinaEyelidCovered = flag;
	}
	return moved;
}


GlobalRecord::GlobalRecordImpl & OctGlobal::GlobalRecord::getImpl(void)
{
	return *d_ptr;
}
