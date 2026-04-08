#include "stdafx.h"
#include "FundusSection.h"
#include "FundusImage.h"
#include "FundusFrame.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

#include <cpprest/json.h>

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusSection::FundusSectionImpl
{
	vector<unique_ptr<FundusImage>> images;
	vector<unique_ptr<FundusFrame>> frames;
	OctFundusSection descript;

	FundusSectionImpl()
	{
	}
};


FundusSection::FundusSection() :
	d_ptr(make_unique<FundusSectionImpl>())
{
}


OctResult::FundusSection::FundusSection(const OctFundusSection& desc) :
	d_ptr(make_unique<FundusSectionImpl>())
{
	d_ptr->descript = desc;
}


OctResult::FundusSection::~FundusSection() = default;
OctResult::FundusSection::FundusSection(FundusSection && rhs) = default;
FundusSection & OctResult::FundusSection::operator=(FundusSection && rhs) = default;



bool OctResult::FundusSection::setImage(const OctFundusImage & image)
{
	if (getImageCount() > 0) {
		clearAllImages();
	}
	return addImage(image);
}


bool OctResult::FundusSection::setFrame(const OctFundusFrame& frame)
{
	if (getFrameCount() > 0) {
		clearAllFrames();
	}
	return addFrame(frame);
}


bool OctResult::FundusSection::addImage(const OctFundusImage & image)
{
	if (!image.getBuffer()) {
		return false;
	}
	d_ptr->images.push_back(make_unique<FundusImage>(image));
	return true;
}


bool OctResult::FundusSection::addFrame(const OctFundusFrame& frame)
{
	if (!frame.getBuffer()) {
		return false;
	}
	d_ptr->frames.push_back(make_unique<FundusFrame>(frame));
	return true;
}


FundusImage * OctResult::FundusSection::getImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}

	FundusImage* p = d_ptr->images[index].get();
	return p;
}


FundusFrame * OctResult::FundusSection::getFrame(int index)
{
	if (index < 0 || index >= getFrameCount()) {
		return nullptr;
	}

	FundusFrame* p = d_ptr->frames[index].get();
	return p;
}


int OctResult::FundusSection::getImageCount(void) const
{
	return (int)d_ptr->images.size();
}


int OctResult::FundusSection::getFrameCount(void) const
{
	return (int)d_ptr->frames.size();
}


void OctResult::FundusSection::clearAllImages(void)
{
	d_ptr->images.clear();
	return;
}


void OctResult::FundusSection::clearAllFrames(void)
{
	d_ptr->frames.clear();
	return;
}


void OctResult::FundusSection::setDescript(const OctFundusSection & section)
{
	d_ptr->descript = section;
	return;
}


const OctFundusSection & OctResult::FundusSection::getDescript(void) const
{
	return d_ptr->descript;
}


bool OctResult::FundusSection::exportFiles(unsigned int imgQuality, const std::wstring& dirPath, bool removeReflectionLight,
	const std::wstring& imageName, const std::wstring& thumbName, const std::wstring& frameExt)
{
	if (!imageName.empty()) {
		wstring path1 = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % kImageExt).str();

		if (!exportImage(imgQuality, path1)) {
			// return false;
		}

		/*
		wstring path2 = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % frameExt).str();

		if (!exportFrame(path2)) {
			// return false;
		}
		*/
	}

	if (!thumbName.empty()) {
		wstring path3 = (boost::wformat(L"%s//%s.%s") % dirPath % thumbName % kImageExt).str();

		if (!exportThumbnail(path3, removeReflectionLight)) {
			// return false;
		}
	}

	return true;
}


bool OctResult::FundusSection::importFiles(const std::wstring & dirPath, const std::wstring & imageName,
			const std::wstring & frameExt)
{
	bool ret;

	ret = importFileWithExt(dirPath, imageName, frameExt, kImageExt);
	if (!ret) {
		ret = importFileWithExt(dirPath, imageName, frameExt, kImageExtOld);
	}

	return ret;
}

bool OctResult::FundusSection::importFileWithExt(const std::wstring& dirPath,
	const std::wstring& imageName, const std::wstring& frameExt, const std::wstring& imageExt)
{
	if (!imageName.empty()) {
		wstring path1 = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % imageExt).str();

		if (!importImage(path1)) {
			return false;
		}

		/*
		wstring path2 = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % frameExt).str();

		if (!importFrame(path2)) {
		// return false;
		}
		*/
	}
	return true;
}

bool OctResult::FundusSection::exportAdjustParams(std::wstring strVersion,
	std::map<std::wstring, float> values, const std::wstring& dirPath,
	const std::wstring& imageName)
{
	if (dirPath.empty() || imageName.empty()) {
		return false;
	}

	wstring path = (boost::wformat(L"%s\\%s.json") % dirPath % imageName).str();

	// compose json format
	web::json::value jsonValue;
	jsonValue[L"br"] = web::json::value::number(values[L"br"]);
	jsonValue[L"ct"] = web::json::value::number(values[L"ct"]);
	jsonValue[L"ub"] = web::json::value::number(values[L"ub"]);
	jsonValue[L"vr"] = web::json::value::number(values[L"vr"]);

	// hwajunlee
	jsonValue[L"cb"] = web::json::value::number(values[L"cb"]);
	jsonValue[L"gc"] = web::json::value::number(values[L"gc"]);
	jsonValue[L"version"] = web::json::value::string(strVersion);

	// convert to stream
	utility::stringstream_t stream;
	jsonValue.serialize(stream);

	// write to file
	wofstream file(path);
	file << stream.str();
	file.close();

	return true;
}

