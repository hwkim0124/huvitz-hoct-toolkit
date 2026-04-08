#pragma once

namespace OctoPlus_USB_Register_Address
{
#pragma region Factory information
	/* RO
	Manufacturer name (size 64)
	String containing the self-describing name of the
	manufacturer
	default "e2v"
	*/
	const unsigned long cUl_Address_Manufacturer_name = 0x0004;

	/* RO
	Device version (size 64)
	String containing the version of the device
	default -
	*/
	const unsigned long cUl_Address_Device_version = 0x00C4;

	/* RO
	Firmware version (size 16)
	Device firmware version Version of the current package Format: MM.mm.ss
	MM: major number
	mm: minor number
	ss: sub minor number
	MM and/or mm are changed when compatibility is
	broken with previous firmware version. In this
	case, cameras containing previous FW will have to
	be updated by Teledyne e2v - upgrading on field
	will not be possible
	default -
	*/
	const unsigned long cUl_Address_Firmware_version = 0x16040;

	/* KOO */
	const unsigned long cUl_Address_Device_ID = 0x0144;
#pragma endregion

#pragma region Acquisition controls
	/* RW
	Circular Buffer (at software level)
	Enable Circular Buffer mode in SDK (1.1.5 and
	further)
	0: Off
	1: On
	default : 0
	*/
	const unsigned long cUl_Address_Circular_Buffer = 0x4F000018;

	/* RW
	Buffer Type (at software level)
	0: FIFO (First In - First Out)
	1: LIFO (Last In - First Out)
	default : 0
	*/
	const unsigned long cUl_Address_Buffer_Type = 0x4F00001C;
#pragma endregion

#pragma region Image controls
	/* RW
	Image height
	Number of line acquired per image
	default : 4
	*/
	const unsigned long cUl_Address_Image_Height = 0x4F00000C;

	/* RW
	DoDataOutput mode
	2 : Pixel 10-bit
	3 : Pixel 11-bit
	4: Pixel 12-bit
	5: Pixel 11 bit - output in 10 bit
	6: Pixel 12 bit - output in 11 bit
	7: Pixel 12 bit - output in 10 bit
	Note: Pixels value provided by USB3_GetBuffer()
	API is coded on 16bit. 10/11/12 bit are placed on
	the LSB of the 16bits
	default : 4
	*/
	const unsigned long cUl_Address_Output_Mode = 0x12020;

	/* RW
	Reverse reading
	0: disable
	1: enable
	default : 0
	*/
	const unsigned long cUl_Address_Reverse_Reading = 0x12024;

	/* RW
	Metadata enable (see ¡×Appendix for advanced
	description)
	Activate or not the metadata information in
	stream
	0: disable
	1: enable

	default : 1
	*/
	const unsigned long cUl_Address_Metadata_Enable = 0x4F000000;
#pragma	endregion

#pragma region Intergration and trigger controls
	/* RW
	Line period
	Line period from Line period min to 65535
	(655,35us). 1 step = 0.01us
	default : 10000
	*/
	const unsigned long cUl_Address_Line_Period = 0x12100;

	/* RO
	Line period min
	Current line period min
	default : -
	*/
	const unsigned long cUl_Address_Line_Period_Min = 0x12104;

	/* RW
	Exposure time
	Exposure time from Exposure time min to
	Exposure time max. 1 step = 0.01us

	Note that some values are forbidden
	inside the min-max range depending on
	the Line period configured.
	Once this register written, read the register to
	verify the value (it can be adjusted to the nearest
	value available out of the forbidden range)

	default : 8000
	*/
	const unsigned long cUl_Address_Exposure_Time = 0x12108;

	/* RO
	Exposure time min
	Minimum Exposure time configuration on camera
	in current camera configuration, from 1 (0.01us) to
	65535 (655,35us). 1 step = 0.01us

	default : -
	*/
	const unsigned long cUl_Address_Exposure_Time_Min = 0x12114;

	/* RO
	Exposure time max
	Maximum Exposure time configuration on camera
	in current camera configuration, from 1 (0.01us) to
	65535 (655,35us). 1 step = 0.01us

	default : -
	*/
	const unsigned long cUl_Address_Exposure_Time_Max = 0x12118;

