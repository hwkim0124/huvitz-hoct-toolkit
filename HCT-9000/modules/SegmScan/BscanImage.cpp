#include "stdafx.h"
#include "BscanImage.h"
#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SegmScan;
using namespace RetSegm;
using namespace CppUtil;
using namespace cv;
using namespace std;


struct BscanImage::BscanImageImpl
{
	RetSegm::SegmImage imgSource;
	RetSegm::SegmImage imgResized;
	RetSegm::SegmImage imgSmoothed; 
	RetSegm::SegmImage imgSample;  
	RetSegm::SegmImage imgGradient;  
	RetSegm::SegmImage imgCost; 

	RetSegm::SegmLayer layerInn;
	RetSegm::SegmLayer layerOut;
	RetSegm::SegmLayer layerILM;
	RetSegm::SegmLayer layerIOS;
	RetSegm::SegmLayer layerRPE;
	RetSegm::SegmLayer layerCHR;

	std::wstring path;

	BscanImageImpl() : path(L"") {

	}
};


SegmScan::BscanImage::BscanImage() :
	d_ptr(make_unique<BscanImageImpl>()) 
{
}


SegmScan::BscanImage::BscanImage(const std::wstring& path) :
	d_ptr(make_unique<BscanImageImpl>())
{
	loadFile(path);
}


SegmScan::BscanImage::~BscanImage() = default;
SegmScan::BscanImage::BscanImage(BscanImage && rhs) = default;
BscanImage & SegmScan::BscanImage::operator=(BscanImage && rhs) = default;


SegmScan::BscanImage::BscanImage(const BscanImage & rhs)
	: d_ptr(make_unique<BscanImageImpl>(*rhs.d_ptr))
{
}


