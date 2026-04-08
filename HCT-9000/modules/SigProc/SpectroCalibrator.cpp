#include "stdafx.h"
#include "SpectroCalibrator.h"

#include <vector>

#include "CppUtil2.h"
#include "QR"

using namespace SigProc;
using namespace CppUtil;
using namespace Eigen;
using namespace std;


struct SpectroCalibrator::SpectroCalibratorImpl
{
	vector<vector<double>> fbgClasses;
	vector<int> peakIndexs;
	vector<int> peakValues;
	vector<double> waveLengths;
	vector<double> parameters;

	int threshPeak;
	int threshSubs;

	SpectroCalibratorImpl() : 
		threshPeak(FBG_PEAK_THRESHOLD_INIT), threshSubs(FBG_PEAK_THRESHOLD_INIT)
	{
		fbgClasses = vector<vector<double>>(NUMBER_OF_FBG_CLASSES);
	}
};


SpectroCalibrator::SpectroCalibrator() :
	d_ptr(make_unique<SpectroCalibratorImpl>())
{
	initFbgClasses();
}


SigProc::SpectroCalibrator::~SpectroCalibrator() = default;
SigProc::SpectroCalibrator::SpectroCalibrator(SpectroCalibrator && rhs) = default;
SpectroCalibrator & SigProc::SpectroCalibrator::operator=(SpectroCalibrator && rhs) = default;


SigProc::SpectroCalibrator::SpectroCalibrator(const SpectroCalibrator & rhs)
	: d_ptr(make_unique<SpectroCalibratorImpl>(*rhs.d_ptr))
{
}


SpectroCalibrator & SigProc::SpectroCalibrator::operator=(const SpectroCalibrator & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SigProc::SpectroCalibrator::updateWavelengths(int fbgClass)
{
	if (fbgClass < 0 || fbgClass >= NUMBER_OF_FBG_CLASSES) {
		return;
	}

	getImpl().waveLengths.clear();
	getImpl().waveLengths.assign(getImpl().fbgClasses[fbgClass].begin(),
								 getImpl().fbgClasses[fbgClass].end());
	return;
}


int SigProc::SpectroCalibrator::findPeaksOnSpectrum(unsigned short * data, int size)
{
	findPeaksInCenter(data, size);
	findPeaksInOutside(data, size);
	return getNumberOfPeaks();
}


bool SigProc::SpectroCalibrator::calculateParameters(int order)
{
	if (getImpl().peakIndexs.size() != getImpl().waveLengths.size()) {
		return false;
	}

	size_t rows = getImpl().peakIndexs.size();
	size_t cols = order;

	MatrixXd A(rows, cols);
	VectorXd b = VectorXd::Map(&getImpl().waveLengths[0], getImpl().waveLengths.size());
	VectorXd x;

	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < cols; j++) {
			A(i, j) = pow((double)getImpl().peakIndexs[i], (double)j);
		}
	}

	// Solve least squares with QR factorization. 
	x = A.householderQr().solve(b);

	getImpl().parameters = std::vector<double>(WAVELENGTH_FUNCTION_COEFFS, 0.0);
	for (int i = 0; i < cols; i++) {
		getImpl().parameters[i] = (x[i]);
	}

	return true;
}


int SigProc::SpectroCalibrator::getThresholdOfPeak(void) const
{
	return getImpl().threshPeak;
}


int SigProc::SpectroCalibrator::getThresholdOfSubs(void) const
{
	return getImpl().threshSubs;
}


void SigProc::SpectroCalibrator::setThresholdOfPeak(int level)
{
	getImpl().threshPeak = level;
	return;
}


void SigProc::SpectroCalibrator::setThresholdOfSubs(int level)
{
	getImpl().threshSubs = level;
	return;
}


int SigProc::SpectroCalibrator::getNumberOfPeaks(void) const
{
	return (int)getImpl().peakIndexs.size();
}


std::vector<int>& SigProc::SpectroCalibrator::getPeakIndexs(void) const
{
	return getImpl().peakIndexs;
}


std::vector<int>& SigProc::SpectroCalibrator::getPeakValues(void) const
{
	return getImpl().peakValues;
}


std::vector<double>& SigProc::SpectroCalibrator::getWavelengths(void) const
{
	return getImpl().waveLengths;
}


const std::vector<double>& SigProc::SpectroCalibrator::getParameters(void) const
{
	return getImpl().parameters;
}


const vector<double>& SigProc::SpectroCalibrator::getFbgClass(int index) const
{
	return getImpl().fbgClasses[index];
}


SpectroCalibrator::SpectroCalibratorImpl & SigProc::SpectroCalibrator::getImpl(void) const
{
	return *d_ptr;
}


