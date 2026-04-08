#pragma once

#include "OctGrabDef.h"
#include "OctoPlusUsb3_Utility.h"

#include <memory>
#include <string>
#include <vector>


namespace OctDevice
{
	class MainBoard;
}


namespace OctGrab
{
	class Usb3Grabber;

	using namespace OctoPlus_USB_Utility;

	class OCTGRAB_DLL_API OctoPlus
	{
	public:
		OctoPlus();
		virtual ~OctoPlus();

	public: 
		static bool initializeLibrary(void);
		static void releaseLibrary(void);
		static bool isLibraryInitialized(void);

		static void setupUsb3Grabber(Usb3Grabber* grabber);
		static bool openLineCamera(void);
		static void closeLineCamera(void);
		static bool isLineCameraOpened(void);
		static bool isLineCameraGrabbing(void);

		static bool startAcquisition(bool restart=false);
		static void cancelAcquisition(void);

		static bool acquireLineCameraImage(int count);
		static bool acquirePreviewImages(std::vector<int> bufferIds);
		static bool acquireMeasureImages(std::vector<int> bufferIds);
		static bool acquireEnfaceImages(std::vector<int> bufferIds);

		static bool startAcquisition2(unsigned long imagesToAcquire);
		static void cancelAcquisition2(void);
		static bool isLineCameraWorking(void);
		
		static int getImageHeight(void);
		static void setImageHeight(size_t height);
		static void setNumberOfBuffers(size_t number);
		static bool setImageParameters(int imageHeight, int numOfBuffer);
		static bool updateImageParameters(int imageHeight, int numBuffer=0);

		static bool setTriggerMode(ETriggerMode mode);
		static bool getTriggerMode(ETriggerMode* mode);
		static bool setBufferType(EBufferType type);
		static bool getBufferType(EBufferType* type);
		static bool setOutputMode(EOutputMode mode);
		static bool getOutputMode(EOutputMode* mode);

		static std::string getTriggerMode(void);
		static std::string getBufferType(void);
		static std::string getOutputMode(void);
		
		static bool setEnableCircularBuffer(bool enable);
		static bool isEnabledCircularBuffer(void);

		static bool setLinePeriod(double time_us);
		static bool getLinePeriod(double* time_us);
		static bool getLinePeriodMin(double* time_us);
		static bool setExposureTime(double time_us);
		static bool getExposureTime(double* time_us);
		static bool getExposureTimeMin(double* time_us);
		static bool getExposureTimeMax(double* time_us);

		static bool setAnalogGain(EAnalogGain gain);
		static bool getAnalogGain(double* gain);
		static bool setDigitalGain(double gain);
		static bool getDigitalGain(double* gain);

		static bool writeRegister(unsigned long address, int value);
		static bool readRegister(unsigned long address, int* value);
		static std::string getErrorText(int errCode);

	private:
		struct OctoPlusImpl;
		static std::unique_ptr<OctoPlusImpl> d_ptr;
		static OctoPlusImpl& getImpl(void);

		static void threadLineCameraImage(const OctoPlus::OctoPlusImpl* impl);
	};
}
