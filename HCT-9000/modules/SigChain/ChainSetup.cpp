#include "stdafx.h"
#include "ChainSetup.h"
#include "Resampler.h"
#include "AngioSetup.h"

#include "CppUtil2.h"
#include "SigProc2.h"
#include "OctConfig2.h"

/*
#include "CL\cl.h"
#include "openCL.misc.h"
#include "clFFT.h"
*/


#include <vector>
#include <mutex>

using namespace OctConfig;
using namespace SigChain;
using namespace SigProc;
using namespace CppUtil;
using namespace std;


struct ChainSetup::ChainSetupImpl
{
	Resampler resampler;
	PhaseCorrector phaseCorr;
	Background background;

	OctScanPattern pattern;
	bool isScanAmplitudesStored = false;

	vector<float> resampleFrameData;
	vector<float> magnitudeFrameData;
	vector<float> windowingFunctionData;

	bool useBackgroundSubtraction;
	bool useFixedNoiseReduction;
	bool useDispersionCompensation;
	bool useAdaptiveGrayscale;
	bool useFFTZeroPadding;
	bool useRefreshDispersionParams;
	bool useACALinePatternPreview;
	bool useFFTWindowing;
	bool useKLinearResampling;
	bool useWindowingSpectrum;

	int multipleOfZeroPadding;
	float windowingAlpha = 0.5f;

	PreviewImageCallback* cbPreviewImage;
	PreviewImageCallback2* cbPreviewImage2;
	PreviewCorneaCallback* cbPreviewCornea;

	SpectrumDataCallback* cbSpectrumData;
	ResampleDataCallback* cbResampleData;
	IntensityDataCallback* cbIntensityData;
	EnfaceImageCallback* cbEnfaceImage;

	mutex mutexRegist;

