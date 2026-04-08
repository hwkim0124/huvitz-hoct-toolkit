#include "stdafx.h"
#include "AngioPost.h"
#include "AngioLayout.h"
#include "AngioLayers.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;

#include <iterator>
#include <algorithm>
#include <numeric>
#include <fftw3.h>


struct AngioPost::AngioPostImpl
{
	int filterOrients = ANGIO_GABOR_FILTER_ORIENTS; // 10;
	float filterSigma = ANGIO_GABOR_FILTER_SIGMA;
	float filterDivider = ANGIO_GABOR_FILTER_DIVIDER; // 1.0f;// 1.50f;
	float filterWeight = ANGIO_GABOR_FILTER_WEIGHT; // 0.50f; // 5f; // 0.5f; // 0.65f; // 0.35f; // 0.50f;

	AngioPostImpl() {
	};
};


AngioPost::AngioPost()
	: d_ptr(make_unique<AngioPostImpl>())
{
}


AngioPost::~AngioPost()
{
}


OctAngio::AngioPost::AngioPost(AngioPost && rhs) = default;
AngioPost & OctAngio::AngioPost::operator=(AngioPost && rhs) = default;


OctAngio::AngioPost::AngioPost(const AngioPost & rhs)
	: d_ptr(make_unique<AngioPostImpl>(*rhs.d_ptr))
{
}


