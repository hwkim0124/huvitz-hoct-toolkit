#include "stdafx.h"
#include "EnfaceTmap.h"


#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct EnfaceTmap::EnfaceTmapImpl
{

	EnfaceTmapImpl() {
	}
};


EnfaceTmap::EnfaceTmap() :
	d_ptr(make_unique<EnfaceTmapImpl>())
{
}


RetParam::EnfaceTmap::~EnfaceTmap() = default;
RetParam::EnfaceTmap::EnfaceTmap(EnfaceTmap && rhs) = default;
EnfaceTmap & RetParam::EnfaceTmap::operator=(EnfaceTmap && rhs) = default;


RetParam::EnfaceTmap::EnfaceTmap(const EnfaceTmap & rhs)
	: d_ptr(make_unique<EnfaceTmapImpl>(*rhs.d_ptr))
{
}


EnfaceTmap & RetParam::EnfaceTmap::operator=(const EnfaceTmap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool RetParam::EnfaceTmap::copyToImage(CppUtil::CvImage * image, int width, int height)
{
	if (isEmpty()) {
		return false;
	}

	Size dsize;
	// width나 height가 유효하지 않은 값일 경우, 원본 크기를 그대로 사용 함
	if (width < 0 || height < 0) {
		dsize.width = getDataImage()->getCvMatConst().cols;
		dsize.height = getDataImage()->getCvMatConst().rows;
	}
	//
	else {
		dsize.width = width;
		dsize.height = height;
	}

	// Source's type should not be 32SC1, instead 32FC1. 
	cv::resize(getDataImage()->getCvMatConst(), image->getCvMat(), dsize, INTER_CUBIC);
	return true;
}

