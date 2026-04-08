#include "stdafx.h"
#include "CameraResult.h"
#include "PhotoProfile.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct CameraResult::CameraResultImpl
{
	PhotoProfile fundus;
	bool empty;

	CameraResultImpl() : empty(true)
	{
	}
};


CameraResult::CameraResult() :
	d_ptr(make_unique<CameraResultImpl>())
{
}


OctResult::CameraResult::~CameraResult() = default;
OctResult::CameraResult::CameraResult(CameraResult && rhs) = default;
CameraResult & OctResult::CameraResult::operator=(CameraResult && rhs) = default;


OctResult::CameraResult::CameraResult(const CameraResult & rhs)
	: d_ptr(make_unique<CameraResultImpl>(*rhs.d_ptr))
{
}


CameraResult & OctResult::CameraResult::operator=(const CameraResult & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


PhotoProfile & OctResult::CameraResult::getPhotoProfile(void) const
{
	return d_ptr->fundus;
}


bool OctResult::CameraResult::isEmpty(void) const
{
	return d_ptr->empty;
}


CameraResult::CameraResultImpl & OctResult::CameraResult::getImpl(void) const
{
	return *d_ptr;
}
