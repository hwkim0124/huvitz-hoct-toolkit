#include "stdafx.h"
#include "NumericFunc.h"
#include <math.h>

using namespace CppUtil;


NumericFunc::NumericFunc()
{
}


NumericFunc::~NumericFunc()
{
}


double CppUtil::NumericFunc::degreeToRadian(double degree)
{
	return ((degree * PI()) / 180.0);
}


double CppUtil::NumericFunc::radianToDegree(double radian)
{
	return ((radian * 180.0) / PI());
}


double CppUtil::NumericFunc::PI(void)
{
	return 3.14159265358979323;
}


double CppUtil::NumericFunc::horizontalLengthOfEllipse(double m, double n, double a)
{
	double rad = degreeToRadian(a);
	double cos_t = cos(rad);
	double sin_t = sin(rad);

	double len = m * n * sqrt(1.0 / (n*n*cos_t*cos_t + m*m*sin_t*sin_t));
	return len;
}
