#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API SystemSettings
	{
	public:
		SystemSettings();
		virtual ~SystemSettings();

		SystemSettings(SystemSettings&& rhs);
		SystemSettings& operator=(SystemSettings&& rhs);
		SystemSettings(const SystemSettings& rhs);
		SystemSettings& operator=(const SystemSettings& rhs);

	public:
		static const int SERIAL_NUMBER_SIZE_MAX = 16;

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		int getModelType(void);
		int getOctGrabberType(void);
		int getOctLineCameraMode(void);
		int getSldFaultDetection(void);
		int getSldFaultTimer(void);
		int getSldFaultThreshold(void);
		int getRetinaTrackingSpeed(void);
		int getTriggerForePadd(int index);
		int getTriggerPostPadd(int index);

		void setModelType(int type);
		void setOctGrabberType(int type);
		void setOctLineCameraMode(int mode);
		void setSldFaultDetection(int value);
		void setSldFaultTimer(int value);
		void setSldFaultThreshold(int value);
		void setRetinaTrackingSpeed(int value);
		void setTriggerForePadd(int index, int value);
		void setTriggerPostPadd(int index, int value);

		int checkMainboardAtStartup(bool isset = false, int value = 0);
		std::string& serialNumber(void);

		void setSerialNumber(std::string serialNo);

	private:
		struct SystemSettingsImpl;
		std::unique_ptr<SystemSettingsImpl> d_ptr;
		SystemSettingsImpl& getImpl(void) const;
	};
}

