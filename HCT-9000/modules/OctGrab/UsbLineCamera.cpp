#include "stdafx.h"
#include "UsbLineCamera.h"

#include "Usb3Grabber.h"

#include "CppUtil2.h"
#include "OctDevice2.h"

#include <atomic>
#include <functional>
#include <mutex>

using namespace OctGrab;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct UsbLineCamera::UsbLineCameraImpl
{
	Usb3Grabber* grabber = NULL;

	UsbLineCameraImpl() {

	}
};


UsbLineCamera::UsbLineCamera() :
	d_ptr(make_unique<UsbLineCameraImpl>())
{
}


OctGrab::UsbLineCamera::~UsbLineCamera()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctGrab::UsbLineCamera::UsbLineCamera(UsbLineCamera && rhs) = default;
UsbLineCamera & OctGrab::UsbLineCamera::operator=(UsbLineCamera && rhs) = default;


bool OctGrab::UsbLineCamera::openLineCamera(Usb3Grabber* grabber)
{
	return false;
}

bool OctGrab::UsbLineCamera::closeLineCamera(void)
{
	return false;
}

bool OctGrab::UsbLineCamera::startAcquisition(bool restart)
{
	return false;
}

bool OctGrab::UsbLineCamera::cancelAcquisition(void)
{
	return false;
}

void OctGrab::UsbLineCamera::setGrabber(Usb3Grabber * grabber)
{
	impl().grabber = grabber;
	return;
}

bool OctGrab::UsbLineCamera::isCameraOpened(void)
{
	return false;
}

bool OctGrab::UsbLineCamera::isCameraGrabbing(void)
{
	return false;
}

bool OctGrab::UsbLineCamera::updateFrameHeight(int height)
{
	return false;
}

bool OctGrab::UsbLineCamera::setExposureToNormal(void)
{
	return false;
}

bool OctGrab::UsbLineCamera::setExposureToFaster(void)
{
	return false;
}

bool OctGrab::UsbLineCamera::setExposureToFastest(void)
{
	return false;
}

float OctGrab::UsbLineCamera::getExposureTime(void)
{
	return 0.0f;
}

float OctGrab::UsbLineCamera::getAnalogGain(void)
{
	return 0.0f;
}

bool OctGrab::UsbLineCamera::setExposureTime(float expTime)
{
	return false;
}

bool OctGrab::UsbLineCamera::setAnalogGain(float gain)
{
	return false;
}

bool OctGrab::UsbLineCamera::acquirePreviewImages(std::vector<int> bufferIds)
{
	return false;
}

bool OctGrab::UsbLineCamera::acquireMeasureImages(std::vector<int> bufferIds)
{
	return false;
}

bool OctGrab::UsbLineCamera::acquireEnfaceImages(std::vector<int> bufferIds)
{
	return false;
}

Usb3Grabber * OctGrab::UsbLineCamera::getGrabber(void) const
{
	return impl().grabber;
}


UsbLineCamera::UsbLineCameraImpl & OctGrab::UsbLineCamera::impl(void) const
{
	return *d_ptr;
}
