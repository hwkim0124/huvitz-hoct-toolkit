#include "stdafx.h"
#include "DiscReport2.h"

#include "CppUtil2.h"
#include "OctResult2.h"
#include "OctData2.h"
#include "SegmScan2.h"


using namespace OctReport;
using namespace OctResult;
using namespace OctData;
using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct DiscReport2::DiscReport2Impl
{
	bool isOpticCup;
	bool isOpticDisc;

	int discStartLine;
	int discCloseLine;
	int discCenterLine;
	int discCenterXpos;
	int discXposMin;
	int discXposMax;

	float discArea;
	float discVolume;

	int cupStartLine;
	int cupCloseLine;
	int cupCenterLine;
	int cupCenterXpos;
	int cupXposMin;
	int cupXposMax;

	float cupArea;
	float cupVolume;

	float rimArea;
	float rimVolume;

	bool isNerveHeadCenter;
	int nerveHeadCenterLine;
	int nerveHeadCenterXpos;
	float nerveHeadCenterX;
	float nerveHeadCenterY;

	bool isCupDiscRatio;
	float cupDiscHorzRatio;
	float cupDiscVertRatio;
	float cupDiscAreaRatio;

	DiscReport2Impl() : isOpticCup(false), isOpticDisc(false), isNerveHeadCenter(false), 
		cupDiscHorzRatio(0.0f), cupDiscVertRatio(0.0f), isCupDiscRatio(false)
	{
	}
};


OctReport::DiscReport2::DiscReport2() :
	d_ptr(make_unique<DiscReport2Impl>())
{
}


OctReport::DiscReport2::~DiscReport2() = default;
OctReport::DiscReport2::DiscReport2(DiscReport2 && rhs) = default;
DiscReport2 & OctReport::DiscReport2::operator=(DiscReport2 && rhs) = default;

bool OctReport::DiscReport2::isNerveHeadCup(void) const
{
	return d_ptr->isOpticCup;
}


bool OctReport::DiscReport2::isNerveHeadDisc(void) const
{
	return d_ptr->isOpticDisc;
}


bool OctReport::DiscReport2::isNerveHeadCenter(void) const
{
	return d_ptr->isNerveHeadCenter;
}


int OctReport::DiscReport2::getNerveHeadCenterLineIndex(void) const
{
	return d_ptr->nerveHeadCenterLine;
}


int OctReport::DiscReport2::getNerveHeadCenterLateralPos(void) const
{
	return d_ptr->nerveHeadCenterXpos;
}


float OctReport::DiscReport2::getNerveHeadCenterX(void) const
{
	return d_ptr->nerveHeadCenterX;
}


float OctReport::DiscReport2::getNerveHeadCenterY(void) const
{
	return d_ptr->nerveHeadCenterY;
}


float OctReport::DiscReport2::getDiscArea(void) const
{
	return d_ptr->discArea;
}


float OctReport::DiscReport2::getDiscVolume(void) const
{
	return d_ptr->discVolume;
}


float OctReport::DiscReport2::getCupArea(void) const
{
	return d_ptr->cupArea;
}


float OctReport::DiscReport2::getCupVolume(void) const
{
	return d_ptr->cupVolume;
}


float OctReport::DiscReport2::getRimArea(void) const
{
	return d_ptr->rimArea;
}


float OctReport::DiscReport2::getCupDiscHorzRatio(void) const
{
	return d_ptr->cupDiscHorzRatio;
}


float OctReport::DiscReport2::getCupDiscVertRatio(void) const
{
	return d_ptr->cupDiscVertRatio;
}


float OctReport::DiscReport2::getCupDiscAreaRatio(void) const
{
	return d_ptr->cupDiscAreaRatio;
}


bool OctReport::DiscReport2::updateContents(void)
{
	MacularReport2::updateContents();

	clearContents();

	if (isEmpty()) {
		return false;
	}

	if (getDescript()->isEnfaceScan() ||
		getDescript()->isRadialScan() ||
		getDescript()->isRasterScan()) {
	}
	else {
		return false;
	}

	locateCupDiscCenter();
	calculateCupDiscRatio();
	calculateCupDiscArea();
	calculateCupDiscVolume();
	return true;
}


