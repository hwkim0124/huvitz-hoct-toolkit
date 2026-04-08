#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace OctGrab {

	constexpr int GRABBER_BUFFER_X_SIZE = 2048;
	constexpr int GRABBER_BUFFER_Y_SIZE_MAX = 2048;

	constexpr int JOYSTICK_PRESSED_EVENT_COUNT = 3;
	constexpr int JOYSTICK_PRESSED_EVENT_DELAY = 5;
	constexpr int GRAB_FIRST_RETRY_COUNT_MAX = 300;
	constexpr int GRAB_FIRST_RETRY_DELAY_TIME = 5;

	constexpr int LEFT_SIDE_IO_VALUE = 0;
	constexpr int RIGHT_SIDE_IO_VALUE = 1;
}


#ifdef __OCTGRAB_DLL
#define OCTGRAB_DLL_API		__declspec(dllexport)
#else
#define OCTGRAB_DLL_API		__declspec(dllimport)
#endif