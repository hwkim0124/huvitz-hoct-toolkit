#include "stdafx.h"
#include "AnteriorLens.h"
#include "CppUtil2.h"
#include "RetSegm2.h"
#include "SegmScan2.h"
#include "SegmScan2.h"

using namespace CorTopo;
using namespace CppUtil;
using namespace RetSegm;
using namespace SegmScan;
using namespace cv;
using namespace std;


struct AnteriorLens::AnteriorLensImpl
{
	SegmImage imgCornea;
	SegmImage imgCornea2;
	SegmImage imgCorneaAsc;
	SegmImage imgCorneaDes;

	std::vector<int> corneaAntes;
	std::vector<int> lensAntes;
	std::vector<int> lensPosts;

	std::wstring corneaName;
	bool isResult = false;

	int topCorneaCenterX = -1;
	int topCorneaCenterY = -1;
	int lensFrontCenterX = -1;
	int lensFrontCenterY = -1;
	int lensBackCenterX = -1;
	int lensBackCenterY = -1;

	AnteriorLensImpl() {

	}
};


AnteriorLens::AnteriorLens() :
	d_ptr(make_unique<AnteriorLensImpl>())
{
}


CorTopo::AnteriorLens::~AnteriorLens() = default;
CorTopo::AnteriorLens::AnteriorLens(AnteriorLens && rhs) = default;
AnteriorLens & CorTopo::AnteriorLens::operator=(AnteriorLens && rhs) = default;


AnteriorLens::AnteriorLensImpl & CorTopo::AnteriorLens::getImpl(void) const
{
	return *d_ptr;
}


CorTopo::AnteriorLens::AnteriorLens(const AnteriorLens & rhs)
	: d_ptr(make_unique<AnteriorLensImpl>(*rhs.d_ptr))
{
}


