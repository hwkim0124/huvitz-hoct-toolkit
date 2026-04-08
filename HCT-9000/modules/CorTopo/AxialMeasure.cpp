#include "stdafx.h"
#include "AxialMeasure.h"
#include "AnteriorLens.h"

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


struct AxialMeasure::AxialMeasureImpl
{
	SegmImage imgCornea;
	SegmImage imgCornea2;
	SegmImage imgRetina;

	std::wstring corneaName;
	std::wstring corneaName2;
	std::wstring retinaName;
	bool isResult = false;

	MacularBsegm retinaSegm;
	CorneaBsegm corneaSegm;
	CorneaBsegm corneaSegm2;
	AnteriorLens anteriorLens;
	AnteriorLens anteriorLens2;

	vector<int> retinaLine;
	vector<int> corneaLine;
	vector<int> corneaLine2;

	int retinaReferPos = -1;
	int corneaReferPos = -1;
	int corneaReferPos2 = -1;
	int retinaMotorPos = -1;
	int corneaMotorPos = -1;
	int corneaMotorPos2 = -1;
};


AxialMeasure::AxialMeasure() :
	d_ptr(make_unique<AxialMeasureImpl>())
{
}


CorTopo::AxialMeasure::~AxialMeasure() = default;
CorTopo::AxialMeasure::AxialMeasure(AxialMeasure && rhs) = default;
AxialMeasure & CorTopo::AxialMeasure::operator=(AxialMeasure && rhs) = default;


AxialMeasure::AxialMeasureImpl & CorTopo::AxialMeasure::getImpl(void) const
{
	return *d_ptr;
}


bool CorTopo::AxialMeasure::loadRetinaImage(const std::wstring & path, int referPos)
{
	try {
		if (!path.empty() && !getImpl().imgRetina.fromFile(path)) {
			LogD() << "Failed to load retina image, path: " + wtoa(path);
			return false;
		}

		getImpl().retinaName = path;

		auto* bits = getImpl().imgRetina.getBitsData();
		auto w = getImpl().imgRetina.getWidth();
		auto h = getImpl().imgRetina.getHeight();

		if (!loadRetinaSegmSource(bits, w, h, referPos)) {
			LogD() << "Failed to load retina image, path: " + wtoa(path);
			return false;
		}
		return true;
	}
	catch (...) {
		LogD() << "Failed to load retina image";
	}
	return false;
}


bool CorTopo::AxialMeasure::loadRetinaSegmSource(const unsigned char * bits, int width, int height, int referPos)
{
	try {
		if (bits && !getImpl().retinaSegm.loadSource(bits, width, height)) {
			LogD() << "Failed to load retina bitmap";
			return false;
		}

		OctScanPattern desc;
		desc.setup(PatternName::MacularLine, width, 1, 4.5f, 0.0f, 0, 1, 0.0f, ScanSpeed::Normal);
		getImpl().retinaSegm.setPatternDescript(desc);

		getImpl().retinaMotorPos = referPos;
		getImpl().retinaReferPos = -1;
		getImpl().isResult = false;
		return true;
	}
	catch (...) {
		LogD() << "Failed to load retina bitmap";
	}
	return false;
}


bool CorTopo::AxialMeasure::loadCorneaImage(const std::wstring & path, int referPos)
{
	try {
		if (!getImpl().anteriorLens.loadCorneaImage(path)) {
			return false;
		}

		getImpl().corneaName = path;
		getImpl().corneaMotorPos = referPos;
		getImpl().corneaReferPos = -1;
		getImpl().isResult = false;
		return true;
	}
	catch (...) {
		LogD() << "Failed to load cornea image";
	}
	return false;
}


bool CorTopo::AxialMeasure::loadCorneaImage2(const std::wstring & path, int referPos)
{
	try {
		if (!getImpl().anteriorLens2.loadCorneaImage(path)) {
			return false;
		}

		getImpl().corneaName2 = path;
		getImpl().corneaMotorPos2 = referPos;
		getImpl().corneaReferPos2 = -1;
		getImpl().isResult = false;
		return true;
	}
	catch (...) {
		LogD() << "Failed to load cornea image2";
	}
	return false;
}


bool CorTopo::AxialMeasure::loadCorneaSegmSource(const unsigned char * bits, int width, int height, int referPos)
{
	try {
		if (bits && !getImpl().corneaSegm.loadSource(bits, width, height)) {
			LogD() << "Failed to load cornea bitmap";
			return false;
		}

		OctScanPattern desc;
		desc.setup(PatternName::AnteriorLine, width, 1, 4.5f, 0.0f, 0, 1, 0.0f, ScanSpeed::Normal);
		getImpl().corneaSegm.setPatternDescript(desc);

		getImpl().corneaMotorPos = referPos;
		getImpl().corneaReferPos = -1;
		getImpl().isResult = false;
		return true;
	}
	catch (...) {
		LogD() << "Failed to load cornea bitmap";
	}
	return false;
}


