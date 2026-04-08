#include "stdafx.h"
#include "FundusOutput.h"
#include "FundusImage.h"
#include "FundusSection.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusOutput::FundusOutputImpl
{
	FundusOutputImpl()
	{
	}
};


FundusOutput::FundusOutput() :
	d_ptr(make_unique<FundusOutputImpl>())
{
}


OctResult::FundusOutput::~FundusOutput() = default;
OctResult::FundusOutput::FundusOutput(FundusOutput && rhs) = default;
FundusOutput & OctResult::FundusOutput::operator=(FundusOutput && rhs) = default;



/*
OctResult::FundusOutput::FundusOutput(const FundusOutput & rhs)
	: d_ptr(make_unique<FundusOutputImpl>(*rhs.d_ptr))
{
}


FundusOutput & OctResult::FundusOutput::operator=(const FundusOutput & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


int OctResult::FundusOutput::exportImages(unsigned int imgQuality, const std::wstring & dirPath, bool removeReflectionLight,
	const std::wstring & imageName,	const std::wstring & thumbName, const std::wstring & frameExt)
{
	if (!SystemFunc::isDirectory(dirPath)) {
		LogE() << "Export fundus directory not found!, path: " << wtoa(dirPath);
		return false;
	}

	auto list = getSectionIndexList();
	int count = 0;
	for (auto index : list) {
		if (exportImage(index, imgQuality, dirPath, removeReflectionLight, imageName, thumbName, frameExt)) {
			count++;
		}
		break;
	}
	LogD() << "All exported fundus images: " << count;
	return count;
}


int OctResult::FundusOutput::importImages(const std::wstring & dirPath, const std::wstring & imageName, 
			const std::wstring & frameExt)
{
	if (!SystemFunc::isDirectory(dirPath)) {
		LogE() << "Import fundus directory not found!, path: " << wtoa(dirPath);
		return false;
	}

	int count = 0;
	if (importImage(0, dirPath, imageName, frameExt)) {
		count++;
	}
	LogD() << "All imported fundus images: " << count;
	return count;
}


bool OctResult::FundusOutput::exportImage(int sectIdx, unsigned int imgQuality, const std::wstring & dirPath, 
	bool removeReflectionLight, const std::wstring & imageName, const std::wstring& thumbName,
	const std::wstring & frameExt)
{
	auto pSect = getSection(sectIdx);
	bool result = false;
	if (pSect) {
		result = pSect->exportFiles(imgQuality, dirPath, removeReflectionLight, imageName, thumbName, frameExt);
	}
	return result;
}


bool OctResult::FundusOutput::importImage(int sectIdx, const std::wstring dirPath, 
			const std::wstring & imageName, const std::wstring & frameExt)
{
	auto desc = OctFundusSection(sectIdx);
	auto pSect = makeSection(desc);
	bool result = false;
	if (pSect) {
		result = pSect->importFiles(dirPath, imageName, frameExt);
	}
	return result;
}

bool OctResult::FundusOutput::exportAdjustParams(std::wstring strVersion,
	std::map<std::wstring, float> values, const std::wstring& dirPath,
	const std::wstring& imageName)
{
	auto desc = OctFundusSection(0);
	auto pSect = makeSection(desc);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->exportAdjustParams(strVersion, values, dirPath, imageName);
}

bool OctResult::FundusOutput::importAdjustParams(std::map<std::wstring, float>& out_values,
	const std::wstring& dirPath, const std::wstring& imageName)
{
	auto desc = OctFundusSection(0);
	auto pSect = makeSection(desc);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->importAdjustParams(out_values, dirPath, imageName);
}

FundusOutput::FundusOutputImpl & OctResult::FundusOutput::getImpl(void) const
{
	return *d_ptr;
}
