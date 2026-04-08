#pragma once

#include "CppUtilDef.h"

#include <vector>


namespace CppUtil
{
	class CPPUTIL_DLL_API Regressor
	{
	public:
		Regressor();
		~Regressor();

	public:
		static bool chevyPolyCoeffs(std::vector<double> dataX, std::vector<double> dataY, int degree, std::vector<double>& coeffs);
		static bool chevyPolyCurve(std::vector<int> dataX, std::vector<int> dataY, int degree, bool nonZero, std::vector<int>& result, std::vector<double>& coeffs = std::vector<double>());
		static bool chevyPolyCurve(std::vector<double> dataX, std::vector<double> dataY, int degree, bool nonZero, std::vector<double>& result, std::vector<double>& coeffs = std::vector<double>());
		static bool calcRmsError(std::vector<double> dataY, std::vector<double> fittY, int degree, double *rms);

		static bool polyFit(const std::vector<double>& dataX, const std::vector<double>& dataY, int degree, std::vector<double>& coeffs);
	};


}

