#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <string>

namespace OctConfig
{
	struct OCTCONFIG_DLL_API CameraItem : public ConfigSettingItem {
		// The default access specifier for a struct is public. 
		CameraType _type;
		float _analogGain[CAMERA_PRESET_SIZE];
		float _digitalGain[CAMERA_PRESET_SIZE];
		int _shiftX;
		int _shiftY;

	public:
		CameraItem(const char* name, CameraType type) : ConfigSettingItem(name) {
			init(type);
		}

		void init(CameraType type, float again = 0.0f, float dgain = 0.0f, int shiftX = 0, int shiftY = 0) {
			_type = type;
			_shiftX = 0;
			_shiftY = 0;

			for (int i = 0; i < 3; i++) {
				_analogGain[i] = again;
				_digitalGain[i] = dgain;
			}
		}

		float& analogGain(int index = 0) {
			return _analogGain[index];
		}

		float& digitalGain(int index = 0) {
			return _digitalGain[index];
		}

		void setDigitalGain(float gain, int index = 0) {
			if (index >= 0 && index < CAMERA_PRESET_SIZE) {
				_digitalGain[index] = min(max(gain, CAMERA_DGAIN_MIN), CAMERA_DGAIN_MAX);
			}
		}

		void setAnalogGain(float gain, int index = 0) {
			if (index >= 0 && index < CAMERA_PRESET_SIZE) {
				_analogGain[index] = min(max(gain, CAMERA_AGAIN_MIN), CAMERA_AGAIN_MAX);
			}
		}

		int& shiftX(void) {
			return _shiftX;
		}

		int& shiftY(void) {
			return _shiftY;
		}

		CameraType getType(void) const {
			return _type;
		}

		bool isType(CameraType type) const {
			return (_type == type);
		}
	};


	class OCTCONFIG_DLL_API CameraSettings
	{
	public:
		CameraSettings();
		virtual ~CameraSettings();

		CameraSettings(CameraSettings&& rhs);
		CameraSettings& operator=(CameraSettings&& rhs);
		CameraSettings(const CameraSettings& rhs);
		CameraSettings& operator=(const CameraSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		CameraItem* getRetinaCamera(void) const;
		CameraItem* getCorneaCamera(void) const;
		CameraItem* getFundusCamera(void) const;

		CameraItem* getCameraItem(CameraType type) const;
		CameraItem* getCameraItem(const char* name) const;
		CameraItem* getCameraItem(int index) const;
		int getCameraListCount(void) const;

	private:
		struct CameraSettingsImpl;
		std::unique_ptr<CameraSettingsImpl> d_ptr;
		CameraSettingsImpl& getImpl(void) const;
	};
};
