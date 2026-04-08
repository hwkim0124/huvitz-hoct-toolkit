#include "stdafx.h"
#include "OclKernel.h"




using namespace SigChain;

OclKernel::OclKernel()
{
}


OclKernel::~OclKernel()
{
}


const char * SigChain::OclKernel::getClFFTPreCallback(void)
{
	const char* precallback = "float convert16To32bit(__global void* input, uint inoffset, __global void* userdata)\n" \
		"{\n" \
		"	__global ushort* p = (__global ushort*) input; \n" \
		"	return (float) p[inoffset]; \n" \
		"}\n";

	return precallback;
}


const char * SigChain::OclKernel::getClFFTPostCallback(void)
{
	const char* postcallback =
		"void restrictOutputValue(__global void* output, uint outoffset, __global void* userdata, float2 fftoutput)\n" \
		"{\n" \
		"	float value = min(fftoutput.x, 2048);\n"  \
		"	value = max(value, -2048);\n" \
		"	*((__global float*)output + outoffset) = value;\n" \
		"}\n";

	return postcallback;
}


const char * SigChain::OclKernel::getHilbertTransformSource(void)
{
	const char *p =
		"void hilbertTransform(								\n" \
		"						__global void* outputRe,	\n" \
		"						__global void* outputIm,	\n" \
		"						uint outoffset,				\n" \
		"						__global void* userdata,	\n" \
		"						float fftoutputRe,			\n" \
		"						float fftoutputIm)			\n" \
		"{													\n" \
		"	const int N = 2048;								\n" \
		"   int index = outoffset % N;						\n" \
		"	if (index >= 1 && index < (N/2)) {				\n" \
		"		*((__global float*)outputRe + outoffset) = 2.0f * fftoutputRe;	\n" \
		"		*((__global float*)outputIm + outoffset) = 2.0f * fftoutputIm;	\n" \
		"	}												\n" \
		"	else if (index == 0 || index == (N/2)) {						\n" \
		"		*((__global float*)outputRe + outoffset) = 1.0f * fftoutputRe;					\n" \
		"		*((__global float*)outputIm + outoffset) = 1.0f * fftoutputIm;					\n" \
		"	}												\n" \
		"	else {						\n" \
		"		*((__global float*)outputRe + outoffset) = 0.0f;					\n" \
		"		*((__global float*)outputIm + outoffset) = 0.0f;					\n" \
		"	}												\n" \
		"	return;											\n" \
		"}													\n";
	return p;
}


const char * SigChain::OclKernel::getHilbertPhaseShiftSource(void)
{
	const char *p =
		"void hilbertPhaseShift(								\n" \
		"						__global void* outputRe,	\n" \
		"						__global void* outputIm,	\n" \
		"						uint outoffset,				\n" \
		"						__global void* userdata,	\n" \
		"						float fftoutputRe,			\n" \
		"						float fftoutputIm)			\n" \
		"{													\n" \
		"	const int N = 2048;								\n" \
		"   int index = outoffset % N;						\n" \
		"	float phase = atan2(fftoutputIm, fftoutputRe);	\n" \
		"	float magnt = hypot(fftoutputIm, fftoutputRe);	\n" \
		"	float shift = phase - *((__global float*)userdata + index);		\n" \
		"													\n" \
		"	*((__global float*)outputRe + outoffset) = magnt * cos(shift);		\n" \
		"	*((__global float*)outputIm + outoffset) = magnt * sin(shift);		\n" \
		"	return;											\n" \
		"}													\n";
	
	/*
	const char *p =
		"#pragma OPENCL EXTENSION cl_khr_fp64 : enable			\n" \
		"void hilbertPhaseShift(								\n" \
		"						__global void* outputRe,	\n" \
		"						__global void* outputIm,	\n" \
		"						uint outoffset,				\n" \
		"						__global void* userdata,	\n" \
		"						float fftoutputRe,			\n" \
		"						float fftoutputIm)			\n" \
		"{													\n" \
		"	const int N = 2048;								\n" \
		"   int index = outoffset % N;						\n" \
		"	double fftoutRe = (double)fftoutputRe;			\n" \
		"	double fftoutIm = (double)fftoutputIm;								\n" \
		"	double phase = (double)atan2(fftoutIm, fftoutRe);							\n" \
		"	double magnt = (double)hypot(fftoutIm, fftoutRe);							\n" \
		"	double shift = phase - (double)*((__global float*)userdata + index);		\n" \
		"																		\n" \
		"	*((__global float*)outputRe + outoffset) = (double)(magnt * cos(shift));		\n" \
		"	*((__global float*)outputIm + outoffset) = (double)(magnt * sin(shift));		\n" \
		"	return;																\n" \
		"}																		\n";
	*/
	return p;
}


