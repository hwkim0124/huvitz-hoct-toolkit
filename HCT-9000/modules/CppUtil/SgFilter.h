#pragma once

#include "CppUtilDef.h"


#include <vector>

namespace CppUtil
{
	class CPPUTIL_DLL_API SgFilter
	{
	public:
		SgFilter();
		~SgFilter();

	public:
		static std::vector<double> smoothDoubles(std::vector<double> data, int window, int degree);
		static std::vector<float> smoothFloats(std::vector<float> data, int window, int degree);
		static std::vector<int> smoothInts(std::vector<int> data, int window, int degree);
	};
}

