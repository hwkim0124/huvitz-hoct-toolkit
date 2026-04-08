#include "stdafx.h"
#include "CubicSpline.h"


using namespace CppUtil;
using namespace std;

CubicSpline::CubicSpline()
{
}


CubicSpline::~CubicSpline()
{
}

void CppUtil::CubicSpline::setPoints(std::vector<double> Xs, std::vector<double> Ys)
{
	m_pTkSpline = make_unique<tk::spline>();
	m_pTkSpline->set_points(Xs, Ys);
	return;
}

std::vector<double> CppUtil::CubicSpline::interpolate(std::vector<double> Xs)
{
	auto Ys = std::vector<double>(Xs.size());
	for (int i = 0; i < Ys.size(); i++) {
		Ys.at(i) = (m_pTkSpline->operator()(Xs.at(i)));
	}
	return Ys;
}
