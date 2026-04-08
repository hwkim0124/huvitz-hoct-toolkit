#pragma once

#include "OctGlobalDef.h"

#include <string>
#include <sstream>

namespace OctGlobal
{
	struct OctPoint
	{
	public:
		float _x;
		float _y;

	public:
		OctPoint() : _x(0.0f), _y(0.0f) {
		}

		OctPoint(float x, float y) {
			_x = x;
			_y = y;
		}

		void set(float x, float y) {
			_x = x;
			_y = y;
		}

		bool operator==(const OctPoint& rhs) const {
			return (this->_x == rhs._x && this->_y == rhs._y);
		}

		OctPoint& operator=(const OctPoint& rhs) {
			this->_x = rhs._x;
			this->_y = rhs._y;
			return *this;
		}

		OctPoint operator+(const OctPoint& rhs) const {
			return OctPoint(this->_x + rhs._x, this->_y + rhs._y);
		}

		OctPoint& operator+=(const OctPoint& rhs) {
			this->_x += rhs._x;
			this->_y += rhs._y;
			return *this;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<Point> " : "");
			ss << "x: " << this->_x << ", y: " << this->_y;
			return ss.str();
		}
	};


	struct OctPointN
	{
	public:
		int x;
		int y;

	public:
		OctPointN() : x(0), y(0) {
		}

		OctPointN(int x_, int y_) {
			x = x_;
			y = y_;
		}

		void set(int x_, int y_) {
			x = x_;
			y = y_;
		}

		bool operator==(const OctPointN& rhs) const {
			return (x == rhs.x && y == rhs.y);
		}

		OctPointN& operator=(const OctPointN& rhs) {
			x = rhs.x;
			y = rhs.y;
			return *this;
		}

		OctPointN operator+(const OctPointN& rhs) const {
			return OctPointN(x + rhs.x, y + rhs.y);
		}
	};


	struct OctRoute
	{
	public:
		bool _circle;
		OctPoint _pos1;
		OctPoint _pos2;

	public:
		OctRoute() : _circle(false) {
		}

		OctRoute(float x1, float y1, float x2, float y2, bool circle = false) {
			setLine(x1, y1, x2, y2);
			_circle = circle;
		}

		OctRoute(OctPoint pos1, OctPoint pos2, bool circle = false) {
			setLine(pos1, pos2);
			_circle = circle;
		}

		void setLine(OctPoint pos1, OctPoint pos2) {
			_pos1 = pos1;
			_pos2 = pos2;
			_circle = false;
			return;
		}

		void setLine(float x1, float y1, float x2, float y2) {
			setLine(OctPoint(x1, y1), OctPoint(x2, y2));
			return;
		}

		void setCircle(float x1, float y1, float radius) {
			setLine(OctPoint(x1 - radius, y1 - radius), OctPoint(x1 + radius, y1 + radius));
			_circle = true;
			return;
		}

		bool isCircle(void) const {
			return _circle;
		}

		OctPoint& start(void) {
			return _pos1;
		}

		OctPoint& close(void) {
			return _pos2;
		}

		OctPoint& center(void) {
			return OctPoint((_pos1._x + _pos2._x) / 2.0f, (_pos1._y + _pos2._y) / 2.0f);
		}

		float radius(bool horz = true) {
			if (horz) {
				return (float)fabs(_pos2._x - _pos1._x) / 2.0f;
			}
			else {
				return (float)fabs(_pos2._y - _pos1._y) / 2.0f;
			}
		}

		float width(void) {
			return (float)(radius(true) * 2.0f);
		}

		float height(void) {
			return (float)(radius(false) * 2.0f);
		}

		bool isHorizontalLine(void) {
			return (_pos2._y == _pos1._y);
		}

		bool isVerticalLine(void) {
			return (_pos2._x == _pos1._x);
		}
		
		void shift(float x, float y) {
			_pos1 = _pos1 + OctPoint(x, y);
			_pos2 = _pos2 + OctPoint(x, y);
			return;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<Route> " : "");
			ss << "start: " << this->_pos1.text(false) << ", close: " << this->_pos2.text(false);
			ss << ", circle: " << this->_circle;
			return ss.str();
		}
	};


	struct OctRouteN
	{
	public:
		bool circle;
		OctPointN pos1;
		OctPointN pos2;
		OctPointN center;
		int radius;

	public:
		void setLine(int _x1, int _y1, int _x2, int _y2) {
			pos1.x = _x1;
			pos1.y = _y1;
			pos2.x = _x2;
			pos2.y = _y2;
			circle = false;
			return;
		}

		void setCircle(int _x1, int _y1, int _radius) {
			center.x = _x1;
			center.y = _y1;
			radius = _radius;
			circle = true;
			return;
		}

		bool isCircle(void) {
			return circle;
		}

		OctPointN& getStartPoint(void) {
			return pos1;
		}

		OctPointN& getEndPoint(void) {
			return pos2;
		}

		OctPointN& getCenterPoint(void) {
			return center;
		}

		int getRadius(void) {
			return radius;
		}

		void addOffset(int _x, int _y) {
			if (circle) {
				center.x += _x;
				center.y += _y;
			}
			else {
				pos1.x += _x;
				pos1.y += _y;
				pos2.x += _x;
				pos2.y += _y;
			}
			return;
		}
	};


	struct OctRange : OctRoute
	{
	public:
		bool _plane ;

	public:
		OctRange() : _plane(true) {
		}

		OctRange(float x1, float y1, float x2, float y2) {
			setPlane(x1, y1, x2, y2);
		}

		void setPlane(float x1, float y1, float x2, float y2) {
			setLine(x1, y1, x2, y2);
			_plane = true;
			return;
		}

		bool hitTest(float xPos, float yPos, float offset) 
		{
			float x1, x2, y1, y2;

			if (isHorizontalLine()) {
				y1 = start()._y - offset;
				y2 = close()._y + offset;
				if (start()._x < close()._x) {
					x1 = start()._x;
					x2 = close()._x;
				}
				else {
					x1 = close()._x;
					x2 = start()._x;
				}
				if (x1 <= xPos && xPos <= x2 && y1 <= yPos && yPos <= y2) {
					return true;
				}
			}
			else if (isVerticalLine()) {
				x1 = start()._x - offset;
				x2 = close()._x + offset;
				if (start()._y < close()._y) {
					y1 = start()._y;
					y2 = close()._y;
				}
				else {
					y1 = close()._y;
					y2 = start()._y;
				}
				if (x1 <= xPos && xPos <= x2 && y1 <= yPos && yPos <= y2) {
					return true;
				}
			}
			else {
				x1 = start()._x;
				y1 = start()._y;
				x2 = close()._x;
				y2 = close()._y;

				float a = (y2 - y1);
				float b = (x2 - x1) * -1.0f;
				float c = (a * x1 + b * y1) * -1.0f;
				float dist = (float)(fabs(a * xPos + b * yPos + c) / sqrt(a*a + b*b));
				if (dist <= offset) {
					return true;
				}
			}
			return false;
		}

		std::string text(bool type = true) {
			std::ostringstream ss;
			ss << (type ? "<Range> " : "");
			ss << "start: " << this->_pos1.text(false) << ", close: " << this->_pos2.text(false) ;
			return ss.str();
		}
	};

}
