#include "stdafx.h"
#include "CorneaTrack.h"

#include <mutex>
#include <atomic>

#include "CppUtil2.h"

using namespace OctSystem;
using namespace CppUtil;
using namespace std;


struct CorneaTrack::CorneaTrackImpl
{
	CorneaTrackImpl() {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<CorneaTrack::CorneaTrackImpl> CorneaTrack::d_ptr(new CorneaTrackImpl());


CorneaTrack::CorneaTrack()
{
}


CorneaTrack::~CorneaTrack()
{
}


bool OctSystem::CorneaTrack::getCorneaCenterOffsetX(float & offsetX, int idxOfImage)
{
	if (GlobalRecord::checkIfCorneaTrackTargetFound(false, false, idxOfImage)) {
		float xpos, ypos;
		GlobalRecord::readCorneaTrackAnteriorCenter(xpos, ypos, idxOfImage);
		offsetX = xpos;
		return true;
	}
	return false;
}


CorneaTrack::CorneaTrackImpl & OctSystem::CorneaTrack::getImpl(void)
{
	return *d_ptr;
}
