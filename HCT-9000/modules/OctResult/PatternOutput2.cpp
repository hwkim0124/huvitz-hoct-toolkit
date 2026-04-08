#include "stdafx.h"
#include "PatternOutput2.h"
#include "BscanSection.h"
#include "BscanImage2.h"
#include "EnfaceImage.h"

#include <boost/format.hpp>
#include <thread>
#include <vector>
#include <algorithm>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PatternOutput2::PatternOutput2Impl
{
	OctScanPattern descript;
	unique_ptr<EnfaceImage> enface;

	PatternOutput2Impl() : enface(make_unique<EnfaceImage>())
	{
	}
};


PatternOutput2::PatternOutput2() :
	d_ptr(make_unique<PatternOutput2Impl>())
{
}


OctResult::PatternOutput2::~PatternOutput2() = default;
OctResult::PatternOutput2::PatternOutput2(PatternOutput2 && rhs) = default;
PatternOutput2 & OctResult::PatternOutput2::operator=(PatternOutput2 && rhs) = default;

/*
OctResult::PatternOutput2::PatternOutput2(const PatternOutput2 & rhs)
	: d_ptr(make_unique<PatternOutput2Impl>(*rhs.d_ptr))
{
}


PatternOutput2 & OctResult::PatternOutput2::operator=(const PatternOutput2 & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


int OctResult::PatternOutput2::updateImages(void)
{
	auto list = getSectionIndexList();
	int count = 0;

	for (auto index : list) {
		if (updateImage(index)) {
			count++;
		}
	}
	LogD() << "All updated pattern images: " << count;
	return count;
}


int OctResult::PatternOutput2::exportImages(const std::wstring dirPath, const std::wstring & prefix)
{
	if (!SystemFunc::isDirectory(dirPath)) {
		LogE() << "Export pattern directory not found!, path: " << wtoa(dirPath);
		return false;
	}

	auto list = getSectionIndexList();
	int count = 0;
	for (auto index : list) {
		if (exportImage(index, dirPath, prefix)) {
			count++;
		}
	}
	LogD() << "All exported pattern images: " << count;
	return count;
}


int OctResult::PatternOutput2::importImages(int numImages, const std::wstring dirPath, const std::wstring & prefix)
{
	if (!SystemFunc::isDirectory(dirPath)) {
		LogE() << "Import pattern directory not found!, path: " << wtoa(dirPath);
		return false;
	}

	clearAllSections();

	int count = 0;
	int total = (numImages <= 0 ? getDescript().getNumberOfScanLines() : numImages);
	for (int index = 0; index < total; index++) {
		if (importImage(index, dirPath, prefix)) {
			count++;
		}
	}
	LogD() << "All imported pattern images: " << count;
	return count;
}


int OctResult::PatternOutput2::importImages(const std::vector<std::wstring>& fileList)
{
	clearAllSections();

	int count = 0;
	for (int index = 0; index < fileList.size(); index++) {
		if (importImage(index, fileList[index])) {
			count++;
		}
	}
	LogD() << "All imported pattern images: " << count;
	return count;
}


bool OctResult::PatternOutput2::updateImage(int sectIdx)
{
	auto pSect = getSection(sectIdx);
	bool result = false;
	if (pSect) {
		result = pSect->updateImages();
	}
	return result;
}


bool OctResult::PatternOutput2::exportImage(int sectIdx, const std::wstring & dirPath, const std::wstring & prefix)
{
	auto pSect = getSection(sectIdx);
	bool result = false;
	if (pSect) {
		result = pSect->exportImages(dirPath, prefix, false);
	}
	return result;
}


bool OctResult::PatternOutput2::importImage(int sectIdx, const std::wstring& dirPath, const std::wstring & prefix)
{
	auto desc = getDescript().makeScanSection(sectIdx);
	auto pSect = makeSection(desc);
	bool result = false;
	if (pSect) {
		result = pSect->importImages(dirPath, prefix);
	}
	// LogD() << "Section: " << desc.text();
	return result;
}


bool OctResult::PatternOutput2::importImage(int sectIdx, const std::wstring & filePath)
{
	auto desc = getDescript().makeScanSection(sectIdx);
	auto pSect = makeSection(desc);
	bool result = false;
	if (pSect) {
		result = pSect->importImage(filePath);
	}
	// LogD() << "Section: " << desc.text();
	return result;
}


float OctResult::PatternOutput2::getAverageOfQualityIndex(void)
{
	float qsum = 0.0f;
	int count = 0;

	for (int i = 0; i < getSectionCount(); i++) {
		auto p = getSection(i)->getImage();
		if (p != nullptr) {
			qsum += p->getQualityIndex();
			count += 1;
		}
	}

	float mean = (count > 0 ? (qsum / count) : 0.0f);
	return mean;
}


bool OctResult::PatternOutput2::applyAveraging(void)
{
	auto n_workers = std::thread::hardware_concurrency();

	std::vector<std::vector<BscanSection*>> tasks(n_workers);
	auto list = getSectionIndexList();

	for (int i = 0; i < list.size(); i++) {
		int index = list[i];
		BscanSection* p = getSection(index);
		if (p != nullptr && p->getImageCount() > 1) {
			int k = i % n_workers;
			tasks[k].push_back(p);
		}
	}

	std::vector<std::thread> workers;

	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k]() {
				for (auto p : tasks[k]) {
					bool res = p->applyAveraging();
					if (res) {
						LogD() << "Image averaging success, index: " << p->getDescript().getIndex();
					}
					else {
						LogD() << "Image averaging failed, index: " << p->getDescript().getIndex();
					}
				}
			}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t)
	{
		t.join();
	});
	return true;
}


PatternOutput2::PatternOutput2Impl & OctResult::PatternOutput2::getImpl(void) const
{
	return *d_ptr;
}