void SigProc::SpectroCalibrator::initFbgClasses(void)
{
	getImpl().fbgClasses[FBG_CLASS1] = vector<double> {
		800.77f, 804.73f, 810.16f, 815.24f, 820.34f,
		824.55f, 830.07f, 834.91f, 840.46f, 844.85f,
		849.91f, 855.15f, 859.39f, 865.04f, 870.42f,
		874.83f, 880.38f, 885.00f };

	getImpl().fbgClasses[FBG_CLASS2] = vector<double>{
		800.74f, 804.69f, 810.01f, 815.22f, 820.32f,
		824.49f, 830.09f, 835.03f, 840.49f, 844.74f,
		849.97f, 855.07f, 859.17f, 864.92f, 870.17f,
		874.88f, 880.30f, 884.89f };

	getImpl().fbgClasses[FBG_CLASS3] = vector<double>{
		800.83f, 804.59f, 810.08f, 815.20f, 820.34f,
		824.49f, 830.00f, 834.96f, 840.51f, 844.74f,
		849.97f, 855.07f, 859.17f, 864.92f, 870.17f,
		874.88f, };//880.30f, 884.89f };

	getImpl().fbgClasses[FBG_CLASS4] = vector<double>{
		789.80f, 795.05f, 800.35f, 804.32f, 809.90f,
		815.03f, 820.15f, 824.27f, 829.79f, 837.41f,
		840.20f, 844.45f, 849.82f, 854.91f, 860.33f,
		864.68f, 869.95f, 874.71f, 880.05f, 884.77f,
		889.20f };

	getImpl().fbgClasses[FBG_CLASS5] = vector<double>{
		789.82f, 795.10f, 800.41f, 804.29f, 809.92f, 
		815.05f, 820.22f, 824.32f, 829.80f, 837.48f, 
		840.23f, 844.48f, 850.06f, 854.89f, 860.32f, 
		864.70f, 869.96f, 874.81f, 880.07f, 884.81f, 
		889.22f };

	return;
}


const char * SigProc::SpectroCalibrator::getFbgClassName(int index) const
{
	switch (index) {
	case FBG_CLASS1:
		return FBG_CLASS1_NAME;
	case FBG_CLASS2:
		return FBG_CLASS2_NAME;
	case FBG_CLASS3:
		return FBG_CLASS3_NAME;
	case FBG_CLASS4:
		return FBG_CLASS4_NAME;
	case FBG_CLASS5:
		return FBG_CLASS5_NAME;
	}
	return FBG_UNKNOWN_NAME;
}


int SigProc::SpectroCalibrator::findPeaksInCenter(unsigned short * data, int size)
{
	getImpl().peakIndexs.clear();
	getImpl().peakValues.clear();

	int thresh = getThresholdOfPeak();

	int maxVal = -1;
	int maxIdx = -1;
	int nDists = 0;
	for (int i = 0; i < size; i++) {
		if (maxIdx < 0) {
			if (data[i] >= thresh) {
				maxIdx = i;
				maxVal = data[i];
				nDists = 0;
			}
		}
		else {
			if (data[i] >= maxVal) {
				maxIdx = i;
				maxVal = data[i];
				nDists = 0;
			}
			else {
				if (++nDists >= FBG_PEAK_WINDOW_SIZE) {
					getImpl().peakIndexs.push_back(maxIdx);
					getImpl().peakValues.push_back(maxVal);
					maxIdx = maxVal = -1;
					nDists = 0;
				}
			}
		}
	}

	if (maxIdx > 0) {
		getImpl().peakIndexs.push_back(maxIdx);
		getImpl().peakValues.push_back(maxVal);
	}

	return (int)getImpl().peakIndexs.size();
}


int SigProc::SpectroCalibrator::findPeaksInOutside(unsigned short * data, int size)
{
	if (getImpl().peakIndexs.size() <= 0) {
		return 0;
	}

	int thresh = getThresholdOfSubs();
	int nFound = 0;

	int maxVal = -1;
	int maxIdx = -1;
	int nDists = 0;
	for (int i = (getImpl().peakIndexs.front() - FBG_PEAK_WINDOW_SIZE); i > 0; i--) {
		if (maxIdx < 0) {
			if (data[i] >= thresh) {
				maxIdx = i;
				maxVal = data[i];
				nDists = 0;
			}
		}
		else {
			if (data[i] >= maxVal) {
				maxIdx = i;
				maxVal = data[i];
				nDists = 0;
			}
			else {
				if (++nDists >= FBG_PEAK_WINDOW_SIZE) {
					getImpl().peakIndexs.insert(getImpl().peakIndexs.begin(), maxIdx);
					getImpl().peakValues.insert(getImpl().peakValues.begin(), maxVal);
					maxIdx = maxVal = -1;
					nDists = 0;
					nFound++;
				}
			}
		}
	}

	if (maxIdx > 0) {
		getImpl().peakIndexs.insert(getImpl().peakIndexs.begin(), maxIdx);
		getImpl().peakValues.insert(getImpl().peakValues.begin(), maxVal);
		nFound++;
	}

	maxIdx = maxVal = -1;
	nDists = 0;

	for (int i = (getImpl().peakIndexs.back() + FBG_PEAK_WINDOW_SIZE); i < size; i++) {
		if (maxIdx < 0) {
			if (data[i] >= thresh) {
				maxIdx = i;
				maxVal = data[i];
				nDists = 0;
			}
		}
		else {
			if (data[i] >= maxVal) {
				maxIdx = i;
				maxVal = data[i];
				nDists = 0;
			}
			else {
				if (++nDists >= FBG_PEAK_WINDOW_SIZE) {
					getImpl().peakIndexs.insert(getImpl().peakIndexs.end(), maxIdx);
					getImpl().peakValues.insert(getImpl().peakValues.end(), maxVal);
					maxIdx = maxVal = -1;
					nDists = 0;
					nFound++;
				}
			}
		}
	}

	if (maxIdx > 0) {
		getImpl().peakIndexs.insert(getImpl().peakIndexs.end(), maxIdx);
		getImpl().peakValues.insert(getImpl().peakValues.end(), maxVal);
		nFound++;
	}

	return nFound;
}