AnteriorLens & CorTopo::AnteriorLens::operator=(const AnteriorLens & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool CorTopo::AnteriorLens::loadCorneaImage(const std::wstring & path, bool vflip)
{
	try {
		if (!path.empty() && !getImpl().imgCornea.fromFile(path)) {
			LogD() << "Failed to load cornea image, path: " + wtoa(path);
			return false;
		}

		getImpl().corneaName = path;

		prepareSamples(vflip);
		return true;
	}
	catch (...) {
		LogD() << "Failed to load cornea image";
	}
	return false;
}


bool CorTopo::AnteriorLens::makeupAnteriorCorneaBorder(void)
{
	vector<int> antes;
	vector<int> posts;

	int centerX, centerY;
	float rangeX = 4.5f;

	if (!isCorneaImageLoaded()) {
		return false;
	}

	if (!segmentCorneaBorderLines(rangeX, centerX, centerY, antes, posts)) {
		centerX = -1;
		centerY = -1;
		antes.clear();
		posts.clear();
	}

	getImpl().topCorneaCenterX = centerX;
	getImpl().topCorneaCenterY = centerY;
	getImpl().corneaAntes = antes;

	upscaleCorneaBorderLines();
	return true;
}


bool CorTopo::AnteriorLens::makeupAnteriorLensBorder(void)
{
	if (!isCorneaImageLoaded()) {
		return false;
	}

	vector<int> antes;

	int centerX, centerY;
	float rangeX = 4.5f;

	if (!segmentLensAnteriorLine(rangeX, centerX, centerY, antes)) {
		centerX = -1;
		centerY = -1;
		antes.clear();
	}

	getImpl().lensFrontCenterX = centerX;
	getImpl().lensFrontCenterY = centerY;
	getImpl().lensAntes = antes;

	upscaleLensAnteriorLine();
	return true;
}


bool CorTopo::AnteriorLens::makeupPosteriorLensBorder(void)
{
	if (!isCorneaImageLoaded()) {
		return false;
	}

	vector<int> posts;

	int centerX, centerY;
	float rangeX = 4.5f;

	if (!segmentLensPosteriorLine(rangeX, centerX, centerY, posts)) {
		centerX = -1;
		centerY = -1;
		posts.clear();
	}

	getImpl().lensBackCenterX = centerX;
	getImpl().lensBackCenterY = centerY;
	getImpl().lensPosts = posts;

	upscaleLensPosteriorLine();
	return true;
}


bool CorTopo::AnteriorLens::isCorneaImageLoaded(void) const
{
	if (imageCornea()->isEmpty()) {
		return false;
	}
	return true;
}


int CorTopo::AnteriorLens::getAnteriorCorneaPosition(void) const
{
	auto& corAntes = corneaAnteriorLine();
	if (corAntes.empty() || getImpl().topCorneaCenterY < 0) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().topCorneaCenterY;
}


int CorTopo::AnteriorLens::getAnteriorLensPosition(void) const
{
	auto& corAntes = lensAnteriorLine();
	if (corAntes.empty() || getImpl().lensFrontCenterY < 0) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().lensFrontCenterY;
}


int CorTopo::AnteriorLens::getPosteriorLensPosition(void) const
{
	auto& corAntes = lensPosteriorLine();
	if (corAntes.empty() || getImpl().lensBackCenterY < 0) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().lensBackCenterY;
}


std::wstring CorTopo::AnteriorLens::corneaImageName(void) const
{
	return getImpl().corneaName;
}


bool CorTopo::AnteriorLens::upscaleCorneaBorderLines(void)
{
	auto& corAntes = corneaAnteriorLine();
	if (corAntes.empty()) {
		return false; 
	}

	int img_w = imageCornea()->getWidth();
	int img_h = imageCornea()->getHeight();
	int size = (int)corAntes.size();

	float ratioH = (float)img_w / SAMPLE_IMAGE_WIDTH;
	float ratioV = (float)img_h / SAMPLE_IMAGE_HEIGHT;

	auto line1 = std::vector<int>(img_w, -1);

	if (!corAntes.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line1[i] = (int)(corAntes[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			if (!corAntes.empty()) {
				for (int c = 0; c < size; c++) {
					dataX[c] = c;
					dataY[c] = (line1[c] < 0 ? 0 : line1[c]);
				}

				vector<double> coeffs;
				if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
					corAntes = fittY;
				}
			}
			else {
				corAntes.clear();
			}
		}
		
		// corAntes = line1;
	}

	if (corAntes.size() > 0) {
		int size = (int)corAntes.size();
		int index = size / 2;
		int centerX = index;
		int centerY = corAntes[index];
		getImpl().topCorneaCenterX = centerX;
		getImpl().topCorneaCenterY = centerY;
	}
	return true;
}


bool CorTopo::AnteriorLens::upscaleLensAnteriorLine(void)
{
	auto& corAntes = lensAnteriorLine();
	if (corAntes.empty()) {
		return false;
	}

	int img_w = imageCornea()->getWidth();
	int img_h = imageCornea()->getHeight();
	int size = (int)corAntes.size();

	float ratioH = (float)img_w / SAMPLE_IMAGE_WIDTH;
	float ratioV = (float)img_h / SAMPLE_IMAGE_HEIGHT;

	auto line1 = std::vector<int>(img_w, -1);

	if (!corAntes.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line1[i] = (int)(corAntes[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			if (!corAntes.empty()) {
				for (int c = 0; c < size; c++) {
					dataX[c] = c;
					dataY[c] = (line1[c] < 0 ? 0 : line1[c]);
				}

				vector<double> coeffs;
				if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
					corAntes = fittY;
				}
			}
			else {
				corAntes.clear();
			}
		}

		// corAntes = line1;
	}

	if (corAntes.size() > 0) {
		int size = (int)corAntes.size();
		int index = size / 2;
		int centerX = index;
		int centerY = corAntes[index];
		getImpl().lensFrontCenterX = centerX;
		getImpl().lensFrontCenterY = centerY;
	}
	return true;
}


bool CorTopo::AnteriorLens::upscaleLensPosteriorLine(void)
{
	auto& corAntes = lensPosteriorLine();
	if (corAntes.empty()) {
		return false;
	}

	int img_w = imageCornea()->getWidth();
	int img_h = imageCornea()->getHeight();
	int size = (int)corAntes.size();

	float ratioH = (float)img_w / SAMPLE_IMAGE_WIDTH;
	float ratioV = (float)img_h / SAMPLE_IMAGE_HEIGHT;

	auto line1 = std::vector<int>(img_w, -1);

	if (!corAntes.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line1[i] = (int)(corAntes[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			if (!corAntes.empty()) {
				for (int c = 0; c < size; c++) {
					dataX[c] = c;
					dataY[c] = (line1[c] < 0 ? 0 : line1[c]);
				}

				vector<double> coeffs;
				if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
					corAntes = fittY;
				}
			}
			else {
				corAntes.clear();
			}
		}

		// corAntes = line1;
	}

	if (corAntes.size() > 0) {
		int size = (int)corAntes.size();
		int index = size / 2;
		int centerX = index;
		int centerY = corAntes[index];
		getImpl().lensBackCenterX = centerX;
		getImpl().lensBackCenterY = (img_h - centerY);

		for (int i = 0; i < img_w; i++) {
			corAntes[i] = img_h - corAntes[i];
		}
	}
	return true;
}


RetSegm::SegmImage * CorTopo::AnteriorLens::imageCornea(void) const
{
	return &(getImpl().imgCornea);
}


RetSegm::SegmImage * CorTopo::AnteriorLens::sampleCornea(void) const
{
	return &(getImpl().imgCornea2);
}

RetSegm::SegmImage * CorTopo::AnteriorLens::ascentCornea(void) const
{
	return &(getImpl().imgCorneaAsc);
}

RetSegm::SegmImage * CorTopo::AnteriorLens::descentCornea(void) const
{
	return &(getImpl().imgCorneaDes);
}

std::vector<int>& CorTopo::AnteriorLens::corneaAnteriorLine(void) const
{
	return getImpl().corneaAntes;
}


std::vector<int>& CorTopo::AnteriorLens::lensAnteriorLine(void) const
{
	return getImpl().lensAntes;
}


std::vector<int>& CorTopo::AnteriorLens::lensPosteriorLine(void) const
{
	return getImpl().lensPosts;
}


bool CorTopo::AnteriorLens::prepareSamples(bool vflip)
{
	if (!imageCornea()->isEmpty()) {
		Size size(SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT);
		resize(imageCornea()->getCvMatConst(), sampleCornea()->getCvMat(), size);

		if (vflip) {
			sampleCornea()->flipVertical();
		}
		sampleCornea()->applyGuidedFilter();

		PrepCornea::collectImageStats(sampleCornea());
		PrepCornea::collectColumnStats(sampleCornea());

		CoarseCornea::createGradientMapOfCornea(sampleCornea(), ascentCornea(), descentCornea());
		PrepCornea::collectImageStats(ascentCornea());
		PrepCornea::collectColumnStats(ascentCornea());
		PrepCornea::collectImageStats(descentCornea());
		PrepCornea::collectColumnStats(descentCornea());
	}

	return true;
}


bool CorTopo::AnteriorLens::segmentCorneaBorderLines(float rangeX, int & centerX, int & centerY, std::vector<int>& antes, std::vector<int>& posts)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(sampleCornea()->getWidth(), -1);
	outer.resize(sampleCornea()->getWidth(), -1);
	fitts.resize(sampleCornea()->getWidth(), -1);

	int hingeY = -1;
	int reflectX1 = -1;
	int reflectX2 = -1;

	if (!CoarseCornea::locateCenterReflection(sampleCornea(), rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::locateCenterEdgesOfCornea(sampleCornea(), ascentCornea(), inner, outer,
		rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInCenterEdges(ascentCornea(), inner, outer, rangeX)) {
		return false;
	}

	if (!CoarseCornea::composeInnerCurveByCenterEdges(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::locateInnerBorderLine(sampleCornea(), inner, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY, true)) {
		return false;
	}

	if (!CoarseCornea::removeFractionsInInnerBorderLine(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInInnerBorderLine(inner, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::interpolateInnerBorderLine(inner, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	antes = inner;
	return true;
}


bool CorTopo::AnteriorLens::segmentLensAnteriorLine(float rangeX, int & centerX, int & centerY, std::vector<int>& line)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(sampleCornea()->getWidth(), -1);
	outer.resize(sampleCornea()->getWidth(), -1);
	fitts.resize(sampleCornea()->getWidth(), -1);

	int hingeY = -1;
	int reflectX1 = -1;
	int reflectX2 = -1;

	if (!CoarseCornea::locateCenterReflection(sampleCornea(), rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::locateCenterEdgesOfCornea(sampleCornea(), ascentCornea(), inner, outer,
		rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInCenterEdges(ascentCornea(), inner, outer, rangeX)) {
		return false;
	}

	if (!CoarseCornea::composeInnerCurveByCenterEdges(inner, rangeX, centerX, centerY)) {
		return false;
	}

	line = inner;
	return true;
}


bool CorTopo::AnteriorLens::segmentLensPosteriorLine(float rangeX, int & centerX, int & centerY, std::vector<int>& line)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(sampleCornea()->getWidth(), -1);
	outer.resize(sampleCornea()->getWidth(), -1);
	fitts.resize(sampleCornea()->getWidth(), -1);

	int hingeY = -1;
	int reflectX1 = -1;
	int reflectX2 = -1;

	if (!CoarseCornea::locateCenterReflection(sampleCornea(), rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::locateCenterEdgesOfCornea(sampleCornea(), ascentCornea(), inner, outer,
		rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInCenterEdges(ascentCornea(), inner, outer, rangeX)) {
		return false;
	}

	if (!CoarseCornea::composeInnerCurveByCenterEdges(inner, rangeX, centerX, centerY)) {
		return false;
	}

	line = inner;
	return true;
}
