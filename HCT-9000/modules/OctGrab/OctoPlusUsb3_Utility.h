#pragma once

#include <string>

namespace OctoPlus_USB_Utility
{
	enum class EBufferType : int
	{
		Type_Unknown = -1,
		Type_FIFO = 0
		, Type_LIFO
	};

	enum class EOutputMode : int
	{
		Mode_Pixel_Unknown = -1,
		Mode_Pixel_10bit = 0
		, Mode_Pixel_11bit
		, Mode_Pixel_12bit
		, Mode_Pixel_11bit_output_int_10bit
		, Mode_Pixel_12bit_output_int_11bit
		, Mode_Pixel_12bit_output_int_10bit
	};

	enum class ETriggerMode : int
	{
		Mode_Trigger_Unknown = -1,
		Mode_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod = 0
		, Mode_InternalLineTrigger_MaxExposure_ProgrammableLinePeriod
		, Mode_ExternalLineTrigger_ProgrammableExposure
		, Mode_ExternalLineTrigger_MaxExposure
		, Mode_ExternalMixedLineAndFrameTrigger_ProgrammableExposure
		, Mode_ExternalMixedLineAndFrameTrigger_MaxExposure
		, Mode_ExternalFrameTrigger_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod
		, Mode_ExternalFrameTrigger_ExternalLineTrigger_MaxExposure_ProgrammableLinePeriod
	};

	enum class EAnalogGain : int
	{
		AnalogGain_Unknown = -1,
		AnalogGain_1_0 = 0
		, AnalogGain_2_0
		, AnalogGain_4_0
	};

	constexpr int REGISTER_VALUE_UNKNOWN = -1;
	const std::string STRING_VALUE_UNNOWN = "";

	std::string getErrorStringFromCode(int code);

	int MicroSecToTimeStep(double time_us);
	double TimeStepToMicroSec(int time_step);
	int DigitalGainValueToStep(double value);
	double DigitalGainStepToValue(int step);

	std::string getStringTriggerMode(ETriggerMode mode);
	int getValueFromTriggerMode(ETriggerMode mode);
	ETriggerMode getTriggerModeFromValue(int value);

	std::string getStringBufferType(EBufferType type);
	int getValueFromBufferType(EBufferType type);
	EBufferType getBufferTypeFromValue(int value);

	std::string getStringOutputMode(EOutputMode mode);
	int getValueFromOutputMode(EOutputMode mode);
	EOutputMode getOutputModeFromValue(int value);

	double getNumericAnalogGain(EAnalogGain mode);
	int getValueFromAnalogGain(EAnalogGain mode);
	EAnalogGain getAnalogGainFromValue(int value);
}