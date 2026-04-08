#include "stdafx.h"
#include "SignalSettings.h"


using namespace OctConfig;
using namespace std;


struct SignalSettings::SignalSettingsImpl
{
	double wavelengthParams[WAVELENGTH_PARAMS_SIZE] = { 0.0 };
	double dispersionParams[DISPERSION_PARAMS_SIZE] = { 0.0 };

	SignalSettingsImpl() {
	}
};


SignalSettings::SignalSettings() :
	d_ptr(make_unique<SignalSettingsImpl>())
{
	initialize();
}


OctConfig::SignalSettings::~SignalSettings() = default;
OctConfig::SignalSettings::SignalSettings(SignalSettings && rhs) = default;
SignalSettings & OctConfig::SignalSettings::operator=(SignalSettings && rhs) = default;


OctConfig::SignalSettings::SignalSettings(const SignalSettings & rhs)
	: d_ptr(make_unique<SignalSettingsImpl>(*rhs.d_ptr))
{
}


SignalSettings & OctConfig::SignalSettings::operator=(const SignalSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::SignalSettings::initialize(void)
{
	for (int i = 0; i < WAVELENGTH_PARAMS_SIZE; i++) {
		getImpl().wavelengthParams[i] = 0.0;
	}

	for (int i = 0; i < DISPERSION_PARAMS_SIZE; i++) {
		getImpl().dispersionParams[i] = 0.0;
	}
	return;
}


double OctConfig::SignalSettings::getWavelengthParameter(int index) const
{
	if (index >= 0 && index < WAVELENGTH_PARAMS_SIZE) {
		return getImpl().wavelengthParams[index];
	}
	return 0.0;
}


void OctConfig::SignalSettings::getWavelengthParameter(double * param) const
{
	for (int i = 0; i < WAVELENGTH_PARAMS_SIZE; i++) {
		param[i] = getImpl().wavelengthParams[i];
	}
	return;
}


double OctConfig::SignalSettings::getDispersionParameter(int index) const
{
	if (index >= 0 && index < DISPERSION_PARAMS_SIZE) {
		return getImpl().dispersionParams[index];
	}
	return 0.0;
}


void OctConfig::SignalSettings::getDispersionParameter(double * param) const
{
	for (int i = 0; i < DISPERSION_PARAMS_SIZE; i++) {
		param[i] = getImpl().dispersionParams[i];
	}
	return;
}


void OctConfig::SignalSettings::setWavelengthParameter(int index, double value)
{
	if (index >= 0 && index < WAVELENGTH_PARAMS_SIZE) {
		getImpl().wavelengthParams[index] = value;
	}
	return;
}


void OctConfig::SignalSettings::setDispersionParameter(int index, double value)
{
	if (index >= 0 && index < DISPERSION_PARAMS_SIZE) {
		getImpl().dispersionParams[index] = value;
	}
	return;
}


SignalSettings::SignalSettingsImpl& OctConfig::SignalSettings::getImpl(void) const
{
	return *d_ptr;
}