	ChainSetupImpl()  
		:	useBackgroundSubtraction(true), useFixedNoiseReduction(true), useDispersionCompensation(true), 
			useAdaptiveGrayscale(true), useFFTZeroPadding(false), multipleOfZeroPadding(4), useFFTWindowing(false),
			useRefreshDispersionParams(false), useACALinePatternPreview(false), useKLinearResampling(true),
			useWindowingSpectrum(false),
			cbPreviewImage(nullptr), cbPreviewImage2(nullptr), cbPreviewCornea(nullptr),
			cbSpectrumData(nullptr), cbResampleData(nullptr), cbIntensityData(nullptr),
			cbEnfaceImage(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ChainSetup::ChainSetupImpl> ChainSetup::d_ptr(new ChainSetupImpl());


ChainSetup::ChainSetup()
{
	// initialize();
}


ChainSetup::~ChainSetup()
{
}


Resampler * SigChain::ChainSetup::getResampler(void)
{
	return &getImpl().resampler;
}


PhaseCorrector * SigChain::ChainSetup::getPhaseCorrector(void)
{
	return &getImpl().phaseCorr;
}


SigProc::Background * SigChain::ChainSetup::getBackground(void)
{
	return &getImpl().background;
}


void SigChain::ChainSetup::initializeChainSetup(void)
{
	double coeffs[WAVELENGTH_FUNCTION_COEFFS] = { 0.0 };
	coeffs[0] = RESAMPLE_INIT_PARAM1;
	coeffs[1] = RESAMPLE_INIT_PARAM2;
	coeffs[2] = RESAMPLE_INIT_PARAM3;
	coeffs[3] = RESAMPLE_INIT_PARAM4;
	getResampler()->setWavelengthFunctionCoefficients(coeffs);

	getPhaseCorrector()->setWavenumbers(getResampler()->getkValueLinearized());

	double param1 = PHASE_SHIFT_INIT_PARAM1;
	double param2 = PHASE_SHIFT_INIT_PARAM2;
	double param3 = PHASE_SHIFT_INIT_PARAM3;
	getPhaseCorrector()->setPhaseShiftParameterToRetina(0, param1);
	getPhaseCorrector()->setPhaseShiftParameterToRetina(1, param2);
	getPhaseCorrector()->setPhaseShiftParameterToRetina(2, param3);

	getPhaseCorrector()->setPhaseParameterOffsetToRetina(0, 0.0);
	getPhaseCorrector()->setPhaseParameterOffsetToRetina(1, 0.0);
	getPhaseCorrector()->setPhaseParameterOffsetToRetina(2, 0.0);

	getPhaseCorrector()->setPhaseShiftParameterToCornea(0, param1);
	getPhaseCorrector()->setPhaseShiftParameterToCornea(1, param2);
	getPhaseCorrector()->setPhaseShiftParameterToCornea(2, param3);

	getPhaseCorrector()->setPhaseParameterOffsetToCornea(0, 0.0);
	getPhaseCorrector()->setPhaseParameterOffsetToCornea(1, 0.0);
	getPhaseCorrector()->setPhaseParameterOffsetToCornea(2, 0.0);

	getPhaseCorrector()->resetPhaseShiftOptimizer();
	getPhaseCorrector()->enablePhaseShiftOptimizer(false);
	return ;
}


bool SigChain::ChainSetup::isCorneaScan(void)
{
	return (getImpl().pattern.isCorneaScan());
}


bool SigChain::ChainSetup::isRetinaScan(void)
{
	return (!getImpl().pattern.isCorneaScan());
}


bool SigChain::ChainSetup::isAngioScan(void)
{
	return getImpl().pattern.isAngioScan();
}


bool SigChain::ChainSetup::isCubeScan(void)
{
	return getImpl().pattern.isCubeScan();
}

void SigChain::ChainSetup::setScanAmplitudesStored(bool flag)
{
	getImpl().isScanAmplitudesStored = flag;
	return;
}


bool SigChain::ChainSetup::isScanAmplitudesStored(void)
{
	return getImpl().isScanAmplitudesStored;
}


void SigChain::ChainSetup::setPatternDomain(PatternDomain domain)
{
	getImpl().pattern.setDomain(domain);
	return;
}


void SigChain::ChainSetup::setPatternName(PatternName name)
{
	getImpl().pattern.setPatternName(name);
	return;
}


void SigChain::ChainSetup::setPatternSpeed(ScanSpeed speed)
{
	getImpl().pattern.setScanSpeed(speed);
	return;
}


bool SigChain::ChainSetup::isSpeedFastest(void)
{
	return (getImpl().pattern.isFastestSpeed());
}


bool SigChain::ChainSetup::isSpeedFaster(void)
{
	return (getImpl().pattern.isFasterSpeed());
}


bool SigChain::ChainSetup::isSpeedNormal(void)
{
	return (getImpl().pattern.isNormalSpeed());
}


bool SigChain::ChainSetup::loadSignalSettings(OctConfig::SignalSettings * sset)
{
	double coeffs[WAVELENGTH_FUNCTION_COEFFS] = { 0.0 };
	for (int i = 0; i < WAVELENGTH_FUNCTION_COEFFS; i++) {
		coeffs[i] = sset->getWavelengthParameter(i);
	}
	getResampler()->setWavelengthFunctionCoefficients(coeffs);

	getPhaseCorrector()->setWavenumbers(getResampler()->getkValueLinearized());

	double param1 = sset->getDispersionParameter(0);
	double param2 = sset->getDispersionParameter(1);
	double param3 = sset->getDispersionParameter(2);

	getPhaseCorrector()->setPhaseShiftParameterToRetina(0, param1);
	getPhaseCorrector()->setPhaseShiftParameterToRetina(1, param2);
	getPhaseCorrector()->setPhaseShiftParameterToRetina(2, param3);

	getPhaseCorrector()->setPhaseShiftParameterToCornea(0, param1);
	getPhaseCorrector()->setPhaseShiftParameterToCornea(1, param2);
	getPhaseCorrector()->setPhaseShiftParameterToCornea(2, param3);
	return true;
}


bool SigChain::ChainSetup::saveSignalSettings(OctConfig::SignalSettings * sset)
{
	Resampler* resampler = getResampler();
	PhaseCorrector* phaseCorr = getPhaseCorrector();

	for (int i = 0; i < WAVELENGTH_FUNCTION_COEFFS; i++) {
		sset->setWavelengthParameter(i, resampler->getWavelengthFunctionCoefficient(i));
	}

	for (int i = 0; i < PHASE_SHIFT_PARAMETER_SIZE; i++) {
		sset->setDispersionParameter(i, phaseCorr->getPhaseShiftParameterToRetina(i));
	}
	return true;
}


void SigChain::ChainSetup::getSpectrometerParameters(double * params)
{
	for (int i = 0; i < WAVELENGTH_FUNCTION_COEFFS; i++) {
		params[i] = getResampler()->getWavelengthFunctionCoefficient(i);
	}
	return;
}


void SigChain::ChainSetup::setSpectrometerParameters(double * params)
{
	getResampler()->setWavelengthFunctionCoefficients(params);
	return;
}


double SigChain::ChainSetup::getSpectrometerParameter(int index)
{
	return getResampler()->getWavelengthFunctionCoefficient(index);
}


void SigChain::ChainSetup::setSpectrometerParameter(int index, double value)
{
	getResampler()->setWavelengthFunctionCoefficient(index, value);

}


double SigChain::ChainSetup::getDispersionParameterToRetina(int index)
{
	return getPhaseCorrector()->getPhaseShiftParameterToRetina(index);
}


double SigChain::ChainSetup::getDispersionParameterToCornea(int index)
{
	return getPhaseCorrector()->getPhaseShiftParameterToCornea(index);
}


void SigChain::ChainSetup::setDispersionParameterToRetina(int index, double value)
{
	getPhaseCorrector()->setPhaseShiftParameterToRetina(index, value);
	return;
}


void SigChain::ChainSetup::setDispersionParameterToCornea(int index, double value)
{
	getPhaseCorrector()->setPhaseShiftParameterToCornea(index, value);
	return;
}


void SigChain::ChainSetup::updateSpectrometerParameters(void)
{
	double coeffs[WAVELENGTH_FUNCTION_COEFFS] = { 0.0 };
	coeffs[0] = SystemConfig::spectrometerParameter(0);
	coeffs[1] = SystemConfig::spectrometerParameter(1);
	coeffs[2] = SystemConfig::spectrometerParameter(2);
	coeffs[3] = SystemConfig::spectrometerParameter(3);
	getResampler()->setWavelengthFunctionCoefficients(coeffs);

	LogD() << "Spectrometer params: " << coeffs[0] << ", " << coeffs[1] << ", " << coeffs[2] << ", " << coeffs[3];

	getPhaseCorrector()->setWavenumbers(getResampler()->getkValueLinearized());
	return;
}


void SigChain::ChainSetup::updateDispersionParameters(double offset1, double offset2)
{
	bool ediScanning = AngioSetup::isEdiScanningMode();
	double signVal = ediScanning ? -1.0 : +1.0;

	if (isRetinaScan()) {
		double param1 = SystemConfig::dispersionParameterToRetina(0) * signVal;
		double param2 = SystemConfig::dispersionParameterToRetina(1) * signVal;
		double param3 = SystemConfig::dispersionParameterToRetina(2) * signVal;
		getPhaseCorrector()->setPhaseShiftParameterToRetina(0, param1);
		getPhaseCorrector()->setPhaseShiftParameterToRetina(1, param2);
		getPhaseCorrector()->setPhaseShiftParameterToRetina(2, param3);
		
		getPhaseCorrector()->clearPhaseParameterOffsetsOfRetina();
		getPhaseCorrector()->setPhaseParameterOffsetToRetina(0, offset1);
		getPhaseCorrector()->setPhaseParameterOffsetToRetina(1, offset2);
		getPhaseCorrector()->updatePhaseShiftValuesToRetina();

		LogD() << "Dispersion params for retina: " << param1 << ", " << param2 << ", " << param3 << ", EDI mode: " << ediScanning;
		LogD() << "Dispersion offsets: " << offset1 << ", " << offset2;
	}
	else {
		double param1 = SystemConfig::dispersionParameterToCornea(0);
		double param2 = SystemConfig::dispersionParameterToCornea(1);
		double param3 = SystemConfig::dispersionParameterToCornea(2);
		getPhaseCorrector()->setPhaseShiftParameterToCornea(0, param1);
		getPhaseCorrector()->setPhaseShiftParameterToCornea(1, param2);
		getPhaseCorrector()->setPhaseShiftParameterToCornea(2, param3);

		getPhaseCorrector()->clearPhaseParameterOffsetsOfCornea();
		getPhaseCorrector()->setPhaseParameterOffsetToCornea(0, offset1);
		getPhaseCorrector()->setPhaseParameterOffsetToCornea(1, offset2);
		getPhaseCorrector()->updatePhaseShiftValuesToCornea();

		LogD() << "Dispersion params for cornea: " << param1 << ", " << param2 << ", " << param3;
		LogD() << "Dispersion offsets: " << offset1 << ", " << offset2;
	}
	return;
}


float* SigChain::ChainSetup::prepareDispersionCorrection(bool update, double offset1, double offset2)
{
	Resampler* resampler = getResampler();
	PhaseCorrector* phaseCorr = getPhaseCorrector();

	// Spectrometer calibration has to be preceded for linearized k values. 
	// phaseCorr->setCenterOfWavelengths(WAVELENGTH_CENTER_OF_SOURCE);
	// phaseCorr->setWavenumbers(resampler->getkValueLinearized());
	bool refresh = useRefreshDispersionParams();

	if (refresh) {
		updateDispersionParameters();
	}
	else if (update) {
		updateDispersionParameters(offset1, offset2);
	}

	auto values = getCurrentPhaseShiftValues(false);
	return values;
}


float * SigChain::ChainSetup::getCurrentPhaseShiftValues(bool recalc)
{
	PhaseCorrector* phaseCorr = getPhaseCorrector();

	// Update the amounts of phase shift proportional to parameters along k values. 
	if (isCorneaScan()) {
		return phaseCorr->getPhaseShiftValuesToCornea(recalc);
	}
	else {
		return phaseCorr->getPhaseShiftValuesToRetina(recalc);
	}
}

double SigChain::ChainSetup::getCurrentPhaseShiftOffset(int index)
{
	auto offset = 0.0;
	if (isCorneaScan()) {
		offset = getPhaseCorrector()->getPhaseParameterOffsetToCornea(index);
	}
	else {
		offset = getPhaseCorrector()->getPhaseParameterOffsetToRetina(index);
	}
	return offset;
}

void SigChain::ChainSetup::resetDynamicDispersionCorrection(void)
{
	getPhaseCorrector()->resetPhaseShiftOptimizer();
	return;
}


void SigChain::ChainSetup::pauseDynamicDispersionCorrection(bool flag)
{
	getPhaseCorrector()->pausePhaseShiftOptimizer(flag);
	return;
}

bool SigChain::ChainSetup::isDynamicDispersionCorrection(void)
{
	return getPhaseCorrector()->isPhaseShiftOptimizing();
}


bool SigChain::ChainSetup::isBackgroundSpectrum(void)
{
	return !getBackground()->isEmpty();
}


bool SigChain::ChainSetup::clearBackgroundSpectrum(void)
{
	getBackground()->setEmpty(true);
	return true;
}


bool SigChain::ChainSetup::resetBackgroundSpectrum(const unsigned short * data, int width, int height, float quality)
{
	// if (quality <= 0.5f) {
	if (true) {
		getBackground()->makeBackgroundSpectrum(data, width, height);
		getBackground()->dumpToFile();
		return true;
	}
	return false;
}


unsigned short * SigChain::ChainSetup::getBackgroundSpectrum(void)
{
	return getBackground()->getProfileData();
}


void SigChain::ChainSetup::setBackgroundSpectrum(const unsigned short * data)
{
	getBackground()->setProfileData(data);
	return;
}


float * SigChain::ChainSetup::getResampleFrameData(void)
{
	if (getImpl().resampleFrameData.empty()) {
		getImpl().resampleFrameData = vector<float>(FRAME_DATA_BUFFER_SIZE, 0.0f);
	}
	return &getImpl().resampleFrameData[0];
}


float * SigChain::ChainSetup::getMagnitudeFrameData(void)
{
	if (getImpl().magnitudeFrameData.empty()) {
		getImpl().magnitudeFrameData = vector<float>(FRAME_DATA_BUFFER_SIZE, 0.0f);
	}
	return &getImpl().magnitudeFrameData[0];
}


double SigChain::ChainSetup::getAxialPixelResolution(double refractiveIndex)
{
	double resol = getResampler()->getPixelResolution(refractiveIndex);

	return resol;
}

float* SigChain::ChainSetup::getWindowingFunctionData(void)
{
	float alpha = getImpl().windowingAlpha;
	int size = LINE_CAMERA_CCD_PIXELS;
	if (getImpl().windowingFunctionData.size() != size) {
		getImpl().windowingFunctionData = vector<float>(size, 1.0f);
	}

	auto window = &getImpl().windowingFunctionData[0];
	auto N = static_cast<int>(size);

	if (alpha <= 0.0f) {
		// Rectangular window
		for (int i = 0; i < N; ++i) {
			window[i] = 1.0f;
		}
	}
	else if (alpha >= 1.0f) {
		// Hann window
		for (int i = 0; i < N; ++i) {
			window[i] = float(0.5f * (1.0f - std::cos(2.0f * M_PI * i / (N - 1))));
		}
	}
	else {
		int taper = static_cast<int>(alpha * (N - 1) / 2.0f);

		for (int i = 0; i < N; ++i) {
			if (i < taper) {
				// Left taper (cosine rise)
				float ratio = static_cast<float>(i) / taper;
				window[i] = float(0.5f * (1.0f + std::cos(M_PI * (ratio - 1.0f))));
			}
			else if (i >= N - taper) {
				// Right taper (cosine fall)
				float ratio = static_cast<float>(N - 1 - i) / taper;
				window[i] = float(0.5f * (1.0f + std::cos(M_PI * (ratio - 1.0f))));
			}
			else {
				// Flat section
				window[i] = 1.0f;
			}
		}
	}
	return window;
}

bool SigChain::ChainSetup::useDynamicDispersionCorrection(bool isSet, bool flag)
{
	if (isSet) {
		getPhaseCorrector()->enablePhaseShiftOptimizer(flag);
	}

	return getPhaseCorrector()->isPhaseShiftOptimzerEnabled();
}

bool SigChain::ChainSetup::useBackgroundSubtraction(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useBackgroundSubtraction = flag;
	}
	return getImpl().useBackgroundSubtraction;
}


bool SigChain::ChainSetup::useFixedNoiseReduction(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useFixedNoiseReduction = flag;
	}
	return getImpl().useFixedNoiseReduction;
}


