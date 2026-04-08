#include "stdafx.h"
#include "OctoPlusUsb3_Utility.h"
#include "OctoPlusUsb3_RegisterAddress.h"

#include "CamCmosOctUsb3.h"

using namespace OctoPlus_USB_Utility;
using namespace OctoPlus_USB_Register_Address;
using namespace std;


std::string OctoPlus_USB_Utility::getErrorStringFromCode(int code)
{
	char text[512];
	size_t size = sizeof(text);

	if (USB3_GetErrorText(code, text, &size) != CAM_ERR_SUCCESS) {
		text[0] = '\0';
	}
	return string(text);
}

int OctoPlus_USB_Utility::MicroSecToTimeStep(double time_us)
{
	const auto unit = 0.01;
	return (int)(time_us / unit);
}

double OctoPlus_USB_Utility::TimeStepToMicroSec(int time_step)
{
	const auto unit = 0.01;
	return (double)time_step * unit;
}

int OctoPlus_USB_Utility::DigitalGainValueToStep(double value)
{
	const auto unit = 0.0039;
	return (int)(value / unit);
}

double OctoPlus_USB_Utility::DigitalGainStepToValue(int step)
{
	const auto unit = 0.0039;
	return (double)step * unit;
}

std::string OctoPlus_USB_Utility::getStringTriggerMode(ETriggerMode mode)
{
	switch (mode)
	{
	case ETriggerMode::Mode_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod: return "InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod";
	case ETriggerMode::Mode_InternalLineTrigger_MaxExposure_ProgrammableLinePeriod: return "InternalLineTrigger_MaxExposure_ProgrammableLinePeriod";
	case ETriggerMode::Mode_ExternalLineTrigger_ProgrammableExposure: return "ExternalLineTrigger_ProgrammableExposure";
	case ETriggerMode::Mode_ExternalLineTrigger_MaxExposure: return "ExternalLineTrigger_MaxExposure";
	case ETriggerMode::Mode_ExternalMixedLineAndFrameTrigger_ProgrammableExposure: return "ExternalMixedLineAndFrameTrigger_ProgrammableExposure";
	case ETriggerMode::Mode_ExternalMixedLineAndFrameTrigger_MaxExposure: return "ExternalMixedLineAndFrameTrigger_MaxExposure";
	case ETriggerMode::Mode_ExternalFrameTrigger_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod: return "ExternalFrameTrigger_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod";
	case ETriggerMode::Mode_ExternalFrameTrigger_ExternalLineTrigger_MaxExposure_ProgrammableLinePeriod: return "ExternalFrameTrigger_ExternalLineTrigger_MaxExposure_ProgrammableLinePeriod";
	default:
		return STRING_VALUE_UNNOWN;
	}
}

int OctoPlus_USB_Utility::getValueFromTriggerMode(ETriggerMode mode)
{
	switch (mode)
	{
	case ETriggerMode::Mode_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod: return 0;
	case ETriggerMode::Mode_InternalLineTrigger_MaxExposure_ProgrammableLinePeriod: return 1;
	case ETriggerMode::Mode_ExternalLineTrigger_ProgrammableExposure: return 2;
	case ETriggerMode::Mode_ExternalLineTrigger_MaxExposure: return 3;
	case ETriggerMode::Mode_ExternalMixedLineAndFrameTrigger_ProgrammableExposure: return 4;
	case ETriggerMode::Mode_ExternalMixedLineAndFrameTrigger_MaxExposure: return 5;
	case ETriggerMode::Mode_ExternalFrameTrigger_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod: return 6;
	case ETriggerMode::Mode_ExternalFrameTrigger_ExternalLineTrigger_MaxExposure_ProgrammableLinePeriod: return 7;
	default:
		return REGISTER_VALUE_UNKNOWN;
	}
}

ETriggerMode OctoPlus_USB_Utility::getTriggerModeFromValue(int value)
{
	switch (value)
	{
	case 0: return ETriggerMode::Mode_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod;
	case 1: return ETriggerMode::Mode_InternalLineTrigger_MaxExposure_ProgrammableLinePeriod;
	case 2: return ETriggerMode::Mode_ExternalLineTrigger_ProgrammableExposure;
	case 3: return ETriggerMode::Mode_ExternalLineTrigger_MaxExposure;
	case 4: return ETriggerMode::Mode_ExternalMixedLineAndFrameTrigger_ProgrammableExposure;
	case 5: return ETriggerMode::Mode_ExternalMixedLineAndFrameTrigger_MaxExposure;
	case 6: return ETriggerMode::Mode_ExternalFrameTrigger_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod;
	case 7: return ETriggerMode::Mode_ExternalFrameTrigger_ExternalLineTrigger_MaxExposure_ProgrammableLinePeriod;
	default:
		return ETriggerMode::Mode_Trigger_Unknown;
	}
}

