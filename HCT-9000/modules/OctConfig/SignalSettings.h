#pragma once

#include "OctConfigDef.h"

#include <memory>


namespace OctConfig
{
	class OCTCONFIG_DLL_API SignalSettings
	{
	public:
		SignalSettings();
		virtual ~SignalSettings();

		SignalSettings(SignalSettings&& rhs);
		SignalSettings& operator=(SignalSettings&& rhs);
		SignalSettings(const SignalSettings& rhs);
		SignalSettings& operator=(const SignalSettings& rhs);

	public:
		void initialize(void);

		double getWavelengthParameter(int index) const;
		void getWavelengthParameter(double* param) const;
		double getDispersionParameter(int index) const;
		void getDispersionParameter(double* param) const;
		void setWavelengthParameter(int index, double value);
		void setDispersionParameter(int index, double value);


	protected:
		struct SignalSettingsImpl;
		std::unique_ptr<SignalSettingsImpl> d_ptr;
		SignalSettingsImpl& getImpl(void) const;
	};
}
