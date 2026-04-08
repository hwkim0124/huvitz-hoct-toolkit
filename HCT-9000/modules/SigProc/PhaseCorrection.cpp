#include "stdafx.h"
#include "PhaseCorrection.h"
#include "ipp.h"
#include "ipps.h"


#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#include "CppUtil2.h"

using namespace SigProc;
using namespace CppUtil;
using namespace std;


struct PhaseCorrection::PhaseCorrectionImpl
{
	bool initialized;
	unique_ptr<float[]> sample;
	unique_ptr<float[]> background;
	unique_ptr<float[]> waveNums;
	float centerOfWaveNum;

	int sampleSize;
	int resultSize;
	Ipp8u* pBuffer;

	IppsHilbertSpec* hilbertSpec;
	Ipp8u* hilbertBuff;
	Ipp32f* hilbertInput;
	Ipp32fc* hilbertOutput;
	Ipp32f* hilbertPhase;
	Ipp32f* hilbertMagnitude;

	Ipp32f* shiftPhase;

	IppsFFTSpec_C_32fc* fourierSpec;
	Ipp8u* fourierBuff;
	Ipp8u* fourierSpecBuff;
	Ipp32fc* fourierInput;
	Ipp32fc* fourierOutput;
	Ipp32f* fourierMagnitude;
	
	PhaseCorrectionImpl() : initialized(false), sampleSize(0), resultSize(0),
			sample(nullptr), background(nullptr),
			hilbertSpec(nullptr), hilbertBuff(nullptr),
			hilbertInput(nullptr), hilbertOutput(nullptr), hilbertPhase(nullptr),
			hilbertMagnitude(nullptr), shiftPhase(nullptr),
			fourierSpec(nullptr), fourierBuff(nullptr), fourierSpecBuff(nullptr), 
			fourierInput(nullptr), fourierOutput(nullptr), fourierMagnitude(nullptr)
	{

	}
};


PhaseCorrection::PhaseCorrection() :
	d_ptr(make_unique<PhaseCorrectionImpl>())
{
	initialize();
}


SigProc::PhaseCorrection::~PhaseCorrection()
{
	release();
}


SigProc::PhaseCorrection::PhaseCorrection(PhaseCorrection && rhs) = default;
PhaseCorrection & SigProc::PhaseCorrection::operator=(PhaseCorrection && rhs) = default;


bool SigProc::PhaseCorrection::initialize(void)
{
	IppStatus status = ippInit();
	if (status != ippStsNoErr) {
		DebugOut2() << "IppInit() Error: " << ippGetStatusString(status);
		return false;
	}

	if (!setSampleSize(LINE_CAMERA_CCD_PIXELS)) {
		return false;
	}

	initWaveNumbers();
	getImpl().initialized = true;
	return true;
}


void SigProc::PhaseCorrection::release(void)
{
	releaseBuffers();
	getImpl().sampleSize = 0;
	getImpl().initialized = false;
	return;
}


int SigProc::PhaseCorrection::getSampleSize(void)
{
	return getImpl().sampleSize;
}


bool SigProc::PhaseCorrection::setSampleSize(int size)
{
	getImpl().sample = make_unique<float[]>(size);
	getImpl().sampleSize = size;
	getImpl().resultSize = (size / 2 + 1);

	getImpl().background = make_unique<float[]>(size);

	releaseBuffers();
	if (!initHilbertBuffer()) {
		return false;
	}

	if (!initFourierBuffer()) {
		return false;
	}
	return true;
}


bool SigProc::PhaseCorrection::setSampleData(float * data, int size)
{
	if (data == nullptr) {
		return false;
	}

	memcpy((void*)getImpl().sample.get(), (void*)data, sizeof(float)*size);
	return true;
}


bool SigProc::PhaseCorrection::setBackgroundData(float * data, int size)
{
	if (data == nullptr) {
		return false;
	}

	memcpy((void*)getImpl().background.get(), (void*)data, sizeof(float)*size);
	return true;
}


