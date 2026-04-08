#pragma once

#include "KerFocusDef.h"

#include <vector>

namespace KerFocus
{
	template <typename T>
	struct KerPoint2 
	{
		T x_;
		T y_;

	public:
		KerPoint2() : x_(0), y_(0) {}

		KerPoint2(T x, T y) {
			x_ = x;
			y_ = y;
		}

		void move(KerPoint2<T> pt) {
			x_ += pt.x_;
			y_ += pt.y_;
		}

		bool isAbove(T y) const {
			return y_ < y;
		}

		bool isBelow(T y) const {
			return y_ > y;
		}

		bool isLeft(T x) const {
			return x_ < x;
		}

		bool isRight(T x) const {
			return x_ > x;
		}

		bool inBetweenX(T x1, T x2) const {
			return (x_ >= x1 && x_ <= x2);
		}

		bool inBetweenY(T y1, T y2) const {
			return (y_ >= y1 && y_ <= y2);
		}
	};

	template <typename T>
	using KerPoint2Vect = std::vector<KerPoint2<T>>;

}