void OctReport::DiscReport2::clearContents(void)
{
	d_ptr->isNerveHeadCenter = false;
	d_ptr->isCupDiscRatio = false;
	d_ptr->isOpticCup = false;
	d_ptr->isOpticDisc = false;

	d_ptr->nerveHeadCenterLine = 0;
	d_ptr->nerveHeadCenterXpos = 0;
	d_ptr->nerveHeadCenterX = 0.0f;
	d_ptr->nerveHeadCenterY = 0.0f;

	d_ptr->discArea = 0.0f;
	d_ptr->discVolume = 0.0f;
	d_ptr->cupArea = 0.0f;
	d_ptr->cupVolume = 0.0f;
	d_ptr->cupDiscHorzRatio = 0.0f;
	d_ptr->cupDiscVertRatio = 0.0f;
	d_ptr->cupDiscAreaRatio = 0.0f;
	d_ptr->rimArea = 0.0f;
	d_ptr->rimVolume = 0.0f;
	return;
}


SegmScan::DiscEshot * OctReport::DiscReport2::getDiscEshot(OcularLayerType upper, OcularLayerType lower)
{
	auto shot = getEnfaceImage(upper, lower);
	return static_cast<DiscEshot*>(shot);
}


SegmScan::DiscEplot * OctReport::DiscReport2::getDiscEplot(OcularLayerType upper, OcularLayerType lower)
{
	auto tmap = getThicknessMap(upper, lower);
	return static_cast<DiscEplot*>(tmap);
}


SegmScan::DiscEnfaceImage * OctReport::DiscReport2::getEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto shot = prepareEnfaceImage(upper, lower, upperOffset, lowerOffset);
	return static_cast<DiscEnfaceImage*>(shot);
}


SegmScan::DiscThicknessMap * OctReport::DiscReport2::getThicknessMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto tmap = prepareEnfaceMap(upper, lower, upperOffset, lowerOffset);
	return static_cast<DiscThicknessMap*>(tmap);
}


SegmScan::DiscClockChart OctReport::DiscReport2::makeDiscClockChart(OcularLayerType upper, OcularLayerType lower, 
	float upperOffset, float lowerOffset, float centerX, float centerY, float outerDiam)
{
	DiscClockChart chart;
	chart.setCircleSize(outerDiam);

	if (getProtocolData()->getDescript().isCircleScan()) {
		if (getProtocolData()->getBscanSegmList().size() > 0) {
			auto bsegm = getProtocolData()->getBscanSegmList()[0];
			if (bsegm) {
				vector<float> thicks;
				if (bsegm->getThicknessLine(upper, lower, thicks, 0, 0, upperOffset, lowerOffset)) {
					chart.setEyeSide(getEyeSide());
					chart.updateMetrics(thicks);
				}
			}
		}
	}
	else {
		auto tmap = getThicknessMap(upper, lower, upperOffset, lowerOffset);
		if (tmap) {
			chart.setEnfaceData(tmap->getThicknessMap());
			chart.setEyeSide(getEyeSide());
			chart.setLocation(centerX, centerY);
			chart.updateMetrics();
		}
	}

	return chart;
}


SegmScan::DiscChart OctReport::DiscReport2::makeDiscChart(OcularLayerType upper, OcularLayerType lower, float centerX, float centerY, float outerDiam)
{
	DiscChart chart;
	chart.setCircleSize(outerDiam);

	if (getProtocolData()->getDescript().isCircleScan()) {
		if (getProtocolData()->getBscanSegmList().size() > 0) {
			auto bsegm = getProtocolData()->getBscanSegmList()[0];
			if (bsegm) {
				vector<float> thicks;
				if (bsegm->getThicknessLine(upper, lower, thicks)) {
					chart.setEyeSide(getEyeSide());
					chart.updateMetrics(thicks);
				}
			}
		}
	}
	else {
		auto tmap = getDiscEplot(upper, lower);
		if (tmap) {
			chart.setEnfaceData(tmap->getThicknessMap());
			chart.setEyeSide(getEyeSide());
			chart.setLocation(centerX, centerY);
			chart.updateMetrics();
		}
	}

	return chart;
}


