#include "stdafx.h"
#include "InferenceResult.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct InferenceResult::InferenceResultImpl
{
	std::vector<float> buffer;

	int width = 0;
	int height = 0;
	int channels = 0;
	cv::Mat mask;

	InferenceResultImpl()
	{
	}
};

InferenceResult::InferenceResult() :
	d_ptr(make_unique<InferenceResultImpl>())
{
}

InferenceResult::InferenceResult(int width, int height, int channels) :
	d_ptr(make_unique<InferenceResultImpl>())
{
	getImpl().width = width;
	getImpl().height = height;
	getImpl().channels = channels;
}

SemtSegm::InferenceResult::~InferenceResult() = default;
SemtSegm::InferenceResult::InferenceResult(InferenceResult && rhs) = default;
InferenceResult & SemtSegm::InferenceResult::operator=(InferenceResult && rhs) = default;

bool SemtSegm::InferenceResult::postprocess(const float * outBuffer, int width, int height, int channels, bool maskLabel)
{
	getImpl().width = width;
	getImpl().height = height;
	getImpl().channels = channels;

	if (maskLabel) {
		getImpl().mask = cv::Mat::zeros(height, width, CV_32FC1);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				float maxProb = -1.0f;
				int classId = 0;
				for (int c = 0; c < channels; c++) {
					float prob = outBuffer[c * width * height + y * width + x];
					if (prob > maxProb) {
						classId = c;
						maxProb = prob;
					}
				}

				getImpl().mask.at<float_t>(y, x) = classId * 32;

				// getImpl().mask.at<float_t>(y, x) = 255.0f * outBuffer[2 * width * height + y * width + x];
			}
		}
	}

	/*
	int w = width;
	int h = height;
	for (int y = 0; y < 120; y++) {
		for (int x = 20; x <= 20; x++) {
			LogD() << y << " " << outBuffer[0 * w * h + y * w + x] << " " << outBuffer[1 * w * h + y * w + x] << " " << outBuffer[2 * w * h + y * w + x] << " " << outBuffer[3 * w * h + y * w + x] << " " << outBuffer[4 * w * h + y * w + x] << " " << outBuffer[5 * w * h + y * w + x];
		}
	}
	*/

	// int s = width * height * channels;
	// LogD() << outBuffer[0] << " " << outBuffer[1] << " " << outBuffer[2] << " " << outBuffer[3] << " " << outBuffer[4] << " " << outBuffer[5] << " " << outBuffer[6] << " " << outBuffer[7] << " " << outBuffer[8];
	// LogD() << outBuffer[s-1] << " " << outBuffer[s-2] << " " << outBuffer[s-3] << " " << outBuffer[s-4] << " " << outBuffer[s-5] << " " << outBuffer[s-6] << " " << outBuffer[s-7] << " " << outBuffer[s-8] << " " << outBuffer[s-9];

	auto buff_size = width * height * channels;
	getImpl().buffer = vector<float>(outBuffer, outBuffer + buff_size);
	return true;
}

bool SemtSegm::InferenceResult::saveImage(int index) const
{
	auto filename = "./r" + to_string(index) + ".jpg";

	OcularImage image;
	image.getCvMat() = getImpl().mask;
	image.saveFile(filename);
	return true;
}

bool SemtSegm::InferenceResult::exists(void) const
{
	if (getImpl().buffer.empty()) {
		return false;
	}

	auto buff_size = getImpl().buffer.size();
	auto data_size = getImpl().width * getImpl().height * getImpl().channels;
	if (buff_size != data_size) {
		return false;
	}

	return true;
}

const float * SemtSegm::InferenceResult::getBuffer() const
{
	if (getImpl().buffer.empty()) {
		return nullptr;
	}
	return &getImpl().buffer[0];
}

int SemtSegm::InferenceResult::getWidth() const
{
	return getImpl().width;
}

int SemtSegm::InferenceResult::getHeight() const
{
	return getImpl().height;
}

int SemtSegm::InferenceResult::getChannels() const
{
	return getImpl().channels;
}

InferenceResult::InferenceResultImpl & SemtSegm::InferenceResult::getImpl(void) const
{
	return *d_ptr;
}


