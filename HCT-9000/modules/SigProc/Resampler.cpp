#include "stdafx.h"
#include "Resampler.h"

#include <memory>
#include <cmath>

#include "CppUtil2.h"

using namespace SigProc; 
using namespace CppUtil;
using namespace std;


struct Resampler::ResamplerImpl
{
	double wavelengthFunctionCoeffs[RESAMPLE_WAVELENGTH_FUNCTION_DEGREE];

	double detectorWavelength[RESAMPLE_PADDED_PIXELS];
	double detectorKValue[RESAMPLE_PADDED_PIXELS];
	double linearizedKValue[RESAMPLE_PADDED_PIXELS];

	unsigned short lowerPixelIndex[RESAMPLE_DETECTOR_PIXELS];
	unsigned short upperPixelIndex[RESAMPLE_DETECTOR_PIXELS];
	unsigned short prevLowerPixelIndex[RESAMPLE_DETECTOR_PIXELS];
	unsigned short nextUpperPixelIndex[RESAMPLE_DETECTOR_PIXELS];

	float cubicSplineA[RESAMPLE_DETECTOR_PIXELS];
	float cubicSplineB[RESAMPLE_DETECTOR_PIXELS];
	float cubicSplineC[RESAMPLE_DETECTOR_PIXELS];
	float cubicSplineD[RESAMPLE_DETECTOR_PIXELS];

	float kValueBetweenUpperAndLower[RESAMPLE_DETECTOR_PIXELS];
	float kValueBetweenLowerAndPrevLower[RESAMPLE_DETECTOR_PIXELS];
	float kValueBetweenNextUpperAndUpper[RESAMPLE_DETECTOR_PIXELS];
	float kValueLinearized[RESAMPLE_DETECTOR_PIXELS];

	float linearizedKInterval;
	float wavelength[RESAMPLE_DETECTOR_PIXELS];

	ResamplerImpl() : wavelengthFunctionCoeffs{} {
	}
};