const char * SigChain::OclKernel::getRestrictOutputValueSource(void)
{
	const char *p =
		"__kernel void restrictOutputValue(					\n" \
		"						__global float* input,		\n" \
		"						const int lineSize,			\n" \
		"						const int numLines,			\n" \
		"						const float minVal,			\n" \
		"						const float maxVal)			\n" \
		"{\n" \
		"	const int x = get_global_id(0);					\n" \
		"	const int y = get_global_id(1);					\n" \
		"	const int idx = y * lineSize + x;				\n" \
		"	float value = input[idx];						\n" \
		"\n" \
		"	if (value > maxVal) {							\n" \
		"		input[idx] = maxVal;						\n"\
		"	}												\n" \
		"	else if (value < minVal) {						\n" \
		"		input[idx] = minVal;						\n"\
		"	}												\n" \
		"\n" \
		"}\n";

	return p;
}


const char * SigChain::OclKernel::getBackgroundSubtractionSource(void)
{
	const char *p =
		"__kernel void backgroundSubtraction(										\n" \
		"						__global ushort* input,								\n" \
		"						__global ushort* background,						\n" \
		"						__global float* output,								\n" \
		"						const uint numRows,									\n" \
		"						const uint numCols)									\n" \
		"{																			\n" \
		"	const int x = get_global_id(0);											\n" \
		"	const int y = get_global_id(1);											\n" \
		"	const int idx = x + y * numCols;										\n" \
		"																			\n" \
		"	// Convert inputs to float type with DC subtraction						\n" \
		"	output[idx] = (float)input[idx] - (float)background[x];					\n" \
		"	return;																	\n" \
		"}																			\n" \
		"																			\n" \
		"__kernel void backgroundSubtraction2(										\n" \
		"						__global ushort* input,								\n" \
		"						__global float* output,								\n" \
		"						const uint numRows,									\n" \
		"						const uint numCols)									\n" \
		"{																			\n" \
		"	const int x = get_global_id(0);											\n" \
		"	const int y = get_global_id(1);											\n" \
		"	const int idx = x + y * numCols;										\n" \
		"																			\n" \
		"	// Convert inputs to float type without DC subtraction					\n" \
		"	output[idx] = (float)((unsigned short)input[idx]);						\n" \
		"	return;																	\n" \
		"}																			\n" \
		"																			\n" \
		"__kernel void backgroundSubtraction3(										\n" \
		"						__global ushort* input,								\n" \
		"						__global ushort* background,						\n" \
		"						__global float* output,								\n" \
		"						const uint numRows,									\n" \
		"						const uint numCols)									\n" \
		"{																			\n" \
		"	const int x = get_global_id(0);											\n" \
		"	const int y = get_global_id(1);											\n" \
		"	const int idx = x + y * numCols;										\n" \
		"	float outVal = 0.0f;													\n" \
		"	float bgdVal = 0.0f;													\n" \
		"	float w = 0.0f;															\n" \
		"	const float pi = 3.141592653589f;										\n" \
		"																			\n" \
		"	w = 0.35875f - 0.48829f*cos(2.0f*pi*x/2047.0f) + 0.14128f*cos(4.0f*pi*x/2047.0f) - 0.01168f*cos(6.0f*pi*x/2047.0f);	\n" \
		"																			\n" \
		"	outVal = (float)(input[idx] * w);										\n" \
		"	bgdVal = (float)(background[x] * w);									\n" \
		"	// outVal = 0.5 * (1.0 - cos(2.0*3.141592653589*x/2047.0)) * outVal;	\n" \
		"	// bgdVal = 0.5 * (1.0 - cos(2.0*3.141592653589*x/2047.0)) * bgdVal;	\n" \
		"	// Convert inputs to float type with DC subtraction						\n" \
		"	output[idx] = outVal - bgdVal;											\n" \
		"	return;																	\n" \
		"}																			\n" \
		"																			\n" \
		"__kernel void backgroundSubtraction4(										\n" \
		"						__global ushort* input,								\n" \
		"						__global float* output,								\n" \
		"						const uint numRows,									\n" \
		"						const uint numCols)									\n" \
		"{																			\n" \
		"	const int x = get_global_id(0);											\n" \
		"	const int y = get_global_id(1);											\n" \
		"	const int idx = x + y * numCols;										\n" \
		"	float outVal = 0.0f;													\n" \
		"	float w = 0.0f;															\n" \
		"	const float pi = 3.141592653589f;										\n" \
		"																			\n" \
		"	w = 0.35875f - 0.48829f*cos(2.0f*pi*x/2047.0f) + 0.14128f*cos(4.0f*pi*x/2047.0f) - 0.01168f*cos(6.0f*pi*x/2047.0f);	\n" \
		"																			\n" \
		"	outVal = (float)(input[idx] * w);												\n" \
		"	// outVal = 0.5 * (1.0 - cos(2.0*3.141592653589*x/2047.0)) * outVal;			\n" \
		"	// Convert inputs to float type without DC subtraction					\n" \
		"	output[idx] = outVal;													\n" \
		"	return;																	\n" \
		"}																			\n";

	return p;
}

