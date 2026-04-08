#include "stdafx.h"
#include "EnfaceOutput.h"
#include "EnfaceImage.h"

#include <boost/format.hpp>
#include <thread>
#include <vector>
#include <algorithm>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct EnfaceOutput::EnfaceOutputImpl
{
	unique_ptr<EnfaceImage> enface;

	EnfaceOutputImpl() : enface(make_unique<EnfaceImage>())
	{
	}
};


EnfaceOutput::EnfaceOutput() :
	d_ptr(make_unique<EnfaceOutputImpl>())
{
}


OctResult::EnfaceOutput::~EnfaceOutput() = default;
OctResult::EnfaceOutput::EnfaceOutput(EnfaceOutput && rhs) = default;
EnfaceOutput & OctResult::EnfaceOutput::operator=(EnfaceOutput && rhs) = default;


const EnfaceImage* OctResult::EnfaceOutput::getEnfaceImage(void)
{
	return d_ptr->enface.get();
}


const OctEnfaceImage * OctResult::EnfaceOutput::getEnfaceImageDescript(void)
{
	auto image = getEnfaceImage();
	if (image != nullptr) {
		return &image->getDescript();
	}
	return nullptr;
}


bool OctResult::EnfaceOutput::setEnfaceImage(const OctEnfaceImage& enface, bool process)
{
	d_ptr->enface = make_unique<EnfaceImage>(enface);
	if (process) {
		d_ptr->enface->processImage();
	}
	return true;
}


bool OctResult::EnfaceOutput::exportImage(const std::wstring & dirName,
	const std::wstring & fileName)
{
	return getImpl().enface->exportFile(dirName, fileName);
}


bool OctResult::EnfaceOutput::importImage(const std::wstring& dirName,
	const std::wstring& fileName)
{
	return getImpl().enface->importFile(dirName, fileName);
}


EnfaceOutput::EnfaceOutputImpl & OctResult::EnfaceOutput::getImpl(void) const
{
	return *d_ptr;
}
