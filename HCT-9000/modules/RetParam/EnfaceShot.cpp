#include "stdafx.h"
#include "EnfaceShot.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;



struct EnfaceShot::EnfaceShotImpl
{
	EnfaceShotImpl() {
	}
};


EnfaceShot::EnfaceShot() :
	d_ptr(make_unique<EnfaceShotImpl>())
{
}


RetParam::EnfaceShot::~EnfaceShot() = default;
RetParam::EnfaceShot::EnfaceShot(EnfaceShot && rhs) = default;
EnfaceShot & RetParam::EnfaceShot::operator=(EnfaceShot && rhs) = default;


RetParam::EnfaceShot::EnfaceShot(const EnfaceShot & rhs)
	: d_ptr(make_unique<EnfaceShotImpl>(*rhs.d_ptr))
{
}


EnfaceShot & RetParam::EnfaceShot::operator=(const EnfaceShot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool RetParam::EnfaceShot::copyToImage(CppUtil::CvImage * image, int width, int height)
{
	if (isEmpty()) {
		return false;
	}

	Size dsize(width, height);
	cv::resize(getDataImage()->getCvMatConst(), image->getCvMat(), dsize, INTER_CUBIC);
	return true;
}