AngioPost & OctAngio::AngioPost::operator=(const AngioPost & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctAngio::AngioPost::createProjectionMask(const AngioLayout & layout, const std::vector<float>& profile, std::vector<float>& mask, bool kernel)
{
	int width = layout.getWidth();
	int height = layout.getHeight();

	if (profile.empty() || profile.size() != (width * height)) {
		return false;
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);

	if (kernel) {
		image.applyGaussianBlur(1.0f);
	}

	mask = image.copyDataInFloats();
	return true;
}


bool OctAngio::AngioPost::performPostProcessing(int width, int height, std::vector<float>& profile, bool outFlows)
{
	if (profile.empty() || profile.size() != (width * height)) {
		return false;
	}
	if (all_of(profile.begin(), profile.end(), [](float i) { return i <= 0; })) {
		return true;
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);
	image.resize(width, height);

	int orients = getImpl().filterOrients;
	float sigma = getImpl().filterSigma;
	float divider = getImpl().filterDivider;
	float weight = getImpl().filterWeight;

	LogD() << "Performing post processing, orients: " << orients << ", sigma: " << sigma << ", divider: " << divider << ", weight: " << weight;

	if (weight > 0.0f) {
		CvImage res1, res2;

		image.copyTo(&res1);
		res1.applyGaborFilter(5, 50, 10, orients, divider, sigma);
		image.applyWeighted(&res1, (1.0f - weight), weight);
	}

	if (true)
	{
		CvImage res1, res2;
		// image.applyGuidedFilter();
		// image.applyMedianBlur(3);
	
		image.copyTo(&res2);
		if (outFlows) {
			res2.applyGaussianBlur(3.0);
			image.applyWeighted(&res2, 1.7, -0.7);
		}
		else {
			res2.applyGaussianBlur(1.5);
			image.applyWeighted(&res2, 1.7, -0.7);
		}
	}

	profile = image.copyDataInFloats();
	for_each(profile.begin(), profile.end(), [&](float &elem) { elem = max(elem, 0.0f); });
	return true;
}

bool OctAngio::AngioPost::performVesselProcessing(int width, int height, std::vector<float>& profile)
{
	if (profile.empty() || profile.size() != (width * height)) {
		return false;
	}
	if (all_of(profile.begin(), profile.end(), [](float i) { return i <= 0; })) {
		return true;
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);
	image.resize(width, height);


	int orients = getImpl().filterOrients;
	float sigma = getImpl().filterSigma;
	float divider = getImpl().filterDivider;
	float weight = getImpl().filterWeight;

	LogD() << "Performing vessel processing, orients: " << orients << ", sigma: " << sigma << ", divider: " << divider << ", weight: " << weight;

	if (weight > 0.0f) {
		CvImage res1, res2;
		image.copyTo(&res1);
		res1.applyGaborFilter(5, 50, 10, orients, divider, sigma);
		image.applyWeighted(&res1, (1.0f - weight), weight);

		// image.applyGuidedFilter();

		image.copyTo(&res2);
		res2.applyGaussianBlur(1.5);
		image.applyWeighted(&res2, 1.7, -0.7);
	}

	profile = image.copyDataInFloats();
	for_each(profile.begin(), profile.end(), [&](float& elem) { elem = max(elem, 0.0f); });
	return true;
}

bool OctAngio::AngioPost::performFilterProcessing(int width, int height, std::vector<float>& profile)
{
	if (profile.empty() || profile.size() != (width * height)) {
		return false;
	}
	if (all_of(profile.begin(), profile.end(), [](float i) { return i <= 0; })) {
		return true;
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);
	image.resize(width, height);

	// image.applyGuidedFilter();
	image.applyGaussianBlur(1.5f);

	profile = image.copyDataInFloats();
	for_each(profile.begin(), profile.end(), [&](float& elem) { elem = max(elem, 0.0f); });
	return true;
}


bool OctAngio::AngioPost::removeProjectionStripeNoise(const AngioLayout & layout, std::vector<float>& profile, bool vertical)
{
	int width = layout.getWidth();
	int height = layout.getHeight();

	if (profile.empty() || profile.size() != (width * height)) {
		return false;
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);

	int optimal_w = width; // cv::getOptimalDFTSize(width);
	int optimal_h = height; // cv::getOptimalDFTSize(height);
	cv::Mat padded;

	// Expand the image to optimal size for FFT
	// cv::copyMakeBorder(image.getCvMatConst(), padded, 0, optimal_h - height, 0, optimal_w - width, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	
	// FFTW setup
	int N = optimal_w * optimal_h;
	fftw_complex* input = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
	fftw_complex* output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

	// Fill the input with image data
	for (int i = 0; i < optimal_h; i++) {
		for (int j = 0; j < optimal_w; j++) {
			if (i < height && j < width) {
				// input[i * optimal_w + j][0] = padded.at<float>(i, j);  // Real part
				input[i * optimal_w + j][0] = profile[i*width + j];  // Real part
			}
			else {
				input[i * optimal_w + j][0] = 0.0f;  // Padding with 0
			}
			input[i * optimal_w + j][1] = 0.0f;  // Imaginary part
		}
	}

	// Apply FFT
	fftw_plan plan = fftw_plan_dft_2d(optimal_h, optimal_w, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(plan);

	int centerY = optimal_h / 2;
	int centerX = optimal_w / 2;
	int stripWidth = 0;  // Adjust this based on the noise frequency

	// Swap quadrants
	for (int i = 0; i < centerY; ++i) {
		for (int j = 0; j < centerX; ++j) {
			// Swap top-left with bottom-right
			std::swap(output[i*optimal_w + j][0], output[(i + centerY)*optimal_w + j + centerX][0]);
			std::swap(output[i*optimal_w + j][1], output[(i + centerY)*optimal_w + j + centerX][1]);

			// Swap top-right with bottom-left
			std::swap(output[i*optimal_w + j + centerY][0], output[(i + centerY)*optimal_w + j][0]);
			std::swap(output[i*optimal_w + j + centerY][1], output[(i + centerY)*optimal_w + j][1]);
		}
	}
	
	// Remove horizontal noise by zeroing out corresponding frequencies
	const float kern_size = (width <= 256 ? 16.0f : (width <= 384 ? 24.0f : 32.0f));

	if (vertical) {
		for (int y = centerY - stripWidth; y <= centerY + stripWidth; y++) {
			for (int i = 0; i < optimal_w; i++) {
				float d = i - centerX;
				d *= (d > 0 ? -1.0f : +1.0f);
				float filt = 1.0f / (1.0f + exp(-(d + kern_size) / 3.0f));

				auto real = output[y * optimal_w + i][0];
				auto imag = output[y * optimal_w + i][1];
				output[y * optimal_w + i][0] = real * filt;
				output[y * optimal_w + i][1] = imag * filt;
			}
		}
	}
	else {
		for (int x = centerX - stripWidth; x <= centerY + stripWidth; x++) {
			for (int i = 0; i < optimal_h; i++) {
				float d = i - centerY;
				d *= (d > 0 ? -1.0f : +1.0f);
				float filt = 1.0f / (1.0f + exp(-(d + kern_size) / 3.0f));

				if (filt < 0.5f) {
					d = filt;
				}

				auto real = output[i * optimal_w + x][0];
				auto imag = output[i * optimal_w + x][1];
				output[i * optimal_w + x][0] = real * filt;
				output[i * optimal_w + x][1] = imag * filt;
			}
		}
	}

	// Zero out area outside a circle. 
	auto radius = min(width, height)/2.0f;
	for (int y = 0; y < optimal_h; y++) {
		for (int x = 0; x < optimal_w; x++) {
			auto dx = x - centerX;
			auto dy = y - centerY;
			auto dist = sqrt(dx*dx + dy*dy);
			if (dist > radius) {
				output[y * optimal_w + x][0] = 0.0;
				output[y * optimal_w + x][1] = 0.0;
			}
		}
	}
	
	// Swap quadrants
	for (int i = 0; i < centerY; ++i) {
		for (int j = 0; j < centerX; ++j) {
			// Swap top-left with bottom-right
			std::swap(output[i*optimal_w + j][0], output[(i + centerY)*optimal_w + j + centerX][0]);
			std::swap(output[i*optimal_w + j][1], output[(i + centerY)*optimal_w + j + centerX][1]);

			// Swap top-right with bottom-left
			std::swap(output[i*optimal_w + j + centerY][0], output[(i + centerY)*optimal_w + j][0]);
			std::swap(output[i*optimal_w + j + centerY][1], output[(i + centerY)*optimal_w + j][1]);
		}
	}

	// Inverse FFT to reconstruct the image without noise
	fftw_plan inversePlan = fftw_plan_dft_2d(optimal_h, optimal_w, output, input, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(inversePlan);

	// Normalize the output
	cv::Mat outputImage(optimal_h, optimal_w, CV_32F);
	for (int i = 0; i < optimal_h; i++) {
		for (int j = 0; j < optimal_w; j++) {
			outputImage.at<float>(i, j) = (float)std::sqrt(input[i * optimal_w + j][0] * input[i * optimal_w + j][0] +
				input[i * optimal_w + j][1] * input[i * optimal_w + j][1]);
		}
	}

	// Crop the image to original size
	outputImage = outputImage(cv::Rect(0, 0, width, height));

	auto result = profile;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			result[i*width + j] = outputImage.at<float>(i, j);
		}
	}
	profile = result;

	// Free memory
	fftw_free(input);
	fftw_free(output);
	fftw_destroy_plan(plan);
	fftw_destroy_plan(inversePlan);
	return true;
}

bool OctAngio::AngioPost::removeProjectionStripeNoise2(const AngioLayout & layout, std::vector<float>& profile, bool vertical)
{
	int width = layout.getWidth();
	int height = layout.getHeight();

	if (profile.empty() || profile.size() != (width * height)) {
		return false;
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);

	// Shift zero frequency to center
	Mat comp;
	cv::dft(image.getCvMat(), comp, cv::DFT_COMPLEX_OUTPUT);

	// Shift zero frequency to center
	cv::Mat shifted;
	int cx = comp.cols / 2;
	int cy = comp.rows / 2;

	shifted = comp.clone();
	for (int i = 0; i < comp.rows; i++) {
		for (int j = 0; j < comp.cols; j++) {
			int ii = (i + cy) % comp.rows;
			int jj = (j + cx) % comp.cols;
			shifted.at<cv::Vec2f>(ii, jj) = comp.at<cv::Vec2f>(i, j);
		}
	}

	// Remove horizontal noise by zeroing out corresponding frequencies
	const float kern_size = (width <= 256 ? 16.0f : (width <= 384 ? 24.0f : 32.0f));
	int centerY = comp.rows / 2;
	int centerX = comp.cols / 2;
	int optimal_w = comp.cols;
	int optimal_h = comp.rows;
	int stripWidth = 0;  // Adjust this based on the noise frequency

	if (vertical) {
		for (int y = centerY - stripWidth; y <= centerY + stripWidth; y++) {
			for (int i = 0; i < optimal_w; i++) {
				float d = i - centerX;
				d *= (d > 0 ? -1.0f : +1.0f);
				float filt = 1.0f / (1.0f + exp(-(d + kern_size) / 3.0f));

				auto data = shifted.at<cv::Vec2f>(y, i);
				data[0] *= filt;
				data[1] *= filt;
				shifted.at<cv::Vec2f>(y, i) = data;
			}
		}
	}
	else {
		for (int x = centerX - stripWidth; x <= centerX + stripWidth; x++) {
			for (int i = 0; i < optimal_h; i++) {
				float d = i - centerY;
				d *= (d > 0 ? -1.0f : +1.0f);
				float filt = 1.0f / (1.0f + exp(-(d + kern_size) / 3.0f));

				auto data = shifted.at<cv::Vec2f>(i, x);
				data[0] *= filt;
				data[1] *= filt;
				shifted.at<cv::Vec2f>(i, x) = data;
			}
		}
	}

	// Zero out area outside a circle. 
	auto radius = (min(width, height) / 2.0f) * 0.85f;
	for (int y = 0; y < optimal_h; y++) {
		for (int x = 0; x < optimal_w; x++) {
			auto dx = x - centerX;
			auto dy = y - centerY;
			auto dist = sqrt(dx*dx + dy*dy);
			if (dist > radius) {
				shifted.at<cv::Vec2f>(y, x) = cv::Vec2f(0.0f, 0.0f);
			}
		}
	}

	for (int i = 0; i < shifted.rows; i++) {
		for (int j = 0; j < shifted.cols; j++) {
			int ii = (i + cy) % shifted.rows;
			int jj = (j + cx) % shifted.cols;
			comp.at<cv::Vec2f>(ii, jj) = shifted.at<cv::Vec2f>(i, j);
		}
	}

	Mat inverse;
	cv::dft(comp, inverse, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);

	Mat output;
	inverse.convertTo(output, image.getCvMat().type());

	auto result = profile;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			result[i*width + j] = output.at<float>(i, j);
		}
	}
	profile = result;
	return true;
}


