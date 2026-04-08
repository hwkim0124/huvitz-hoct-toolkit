#include "stdafx.h"
#include "BscanData.h"

#include <json\json.hpp>
#include <fstream>
#include <iostream>

#include "CppUtil2.h"
#include "OctResult2.h"
#include "RetSegm2.h"
#include "SegmScan2.h"

using namespace OctData;
using namespace OctResult;
using namespace RetSegm;
using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct BscanData::BscanDataImpl
{
	BscanImage2* image;
	std::unique_ptr<OcularBsegm> bsegm;
	int idxOverlap = 0;
	int idxSection = 0;

	BscanDataImpl() : image(nullptr)
	{
	}
};


BscanData::BscanData() :
	d_ptr(make_unique<BscanDataImpl>())
{
}


OctData::BscanData::BscanData(OctResult::BscanImage2 * image) :
	d_ptr(make_unique<BscanDataImpl>())
{
	d_ptr->image = image;
}


OctData::BscanData::~BscanData() = default;
OctData::BscanData::BscanData(BscanData && rhs) = default;
BscanData & OctData::BscanData::operator=(BscanData && rhs) = default;

/*
OctData::BscanData::BscanData(const BscanData & rhs)
	: d_ptr(make_unique<BscanDataImpl>(*rhs.d_ptr))
{
}


BscanData & OctData::BscanData::operator=(const BscanData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


const OctScanImage* OctData::BscanData::getImageDescript(void) const
{
	if (!isEmpty()) {
		return &d_ptr->image->getDescript();
	}
	return nullptr;
}


OctResult::BscanImage2 * OctData::BscanData::getImageObject(void) const
{
	if (!isEmpty()) {
		return d_ptr->image;
	}
	return nullptr;
}


std::uint8_t * OctData::BscanData::getImageBuffer(void) const
{
	if (!isEmpty()) {
		return d_ptr->image->getBuffer();
	}
	return nullptr;
}


std::wstring OctData::BscanData::getImageName(bool path) const
{
	std::wstring name;
	if (!isEmpty()) {
		name = d_ptr->image->getFileName(path);
	}
	return name;
}


int OctData::BscanData::getImageWidth(void) const
{
	if (!isEmpty()) {
		return d_ptr->image->getWidth();
	}
	return 0;
}


int OctData::BscanData::getImageHeight(void) const
{
	if (!isEmpty()) {
		return d_ptr->image->getHeight();
	}
	return 0;
}

int OctData::BscanData::getSectionIndex(void) const
{
	return d_ptr->idxSection;
}

int OctData::BscanData::getOverlapIndex(void) const
{
	return d_ptr->idxOverlap;
}


bool OctData::BscanData::isBsegmResult(void) const
{
	return (d_ptr->bsegm != nullptr /* && d_ptr->bsegm->isResult() */);
}


bool OctData::BscanData::isBsegmEmpty(void) const
{
	return (d_ptr->bsegm != nullptr && d_ptr->bsegm->isEmpty());
}


void OctData::BscanData::setSectionIndex(int index)
{
	d_ptr->idxSection = index;
}

void OctData::BscanData::setOverlapIndex(int index)
{
	d_ptr->idxOverlap = index;
}

void OctData::BscanData::setOcularBsegm(std::unique_ptr<OcularBsegm> bsegm)
{
	d_ptr->bsegm = std::move(bsegm);
	return;
}


bool OctData::BscanData::employBsegmImage(void)
{
	auto segm = getOcularBsegm();
	if (segm && segm->isResult()) {
		auto image = segm->getImageObject();
		if (d_ptr->image->setImage(image)) {
			return true;
		}
	}
	return false;
}


SegmScan::OcularBsegm * OctData::BscanData::getOcularBsegm(void) const
{
	if (d_ptr->bsegm) {
		return d_ptr->bsegm.get();
	}
	return nullptr;
}


RetSegm::SegmImage * OctData::BscanData::getBsegmSource(void)
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getImageObject(OcularBsegm::OcularImageType::SOURCE);
	}
	return nullptr;
}


RetSegm::SegmImage * OctData::BscanData::getBsegmSample(void)
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getImageObject(OcularBsegm::OcularImageType::SAMPLE);
	}
	return nullptr;
}