const char* SigChain::OclKernel::getWindowingSpectrumSource(void)
{
	const char* p =
		"__kernel void windowingSpectrum(											\n" \
		"						__global float* input,								\n" \
		"						__global float* window,								\n" \
		"						__global float* output,								\n" \
		"						const uint numRows,									\n" \
		"						const uint numCols)									\n" \
		"{																			\n" \
		"	const int x = get_global_id(0);											\n" \
		"	const int y = get_global_id(1);											\n" \
		"	const int idx = x + y * numCols;										\n" \
		"																			\n" \
		"	// Windowing function to spectrum data									\n" \
		"	output[idx] = (float)input[idx] * (float)window[x];						\n" \
		"	return;																	\n" \
		"}																			\n";

	return p;
}


const char * SigChain::OclKernel::getResamplingInputSource(void)
{
	const char *p =
		"#define KLINEAR_CUBIC														\n" \
		"																			\n" \
		"__kernel void resamplingInput(												\n" \
		"						__global float* input,								\n" \
		"						__global float* output,								\n" \
		"						const uint numRows,									\n" \
		"						const uint numCols,									\n" \
		"						__global float* splineA,							\n" \
		"						__global float* splineB,							\n" \
		"						__global float* splineC,							\n" \
		"						__global float* splineD,							\n" \
		"						__global ushort* resampleIdx,						\n" \
		"						__global float* kValueUL,							\n" \
		"						__global float* kValueLL,							\n" \
		"						__global float* kValueUU,							\n" \
		"						const float kInterval)								\n" \
		"{																			\n" \
		"	const int x = get_global_id(0);											\n" \
		"	const int y = get_global_id(1);											\n" \
		"	const int ridx = y * numCols ;											\n" \
		"																			\n" \
		"	// Index range of input data is supposed to be fixed by CCD pixels		\n" \
		"	const int lowerIndex = resampleIdx[x] ;									\n" \
		"	const int upperIndex = max(resampleIdx[x] - 1, 0);						\n" \
		"	const int lowerPrevIndex = min(resampleIdx[x] + 1, 2047);				\n" \
		"	const int upperNextIndex = max(resampleIdx[x] - 2, 0);					\n" \
		"																			\n" \
		"	global float* data = &input[ridx];										\n" \
		"	float outVal = 0.0f;													\n" \
		"	float s1, s2;															\n" \
		"																			\n" \
		"#ifdef KLINEAR_CUBIC														\n" \
		"	s1 = (data[upperIndex] - data[lowerIndex]) / kValueUL[x];				\n" \
		"	s1 -= (data[lowerIndex] - data[lowerPrevIndex]) / kValueLL[x];			\n" \
		"	s1 /= kInterval;														\n" \
		"																			\n" \
		"	s2 = (data[upperNextIndex] - data[upperIndex]) / kValueUU[x];			\n" \
		"	s2 -= (data[upperIndex] - data[lowerIndex]) / kValueUL[x];				\n" \
		"	s2 /= kInterval;														\n" \
		"																			\n" \
		"	outVal = data[lowerIndex] * splineA[x];									\n" \
		"	outVal += data[upperIndex] * splineB[x];								\n" \
		"	outVal += s1 * splineC[x];												\n" \
		"	outVal += s2 * splineD[x];												\n" \
		"#else																		\n" \
		"	float weight = (data[upperIndex] - data[lowerIndex]);					\n" \
		"	weight *= splineB[x];													\n" \
		"	outVal = weight + data[lowerIndex];										\n" \
		"#endif																		\n" \
		"																			\n" \
		"	output[ridx + x] = outVal;												\n" \
		"	//output[ridx + x] = data[x];											\n" \
		"	return;																	\n" \
		"}																			\n";

	return p;
}


const char * SigChain::OclKernel::getFixedNoiseReductionSource(void)
{
	const char *p =
		"__kernel void fixedNoiseReduction(					\n" \
		"						__global float* input,		\n" \
		"						const uint numRows,			\n" \
		"						const uint numCols,			\n" \
		"						const uint rowOffset,		\n" \
		"						const uint lineSize)		\n" \
		"{													\n" \
		"	constant int HIST_BINS = 512;					\n" \
		"	constant int MAX_VAL = +2048;					\n" \
		"	constant int MIN_VAL = -2048;					\n" \
		"	constant int VALUE_DIVIDER = 8;					\n" \
		"	constant int CENTER_INDEX = 256;				\n" \
		"	constant int COLUMN_STEP = 4;					\n" \
		"	constant int MEDIAN_SIZE = 8; // (COLUMN_STEP*2);		\n" \
		"													\n" \
		"	ushort hist_cnts[512] = {0};				\n" \
		"	int hist_sums[512] = {0};					\n" \
		"													\n" \
		"	int row = get_global_id(0) + rowOffset;			\n" \
		"	int idx;										\n" \
		"	int x;											\n" \
		"	int data;										\n" \
		"	int size = numCols / MEDIAN_SIZE;				\n" \
		"													\n" \
		"	global float* p = NULL;							\n" \
		"	float med;										\n" \
		"													\n" \
		"	if (row >= numRows) {							\n" \
		"		return;										\n" \
		"	}												\n" \
		"													\n" \
		"	p = &input[row * lineSize];						\n" \
		"	for (x = 0; x < numCols; x += COLUMN_STEP) {	\n" \
		"		data = (int) p[x];							\n" \
		"		if (data < MIN_VAL) {						\n" \
		"			idx = 0;								\n" \
		"		} else if (data >= MAX_VAL) {				\n" \
		"			idx = HIST_BINS-1;						\n" \
		"		} else {									\n" \
		"			idx = data / VALUE_DIVIDER + CENTER_INDEX;	\n" \
		"		}												\n" \
		"		hist_cnts[idx] += 1;							\n" \
		"		hist_sums[idx] += data;							\n" \
		"	}													\n" \
		"														\n" \
		"	for (idx = 0, data = 0; idx < HIST_BINS; idx++) {	\n" \
		"		data += hist_cnts[idx];							\n" \
		"		if (data >= size) {								\n" \
		"			break;										\n" \
		"		}											\n" \
		"	}												\n" \
		"													\n" \
		"	med = (float)(hist_sums[idx] / hist_cnts[idx]);	\n" \
		"													\n" \
		"	for (x = 0; x < numCols; x++) {					\n" \
		"		p[x] -= med;								\n" \
		"	}												\n" \
		"	return;											\n" \
		"}													\n";

	return p;
}


