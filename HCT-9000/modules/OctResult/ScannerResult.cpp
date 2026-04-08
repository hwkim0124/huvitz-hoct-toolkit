#include "stdafx.h"
#include "ScannerResult.h"
#include "PatternOutput2.h"
#include "PreviewOutput.h"
#include "EnfaceOutput.h"

#include "RetinaImage.h"
#include "CorneaImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct ScannerResult::ScannerResultImpl
{
	OctScanMeasure descript;

	PreviewOutput preview;
	PatternOutput2 pattern;
	EnfaceOutput enface;

	RetinaImage retinaImage;
	CorneaImage corneaImage;

	ScannerResultImpl() 
	{
	}
};


ScannerResult::ScannerResult() :
	d_ptr(make_unique<ScannerResultImpl>())
{
}


OctResult::ScannerResult::ScannerResult(const OctScanMeasure& desc) :
	d_ptr(make_unique<ScannerResultImpl>())
{
	setDescript(desc);
}


OctResult::ScannerResult::~ScannerResult() = default;
OctResult::ScannerResult::ScannerResult(ScannerResult && rhs) = default;
ScannerResult & OctResult::ScannerResult::operator=(ScannerResult && rhs) = default;

/*
OctResult::ScannerResult::ScannerResult(const ScannerResult & rhs)
	: d_ptr(make_unique<ScannerResultImpl>(*rhs.d_ptr))
{
}


ScannerResult & OctResult::ScannerResult::operator=(const ScannerResult & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/

void OctResult::ScannerResult::setDescript(const OctScanMeasure& desc)
{
	d_ptr->descript = desc;

	getPatternOutput().setDescript(getDescript().getPattern());
	getPreviewOutput().setDescript(getDescript().getPattern());
	return;
}


OctScanMeasure & OctResult::ScannerResult::getDescript(void) const
{
	return d_ptr->descript;
}


EyeSide OctResult::ScannerResult::getEyeSide(void) const
{
	return getDescript().getEyeSide();
}


bool OctResult::ScannerResult::isOD(void) const
{
	return getDescript().isOD();
}


PreviewOutput & OctResult::ScannerResult::getPreviewOutput(void) const
{
	return d_ptr->preview;
}


PatternOutput2 & OctResult::ScannerResult::getPatternOutput(void) const
{
	return d_ptr->pattern;
}


EnfaceOutput & OctResult::ScannerResult::getEnfaceOutput(void) const
{
	return d_ptr->enface;
}


RetinaImage & OctResult::ScannerResult::getRetinaImage(void) const
{
	return d_ptr->retinaImage;
}


CorneaImage & OctResult::ScannerResult::getCorneaImage(void) const
{
	return d_ptr->corneaImage;
}


bool OctResult::ScannerResult::setRetinaImage(const OctRetinaImage & desc)
{
	d_ptr->retinaImage.setData(desc);
	return true;
}


bool OctResult::ScannerResult::setCorneaImage(const OctCorneaImage & desc)
{
	d_ptr->corneaImage.setData(desc);
	return true;
}


const OctRetinaImage* OctResult::ScannerResult::getRetinaImageDescript(void) const
{
	return &d_ptr->retinaImage.getDescript();
}


const OctCorneaImage* OctResult::ScannerResult::getCorneaImageDescript(void) const
{
	return &d_ptr->corneaImage.getDescript();
}


bool OctResult::ScannerResult::exportFiles(std::wstring& dirPath, bool subPath,
	const std::wstring& imagePrefix, const std::wstring& enfaceName,
	const std::wstring& previewName, const std::wstring& retinaName,
	const std::wstring& corneaName)
{
	if (subPath) {
		if (!createExportDirectory(dirPath)) {
			LogE() << "Scanner result export failed, path: " << wtoa(dirPath);
			return false;
		}
	}

	if (!getPatternOutput().exportImages(dirPath, imagePrefix)) {
		// return false;
	}

	if (!previewName.empty() && !getPreviewOutput().exportImages(dirPath, previewName)) {
		// return false;
	}

	if (!enfaceName.empty() && !getEnfaceOutput().exportImage(dirPath, enfaceName)) {
		// return false;
	}

	if (!retinaName.empty() && !getRetinaImage().exportFile(dirPath, retinaName)) {
		// return false;
	}

	if (!corneaName.empty() && !getCorneaImage().exportFile(dirPath, corneaName)) {
		// return false;
	}
	return true;
}


bool OctResult::ScannerResult::importFiles(std::wstring & dirPath, 
	int numImages,
	const std::wstring & imagePrefix, const std::wstring & enfaceName, 
	const std::wstring & previewName, const std::wstring & retinaName, 
	const std::wstring & corneaName)
{
	if (!getPatternOutput().importImages(numImages, dirPath, imagePrefix)) {
		// return false;
	}

	int numPreviews = (getDescript().getPattern().isPreviewCross() ? 2 : 1);
	if (!previewName.empty() && !getPreviewOutput().importImages(numPreviews, dirPath, previewName)) {
		// return false;
	}

	if (!enfaceName.empty() && !getEnfaceOutput().importImage(dirPath, enfaceName)) {
		// return false;
	}

	if (!retinaName.empty() && !getRetinaImage().importFile(dirPath, retinaName)) {
		// return false;
	}

	if (!corneaName.empty() && !getCorneaImage().importFile(dirPath, corneaName)) {
		// return false;
	}
	return true;
}


bool OctResult::ScannerResult::createExportDirectory(std::wstring& dirName)
{
	wstring dest = dirName;
	if (dest.empty()) {
		dest = L".//";
	}
	else {
		dest += L"//";
	}

	CTime time = CTime::GetCurrentTime();
	CString name = time.Format(_T("Scan_%y%m%d_%H%M%S"));
	dest += name;
	dest += L"_";

	string label = getDescript().getLabel();
	wstring wstr = L"";
	wstr.assign(label.begin(), label.end());
	dest += wstr;

	if (CreateDirectory(dest.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		dirName = dest;
		return true;
	}
	return false;
}


ScannerResult::ScannerResultImpl & OctResult::ScannerResult::getImpl(void) const
{
	return *d_ptr;
}
