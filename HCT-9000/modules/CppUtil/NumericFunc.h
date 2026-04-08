#pragma once

#include "CppUtilDef.h"

namespace CppUtil
{
	class CPPUTIL_DLL_API NumericFunc
	{
	public:
		NumericFunc();
		~NumericFunc();

	public:
		static double degreeToRadian(double degree);
		static double radianToDegree(double radian);
		static double PI(void);

		static double horizontalLengthOfEllipse(double m, double n, double a);
	};
}