void OctAngio::AngioPost::detectVessels(int width, int height, std::vector<float>& profile, float thresh1, float thresh2)
{
	if (profile.empty()) {
		return;
	}

	const float FLOW = 255.0f;
	const float BACK = 0.0f;
	int n_flows = 0;
	int n_iters = 0;

	auto data = profile;

	// Seeds for hysterisis thresholding. 
	transform(begin(data), end(data), begin(data), [&](float e) {
		return (e >= thresh1 ? FLOW : e);
	});

	float* p = &data[0];

	// Remoe the isolated points, works like median filter with kernel size. 
	const int FILT_SIZE = 5;
	const int half = FILT_SIZE / 2;
	int removed = 0;
	int remains = 0;

	for (int y = half; y < (height - half); y++) {
		for (int x = half; x < (width - half); x++) {
			int n = y * width + x;
			if (p[n] >= FLOW) {
				int size = 0;
				int q = n - (width * half);
				for (int m = 0; m < FILT_SIZE; m++) {
					for (int k = -half; k <= half; k++) {
						if (p[q+k] >= FLOW) {
							size++;
						}
					}
					q += width;
				}
				if (size <= half) {
					p[n] = BACK;
					removed++;
				}
				else {
					remains++;
				}
			}
		}
	}
	LogD() << "Angio vessels, points removed: " << removed << ", remains: " << remains;
	// return;

	// Double thresholding until flows size not further changed.
	while (remains > 0) {
		for (int y = 1; y < (height - 1); y++) {
			for (int x = 1; x < (width - 1); x++) {
				int n = y * width + x;
				if (p[n] >= FLOW) {
					if (p[n - 1] >= thresh2) {
						p[n - 1] = FLOW;
					}
					if (p[n + 1] >= thresh2) {
						p[n + 1] = FLOW;
					}
					n -= width;
					if (p[n - 1] >= thresh2) {
						p[n - 1] = FLOW;
					}
					if (p[n] >= thresh2) {
						p[n] = FLOW;
					}
					if (p[n + 1] >= thresh2) {
						p[n + 1] = FLOW;
					}
					n += (width * 2);
					if (p[n - 1] >= thresh2) {
						p[n - 1] = FLOW;
					}
					if (p[n] >= thresh2) {
						p[n] = FLOW;
					}
					if (p[n + 1] >= thresh2) {
						p[n + 1] = FLOW;
					}
				}
			}
		}

		int size = 0;
		for_each(begin(data), end(data), [&](float& e) {
			size += (e == FLOW ? 1 : 0);
		});
		if (n_flows != 0 && n_flows == size) {
			break;
		}
		n_flows = size;
		n_iters += 1;
	}

	transform(begin(profile), end(profile), begin(data), begin(profile), [](float e1, float e2) {
		return e1 * e2;
	});

	// profile = data;

	LogD() << "Angio vessels, iterations: " << n_iters;
	return;
}


