#include "stdafx.h"
#include "OcularEnfaceImage.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct OcularEnfaceImage::OcularEnfaceImageImpl
{
	RetParam::EnfaceShot enfaceShot;

	OcularEnfaceImageImpl()
	{
	}
};


OcularEnfaceImage::OcularEnfaceImage() :
	d_ptr(make_unique<OcularEnfaceImageImpl>())
{
}


SegmScan::OcularEnfaceImage::~OcularEnfaceImage() = default;
SegmScan::OcularEnfaceImage::OcularEnfaceImage(OcularEnfaceImage && rhs) = default;
OcularEnfaceImage & SegmScan::OcularEnfaceImage::operator=(OcularEnfaceImage && rhs) = default;


SegmScan::OcularEnfaceImage::OcularEnfaceImage(const OcularEnfaceImage & rhs)
	: d_ptr(make_unique<OcularEnfaceImageImpl>(*rhs.d_ptr))
{
}


OcularEnfaceImage & SegmScan::OcularEnfaceImage::operator=(const OcularEnfaceImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool SegmScan::OcularEnfaceImage::setupEnfaceImage(const OctScanPattern & pattern, OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, const std::vector<OcularBsegm*>& bsegms)
{
	auto lines = vector<vector<uint8_t>>();
	auto flags = vector<bool>();
	auto pixels = vector<uint8_t>();
	bool output;

	for (const auto& b : bsegms) {
		if (b != nullptr) {
			if (pattern.getPatternName() == PatternName::MacularAngio ||
				pattern.getPatternName() == PatternName::DiscAngio ||
				pattern.getPatternName() == PatternName::AnteriorAngio)
			{
				output = b->getLateralLine(upper, lower, pixels, upperOffset, lowerOffset);
			}
			else {
				// output = b->getMaxValueLine(upper, lower, pixels);
				output = b->getLateralLine(upper, lower, pixels, upperOffset, lowerOffset);
			}
			lines.push_back(std::move(pixels));
			flags.push_back(output);
		}
		else {
			return false;
		}
	}

	bool result = getImpl().enfaceShot.setupData(pattern, upper, lower, upperOffset, lowerOffset, lines, flags);
	return result;
}


bool SegmScan::OcularEnfaceImage::isIdentical(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset) const
{
	return getImpl().enfaceShot.isIdentical(upper, lower, upperOffset, lowerOffset);
}


CppUtil::CvImage SegmScan::OcularEnfaceImage::makeImage(int width, int height, bool enhance) const
{
	auto image = CppUtil::CvImage();
	getImpl().enfaceShot.copyToImage(&image, width, height);

	if (enhance) {
		bool vert = getImpl().enfaceShot.isVertical();
		image.applyBiasFieldCorrection(vert);
		// image.applyGuidedFilter(1.0, 0.5);
		// image.applyVesselEnhancement(1.5, 1.7, -0.7);
		image.equalizeHistogram(3.0);
	}
	return image;
}


OcularEnfaceImage::OcularEnfaceImageImpl & SegmScan::OcularEnfaceImage::getImpl(void) const
{
	return *d_ptr;
}
