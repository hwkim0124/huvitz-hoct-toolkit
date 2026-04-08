#include "stdafx.h"
#include "MacularBscan.h"

using namespace SemtSegm;
using namespace std;


struct MacularBscan::MacularBscanImpl
{
	MacularBscanImpl()
	{
	}
};


MacularBscan::MacularBscan() :
	d_ptr(make_unique<MacularBscanImpl>())
{
}


SemtSegm::MacularBscan::~MacularBscan() = default;
SemtSegm::MacularBscan::MacularBscan(MacularBscan && rhs) = default;
MacularBscan & SemtSegm::MacularBscan::operator=(MacularBscan && rhs) = default;


MacularBscan::MacularBscanImpl & SemtSegm::MacularBscan::getImpl(void) const
{
	return *d_ptr;
}