RetSegm::SegmImage * OctData::BscanData::getBsegmDenoised(void)
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getImageObject(OcularBsegm::OcularImageType::DENOISED);
	}
	return nullptr;
}


RetSegm::SegmImage * OctData::BscanData::getBsegmGradients(void)
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getImageObject(OcularBsegm::OcularImageType::GRADIENTS);
	}
	return nullptr;
}


RetSegm::SegmImage * OctData::BscanData::getBsegmAscends(void)
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getImageObject(OcularBsegm::OcularImageType::ASCENDS);
	}
	return nullptr;
}


RetSegm::SegmImage * OctData::BscanData::getBsegmDescends(void)
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getImageObject(OcularBsegm::OcularImageType::DESCENDS);
	}
	return nullptr;
}


std::vector<int> OctData::BscanData::getLayerPoints(OcularLayerType layer, int width, int height) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getRetinaLayers()->getLayerPoints(layer, width, height);
	}
	return std::vector<int>();
}


std::vector<float> OctData::BscanData::getLayerThickness(OcularLayerType upper, OcularLayerType lower, int width, int height) const
{
	if (isBsegmResult()) {
		std::vector<float> thicks;
		if (d_ptr->bsegm->getThicknessLine(upper, lower, thicks, width, height)) {
			return thicks;
		}
	}
	return std::vector<float>();
}


std::vector<float> OctData::BscanData::getLayerDistance(OcularLayerType upper, OcularLayerType lower, int width, int height) const
{
	if (isBsegmResult()) {
		std::vector<float> thicks;
		if (d_ptr->bsegm->getDistanceLine(upper, lower, thicks, width, height)) {
			return thicks;
		}
	}
	return std::vector<float>();
}


std::vector<float> OctData::BscanData::getCurvatureRadius(void) const
{
	std::vector<float> curvs;
	if (isBsegmResult()) {
		d_ptr->bsegm->getCurvatureRadius(curvs);
	}
	return curvs;
}

std::vector<float> OctData::BscanData::getCorneaCurvature(int mapType) const
{
	std::vector<float> curvs;
	if (isBsegmResult()) {
		d_ptr->bsegm->getCorneaCurvature(curvs, mapType);
	}
	return curvs;
}

bool OctData::BscanData::isLayerPoints(OcularLayerType layer) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getRetinaLayers()->isLayerPoints(layer);
	}
	return false;
}

void OctData::BscanData::setLayerPoints(OcularLayerType layer, std::vector<int> points) const
{
	if (isBsegmResult()) {
		d_ptr->bsegm->getRetinaLayers()->setLayerPoints(layer, points);
	}
}

void OctData::BscanData::setCurvatureRadius(const std::vector<float>& rads)
{
	if (isBsegmResult()) {
		d_ptr->bsegm->setCurvatureRadius(rads);
	}
}


bool OctData::BscanData::isOpticNerveDisc(void) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->isOpticNerveDisc();
	}
	return false;
}


bool OctData::BscanData::getOpticNerveDiscRange(int & x1, int & x2) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getOpticNerveDiscRange(x1, x2);
	}
	return false;
}


bool OctData::BscanData::getOpticNerveDiscPixels(int & pixels) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getOpticNerveDiscPixels(pixels);
	}
	return false;
}


bool OctData::BscanData::isOpticNerveCup(void) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->isOpticNerveCup();
	}
	return false;
}


bool OctData::BscanData::getOpticNerveCupRange(int & x1, int & x2) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getOpticNerveCupRange(x1, x2);
	}
	return false;
}


bool OctData::BscanData::getOpticNerveCupPixels(int & pixels) const
{
	if (isBsegmResult()) {
		return d_ptr->bsegm->getOpticNerveCupPixels(pixels);
	}
	return false;
}


void OctData::BscanData::setOpticNerveDiscRange(int x1, int x2)
{
	if (isBsegmResult()) {
		d_ptr->bsegm->setOpticNerveDiscRange(x1, x2);
	}
	return;
}


void OctData::BscanData::setOpticNerveDiscPixels(int pixels)
{
	if (isBsegmResult()) {
		d_ptr->bsegm->setOpticNerveDiscPixels(pixels);
	}
	return;
}


