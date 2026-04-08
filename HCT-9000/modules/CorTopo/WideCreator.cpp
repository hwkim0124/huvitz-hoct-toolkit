#include "stdafx.h"
#include "WideCreator.h"
#include "Composite.h"
#include "CppUtil2.h"
#include "RetSegm2.h"
#include "SegmScan2.h"

#include <boost/format.hpp>

using namespace CorTopo;
using namespace CppUtil;
using namespace RetSegm;
using namespace SegmScan;
using namespace cv;
using namespace std;


struct WideCreator::WideCreatorImpl
{
	SegmImage imgCornea;
	SegmImage imgChamber;
	SegmImage imgCornea2;
	SegmImage imgChamber2;
	SegmImage imgCorneaAsc;
	SegmImage imgChamberAsc;
	SegmImage imgCorneaDes;
	SegmImage imgChamberDes;
	SegmImage imgAnterior;

	std::vector<int> corneaAntes;
	std::vector<int> corneaPosts;
	std::vector<int> corneaSides;
	std::vector<int> corneaHinge1;
	std::vector<int> corneaHinge2;

	std::vector<int> chamberAntes;
	std::vector<int> chamberPosts;
	std::vector<int> chamberHinge1;
	std::vector<int> chamberHinge2;

	std::wstring corneaName;
	std::wstring chamberName;
	bool isResult = false;

	int topCorneaCenterX = -1;
	int topCorneaCenterY = -1;
	int revCorneaCenterX = -1;
	int revCorneaCenterY = -1;
	int reversedOffsetX = -1;
	int reversedOffsetY = -1;
	int stitchingOffsetX = 0;
	int stitchingOffsetY = 0;
};


WideCreator::WideCreator() :
	d_ptr(make_unique<WideCreatorImpl>())
{
}


CorTopo::WideCreator::~WideCreator() = default;
CorTopo::WideCreator::WideCreator(WideCreator && rhs) = default;
WideCreator & CorTopo::WideCreator::operator=(WideCreator && rhs) = default;


WideCreator::WideCreatorImpl & CorTopo::WideCreator::getImpl(void) const
{
	return *d_ptr;
}


CorTopo::WideCreator::WideCreator(const WideCreator & rhs)
	: d_ptr(make_unique<WideCreatorImpl>(*rhs.d_ptr))
{
}


