#include "stdafx.h"
#include "Classifier.h"
#include "Sampling.h"
#include "BscanSegmentator.h"

#include "InferenceModel.h"
#include "InferenceInput.h"
#include "InferenceResult.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace CppUtil;
using namespace SemtSegm;
using namespace std;
using namespace cv;

#include <algorithm>

struct Classifier::ClassifierImpl
{
	BscanSegmentator* pSegm = nullptr;

	OcularImage image;
	OcularImage denoised;

	OcularImage mapVIT;
	OcularImage mapNFL;
	OcularImage mapOPL;
	OcularImage mapONL;
	OcularImage mapRPE;
	OcularImage mapCHO;
	int index;

	std::vector<int> shifts;
	InferenceInput input;

	ClassifierImpl()
	{
	}
};



Classifier::Classifier(BscanSegmentator* pSegm) :
	d_ptr(make_unique<ClassifierImpl>()) 
{
	getImpl().pSegm = pSegm;
}


SemtSegm::Classifier::~Classifier() = default;
SemtSegm::Classifier::Classifier(Classifier && rhs) = default;
Classifier & SemtSegm::Classifier::operator=(Classifier && rhs) = default;

bool SemtSegm::Classifier::prepareInput()
{
	if (!getImpl().pSegm) {
		return false;
	}

	const auto* source = getImpl().pSegm->getSampling()->source();
	const auto* sample = getImpl().pSegm->getSampling()->sample();

	OcularImage cinput;

	if (source->getWidth() == SOURCE_WIDTH && source->getHeight() == SOURCE_HEIGHT) {
		cinput.getCvMat() = source->getCvMatConst();
	}
	else {
		Size size(SOURCE_WIDTH, SOURCE_HEIGHT);
		cv::resize(source->getCvMatConst(), cinput.getCvMat(), size, 0.0, 0.0, INTER_LINEAR);
	}

	// const auto* sample = getImpl().pSegm->getSampling()->sample();
	const auto& inners = getImpl().pSegm->getLayerInner()->getOptimalPath();
	int index = getImpl().pSegm->getImageIndex();

	auto layer = inners;
	float y_ratio = 1.0f;

	if (sample->getHeight() != SOURCE_HEIGHT) {
		y_ratio = (float)SOURCE_HEIGHT / sample->getHeight();
		std::transform(begin(layer), end(layer), begin(layer), [=](int elem) { return elem * y_ratio; });
	}

	if (!prepareInput(&cinput, layer, index)) {
		return false;
	}
	return true;
}

bool SemtSegm::Classifier::prepareInput(const OcularImage * image, const std::vector<int>& inner, int index)
{
	if (!createAlignImage(image, inner)) {
		return false;
	}

	const auto* align = &getImpl().image;
	if (!getImpl().input.setInputImage(align)) {
		return false;
	}

	getImpl().index = index;
	return true;
}

bool SemtSegm::Classifier::classifyPixels(bool save)
{
	if (!getImpl().input.requestPrediction(save)) {
		return false;
	}

	if (!composeLayerMaps()) {
		return false;
	}

	if (save) {
		int index = getImpl().index;
		getImpl().input.getPredictionResult().saveImage(index);
		getImpl().input.saveImage(index);
	}
	return true;
}

int SemtSegm::Classifier::getAlignTopPosition() const
{
	return SOURCE_ROI_TOP;
}

const std::vector<int>& SemtSegm::Classifier::getAlignShifts() const
{
	return getImpl().shifts;
}

const OcularImage * SemtSegm::Classifier::getInputImage(void) const
{
	return &getImpl().image;
}

const OcularImage * SemtSegm::Classifier::getInputDenoised(void) const
{
	return &getImpl().denoised;
}

const OcularImage * SemtSegm::Classifier::getScoreVIT() const
{
	return &getImpl().mapVIT;
}

const OcularImage * SemtSegm::Classifier::getScoreNFL() const
{
	return &getImpl().mapNFL;
}

const OcularImage * SemtSegm::Classifier::getScoreOPL() const
{
	return &getImpl().mapOPL;
}

const OcularImage * SemtSegm::Classifier::getScoreONL() const
{
	return &getImpl().mapONL;
}

const OcularImage * SemtSegm::Classifier::getScoreRPE() const
{
	return &getImpl().mapRPE;
}

const OcularImage * SemtSegm::Classifier::getScoreCHO() const
{
	return &getImpl().mapCHO;
}

