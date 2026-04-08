#pragma once

#include "OctGlobalDef.h"


namespace OctGlobal
{
	enum class Ethinicity : unsigned short {
		UNKNOWN = 0,
		ASIAN = 1,
		COCASIAN = 2,
		LATINO = 3,
		OTHER = 7,
		//MIXED = 7,
	};

	enum class Gender
	{
		UNKNOWN = 0, 
		MALE = 1, 
		FEMALE = 2
	};
}