std::string OctoPlus_USB_Utility::getStringBufferType(EBufferType type)
{
	switch (type)
	{
	case EBufferType::Type_FIFO: return "FIFO";
	case EBufferType::Type_LIFO: return "LIFO";
	default:
		return STRING_VALUE_UNNOWN;
	}
}

int OctoPlus_USB_Utility::getValueFromBufferType(EBufferType type)
{
	switch (type)
	{
	case EBufferType::Type_FIFO: return 0;
	case EBufferType::Type_LIFO: return 1;
	default:
		return REGISTER_VALUE_UNKNOWN;
	}
}

EBufferType OctoPlus_USB_Utility::getBufferTypeFromValue(int value)
{
	switch (value)
	{
	case 0: return EBufferType::Type_FIFO;
	case 1: return EBufferType::Type_LIFO;
	default:
		return EBufferType::Type_Unknown;
	}
}

std::string OctoPlus_USB_Utility::getStringOutputMode(EOutputMode mode)
{
	switch (mode)
	{
	case EOutputMode::Mode_Pixel_10bit: return "Pixel_10bit";
	case EOutputMode::Mode_Pixel_11bit: return "Pixel_11bit";
	case EOutputMode::Mode_Pixel_12bit: return "Pixel_12bit";
	case EOutputMode::Mode_Pixel_11bit_output_int_10bit: return "Pixel_11bit_output_int_10bit";
	case EOutputMode::Mode_Pixel_12bit_output_int_11bit: return "Pixel_12bit_output_int_11bit";
	case EOutputMode::Mode_Pixel_12bit_output_int_10bit: return "12bit_output_int_10bit";
	default:
		return STRING_VALUE_UNNOWN;
	}
}

int OctoPlus_USB_Utility::getValueFromOutputMode(EOutputMode mode)
{
	switch (mode)
	{
	case EOutputMode::Mode_Pixel_10bit: return 2;
	case EOutputMode::Mode_Pixel_11bit: return 3;
	case EOutputMode::Mode_Pixel_12bit: return 4;
	case EOutputMode::Mode_Pixel_11bit_output_int_10bit: return 5;
	case EOutputMode::Mode_Pixel_12bit_output_int_11bit: return 6;
	case EOutputMode::Mode_Pixel_12bit_output_int_10bit: return 7;
	default:
		return REGISTER_VALUE_UNKNOWN;
	}
}

EOutputMode OctoPlus_USB_Utility::getOutputModeFromValue(int value)
{
	switch (value)
	{
	case 2: return EOutputMode::Mode_Pixel_10bit;
	case 3: return EOutputMode::Mode_Pixel_11bit;
	case 4: return EOutputMode::Mode_Pixel_12bit;
	case 5: return EOutputMode::Mode_Pixel_11bit_output_int_10bit;
	case 6: return EOutputMode::Mode_Pixel_12bit_output_int_11bit;
	case 7: return EOutputMode::Mode_Pixel_12bit_output_int_10bit;
	default:
		return EOutputMode::Mode_Pixel_Unknown;
	}
}

double OctoPlus_USB_Utility::getNumericAnalogGain(EAnalogGain mode)
{
	switch (mode)
	{
	case EAnalogGain::AnalogGain_1_0: return 1.0;
	case EAnalogGain::AnalogGain_2_0: return 2.0;
	case EAnalogGain::AnalogGain_4_0: return 4.0;
	default:
		return 0.0;
	}
}

int OctoPlus_USB_Utility::getValueFromAnalogGain(EAnalogGain mode)
{
	switch (mode)
	{
	case EAnalogGain::AnalogGain_1_0: return 0;
	case EAnalogGain::AnalogGain_2_0: return 1;
	case EAnalogGain::AnalogGain_4_0: return 2;
	default:
		return REGISTER_VALUE_UNKNOWN;
	}
}

EAnalogGain OctoPlus_USB_Utility::getAnalogGainFromValue(int value)
{
	switch (value)
	{
	case 0: return EAnalogGain::AnalogGain_1_0;
	case 1: return EAnalogGain::AnalogGain_2_0;
	case 2: return EAnalogGain::AnalogGain_4_0;
	default:
		return EAnalogGain::AnalogGain_Unknown;
	}
}