bool SigChain::ChainSetup::useDispersionCompensation(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useDispersionCompensation = flag;
	}
	return getImpl().useDispersionCompensation;
}


bool SigChain::ChainSetup::useAdaptiveGrayscale(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useAdaptiveGrayscale = flag;
	}
	return getImpl().useAdaptiveGrayscale;
}


bool SigChain::ChainSetup::useFFTZeroPadding(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useFFTZeroPadding = flag;
	}
	return getImpl().useFFTZeroPadding;
}


bool SigChain::ChainSetup::useRefreshDispersionParams(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useRefreshDispersionParams = flag;
	}
	return getImpl().useRefreshDispersionParams;
}


bool SigChain::ChainSetup::useACALinePatternPreview(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useACALinePatternPreview = flag;
	}
	return getImpl().useACALinePatternPreview;
}


bool SigChain::ChainSetup::useFFTWindowing(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useFFTWindowing = flag;
	}
	return getImpl().useFFTWindowing;
}

bool SigChain::ChainSetup::useKLinearResampling(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useKLinearResampling = flag;
	}
	return getImpl().useKLinearResampling;
}

bool SigChain::ChainSetup::useWindowingSpectrum(bool isSet, bool flag)
{
	if (isSet) {
		getImpl().useWindowingSpectrum = flag;
	}
	return getImpl().useWindowingSpectrum;
}

