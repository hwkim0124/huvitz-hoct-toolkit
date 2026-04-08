#pragma once


#include "OctDeviceDef.h"
#include "BoardDevice.h"

#include <memory>


namespace OctConfig {
	class CameraSettings;
}


namespace CppUtil {
	class CvImage;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API IrCamera : public BoardDevice
	{
	public:
		IrCamera();
		IrCamera(MainBoard* board, CameraType type);
		virtual ~IrCamera();

		IrCamera(IrCamera&& rhs);
		IrCamera& operator=(IrCamera&& rhs);

		// Prevent copy construction and assignment. 
		IrCamera(const IrCamera& rhs) = delete;
		IrCamera& operator=(const IrCamera& rhs) = delete;

	public:
		virtual bool initialize(void);
		bool isInitiated(void) const;

		void setCallback(IrCameraFrameCallback* callback);
		bool isPlaying(void) const;
		bool play(void);
		void pause(void);
		int getErrorCount(void);
		int getFrameCount(void);

		virtual bool setAnalogGain(float gain, bool control = true);
		virtual bool setDigitalGain(float gain, bool control = true);
		virtual float getAnalogGain(void) const;
		virtual float getDigitalGain(void) const;

		bool setExposureTime(bool highSpeed = false);
		bool isHighSpeedMode(void);

		std::uint8_t getAnalogGainData(float gain) const;
		std::uint8_t getDigitalGainData(float gain) const;

		CameraType getType(void) const;
		std::string getCameraName(void) const;
		std::uint8_t* getBuffer(void) const;
		std::uint32_t getFrameWidth(void) const;
		std::uint32_t getFrameHeight(void) const;
		std::uint32_t getFrameSize(void) const;

		bool captureFrame(std::uint8_t* frame) const;
		bool captureFrame(CppUtil::CvImage& image) const;
		bool captureFrame(OctFrameImage& image) const;

		bool loadConfig(OctConfig::CameraSettings* dset);
		bool saveConfig(OctConfig::CameraSettings* dset);

	protected:
		MainBoard* getMainBoard(void) const;

	private:
		struct IrCameraImpl;
		std::unique_ptr<IrCameraImpl> d_ptr;
		IrCameraImpl& getImpl(void) const;
		
		void acquireCameraData(void);
		bool recoverFromUsbError(void);
		bool isValidFrameHeader(unsigned char* buff);
	};
}