BscanImage & SegmScan::BscanImage::operator=(const BscanImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool SegmScan::BscanImage::loadFile(const std::wstring& path)
{
	if (!d_ptr->imgSource.fromFile(path)) {
		return false;
	}
	d_ptr->path = path;
	return true;
}


bool SegmScan::BscanImage::loadBitsData(const unsigned char * pBits, int width, int height, int padding)
{
	if (!d_ptr->imgSource.fromBitsData(pBits, width, height, padding)) {
		return false;
	}
	return true;
}


bool SegmScan::BscanImage::isEmpty(void) const
{
	return d_ptr->imgSource.isEmpty();
}


bool SegmScan::BscanImage::performAnalysis(void)
{
	if (source()->isEmpty()) { 
		return false;
	}

	Preprocess::createSampleImageByRatio(source(), resized(), RESIZED_WIDTH_RATIO, RESIZED_HEIGHT_RATIO);
	Preprocess::createSampleImageBySize(resized(), sample(), SAMPLE_WIDTH_MIN, SAMPLE_HEIGHT_MIN);
	Preprocess::smoothBackgroundNoise(sample(), smoothed());
	Preprocess::collectImageStats(smoothed());
	Preprocess::suppressMoirePattern(smoothed());
	Preprocess::collectColumnStats(smoothed());
	
	if (Preprocess::checkValidityOfScanImage(smoothed()) == false) {
		// return false;
	}

	Coarse::createGradientMap(smoothed(), gradients());
	Preprocess::collectColumnStats(gradients());

	Coarse::buildBoundaryPairOfRetina(gradients(), layerInn(), layerOut());
	OptimalPath::findBoundaryToILM(resized(), layerInn(), layerOut(), costs(), layerILM());
	OptimalPath::findBoundaryToIOS(resized(), layerILM(), layerOut(), costs(), layerIOS());
	return false;
}


Gdiplus::Bitmap * SegmScan::BscanImage::getBitmap(ImageType type)
{
	SegmImage *p = getSegmImage(type);
	if (p != nullptr) {
		return p->getBitmap();
	}
	return nullptr;
}


int SegmScan::BscanImage::getWidth(ImageType type) const
{
	SegmImage *p = getSegmImage(type);
	if (p != nullptr) {
		return p->getWidth();
	}
	return 0;
}


int SegmScan::BscanImage::getHeight(ImageType type) const
{
	SegmImage *p = getSegmImage(type);
	if (p != nullptr) {
		return p->getHeight();
	}
	return 0;
}


const unsigned char * SegmScan::BscanImage::getBitsData(ImageType type) const
{
	SegmImage *p = getSegmImage(type);
	if (p != nullptr) {
		return p->getBitsData();
	}
	return 0;
}


std::vector<int> SegmScan::BscanImage::getLayerPoints(LayerType type, ImageType image) const
{
	SegmLayer *p = getSegmLayer(type);
	SegmImage *i = getSegmImage(image);
	if (p != nullptr && i != nullptr) {
		return p->getYsResized(i->getWidth(), i->getHeight());
	}
	return std::vector<int>();
}


std::vector<int> SegmScan::BscanImage::getLayerPoints(LayerType type, int width, int height) const
{
	SegmLayer *p = getSegmLayer(type);
	if (p != nullptr) {
		return p->getYsResized(width, height);
	}
	return std::vector<int>();
}


std::wstring SegmScan::BscanImage::getFilePath(void) const
{
	return d_ptr->path;
}


std::wstring SegmScan::BscanImage::getFileName(void) const
{
	wstring path = getFilePath();
	wstring name = L"";
	int pos = (int)path.find_last_of(L"/\\");
	if (pos >= 0) {
		name = path.substr(pos + 1);
	}
	else {
		name = path;
	}
	return name;
}


RetSegm::SegmImage * SegmScan::BscanImage::resized(void) const
{
	return &(d_ptr->imgResized);
}


RetSegm::SegmImage * SegmScan::BscanImage::sample(void) const
{
	return &(d_ptr->imgSample);
}


RetSegm::SegmImage * SegmScan::BscanImage::smoothed(void) const
{
	return &(d_ptr->imgSmoothed);
}


RetSegm::SegmImage * SegmScan::BscanImage::source(void) const
{
	return &(d_ptr->imgSource);
}


RetSegm::SegmImage * SegmScan::BscanImage::gradients(void) const
{
	return &(d_ptr->imgGradient);
}


RetSegm::SegmImage * SegmScan::BscanImage::costs(void) const
{
	return &(d_ptr->imgCost);
}


RetSegm::SegmLayer * SegmScan::BscanImage::layerInn(void) const
{
	return &(d_ptr->layerInn);
}


RetSegm::SegmLayer * SegmScan::BscanImage::layerOut(void) const
{
	return &(d_ptr->layerOut);
}


RetSegm::SegmLayer * SegmScan::BscanImage::layerILM(void) const
{
	return &(d_ptr->layerILM);
}


RetSegm::SegmLayer * SegmScan::BscanImage::layerIOS(void) const
{
	return &(d_ptr->layerIOS);
}


RetSegm::SegmLayer * SegmScan::BscanImage::layerRPE(void) const
{
	return &(d_ptr->layerRPE);
}


RetSegm::SegmImage * SegmScan::BscanImage::getSegmImage(ImageType type) const
{
	switch (type)
	{
	case ImageType::SOURCE:
		return source();
	case ImageType::RESIZED:
		return resized();
	case ImageType::SAMPLE:
		return sample();
	case ImageType::SMOOTHED:
		return smoothed();
	case ImageType::GRADIENTS:
		return gradients();
	case ImageType::COSTS:
		return costs();
	default:
		break;
	}
	return nullptr;
}


RetSegm::SegmLayer * SegmScan::BscanImage::getSegmLayer(LayerType type) const
{
	switch (type)
	{
	case LayerType::INNER:
		return layerInn();
	case LayerType::OUTER:
		return layerOut();
	case LayerType::ILM:
		return layerILM();
	case LayerType::IOS:
		return layerIOS();
	case LayerType::RPE:
		return layerRPE();
	case LayerType::NONE:
	default:
		break;
	}
	return nullptr;
}



