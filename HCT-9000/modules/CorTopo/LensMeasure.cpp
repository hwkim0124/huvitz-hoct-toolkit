#include "stdafx.h"
#include "LensMeasure.h"
#include "AnteriorLens.h"

#include "CppUtil2.h"
#include "RetSegm2.h"
#include "SegmScan2.h"

#include <boost/format.hpp>
#include <algorithm> 

using namespace CorTopo;
using namespace CppUtil;
using namespace RetSegm;
using namespace SegmScan;
using namespace cv;
using namespace std;


struct LensMeasure::LensMeasureImpl
{
	SegmImage imgCornea;

	std::wstring corneaName;
	std::wstring lensFrontName;
	std::wstring lensBackName;
	bool isResult = false;

	CorneaBsegm corneaSegm;
	AnteriorLens lensFront;
	AnteriorLens lensBack;

	vector<int> corneaAnteLine;
	vector<int> corneaPostLine;
	vector<int> lensAnteLine;
	vector<int> lensPostLine;

	int corneaReferPos = -1;
	int corneaMotorPos = -1;
	int lensFrontReferPos = -1;
	int lensFrontMotorPos = -1;
	int lensBackReferPos = -1;
	int lensBackMotorPos = -1;

	float corneaCCT = 0.0f;
	int corneaCenterX = 0;
};


LensMeasure::LensMeasure() :
	d_ptr(make_unique<LensMeasureImpl>())
{
}


CorTopo::LensMeasure::~LensMeasure() = default;
CorTopo::LensMeasure::LensMeasure(LensMeasure && rhs) = default;
LensMeasure & CorTopo::LensMeasure::operator=(LensMeasure && rhs) = default;


LensMeasure::LensMeasureImpl & CorTopo::LensMeasure::getImpl(void) const
{
	return *d_ptr;
}


bool CorTopo::LensMeasure::loadCorneaImage(const std::wstring & path, int motorPos)
{
	try {
		if (!path.empty() && !getImpl().imgCornea.fromFile(path)) {
			LogD() << "Failed to load cornea image, path: " + wtoa(path);
			return false;
		}

		getImpl().corneaName = path;
		getImpl().corneaMotorPos = motorPos;
		getImpl().corneaReferPos = -1;
		getImpl().isResult = false;

		auto* bits = getImpl().imgCornea.getBitsData();
		auto w = getImpl().imgCornea.getWidth();
		auto h = getImpl().imgCornea.getHeight();

		if (!loadCorneaSegmSource(bits, w, h, motorPos)) {
			LogD() << "Failed to load cornea image, path: " + wtoa(path);
			return false;
		}
		return true;
	}
	catch (...) {
		LogD() << "Failed to load retina image";
	}
	return false;
}


bool CorTopo::LensMeasure::loadCorneaSegmSource(const unsigned char * bits, int width, int height, int motorPos)
{
	try {
		if (bits && !getImpl().corneaSegm.loadSource(bits, width, height)) {
			LogD() << "Failed to load cornea bitmap";
			return false;
		}

		OctScanPattern desc;
		desc.setup(PatternName::AnteriorLine, width, 1, 4.5f, 0.0f, 0, 1, 0.0f, ScanSpeed::Normal);
		getImpl().corneaSegm.setPatternDescript(desc);
		return true;
	}
	catch (...) {
		LogD() << "Failed to load cornea bitmap";
	}
	return false;
}


bool CorTopo::LensMeasure::loadLensFrontImage(const std::wstring & path, int motorPos)
{
	try {
		if (!getImpl().lensFront.loadCorneaImage(path)) {
			return false;
		}

		getImpl().lensFrontName = path;
		getImpl().lensFrontMotorPos = motorPos;
		getImpl().lensFrontReferPos = -1;
		getImpl().isResult = false;
		return true;
	}
	catch (...) {
		LogD() << "Failed to load lens front image";
	}
	return false;
}


bool CorTopo::LensMeasure::loadLensBackImage(const std::wstring & path, int motorPos)
{
	try {
		if (!getImpl().lensBack.loadCorneaImage(path, true)) {
			return false;
		}

		getImpl().lensBackName = path;
		getImpl().lensBackMotorPos = motorPos;
		getImpl().lensBackReferPos = -1;
		getImpl().isResult = false;
		return true;
	}
	catch (...) {
		LogD() << "Failed to load lens back image";
	}
	return false;
}


bool CorTopo::LensMeasure::isCorneaImageLoaded(void)
{
	if (!getImpl().corneaSegm.isEmpty()) {
		return true;
	}
	return false;
}