const char * SigChain::OclKernel::getComplexMagnitudeSource(void)
{
	const char *p =
		"__kernel void complexMagnitude(									\n" \
		"						__global float* reals,						\n" \
		"						__global float* imags,						\n" \
		"						__global float* output,						\n" \
		"						const uint numRows,							\n" \
		"						const uint numCols,							\n" \
		"						const uint rowOffset,						\n" \
		"						const uint lineSize)						\n" \
		"{																	\n" \
		"	const int x = get_global_id(0);									\n" \
		"	const int y = get_global_id(1) + rowOffset;						\n" \
		"	const int idx1 = y * lineSize + x;								\n" \
		"	//const int idx2 = (y + 1) * lineSize - (x + 1);				\n" \
		"	const int idx2 = y * lineSize + numCols - (x + 1);				\n" \
		"	float2 cpl;														\n" \
		"																	\n" \
		"	cpl.x = reals[idx1];											\n" \
		"	cpl.y = imags[idx1];											\n" \
		"																	\n" \
		"	// Flip horizontally.											\n" \
		"	// output[idx2] = length(cpl);									\n" \
		"	output[idx1] = length(cpl);										\n" \
		"	return;															\n" \
		"}																	\n";

	return p;
}


const char * SigChain::OclKernel::getMagnitudeHistogramSource(void)
{
	const char *p =
		"__kernel void magnitudeHistogram(									\n" \
		"						__global float* input,						\n" \
		"						__global uint* histogram,					\n" \
		"						const int numRows,							\n" \
		"						const int numCols,							\n" \
		"						const int rowOffset,						\n" \
		"						const int lineSize,							\n" \
		"						const int columnStep,						\n" \
		"						const int histBins,							\n" \
		"						const int divider)							\n" \
		"{																	\n" \
		"																	\n" \
		"	uint local_hist[256] = {0};										\n" \
		"	int x = get_global_id(0) * columnStep;							\n" \
		"	int y, sum, idx, bin;											\n" \
		"																	\n" \
		"	if (x >= numCols) { 											\n" \
		"		return;														\n" \
		"	}																\n" \
		"																	\n" \
		"	// sum = 0;														\n" \
		"	for (y = 0; y < numRows; y++) {									\n" \
		"		idx = (y + rowOffset) * lineSize + x;						\n" \
		"		bin = (int) input[idx] / divider;							\n" \
		"		bin = min(bin, histBins - 1);								\n" \
		"		local_hist[bin] += 1;										\n" \
		"																	\n" \
		"		//sum += (uint) input[idx];									\n" \
		"	}																\n" \
		"																	\n" \
		"	for (bin = 0; bin < histBins; bin++) {							\n" \
		"		atomic_add(&histogram[bin], local_hist[bin]);				\n" \
		"	}																\n" \
		"																	\n" \
		"	idx = (get_global_id(0) + 1) * histBins;						\n" \
		"	for (bin = 0; bin < histBins; bin++) {							\n" \
		"		histogram[bin + idx] = local_hist[bin];						\n" \
		"	}																\n" \
		"	return;															\n" \
		"}																	\n";
		/*
		"	const int row = get_global_id(0) + rowOffset;					\n" \
		"	int x;															\n" \
		"	uint idx;														\n" \
		"																	\n" \
		"	uint local_hist[HIST_BINS] = {0};								\n" \
		"	global float* p = &input[row * lineSize];						\n" \
		"																	\n" \
		"	for (x = 0; x < numCols; x += COLUMN_STEP) {					\n" \
		"		idx = (uint) p[x] / VALUE_DIVIDER;							\n" \
		"		idx = min(idx, HIST_BINS-1);								\n" \
		"		local_hist[idx] += 1;										\n" \
		"	}																\n" \
		"																	\n" \
		"	for (idx = 0; idx < HIST_BINS; idx++) {							\n" \
		"		atomic_add(&histogram[idx], local_hist[idx]);				\n" \
		"	}																\n" \
		"	return;															\n" \
		"}																	\n";
		*/
	return p;
}


