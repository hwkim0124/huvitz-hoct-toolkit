#include "stdafx.h"
#include "InferenceInput.h"
#include "InferenceModel.h"
#include "InferenceResult.h"

#include <inference_engine.hpp>

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;
using namespace InferenceEngine;

struct InferenceInput::InferenceInputImpl
{
	const OcularImage* input;
	cv::Mat matROI;

	Blob::Ptr inputBlob;
	InferenceResult result;

	InferenceInputImpl()
	{
	}
};

InferenceInput::InferenceInput() :
	d_ptr(make_unique<InferenceInputImpl>())
{
}

SemtSegm::InferenceInput::~InferenceInput() = default;
SemtSegm::InferenceInput::InferenceInput(InferenceInput && rhs) = default;
InferenceInput & SemtSegm::InferenceInput::operator=(InferenceInput && rhs) = default;

bool SemtSegm::InferenceInput::setInputImage(const OcularImage * image)
{
	if (image->isEmpty()) {
		return false;
	}

	getImpl().input = image;
	return true;
}

bool SemtSegm::InferenceInput::requestPrediction(bool maskLabel)
{
	if (!wrapImageToBlobFp32()) {
		return false;
	}

	InferenceResult result;
	if (!InferenceModel::requestPrediction(*this, result, maskLabel)) {
		return false;
	}

	getImpl().result = move(result);
	return true;
}

const InferenceResult & SemtSegm::InferenceInput::getPredictionResult() const
{
	return getImpl().result;
}

bool SemtSegm::InferenceInput::saveImage(int index) const
{
	auto filename = "./" + to_string(index) + ".jpg";

	((OcularImage*)const_cast<OcularImage*>(getImpl().input))->saveFile(filename, false, 100);
	return true;
}

std::shared_ptr<InferenceEngine::Blob> SemtSegm::InferenceInput::getInputBlob() const
{
	return getImpl().inputBlob;
}

bool SemtSegm::InferenceInput::wrapImageToBlobFp32()
{
	if (getImpl().input->isEmpty()) {
		return false;
	}

	/*
	cv::Mat mat;
	getImpl().image->getCvMatConst().convertTo(mat, CV_32FC1);
	getImpl().matRoi = mat;
	*/

	// cv::Mat mat; 
	// getImpl().input->getCvMatConst().convertTo(mat, CV_32FC1);
	auto& mat = getImpl().matROI;
	getImpl().input->getCvMatConst().convertTo(mat, CV_32FC1);

	size_t channels = mat.channels();
	size_t height = mat.size().height;
	size_t width = mat.size().width;

	size_t strideH = mat.step.buf[0];
	size_t strideW = mat.step.buf[1];
	size_t elemSize = mat.elemSize();
	auto data = reinterpret_cast<float*>(mat.data);

	/*
	double minVal;
	double maxVal;
	Point minLoc;
	Point maxLoc;

	minMaxLoc(mat, &minVal, &maxVal, &minLoc, &maxLoc);
	*/

	bool is_dense =
		strideW == channels * elemSize &&
		strideH == channels * width * elemSize;

	if (!is_dense) THROW_IE_EXCEPTION
		<< "Doesn't support conversion from not dense cv::Mat";

	InferenceEngine::TensorDesc tDesc(InferenceEngine::Precision::FP32,
		{ 1, channels, height, width },
		InferenceEngine::Layout::NCHW);

	/** Fill input tensor with planes. First b channel, then g and r channels **/
	// assuming input precision was asked to be U8 in prev step
	auto input = InferenceEngine::make_shared_blob<float>(tDesc, data);
	getImpl().inputBlob = input;
	// input->allocate();

	/*
	{
		auto const memLocker = input->cbuffer(); // use const memory locker
		const float *output_buffer = memLocker.as<const float *>();

		int c = 20;
		for (int r = 0; r < 120; r++) {
			LogD() << r << " : " << output_buffer[r * 256 + c];
		}
	}
	*/
	return true;
}


InferenceInput::InferenceInputImpl & SemtSegm::InferenceInput::getImpl(void) const
{
	return *d_ptr;
}