void OctData::BscanData::setOpticNerveCupRange(int x1, int x2)
{
	if (isBsegmResult()) {
		d_ptr->bsegm->setOpticNerveCupRange(x1, x2);
	}
	return;
}


void OctData::BscanData::setOpticNerveCupPixels(int pixels)
{
	if (isBsegmResult()) {
		d_ptr->bsegm->setOpticNerveCupPixels(pixels);
	}
	return;
}


bool OctData::BscanData::isEmpty(void) const
{
	if (d_ptr->image == nullptr || d_ptr->image->isEmpty()) {
		return true;
	}
	return false;
}


bool OctData::BscanData::exportBsegmResult(std::wstring path, const OctScanPattern desc)
{
	if (!isBsegmResult()) {
		return false;
	}

	using json = nlohmann::json;
	json j;
	// j["name"] = wtoa(getImageName());
	j["width"] = getImageWidth();
	j["height"] = getImageHeight();
	
	std::vector<int> layer;
	if (!desc.isCorneaScan()) {
		if (isLayerPoints(OcularLayerType::ILM) || true) {
			layer = getLayerPoints(OcularLayerType::ILM);
			j["layers"]["ILM"] = layer;
		}
		if (isLayerPoints(OcularLayerType::NFL) || true) {
			layer = getLayerPoints(OcularLayerType::NFL);
			j["layers"]["NFL"] = layer;
		}
		if (isLayerPoints(OcularLayerType::IPL) || true) {
			layer = getLayerPoints(OcularLayerType::IPL);
			j["layers"]["IPL"] = layer;
		}
		if (isLayerPoints(OcularLayerType::OPL) || true) {
			layer = getLayerPoints(OcularLayerType::OPL);
			j["layers"]["OPL"] = layer;
		}
		if (isLayerPoints(OcularLayerType::IOS) || true) {
			layer = getLayerPoints(OcularLayerType::IOS);
			j["layers"]["IOS"] = layer;
		}
		if (isLayerPoints(OcularLayerType::RPE) || true) {
			layer = getLayerPoints(OcularLayerType::RPE);
			j["layers"]["RPE"] = layer;
		}
		if (isLayerPoints(OcularLayerType::BRM) || true) {
			layer = getLayerPoints(OcularLayerType::BRM);
			j["layers"]["BRM"] = layer;
		}
		if (isLayerPoints(OcularLayerType::OPR) || true) {
			layer = getLayerPoints(OcularLayerType::OPR);
			j["layers"]["OPR"] = layer;
		}

		if (desc.isDiscScan()) {
			int discX1, discX2, discPixels;
			int cupX1, cupX2, cupPixels;

			getOpticNerveDiscRange(discX1, discX2);
			getOpticNerveDiscPixels(discPixels);
			getOpticNerveCupRange(cupX1, cupX2);
			getOpticNerveCupPixels(cupPixels);

			j["opticDisc"]["state"] = isOpticNerveDisc();
			j["opticDisc"]["x1"] = discX1;
			j["opticDisc"]["x2"] = discX2;
			j["opticDisc"]["pixels"] = discPixels;

			j["opticCup"]["state"] = isOpticNerveCup();
			j["opticCup"]["x1"] = cupX1;
			j["opticCup"]["x2"] = cupX2;
			j["opticCup"]["pixels"] = cupPixels;
		}
	}
	else {
		if (isLayerPoints(OcularLayerType::EPI) || true) {
			layer = getLayerPoints(OcularLayerType::EPI);
			j["layers"]["EPI"] = layer;
		}
		if (isLayerPoints(OcularLayerType::BOW) || true) {
			layer = getLayerPoints(OcularLayerType::BOW);
			j["layers"]["BOW"] = layer;
		}
		if (isLayerPoints(OcularLayerType::END) || true) {
			layer = getLayerPoints(OcularLayerType::END);
			j["layers"]["END"] = layer;
		}

		std::vector<float> curvs;
		curvs = getCurvatureRadius();
		j["curvature"] = curvs;
	}

	try {
		std::ofstream f(path);
		f << j.dump();
		f.close();
		return true;
	}
	catch (...) {
		return false;
	}
}


