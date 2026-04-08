#include "stdafx.h"
#include "ProtocolData.h"
#include "BscanData.h"

#include <boost/format.hpp>

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct ProtocolData::ProtocolDataImpl
{
	std::shared_ptr<ScannerResult> result;
	std::vector<std::shared_ptr<BscanData>> bscans;
	std::vector<std::shared_ptr<BscanData>> previews;

	ProtocolDataImpl() : result(new ScannerResult())
	{
	}
};


ProtocolData::ProtocolData() :
	d_ptr(make_unique<ProtocolDataImpl>())
{
}


OctData::ProtocolData::ProtocolData(const OctScanPattern & desc)
{
	setDescript(desc);
}


OctData::ProtocolData::ProtocolData(std::unique_ptr<OctResult::ScannerResult>&& result)
{
	importScannerResult(std::move(result));
}


OctData::ProtocolData::ProtocolData(std::shared_ptr<OctResult::ScannerResult> result)
{
	importScannerResult(result);
}


OctData::ProtocolData::~ProtocolData() = default;
OctData::ProtocolData::ProtocolData(ProtocolData && rhs) = default;
ProtocolData & OctData::ProtocolData::operator=(ProtocolData && rhs) = default;


void OctData::ProtocolData::importScannerResult(std::shared_ptr<OctResult::ScannerResult> result)
{
	if (result != nullptr) {
		// d_ptr->result = std::move(result);
		d_ptr->result = result;
		updateBscanDataList(true);
	}
	return;
}


float OctData::ProtocolData::getPixelWidth(void) const
{
	auto& desc = getDescript();
	int points = desc.getNumberOfScanPoints();
	float range = (desc.isHorizontal() ? desc.getScanRangeX() : desc.getScanRangeY());
	
	if (points <= 0) {
		return 0.0f;
	}
	else {
		return (range / points);
	}
}


float OctData::ProtocolData::getPixelHeight(void) const
{
	if (getDescript().isCorneaScan()) {
		return (float)GlobalSettings::getCorneaScanAxialResolution();
		// return (float)GlobalSettings::getInAirScanAxialResolution();
	}
	else {
		return (float)GlobalSettings::getRetinaScanAxialResolution();
	}
}


float OctData::ProtocolData::getLinesDistance(void) const
{
	auto& desc = getDescript();
	int lines = desc.getNumberOfScanLines();
	float range = (desc.isHorizontal() ? desc.getScanRangeY() : desc.getScanRangeX());

	if (lines <= 0) {
		return 0.0f;
	}
	else {
		return (range / lines);
	}
}


int OctData::ProtocolData::numberOfBscanPoints(void) const
{
	auto& desc = getDescript();
	int points = desc.getNumberOfScanPoints();
	return points;
}


BscanData * OctData::ProtocolData::getBscanData(int index) const
{
	if (index >= 0 && index < countBscanData()) {
		return d_ptr->bscans[index].get();
	}
	return nullptr;
}


BscanData * OctData::ProtocolData::getBscanDataOfSection(int sectIdx, int imageIdx) const
{
	// int index = sectIdx * getDescript().getScanOverlaps() + imageIdx;
	int index = sectIdx;
	return getBscanData(index);
}


int OctData::ProtocolData::countBscanData() const
{
	int size = (int) d_ptr->bscans.size();
	return size;
}


int OctData::ProtocolData::countBscanSections(void) const
{
	int size = countBscanData();
	int overlaps = getDescript().getScanOverlaps();
	int lines = getDescript().getNumberOfScanLines();

	if (size == overlaps * lines) {
		return lines;
	}
	else {
		return size;
	}
}


std::vector<SegmScan::OcularBsegm*> OctData::ProtocolData::getBscanSegmList(void) const
{
	auto list = std::vector<SegmScan::OcularBsegm*>();

	int sections = countBscanData();
	for (int i = 0; i < sections; i++) {
		auto data = getBscanData(i);
		if (data) {
			list.push_back(data->getOcularBsegm());
		}
	}
	return list;
}


BscanData * OctData::ProtocolData::getPreviewData(int index) const
{
	if (index >= 0 && index < countPreviewData()) {
		return d_ptr->previews[index].get();
	}
	return nullptr;
}


