#pragma once

#include "OctDeviceDef.h"
#include "Spinnaker.h"
#include "CppUtil2.h"

#include <memory>


namespace OctConfig {
	class CameraSettings;
}

namespace OctDevice
{
	class MainBoard;
	class ColorSensor;

	class OCTDEVICE_DLL_API ColorCamera
	{
	public:
		ColorCamera(MainBoard *board);
		virtual ~ColorCamera();

	public:
		bool initialize(void);
		bool isInitiated(void) const;

		void setDefaultParameters(bool update = true);
		void setDefaultParametersForSingle();
		void setDefaultParametersForLive();
		void updateParameters(void);

		void setUsingLv4(bool enable);
		bool isUsingLv4() const;

		void setFrameCallback(ColorCameraFrameCallback* callback);
		void setImageCallback(ColorCameraImageCallback* callback);

		void takePicture(bool process = false);
		void startLiveMode(void);
		void startOriginalMode(void);
		void pauseLiveMode(void);
		void pauseOriginalMode(void);
		bool isLiveMode(void) const;
		bool isOriginalMode(void) const;

		bool processImage(void);
		
		bool getSensorID(unsigned int *id);
		int getFrameWidth(void) const;
		int getFrameHeight(void) const;
		int getFrameSizeInBytes(void) const;
		int getTotalFrames(void) const;

		int getImageWidth(void) const;
		int getImageHeight(void) const;

		const std::uint8_t* getFrameBuffer(void) const;
		const std::uint8_t* getImageBuffer(void) const;

		void setROI_X_Start(unsigned int val);
		void setROI_X_Width(unsigned int val);
		void setROI_Y_Start(unsigned int val);
		void setROI_Y_Height(unsigned int val);
		void setROI_DecimationRatio(unsigned int val, bool setIdx = false);
		unsigned int getROI_X_Start(void);
		unsigned int getROI_X_Width(void);
		unsigned int getROI_Y_Start(void);
		unsigned int getROI_Y_Height(void);
		unsigned int getROI_DecimationRatio(bool retIdx = false);

		void setAnalogGain(unsigned int val, bool setIdx = false);
		unsigned int getAnalogGain(bool retIdx = false);
		void setDigitalGain(float val);
		void setDigitalIO(bool flag);
		float getDigitalGain(void);

		void setExposureCount(unsigned int val);
		void setExposureOffCount(unsigned int val);
		unsigned int getExposureCount(void);
		unsigned int getExposureOffCount(void);

		void setStrobeDelay(unsigned int val);
		void setStrobeActTime(unsigned int val);
		void setStrobeMode(unsigned int val);
		unsigned int getStrobeDelay(void);
		unsigned int getStrobeActTime(void);
		unsigned int getStrobeMode(void);

		void setShutterMode(unsigned int val, bool setIdx = false);
		void setFlipMode(unsigned int val, bool setIdx = false);
		void setFrameSize(unsigned int val, bool setIdx = false);
		void setCaptureMode(unsigned int val, bool setIdx = false);
		unsigned int getShutterMode(bool retIdx = false);
		unsigned int getFlipMode(bool retIdx = false);
		unsigned int getFrameSize(bool retIdx = false);
		unsigned int getCaptureMode(bool retIdx = false);

		bool loadConfig(OctConfig::CameraSettings* dset);
		bool saveConfig(OctConfig::CameraSettings* dset);

		bool loadDefectPixels(std::vector<std::pair<unsigned int, unsigned int>> &pixels);
		bool saveDefectPixels(const std::vector<std::pair<unsigned int, unsigned int>> &pixels);
	private:
		struct ColorCameraImpl;
		std::unique_ptr<ColorCameraImpl> d_ptr;
		ColorCameraImpl& getImpl(void) const;

		MainBoard* getMainBoard(void) const;
		ColorSensor& getSensor(void) const;

#ifdef __USE_SPINNAKER__
		Spinnaker::CameraPtr getCamera(void) const;
#endif

		void flushImagePipe(void);
		bool grabStart(void);
		bool grabStop(void);

		void acquireCameraData(void);
		void acquireOriginalData(void);
		std::wstring getTimestampedFilename(const std::wstring & baseName, const std::wstring & extension);
		void processCameraData(int frameCount = 0);
		void processImageLv4(CppUtil::CvImage& image_lv6, float focusValue, int fixation);
		void processImageLv4Optimized(CppUtil::CvImage& image_lv6, float focusValue, int fixation);

		void convertBayer16ToRGB16(unsigned char* data, unsigned int width, unsigned int height);
		bool convertRGB16ToRGB8(void);

	};
}


