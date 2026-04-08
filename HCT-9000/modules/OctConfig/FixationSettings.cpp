#include "stdafx.h"
#include "FixationSettings.h"


using namespace OctConfig;
using namespace std;


struct FixationSettings::FixationSettingsImpl
{
	bool useLcdFix;
	bool useBlinkOn;
	pair<int, int> centerOD;
	pair<int, int> fundusOD;
	pair<int, int> scanDiskOD;
	pair<int, int> leftSideOD[3];
	pair<int, int> rightSideOD[3];

	pair<int, int> centerOS;
	pair<int, int> fundusOS;
	pair<int, int> scanDiskOS;
	pair<int, int> leftSideOS[3];
	pair<int, int> rightSideOS[3];

	int blinkPeriod;
	int blinkOnTime;
	int brightness;
	int fixationType;

	FixationSettingsImpl() {
	}
};


FixationSettings::FixationSettings() :
	d_ptr(make_unique<FixationSettingsImpl>())
{
	initialize();
}


OctConfig::FixationSettings::~FixationSettings() = default;
OctConfig::FixationSettings::FixationSettings(FixationSettings && rhs) = default;
FixationSettings & OctConfig::FixationSettings::operator=(FixationSettings && rhs) = default;


OctConfig::FixationSettings::FixationSettings(const FixationSettings & rhs)
	: d_ptr(make_unique<FixationSettingsImpl>(*rhs.d_ptr))
{
}