void OctReport::DiscReport2::locateCupDiscCenter(void)
{
	int size = getProtocolData()->countBscanData();

	auto disc_lines = std::vector<int>();
	auto disc_cents = std::vector<int>();
	auto disc_xmins = std::vector<int>();
	auto disc_xmaxs = std::vector<int>();

	int x1, x2;
	for (int i = 0; i < size; i++) {
		auto p = getProtocolData()->getBscanData(i);
		if (p->getOpticNerveDiscRange(x1, x2)) {
			disc_lines.push_back(i);
			disc_cents.push_back((x1 + x2) / 2);
			disc_xmins.push_back(x1);
			disc_xmaxs.push_back(x2);
		}
	}

	if (!disc_lines.empty()) {
		d_ptr->isOpticDisc = true;
		d_ptr->discStartLine = disc_lines.front();
		d_ptr->discCloseLine = disc_lines.back();

		sort(disc_cents.begin(), disc_cents.end());
		sort(disc_xmins.begin(), disc_xmins.end());
		sort(disc_xmaxs.begin(), disc_xmaxs.end());

		d_ptr->discCenterLine = disc_lines[disc_lines.size() / 2];
		d_ptr->discCenterXpos = disc_cents[disc_cents.size() / 2];
		d_ptr->discXposMin = disc_xmins.front();
		d_ptr->discXposMax = disc_xmaxs.back();

		auto p = getProtocolData()->getBscanData(d_ptr->discCenterLine);
		if (p->getOpticNerveDiscRange(x1, x2)) {
			d_ptr->discXposMin = x1;
			d_ptr->discXposMax = x2;
		}
	}
	else {
		d_ptr->isOpticDisc = false;
	}

	auto cup_lines = std::vector<int>();
	auto cup_cents = std::vector<int>();
	auto cup_xmins = std::vector<int>();
	auto cup_xmaxs = std::vector<int>();

	for (int i = 0; i < size; i++) {
		auto p = getProtocolData()->getBscanData(i);
		if (p->getOpticNerveCupRange(x1, x2)) {
			cup_lines.push_back(i);
			cup_cents.push_back((x1 + x2) / 2);
			cup_xmins.push_back(x1);
			cup_xmaxs.push_back(x2);
		}
	}

	if (!cup_lines.empty()) {
		d_ptr->isOpticCup = true;
		d_ptr->cupStartLine = cup_lines.front();
		d_ptr->cupCloseLine = cup_lines.back();

		sort(cup_cents.begin(), cup_cents.end());
		sort(cup_xmins.begin(), cup_xmins.end());
		sort(cup_xmaxs.begin(), cup_xmaxs.end());

		d_ptr->cupCenterLine = cup_lines[cup_lines.size() / 2];
		d_ptr->cupCenterXpos = cup_cents[cup_cents.size() / 2];
		d_ptr->cupXposMin = cup_xmins.front();
		d_ptr->cupXposMax = cup_xmaxs.back();

		auto p = getProtocolData()->getBscanData(d_ptr->cupCenterLine);
		if (p->getOpticNerveCupRange(x1, x2)) {
			d_ptr->cupXposMin = x1;
			d_ptr->cupXposMax = x2;
		}
	}
	else {
		d_ptr->isOpticCup = false;
	}

	/*
	if (d_ptr->isOpticCup) {
		if (cup_lines.size() >= (int)(disc_lines.size() * 0.25f)) {
			d_ptr->nerveHeadCenterLine = d_ptr->cupCenterLine;
			d_ptr->nerveHeadCenterXpos = d_ptr->cupCenterXpos;
		}
		else {
			d_ptr->nerveHeadCenterLine = d_ptr->discCenterLine;
			d_ptr->nerveHeadCenterXpos = d_ptr->discCenterXpos;
		}
		d_ptr->isNerveHeadCenter = true;
	}
	else */ if (d_ptr->isOpticDisc) {
		d_ptr->nerveHeadCenterLine = d_ptr->discCenterLine;
		d_ptr->nerveHeadCenterXpos = d_ptr->discCenterXpos;
		d_ptr->isNerveHeadCenter = true;
	}
	else {
		d_ptr->isNerveHeadCenter = false;
	}

	if (d_ptr->isNerveHeadCenter) {
		float rangeX = getDescript()->getScanRangeX();
		float rangeY = getDescript()->getScanRangeY();
		int numLines = getDescript()->getNumberOfScanLines();
		int numPoints = getDescript()->getNumberOfScanPoints();

		int centerLine = d_ptr->nerveHeadCenterLine;
		int centerXpos = d_ptr->nerveHeadCenterXpos;

		float centerX, centerY;
		if (getDescript()->isHorizontal()) {
			centerX = (((float)centerXpos / (float)numPoints) - 0.5f) * rangeX;
			centerY = (((float)centerLine / (float)numLines) - 0.5f) * rangeY;
		}
		else {
			centerY = (((float)centerXpos / (float)numPoints) - 0.5f) * rangeY;
			centerX = (((float)centerLine / (float)numLines) - 0.5f) * rangeX;
		}

		d_ptr->nerveHeadCenterX = centerX;
		d_ptr->nerveHeadCenterY = centerY;

		LogD() << "Nerve head center, line: " << centerLine << ", xpos: " << centerXpos;
	}
	return;
}