int OctData::ProtocolData::countPreviewData(void) const
{
	return (int)d_ptr->previews.size();
}


int OctData::ProtocolData::updatePreviewDataList(bool vflip)
{
	auto list = getPreview()->getSectionIndexList();
	int count = 0;

	d_ptr->previews.clear();
	for (auto index : list) {
		auto image = getPreview()->getSectionImage(index);
		if (image) {
			if (vflip) {
				image->flipVert();
			}
			auto data = make_shared<BscanData>(image);
			data->setSectionIndex(index);
			d_ptr->previews.push_back(data);
			count++;
		}
	}
	return count;
}


OctResult::ScannerResult * OctData::ProtocolData::getResult(void) const
{
	return d_ptr->result.get();
}


OctResult::PreviewOutput * OctData::ProtocolData::getPreview(void) const
{
	return &getResult()->getPreviewOutput();;
}


OctResult::PatternOutput2 * OctData::ProtocolData::getPattern(void) const
{
	return &getResult()->getPatternOutput();
}


OctScanPattern & OctData::ProtocolData::getDescript(void) const
{
	return getPattern()->getDescript();
}


void OctData::ProtocolData::setDescript(const OctScanPattern & desc)
{
	getPattern()->setDescript(desc);
	return;
}


const OctScanImage * OctData::ProtocolData::getPreviewImageDescript(int sectIdx, int imageIdx)
{
	if (getResult()) {
		return getResult()->getPreviewOutput().getSectionImageDescript(sectIdx, imageIdx);
	}
	return nullptr;
}


const OctScanImage * OctData::ProtocolData::getScanImageDescript(int sectIdx, int imageIdx)
{
	if (getResult()) {
		return getResult()->getPatternOutput().getSectionImageDescript(sectIdx, imageIdx);
	}
	return nullptr;
}


const OctRetinaImage * OctData::ProtocolData::getRetinaImageDescript(void) const
{
	if (getResult()) {
		return getResult()->getRetinaImageDescript();
	}
	return nullptr;
}


const OctCorneaImage * OctData::ProtocolData::getCorneaImageDescript(void) const
{
	if (getResult()) {
		return getResult()->getCorneaImageDescript();
	}
	return nullptr;
}


int OctData::ProtocolData::importPatternImages(const std::wstring & dirPath, int numImages)
{
	int count = getPattern()->importImages(numImages, dirPath);
	updateBscanDataList();
	return count;
}


int OctData::ProtocolData::importPatternImages(const std::vector<std::wstring>& fileList)
{
	int count = getPattern()->importImages(fileList);
	updateBscanDataList();
	return count;
}


int OctData::ProtocolData::importPreviewImages(const std::wstring & dirPath, int numImages)
{
	int count = getPreview()->importImages(numImages, dirPath);
	updatePreviewDataList();
	return count;
}


int OctData::ProtocolData::updatePatternImages(void)
{
	int count = getPattern()->updateImages();
	return count;
}


int OctData::ProtocolData::updatePreviewImages(void)
{
	int count = getPreview()->updateImages();
	return count;
}