WideCreator & CorTopo::WideCreator::operator=(const WideCreator & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int CorTopo::WideCreator::getStitchingOffsetX() const {
	return getImpl().stitchingOffsetX;
}

int CorTopo::WideCreator::getStitchingOffsetY() const {
	return getImpl().stitchingOffsetY;
}


bool CorTopo::WideCreator::loadImages(const std::wstring & corneaPath, const std::wstring & chamberPath)
{
	try {
		if (!corneaPath.empty() && !getImpl().imgCornea.fromFile(corneaPath)) {
			LogD() << "Failed to load cornea image, path: " + wtoa(corneaPath);
			return false;
		}
		if (!chamberPath.empty() && !getImpl().imgChamber.fromFile(chamberPath)) {
			LogD() << "Failed to load chamber image, path: " + wtoa(chamberPath);
			return false;
		}

		getImpl().corneaName = corneaPath;
		getImpl().chamberName = chamberPath;

		if (!prepareSamples()) {
			return false;
		}
		return true;
	}
	catch (...) {
		LogD() << "Failed to load wide anterior images";
	}
	return false;
}


bool CorTopo::WideCreator::loadImages(const unsigned char * corneaBits, const unsigned char * chamberBits, int width, int height)
{
	try {
		if (!corneaBits && !imageCornea()->fromBitsData(corneaBits, width, height)) {
			LogD() << "Failed to load cornea bitmap";
			return false;
		}
		if (!chamberBits && !imageChamber()->fromBitsData(chamberBits, width, height)) {
			LogD() << "Failed to load chamber bitmap";
			return false;
		}

		if (!prepareSamples()) {
			return false;
		}
		return true;
	}
	catch (...) {
	}
	return false;
}


bool CorTopo::WideCreator::saveWideAnteriorImage(const std::wstring & dirPath, const std::wstring & fileName)
{
	if (dirPath.empty() || fileName.empty()) {
		return false;
	}

	if (!getImpl().isResult) {
		return false;
	}

	wstring path;
	path = (boost::wformat(L"%s//%s") % dirPath % fileName).str();

	bool result = imageAnterior()->saveFile(wtoa(path));
	return result;
}

bool CorTopo::WideCreator::isResult(void) const
{
	return getImpl().isResult;
}


bool CorTopo::WideCreator::isImagesLoaded(void) const
{
	if (imageCornea()->isEmpty()) {
		return false;
	}
	if (imageChamber()->isEmpty()) {
		return false;
	}
	return true;
}

std::wstring CorTopo::WideCreator::corneaImageName(void) const
{
	return getImpl().corneaName;
}

std::wstring CorTopo::WideCreator::chamberImageName(void) const
{
	return getImpl().chamberName;
}


bool CorTopo::WideCreator::process(void)
{
	if (!makeupAnteriorCorneaBorder()) {
		return false;
	}
	if (!makeupAnteriorChamberBorder()) {
		return false;
	}
	if (!composeWideAnteriorImage()) {
		return false;
	}

	getImpl().isResult = true;
	return true;
}


bool CorTopo::WideCreator::makeupAnteriorCorneaBorder(void)
{
	vector<int> antes;
	vector<int> posts;
	vector<int> sides;

	int centerX, centerY;
	float rangeX = 9.0f;

	if (!segmentCorneaBorderLines(rangeX, centerX, centerY, antes, posts, sides)) {
		centerX = -1;
		centerY = -1;
		antes.clear();
		posts.clear();
		sides.clear();
	}

	vector<int> line1;
	vector<int> line2;
	if (!Composite::detectTrunkLinesOnCornea(sampleCornea(), antes, posts, line1, line2)) {
		line1.clear();
		line2.clear();
	}

	getImpl().corneaHinge1 = line1;
	getImpl().corneaHinge2 = line2;
	getImpl().topCorneaCenterX = centerX;
	getImpl().topCorneaCenterY = centerY;
	getImpl().corneaAntes = antes;
	getImpl().corneaPosts = posts;
	getImpl().corneaSides = sides;
	return true;
}


bool CorTopo::WideCreator::makeupAnteriorChamberBorder(void)
{
	vector<int> line1;
	vector<int> line2;

	if (!Composite::detectTrunkLinesOnChamber(sampleChamber(), line1, line2)) {
		line1.clear();
		line2.clear();
	}

	getImpl().chamberHinge1 = line1;
	getImpl().chamberHinge2 = line2;

	vector<int> antes;
	vector<int> posts;

	int centerX, centerY;
	float rangeX = 16.0f;

	if (!segmentChamberBorderLines(rangeX, centerX, centerY, antes, posts)) {
		centerX = -1;
		centerY = -1;
		antes.clear();
		posts.clear();
	}
	getImpl().revCorneaCenterX = centerX;
	getImpl().revCorneaCenterY = centerY;
	getImpl().chamberAntes = antes;
	getImpl().chamberPosts = posts;

	int offsetX, offsetY;
	if (centerX < 0 || centerY < 0 || getImpl().topCorneaCenterX < 0 || getImpl().topCorneaCenterY < 0) {
		offsetX = 0;
		offsetY = 0;
	}
	else {
		int img_h = sampleChamber()->getHeight();
		offsetX = -(centerX - getImpl().topCorneaCenterX);
		offsetY = (img_h - 1) - centerY - getImpl().topCorneaCenterY;
	}
	getImpl().reversedOffsetX = offsetX;
	getImpl().reversedOffsetY = offsetY;
	return true;
}



bool CorTopo::WideCreator::composeWideAnteriorImage(void)
{
	int offsetX, offsetY;
	int centerX, centerY;
	int corneaX, corneaY;

	upscaleBorderLines();

	corneaX = getImpl().topCorneaCenterX;
	corneaY = getImpl().topCorneaCenterY;
	centerX = getImpl().revCorneaCenterX;
	centerY = getImpl().revCorneaCenterY;
	offsetX = getImpl().reversedOffsetX;
	offsetY = getImpl().reversedOffsetY;

	if (!Composite::removeReversedCorneaOnChamber2(imageCornea(), imageChamber(),
		getImpl().corneaAntes, getImpl().corneaPosts, getImpl().corneaSides, 
		getImpl().chamberHinge1, getImpl().chamberHinge2, centerX, centerY, offsetX, offsetY)) {
		// return false;
	}

	if (!Composite::removeBackgroundOnCornea(imageCornea(), corneaX, corneaY, corneaAnteriorLine(), corneaPosteriorLine())) {
		// return false;
	}

	if (!Composite::updateCorneaChamberOffsets2(imageCornea(), getImpl().corneaSides,
		getImpl().chamberHinge1, getImpl().chamberHinge2, offsetX, offsetY)) {
	}

	if (!Composite::stitchCorneaChamberImages(imageCornea(), imageChamber(), offsetX, offsetY, imageAnterior())) {
		// return false;
	}

	if (!Composite::removeBackgroundOnAnterior(imageAnterior())) {
		// return false;
	}

	if (!Composite::applyDewarpingOnAnterior(imageAnterior())) {
		// return false;
	}

	getImpl().stitchingOffsetX = offsetX;
	getImpl().stitchingOffsetY = offsetY;

	imageAnterior()->applyMedianBlur(3);
	imageAnterior()->equalizeHistogram(0.5);
	return true;
}


bool CorTopo::WideCreator::segmentCorneaBorderLines(float rangeX, int & centerX, int & centerY, std::vector<int>& antes, std::vector<int>& posts, std::vector<int>& sides)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	vector<int> inner2;
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

	inner2 = inner;

	if (!CoarseCornea::locateInnerBorderLine(sampleCornea(), inner, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY)) {
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

	if (!CoarseCornea::locateInnerBorderLineBySides(sampleCornea(), inner2, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY)) {
		return false;
	}

	int centerX2, centerY2;
	centerX2 = centerX;
	centerY2 = centerY;
	if (!CoarseCornea::interpolateInnerBorderLine(inner2, rangeX, hingeY, centerX2, centerY2)) {
		return false;
	}


	if (!CoarseCornea::estimateOuterCurveByInnerCurve(descentCornea(), inner, outer, rangeX, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::locateOuterBorderLineForWide(descentCornea(), inner, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY)) {
		return false;
	}


	if (!CoarseCornea::removeFractionsInInnerBorderLine(outer, rangeX, centerX, centerY)) {
		return false;
	}

	/*
	if (!CoarseCornea::removeOutliersInInnerBorderLine(outer, rangeX, hingeY, centerX, centerY)) {
		return false;
	}
	*/

	if (!CoarseCornea::interpolateOuterBorderLine(inner, outer, rangeX, hingeY, centerX, centerY, 2.0f)) {
		return false;
	}

	antes = inner;
	posts = outer;
	sides = inner2;
	return true;
}



bool CorTopo::WideCreator::segmentChamberBorderLines(float rangeX, int & centerX, int & centerY, std::vector<int>& antes, std::vector<int>& posts)
{
	vector<int> inner;
	vector<int> outer;
	inner.resize(sampleCornea()->getWidth(), -1);
	outer.resize(sampleCornea()->getWidth(), -1);

	if (!CoarseCornea::locateCenterEdgesOfCorneaReversed(descentChamber(), inner, outer, rangeX, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInCenterEdgesReversed(descentChamber(), inner, outer, rangeX)) {
		return false;
	}

	if (!CoarseCornea::composeInnerCurveByCenterEdgesReversed(inner, rangeX, centerX, centerY)) {
		return false;
	}

	int img_h = sampleCornea()->getHeight();
	for (int i = 0; i < inner.size(); i++) {
		inner[i] = img_h - inner[i] - 1;
		outer[i] = img_h - outer[i] - 1;
	}
	centerY = img_h - centerY - 1;

	antes = inner;
	posts = inner;
	return true;
}


bool CorTopo::WideCreator::prepareSamples(void)
{
	if (!isImagesLoaded()) {
		return false;
	}

	Size size(SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT);
	resize(imageCornea()->getCvMatConst(), sampleCornea()->getCvMat(), size);
	resize(imageChamber()->getCvMatConst(), sampleChamber()->getCvMat(), size);

	sampleCornea()->applyGuidedFilter();
	sampleChamber()->applyGuidedFilter();

	imageCornea()->applyGuidedFilter();
	imageChamber()->applyGuidedFilter();

	PrepCornea::collectImageStats(sampleCornea());
	PrepCornea::collectColumnStats(sampleCornea());
	PrepCornea::collectImageStats(sampleChamber());
	PrepCornea::collectColumnStats(sampleChamber());

	CoarseCornea::createGradientMapOfCornea(sampleCornea(), ascentCornea(), descentCornea());
	PrepCornea::collectImageStats(ascentCornea());
	PrepCornea::collectColumnStats(ascentCornea());
	PrepCornea::collectImageStats(descentCornea());
	PrepCornea::collectColumnStats(descentCornea());

	CoarseCornea::createGradientMapOfChamber(sampleChamber(), ascentChamber(), descentChamber());
	PrepCornea::collectImageStats(ascentChamber());
	PrepCornea::collectColumnStats(ascentChamber());
	PrepCornea::collectImageStats(descentChamber());
	PrepCornea::collectColumnStats(descentChamber());
	return true;
}


void CorTopo::WideCreator::upscaleBorderLines(void)
{
	int img_w = imageCornea()->getWidth();
	int img_h = imageCornea()->getHeight();
	auto line1 = std::vector<int>(img_w, -1);
	auto line2 = std::vector<int>(img_w, -1);
	auto line3 = std::vector<int>(img_w, -1);
	auto line4 = std::vector<int>(img_w, -1);
	auto line5 = std::vector<int>(img_w, -1);

	auto& hinge1 = getImpl().corneaHinge1;
	auto& hinge2 = getImpl().corneaHinge2;
	auto& hinge3 = getImpl().chamberHinge1;
	auto& hinge4 = getImpl().chamberHinge2;

	if (corneaAnteriorLine().empty() || corneaPosteriorLine().empty()) {
		return;
	}

	int size = (int)corneaAnteriorLine().size();
	float ratioH = (float)img_w / SAMPLE_IMAGE_WIDTH;
	float ratioV = (float)img_h / SAMPLE_IMAGE_HEIGHT;

	if (!hinge1.empty()) {
		for (int i = 0; i < img_w/2; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line1[i] = (int)(hinge1[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line1[c] < 0 ? 0 : line1[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
				hinge1 = fittY;
			}
		}
		line1 = std::vector<int>(img_w, -1);
	}

	if (!hinge2.empty()) {
		for (int i = img_w/2; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line2[i] = (int)(hinge2[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line2[c] < 0 ? 0 : line2[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
				hinge2 = fittY;
			}
		}
		line2 = std::vector<int>(img_w, -1);
	}

	if (!hinge3.empty()) {
		for (int i = 0; i < img_w / 2; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line3[i] = (int)(hinge3[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line3[c] < 0 ? 0 : line3[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
				hinge3 = fittY;
			}
		}
		line3 = std::vector<int>(img_w, -1);
	}

	if (!hinge4.empty()) {
		for (int i = img_w/2; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line4[i] = (int)(hinge4[idx] * ratioV);
		}

		{
			auto size = img_w;
			auto dataX = vector<int>(size, -1);
			auto dataY = vector<int>(size, -1);
			auto fittY = vector<int>(size, -1);

			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line4[c] < 0 ? 0 : line4[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 1, true, fittY, coeffs)) {
				hinge4 = fittY;
			}
		}
		line4 = std::vector<int>(img_w, -1);
	}

	auto& corAntes = corneaAnteriorLine();
	auto& corPosts = corneaPosteriorLine();
	auto& chaAntes = chamberAnteriorLine();
	auto& chaPosts = chamberPosteriorLine();
	auto& corSides = corneaAnteriorSides();

	if (!corAntes.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line1[i] = (int)(corAntes[idx] * ratioV);
		}
	}

	if (!corPosts.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line2[i] = (int)(corPosts[idx] * ratioV);
		}
	}

	if (!chaAntes.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line3[i] = (int)(chaAntes[idx] * ratioV);
		}
	}

	if (!chaPosts.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line4[i] = (int)(chaPosts[idx] * ratioV);
		}
	}

	if (!corSides.empty()) {
		for (int i = 0; i < img_w; i++) {
			int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
			line5[i] = (int)(corSides[idx] * ratioV);
		}
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

		if (!corPosts.empty()) {
			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line2[c] < 0 ? 0 : line2[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				corPosts = fittY;
			}
		}
		else {
			corPosts.clear();
		}

		if (!chaAntes.empty()) {
			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line3[c] < 0 ? 0 : line3[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				chaAntes = fittY;
			}
		}
		else {
			chaAntes.clear();
		}

		if (!chaPosts.empty()) {
			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line4[c] < 0 ? 0 : line4[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				chaPosts = fittY;
			}
		}
		else {
			chaPosts.clear();
		}

		if (!corSides.empty()) {
			for (int c = 0; c < size; c++) {
				dataX[c] = c;
				dataY[c] = (line5[c] < 0 ? 0 : line5[c]);
			}

			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				corSides = fittY;
			}
		}
		else {
			corSides.clear();
		}
	}

	if (corAntes.size() > 0) {
		int size = (int) corAntes.size();
		int min_val = 9999;
		int min_pos = -1;
		for (int i = 0; i < size; i++) {
			if (corAntes[i] < min_val) {
				min_val = corAntes[i];
				min_pos = i;
			}
		}
		int xsum = 0;
		int wsum = 0;
		for (int i = min_pos; i < size; i++) {
			if (corAntes[i] == min_val) {
				xsum += i;
				wsum += 1;
			}
		}
		int centerX = xsum / wsum;
		int centerY = min_val;
		getImpl().topCorneaCenterX = centerX;
		getImpl().topCorneaCenterY = centerY;
	}

	if (chaPosts.size() > 0) {
		int size = (int) chaPosts.size();
		int max_val = -9999;
		int max_pos = -1;
		for (int i = 0; i < size; i++) {
			if (chaPosts[i] > max_val) {
				max_val = chaPosts[i];
				max_pos = i;
			}
		}
		int xsum = 0;
		int wsum = 0;
		for (int i = max_pos; i < size; i++) {
			if (chaPosts[i] == max_val) {
				xsum += i;
				wsum += 1;
			}
		}
		int centerX = xsum / wsum;
		int centerY = max_val;
		getImpl().revCorneaCenterX = centerX;
		getImpl().revCorneaCenterY = centerY;
	}

	if (corAntes.size() > 0 && chaPosts.size() > 0) {
		int offsetX, offsetY;
		offsetX = getImpl().topCorneaCenterX - getImpl().revCorneaCenterX;
		offsetY = ((img_h - getImpl().topCorneaCenterY) - getImpl().revCorneaCenterY);
		getImpl().reversedOffsetX = offsetX;
		getImpl().reversedOffsetY = offsetY;

		// LogD() << "Updated offsets: " << offsetX << ", " << offsetY;
	}
	else {
		getImpl().reversedOffsetX = 0;
		getImpl().reversedOffsetY = 0;
	}
	return;
}



std::vector<int>& CorTopo::WideCreator::corneaAnteriorLine(void) const
{
	return getImpl().corneaAntes;
}


std::vector<int>& CorTopo::WideCreator::corneaPosteriorLine(void) const
{
	return getImpl().corneaPosts;
}

std::vector<int>& CorTopo::WideCreator::corneaAnteriorSides(void) const
{
	return getImpl().corneaSides;
}

std::vector<int>& CorTopo::WideCreator::chamberAnteriorLine(void) const
{
	return getImpl().chamberAntes;
}


std::vector<int>& CorTopo::WideCreator::chamberPosteriorLine(void) const
{
	return getImpl().chamberPosts;
}

std::vector<int>& CorTopo::WideCreator::chamberHingeLine(int index) const
{
	return (index == 0 ? getImpl().chamberHinge1 : getImpl().chamberHinge2);
}


RetSegm::SegmImage * CorTopo::WideCreator::imageAnterior(void) const
{
	return &(getImpl().imgAnterior);
}

RetSegm::SegmImage* CorTopo::WideCreator::imageCornea(void) const
{
	return &(getImpl().imgCornea);
}


RetSegm::SegmImage* CorTopo::WideCreator::imageChamber(void) const
{
	return &(getImpl().imgChamber);
}


RetSegm::SegmImage* CorTopo::WideCreator::sampleCornea(void) const
{
	return &(getImpl().imgCornea2);
}


RetSegm::SegmImage* CorTopo::WideCreator::sampleChamber(void) const
{
	return &(getImpl().imgChamber2);
}

RetSegm::SegmImage * CorTopo::WideCreator::ascentCornea(void) const
{
	return &(getImpl().imgCorneaAsc);
}

RetSegm::SegmImage * CorTopo::WideCreator::descentCornea(void) const
{
	return &(getImpl().imgCorneaDes);
}

RetSegm::SegmImage * CorTopo::WideCreator::ascentChamber(void) const
{
	return &(getImpl().imgChamberAsc);
}

RetSegm::SegmImage * CorTopo::WideCreator::descentChamber(void) const
{
	return &(getImpl().imgChamberDes);
}
