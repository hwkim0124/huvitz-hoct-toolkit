#include "stdafx.h"
#include "BscanSection.h"
#include "BscanImage2.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct BscanSection::BscanSectionImpl
{
	vector<unique_ptr<BscanImage2>> images;
	OctScanSection descript;

	BscanSectionImpl()
	{
	}
};


BscanSection::BscanSection() :
	d_ptr(make_unique<BscanSectionImpl>())
{
}

/*
OctResult::BscanSection::BscanSection(OctRoute route) :
	d_ptr(make_unique<BscanSectionImpl>())
{
	d_ptr->descript._route = route;
}
*/


OctResult::BscanSection::BscanSection(const OctScanSection& desc) :
	d_ptr(make_unique<BscanSectionImpl>())
{
	d_ptr->descript = desc;
}


OctResult::BscanSection::~BscanSection() = default;
OctResult::BscanSection::BscanSection(BscanSection && rhs) = default;
BscanSection & OctResult::BscanSection::operator=(BscanSection && rhs) = default;


bool OctResult::BscanSection::setImage(const OctScanImage& desc)
{
	if (getImageCount() > 0) {
		clearAllImages();
	}
	return addImage(desc);
}


bool OctResult::BscanSection::addImage(const OctScanImage & desc)
{
	if (!desc.getData()) {
		return false;
	}

	d_ptr->images.push_back(make_unique<BscanImage2>(desc));
	return true;
}


bool OctResult::BscanSection::removeImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return false;
	}
	d_ptr->images.erase(d_ptr->images.begin() + index);
	return true;
}


BscanImage2 * OctResult::BscanSection::getImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}

	BscanImage2* p = d_ptr->images[index].get();
	return p;
}


int OctResult::BscanSection::getImageCount(void) const
{
	return (int)d_ptr->images.size();
}


void OctResult::BscanSection::clearAllImages(void)
{
	d_ptr->images.clear();
	return;
}


void OctResult::BscanSection::setDescript(const OctScanSection & desc)
{
	d_ptr->descript = desc;
	return;
}


const OctScanSection & OctResult::BscanSection::getDescript(void) const
{
	return d_ptr->descript;
}


void OctResult::BscanSection::setRouteOfScan(OctRoute route)
{
	d_ptr->descript._route = route;
	return;
}


const OctRoute& OctResult::BscanSection::getRouteOfScan(void) const
{
	return d_ptr->descript._route;
}


const OctPoint & OctResult::BscanSection::getStartPoint(void) const
{
	return d_ptr->descript._route.start();
}


const OctPoint & OctResult::BscanSection::getEndPoint(void) const
{
	return d_ptr->descript._route.close();
}


bool OctResult::BscanSection::isCircle(void)
{
	return d_ptr->descript._route.isCircle();
}


bool OctResult::BscanSection::updateImages(void)
{
	int size = getImageCount();

	for (int i = 0; i < size; i++) {
		if (!updateImage(i)) {
			return false;
		}
	}
	return true;
}


bool OctResult::BscanSection::exportImages(const std::wstring& dirPath, const std::wstring& prefix, bool single)
{
	int size = getImageCount();

	wstring path;
	wstring name;
	for (int i = 0; i < size; i++) {
		if (!prefix.empty()) {
			name = (boost::wformat(L"%s%03d") % prefix % getDescript().getIndex()).str();
		}
		else {
			name = (boost::wformat(L"%03d") % getDescript().getIndex()).str();
		}

		if (size == 1 || single) {
			path = (boost::wformat(L"%s//%s.%s") % dirPath % name % kBScanImageExt).str();
		}
		else {
			path = (boost::wformat(L"%s//%s_%02d.%s") % dirPath % name % i % kBScanImageExt).str();
		}

		if (!exportImage(path, i)) {
			return false;
		}
		if (single) {
			break;
		}
	}
	return true;
}


bool OctResult::BscanSection::importImages(const std::wstring & dirPath, const std::wstring & prefix, int size)
{
	wstring path;
	wstring name;
	bool ret;

	clearAllImages();
	for (int i = 0; i < size; i++) {
		// get name
		if (!prefix.empty()) {
			name = (boost::wformat(L"%s%03d") % prefix % getDescript().getIndex()).str();
		}
		else {
			name = (boost::wformat(L"%03d") % getDescript().getIndex()).str();
		}

		// import with kImageExt
		ret = importImageWithExt(dirPath, name, kBScanImageExt, size, i);
		if (!ret) {
			ret = importImageWithExt(dirPath, name, kImageExt, size, i);
			if (!ret) {
				ret = importImageWithExt(dirPath, name, kImageExtOld, size, i);
			}
		}
	}
	return true;
}


