#pragma once

#include "KerFocusDef.h"
#include "KerPoint.h"
#include "KerRect.h"

#include <vector>

namespace KerFocus
{
	template <typename T>
	struct KerSpot2 
	{
		KerRect2<T> window_;
		KerPoint2<T> center_;

		int size_;
		int thresh_;
		bool focus_;
		bool valid_;

	public:
		KerSpot2() : size_(0), focus_(false), valid_(false) {}

		KerSpot2(KerRect2<T> wind) : size_(0), focus_(false), valid_(false) {
			window_ = wind;
		}

		const KerPoint2<T>& pt1(void) const {
			return window_.pt1_;
		}

		const KerPoint2<T>& pt2(void) const {
			return window_.pt2_;
		}

		const KerPoint2<T>& center(void) const {
			return center_;
		}

		const KerRect2<T>& window(void) const {
			return window_;
		}

	};

	template <typename T>
	using KerSpot2Vect = std::vector<KerSpot2<T>>;

	template <typename T, typename E>
	float distanceBetween(const T& rect, const E& pt) {
		return (float)std::sqrt(std::pow(rect.center().x_ - pt.x_, 2.0) + std::pow(rect.center().y_ - pt.y_, 2.0));
	}

	template <typename T>
	float distanceBetween(const T& rect1, const T& rect2) {
		return (float)std::sqrt(std::pow(rect1.center().x_ - rect2.center().x_, 2.0) + std::pow(rect1.center().y_ - rect2.center().y_, 2.0));
	}
}