const char * SigChain::OclKernel::getMagnitudeHistogramSource2(void)
{
	const char *p =
		"__kernel void magnitudeHistogram(									\n" \
		"						__global float* input,						\n" \
		"						__global uint* blocks,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowOffset,								\n" \
		"						uint lineSize,								\n" \
		"						uint columnStep,							\n" \
		"						uint blockSize,								\n" \
		"						uint histBins,								\n" \
		"						uint divider,								\n" \
		"						uint sigSize,								\n" \
		"						uint bgdSize)								\n" \
		"{																	\n" \
		"																	\n" \
		"	int gid = get_global_id(0);										\n" \
		"	uint x = gid * columnStep;										\n" \
		"	uint y, idx, pos, bin, val;										\n" \
		"																	\n" \
		"	uint local_hist[8192] = {0};									\n" \
		"	uint sum = 0;													\n" \
		"	uint wgt = 0;													\n" \
		"	uint peak = 0;													\n" \
		"	uint pidx = 0;													\n" \
		"																	\n" \
		"	if (x >= numCols) { 											\n" \
		"		return;														\n" \
		"	}																\n" \
		"																	\n" \
		"	for (y = 0; y < numRows; y++) {									\n" \
		"		idx = (y + rowOffset) * lineSize + x;						\n" \
		"		val = (int) input[idx];										\n" \
		"		bin = val / divider;										\n" \
		"		bin = min(bin, histBins - 1);								\n" \
		"		local_hist[bin] += 1;										\n" \
		"																	\n" \
		"		// sum += (val * y);										\n" \
		"		// wgt += val;												\n" \
		"		if (peak < val) {											\n" \
		"			peak = val;												\n" \
		"			pidx = y;												\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	pos = gid * blockSize;											\n" \
		"	blocks[pos] = pidx ; // (wgt > 0 ? (int)(sum / wgt) : 0);		\n" \
		"	blocks[pos+1] = peak ; // (int)(wgt / numRows);					\n" \
		"																	\n" \
		"	sum = 0;														\n" \
		"	for (bin = 0; bin < histBins; bin++) {							\n" \
		"		sum += local_hist[bin];										\n" \
		"		if (sum >= bgdSize) {										\n" \
		"			blocks[pos+2] = (bin * divider);						\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	sum = 0;														\n" \
		"	for (bin = histBins-1; bin >= 0; bin--) {						\n" \
		"		sum += local_hist[bin];										\n" \
		"		if (sum >= sigSize) {										\n" \
		"			blocks[pos+3] = (bin * divider);						\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	return;															\n" \
		"}																	\n";

	return p;
}


const char * SigChain::OclKernel::getMagnitudeHistogramSource3(void)
{
	const char *p =
		"__kernel void magnitudeHistogram(									\n" \
		"						__global float* input,						\n" \
		"						__global uint* blocks,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowOffset,								\n" \
		"						uint lineSize,								\n" \
		"						uint columnStep,							\n" \
		"						uint blockSize,								\n" \
		"						uint histBins,								\n" \
		"						uint divider,								\n" \
		"						uint sigSize,								\n" \
		"						uint bgdSize)								\n" \
		"{																	\n" \
		"																	\n" \
		"	int gid = get_global_id(0);										\n" \
		"	uint x = gid * columnStep;										\n" \
		"	uint y, pos, bin;												\n" \
		"																	\n" \
		"	uint local_hist[8192] = {0};									\n" \
		"	uint dsum = 0;													\n" \
		"	uint dcnt = 0;													\n" \
		"	uint peak = 0;													\n" \
		"	uint pidx = 0;													\n" \
		"	uint davg = 0;													\n" \
		"	uint didx = 0;													\n" \
		"	uint dval = 0;													\n" \
		"	uint size = 0;													\n" \
		"	int diff = 0;													\n" \
		"																	\n" \
		"	if (x >= numCols) { 											\n" \
		"		return;														\n" \
		"	}																\n" \
		"																	\n" \
		"	for (y = 0; y < numRows; y++) {									\n" \
		"		didx = (y + rowOffset) * lineSize + x;						\n" \
		"		dval = (uint) input[didx];									\n" \
		"		bin = dval / divider;										\n" \
		"		bin = min(bin, histBins - 1);								\n" \
		"		local_hist[bin] += 1;										\n" \
		"																	\n" \
		"		if (peak < dval) {											\n" \
		"			peak = dval;											\n" \
		"			pidx = y;												\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	pos = gid * blockSize;											\n" \
		"	blocks[pos] = pidx ;											\n" \
		"	blocks[pos+1] = peak ;											\n" \
		"																	\n" \
		"	for (bin = 0; bin < histBins; bin++) {							\n" \
		"		size = local_hist[bin];										\n" \
		"		dcnt += size;												\n" \
		"		dsum += (size*(bin+1));										\n" \
		"		if (dcnt >= bgdSize) {										\n" \
		"			// davg = (dsum / dcnt) ;								\n" \
		"			davg = (dsum / dcnt) * divider;							\n" \
		"			didx = bin;												\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	// dsum = 0;													\n" \
		"	// for (bin = 0; bin <= didx; bin++) {							\n" \
		"	//	size = local_hist[bin];										\n" \
		"	// 	diff = ((bin+1) - davg) * divider;							\n" \
		"	//	dsum += ((diff * diff) * size);								\n" \
		"	// }															\n" \
		"	// blocks[pos+2] = (uint)sqrt(dsum / dcnt);						\n" \
		"	// blocks[pos+2] = (dsum / dcnt);								\n" \
		"	blocks[pos+2] = davg;											\n" \
		"																	\n" \
		"	dsum = 0;														\n" \
		"	dcnt = 0;														\n" \
		"	for (bin = histBins-1; bin >= 0; bin--) {						\n" \
		"		size = local_hist[bin];										\n" \
		"		dcnt += size;												\n" \
		"		dsum += (size*(bin+1));										\n" \
		"		if (dcnt >= sigSize) {										\n" \
		"			davg = (dsum / dcnt) * divider;							\n" \
		"			didx = bin;												\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	blocks[pos+3] = davg;											\n" \
		"	return;															\n" \
		"}																	\n";

	return p;
}


