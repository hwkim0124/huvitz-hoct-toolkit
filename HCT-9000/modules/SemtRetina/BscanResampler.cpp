#include "stdafx.h"
#include "BscanResampler.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BscanResampler::BscanResamplerImpl
{
	const RetinaSegmenter* segm = nullptr;

	BscanImageData source;
	BscanImageData sample;
	BscanImageData coarse;

	BscanImageData srcRiseEdge;
	BscanImageData srcRiseEdgeHigh;
	BscanImageData srcFallEdge;
	BscanImageData srcFallEdgeHigh;

	int sampleIndex = 0;
	bool isAngio = false;
	float rangeX = 0.0f;
	float rangeY = 0.003f * 768.0f; // 0.003 mm/pixel * 768 pixels = 2.304 mm

	BscanResamplerImpl()
	{
	}
};


BscanResampler::BscanResampler(RetinaSegmenter* segm) :
	d_ptr(make_unique<BscanResamplerImpl>())
{
	impl().segm = segm;
}


SemtRetina::BscanResampler::~BscanResampler() = default;
SemtRetina::BscanResampler::BscanResampler(BscanResampler&& rhs) = default;
BscanResampler& SemtRetina::BscanResampler::operator=(BscanResampler&& rhs) = default;


bool SemtRetina::BscanResampler::runResampling(OctScanImage image, bool angio)
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

	impl().isAngio = angio;
	impl().sampleIndex = index;
	impl().rangeX = image.getRangeX();

	if (source.fromBitsData(dptr, src_w, src_h)) {
		auto target_w = (angio ? SAMPLE_WIDTH_S : SAMPLE_WIDTH_M);
		auto target_h = (angio ? SAMPLE_HEIGHT_S : SAMPLE_HEIGHT_M);
		
		if (target_w != src_w || target_h != src_h) {
			Size size(target_w, target_h);
			if (target_w > src_w || target_h > src_h) {
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

		auto coarse_w = target_w;
		auto coarse_h = target_h;
		auto radius = (angio ? COARSE_GUIDED_RADIUS_S : COARSE_GUIDED_RADIUS_M);
		auto epsilon = (angio ? COARSE_GUIDED_EPSILON_S : COARSE_GUIDED_EPSILON_M);

		if (coarse_w != target_w || coarse_h != target_h) {
			cv::resize(sample.getCvMatConst(), coarse.getCvMat(), Size(coarse_w, coarse_h), 0.0, 0.0, INTER_AREA);
		}
		else {
			sample.copyTo(&coarse);
		}
		coarse.applyGuidedFilter(radius, epsilon);

		sample.estimateStatitics();
		coarse.estimateStatitics();
		coarse.estimateColSnRatios();

		// string path = format_string("corased_%d.jpg", image.getIndex());
		// coarse.saveFile(path);

		float scaleX = sampleScaleRatioX();
		float scaleY = sampleScaleRatioY();
		float spaceX = samplePixelSpaceX();
		float spaceY = samplePixelSpaceY();

		auto* crt = impl().segm->retinaSegmCriteria();
		crt->setSampleScaleFactors(scaleX, scaleY);
		crt->setSampleDimensions(target_w, target_h, spaceX, spaceY);

		spaceX = sourcePixelSpaceX();
		spaceY = sourcePixelSpaceY();
		crt->setSourceDimensions(src_w, src_h, spaceX, spaceY);

		createSourceEdgeMaps();
		return true;
	}
	return false;
}


bool SemtRetina::BscanResampler::checkRetinaSegmentable(void) const
{
	const float OBJ_SNR_MIN = COLUMN_OBJ_SN_RATIO_MIN; // 18.0f; //  25.0f; // 33.0f;
	const float OBJ_RATIO_MIN = COLUMN_VALID_RATIO_MIN; // 0.6f;

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

	/*
	auto rows = image.rowMeans();
	auto ysum = 0;
	auto wsum = 0;
	for (int y = 0; y < rows.size(); y++) {
		auto w = rows[y];
		w = w * w;
		ysum += w * y;
		wsum += w;
	}

	if (wsum > 0) {
		auto ycen = (int)(ysum / wsum + 0.5f);
		auto height = image.getHeight();
		auto limit = (int)(height * 0.15f);
		if (ycen < limit) {
			LogD() << "Sampling index: " << impl().sampleIndex << ", retina center y: " << ycen << ", height: " << image.getHeight();
			return false;
		}
	}
	*/
	return true;
}

bool SemtRetina::BscanResampler::createSourceEdgeMaps(void) const
{
	auto* image = imageSource();
	auto* sample = imageSample();
	const int KERNEL_ROWS = 15;
	const int KERNEL_COLS = 5;

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(-1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(+1.0f);

	{
		auto mean = sample->imageMean();
		auto stdv = sample->imageStdev();
		int gmax = (int)(mean + stdv * 2.0f);

		Mat matDst, matGrad, matOut;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		// matDst.setTo(gmax, (matDst > gmax));
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0f, 1.0f, cv::NORM_MINMAX);
		impl().srcRiseEdge.getCvMat() = matOut;
	}
	/*
	{
		auto mean = sample->imageMean();
		auto stdv = sample->imageStdev();
		int gmin = (int)(mean + stdv * 1.0f);

		Mat matDst, matGrad, matOut;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		matDst.setTo(gmin, (matDst < gmin));
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0f, 1.0f, cv::NORM_MINMAX);
		impl().srcRiseEdgeHigh.getCvMat() = matOut;
	}
	*/
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(+1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(-1.0f);

	{
		auto mean = sample->imageMean();
		auto stdv = sample->imageStdev();
		int gmax = (int)(mean + stdv * 2.0f);

		Mat matDst, matGrad, matOut;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		// matDst.setTo(gmax, (matDst > gmax));
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0f, 1.0f, cv::NORM_MINMAX);
		impl().srcFallEdge.getCvMat() = matOut;
	}

	/*
	{
		auto mean = sample->imageMean();
		auto stdv = sample->imageStdev();
		int gmin = (int)(mean + stdv * 1.0f);

		Mat matDst, matGrad, matOut;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		matDst.setTo(gmin, (matDst < gmin));
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0f, 1.0f, cv::NORM_MINMAX);
		impl().srcFallEdgeHigh.getCvMat() = matOut;
	}
	*/
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

bool SemtRetina::BscanResampler::isAngio(void) const
{
	auto angio = impl().isAngio;
	return angio;
}

float SemtRetina::BscanResampler::sourcePixelSpaceX(void) const
{
	float src_w = (float)sourceWidth();
	float range = impl().rangeX;
	if (src_w > 0.0f && range > 0.0f) {
		return range / src_w;
	}
	return 0.0f;
}

float SemtRetina::BscanResampler::sourcePixelSpaceY(void) const
{
	float src_h = (float)sourceHeight();
	float range = impl().rangeY;
	if (src_h > 0.0f && range > 0.0f) {
		return range / src_h;
	}
	return 0.0f;
}

float SemtRetina::BscanResampler::samplePixelSpaceX(void) const
{
	float samp_w = (float)sampleWidth();
	float range = impl().rangeX;
	if (samp_w > 0.0f && range > 0.0f) {
		return range / samp_w;
	}
	return 0.0f;
}

float SemtRetina::BscanResampler::samplePixelSpaceY(void) const
{
	float samp_h = (float)sampleHeight();
	float range = impl().rangeY;
	if (samp_h > 0.0f && range > 0.0f) {
		return range / samp_h;
	}
	return 0.0f;
}

float SemtRetina::BscanResampler::sampleScaleRatioX(void) const
{
	float src_w = (float)sourceWidth();
	float samp_w = (float)sampleWidth();
	if (src_w > 0.0f) {
		return samp_w / src_w;
	}
	return 0.0f;
}

float SemtRetina::BscanResampler::sampleScaleRatioY(void) const
{
	float src_h = (float)sourceHeight();
	float samp_h = (float)sampleHeight();
	if (src_h > 0.0f) {
		return samp_h / src_h;
	}
	return 0.0f;
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

BscanImageData* SemtRetina::BscanResampler::sourceRiseEdge(void) const
{
	return &(impl().srcRiseEdge);
}

BscanImageData* SemtRetina::BscanResampler::sourceRiseEdgeHigh(void) const
{
	return &(impl().srcRiseEdgeHigh);
}

BscanImageData* SemtRetina::BscanResampler::sourceFallEdge(void) const
{
	return &(impl().srcFallEdge);
}

BscanImageData* SemtRetina::BscanResampler::sourceFallEdgeHigh(void) const
{
	return &(impl().srcFallEdgeHigh);
}


BscanResampler::BscanResamplerImpl& SemtRetina::BscanResampler::impl(void) const
{
	return *d_ptr;
}
