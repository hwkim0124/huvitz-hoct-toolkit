#pragma once

#include "CppUtilDef.h"
#include "TkSpline.h"


#include <memory>
#include <vector>

namespace CppUtil
{
	class CPPUTIL_DLL_API CubicSpline
	{
	public:
		CubicSpline();
		virtual ~CubicSpline();

	private:
		std::unique_ptr<tk::spline> m_pTkSpline;

	public:
		void setPoints(std::vector<double> Xs, std::vector<double> Ys);
		std::vector<double> interpolate(std::vector<double> Xs);
	};
}
