#include "stdafx.h"
#include "CorneaBscan.h"

using namespace SemtSegm;
using namespace std;


struct CorneaBscan::CorneaBscanImpl
{
	CorneaBscanImpl()
	{
	}
};


CorneaBscan::CorneaBscan() :
	d_ptr(make_unique<CorneaBscanImpl>())
{
}


SemtSegm::CorneaBscan::~CorneaBscan() = default;
SemtSegm::CorneaBscan::CorneaBscan(CorneaBscan && rhs) = default;
CorneaBscan & SemtSegm::CorneaBscan::operator=(CorneaBscan && rhs) = default;


CorneaBscan::CorneaBscanImpl & SemtSegm::CorneaBscan::getImpl(void) const
{
	return *d_ptr;
}

