#pragma once

#include "SigChainDef.h"

#include <memory>
#include <string>


namespace SigProc
{
	class PhaseCorrector;
	class Resampler;
	class Background;
}


namespace OctConfig 
{
	class SignalSettings;
}


namespace SigChain
{
	class SIGCHAIN_DLL_API ChainSetup
	{
	public:
		ChainSetup();
		virtual ~ChainSetup();

	public:
		static SigProc::Resampler* getResampler(void);
		static SigProc::PhaseCorrector* getPhaseCorrector(void);
		static SigProc::Background* getBackground(void);

		static void initializeChainSetup(void);
		static bool isCorneaScan(void);
		static bool isRetinaScan(void);
		static bool isAngioScan(void);
		static bool isCubeScan(void);

		static void setScanAmplitudesStored(bool flag);
		static bool isScanAmplitudesStored(void) ;

		static void setPatternDomain(PatternDomain domain);
		static void setPatternName(PatternName name);
		static void setPatternSpeed(ScanSpeed speed);
		static bool isSpeedFastest(void);
		static bool isSpeedFaster(void);
		static bool isSpeedNormal(void);

		static bool loadSignalSettings(OctConfig::SignalSettings* sset);
		static bool saveSignalSettings(OctConfig::SignalSettings* sset);

		static void getSpectrometerParameters(double* params);
		static void setSpectrometerParameters(double* params);
		static double getSpectrometerParameter(int index);
		static void setSpectrometerParameter(int index, double value);

		static double getDispersionParameterToRetina(int index);
		static double getDispersionParameterToCornea(int index);
		static void setDispersionParameterToRetina(int index, double value);
		static void setDispersionParameterToCornea(int index, double value);

		static void updateSpectrometerParameters(void);
		static void updateDispersionParameters(double offset1=0.0, double offset2=0.0);
		static float* prepareDispersionCorrection(bool update = false, double offset1 = 0.0, double offset = 0.0);

		static void resetDynamicDispersionCorrection(void);
		static void pauseDynamicDispersionCorrection(bool flag);
		static bool isDynamicDispersionCorrection(void);
		static float* getCurrentPhaseShiftValues(bool recalc = false);
		static double getCurrentPhaseShiftOffset(int index);

		static bool useDynamicDispersionCorrection(bool isSet = false, bool flag = false);
		static bool useBackgroundSubtraction(bool isSet = false, bool flag = false);
		static bool useFixedNoiseReduction(bool isSet = false, bool flag = false);
		static bool useDispersionCompensation(bool isSet = false, bool flag = false);
		static bool useAdaptiveGrayscale(bool isSet = false, bool flag = false);
		static bool useFFTZeroPadding(bool isSet = false, bool flag = false);
		static bool useRefreshDispersionParams(bool isSet = false, bool flag = false);
		static bool useACALinePatternPreview(bool isSet = false, bool flag = false);
		static bool useFFTWindowing(bool isSet = false, bool flag = false);
		static bool useKLinearResampling(bool isSet = false, bool flag = false);
		static bool useWindowingSpectrum(bool isSet = false, bool flag = false);

		static int getFFTImageRowOffset(void);
		static int getMultipleOfZeroPaddingSize(void);
		static void setMultipleOfZeroPaddingSize(int size);
		static float getWindowingFunctionAlpha(void);
		static void setWindowingFunctionAlpha(float alpha = 0.0f);

		static bool usePreviewImageCallback(bool isSet = false, PreviewImageCallback* callback = nullptr);
		static bool usePreviewImageCallback2(bool isSet = false, PreviewImageCallback2* callback = nullptr);
		static bool usePreviewCorneaCallback(bool isSet = false, PreviewImageCallback2* callback = nullptr);

		static bool useEnfaceImageCallback(bool isSet = false, EnfaceImageCallback* callback = nullptr);
		static bool useSpectrumDataCallback(bool isSet = false, SpectrumDataCallback* callback = nullptr);
		static bool useResampleDataCallback(bool isSet = false, ResampleDataCallback* callback = nullptr);
		static bool useIntensityDataCallback(bool isSet = false, IntensityDataCallback* callback = nullptr);

		static PreviewImageCallback* getPreviewImageCallback(void);
		static PreviewImageCallback2* getPreviewImageCallback2(void);
		static PreviewCorneaCallback* getPreviewCorneaCallback(void);

		static EnfaceImageCallback* getEnfaceImageCallback(void);
		static SpectrumDataCallback* getSpectrumDataCallback(void);
		static ResampleDataCallback* getResampleDataCallback(void);
		static IntensityDataCallback* getIntensityDataCallback(void);

		static bool isBackgroundSpectrum(void);
		static bool clearBackgroundSpectrum(void);
		static bool resetBackgroundSpectrum(const unsigned short* data, int width, int height, float quality);
		static unsigned short* getBackgroundSpectrum(void);
		static void setBackgroundSpectrum(const unsigned short* data);

		static float* getResampleFrameData(void);
		static float* getMagnitudeFrameData(void);
		static double getAxialPixelResolution(double refractiveIndex);
		static float* getWindowingFunctionData(void);

	private:
		struct ChainSetupImpl;
		static std::unique_ptr<ChainSetupImpl> d_ptr;
		static ChainSetupImpl& getImpl(void) ;


		/*
		static bool initialize(void);
		static bool initializeOclSystem(void);
		static void release(void);

		static bool loadLineCameraDataFile(const std::wstring& path);

		static unsigned short* getLineBuffer(void) ;
		static int getLineDataCount(void);
		static int getLineTotalSize(void);

		static float* getOutputRealBuffer(void);
		static float* getOutputImagBuffer(void);
		static unsigned char* getImageBuffer(void);

		static int getOutputLineLength(void);
		static int getOutputLineCount(void);
		static void setOutputLineLength(int len);
		static void setOutputLineCount(int count);
		static int getOutputImageWidth(void);
		static int getOutputImageHeight(void);


		static float* getFFTOutBuffer(void);
		static int getFFTOutBuffSize(void);
		static void setFFTOutBuffSize(int size);
		*/

		friend class ImageForm;
	};
}
