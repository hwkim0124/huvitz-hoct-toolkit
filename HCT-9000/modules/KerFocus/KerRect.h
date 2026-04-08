#pragma once

#include "KerFocusDef.h"
#include "KerPoint.h"

#include <vector>

namespace KerFocus
{
	template <typename T>
	struct KerRect2 
	{
		KerPoint2<T> pt1_;
		KerPoint2<T> pt2_;

	public:
		KerRect2() {

		}

		KerRect2(const KerPoint2<T>& pt1, const KerPoint2<T>& pt2) {
			pt1_ = pt1;
			pt2_ = pt2;
		}

		T width(void) const {
			return std::abs(pt2_.x_ - pt1_.x_) + 1;
		}

		T height(void) const {
			return std::abs(pt2_.y_ - pt1_.y_) + 1;
		}

		KerPoint2<T> center(void) const {
			return KerPoint2<T>((pt1_.x_ + pt2_.x_) / 2.0f, (pt1_.y_ + pt2_.y_) / 2.0f);
		}

		CRect getCRect(void) const {
			return CRect((int)pt1_.x_, (int)pt1_.y_, (int)pt2_.x_, (int)pt2_.y_);
		}

		void setOrigin(T x, T y) {
			pt1_.move(x, y);
			pt2_.move(x, y);
		}

		float sizeRatio(void) const {
			float w = (float)width();
			float h = (float)height();

			if (w == 0.0f || h == 0.0f) {
				return 0.0f;
			}
			else {
				return (w > h ? (w / h) : (h / w));
			}
		}

		void merge(const KerRect2<T>& rect) {
			pt1_.x_ = (pt1_.x_ <= rect.pt1_.x_ ? pt1_.x_ : rect.pt1_.x_);
			pt1_.y_ = (pt1_.y_ <= rect.pt1_.y_ ? pt1_.y_ : rect.pt1_.y_);
			pt2_.x_ = (pt2_.x_ >= rect.pt2_.x_ ? pt2_.x_ : rect.pt2_.x_);
			pt2_.y_ = (pt2_.y_ >= rect.pt2_.y_ ? pt2_.y_ : rect.pt2_.y_);
			return;
		}

		bool alignedX(const KerRect2<T>& rect, T offset = 1.0) const {
			if ((pt2_.x_ + offset) >= (rect.pt1_.x_ - offset) && 
				(pt1_.x_ - offset) <= (rect.pt2_.x_ + offset)) {
				return true;
			}
			return false;
		}

		bool alignedY(const KerRect2<T>& rect, T offset = 1.0) const {
			if ((pt2_.y_ + offset) >= (rect.pt1_.y_ - offset) && 
				(pt1_.y_ - offset) <= (rect.pt2_.y_ + offset)) {
				return true;
			}
			return false;
		}

		bool includeX(T x, T offset = 1.0) const {
			if ((pt1_.x_ - offset) <= x && x <= (pt2_.x_ + offset)) {
				return true;
			}
			return false;
		}

		bool includeY(T y, T offset = 1.0) const {
			if ((pt1_.y_ - offset) <= y && y <= (pt2_.y_ + offset)) {
				return true;
			}
			return false;
		}
	};

	template <typename T>
	using KerRect2Vect = std::vector<KerRect2<T>> ;


}