const char* SigChain::OclKernel::getMagnitudeHistogramSource4(void)
{
	const char* p =
		"__kernel void magnitudeHistogram(									\n" \
		"						__global float* input,						\n" \
		"						__global uint* blocks,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowOffset,								\n" \
		"						uint lineSize,								\n" \
		"						uint columnStep,							\n" \
		"						uint blockSize,								\n" \
		"						uint histBins,								\n" \
		"						uint divider,								\n" \
		"						uint sigSize,								\n" \
		"						uint bgdSize)								\n" \
		"{																	\n" \
		"																	\n" \
		"	int gid = get_global_id(0);										\n" \
		"	uint x = gid * columnStep;										\n" \
		"	uint y, pos, bin;												\n" \
		"																	\n" \
		"	uint local_hist[8192] = {0};									\n" \
		"	uint peak = 0;													\n" \
		"	uint pidx = 0;													\n" \
		"	uint didx = 0;													\n" \
		"	uint dcnt = 0;													\n" \
		"	float davg = 0.0f;												\n" \
		"	float dval = 0.0f;												\n" \
		"	float dsum = 0.0f;												\n" \
		"	float diff = 0.0f;												\n" \
		"	uint size = 0;													\n" \
		"																	\n" \
		"	if (x >= numCols) { 											\n" \
		"		return;														\n" \
		"	}																\n" \
		"																	\n" \
		"	for (y = 0; y < numRows; y++) {									\n" \
		"		didx = (y + rowOffset) * lineSize + x;						\n" \
		"		dval = (uint)input[didx];									\n" \
		"		bin = dval / divider;										\n" \
		"		bin = min(bin, histBins - 1);								\n" \
		"		local_hist[bin] += 1;										\n" \
		"																	\n" \
		"		if (peak < dval) {											\n" \
		"			peak = dval;											\n" \
		"			pidx = y;												\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	pos = gid * blockSize;											\n" \
		"	blocks[pos] = pidx ;											\n" \
		"	blocks[pos+1] = peak ;											\n" \
		"																	\n" \
		"	for (bin = 0; bin < histBins; bin++) {							\n" \
		"		size = local_hist[bin];										\n" \
		"		dcnt += size;												\n" \
		"		dsum += (size * (bin + 0.5f));								\n" \
		"		if (dcnt >= bgdSize) {										\n" \
		"			dval = (bin + 0.5f) * divider;							\n" \
		"			davg = (dsum / dcnt) * divider;							\n" \
		"			didx = bin;												\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"	blocks[pos+3] = (uint)(dval);									\n" \
		"																	\n" \
		"	dsum = 0.0f;													\n" \
		"	dcnt = 0;														\n" \
		"	for (bin = 0; bin <= didx; bin++) {								\n" \
		"		size = local_hist[bin];										\n" \
		"		dcnt += size;												\n" \
		"		dval = (bin + 0.5f) * divider;								\n" \
		"	 	diff = dval - davg;											\n" \
		"		dsum += ((diff * diff) * size);								\n" \
		"	}																\n" \
		"	blocks[pos+4] = (uint)sqrt(dsum /(float)dcnt);					\n" \
		"																	\n" \
		"	dsum = 0.0f;													\n" \
		"	dcnt = 0;														\n" \
		"	for (bin = histBins-1; bin >= 0; bin--) {						\n" \
		"		size = local_hist[bin];										\n" \
		"		dcnt += size;												\n" \
		"		dsum += (size * (bin + 0.5f));								\n" \
		"		if (dcnt >= sigSize) {										\n" \
		"			dval = (bin + 0.5f) * divider;							\n" \
		"			davg = (dsum / dcnt) * divider;							\n" \
		"			didx = bin;												\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	blocks[pos+2] = (uint)(dval);									\n" \
		"	return;															\n" \
		"}																	\n";

	return p;
}


const char * SigChain::OclKernel::getAdaptiveGrayScaleSource(void)
{
	const char *p =
		"__kernel void makeGrayScaleRange(					\n" \
		"						__global uint* histogram,	\n" \
		"						const uint histBins,		\n" \
		"						const uint divider,			\n" \
		"						const uint bgdSize,			\n" \
		"						const uint sigSize,			\n" \
		"						const float minLogRange)	\n" \
		"{													\n" \
		"	const float LOG_SCALER = 10000.0f;				\n" \
		"													\n" \
		"	const int tid = get_global_id(0);				\n" \
		"	uint size;										\n" \
		"	uint idx;										\n" \
		"	uint sum = 0;									\n" \
		"	uint bins = histBins;							\n" \
		"	local float sigLog ;							\n" \
		"	local float bgdLog ;							\n" \
		"	local uint sigIdx ;								\n" \
		"	local uint bgdIdx ;								\n" \
		"													\n" \
		"	if (tid == 0) {									\n" \
		"		size = bgdSize;								\n" \
		"		for (idx = 0; idx < bins; idx++) {			\n" \
		"			sum += histogram[idx];					\n" \
		"			if (sum >= size) {						\n" \
		"				bgdLog = log10((float)(idx*divider));	\n" \
		"				bgdIdx = idx;						\n" \
		"				break;								\n" \
		"			}										\n" \
		"		}											\n" \
		"	}												\n" \
		"	if (tid == 1) {									\n" \
		"		size = sigSize;								\n" \
		"		for (idx = bins-1; idx >= 0; idx--) {		\n" \
		"			sum += histogram[idx];					\n" \
		"			if (sum >= size) {						\n" \
		"				sigLog = log10((float)(idx*divider));	\n" \
		"				sigIdx = idx;						\n" \
		"				break;								\n" \
		"			}										\n" \
		"		}											\n" \
		"	}												\n" \
		"													\n" \
		"	barrier(CLK_LOCAL_MEM_FENCE);					\n" \
		"													\n" \
		"	if (tid == 0) {									\n" \
		"		sigLog = max(sigLog, (bgdLog + minLogRange));		\n" \
		"		histogram[0] = (uint)(bgdLog * LOG_SCALER);		\n" \
		"		histogram[1] = (uint)(sigLog * LOG_SCALER);		\n" \
		"		histogram[2] = bgdIdx;						\n" \
		"		histogram[3] = sigIdx;						\n" \
		"		histogram[4] = bgdSize;						\n" \
		"		histogram[5] = sigSize;						\n" \
		"	}												\n" \
		"	return;											\n" \
		"}													\n" \
		"													\n" \
		"													\n" \
		"													\n" \
		"__kernel void adaptiveGrayScale(					\n" \
		"						__global float* input,		\n" \
		"						__global uchar* output,		\n" \
		"						__global uint* histogram,	\n" \
		"						const uint numRows,			\n" \
		"						const uint numCols,			\n" \
		"						const uint rowOffset,		\n" \
		"						const uint lineSize,		\n" \
		"						const uint linesNum)		\n" \
		"{													\n" \
		"	const float LOG_SCALER = 10000.0f;				\n" \
		"													\n" \
		"	const int x = get_global_id(0);					\n" \
		"	const int y1 = get_global_id(1);				\n" \
		"	const int y2 = y1 + rowOffset;					\n" \
		"	const int idx1 = (numRows - y1 - 1) * numCols + x ;	\n" \
		"	const int idx2 = y2 * lineSize + x ;			\n" \
		"													\n" \
		"	const float bgdLog = (float)histogram[0]/LOG_SCALER;	\n" \
		"	const float sigLog = (float)histogram[1]/LOG_SCALER;	\n" \
		"													\n" \
		"	if (y2 >= linesNum) {							\n" \
		"		return;										\n" \
		"	}												\n" \
		"	if (x >= numCols) {								\n" \
		"		return;										\n" \
		"	}												\n" \
		"													\n" \
		"	output[idx1] = (uchar)(((log10(input[idx2]) - bgdLog) / (sigLog - bgdLog))*255);		\n" \
		"	// output[idx1] = (uchar)(log10(input[idx2]));		\n" \
		"	return;											\n" \
		"}													\n";

	return p;
}


const char * SigChain::OclKernel::getAdaptiveGrayScaleSource2(void)
{
	const char *p =
		"__kernel void adaptiveGrayScale(									\n" \
		"						__global float* input,						\n" \
		"						__global uchar* output,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowOffset,								\n" \
		"						uint lineSize,								\n" \
		"						uint linesNum,								\n" \
		"						float sigLog,								\n" \
		"						float bgdLog)								\n" \
		"{																	\n" \
		"	const int x = get_global_id(0);									\n" \
		"	const int y1 = get_global_id(1);								\n" \
		"	const int y2 = y1 + rowOffset;									\n" \
		"	const int idx1 = (numRows - y1 - 1) * numCols + x;				\n" \
		"	const int idx2 = y2 * lineSize + x ;							\n" \
		"																	\n" \
		"	if (y2 >= linesNum) {							\n" \
		"		return;										\n" \
		"	}												\n" \
		"	if (x >= numCols) {								\n" \
		"		return;										\n" \
		"	}												\n" \
		"	float value = log10(input[idx2]);				\n" \
		"   value = max(min(value, sigLog), bgdLog);		\n" \
		"	output[idx1] = (uchar)(((value - bgdLog) / (sigLog - bgdLog))*255);		\n" \
		"	// output[idx1] = (uchar)(log10(input[idx2]));		\n" \
		"	return;											\n" \
		"}													\n";
	return p;
}


const char * SigChain::OclKernel::getIntensityAmplitudeSource(void)
{
	const char *p =
		"__kernel void intensityAmplitude(									\n" \
		"						__global float* input,						\n" \
		"						__global float* output,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowOffset,								\n" \
		"						uint lineSize,								\n" \
		"						uint linesNum)								\n" \
		"{																	\n" \
		"	const int x = get_global_id(0);									\n" \
		"	const int y1 = get_global_id(1);								\n" \
		"	const int y2 = y1 + rowOffset;									\n" \
		"	const int idx1 = y1 * numCols + x;				\n" \
		"	const int idx2 = y2 * lineSize + x ;							\n" \
		"																	\n" \
		"	if (y2 >= linesNum) {							\n" \
		"		return;										\n" \
		"	}												\n" \
		"	if (x >= numCols) {								\n" \
		"		return;										\n" \
		"	}												\n" \
		"													\n" \
		"	output[idx1] = input[idx2];						\n" \
		"	// output[idx1] = (uchar)(log10(input[idx2]));		\n" \
		"	return;											\n" \
		"}													\n";
	return p;
}


const char * SigChain::OclKernel::getAxialProjectionSource(void)
{
	const char *p =
		"__kernel void axialProjection(										\n" \
		"						__global uchar* input,						\n" \
		"						__global uchar* output,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowStart,								\n" \
		"						uint rowClose)								\n" \
		"{																	\n" \
		"	const int x = get_global_id(0);									\n" \
		"	int y, idx, sum, size;											\n" \
		"																	\n" \
		"	if (x >= numCols) {												\n" \
		"		return;														\n" \
		"	}																\n" \
		"																	\n" \
		"	sum = 0	;														\n" \
		"	for (y = rowStart; y <= rowClose; y++) {						\n" \
		"		idx = y * numCols + x;										\n" \
		"		sum += input[idx];											\n" \
		"	}																\n" \
		"	size = (rowClose - rowStart + 1);								\n" \
		"	output[x] = (uchar)(sum / size);								\n" \
		"	return;															\n" \
		"}																	\n";

	/*
	const char *p =
		"__kernel void axialProjection(										\n" \
		"						__global uchar* input,						\n" \
		"						__global uchar* output,						\n" \
		"						uint numRows,								\n" \
		"						uint numCols,								\n" \
		"						uint rowStart,								\n" \
		"						uint rowClose)								\n" \
		"{																	\n" \
		"	const int x = get_global_id(0);									\n" \
		"	int y, idx, max_i, max_v, row_s, row_e, sum, cnt, thresh;		\n" \
		"																	\n" \
		"	if (x >= numCols) {												\n" \
		"		return;														\n" \
		"	}																\n" \
		"																	\n" \
		"	max_i = 0; 														\n" \
		"	max_v = 0;														\n" \
		"	for (y = rowStart; y <= rowClose; y++) {						\n" \
		"		idx = y * numCols + x;										\n" \
		"		if (max_v < input[idx]) {									\n" \
		"			max_v = input[idx]; 									\n" \
		"			max_i = idx;											\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	thresh = (int)(max_v / 4);										\n" \
		"	row_s = max(max_i - 125, 0); 									\n" \
		"	row_e = max_i;													\n" \
		"	for (y = row_s; y <= row_e; y++) {								\n" \
		"		idx = y * numCols + x;										\n" \
		"		if (input[idx] >= thresh) {									\n" \
		"			// row_s = y; 												\n" \
		"			// row_e = min(y+75, (int)numRows-1);						\n" \
		"			break;													\n" \
		"		}															\n" \
		"	}																\n" \
		"																	\n" \
		"	sum = 0; 														\n" \
		"	cnt = 0;														\n" \
		"	for (y = row_s; y <= row_e; y++) {								\n" \
		"		idx = y * numCols + x;										\n" \
		"		sum += input[idx];											\n" \
		"		cnt += 1;													\n" \
		"	}																\n" \
		"	output[x] = max_v;											\n" \
		"	return;															\n" \
		"}																	\n";
	*/
	return p;

}