void OctReport::DiscReport2::calculateCupDiscArea(void)
{
	if (!d_ptr->isNerveHeadCenter) {
		return;
	}

	float unit_w = getProtocolData()->getPixelWidth();
	float unit_h = getProtocolData()->getLinesDistance();
	float unit_a = unit_w * unit_h;

	float cupArea = 0.0f;
	float discArea = 0.0f;

	int x1, x2;
	int pixels = 0;

	if (d_ptr->isOpticDisc) {
		for (int i = d_ptr->discStartLine; i <= d_ptr->discCloseLine; i++) {
			auto p = getProtocolData()->getBscanData(i);

			if (p->getOpticNerveDiscRange(x1, x2)) {
				// LogD() << "Area line: " << i << ", disc range: " << x1 << ", " << x2;
				pixels += (x2 - x1 + 1);
			}
		}

		discArea = pixels * unit_a;
	}

	if (d_ptr->isOpticCup) {
		pixels = 0;
		for (int i = d_ptr->cupStartLine; i <= d_ptr->cupCloseLine; i++) {
			auto p = getProtocolData()->getBscanData(i);

			if (p->getOpticNerveCupRange(x1, x2)) {
				// LogD() << "Area line: " << i << ", cup range: " << x1 << ", " << x2;
				pixels += (x2 - x1 + 1);
			}
		}

		cupArea = pixels * unit_a;
	}

	d_ptr->discArea = discArea;
	d_ptr->cupArea = cupArea;
	d_ptr->rimArea = discArea - cupArea;
	d_ptr->cupDiscAreaRatio = cupArea / discArea;

	LogD() << "Disc area: " << discArea << ", cup area: " << cupArea << ", ratio: " << d_ptr->cupDiscAreaRatio;
	return;
}


void OctReport::DiscReport2::calculateCupDiscVolume(void)
{
	if (!d_ptr->isNerveHeadCenter) {
		return;
	}

	float unit_w = getProtocolData()->getPixelWidth();
	float unit_d = getProtocolData()->getPixelHeight() * 0.001f;
	float unit_h = getProtocolData()->getLinesDistance();
	float unit_v = unit_w * unit_h * unit_d;

	float cupVolume = 0.0f;
	float discVolume = 0.0f;

	int pixels = 0;
	int voxels = 0;

	if (d_ptr->isOpticDisc) {
		for (int i = d_ptr->discStartLine; i <= d_ptr->discCloseLine; i++) {
			auto p = getProtocolData()->getBscanData(i);

			if (p->getOpticNerveDiscPixels(pixels)) {
				voxels += pixels;
			}
		}

		discVolume = voxels * unit_v;
	}

	if (d_ptr->isOpticCup) {
		pixels = 0;
		voxels = 0;
		for (int i = d_ptr->cupStartLine; i <= d_ptr->cupCloseLine; i++) {
			auto p = getProtocolData()->getBscanData(i);

			if (p->getOpticNerveCupPixels(pixels)) {
				voxels += pixels;
			}
		}

		cupVolume = voxels * unit_v;
	}

	d_ptr->discVolume = discVolume;
	d_ptr->cupVolume = cupVolume;
	return;
}


