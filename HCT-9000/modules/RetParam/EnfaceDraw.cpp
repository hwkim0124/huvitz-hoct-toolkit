#include "stdafx.h"
#include "EnfaceDraw.h"
#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct EnfaceDraw::EnfaceDrawImpl
{
	CvImage plate;
	vector<SegmImage*> bscans;
	vector<SegmLayer*> layers1;
	vector<SegmLayer*> layers2;

	float areaWidth;
	float areaHeight;
	int bscanWidth;
	int bscanHeight;
	int plateWidth;
	int plateHeight;

	EnfaceDrawImpl() {
	}
};


EnfaceDraw::EnfaceDraw() :
	d_ptr(make_unique<EnfaceDrawImpl>())
{
}


RetParam::EnfaceDraw::~EnfaceDraw() = default;
RetParam::EnfaceDraw::EnfaceDraw(EnfaceDraw && rhs) = default;
EnfaceDraw & RetParam::EnfaceDraw::operator=(EnfaceDraw && rhs) = default;


RetParam::EnfaceDraw::EnfaceDraw(const EnfaceDraw & rhs)
	: d_ptr(make_unique<EnfaceDrawImpl>(*rhs.d_ptr))
{
}


EnfaceDraw & RetParam::EnfaceDraw::operator=(const EnfaceDraw & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool RetParam::EnfaceDraw::checkIfValidAllBscans(void)
{
	int count = 0;
	for (auto p : d_ptr->bscans) {
		if (p == nullptr || p->isEmpty()) {
			return false;
		}
		count++;
	}

	if (count < ENFACE_BSCANS_MIN) {
		return false;
	}

	d_ptr->bscanWidth = d_ptr->bscans[0]->getWidth();
	d_ptr->bscanHeight = d_ptr->bscans[0]->getHeight();
	d_ptr->plateWidth = d_ptr->bscanWidth;
	d_ptr->plateHeight = (int)d_ptr->bscans.size();

	for (auto p : d_ptr->bscans) {
		if (p->getWidth() != d_ptr->bscanWidth || p->getHeight() != d_ptr->bscanHeight) {
			return false;
		}
	}

	return true;
}


bool RetParam::EnfaceDraw::checkIfValidAllLayers(void)
{
	int count = (int)d_ptr->bscans.size();
	if (count != d_ptr->layers1.size() || count != d_ptr->layers2.size()) {
		return false;
	}

	for (auto p : d_ptr->layers1) {
		if (p != nullptr && p->getSize() != d_ptr->bscanWidth) {
			return false;
		}
	}

	for (auto p : d_ptr->layers2) {
		if (p != nullptr && p->getSize() != d_ptr->bscanWidth) {
			return false;
		}
	}

	return true;
}


void RetParam::EnfaceDraw::buildPlate(void)
{
	int h = getPlateHeight();
	int w = getPlateWidth();
	Mat mat = Mat::zeros(h, w, CV_8UC1);

	int gsum, gcnt;
	int ypos1, ypos2;

	vector<int> ys1_def = vector<int>(mat.cols, 0);
	vector<int> ys2_def = vector<int>(mat.cols, getBscanHeight() - 1);


	//DebugOut2() << "\n\n\n\n";
	for (int r = 0; r < mat.rows; r++) {
		unsigned char* p = (unsigned char*)mat.ptr(r);
		auto l1 = getUpperLayer(r);
		auto l2 = getLowerLayer(r);
		auto img = d_ptr->bscans[r]->getCvMat();

		auto ys1 = (l1 == nullptr ? ys1_def : l1->getYs());
		auto ys2 = (l2 == nullptr ? ys2_def : l2->getYs());

		//std::string str;
		//str = "[" + to_string(r) + "] ";
		for (int c = 0; c < mat.cols; c++) {
			// If layer is not valid, get a average gray level through entire column. 
			ypos1 = (ys1[c] >= 0 ? ys1[c] : 0);
			ypos2 = (ys2[c] >= 0 ? ys2[c] : getBscanHeight() - 1) + 150;

			gsum = gcnt = 0;
			for (int k = ypos1; k <= ypos2; k++) {
				gsum += img.at<uchar>(k, c);
				gcnt++;
			}
			if (gcnt > 0) {
				p[c] = (unsigned char)(gsum / gcnt);
			}

			//str += to_string(ypos1);
			//str += ":";
			//str += to_string(ypos2);
			//str += " ";
		}

		//DebugOut2() << str;
	}

	//DebugOut2() << "\n\n\n\n";

	getPlate()->getCvMat() = mat;
	return;
}


void RetParam::EnfaceDraw::copyToImage(CppUtil::CvImage* image, int width, int height)
{
	Size dsize(width, height);
	cv::resize(getPlate()->getCvMatConst(), image->getCvMat(), dsize, INTER_CUBIC);

	equalizeHistogram(image);
	return;
}


void RetParam::EnfaceDraw::equalizeHistogram(CppUtil::CvImage* image)
{
	// Refer
	// http://docs.opencv.org/3.1.0/d5/daf/tutorial_py_histogram_equalization.html#gsc.tab=0

	Mat dest;
	cv::Ptr<cv::CLAHE> p = cv::createCLAHE(4.0);
	p->apply(image->getCvMat(), dest);
	//cv::equalizeHist(getImage()->getCvMat(), dest);
	cv::add(dest, Scalar(25), dest);
	image->getCvMat() = dest;
	return;
}


int RetParam::EnfaceDraw::getPlateWidth(void) const
{
	return d_ptr->plateWidth;
}


int RetParam::EnfaceDraw::getPlateHeight(void) const
{
	return d_ptr->plateHeight;
}


int RetParam::EnfaceDraw::getBscanWidth(void) const
{
	return d_ptr->bscanWidth;
}


int RetParam::EnfaceDraw::getBscanHeight(void) const
{
	return d_ptr->bscanHeight;
}


float RetParam::EnfaceDraw::getScanWidth(void) const
{
	return d_ptr->areaWidth;
}


float RetParam::EnfaceDraw::getScanHeight(void) const
{
	return d_ptr->areaHeight;
}


float RetParam::EnfaceDraw::getHorzPixelPerMM(void) const
{
	return getPlateWidth()/getScanWidth();
}


float RetParam::EnfaceDraw::getVertPixelPerMM(void) const
{
	return getPlateHeight()/getScanHeight();
}


RetSegm::SegmLayer* RetParam::EnfaceDraw::getUpperLayer(int index) const
{
	if (index >= 0 && index < d_ptr->layers1.size()) {
		return d_ptr->layers1[index];
	}
	else {
		return nullptr;
	}
}


RetSegm::SegmLayer* RetParam::EnfaceDraw::getLowerLayer(int index) const
{
	if (index >= 0 && index < d_ptr->layers2.size()) {
		return d_ptr->layers2[index];
	}
	else {
		return nullptr;
	}
}


CvImage * RetParam::EnfaceDraw::getPlate(void) const
{
	return &(d_ptr->plate);
}


bool RetParam::EnfaceDraw::initialize(const std::vector<RetSegm::SegmImage*> bscans, const std::vector<RetSegm::SegmLayer*> layers1,
										const std::vector<RetSegm::SegmLayer*> layers2, float scanWidth, float scanHeight)
{
	d_ptr->bscans = bscans;
	d_ptr->layers1 = layers1;
	d_ptr->layers2 = layers2;
	d_ptr->areaWidth = scanWidth;
	d_ptr->areaHeight = scanHeight;

	if (checkIfValidAllBscans() == false) {
		return false;
	}

	if (checkIfValidAllLayers() == false) {
		return false;
	}

	buildPlate();
	return true;
}


bool RetParam::EnfaceDraw::drawImage(CppUtil::CvImage* image, int width, int height)
{
	if (getPlate()->isEmpty() || image == nullptr || width <= 0 || height <= 0) {
		return false;
	}

	copyToImage(image, width, height);
	return true;
}

