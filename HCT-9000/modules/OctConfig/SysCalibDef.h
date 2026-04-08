#pragma once

#include <cstdint>

//namespace OctConfig
//{
// #pragma pack(push, 1)

	typedef struct
	{
		std::uint32_t SpectroSN;
		std::uint32_t ColorSensorSN;
		std::int8_t SLD_SN[8];
		std::uint32_t reserved[4];
	} SerialNumber_st;

	typedef struct
	{
		std::int32_t IR_focus_zeroD_pos;
		std::int32_t Scan_focus_zeroD_pos;
		std::int32_t Scan_focus_topography_pos;
		std::int32_t reserved[253]; //254 = Value before adding a topography.
	} DiopterCal_st;

	typedef struct
	{
		double a1;
		double a2;
		double a3;
	} Dispersion_st;

	typedef struct
	{
		double a1;
		double a2;
		double a3;
		double a4;
	} SpectroCal_st;

	typedef struct
	{
		std::uint16_t IM_MAX;
		std::uint16_t IM_MIN;
		std::uint16_t IS_MAX;
		std::uint16_t IS_MIN;
		std::uint16_t DN_REF_EPD_MAX;
		std::uint16_t DN_REF_EPD_MIN;

		std::uint16_t RmonHighCode;
		std::uint16_t RmonLowCode1;
		std::uint16_t RmonLowCode2;
		std::uint16_t RsiCode;
	} SLDparam_st;

	typedef struct
	{
		float Galvano_offset;
		float Galvano_Range;
	} GalvanoCal_st;

	typedef struct
	{
		std::int16_t offsetX;
		std::int16_t offsetY;
	} Galvano_Dynamic_st;

	typedef struct
	{
		std::int32_t duty_base;
		std::int32_t duty_fcw;
		std::int32_t duty_fccw;
		std::int32_t duty_ncw;
		std::int32_t duty_nccw;
		std::int32_t enc_near;
		std::int32_t enc_offs;
	} dcm_setup_st;

	typedef struct
	{
		dcm_setup_st dcm_setup_x;
		dcm_setup_st dcm_setup_z;
	} AutostageCal_st;

	typedef struct
	{
		std::int16_t InPos;
		std::int16_t OutPos;
	} ReturnMirrorPos_st;

	typedef struct
	{
		std::int16_t SmallPupilPos;
		std::int16_t NormalPupilPos;
	} PupilMaskPos_st;

	typedef struct
	{
		std::int16_t MinusLensPos;
		std::int16_t PlusLensPos;
		std::int16_t NoLensPos;
		std::int16_t reserved;
	} DiopterCompLensPos_st;

	typedef struct
	{
		std::uint8_t CorneaAgain;
		std::uint8_t CorneaDgain;
		std::uint8_t RetinaAgain;
		std::uint8_t RetinaDgain;
	} IRCamParam_st;

	typedef struct
	{
		float m;
		float n;
		float a;
		float mlen;
		float flen;
		float diff;
		bool valid;
		std::uint8_t reserved[3];
	} KerSetupData_st;

	typedef struct
	{
		float mm;
		float diopt;
		KerSetupData_st data[11];
	} KerSetupDiopter_st;

	typedef struct
	{
		KerSetupDiopter_st diopt[5];
	} KerSetupTable_st;

	typedef struct
	{
		std::int16_t xoffset;
		std::int16_t yoffset;
	} IntFixationOffset_st;

	typedef struct
	{
		SerialNumber_st SysSerial;
		std::int32_t REF_RetinaPos;
		std::int32_t REF_CorneaPos;
		std::int32_t PolarizationPos;
		DiopterCal_st Diopter_Cal;
		Dispersion_st RetinaDispersion;
		Dispersion_st CorneaDispersion;
		SpectroCal_st SpectroCal;
		GalvanoCal_st Galvano_Xcal;
		GalvanoCal_st Galvano_Ycal;
		SLDparam_st  SLD_Param;
		std::uint16_t SpectroRefProfile[2048];//spectrometer reference level 
		AutostageCal_st AutoStage_Cal;
		ReturnMirrorPos_st QuickReturnMirrorCal;
		ReturnMirrorPos_st SplitFocusMirrorCal;
		PupilMaskPos_st PupilMaskCal;
		DiopterCompLensPos_st FdiopterCompLensPosCal; //Fundus
		DiopterCompLensPos_st SdiopterCompLensPosCal; //Scan
		IRCamParam_st IRCamParam; //2016.01
		
		KerSetupTable_st KerSetup; //2016.11
		IntFixationOffset_st IntFixationOffset; // 2017.5.21
		std::uint64_t chksum; //8 bytes.SysSerial~
	} SysCal_st;

// #pragma pack(pop)
//}