bool SemtSegm::Classifier::createAlignImage(const OcularImage * image, const std::vector<int>& inner)
{
	if (image->isEmpty()) {
		return false;
	}

	cv::Mat matImg = image->getCvMatConst();
	int width = image->getWidth();
	int height = image->getHeight();

	const int ROI_W = INPUT_WIDTH;
	const int ROI_H = INPUT_HEIGHT;
	const int ROI_UPPER_MARGIN = SOURCE_ROI_TOP;

	cv::Mat matRoi = cv::Mat::zeros(ROI_H, ROI_W, CV_8UC1);

	if (width != ROI_W || width != inner.size()) {
		// it should be resized before. 
		return false;
	}

	auto shift = vector<int>(width, 0);

	int r, c;
	for (c = 0; c < width; c++) {
		int from_y = inner[c] - ROI_UPPER_MARGIN;
		int dest_y = (from_y < 0 ? from_y * -1 : 0);
		int draw_y = max(from_y, 0);

		for (r = draw_y; r < height; r++) {
			matRoi.at<uchar>(dest_y, c) = matImg.at<uchar>(r, c);
			if (++dest_y >= ROI_H) {
				break;
			}
		}

		shift[c] = from_y * -1;
	}

	getImpl().shifts = shift;
	getImpl().image.getCvMat() = matRoi;

	getImpl().denoised.getCvMat() = matRoi.clone();
	getImpl().denoised.applyGuidedFilter(1);
	getImpl().denoised.updateImageStats();
	return true;
}

bool SemtSegm::Classifier::composeLayerMaps()
{
	const auto& result = getImpl().input.getPredictionResult();
	if (!result.exists()) {
		return false;
	}

	const float* p = result.getBuffer();
	const int width = result.getWidth();
	const int height = result.getHeight();
	const int channels = result.getChannels();
	const int elem_size = sizeof(float);
	const int row_steps = width * elem_size;
	const int plane_size = width * height;

	getImpl().mapVIT.getCvMat() = cv::Mat(height, width, CV_32FC1, (void*)(p + plane_size*1), row_steps);
	getImpl().mapNFL.getCvMat() = cv::Mat(height, width, CV_32FC1, (void*)(p + plane_size*2), row_steps);
	getImpl().mapOPL.getCvMat() = cv::Mat(height, width, CV_32FC1, (void*)(p + plane_size*3), row_steps);
	getImpl().mapONL.getCvMat() = cv::Mat(height, width, CV_32FC1, (void*)(p + plane_size*4), row_steps);
	getImpl().mapRPE.getCvMat() = cv::Mat(height, width, CV_32FC1, (void*)(p + plane_size*5), row_steps);
	getImpl().mapCHO.getCvMat() = cv::Mat(height, width, CV_32FC1, (void*)(p + plane_size*6), row_steps);
	/*
	uchar* p2 = (uchar*)p;
	float a = *(float*)(&p2[plane_size * 3 * elem_size + row_steps * 100 + 100 * elem_size]);
	float b = getImpl().mapOPL.getCvMat().at<float>(100, 100);
	float a2 = *(float*)(&p2[plane_size * 3 * elem_size + row_steps * 100 + 108 * elem_size]);
	float b2 = getImpl().mapOPL.getCvMat().at<float>(100, 108);
	*/

	/*
	int c = 36;
	int r1 = 40;
	int r2 = 120;
	auto v1 = vector<float>();
	for (int r = r1; r < r2; r++) {
		float a = getImpl().mapNFL.getCvMat().at<float>(r, c);
		v1.push_back(a);
	}

	auto v2 = vector<float>();
	for (int r = r1; r < r2; r++) {
		float a = getImpl().mapOPL.getCvMat().at<float>(r, c);
		v2.push_back(a);
	}

	auto v3 = vector<float>();
	for (int r = r1; r < r2; r++) {
		float a = getImpl().mapONL.getCvMat().at<float>(r, c);
		v3.push_back(a);
	}

	auto v4 = vector<float>();
	for (int r = r1; r < r2; r++) {
		float a = getImpl().mapRPE.getCvMat().at<float>(r, c);
		v4.push_back(a);
	}

	auto v5 = vector<float>();
	for (int r = r1; r < r2; r++) {
		float a = getImpl().mapCHO.getCvMat().at<float>(r, c);
		v5.push_back(a);
	}

	for (int i = 0; i < v1.size(); i++) {
		LogD() << c << ", " << (r1 + i) << " : " << v1[i] << ", " << v2[i] << ", " << v3[i] << ", " << v4[i] << ", " << v5[i];
	}
	*/
	return true;
}


Classifier::ClassifierImpl & SemtSegm::Classifier::getImpl(void) const
{
	return *d_ptr;
}