bool OctResult::FundusSection::importAdjustParams(std::map<std::wstring, float>& out_values,
	const std::wstring& dirPath, const std::wstring& imageName)
{
	if (dirPath.empty() || imageName.empty()) {
		return false;
	}

	wstring path = (boost::wformat(L"%s\\%s.json") % dirPath % imageName).str();

	// read from file
	// - open
	std::wifstream wif(path);
	if (!wif.is_open()) {
		return false;
	}
	// - read contents
	std::wstring contents;
	wif >> contents;
	wif.close();
	// - parse contents
	std::error_code err;
	auto jsonValue = web::json::value::parse(contents, err);
	if (err.value() != 0) {
		return false;
	}
	// - get data
	float br = jsonValue[L"br"].as_double();
	float ct = jsonValue[L"ct"].as_double();
	float ub = jsonValue[L"ub"].as_double();
	float vr = jsonValue[L"vr"].as_double();

	float cb = 1.0f;
	float gc = 1.0f;

	if (jsonValue.is_object())
	{
		if (jsonValue.has_field(L"cb"))
		{
			cb = jsonValue[L"cb"].as_double();
		}
		if (jsonValue.has_field(L"gc"))
		{
			gc = jsonValue[L"gc"].as_double();
		}
	}

	auto fieldVersion = jsonValue[L"version"];
	if (fieldVersion.is_null()) {
		// invalid values.. delete file
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::string fileNameA = converter.to_bytes(path);
		std::remove(fileNameA.c_str());
		return false;
	}
	// - pack data
	out_values.insert(std::make_pair(L"br", br));
	out_values.insert(std::make_pair(L"ct", ct));
	out_values.insert(std::make_pair(L"ub", ub));
	out_values.insert(std::make_pair(L"vr", vr));

	// hwajunlee
	out_values.insert(std::make_pair(L"cb", cb));
	out_values.insert(std::make_pair(L"gc", gc));

	return true;
}

/*
bool OctResult::FundusSection::exportFiles(std::wstring dirPath, std::wstring imageName, std::wstring thumbName, 
											std::wstring imageExt, std::wstring frameExt)
{
	int size = getImageCount();

	wstring path = dirPath;
	if (path.empty()) {
		path = L".";
	}

	wstring name1, name2, name3;
	for (int i = 0; i < size; i++) {
		FundusImage* image = getImage(i);
		if (image != nullptr) {
			if (size == 1) {
				name1 = (boost::wformat(L"%s//%s.%s") % path % imageName % imageExt).str();
				name2 = (boost::wformat(L"%s//%s.%s") % path % imageName % frameExt).str();
				name3 = (boost::wformat(L"%s//%s.%s") % path % thumbName % imageExt).str();
			}
			else {
				name1 = (boost::wformat(L"%s//%s_%02d.%s") % path % imageName % i % imageExt).str();
				name2 = (boost::wformat(L"%s//%s_%02d.%s") % path % imageName % i % frameExt).str();
				name3 = (boost::wformat(L"%s//%s_%02d.%s") % path % thumbName % i % imageExt).str();
			}

			if (!image->exportImage(name1)) {
				return false;
			}

			if (!image->exportThumbnail(name3)) {
				return false;
			}

			FundusFrame* frame = getFrame(i);
			if (frame != nullptr) {
				if (!frame->exportImage(name2)) {
					return false;
				}
			}
		}
	}
	return true;
}
*/

bool OctResult::FundusSection::exportImage(unsigned int imgQuality, const std::wstring & path,
	int imageIdx)
{
	FundusImage* image = getImage(imageIdx);
	bool result = false;
	if (image != nullptr) {
		result = image->exportFile(imgQuality, path);
	}
	LogD() << "Export fundus image, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::FundusSection::exportThumbnail(const std::wstring & path, bool removeReflectionLight, int imageIdx)
{
	FundusImage* image = getImage(imageIdx);
	bool result = false;
	if (image != nullptr) {
		result = image->exportThumbnail(path, removeReflectionLight);
	}
	LogD() << "Export fundus thumbnail, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::FundusSection::exportFrame(const std::wstring & path, int frameIdx)
{
	FundusFrame* image = getFrame(frameIdx);
	bool result = false;
	if (image != nullptr) {
		result = image->exportFile(path);
	}
	LogD() << "Export fundus frame, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::FundusSection::importImage(const std::wstring & path, bool append)
{
	if (!append) {
		clearAllImages();
	}

	auto image = make_unique<FundusImage>();
	bool result = false;
	if (image->importFile(path)) {
		d_ptr->images.push_back(std::move(image));
		result = true;
	}
	LogD() << "Import fundus image, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::FundusSection::importFrame(const std::wstring & path, bool append)
{
	if (!append) {
		clearAllFrames();
	}

	auto image = make_unique<FundusFrame>();
	bool result = false;
	if (image->importFile(path)) {
		d_ptr->frames.push_back(std::move(image));
		result = true;
	}
	LogD() << "Import fundus frame, path: " << wtoa(path) << " => " << result;
	return result;
}


FundusSection::FundusSectionImpl & OctResult::FundusSection::getImpl(void) const
{
	return *d_ptr;
}