void OctReport::DiscReport2::calculateCupDiscRatio(void)
{


	if (d_ptr->isOpticCup && d_ptr->isOpticDisc)
	{
		int discLines = (d_ptr->discCloseLine - d_ptr->discStartLine + 1);
		// int discWidth = min(abs(d_ptr->discCenterXpos - d_ptr->discXposMax), abs(d_ptr->discCenterXpos - d_ptr->discXposMin)) + 1;
		int discWidth = (d_ptr->discXposMax - d_ptr->discXposMin + 1);

		int cupLines = (d_ptr->cupCloseLine - d_ptr->cupStartLine + 1);
		// int cupWidth = min(abs(d_ptr->cupCenterXpos - d_ptr->cupXposMax), abs(d_ptr->cupCenterXpos - d_ptr->cupXposMin)) + 1;
		int cupWidth = (d_ptr->cupXposMax - d_ptr->cupXposMin + 1);

		int x1, x2;
		int dsum = 0, dcnt = 0;
		int dOffset = max((int)(discLines * 0.2588f) / 2, 2);
		for (int i = (d_ptr->discCenterLine - dOffset); i <= (d_ptr->discCenterLine + dOffset); i++) {
			if (i < 0 || i >= getProtocolData()->countBscanData()) {
				continue;
			}
			auto p = getProtocolData()->getBscanData(i);
			if (p->getOpticNerveDiscRange(x1, x2)) {
				dsum += (x2 - x1 + 1);
				dcnt += 1;
			}
		}
		if (dcnt > 0) {
			discWidth = dsum / dcnt;
		}

		int csum = 0, ccnt = 0;
		int cOffset = max((int)(cupLines * 0.2588f) / 2, 1);
		for (int i = (d_ptr->cupCenterLine - cOffset); i <= (d_ptr->cupCenterLine + cOffset); i++) {
			if (i < 0 || i >= getProtocolData()->countBscanData()) {
				continue;
			}
			auto p = getProtocolData()->getBscanData(i);
			if (p->getOpticNerveCupRange(x1, x2)) {
				csum += (x2 - x1 + 1);
				ccnt += 1;
			}
		}
		if (ccnt > 0) {
			cupWidth = csum / ccnt;
		}

		if (getProtocolData()->getDescript().isHorizontal()) {
			d_ptr->cupDiscHorzRatio = (float)cupWidth / (float)discWidth;
			d_ptr->cupDiscVertRatio = (float)cupLines / (float)discLines;
		}
		else {
			d_ptr->cupDiscHorzRatio = (float)cupLines / (float)discLines;
			d_ptr->cupDiscVertRatio = (float)cupWidth / (float)discWidth;
		}
		d_ptr->isCupDiscRatio = true;
	}
	return;
}


std::unique_ptr<SegmScan::OcularEnfaceImage> OctReport::DiscReport2::createEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto enface = unique_ptr<DiscEnfaceImage>(new DiscEnfaceImage);

	const auto& desc = getProtocolData()->getDescript();
	const auto& bsegms = getProtocolData()->getBscanSegmList();

	bool result = enface->setupEnfaceImage(desc, upper, lower, upperOffset, lowerOffset, bsegms);

	if (result) {
		return std::move(enface);
	}
	return nullptr;
}


std::unique_ptr<SegmScan::OcularEnfaceMap> OctReport::DiscReport2::createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto plot = unique_ptr<DiscEnfaceMap>(new DiscEnfaceMap);

	const auto& desc = getProtocolData()->getDescript();
	const auto& bsegms = getProtocolData()->getBscanSegmList();

	bool result = plot->setupThicknessMap(desc, upper, lower, upperOffset, lowerOffset, bsegms);

	if (result) {
		return std::move(plot);
	}
	return nullptr;
}



DiscReport2::DiscReport2Impl & OctReport::DiscReport2::getImpl(void) const
{
	return *d_ptr;
}