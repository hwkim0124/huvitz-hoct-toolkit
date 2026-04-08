#include "stdafx.h"
#include "TraceProfile.h"


using namespace OctDevice;
using namespace std;


struct TraceProfile::TraceProfileImpl
{
	float timeStepInUs;
	uint32_t triggerDelay;
	uint16_t numRepeats;
	uint16_t sampleSize;
	uint16_t cameraTriggerCount;
	uint16_t cameraTriggerInterval;
	uint16_t triggerStartIndex;
	int16_t  xPos[TRAJECT_SAMPLE_SIZE_MAX];
	int16_t  yPos[TRAJECT_SAMPLE_SIZE_MAX];

	TraceProfileImpl() : timeStepInUs(TRAJECT_TIME_STEP_IN_US), triggerDelay(TRAJECT_TRIGGER_DELAY),
		numRepeats(TRAJECT_NUMBER_OF_REPEATS), sampleSize(TRAJECT_SAMPLE_SIZE), 
		cameraTriggerCount(TRAJECT_CAMERA_TRIGGER_COUNT), cameraTriggerInterval(TRAJECT_CAMERA_TRIGGER_INTERVAL), 
		triggerStartIndex(TRAJECT_TRIGGER_START_INDEX)
	{
	}
};


TraceProfile::TraceProfile() :
	d_ptr(make_unique<TraceProfileImpl>())
{
}


OctDevice::TraceProfile::~TraceProfile() = default;
OctDevice::TraceProfile::TraceProfile(TraceProfile && rhs) = default;
TraceProfile & OctDevice::TraceProfile::operator=(TraceProfile && rhs) = default;


OctDevice::TraceProfile::TraceProfile(const TraceProfile & rhs)
	: d_ptr(make_unique<TraceProfileImpl>(*rhs.d_ptr))
{
}


TraceProfile & OctDevice::TraceProfile::operator=(const TraceProfile & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctDevice::TraceProfile::setNumberOfRepeats(int repeats)
{
	getImpl().numRepeats = repeats;
	return;
}


void OctDevice::TraceProfile::setTimeStepInUs(float timeStep)
{
	getImpl().timeStepInUs = timeStep;
	return;
}


void OctDevice::TraceProfile::setSampleSize(int samples)
{
	getImpl().sampleSize = samples;
	return;
}


void OctDevice::TraceProfile::setTriggerCount(int count)
{
	getImpl().cameraTriggerCount = count;
	return;
}


void OctDevice::TraceProfile::setTriggerDelay(int delay)
{
	getImpl().triggerDelay = delay;
	return;
}


void OctDevice::TraceProfile::setTriggerStartIndex(int index)
{
	getImpl().triggerStartIndex = index;
	return;
}


int OctDevice::TraceProfile::getTriggerCount(void) const
{
	return getImpl().cameraTriggerCount;
}


int OctDevice::TraceProfile::getSampleSize(void) const
{
	return getImpl().sampleSize;
}


int OctDevice::TraceProfile::getTriggerStartIndex(void) const
{
	return getImpl().triggerStartIndex;
}


short * OctDevice::TraceProfile::getPositionsX(void) const
{
	return getImpl().xPos;
}


short * OctDevice::TraceProfile::getPositionsY(void) const
{
	return getImpl().yPos;
}


void OctDevice::TraceProfile::getTrajectoryProfileParams(TrajectoryProfileParams & params)
{
	params.time_step_us = getImpl().timeStepInUs;
	params.trig_delay = getImpl().triggerDelay;
	params.repeat_num = getImpl().numRepeats;
	params.sample_size = getImpl().sampleSize;
	params.cam_trig_cnt = getImpl().cameraTriggerCount;
	params.cam_trig_itv = getImpl().cameraTriggerInterval;
	params.trig_st_index = getImpl().triggerStartIndex;
	return;
}


TraceProfile::TraceProfileImpl& OctDevice::TraceProfile::getImpl(void) const
{
	return *d_ptr;
}
