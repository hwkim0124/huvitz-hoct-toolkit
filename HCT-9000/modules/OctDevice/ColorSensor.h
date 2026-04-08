#pragma once

#include "OctDeviceDef.h"
#include "kac12040_def.h"

#include <memory>

namespace OctDevice
{
	class MainBoard;
	class UsbPort;

	class OCTDEVICE_DLL_API ColorSensor
	{
	public:
		ColorSensor();
		ColorSensor(MainBoard *board);
		virtual ~ColorSensor();

	public:
		bool initialize(void);
		void release(void);

		int camRegWrite(unsigned Addr, unsigned data);
		int camRegRead(unsigned Addr);
		int cmdSensorRegRead(unsigned Addr);
		int cmdSensorRegWrite(unsigned Addr, unsigned data);
		int cmdFlushImagePipe(unsigned FlushOption);
		unsigned int calcChkSum(unsigned int *cmd);
		int checkRxPacket(unsigned int *buf, int len);

		int sensorRegRead(unsigned int addr, unsigned int *RdData);
		int fpgaRegRead(unsigned int addr, unsigned int *RdData);
		bool readFrame(unsigned char *data, unsigned int size);

		void cmdUpdateStart(void);
		void cmdUpdateClose(void);

		int cmdFrameRequest(void);
		int cmdSetROI(const CameraROI_t &param);
		int cmdCaptureParam(const CaptureParam_t &param);
		int cmdTrigExposure(const TrigExposureParam_t &param);
		int cmdStrobeParam(const StrbParam_t &param);
		int cmdSensorGain(const SensorGain_t &param);
		int cmdGrabStart(void);
		int cmdGrabStop(void);

		int setSensorMode(const SensorMode_t &param);
		int setSensorConfig1(unsigned config1);
		int setROI(const CameraROI_t &param);

		bool saveDefectPixels(const std::vector<std::pair<unsigned int, unsigned int>>& pixels);
		bool loadDefectPixels(std::vector<std::pair<unsigned int, unsigned int>>& pixels);

	private:
		struct ColorSensorImpl;
		std::unique_ptr<ColorSensorImpl> d_ptr;
		ColorSensorImpl& getImpl(void) const;

		UsbPort& getUsbPort(void) const;
	};
}