int SigChain::ChainSetup::getFFTImageRowOffset(void)
{
	/*
	if (useACALinePatternPreview()) {
		return FFT_IMAGE_ROW_OFFSET_FOR_ACA_LINE_PREVIEW;
	}
	else {
		return FFT_IMAGE_ROW_OFFSET;
	}
	*/
	return FFT_IMAGE_ROW_OFFSET;
}


int SigChain::ChainSetup::getMultipleOfZeroPaddingSize(void)
{
	return getImpl().multipleOfZeroPadding;
}


void SigChain::ChainSetup::setMultipleOfZeroPaddingSize(int size)
{
	getImpl().multipleOfZeroPadding = size;
	return;
}

float SigChain::ChainSetup::getWindowingFunctionAlpha(void)
{
	return getImpl().windowingAlpha;
}

void SigChain::ChainSetup::setWindowingFunctionAlpha(float alpha)
{
	getImpl().windowingAlpha = alpha;
	return;
}

bool SigChain::ChainSetup::usePreviewImageCallback(bool isSet, PreviewImageCallback * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbPreviewImage = callback;
	}
	return (getImpl().cbPreviewImage != nullptr);
}


bool SigChain::ChainSetup::usePreviewImageCallback2(bool isSet, PreviewImageCallback2 * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbPreviewImage2 = callback;
	}
	return (getImpl().cbPreviewImage2 != nullptr);
}


