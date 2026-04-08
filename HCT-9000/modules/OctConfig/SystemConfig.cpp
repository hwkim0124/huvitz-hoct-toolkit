#include "stdafx.h"
#include "SystemConfig.h"


using namespace OctConfig;
using namespace std;


struct SystemConfig::SystemConfigImpl
{
	SysCal_st sysCal;
	bool modified;

	SystemConfigImpl() : sysCal{ 0 }, modified(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<SystemConfig::SystemConfigImpl> SystemConfig::d_ptr(new SystemConfigImpl());


SystemConfig::SystemConfig()
{
	initialize();
}


SystemConfig::~SystemConfig()
{
}

void OctConfig::SystemConfig::initialize(void)
{
	resetToDefaultValues();
	return;
}

void OctConfig::SystemConfig::resetToDefaultValues(void)
{
	getImpl().sysCal.QuickReturnMirrorCal.InPos = 100;
	getImpl().sysCal.QuickReturnMirrorCal.OutPos = -20;
	getImpl().sysCal.SplitFocusMirrorCal.InPos = 50;
	getImpl().sysCal.SplitFocusMirrorCal.OutPos = -46;
	getImpl().sysCal.PupilMaskCal.NormalPupilPos = 1900;
	getImpl().sysCal.RetinaDispersion.a1 = -7.5f;
	getImpl().sysCal.FdiopterCompLensPosCal.MinusLensPos = 880;
	getImpl().sysCal.FdiopterCompLensPosCal.PlusLensPos = 1540;
	//getImpl().sysCal.SdiopterCompLensPosCal.MinusLensPos = 1030;
	//getImpl().sysCal.SdiopterCompLensPosCal.PlusLensPos = 2030;

	int temp = sizeof(getImpl().sysCal.SysSerial.SLD_SN) / sizeof(int8_t);
	for (int i = 0; i < temp; i++) {
		getImpl().sysCal.SysSerial.SLD_SN[i] = '\0';
	}
	getImpl().sysCal.SysSerial.SLD_SN[0] = 'D';
	return;
}

SysCal_st * OctConfig::SystemConfig::getSysCalibData(bool checksum)
{
	if (checksum) {
		updateCheckSum();
	}
	return &getImpl().sysCal;
}


bool OctConfig::SystemConfig::isModified(void)
{
	return getImpl().modified;
}


void OctConfig::SystemConfig::setModified(bool flag)
{
	getImpl().modified = flag;
	return;
}


bool OctConfig::SystemConfig::isValidChecksum(void)
{
	auto chksum = calculateCheckSum();
	if (chksum != 0 && getCheckSum() == chksum) {
		return true;
	}
	return false;
}


KerSetupDiopter_st* OctConfig::SystemConfig::keratoSetupDiopter(int index)
{
	if (index >= 0 && index < 5) {
		return &getImpl().sysCal.KerSetup.diopt[index];
	}
	return nullptr;
}


KerSetupData_st * OctConfig::SystemConfig::keratoSetupData(int diopt, int index)
{
	if (diopt >= 0 && diopt < 5 && index >= 0 && index < 11) {
		return &getImpl().sysCal.KerSetup.diopt[diopt].data[index];
	}
	return nullptr;
}


std::int32_t OctConfig::SystemConfig::referencePosToRetina(bool isSet, std::int32_t value)
{
	int32_t& data = getImpl().sysCal.REF_RetinaPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int32_t OctConfig::SystemConfig::referencePosToCornea(bool isSet, std::int32_t value)
{
	int32_t& data = getImpl().sysCal.REF_CorneaPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int32_t OctConfig::SystemConfig::polarizationPos(bool isSet, std::int32_t value)
{
	int32_t& data = getImpl().sysCal.PolarizationPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int32_t OctConfig::SystemConfig::zeroDioptPosToIrFocus(bool isSet, std::int32_t value)
{
	int32_t& data = getImpl().sysCal.Diopter_Cal.IR_focus_zeroD_pos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int32_t OctConfig::SystemConfig::zeroDioptPosToScanFocus(bool isSet, std::int32_t value)
{
	int32_t& data = getImpl().sysCal.Diopter_Cal.Scan_focus_zeroD_pos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int32_t OctConfig::SystemConfig::zeroDioptPosToTopographyScanFocus(bool isSet, std::int32_t value)
{
	int32_t& data = getImpl().sysCal.Diopter_Cal.Scan_focus_topography_pos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::double_t OctConfig::SystemConfig::dispersionParameterToRetina(int index, bool isSet, std::double_t value)
{
	double_t* data;
	if (index == 0) {
		data = &getImpl().sysCal.RetinaDispersion.a1;
	}
	else if (index == 1) {
		data = &getImpl().sysCal.RetinaDispersion.a2;
	}
	else {
		data = &getImpl().sysCal.RetinaDispersion.a3;
	}

	if (isSet) {
		*data = value;
	}
	return *data;
}


std::double_t OctConfig::SystemConfig::dispersionParameterToCornea(int index, bool isSet, std::double_t value)
{
	double_t* data;
	if (index == 0) {
		data = &getImpl().sysCal.CorneaDispersion.a1;
	}
	else if (index == 1) {
		data = &getImpl().sysCal.CorneaDispersion.a2;
	}
	else {
		data = &getImpl().sysCal.CorneaDispersion.a3;
	}

	if (isSet) {
		*data = value;
	}
	return *data;
}


std::double_t OctConfig::SystemConfig::spectrometerParameter(int index, bool isSet, std::double_t value)
{
	double_t* data;
	if (index == 0) {
		data = &getImpl().sysCal.SpectroCal.a1;
	}
	else if (index == 1) {
		data = &getImpl().sysCal.SpectroCal.a2;
	}
	else if (index == 2) {
		data = &getImpl().sysCal.SpectroCal.a3;
	}
	else {
		data = &getImpl().sysCal.SpectroCal.a4;
	}

	if (isSet) {
		*data = value;
	}
	return *data;
}


std::uint16_t OctConfig::SystemConfig::sldParameterHighCode(bool isSet, std::uint16_t value)
{
	uint16_t& data = getImpl().sysCal.SLD_Param.RmonHighCode;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint16_t OctConfig::SystemConfig::sldParameterLowCode1(bool isSet, std::uint16_t value)
{
	uint16_t& data = getImpl().sysCal.SLD_Param.RmonLowCode1;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint16_t OctConfig::SystemConfig::sldParameterLowCode2(bool isSet, std::uint16_t value)
{
	uint16_t& data = getImpl().sysCal.SLD_Param.RmonLowCode2;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint16_t OctConfig::SystemConfig::sldParameterRsiCode(bool isSet, std::uint16_t value)
{
	uint16_t& data = getImpl().sysCal.SLD_Param.RsiCode;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::quickReturnMirrorPos(bool isOut, bool isSet, std::int16_t value)
{
	int16_t* data;
	if (isOut) {
		data = &getImpl().sysCal.QuickReturnMirrorCal.OutPos;
	}
	else {
		data = &getImpl().sysCal.QuickReturnMirrorCal.InPos;
	}

	if (isSet) {
		*data = value;
	}
	return *data;
}


std::int16_t OctConfig::SystemConfig::splitFocusMirrorPos(bool isOut, bool isSet, std::int16_t value)
{
	int16_t* data;
	if (isOut) {
		data = &getImpl().sysCal.SplitFocusMirrorCal.OutPos;
	}
	else {
		data = &getImpl().sysCal.SplitFocusMirrorCal.InPos;
	}

	if (isSet) {
		*data = value;
	}
	return *data;
}


std::int16_t OctConfig::SystemConfig::pupilMaskPos(bool isSmall, bool isSet, std::int16_t value)
{
	int16_t* data;
	if (isSmall) {
		data = &getImpl().sysCal.PupilMaskCal.SmallPupilPos;
	}
	else {
		data = &getImpl().sysCal.PupilMaskCal.NormalPupilPos;
	}

	if (isSet) {
		*data = value;
	}
	return *data;
}


std::int16_t OctConfig::SystemConfig::minusLensPosToFundus(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.FdiopterCompLensPosCal.MinusLensPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::plusLensPosToFundus(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.FdiopterCompLensPosCal.PlusLensPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::zeroLensPosToFundus(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.FdiopterCompLensPosCal.NoLensPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::minusLensPosToScan(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.SdiopterCompLensPosCal.MinusLensPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::plusLensPosToScan(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.SdiopterCompLensPosCal.PlusLensPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::zeroLensPosToScan(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.SdiopterCompLensPosCal.NoLensPos;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint8_t OctConfig::SystemConfig::retinaCameraAgain(bool isSet, std::uint8_t value)
{
	uint8_t& data = getImpl().sysCal.IRCamParam.RetinaAgain;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint8_t OctConfig::SystemConfig::retinaCameraDgain(bool isSet, std::uint8_t value)
{
	uint8_t& data = getImpl().sysCal.IRCamParam.RetinaDgain;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint8_t OctConfig::SystemConfig::corneaCameraAgain(bool isSet, std::uint8_t value)
{
	uint8_t& data = getImpl().sysCal.IRCamParam.CorneaAgain;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint8_t OctConfig::SystemConfig::corneaCameraDgain(bool isSet, std::uint8_t value)
{
	uint8_t& data = getImpl().sysCal.IRCamParam.CorneaDgain;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::intFixationOffsetX(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.IntFixationOffset.xoffset;

	if (isSet) {
		data = value;
	}
	return data;
}


std::int16_t OctConfig::SystemConfig::intFixationOffsetY(bool isSet, std::int16_t value)
{
	int16_t& data = getImpl().sysCal.IntFixationOffset.yoffset;

	if (isSet) {
		data = value;
	}
	return data;
}


std::uint64_t OctConfig::SystemConfig::calculateCheckSum(void)
{
	size_t size = sizeof(getImpl().sysCal) - 8;		// except checksum bytes
	uint8_t* ptr = (uint8_t*)&(getImpl().sysCal);

	uint64_t chksum = 0;
	for (int i = 0; i < size; i++) {
		chksum += ptr[i];
	}

	return chksum;
}


std::uint64_t OctConfig::SystemConfig::getCheckSum(void)
{
	return getImpl().sysCal.chksum;
}


void OctConfig::SystemConfig::updateCheckSum(void)
{
	getImpl().sysCal.chksum = calculateCheckSum();
	return;
}


SystemConfig::SystemConfigImpl & OctConfig::SystemConfig::getImpl(void)
{
	return *d_ptr;
}