bool CorTopo::AxialMeasure::isRetinaImageLoaded(void)
{
	if (!getImpl().retinaSegm.isEmpty()) {
		return true;
	}
	return false;
}


bool CorTopo::AxialMeasure::isCorneaImageLoaded(void)
{
	if (!getImpl().anteriorLens.isCorneaImageLoaded()) {
		return false;
	}
	return true;
}


bool CorTopo::AxialMeasure::isCorneaImageLoaded2(void)
{
	if (!getImpl().anteriorLens2.isCorneaImageLoaded()) {
		return false;
	}
	return true;
}


bool CorTopo::AxialMeasure::performCalculation(void)
{
	if (!isRetinaImageLoaded() || !isCorneaImageLoaded()) {
		return false;
	}

	if (!getImpl().retinaSegm.performAnalysis()) {
		LogD() << "Failed to segment retina layers";
		getImpl().retinaLine.clear();
		getImpl().retinaReferPos = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().retinaLine = getImpl().retinaSegm.getRetinaLayers()->getIOS()->getYs();
		getImpl().retinaReferPos = *max_element(getImpl().retinaLine.begin(), getImpl().retinaLine.end());
	}

	if (!getImpl().anteriorLens.makeupAnteriorCorneaBorder()) {
		LogD() << "Failed to segment cornea boundary line";
		getImpl().corneaLine.clear();
		getImpl().corneaReferPos = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().corneaLine = getImpl().anteriorLens.corneaAnteriorLine();
		getImpl().corneaReferPos = getImpl().anteriorLens.getAnteriorCorneaPosition();
	}
	
	LogD() << "Axial measure, retina line: " << getImpl().retinaReferPos << ", cornea line: " << getImpl().corneaReferPos;

	if (getImpl().retinaReferPos >= 0 && getImpl().corneaReferPos >= 0) {
		getImpl().isResult = true;
	}
	return true;
}


bool CorTopo::AxialMeasure::performCalculation2()
{
	if (!isCorneaImageLoaded2() || !isCorneaImageLoaded()) {
		return false;
	}

	if (!getImpl().anteriorLens2.makeupAnteriorCorneaBorder()) {
		LogD() << "Failed to segment cornea boundary line2";
		getImpl().corneaLine2.clear();
		getImpl().corneaReferPos2 = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().corneaLine2 = getImpl().anteriorLens2.corneaAnteriorLine();
		getImpl().corneaReferPos2 = getImpl().anteriorLens2.getAnteriorCorneaPosition();
	}

	if (!getImpl().anteriorLens.makeupAnteriorCorneaBorder()) {
		LogD() << "Failed to segment cornea boundary line";
		getImpl().corneaLine.clear();
		getImpl().corneaReferPos = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().corneaLine = getImpl().anteriorLens.corneaAnteriorLine();
		getImpl().corneaReferPos = getImpl().anteriorLens.getAnteriorCorneaPosition();
	}

	LogD() << "Axial measure, retina line: " << getImpl().corneaReferPos2 << ", cornea line: " << getImpl().corneaReferPos;

	if (getImpl().corneaReferPos2 >= 0 && getImpl().corneaReferPos >= 0) {
		getImpl().isResult = true;
	}
	return true;
}


bool CorTopo::AxialMeasure::performCalculation3()
{
	if (!isCorneaImageLoaded()) {
		return false;
	}

	if (!getImpl().anteriorLens.makeupAnteriorCorneaBorder()) {
		LogD() << "Failed to segment cornea boundary line";
		getImpl().corneaLine.clear();
		getImpl().corneaReferPos = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().corneaLine = getImpl().anteriorLens.corneaAnteriorLine();
		getImpl().corneaReferPos = getImpl().anteriorLens.getAnteriorCorneaPosition();
	}

	if (!getImpl().anteriorLens.makeupPosteriorLensBorder()) {
		LogD() << "Failed to segment cornea boundary line2";
		getImpl().corneaLine2.clear();
		getImpl().corneaReferPos2 = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().corneaLine2 = getImpl().anteriorLens.lensPosteriorLine();
		getImpl().corneaReferPos2 = getImpl().anteriorLens.getPosteriorLensPosition();
	}

	getImpl().corneaMotorPos2 = getImpl().corneaMotorPos;
	LogD() << "Axial measure, retina line: " << getImpl().corneaReferPos2 << ", cornea line: " << getImpl().corneaReferPos;

	if (getImpl().corneaReferPos2 >= 0 && getImpl().corneaReferPos >= 0) {
		getImpl().isResult = true;
	}
	return true;
}


bool CorTopo::AxialMeasure::isResult(void)
{
	return getImpl().isResult;
}


float CorTopo::AxialMeasure::getAxialLength(float refIndex)
{
	if (!isResult()) {
		return 0.0f;
	}

	auto retinaRefer = getImpl().retinaMotorPos;
	auto corneaRefer = getImpl().corneaMotorPos;
	auto retinaLine = getImpl().retinaReferPos;
	auto corneaLine = getImpl().corneaReferPos;

	auto length = getAxialLength(retinaRefer, retinaLine, corneaRefer, corneaLine, refIndex);
	return length;
}