bool SigChain::ChainSetup::usePreviewCorneaCallback(bool isSet, PreviewImageCallback2 * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbPreviewCornea = callback;
	}
	return (getImpl().cbPreviewCornea != nullptr);
}


bool SigChain::ChainSetup::useEnfaceImageCallback(bool isSet, EnfaceImageCallback * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbEnfaceImage = callback;
	}
	return (getImpl().cbEnfaceImage != nullptr);
}


bool SigChain::ChainSetup::useSpectrumDataCallback(bool isSet, SpectrumDataCallback * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbSpectrumData = callback;
	}
	return (getImpl().cbSpectrumData != nullptr);
}


bool SigChain::ChainSetup::useResampleDataCallback(bool isSet, ResampleDataCallback * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbResampleData = callback;
	}
	return (getImpl().cbResampleData != nullptr);
}


bool SigChain::ChainSetup::useIntensityDataCallback(bool isSet, IntensityDataCallback * callback)
{
	if (isSet) {
		unique_lock<mutex> lock(getImpl().mutexRegist);
		getImpl().cbIntensityData = callback;
	}
	return (getImpl().cbIntensityData != nullptr);
}


PreviewImageCallback * SigChain::ChainSetup::getPreviewImageCallback(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbPreviewImage;
}


PreviewImageCallback2 * SigChain::ChainSetup::getPreviewImageCallback2(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbPreviewImage2;
}


PreviewCorneaCallback * SigChain::ChainSetup::getPreviewCorneaCallback(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbPreviewCornea;
}


EnfaceImageCallback * SigChain::ChainSetup::getEnfaceImageCallback(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbEnfaceImage;
}


SpectrumDataCallback * SigChain::ChainSetup::getSpectrumDataCallback(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbSpectrumData;
}


ResampleDataCallback * SigChain::ChainSetup::getResampleDataCallback(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbResampleData;
}


IntensityDataCallback * SigChain::ChainSetup::getIntensityDataCallback(void)
{
	unique_lock<mutex> lock(getImpl().mutexRegist);
	return getImpl().cbIntensityData;
}



ChainSetup::ChainSetupImpl & SigChain::ChainSetup::getImpl(void) 
{
	return *d_ptr;
}