bool CorTopo::LensMeasure::isLensFrontImageLoaded(void)
{
	if (!getImpl().lensFront.isCorneaImageLoaded()) {
		return false;
	}
	return true;

}


bool CorTopo::LensMeasure::isLensBackImageLoaded(void)
{
	if (!getImpl().lensBack.isCorneaImageLoaded()) {
		return false;
	}
	return true;

}


bool CorTopo::LensMeasure::performCalculation(void)
{
	if (!getImpl().corneaSegm.performAnalysis()) {
		LogD() << "Failed to segment cornea layers";
		getImpl().corneaAnteLine.clear();
		getImpl().corneaPostLine.clear();
		getImpl().corneaReferPos = TARGET_REFER_POST_INIT;
		getImpl().corneaCCT = 0.0f; 
	}
	else {
		getImpl().corneaAnteLine = getImpl().corneaSegm.getCorneaLayers()->getEPI()->getYs();
		getImpl().corneaPostLine = getImpl().corneaSegm.getCorneaLayers()->getEND()->getYs();

		auto min_pos = min_element(getImpl().corneaAnteLine.begin(), getImpl().corneaAnteLine.end());
		auto min_idx = std::distance(getImpl().corneaAnteLine.begin(), min_pos);
		auto cct_size = abs(getImpl().corneaPostLine[min_idx] - getImpl().corneaAnteLine[min_idx]);
		auto cct_value = (float)(cct_size * GlobalSettings::getCorneaScanAxialResolution());
		auto refer_pos = *min_element(getImpl().corneaPostLine.begin(), getImpl().corneaPostLine.end());
		LogD() << "Cornea center pos: " << min_idx << ", cct size: " << cct_size << ", value: " << cct_value << ", referPos: " << refer_pos;
		getImpl().corneaCCT = cct_value;
		getImpl().corneaReferPos = refer_pos;
	}

	if (!getImpl().lensFront.makeupAnteriorLensBorder()) {
		LogD() << "Failed to segment lens anterior line";
		getImpl().lensAnteLine.clear();
		getImpl().lensFrontReferPos = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().lensAnteLine = getImpl().lensFront.lensAnteriorLine();
		getImpl().lensFrontReferPos = getImpl().lensFront.getAnteriorLensPosition();
	}

	if (!getImpl().lensBack.makeupPosteriorLensBorder()) {
		LogD() << "Failed to segment lens posterior line";
		getImpl().lensPostLine.clear();
		getImpl().lensBackReferPos = TARGET_REFER_POST_INIT;
	}
	else {
		getImpl().lensPostLine = getImpl().lensBack.lensPosteriorLine();
		getImpl().lensBackReferPos = getImpl().lensBack.getPosteriorLensPosition();
	}

	if (getImpl().lensFrontReferPos >= 0 && getImpl().lensBackReferPos >= 0 && getImpl().corneaReferPos >= 0) {
		getImpl().isResult = true;
	}
	return true;

}


bool CorTopo::LensMeasure::performCalculation2(void)
{
	if (!getImpl().corneaSegm.performAnalysis(true)) {
		LogD() << "Failed to segment cornea layers";
		getImpl().corneaAnteLine.clear();
		getImpl().corneaPostLine.clear();
		getImpl().corneaReferPos = TARGET_REFER_POST_INIT;
		getImpl().corneaCCT = 0.0f;
	}
	else {
		auto y1 = getImpl().corneaSegm.getCorneaLayers()->getEPI()->getYs();
		auto y2 = getImpl().corneaSegm.getCorneaLayers()->getEND()->getYs();
		getImpl().corneaAnteLine = y1;
		getImpl().corneaPostLine = y2;

		auto min_pos = min_element(getImpl().corneaAnteLine.begin(), getImpl().corneaAnteLine.end());
		auto min_idx = std::distance(getImpl().corneaAnteLine.begin(), min_pos);
		auto cct_size = abs(getImpl().corneaPostLine[min_idx] - getImpl().corneaAnteLine[min_idx]);
		auto cct_value = (float)(cct_size * GlobalSettings::getCorneaScanAxialResolution());
		auto refer_pos = *min_element(getImpl().corneaPostLine.begin(), getImpl().corneaPostLine.end());
		LogD() << "Cornea center pos: " << min_idx << ", cct size: " << cct_size << ", value: " << cct_value << ", referPos: " << refer_pos;
		getImpl().corneaCCT = cct_value;
		getImpl().corneaReferPos = refer_pos;
	}

	if (getImpl().corneaReferPos >= 0) {
		getImpl().isResult = true;
	}
	return isResult();
}


bool CorTopo::LensMeasure::isResult(void)
{
	return getImpl().isResult;
}