float CorTopo::AxialMeasure::getAxialLength(int retinaReferPos, int retinaLinePos, int corneaReferPos, int corneaLinePos, float refIndex)
{
	auto index = refIndex == 0.0f ? REFRACTIVE_INDEX_MEASURE : refIndex;
	auto length = (abs(retinaReferPos - corneaReferPos) * 1.25f) / (1000.0f * index);
	LogD() << "Auto Measure: retina pos: " << retinaReferPos << ", cornea pos: " << corneaReferPos << ", length: " << length;

	if (retinaReferPos <= 0 || corneaReferPos <= 0) {
		return 0.0f;
	}

	int img_cy = 384;
	float corneaResol = (float)GlobalSettings::getCorneaScanAxialResolution();
	float retinaResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int corneaOffs = (img_cy - corneaLinePos);
	corneaOffs = (corneaLinePos <= 0 ? 0 : corneaOffs);
	float corneaAdds = corneaOffs * corneaResol * 0.001f;

	int retinaOffs = (retinaLinePos - img_cy);
	retinaOffs = (retinaLinePos <= 0 ? 0 : retinaOffs);
	float retinaAdds = retinaOffs * retinaResol * 0.001f;

	length = length + corneaAdds + retinaAdds;

	LogD() << "Cornea line: " << corneaLinePos << ", offset: " << corneaOffs << " => " << corneaAdds << ", " << corneaResol;
	LogD() << "Retina line: " << retinaLinePos << ", offset: " << retinaOffs << " => " << retinaAdds << ", " << retinaResol;
	LogD() << "Axial length: " << length;
	return length;
}


float CorTopo::AxialMeasure::getAxialLength2(float refIndex)
{
	if (!isResult()) {
		return 0.0f;
	}

	auto retinaRefer = getImpl().corneaMotorPos2;
	auto corneaRefer = getImpl().corneaMotorPos;
	auto retinaLine = getImpl().corneaReferPos2;
	auto corneaLine = getImpl().corneaReferPos;

	auto length = getAxialLength2(retinaRefer, retinaLine, corneaRefer, corneaLine, refIndex);
	return length;
}


float CorTopo::AxialMeasure::getAxialLength2(int retinaReferPos, int retinaLinePos, int corneaReferPos, int corneaLinePos, float refIndex)
{
	auto index = refIndex == 0.0f ? REFRACTIVE_INDEX_MEASURE : refIndex;
	auto length = (abs(retinaReferPos - corneaReferPos) * 1.25f) / (1000.0f * index);
	LogD() << "Auto Measure: retina pos: " << retinaReferPos << ", cornea pos: " << corneaReferPos << ", length: " << length;

	if (retinaReferPos <= 0 || corneaReferPos <= 0) {
		return 0.0f;
	}

	int img_cy = 384;
	float corneaResol = (float)GlobalSettings::getCorneaScanAxialResolution();
	float retinaResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int corneaOffs = (img_cy - corneaLinePos);
	corneaOffs = (corneaLinePos <= 0 ? 0 : corneaOffs);
	float corneaAdds = corneaOffs * corneaResol * 0.001f;

	int retinaOffs = (retinaLinePos - img_cy);
	retinaOffs = (retinaLinePos <= 0 ? 0 : retinaOffs);
	float retinaAdds = retinaOffs * corneaResol * 0.001f;

	length = length + corneaAdds + retinaAdds;

	LogD() << "Cornea line: " << corneaLinePos << ", offset: " << corneaOffs << " => " << corneaAdds << ", " << corneaResol;
	LogD() << "Retina line: " << retinaLinePos << ", offset: " << retinaOffs << " => " << retinaAdds << ", " << retinaResol;
	LogD() << "Axial length: " << length;
	return length;
}


float CorTopo::AxialMeasure::getAxialLength3(float refIndex)
{
	if (!isResult()) {
		return 0.0f;
	}

	auto posteriorLine = getImpl().corneaReferPos2;
	auto anteriorLine = getImpl().corneaReferPos;

	float corneaResol = (float)GlobalSettings::getCorneaScanAxialResolution();
	float length = fabs(anteriorLine - posteriorLine) * corneaResol * 0.001f;

	LogD() << "Axial length: " << length;
	return length;
}


int CorTopo::AxialMeasure::getRetinaLinePosition(void)
{
	if (!isResult()) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().retinaReferPos;
}


int CorTopo::AxialMeasure::getCorneaLinePosition(void)
{
	if (!isResult()) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().corneaReferPos;
}


vector<int> CorTopo::AxialMeasure::getRetinaLinePoints(void)
{
	if (!isResult()) {
		return vector<int>();
	}
	return getImpl().retinaLine;
}


vector<int> CorTopo::AxialMeasure::getCorneaLinePoints(void)
{
	if (!isResult()) {
		return vector<int>();
	}
	return getImpl().corneaLine;
}

