#include "stdafx.h"
#include "FundusResult.h"
#include "FundusOutput.h"

#include "RetinaImage.h"
#include "CorneaImage.h"

#include "CppUtil2.h"

#include <map>

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusResult::FundusResultImpl
{
	OctFundusMeasure descript;

	FundusOutput output;

	RetinaImage retinaImage;
	CorneaImage corneaImage;

	FundusResultImpl() 
	{
	}
};


FundusResult::FundusResult() :
	d_ptr(make_unique<FundusResultImpl>())
{
}

OctResult::FundusResult::FundusResult(const OctFundusMeasure & desc) :
	d_ptr(make_unique<FundusResultImpl>())
{
	setDescript(desc);
}


OctResult::FundusResult::~FundusResult() = default;
OctResult::FundusResult::FundusResult(FundusResult && rhs) = default;
FundusResult & OctResult::FundusResult::operator=(FundusResult && rhs) = default;

/*
OctResult::FundusResult::FundusResult(const FundusResult & rhs)
	: d_ptr(make_unique<FundusResultImpl>(*rhs.d_ptr))
{
}


FundusResult & OctResult::FundusResult::operator=(const FundusResult & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


void OctResult::FundusResult::setDescript(const OctFundusMeasure & desc)
{
	d_ptr->descript = desc;
	return;
}


OctFundusMeasure& OctResult::FundusResult::getDescript(void) const
{
	return d_ptr->descript;
}


EyeSide OctResult::FundusResult::getEyeSide(void) const
{
	return getDescript().eyeSide();
}


bool OctResult::FundusResult::isOD(void) const
{
	return getDescript().isOD();
}


FundusOutput & OctResult::FundusResult::getFundusOutput(void) const
{
	return d_ptr->output;
}


RetinaImage & OctResult::FundusResult::getRetinaImage(void) const
{
	return d_ptr->retinaImage;
}


CorneaImage & OctResult::FundusResult::getCorneaImage(void) const
{
	return d_ptr->corneaImage;
}


bool OctResult::FundusResult::setRetinaImage(const OctRetinaImage & desc)
{
	d_ptr->retinaImage.setData(desc);
	return true;
}


bool OctResult::FundusResult::setCorneaImage(const OctCorneaImage & desc)
{
	d_ptr->corneaImage.setData(desc);
	return true;
}


const OctRetinaImage * OctResult::FundusResult::getRetinaImageDescript(void) const
{
	return &d_ptr->retinaImage.getDescript();
}


const OctCorneaImage * OctResult::FundusResult::getCorneaImageDescript(void) const
{
	return &d_ptr->corneaImage.getDescript();
}


bool OctResult::FundusResult::exportFiles(unsigned int imgQuality, std::wstring& dirPath,
	bool subPath, bool removeReflectionLight, const std::wstring& imageName, const std::wstring& thumbName,
	const std::wstring& retinaName, const std::wstring& corneaName, const std::wstring& frameExt)
{
	if (subPath) {
		if (!createExportDirectory(dirPath)) {
			LogE() << "Fundus result export failed, path: " << wtoa(dirPath);
			return false;
		}
	}

	if (!imageName.empty() && !getFundusOutput().exportImages(imgQuality, dirPath, removeReflectionLight, imageName, thumbName, frameExt)) {
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


bool OctResult::FundusResult::importFiles(std::wstring & dirPath, const std::wstring & imageName,
	const std::wstring & retinaName, const std::wstring & corneaName, const std::wstring & frameExt)
{
	if (!imageName.empty() && !getFundusOutput().importImages(dirPath, imageName, frameExt)) {
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

bool OctResult::FundusResult::exportAdjustParams(CString szVersion, float br, float ct, float ub, float vr, float cb, float gc,
	std::wstring& dirPath, const std::wstring& imageName)
{
	if (dirPath.empty() || imageName.empty()) {
		return false;
	}

	std::map<std::wstring, float> values;
	values.insert(std::make_pair(L"br", br));
	values.insert(std::make_pair(L"ct", ct));
	values.insert(std::make_pair(L"ub", ub));
	values.insert(std::make_pair(L"vr", vr));

	// hwajunlee
	values.insert(std::make_pair(L"cb", cb));
	values.insert(std::make_pair(L"gc", gc));

	//
	std::wstring strVersion = CT2CW(szVersion);

	return getFundusOutput().exportAdjustParams(strVersion, values, dirPath, imageName);
}

bool OctResult::FundusResult::importAdjustParams(float& out_br, float& out_ct, float& out_ub, float& out_vr, float& out_cb, float& out_gc, std::wstring& dirPath, const std::wstring& imageName)
{
	LogI() << "[OctResult / FundusResult.cpp] OctResult::FundusResult::importAdjustParams";

	if (dirPath.empty() || imageName.empty()) {
		return false;
	}

	std::map<std::wstring, float> values;
	if (!getFundusOutput().importAdjustParams(values, dirPath, imageName)) {
		return false;
	}

	out_br = values[L"br"];
	out_ct = values[L"ct"];
	out_ub = values[L"ub"];
	out_vr = values[L"vr"];
	out_cb = values[L"cb"]; // Central Brightness	
	out_gc = values[L"gc"]; // gamma correction

	return true;
}

bool OctResult::FundusResult::createExportDirectory(std::wstring& dirName)
{
	wstring dest = dirName;
	if (dest.empty()) {
		dest = L".//";
	}
	else {
		dest += L"//";
	}

	CTime time = CTime::GetCurrentTime();
	CString name = time.Format(_T("Fundus_%y%m%d_%H%M%S"));
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


FundusResult::FundusResultImpl & OctResult::FundusResult::getImpl(void) const
{
	return *d_ptr;
}