bool OctAngio::AngioPost::applyNoiseReduction(const AngioLayout & layout, std::vector<float>& profile, float rate)
{
	if (profile.empty()) {
		return false;
	}
	if (rate <= 0.0f) {
		return false;
	}

	auto height = layout.getHeight();
	auto width = layout.getWidth();
	
	const int RESIZE = 4;
	const int PATCH_W = width / RESIZE; // 64;
	const int PATCH_H = height / RESIZE;
	const int FILTER_W = (PATCH_W / 4 + 1);
	const int FILTER_H = (PATCH_H / 4 + 1);
	const float BACKGROUND = 0.25f; // 0.25f;

	CvImage image;
	image.fromFloat32((const unsigned char*)&profile[0], width, height);
	image.resize(PATCH_W, PATCH_H);

	auto img_w = image.getWidth();
	auto img_h = image.getHeight();
	auto data = image.copyDataInFloats();
	auto dout = vector<float>(img_w * img_h);

	if (layout.isVerticalScan()) {
		auto filt_size = FILTER_H;
		auto filt_half = FILTER_H / 2;

		for (int x = 0; x < img_w; x++) {
			for (int y = 0; y < img_h; y++) {
				int idx = y * img_w + x;
				int r1 = max(y - filt_half, 0);
				int r2 = min(y + filt_half, img_h - 1);
				int dcnt = 0;

				auto vect = vector<float>();

				for (int k = r1; k <= r2; k++) {
					// dsum += data[k*img_w + x];
					vect.push_back(data[k*img_w + x]);
					dcnt += 1;
				}

				sort(vect.begin(), vect.end());
				auto bidx = (int)(dcnt *  BACKGROUND);
				dout[idx] = vect[bidx];
				// dout[idx] = dsum / dcnt;
			}
		}
	}
	else {
		auto filt_size = FILTER_W;
		auto filt_half = FILTER_W / 2;

		for (int y = 0; y < img_h; y++) {
			for (int x = 0; x < img_w; x++) {
				int idx = y * img_w + x;
				int c1 = max(x - filt_half, 0);
				int c2 = min(x + filt_half, img_w - 1);
				int dcnt = 0;

				auto vect = vector<float>();

				for (int k = c1; k <= c2; k++) {
					// dsum += data[k*img_w + x];
					vect.push_back(data[y*img_w + k]);
					dcnt += 1;
				}

				sort(vect.begin(), vect.end());
				auto bidx = (int)(dcnt *  BACKGROUND);
				dout[idx] = vect[bidx];
				// dout[idx] = dsum / dcnt;
			}
		}
	}

	image.fromFloat32((const unsigned char*)&dout[0], img_w, img_h);
	image.resize(width, height);
	auto backs = image.copyDataInFloats();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			auto idx = y * width + x;
			auto d_val = profile[idx];
			auto b_val = backs[idx];
			auto thresh = b_val * rate;

			// LogD() << d_val << ", " << b_val << ", " << rate << ", " << thresh;
			profile[idx] = max(d_val - thresh, 0.0f);
		}
	}
	return true;
}


