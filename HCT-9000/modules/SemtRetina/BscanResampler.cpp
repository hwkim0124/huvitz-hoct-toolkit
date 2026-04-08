#include "stdafx.h"
#include "BscanResampler.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BscanResampler::BscanResamplerImpl
{
	BscanImageData source;
	BscanImageData sample;
	BscanImageData coarse;

	int sampleIndex = 0;

	BscanResamplerImpl()
	{
	}
};


BscanResampler::BscanResampler() :
	d_ptr(make_unique<BscanResamplerImpl>())
{
}


SemtRetina::BscanResampler::~BscanResampler() = default;
SemtRetina::BscanResampler::BscanResampler(BscanResampler&& rhs) = default;
BscanResampler& SemtRetina::BscanResampler::operator=(BscanResampler&& rhs) = default;


bool SemtRetina::BscanResampler::runResampling(OctScanImage image, int width, int height)
{
	if (image.isEmpty()) {
		return false;
	}

	auto& sample = impl().sample;
	auto& source = impl().source;
	auto& coarse = impl().coarse;

	auto dptr = image.getData();
	auto src_w = image.getWidth();
	auto src_h = image.getHeight();
	auto index = image.getIndex();

	const auto GUIDED_RADIUS = 5;
	const auto GUIDED_EPSILON = 0.05;

	const auto GUIDED_RADIUS2 = 3;
	const auto GUIDED_EPSILON2 = 0.05;

	impl().sampleIndex = index;
	if (source.fromBitsData(dptr, src_w, src_h)) {
		
		// CppUtil::ClockTimer::start();
		if (width != src_w || height != src_h) {
			Size size(width, height);
			if (width > src_w || height > src_h) {
				// For upsampling, use cubic interpolation to avoid blocky effect.
				cv::resize(source.getCvMatConst(), sample.getCvMat(), size, 0.0, 0.0, INTER_CUBIC);
			}
			else {
				// For downsampling, use area interpolation with best integrity.
				cv::resize(source.getCvMatConst(), sample.getCvMat(), size, 0.0, 0.0, INTER_AREA);
			}
		}
		else {
			sample.getCvMat() = source.getCvMatConst();
		}

		//LogD() << "resize elapsed: " << CppUtil::ClockTimer::elapsedMsec();

		cv::resize(sample.getCvMatConst(), coarse.getCvMat(), Size(COARSE_IMAGE_WIDTH, COARSE_IMAGE_HEIGHT), 0.0, 0.0, INTER_AREA);
		coarse.applyGuidedFilter(COARSE_GUIDED_RADIUS, COARSE_GUIDED_EPSILON);
		//cv::resize(coarse.getCvMatConst(), coarse.getCvMat(), Size(512, 768), 0.0, 0.0, INTER_AREA);
		
		// sample.copyTo(&coarse);
		// coarse.applyGuidedFilter(GUIDED_RADIUS, GUIDED_EPSILON);
		/*
		LogD() << "resize elapsed2: " << CppUtil::ClockTimer::elapsedMsec();
		coarse.applyGuidedFilter(GUIDED_RADIUS, GUIDED_EPSILON);

		LogD() << "guided elapsed2: " << CppUtil::ClockTimer::elapsedMsec();

		sample.copyTo(&coarse);
		coarse.applyGuidedFilter(GUIDED_RADIUS, GUIDED_EPSILON);
		*/
		//LogD() << "guided elapsed: " << CppUtil::ClockTimer::elapsedMsec();

		sample.estimateStatitics();
		coarse.estimateStatitics();
		coarse.estimateColSnRatios();
		
		coarse.resizeToMatchSample(MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT);
		// coarse.upscaleStatistics(sample.getWidth(), sample.getHeight());

		//LogD() << "ratio elapsed: " << CppUtil::ClockTimer::elapsedMsec();
		// string path = format_string("corased_%d.jpg", image.getIndex());
		// coarse.saveFile(path);
		return true;
	}
	return false;
}


bool SemtRetina::BscanResampler::checkRetinaSegmentable(void) const
{
	const float OBJ_SNR_MIN = 18.0f; //  25.0f; // 33.0f;
	const float OBJ_RATIO_MIN = 0.45f; // 0.6f;

	const auto& image = impl().coarse;
	auto snrs = image.columnSnRatios();

	int obj_count = 0;
	int all_count = (int)snrs.size() + 1;
	auto ssum = 0.0f;

	for (int i = 0; i < snrs.size(); i++) {
		if (snrs[i] >= OBJ_SNR_MIN) {
			obj_count += 1;
		}
		ssum += snrs[i];
	}

	float ratio = (float)obj_count / (float)all_count;
	float mean = image.imageMean();
	float stddev = image.imageStdev();
	float snr_avg = ssum / snrs.size();

	LogD() << "Sampling index: " << impl().sampleIndex << ", mean: " << mean << ", stdev: " << stddev << ", snr_avg: " << snr_avg << ", object ratios : " << ratio;
	if (ratio < OBJ_RATIO_MIN) {
		return false;
	}
	return true;
}


int SemtRetina::BscanResampler::sourceWidth(void) const
{
	auto width = imageSource()->getWidth();
	return width;
}

int SemtRetina::BscanResampler::sourceHeight(void) const
{
	auto height = imageSource()->getHeight();
	return height;
}

int SemtRetina::BscanResampler::sampleWidth(void) const
{
	auto width = imageSample()->getWidth();
	return width;
}

int SemtRetina::BscanResampler::sampleHeight(void) const
{
	auto height = imageSample()->getHeight();
	return height;
}

int SemtRetina::BscanResampler::sampleIndex(void) const
{
	auto index = impl().sampleIndex;
	return index;
}

BscanImageData* SemtRetina::BscanResampler::imageSource(void) const
{
	return &(impl().source);
}

BscanImageData* SemtRetina::BscanResampler::imageSample(void) const
{
	return &(impl().sample);
}

BscanImageData* SemtRetina::BscanResampler::imageCoarse(void) const
{
	return &(impl().coarse);
}


BscanResampler::BscanResamplerImpl& SemtRetina::BscanResampler::impl(void) const
{
	return *d_ptr;
}