bool OctResult::BscanSection::updateImage(int imageIdx)
{
	BscanImage2* image = getImage(imageIdx);
	bool result = false;
	if (image != nullptr) {
		result = image->updateFile();
	}
	return result;
}


bool OctResult::BscanSection::exportImage(const std::wstring & path, int imageIdx)
{
	BscanImage2* image = getImage(imageIdx);
	bool result = false;
	if (image != nullptr) {
		result = image->exportFile(path);
		if (!result) {
			LogD() << "Failed exporting section image, path: " << wtoa(path) << " => " << result;
		}
	}
	// LogD() << "Export section image, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::BscanSection::importImage(const std::wstring& path, bool append)
{
	if (!append) {
		clearAllImages();
	}

	auto image = make_unique<BscanImage2>();
	bool result = false;
	if (image->importFile(path)) {
		d_ptr->images.push_back(std::move(image));
		result = true;
	}
	else {
		LogD() << "Failed importing section image, path: " << wtoa(path) << " => " << result;
	}
	// LogD() << "Import section image, path: " << wtoa(path) << " => " << result;
	return result;
}

bool OctResult::BscanSection::importImageWithExt(const std::wstring& dirPath,
	const std::wstring& name, const std::wstring& imageExt, int size, int idx)
{
	wstring path;

	if (size == 1) {
		path = (boost::wformat(L"%s//%s.%s") % dirPath % name % imageExt).str();
	}
	else {
		path = (boost::wformat(L"%s//%s_%02d.%s") % dirPath % name % idx % imageExt).str();
	}

	if (!importImage(path, true)) {
		return false;
	}
	return true;
}

bool OctResult::BscanSection::applyAveraging(bool forward)
{
	int size = (int) getImpl().images.size();
	if (size <= 1) {
		return true;
	}

	int baseIdx;
	BscanImage2* p; // = getImage(baseIdx);
	CvProcess cvProc;

	if (forward) {
		baseIdx = 0;
	}
	else {
		baseIdx = 0;
		float qualityMax = 0.0f;
		float qualityStd = 4.0f;
		for (int i = (size - 1); i >= 0; i--) {
			if ((p = getImage(i)) != nullptr) {
				if (p->getSignalRatio() > qualityMax) {
					qualityMax = p->getSignalRatio();
					baseIdx = i;
				}
				if (qualityMax >= qualityStd) {
					break;
				}
			}
		}
		if (baseIdx == 0) {
			baseIdx = size - 1;
		}
	}

	p = getImage(baseIdx);
	if (p == nullptr || !cvProc.startAveraging(p->getImage())) {
		return false;
	}

	if (forward) {
		for (int i = baseIdx + 1; i < size; i++) {
			if ((p = getImage(i)) != nullptr) {
				cvProc.insertAveraging(p->getImage());
				// DebugOut2() << "Averaging image, idx: " << i;
			}
		}
	}
	else {
		int count = 1;
		for (int i = baseIdx - 1; i >= 0; i--) {
			if (i != baseIdx && (p = getImage(i)) != nullptr) {
				if (cvProc.insertAveraging(p->getImage())) {
					// DebugOut2() << "Averaging image, idx: " << i;
					if (++count >= PATTERN_PREVIEW_AVERAGE_SIZE) {
						break;
					}
				}
			}
		}
	}

	CvImage output;
	bool result = false; 
	int count = cvProc.closeAveraging(output);
	if (count > 0) {
		p = getImage(0);
		OctScanImage desc = p->getDescript();
		desc._data = output.getBitsData();
		p->setData(desc);
		result = true;
		LogD() << "Averaging images, result: " << count << " / " << size;
	}

	getImpl().images.erase(getImpl().images.cbegin() + 1, getImpl().images.cend());
	return true;
}


BscanSection::BscanSectionImpl & OctResult::BscanSection::getImpl(void) const
{
	return *d_ptr;
}