Resampler::Resampler() :
	d_ptr(make_unique<ResamplerImpl>())
{
	double coeffs[4] = { RESAMPLE_WAVELENGTH_PARAM_1, RESAMPLE_WAVELENGTH_PARAM_2, 
						 RESAMPLE_WAVELENGTH_PARAM_3, RESAMPLE_WAVELENGTH_PARAM_4 };
	setWavelengthFunctionCoefficients(coeffs);
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigProc::Resampler::~Resampler() = default;
SigProc::Resampler::Resampler(Resampler && rhs) = default;
Resampler & SigProc::Resampler::operator=(Resampler && rhs) = default;


double SigProc::Resampler::getWavelengthFunctionCoefficient(int index) const
{
	if (index >= 0 && index < RESAMPLE_WAVELENGTH_FUNCTION_DEGREE) {
		return getImpl().wavelengthFunctionCoeffs[index];
	}
	return 0.0;
}


void SigProc::Resampler::setWavelengthFunctionCoefficient(int index, double value)
{
	if (index >= 0 && index < RESAMPLE_WAVELENGTH_FUNCTION_DEGREE) {
		getImpl().wavelengthFunctionCoeffs[index] = value;
	}
	return;
}


void SigProc::Resampler::setWavelengthFunctionCoefficients(double coeffs[RESAMPLE_WAVELENGTH_FUNCTION_DEGREE])
{
	// Define wavelength function of detector.
	for (int i = 0; i < RESAMPLE_WAVELENGTH_FUNCTION_DEGREE; i++) {
		getImpl().wavelengthFunctionCoeffs[i] = coeffs[i];
	}

	updateResamplingParameters();
	return;
}


float * SigProc::Resampler::getCubicSplineA(void) const
{
	return &getImpl().cubicSplineA[0];
}


float * SigProc::Resampler::getCubicSplineB(void) const
{
	return &getImpl().cubicSplineB[0];
}


float * SigProc::Resampler::getCubicSplineC(void) const
{
	return &getImpl().cubicSplineC[0];
}


float * SigProc::Resampler::getCubicSplineD(void) const
{
	return &getImpl().cubicSplineD[0];
}


float * SigProc::Resampler::getkValueBetweenUpperAndLower(void) const
{
	return &getImpl().kValueBetweenUpperAndLower[0];
}


float * SigProc::Resampler::getkValueBetweenLowerAndPrevLower(void) const
{
	return &getImpl().kValueBetweenLowerAndPrevLower[0];
}


float * SigProc::Resampler::getkValueBetweenNextUpperAndUpper(void) const
{
	return &getImpl().kValueBetweenNextUpperAndUpper[0];
}


float * SigProc::Resampler::getkValueLinearized(void) const
{
	return &getImpl().kValueLinearized[0];
}


unsigned short * SigProc::Resampler::getLowerPixelIndex(void) const
{
	return &getImpl().lowerPixelIndex[0];
}


float SigProc::Resampler::getLinearizedKValueInterval(void) const
{
	return getImpl().linearizedKInterval;
}


double * SigProc::Resampler::detectorWaveLength(void) const
{
	return getImpl().detectorWavelength;
}


double SigProc::Resampler::getWavenumberResampledAtPixelPosition(int index) const
{
	double coeffs[4] = { 0 };
	coeffs[0] = getWavelengthFunctionCoefficient(0);
	coeffs[1] = getWavelengthFunctionCoefficient(1);
	coeffs[2] = getWavelengthFunctionCoefficient(2);
	coeffs[3] = getWavelengthFunctionCoefficient(3);
	double k = (2.0 * M_PI) / ((coeffs[0] + coeffs[1] * pow(index, 1) + coeffs[2] * pow(index, 2) + coeffs[3] * pow(index, 3)) * 0.000000001);
	return k;
}


double SigProc::Resampler::getWavenumberResampledAtPixelPosition(float index) const
{
	double coeffs[4] = { 0 };
	coeffs[0] = getWavelengthFunctionCoefficient(0);
	coeffs[1] = getWavelengthFunctionCoefficient(1);
	coeffs[2] = getWavelengthFunctionCoefficient(2);
	coeffs[3] = getWavelengthFunctionCoefficient(3);
	double k = (2.0 * M_PI) / ((coeffs[0] + coeffs[1] * pow(index, 1) + coeffs[2] * pow(index, 2) + coeffs[3] * pow(index, 3)) * 0.000000001);
	return k;
}


double SigProc::Resampler::getWavelengthResampledAtPixelPosition(float index) const
{
	double k = getWavenumberResampledAtPixelPosition(index);
	if (k != 0.0) {
		float w = (float)((2.0 * M_PI) / k);
		w *= 1000000000.0f;
		return w;
	}
	else {
		return 0.0;
	}
}


double SigProc::Resampler::getPixelResolutionInAir(void) const
{
	double k1 = getWavenumberResampledAtPixelPosition(0);
	double k2 = getWavenumberResampledAtPixelPosition(LINE_CAMERA_CCD_PIXELS - 1);
	double dk = k1 - k2;
	double resol = M_PI / dk;

	return resol;
}

double SigProc::Resampler::getPixelResolutionInTissue(double fwhm, double refIndex) const
{
	double air = getPixelResolutionInAir();
	double resol = (air * fwhm) / refIndex;

	return (resol * 1000000.0);
}

double SigProc::Resampler::getPixelResolution(double refractiveIndex) const
{
	double airsol = getPixelResolutionInAir();
	double pixsol = (airsol / refractiveIndex) * 1000000.0;
	// LogD() << "Scan pixel resol : " << pixsol << ", refractiveIndex : " << refractiveIndex;

	return pixsol;
}

Resampler::ResamplerImpl & SigProc::Resampler::getImpl(void) const
{
	return *d_ptr;
}


void SigProc::Resampler::updateDetectorWavelengths(void)
{
	// Initialize wavelength on detector pixels through wavelength function 
	// across the padded pixels count.  
	double wavelenght;
	for (int idx = 0; idx < RESAMPLE_PADDED_PIXELS; idx++) {
		wavelenght = getImpl().wavelengthFunctionCoeffs[0];
		wavelenght += (idx * getImpl().wavelengthFunctionCoeffs[1]);
		wavelenght += (pow(idx, 2.0) * getImpl().wavelengthFunctionCoeffs[2]);
		wavelenght += (pow(idx, 3.0) * getImpl().wavelengthFunctionCoeffs[3]);

		getImpl().detectorWavelength[idx] = wavelenght;
	}

	return;
}


void SigProc::Resampler::updateDetectorKValues(void)
{
	double kvalue;

	// To use M_PI in VC++, _USE_MATH_DEFINES should be defined as preprocessor definitions.
	for (int idx = 0; idx < RESAMPLE_PADDED_PIXELS; idx++) {
		kvalue = 2.0 * M_PI / getImpl().detectorWavelength[idx];
		getImpl().detectorKValue[idx] = kvalue;
	}
	return;
}


void SigProc::Resampler::updateLinearizedKValues(void)
{
	double detectorKValueStart = getImpl().detectorKValue[RESAMPLE_KVALUE_START_INDEX];
	double detectorKValueEnd = getImpl().detectorKValue[RESAMPLE_KVALUE_END_INDEX];
	double linearizedKInterval = (detectorKValueEnd - detectorKValueStart) / (double)(RESAMPLE_DETECTOR_PIXELS - 1);

	for (int idx = 0; idx < RESAMPLE_PADDED_PIXELS; idx++) {
		getImpl().linearizedKValue[idx] = detectorKValueStart + (linearizedKInterval * idx);
	}
	getImpl().linearizedKInterval = (float)abs(linearizedKInterval);
	return;
}


void SigProc::Resampler::updateResamplingParameters(void)
{
	// Each K value is dependent on wavelength at the same pixel position. 
	updateDetectorWavelengths();
	updateDetectorKValues();
	updateLinearizedKValues();

	int resamplingIdx;
	double lowerKValue;
	double upperKValue;
	double prevLowerKValue;
	double nextUpperKValue;
	double parameterA, parameterB, parameterC, parameterD;

	int prevIdx = 0;
	bool isSet = false;

	for (int i = 0; i < RESAMPLE_DETECTOR_PIXELS; i++) 
	{
		isSet = false;
		for (int j = 0; j < RESAMPLE_PADDED_PIXELS; j++)
		{
			if (getImpl().detectorKValue[j] <= getImpl().linearizedKValue[i]) 
			{
				resamplingIdx = j;
				getImpl().lowerPixelIndex[i] = (unsigned short) resamplingIdx;
				getImpl().upperPixelIndex[i] = (unsigned short) max(resamplingIdx - 1, 0);
				getImpl().prevLowerPixelIndex[i] = (unsigned short) min(resamplingIdx + 1, RESAMPLE_DETECTOR_PIXELS - 1);
				getImpl().nextUpperPixelIndex[i] = (unsigned short) max(resamplingIdx - 2, 0);

				lowerKValue = getImpl().detectorKValue[resamplingIdx];
				upperKValue = getImpl().detectorKValue[resamplingIdx - 1];
				prevLowerKValue = getImpl().detectorKValue[resamplingIdx + 1];

				if ((resamplingIdx - 2) < 0) {
					nextUpperKValue = upperKValue + (upperKValue - lowerKValue);
				}
				else {
					nextUpperKValue = getImpl().detectorKValue[resamplingIdx - 2];
				}

				isSet = true;
				prevIdx = resamplingIdx - 1;
				break;
			}
		}

		if (isSet == false)
		{
			resamplingIdx = RESAMPLE_PADDED_PIXELS - 1;
			getImpl().lowerPixelIndex[i] = (unsigned short) resamplingIdx;
			getImpl().upperPixelIndex[i] = (unsigned short) max(resamplingIdx - 1, 0);
			getImpl().prevLowerPixelIndex[i] = (unsigned short) min(resamplingIdx + 1, RESAMPLE_DETECTOR_PIXELS - 1);
			getImpl().nextUpperPixelIndex[i] = (unsigned short) max(resamplingIdx - 2, 0);

			lowerKValue = getImpl().detectorKValue[resamplingIdx];
			upperKValue = getImpl().detectorKValue[resamplingIdx - 1];
			prevLowerKValue = lowerKValue + (lowerKValue - upperKValue);
			nextUpperKValue = getImpl().detectorKValue[resamplingIdx - 2];
		}

		parameterA = (upperKValue - getImpl().linearizedKValue[i]) / (upperKValue - lowerKValue);
		parameterB = (getImpl().linearizedKValue[i] - lowerKValue) / (upperKValue - lowerKValue);
		parameterC = (1.0 / 6.0) * (pow(parameterA, 3.0) - parameterA) * (pow(upperKValue - lowerKValue, 2.0));
		parameterD = (1.0 / 6.0) * (pow(parameterB, 3.0) - parameterB) * (pow(upperKValue - lowerKValue, 2.0));

		getImpl().cubicSplineA[i] = (float) parameterA;
		getImpl().cubicSplineB[i] = (float) parameterB;
		getImpl().cubicSplineC[i] = (float) parameterC;
		getImpl().cubicSplineD[i] = (float) parameterD;

		getImpl().kValueBetweenUpperAndLower[i] = (float)(upperKValue - lowerKValue);
		getImpl().kValueBetweenLowerAndPrevLower[i] = (float)(lowerKValue - prevLowerKValue);
		getImpl().kValueBetweenNextUpperAndUpper[i] = (float)(nextUpperKValue - upperKValue);

		// Type conversion from double to float for GPU processing.
		getImpl().kValueLinearized[i] = (float)getImpl().linearizedKValue[i];
		getImpl().wavelength[i] = (float)getImpl().detectorWavelength[i];

		// DebugOut2() << "Index: " << i << ", kValueLinearized: " << getImpl().kValueLinearized[i];
		// DebugOut2() << i << "th " << getImpl().lowerPixelIndex[i] << ", " << getImpl().kValueBetweenUpperAndLower[i] << ", " << getImpl().kValueBetweenLowerAndPrevLower[i] << ", " << getImpl().kValueBetweenNextUpperAndUpper[i];
	}
	return;
}

