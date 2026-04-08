#pragma once

#include "KerFocusDef.h"

#include <vector>

namespace KerFocus
{
	struct KerPoint {
		int _x, _y;

	public:
		KerPoint() {
			_x = 0, _y = 0;
		}

		KerPoint(int x1, int y1) {
			_x = x1, _y = y1;
		}

		void add(KerPoint pt) {
			_x += pt._x;
			_y += pt._y;
		}
	};


	struct KerRect
	{
		int _sx;
		int _sy;
		int _ex;
		int _ey;

	public:
		KerRect() : _sx(0), _sy(0), _ex(0), _ey(0) {
		}

		KerRect(int sx, int sy, int ex, int ey) {
			_sx = sx;
			_sy = sy;
			_ex = ex;
			_ey = ey;
		}

		void setRect(KerRect rect) {
			_sx = rect._sx;
			_sy = rect._sy;
			_ex = rect._ex;
			_ey = rect._ey;
		}

		int getWidth(void) const {
			return abs(_ex - _sx) + 1;
		}

		int getHeight(void) const {
			return abs(_ey - _sy) + 1;
		}

		CRect getCRect(void) const {
			return CRect(_sx, _sy, _ex, _ey);
		}

		KerPoint getCenter(void) const {
			return KerPoint((_sx + _ex) / 2, (_sy + _ey) / 2);
		}

		void setOrigin(int cx, int cy) {
			_sx -= cx;
			_ex -= cx;
			_sy -= cy;
			_ey -= cy;
			return;
		}

		float getNorm2(void) const {
			KerPoint cent = getCenter();
			float norm = (float)sqrt(pow(cent._x, 2.0) + pow(cent._y, 2.0));
			return norm;
		}

		float getSizeRatio(void) {
			float w = (float)getWidth();
			float h = (float)getHeight();

			if (w == 0.0f || h == 0.0f) {
				return 0.0f;
			}
			else {
				return (w > h ? (w / h) : (h / w));
			}
		}

		float getDistance(int x, int y) {
			KerPoint cent = getCenter();
			float dist = (float)(sqrt(pow(cent._x - x, 2.0) + pow(cent._y - y, 2.0)));
			return dist;
		}

		float getDistance(KerPoint pt) {
			KerPoint cent = getCenter();
			float dist = (float)(sqrt(pow(cent._x - pt._x, 2.0) + pow(cent._y - pt._y, 2.0)));
			return dist;
		}

		void merge(KerRect rect) {
			_sx = (_sx <= rect._sx ? _sx : rect._sx);
			_sy = (_sy <= rect._sy ? _sy : rect._sy);
			_ex = (_ex >= rect._ex ? _ex : rect._ex);
			_ey = (_ey >= rect._ey ? _ey : rect._ey);
			return;
		}

		bool alignedX(KerRect rect, int offset = 1) {
			if ((_ex + offset) >= (rect._sx - offset) && (_sx - offset) <= (rect._ex + offset)) {
				return true;
			}
			return false;
		}

		bool alignedY(KerRect rect, int offset = 1) {
			if ((_ey + offset) >= (rect._sy - offset) && (_sy - offset) <= (rect._ey + offset)) {
				return true;
			}
			return false;
		}

		void resize(float xRatio, float yRatio) {
			_sx = (int)(_sx * xRatio);
			_sy = (int)(_sy * yRatio);
			_ex = (int)(_ex * xRatio);
			_ey = (int)(_ey * yRatio);
		}

		void expand(int size) {
			_sx = _sx - size;
			_sy = _sy - size;
			_ex = _ex + size;
			_ey = _ey + size;
		}

		bool enlargeToNextHorzLine(int sx, int sy, int ex, int ey) {
			if (((_sy - 1) == sy || (_ey + 1) == ey) &&
				((sx >= _sx && sx <= _ex) || (ex >= _sx && ex <= _ex) ||
				 (_sx >= sx && _sx <= ex) || (_ex >= sx && _ex <= ex))) {
				_sy = (_sy > sy ? sy : _sy);
				_ey = (_ey < ey ? ey : _ey);
				_sx = (_sx > sx ? sx : _sx);
				_ex = (_ex < ex ? ex : _ex);
				return true;
			}
			return false;
		}
	};


	struct KerSpot : KerRect {
		float _cx;
		float _cy;

		int _thresh;
		int _size;
		bool _valid;
		bool _focus;

		KerRect _rect;

	public:
		KerSpot() {
			init();
		}

		KerSpot(float cx, float cy, int size, int thresh, KerRect rect) {
			init();
			_cx = cx;
			_cy = cy;
			_size = size;
			_thresh = thresh;
			_rect = rect;
			_valid = true;
		}

		KerSpot(KerRect rect) {
			init();
			setRect(rect);
		}

		void init(void) {
			_cx = 0.0f;
			_cy = 0.0f;
			_thresh = 0;
			_size = 0;
			_valid = false;
			_focus = false;
		}

		bool isFocus(void) {
			return _focus;
		}

		KerPoint getCenter(void) {
			KerPoint pt((int)_cx, (int)_cy);
			return pt;
		}
	};


	typedef std::vector<KerPoint> KerPointVect;
	typedef std::vector<KerSpot> KerSpotVect;
	typedef std::vector<KerRect> KerRectVect;
}