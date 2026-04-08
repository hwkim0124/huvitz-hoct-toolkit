#pragma once


#include "RetSegmDef.h"

#include <memory>
#include <vector>


namespace RetSegm
{
	template <typename T>
	struct SegmPoint
	{
		T x;
		T y;
		bool valid;

		SegmPoint(void) :
			x(-1), y(-1), valid(false)
		{
		}

		SegmPoint(T nx, T ny) :
			x(nx), y(ny)
		{
			valid = (ny >= 0 ? true : false);
		}

		SegmPoint(T nx, T ny, bool flag) :
			x(nx), y(ny), valid(flag)
		{
		}

		void set(T nx, T ny) {
			x = nx; 
			y = ny;
			valid = (ny >= 0 ? true : false);
		}

		void set(T nx, T ny, bool flag) {
			x = nx;
			y = ny;
			valid = flag;
		}

		T getX(void) const {
			return x;
		}

		T getY(void) const {
			return y;
		}

		bool isValid(void) const {
			return valid;
		}
	};

	template <typename T>
	using SegmPointList = std::vector<SegmPoint<T>>;

	using LayerPoint = SegmPoint<int>;
	using LayerPointList = SegmPointList<int>;
}