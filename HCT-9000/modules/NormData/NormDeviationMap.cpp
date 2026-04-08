#include "stdafx.h"
#include "NormDeviationMap.h"

#include <stdio.h>
#include <string>
#include <map>

#include "CppUtil2.h"

using namespace NormData;
using namespace CppUtil;
using namespace std;


struct NormDeviationMap::NormDeviationMapImpl
{
	NormDeviationMapImpl()
	{
	}
};


NormDeviationMap::NormDeviationMap() : d_ptr(make_unique<NormDeviationMapImpl>())
{
}


NormDeviationMap::~NormDeviationMap()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
NormData::NormDeviationMap::NormDeviationMap(NormDeviationMap && rhs) = default;
NormDeviationMap & NormData::NormDeviationMap::operator=(NormDeviationMap && rhs) = default;


CppUtil::CvImage NormData::NormDeviationMap::makeImage(Ethinicity race, Gender gender, int age, EyeSide side, const std::vector<float>& data, int lines, int points, int width, int height, bool isWide) 
{
	return CppUtil::CvImage();
}

NormDeviationMap::NormDeviationMapImpl & NormData::NormDeviationMap::getImpl(void) const
{
	return *d_ptr;
}