bool SigProc::PhaseCorrection::performPhaseZero(float * output)
{
	if (!prepareInputData()) {
		return false;
	}

	IppStatus status = ippsHilbert_32f32fc(getImpl().hilbertInput, getImpl().hilbertOutput,
											getImpl().hilbertSpec, getImpl().hilbertBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsHilbert() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsFFTFwd_CToC_32fc(getImpl().hilbertOutput, getImpl().fourierOutput, getImpl().fourierSpec, getImpl().fourierBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsFFTFwd_CToC() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsMagnitude_32fc(getImpl().fourierOutput, getImpl().fourierMagnitude, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsMagnitude() Error: " << ippGetStatusString(status);
		return false;
	}

	for (int i = 0; i < getImpl().resultSize; i++) {
		output[i] = getImpl().fourierMagnitude[i];
	}
	return true;
}


bool SigProc::PhaseCorrection::performPhaseShift(float alpha2, float alpha3, float * output)
{
	if (!prepareInputData()) {
		return false;
	}

	if (!hilbertTransform()) {
		return false;
	}

	float* waveNums = getImpl().waveNums.get();
	float centerNum = getImpl().centerOfWaveNum;
	float waveDiff;

	for (int i = 0; i < getImpl().sampleSize; i++) {
		waveDiff = waveNums[i] - centerNum;
		getImpl().shiftPhase[i] = (float)(getImpl().hilbertPhase[i] - pow(waveDiff, 2.0) * alpha2 - pow(waveDiff, 3.0) * alpha3);
	}

	IppStatus status = ippsPolarToCart_32fc(getImpl().hilbertMagnitude, getImpl().shiftPhase, getImpl().fourierInput, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsPolarToCart() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsFFTFwd_CToC_32fc(getImpl().fourierInput, getImpl().fourierOutput, getImpl().fourierSpec, getImpl().fourierBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsFFTFwd_CToC() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsMagnitude_32fc(getImpl().fourierOutput, getImpl().fourierMagnitude, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsMagnitude() Error: " << ippGetStatusString(status);
		return false;
	}

	for (int i = 0; i < getImpl().resultSize; i++) {
		output[i] = getImpl().fourierMagnitude[i];
	}
	return true;
}


bool SigProc::PhaseCorrection::performPhaseShift(float* output)
{
	if (!prepareInputData()) {
		return false;
	}
	
	IppStatus status = ippsHilbert_32f32fc(getImpl().hilbertInput, getImpl().hilbertOutput,
											getImpl().hilbertSpec, getImpl().hilbertBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsHilbert() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsPhase_32fc(getImpl().hilbertOutput, getImpl().hilbertPhase, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsPhase() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsMagnitude_32fc(getImpl().hilbertOutput, getImpl().hilbertMagnitude, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsMagnitude() Error: " << ippGetStatusString(status);
		return false;
	}

	// Phase shift 
	double centerOfWavenumber = (2.0*3.141592653589*10000000.0 / (840.0));// 840.0f; // (10000.0f / 840.0f);
	double alpha2 = 0.0;
	double astep2 = 0.000001;

	double waveNums[2048];
	for (int i = 0; i < 2048; i++) {
		// waveNums[i] = (float)(10000.0/(800.752136 + i*0.047172 + pow(i, 2)*-0.00003));
		waveNums[i] = (2.0*3.141592653589*10000000.0 / (800.752136 + i*0.047172 + pow(i, 2)*-0.00003));
	}

	float maxRatio = 0.0f;
	float maxPeak = 0.0f;
	int maxIndex = 0;

	for (int n = 0; n < 1000; n++)
	{
		alpha2 = astep2 * n;
		// alpha2 = 0.890;
		for (int i = 0; i < getImpl().sampleSize; i++) {
			getImpl().shiftPhase[i] = (float)(getImpl().hilbertPhase[i] - pow((waveNums[i] - centerOfWavenumber), 2.0) * alpha2);
		}

		status = ippsPolarToCart_32fc(getImpl().hilbertMagnitude, getImpl().shiftPhase, getImpl().fourierInput, getImpl().sampleSize);
		if (status != ippStsNoErr) {
			DebugOut2() << "IppsPolarToCart() Error: " << ippGetStatusString(status);
			return false;
		}

		status = ippsFFTFwd_CToC_32fc(getImpl().fourierInput, getImpl().fourierOutput, getImpl().fourierSpec, getImpl().fourierBuff);
		if (status != ippStsNoErr) {
			DebugOut2() << "IppsFFTFwd_CToC() Error: " << ippGetStatusString(status);
			return false;
		}

		status = ippsMagnitude_32fc(getImpl().fourierOutput, getImpl().fourierMagnitude, getImpl().sampleSize);
		if (status != ippStsNoErr) {
			DebugOut2() << "IppsMagnitude() Error: " << ippGetStatusString(status);
			return false;
		}

		int thresh = 4096;
		int sizeM = 0;
		float peak = 0.0f;
		for (int i = 25; i < getImpl().resultSize; i++) {
			if (getImpl().fourierMagnitude[i] > thresh) {
				sizeM++;
				if (getImpl().fourierMagnitude[i] > peak) {
					peak = getImpl().fourierMagnitude[i];
				}
			}


		}
		float ratioM = (sizeM > 0 ? (1.0f / (float)sizeM) : 0.0f);
		// DebugOut2() << "N: " << n << ", RatioM : " << ratioM;

		if (maxRatio < ratioM || (maxRatio == ratioM && maxPeak < peak)) {
			maxRatio = ratioM;
			maxIndex = n;
			maxPeak = peak;

			for (int i = 0; i < getImpl().resultSize; i++) {
				output[i] = getImpl().fourierMagnitude[i];
			}
		}
		break;
	}
	DebugOut2() << "Index: " << maxIndex << ", ratioM : " << maxRatio << ", peak: " << maxPeak;
	return true;
}


bool SigProc::PhaseCorrection::saveBackgroundData(const std::wstring & path)
{
	float* pBuff = getImpl().background.get();
	float* pLine = pBuff;
	int lineSize = getSampleSize() * sizeof(float);
	int lineCount = 0;

	std::ofstream ofs(path, std::ios::binary | std::ios::out);
	ofs.write(reinterpret_cast<char*>(pLine), lineSize);
	lineCount++;

	DebugOut2() << "Save background data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


bool SigProc::PhaseCorrection::loadBackgroundData(const std::wstring & path)
{
	float* pBuff = getImpl().background.get();
	float* pLine = pBuff;
	size_t readLast;
	int lineSize = getSampleSize() * sizeof(float);
	int lineCount = 0;

	std::ifstream ifs(path, std::ios::binary | std::ios::in);
	if (!ifs.eof()) {
		ifs.read(reinterpret_cast<char*>(pLine), lineSize);
		readLast = ifs.gcount();
		if (readLast == lineSize) {
			lineCount++;
		}
	}

	DebugOut2() << "Load background data file: " << path.c_str() << ", lines: " << lineCount;
	return true;
}


PhaseCorrection::PhaseCorrectionImpl & SigProc::PhaseCorrection::getImpl(void) const
{
	return *d_ptr;
}


bool SigProc::PhaseCorrection::prepareInputData(void)
{
	int size = getSampleSize();
	if (size <= 0) {
		return false;
	}

	float* sample = getImpl().sample.get();
	float* backgd = getImpl().background.get();
	Ipp32f* input = getImpl().hilbertInput;

	if (getImpl().background != nullptr) {
		ippsSub_32f(backgd, sample, input, size);

		for (int i = 0; i < size; i++) {
			if (input[i] < 0.0f) {
				input[i] = 0.0f;
			}
			//DebugOut2() << "Idx: " << i << ", s: " << sample[i] << ", b: " << backgd[i] << " => " << input[i];
		}
	}
	else {
		ippsCopy_32f(sample, input, size);
	}

	return true;
}


bool SigProc::PhaseCorrection::hilbertTransform(void)
{
	IppStatus status = ippsHilbert_32f32fc(getImpl().hilbertInput, getImpl().hilbertOutput,
		getImpl().hilbertSpec, getImpl().hilbertBuff);

	if (status != ippStsNoErr) {
		DebugOut2() << "IppsHilbert() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsPhase_32fc(getImpl().hilbertOutput, getImpl().hilbertPhase, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsPhase() Error: " << ippGetStatusString(status);
		return false;
	}

	status = ippsMagnitude_32fc(getImpl().hilbertOutput, getImpl().hilbertMagnitude, getImpl().sampleSize);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsMagnitude() Error: " << ippGetStatusString(status);
		return false;
	}

	return true;
}


void SigProc::PhaseCorrection::initWaveNumbers(void)
{
	getImpl().waveNums = make_unique<float[]>(LINE_CAMERA_CCD_PIXELS);

	for (int i = 0; i < 2048; i++) {
		// waveNums[i] = (float)(10000.0/(800.752136 + i*0.047172 + pow(i, 2)*-0.00003));

		// To use M_PI in VC++, _USE_MATH_DEFINES should be defined as preprocessor definitions.
		// getImpl().waveNums[i] = (float)(2.0*M_PI / (800.752136 + i*0.047172 + pow(i, 2)*-0.00003));
		getImpl().waveNums[i] = (float)(1.0/ (800.752136 + i*0.047172 + pow(i, 2)*-0.00003));
	}

	// getImpl().centerOfWaveNum = (float)(2.0*M_PI / 840.0);
	getImpl().centerOfWaveNum = (float)(1.0 / 840.0);
	return;
}


bool SigProc::PhaseCorrection::initHilbertBuffer(void)
{
	int sizeSpec, sizeBuff;
	int size = getSampleSize();

	IppStatus status = ippsHilbertGetSize_32f32fc(size, ippAlgHintNone, &sizeSpec, &sizeBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsHilbertGetSize() Error: " << ippGetStatusString(status);
		return false;
	}

	getImpl().hilbertSpec = (IppsHilbertSpec*)ippMalloc(sizeSpec);
	getImpl().hilbertBuff = (Ipp8u*)ippMalloc(sizeBuff);

	status = ippsHilbertInit_32f32fc(size, ippAlgHintNone, getImpl().hilbertSpec, getImpl().hilbertBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsHilbertInit() Error: " << ippGetStatusString(status);
		return false;
	}

	getImpl().hilbertInput = ippsMalloc_32f(size);
	getImpl().hilbertOutput = ippsMalloc_32fc(size);
	getImpl().hilbertPhase = ippsMalloc_32f(size);
	getImpl().hilbertMagnitude = ippsMalloc_32f(size);

	getImpl().shiftPhase = ippsMalloc_32f(size);
	return true;
}


bool SigProc::PhaseCorrection::initFourierBuffer(void)
{
	int size = getSampleSize();
	int order = (int) log2(size);
	int flag = IPP_FFT_NODIV_BY_ANY;
	int sizeSpec, sizeInit, sizeBuff;
	Ipp8u* pMemInit = 0;

	IppStatus status = ippsFFTGetSize_C_32fc(order, flag, ippAlgHintNone,
											&sizeSpec, &sizeInit, &sizeBuff);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsFFTGetSize() Error: " << ippGetStatusString(status);
		return false;
	}

	getImpl().fourierSpecBuff = (Ipp8u*)ippMalloc(sizeSpec);
	if (sizeInit > 0) {
		pMemInit = (Ipp8u*)ippMalloc(sizeInit);
	}
	if (sizeBuff > 0) {
		getImpl().fourierBuff = (Ipp8u*)ippMalloc(sizeBuff);
	}

	status = ippsFFTInit_C_32fc(&getImpl().fourierSpec, order, flag, ippAlgHintNone,
								getImpl().fourierSpecBuff, pMemInit);
	if (status != ippStsNoErr) {
		DebugOut2() << "IppsFFTInit() Error: " << ippGetStatusString(status);
		return false;
	}

	if (sizeInit > 0) {
		ippFree(pMemInit);
	}

	getImpl().fourierInput = ippsMalloc_32fc(size);
	getImpl().fourierOutput = ippsMalloc_32fc(size);
	getImpl().fourierMagnitude = ippsMalloc_32f(size);
	return true;
}


void SigProc::PhaseCorrection::releaseBuffers(void)
{
	if (getImpl().hilbertSpec != nullptr) {
		ippFree(getImpl().hilbertSpec);
		getImpl().hilbertSpec = nullptr;
	}
	if (getImpl().hilbertBuff != nullptr) {
		ippFree(getImpl().hilbertBuff);
		getImpl().hilbertBuff = nullptr;
	}

	if (getImpl().hilbertInput != nullptr) {
		ippsFree(getImpl().hilbertInput);
		getImpl().hilbertInput = nullptr;
	}
	if (getImpl().hilbertOutput != nullptr) {
		ippsFree(getImpl().hilbertOutput);
		getImpl().hilbertOutput = nullptr;
	}

	if (getImpl().shiftPhase != nullptr) {
		ippsFree(getImpl().shiftPhase);
		getImpl().shiftPhase = nullptr;
	}

	if (getImpl().hilbertPhase != nullptr) {
		ippsFree(getImpl().hilbertPhase);
		getImpl().hilbertPhase = nullptr;
	}
	if (getImpl().hilbertMagnitude != nullptr) {
		ippsFree(getImpl().hilbertMagnitude);
		getImpl().hilbertMagnitude = nullptr;
	}

	if (getImpl().fourierSpecBuff != nullptr) {
		ippFree(getImpl().fourierSpecBuff);
		getImpl().fourierSpecBuff = nullptr;
	}
	if (getImpl().fourierBuff != nullptr) {
		ippFree(getImpl().fourierBuff);
		getImpl().fourierBuff = nullptr;
	}

	if (getImpl().fourierInput != nullptr) {
		ippsFree(getImpl().fourierInput);
		getImpl().fourierInput = nullptr;
	}
	if (getImpl().fourierOutput != nullptr) {
		ippsFree(getImpl().fourierOutput);
		getImpl().fourierOutput = nullptr;
	}

	if (getImpl().fourierMagnitude != nullptr) {
		ippsFree(getImpl().fourierMagnitude);
		getImpl().fourierMagnitude = nullptr;
	}
	return;
}