	/* RW
	Trigger modes(see ¡×Appendix for advanced
	description)
	0: Internal line trigger with programmable
	exposure time and line period
	1: Internal line trigger with max exposure time and
	programmable line period
	2: External line trigger with programmable
	exposure time
	3: External line trigger with maximum exposure
	time
	4: External mixed line and frame trigger with
	programmable exposure time
	5: External mixed line and frame trigger with
	maximum exposure time
	6: External frame trigger and internal line trigger
	with programmable exposure time and line period
	7: External frame trigger and internal line trigger
	with max exposure time and programmable line
	period

	default : 0
	*/
	const unsigned long cUl_Address_Trigger_Mode = 0x1210C;

	const unsigned long cUl_Address_GPI_Format = 0x12120;

	/* RW
	Max trigger period. Timeout between 2 triggers.
	When reached, USB3 buffers are discarded and
	missing data are filled with 0 at software level.
	Step of 20.48 us. From 0 to
	65535*20.48=1 342 156.8 us

	default : 48828 (999 997.44 us)
	*/
	const unsigned long cUl_Address_Max_Trriger_Period = 0x12210;
#pragma endregion

#pragma region Gain and offset controls
	/* RW
	Analog gain
	Pre-amplifier gain:
	0: x1.0
	1: x2.0
	2: x4.0

	default : 0
	*/
	const unsigned long cUl_Address_Analog_Gain = 0x12200;

	/* RW
	Digital gain
	Digital gain from x1.0 (0) to x7.9961 (1791)
	1 step = x0.0039

	default : 0
	*/
	const unsigned long cUl_Address_Digital_Gain = 0x12204;

	/* RW
	Digital offset
	Digital offset from -4096 to 4095 step 1 in 12 bits
	mode.
	In 11bits mode, offset applied will be divided by 2.
	In 10bits mode, offset applied will be divided by 4.

	default : 0
	*/
	const unsigned long cUl_Address_Digital_Offset = 0x1220C;
#pragma endregion

#pragma region Save and recall parameters controls
	/* RW
	UserSet load
	Restore current user settings from UserSet bank
	number <val>, from 0 to 5.
	0 is factory bank.
	1 is integrator
	2 to 5 are user bank

	default : 0
	*/
	const unsigned long cUl_Address_UsetSet_Load = 0x17000;

	/* WO
	UserSet save
	Save current user settings to UserSet bank number
	<val>, from 1 to 4
	0 is factory bank and cannot be saved
	1 can only be saved in integrator mode
	2 to 5 are user banks

	default : -
	*/
	const unsigned long cUl_Address_UsetSet_Save = 0x17004;

	/* RW
	Restore FFC from bank
	Restore current FFC parameters (including FPN
	and FFCGain) from FFC bank number <val>, from 0
	to 4
	0: Reset FFC coefficients
	1 to 4: Restore user FFC bank

	default : 0
	*/
	const unsigned long cUl_Address_FFC_Restore = 0x17008;

	/* WO
	Save FFC to bank
	Save current FFC parameters (including FPN and
	FFCGain) to FFC bank number <val>, from 1 to 4
	0 cannot be saved
	1 to 4 are user FFC banks

	default : -
	*/
	const unsigned long cUl_Address_FFC_Save = 0x1700C;
#pragma endregion

#pragma region Camera status
	/* RO
	Status
	Camera Status; bit set when :
	Bit0: no trigger during more than 1s
	Bit1: trigger too fast
	Bit4: acquisition is running
	Bit8: overflow occurs during FFC calibration
	Bit9: underflow occurs during FFC calibration
	Bit10: error occurs during FFC calibration
	Bit15: under initialization
	Bit16: hardware error detected

	default : -
	*/
	const unsigned long cUl_Address_Cam_Status = 0x12000;

	/* WO
	Reboot camera
	Reboot the camera with a command
	1: restart the camera (like a power cycle)
	2: restart only camera application (bypass upgrade
	application)

	default : -
	*/
	const unsigned long cUl_Address_Cam_Reboot = 0x18100;
#pragma endregion

#pragma region Save/restore line
#pragma endregion

#pragma region Camera privileges
#pragma endregion


}