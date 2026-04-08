#include "stdafx.h"
#include "ScanProfile.h"

#include "CppUtil2.h"
#include "OctGlobal2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct ScanProfile::ScanProfileImpl
{
	EyeSide eyeSide;
	PatternDomain domain;
	PatternType type;

	float scanAngle;
	float scanRange;
	float scanOffsetX;
	float scanOffsetY;
	int numAverage;

	ScanProfileImpl()
	{
	}
};


OctResult::ScanProfile::ScanProfile() :
	d_ptr(make_unique<ScanProfileImpl>())
{
}


ScanProfile::ScanProfile(EyeSide side, PatternDomain domain, PatternType type) :
	d_ptr(make_unique<ScanProfileImpl>())
{
	getImpl().eyeSide = side;
	getImpl().domain = domain;
	getImpl().type = type;
}


OctResult::ScanProfile::~ScanProfile() = default;
OctResult::ScanProfile::ScanProfile(ScanProfile && rhs) = default;
ScanProfile & OctResult::ScanProfile::operator=(ScanProfile && rhs) = default;


OctResult::ScanProfile::ScanProfile(const ScanProfile & rhs)
	: d_ptr(make_unique<ScanProfileImpl>(*rhs.d_ptr))
{
}


ScanProfile & OctResult::ScanProfile::operator=(const ScanProfile & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


EyeSide OctResult::ScanProfile::eyeSide(void) const
{
	return getImpl().eyeSide;
}


PatternDomain OctResult::ScanProfile::patternDomain(void) const
{
	return getImpl().domain;
}


PatternType OctResult::ScanProfile::patternType(void) const
{
	return getImpl().type;
}


ScanProfile::ScanProfileImpl & OctResult::ScanProfile::getImpl(void) const
{
	return *d_ptr;
}