FixationSettings & OctConfig::FixationSettings::operator=(const FixationSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::FixationSettings::initialize(void)
{
	resetToDefaultValues();
	return;
}


void OctConfig::FixationSettings::resetToDefaultValues(void)
{
	getImpl().centerOD = std::pair<int, int>(67, 63);
	getImpl().fundusOD = std::pair<int, int>(9, 0);  // std::pair<int, int>(73, 60);
	getImpl().scanDiskOD = std::pair<int, int>(26, -3); // std::pair<int, int>(90, 57);

	getImpl().leftSideOD[0] = std::pair<int, int>(-40, 0);
	getImpl().leftSideOD[1] = std::pair<int, int>(-26, -19);
	getImpl().leftSideOD[2] = std::pair<int, int>(-26, 20);
	getImpl().rightSideOD[0] = std::pair<int, int>(20, 0);
	getImpl().rightSideOD[1] = std::pair<int, int>(6, -19);
	getImpl().rightSideOD[2] = std::pair<int, int>(6, 20);

	getImpl().centerOS = std::pair<int, int>(67, 63);
	getImpl().fundusOS = std::pair<int, int>(-9, 0);  //std::pair<int, int>(55, 60);
	getImpl().scanDiskOS = std::pair<int, int>(-26, -3);  // std::pair<int, int>(38, 57);

	getImpl().leftSideOS[0] = std::pair<int, int>(-20, 0);
	getImpl().leftSideOS[1] = std::pair<int, int>(-6, -19);
	getImpl().leftSideOS[2] = std::pair<int, int>(-6, 20);
	getImpl().rightSideOS[0] = std::pair<int, int>(40, 0);
	getImpl().rightSideOS[1] = std::pair<int, int>(26, -19);
	getImpl().rightSideOS[2] = std::pair<int, int>(26, 20);

	getImpl().useLcdFix = true;
	getImpl().useBlinkOn = false;
	getImpl().blinkOnTime = 4000; // 2500;
	getImpl().blinkPeriod = 5000;
	getImpl().brightness = 60; // 30;
	getImpl().fixationType = 2;// 3;
	return;
}


std::pair<int, int> OctConfig::FixationSettings::getCenterOD(void) const
{
	return getImpl().centerOD;
}


std::pair<int, int> OctConfig::FixationSettings::getFundusOD(void) const
{
	return getImpl().fundusOD;
}


std::pair<int, int> OctConfig::FixationSettings::getScanDiskOD(void) const
{
	return getImpl().scanDiskOD;
}

std::pair<int, int> OctConfig::FixationSettings::getLeftSideOD(int index) const
{
	if (index >= 0 && index < 3) {
		return getImpl().leftSideOD[index];
	}
	return std::pair<int, int>();
}

std::pair<int, int> OctConfig::FixationSettings::getRightSideOD(int index) const
{
	if (index >= 0 && index < 3) {
		return getImpl().rightSideOD[index];
	}
	return std::pair<int, int>();
}


std::pair<int, int> OctConfig::FixationSettings::getCenterOS(void) const
{
	return getImpl().centerOS;
}


std::pair<int, int> OctConfig::FixationSettings::getFundusOS(void) const
{
	return getImpl().fundusOS;
}


std::pair<int, int> OctConfig::FixationSettings::getScanDiskOS(void) const
{
	return getImpl().scanDiskOS;
}

std::pair<int, int> OctConfig::FixationSettings::getLeftSideOS(int index) const
{
	if (index >= 0 && index < 3) {
		return getImpl().leftSideOS[index];
	}
	return std::pair<int, int>();
}

std::pair<int, int> OctConfig::FixationSettings::getRightSideOS(int index) const
{
	if (index >= 0 && index < 3) {
		return getImpl().rightSideOS[index];
	}
	return std::pair<int, int>();
}


int OctConfig::FixationSettings::getBrightness(void) const
{
	return getImpl().brightness;
}


int OctConfig::FixationSettings::getBlinkPeriod(void) const
{
	return getImpl().blinkPeriod;
}


int OctConfig::FixationSettings::getBlinkOnTime(void) const
{
	return getImpl().blinkOnTime;
}

int OctConfig::FixationSettings::getFixationType(void) const
{
	return getImpl().fixationType;
}


void OctConfig::FixationSettings::setCenterOD(std::pair<int, int> pos)
{
	getImpl().centerOD = pos;
	return;
}


void OctConfig::FixationSettings::setFundusOD(std::pair<int, int> pos)
{
	getImpl().fundusOD = pos;
	return;
}


void OctConfig::FixationSettings::setScanDiskOD(std::pair<int, int> pos)
{
	getImpl().scanDiskOD = pos;
	return;
}


void OctConfig::FixationSettings::setLeftSideOD(int index, std::pair<int, int> pos)
{
	if (index >= 0 && index < 3) {
		getImpl().leftSideOD[index] = pos;
	}
	return;
}


void OctConfig::FixationSettings::setRightSideOD(int index, std::pair<int, int> pos)
{
	if (index >= 0 && index < 3) {
		getImpl().rightSideOD[index] = pos;
	}
	return;
}


void OctConfig::FixationSettings::setCenterOS(std::pair<int, int> pos)
{
	getImpl().centerOS = pos;
	return;
}


void OctConfig::FixationSettings::setFundusOS(std::pair<int, int> pos)
{
	getImpl().fundusOS = pos;
	return;
}


void OctConfig::FixationSettings::setScanDiskOS(std::pair<int, int> pos)
{
	getImpl().scanDiskOS = pos;
	return;
}

void OctConfig::FixationSettings::setLeftSideOS(int index, std::pair<int, int> pos)
{
	if (index >= 0 && index < 3) {
		getImpl().leftSideOS[index] = pos;
	}
	return;
}

void OctConfig::FixationSettings::setRightSideOS(int index, std::pair<int, int> pos)
{
	if (index >= 0 && index < 3) {
		getImpl().rightSideOS[index] = pos;
	}
	return;
}


void OctConfig::FixationSettings::setBrightness(int bright)
{
	getImpl().brightness = bright;
}


void OctConfig::FixationSettings::setBlinkPeriod(int period)
{
	getImpl().blinkPeriod = period;
	return;
}


void OctConfig::FixationSettings::setBlinkOnTime(int onTime)
{
	getImpl().blinkOnTime = onTime;
	return;
}


void OctConfig::FixationSettings::setFixationType(int type)
{
	getImpl().fixationType = ((type >= 0 && type <= 4) ? type : 2);
	return;
}


bool OctConfig::FixationSettings::useLcdFixation(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useLcdFix = flag;
	}
	return getImpl().useLcdFix;
}


bool OctConfig::FixationSettings::useLcdBlinkOn(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useBlinkOn = flag;
	}
	return getImpl().useBlinkOn;
}


FixationSettings::FixationSettingsImpl & OctConfig::FixationSettings::getImpl(void) const
{
	return *d_ptr;
}