float CorTopo::LensMeasure::getLensThickness(void)
{
	if (!isResult()) {
		return 0.0f;
	}

	auto frontRefer = getImpl().lensFrontMotorPos;
	auto backRefer = getImpl().lensBackMotorPos;
	auto frontLine = getImpl().lensFrontReferPos;
	auto backLine = getImpl().lensBackReferPos;

	auto length = getLensThickness(frontRefer, frontLine, backRefer, backLine);
	return length;
}


float CorTopo::LensMeasure::getLensThickness(int frontReferPos, int frontLinePos, int backReferPos, int backLinePos)
{
	auto length = (abs(backReferPos - frontReferPos) * 1.25f) / (1000.0f * 1.44f);
	LogD() << "Front pos: " << frontReferPos << ", back pos: " << backReferPos << ", length: " << length;

	int img_cy = 384;
	float corneaResol = (float)GlobalSettings::getCorneaScanAxialResolution();
	float retinaResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int frontOffs = (img_cy - frontLinePos);
	frontOffs = (frontLinePos < 0 ? 0 : frontOffs);
	float frontAdds = frontOffs * retinaResol * 0.001f;

	int backOffs = (backLinePos - img_cy);
	backOffs = (backLinePos < 0 ? 0 : backOffs);
	float backAdds = backOffs * retinaResol * 0.001f;

	length = length + backAdds + frontAdds;

	LogD() << "Front line: " << frontLinePos << ", offset: " << frontOffs << " => " << frontAdds << ", " << retinaResol;
	LogD() << "Back line: " << backLinePos << ", offset: " << backOffs << " => " << backAdds << ", " << retinaResol;
	LogD() << "Lens thickness: " << length;
	return length;
}


float CorTopo::LensMeasure::getAnteriorChamberDistance(void)
{
	if (!isResult()) {
		return 0.0f;
	}

	auto frontRefer = getImpl().lensFrontMotorPos;
	auto corneaRefer = getImpl().corneaMotorPos;
	auto frontLine = getImpl().lensFrontReferPos;
	auto corneaLine = getImpl().corneaReferPos;

	auto length = getAnteriorChamberDistance(corneaRefer, corneaLine, frontRefer, frontLine);
	return length;
}


float CorTopo::LensMeasure::getAnteriorChamberDistance(int corneaReferPos, int corneaLinePos, int frontReferPos, int frontLinePos)
{
	auto length = (abs(frontReferPos - corneaReferPos) * 1.25f) / (1000.0f * 1.339f);
	LogD() << "Cornea pos: " << corneaReferPos << ", front pos: " << frontReferPos << ", length: " << length;

	int img_cy = 384;
	float corneaResol = (float)GlobalSettings::getCorneaScanAxialResolution();
	float retinaResol = (float)GlobalSettings::getRetinaScanAxialResolution();

	int corneaOffs = (img_cy - corneaLinePos);
	corneaOffs = (corneaLinePos < 0 ? 0 : corneaOffs);
	float corneaAdds = corneaOffs * corneaResol * 0.001f;

	int frontOffs = (frontLinePos - img_cy);
	frontOffs = (frontLinePos < 0 ? 0 : frontOffs);
	float frontAdds = frontOffs * corneaResol * 0.001f;

	length = length + corneaAdds + frontAdds;

	LogD() << "Cornea line: " << corneaLinePos << ", offset: " << corneaOffs << " => " << corneaAdds << ", " << corneaResol;
	LogD() << "Front line: " << frontLinePos << ", offset: " << frontOffs << " => " << frontAdds << ", " << corneaResol;
	LogD() << "Anterior chamber distance: " << length;
	return length;

}


float CorTopo::LensMeasure::getCorneaCenterThickness(void)
{
	if (!isResult()) {
		return 0.0f;
	}
	return getImpl().corneaCCT;
}


int CorTopo::LensMeasure::getCorneaLinePosition(void)
{
	if (!isResult()) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().corneaReferPos;
}


int CorTopo::LensMeasure::getLensFrontLinePosition(void)
{
	if (!isResult()) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().lensFrontReferPos;
}


int CorTopo::LensMeasure::getLensBackLinePosition(void)
{
	if (!isResult()) {
		return TARGET_REFER_POST_INIT;
	}

	return getImpl().lensBackReferPos;
}


std::vector<int> CorTopo::LensMeasure::getCorneaAnteriorPoints(void)
{
	if (!isResult()) {
		return std::vector<int>();
	}
	return getImpl().corneaAnteLine;
}


std::vector<int> CorTopo::LensMeasure::getCorneaPosteriorPoints(void)
{
	if (!isResult()) {
		return std::vector<int>();
	}
	return getImpl().corneaPostLine;
}