bool OctData::ProtocolData::exportBsegmResults(const std::wstring dirPath)
{
	wstring path, name;

	int lines = getDescript().getNumberOfScanLines();
	int overlaps = getDescript().getScanOverlaps();
	bool isAngio = getDescript().isAngioScan();
	int count = (int)countBscanData();

	for (int i = 0; i < count; i++) {
		if (false) { // isAngio || (count == lines * overlaps)) {
			int major = i / overlaps;
			int minor = i % overlaps;

			auto bscan = getBscanDataOfSection(major, minor);
			if (bscan) {
				name = (boost::wformat(L"%03d_%02d") % major % minor).str();
				path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
				if (!bscan->exportBsegmResult(path, getDescript())) {
					return false;
				}
			}
		}
		else {
			auto bscan = getBscanData(i);
			if (bscan) {
				int index = bscan->getSectionIndex();
				name = (boost::wformat(L"%03d") % index).str();
				path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
				if (!bscan->exportBsegmResult(path, getDescript())) {
					return false;
				}
			}
		}


		/*
		int major = i / overlaps;
		int minor = i % overlaps;

		auto bscan = getBscanData(major, minor);
		if (bscan) {
			*//*
			name = bscan->getImageName();
			if (name.empty()) {
				if (overlaps > 1) {
					name = (boost::wformat(L"%03d_%02d") % (i / overlaps) % (i % overlaps)).str();
				}
				else {
					name = (boost::wformat(L"%03d") % i).str();
				}
			}
			else {
				name = name.substr(0, std::max<int>((int)(name.size() - 4), 0));
			}
			*/
			/*
			if (angio) {
				if (minor > 0) {
					name = (boost::wformat(L"%03d_%02d") % major % minor).str();
				}
				else {
					name = (boost::wformat(L"%03d") % major).str();
				}
			}
			else {
				name = (boost::wformat(L"%03d") % major).str();
			}

			path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
			if (!bscan->exportBsegmResult(path, getDescript())) {
				return false;
			}
		}
		*/
	}

	for (int i = 0; i < countPreviewData(); i++) {
		auto bscan = getPreviewData(i);
		if (bscan) {
			name = L"preview";
			if (i > 0) {
				name += (boost::wformat(L"_%1d") % i).str();
			}

			path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
			if (!bscan->exportBsegmResult(path, getDescript())) {
				return false;
			}
		}
	}
	return true;
}


bool OctData::ProtocolData::importBsegmResults(const std::wstring dirPath)
{
	wstring path, name;

	int lines = getDescript().getNumberOfScanLines();
	int overlaps = getDescript().getScanOverlaps();
	bool isAngio = getDescript().isAngioScan();
	int count = (int)countBscanData();

	for (int i = 0; i < count; i++) {
		if (false) { // isAngio || (count == lines * overlaps)) {
			int major = i / overlaps;
			int minor = i % overlaps;

			auto bscan = getBscanDataOfSection(major, minor);
			if (bscan) {
				name = (boost::wformat(L"%03d_%02d") % major % minor).str();
				path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
				if (!bscan->importBsegmResult(path, getDescript())) {
					return false;
				}
			}
		}
		else {
			auto bscan = getBscanData(i);
			if (bscan) {
				name = (boost::wformat(L"%03d") % i).str();
				path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
				if (!bscan->importBsegmResult(path, getDescript())) {
					return false;
				}
			}
		}

		/*
		int major = i / overlaps;
		int minor = i % overlaps;

		auto bscan = getBscanData(major, minor);
		if (bscan) {
			if (angio) {
				if (minor > 0) {
					name = (boost::wformat(L"%03d_%02d") % major % minor).str();
				}
				else {
					name = (boost::wformat(L"%03d") % major).str();
				}
			}
			else {
				name = (boost::wformat(L"%03d") % major).str();
			}

			path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
			if (!bscan->importBsegmResult(path, getDescript())) {
				return false;
			}
		}
		*/
	}

	for (int i = 0; i < countPreviewData(); i++) {
		auto bscan = getPreviewData(i);
		if (bscan) {
			name = L"preview";
			if (i > 0) {
				name += (boost::wformat(L"_%1d") % i).str();
			}

			path = (boost::wformat(L"%s//%s.json") % dirPath % name).str();
			if (!bscan->importBsegmResult(path, getDescript())) {
				return false;
			}
		}
	}
	return true;
}


void OctData::ProtocolData::clear(void)
{
	d_ptr->result.reset(new ScannerResult());
	d_ptr->bscans.clear();
	return;
}


int OctData::ProtocolData::updateBscanDataList(bool vflip)
{
	auto list = getPattern()->getSectionIndexList();
	int count = 0;

	d_ptr->bscans.clear();
	for (auto index : list) {
		for (int i = 0; i < getPattern()->getSectionImageCount(index); i++) {
			auto image = getPattern()->getSectionImage(index, i);
			if (image) {
				if (vflip) {
					image->flipVert();
				}
				auto data = make_shared<BscanData>(image);
				data->setSectionIndex(index);
				data->setOverlapIndex(i);
				d_ptr->bscans.push_back(data);
				count++;
			}
		}
	}

	updatePreviewDataList(vflip);
	return count;
}



ProtocolData::ProtocolDataImpl & OctData::ProtocolData::getImpl(void) const
{
	return *d_ptr;
}