bool OctAngio::AngioPost::removeFoveaRegionNoise(const AngioLayout & layout, const AngioLayers & layers, std::vector<float>& profile)
{
	bool isFovea = layout.isMacularScan();
	if (!isFovea) {
		return false;
	}

	int cx, cy, size1, size2;
	layout.getFoveaCenterInPixel(cx, cy);
	layout.getFoveaRadiusInPixel(size1, size2);

	auto uppers = layers.getUpperLayers(OcularLayerType::ILM);
	auto lowers = layers.getLowerLayers(OcularLayerType::IPL);
	auto outers = layers.getLowerLayers(OcularLayerType::OPL);

	auto vertical = layout.isVerticalScan();
	auto lines = layout.numberOfLines();
	auto points = layout.numberOfPoints();

	auto width = layout.getWidth();
	auto height = layout.getHeight();

	auto fsx = cx - size1 / 2;
	auto fex = fsx + size1;
	auto fsy = cy - size2 / 2;
	auto fey = fsy + size2;

	for (auto r = 0; r < lines; r++) {
		for (auto c = 0; c < points; c++) {
			auto index = (vertical ? (c * lines + r) : (r * points + c));
		}
	}

	return false;
}

bool OctAngio::AngioPost::applyBiasFieldCorrection(const AngioLayout& layout, std::vector<float>& profile, bool isFovea)
{
	if (profile.empty() || profile.size() != layout.getSize()) {
		return false;
	}

	int w = layout.getWidth();
	int h = layout.getHeight();
	bool vertical = layout.isVerticalScan();

	std::vector<float> img_data = profile;
	/*
	{
		auto minmax = std::minmax_element(img_data.begin(), img_data.end());
		float img_min = (minmax.first != img_data.end()) ? *minmax.first : 0.0f;
		float img_max = (minmax.second != img_data.end()) ? *minmax.second : 0.0f;
		transform(begin(img_data), end(img_data), begin(img_data), [&](float e) {
			return (e - img_min) / (img_max - img_min + 1e-6f);
			});
	}
	*/

	CvImage img_blur;
	img_blur.fromFloat32((const unsigned char*)&img_data[0], w, h);
    img_blur.applyGaussianBlur(3.0);

	auto minmax = std::minmax_element(img_data.begin(), img_data.end());
	float img_min = (minmax.first != img_data.end()) ? *minmax.first : 0.0f;
	float img_max = (minmax.second != img_data.end()) ? *minmax.second : 0.0f;

	vector<float> img_norm = vector<float>(w * h);
	transform(begin(img_data), end(img_data), begin(img_norm), [&](float e) {
		return 1.0f - (e - img_min) / (img_max - img_min + 1e-6f);
	});
	vector<float> pix_gain = vector<float>(w * h);

	if (vertical) {
		vector<float> cols_mean;
		vector<float> cols_stde;
		img_blur.reduceMeanStddev(1, cols_mean, cols_stde);

		std::vector<float> sorted = cols_mean;
		std::sort(sorted.begin(), sorted.end());
		auto quater = (int)(sorted.size() * 0.95f);
		auto col_target = sorted[quater];

		vector<float> cols_gain = vector<float>(cols_mean.size());
		for (size_t i = 0; i < cols_mean.size(); ++i) {
			cols_gain[i] = min(max(col_target / (cols_mean[i] + 1e-6f), 1.0f), 2.5f);
		}
		
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				size_t idx = y * w + x;
				float gain = (cols_gain[x] - 1.0f) * img_norm[idx] + 1.0f;
				pix_gain[idx] = gain;
			}
		}
	}
	else {
		vector<float> rows_mean;
		vector<float> rows_stde;
		img_blur.reduceMeanStddev(1, rows_mean, rows_stde);

		std::vector<float> sorted = rows_mean;
		std::sort(sorted.begin(), sorted.end());
		auto quater = (int)(sorted.size() * 0.95f);
		auto row_target = sorted[quater];

		vector<float> rows_gain = vector<float>(rows_mean.size());
		for (size_t i = 0; i < rows_mean.size(); ++i) {
			rows_gain[i] = min(max(row_target / (rows_mean[i] + 1e-6f), 1.0f), 2.5f);
		}

		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				size_t idx = y * w + x;
				float gain = (rows_gain[y] - 1.0f) * img_norm[idx] + 1.0f;
				pix_gain[idx] = gain;
			}
		}
	}

	/*
	vector<float> sorted = img_data;
	sort(sorted.begin(), sorted.end());
	auto quater = (int)(sorted.size() * 0.15f);
	*/
	auto value = 0.0f;// sorted[quater];
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			size_t idx = y * w + x;
			if (img_data[idx] >= value) {
				img_data[idx] = min(pix_gain[idx] * img_data[idx], img_max);
			}
		}
	}

	CvImage img_L1, img_L2;
	img_L1.fromFloat32((const unsigned char*)&img_data[0], w, h);
	img_L2.fromFloat32((const unsigned char*)&img_data[0], w, h);
	img_L1.applyGaussianBlur(45.0f);
	img_L2.applyGaussianBlur(3.0f);

	double l2_min, l2_max;
	img_L2.getMinMaxValue(&l2_min, &l2_max);
	
	vector<float> l2_data = img_L2.copyDataInFloats();
	vector<float> bias_mod = vector<float>(w * h);
	transform(begin(l2_data), end(l2_data), begin(bias_mod), [&](float e) {
		auto norm = 1.0f - (e - l2_min) / (l2_max - l2_min + 1e-6f);
		return pow(norm, 1.0f / 2.2f);
	});

	auto l1_data = img_L1.copyDataInFloats();
	auto sum = accumulate(l1_data.begin(), l1_data.end(), 0.0f);
	auto l1_mean = sum / (l1_data.size() > 0 ? l1_data.size() : 1);

	minmax = std::minmax_element(img_data.begin(), img_data.end());
	img_max = (minmax.second != img_data.end()) ? *minmax.second : 0.0f;

	/*
	sorted = img_data;
	sort(sorted.begin(), sorted.end());
	quater = (int)(sorted.size() * 0.15f);
	value = 0.0f; // sorted[quater];
	*/

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			size_t idx = y * w + x;
			auto gain = max(l1_mean / (l1_data[idx] + 1e-6f), 1.0f);
			gain = (gain - 1.0f) * bias_mod[idx] + 1.0f;
			img_data[idx] = min(img_data[idx] * gain, img_max);
		}
	}

	profile = img_data;
	return true;
}


int & OctAngio::AngioPost::garborFilerOrients(void)
{
	return getImpl().filterOrients;
}

float & OctAngio::AngioPost::garborFilterSigma(void)
{
	return getImpl().filterSigma;
}

float & OctAngio::AngioPost::garborFilterDivider(void)
{
	return getImpl().filterDivider;
}

float & OctAngio::AngioPost::garborFilterWeight(void)
{
	return getImpl().filterWeight;
}



AngioPost::AngioPostImpl & OctAngio::AngioPost::getImpl(void) const
{
	return *d_ptr;
}