bool OctData::BscanData::importBsegmResult(std::wstring path, const OctScanPattern desc)
{
	if (isEmpty()) {
		return false;
	}

	using json = nlohmann::json;

	json j;

	try {
		std::ifstream f(path);
		f >> j;
		f.close();

		std::unique_ptr<OcularBsegm> bsegm;
		std::vector<int> points;
		if (desc.isCorneaScan()) {
			bsegm = make_unique<CorneaBsegm>();
		}
		else {
			bsegm = (desc.isDiscScan() ? make_unique<DiscBsegm>() : make_unique<MacularBsegm>());
		}

		bsegm->loadSource(getImageBuffer(), getImageWidth(), getImageHeight());
		bsegm->setPatternDescript(desc);

		if (desc.isCorneaScan()) {
			auto layers = bsegm->getCorneaLayers();
			points = j.at("layers").at("EPI").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::EPI, points);
			points = j.at("layers").at("BOW").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::BOW, points);
			points = j.at("layers").at("END").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::END, points);

			std::vector<float> curvs;
			curvs = j.at("curvature").get<std::vector<float>>();
			bsegm->setCurvatureRadius(curvs);

			setOcularBsegm(std::move(bsegm));
		}
		else {
			auto layers = bsegm->getRetinaLayers();
			points = j.at("layers").at("ILM").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::ILM, points);
			points = j.at("layers").at("NFL").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::NFL, points);
			points = j.at("layers").at("IPL").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::IPL, points);
			points = j.at("layers").at("OPL").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::OPL, points);
			points = j.at("layers").at("IOS").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::IOS, points);
			points = j.at("layers").at("RPE").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::RPE, points);
			points = j.at("layers").at("BRM").get<std::vector<int>>();
			layers->setLayerPoints(OcularLayerType::BRM, points);

			try {
				points = j.at("layers").at("OPR").get<std::vector<int>>();
			}
			catch (...) {
				points = std::vector<int>(desc.getNumberOfScanPoints(), -1);
			}
			layers->setLayerPoints(OcularLayerType::OPR, points);
			/*
			layers->getILM()->setRegionSize(256, 384);
			layers->getNFL()->setRegionSize(256, 384);
			layers->getIPL()->setRegionSize(256, 384);
			layers->getOPL()->setRegionSize(256, 384);
			layers->getIOS()->setRegionSize(256, 384);
			layers->getRPE()->setRegionSize(256, 384);
			layers->getBRM()->setRegionSize(256, 384);
			layers->getOPR()->setRegionSize(256, 384);

			layers->getILM()->resize(250, 300);
			layers->getNFL()->resize(250, 300);
			layers->getIPL()->resize(250, 300);
			layers->getOPL()->resize(250, 300);
			layers->getIOS()->resize(250, 300);
			layers->getRPE()->resize(250, 300);
			layers->getBRM()->resize(250, 300);
			layers->getOPR()->resize(250, 300);
			*/
			setOcularBsegm(std::move(bsegm));
			
			if (desc.isDiscScan()) {
				int discX1, discX2, discPixels;
				int cupX1, cupX2, cupPixels;

				discX1 = j["opticDisc"]["x1"];
				discX2 = j["opticDisc"]["x2"];
				discPixels = j["opticDisc"]["pixels"];

				if (!j["opticDisc"]["state"]) {
					discX1 = discX2 = -1;
					discPixels = 0;
				}

				cupX1 = j["opticCup"]["x1"];
				cupX2 = j["opticCup"]["x2"];
				cupPixels = j["opticCup"]["pixels"];

				if (!j["opticCup"]["state"]) {
					cupX1 = cupX2 = -1;
					cupPixels = 0;
				}

				setOpticNerveDiscRange(discX1, discX2);
				setOpticNerveDiscPixels(discPixels);
				setOpticNerveCupRange(cupX1, cupX2);
				setOpticNerveCupPixels(cupPixels);
			}
		}
	}
	catch (...) {
		return false;
	}

	return true;
}


BscanData::BscanDataImpl & OctData::BscanData::getImpl(void) const
{
	return *d_ptr;
}


