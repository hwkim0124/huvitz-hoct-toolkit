#pragma once

#include "OctGlobalDef.h"


namespace OctGlobal
{
	struct OCTGLOBAL_DLL_API OctPoint
	{
	public:
		float x;
		float y;

	public:
		OctPoint() : x(0.0f), y(0.0f) {
		}

		OctPoint(float x_, float y_) {
			x = x_;
			y = y_;
		}

		void set(float x_, float y_) {
			x = x_;
			y = y_;
		}

		bool operator==(const OctPoint& rhs) const {
			return (x == rhs.x && y == rhs.y);
		}

		OctPoint& operator=(const OctPoint& rhs) {
			x = rhs.x;
			y = rhs.y;
			return *this;
		}

		OctPoint operator+(const OctPoint& rhs) const {
			return OctPoint(x + rhs.x, y + rhs.y);
		}
	};


	struct OCTGLOBAL_DLL_API OctPointN
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


	struct OCTGLOBAL_DLL_API OctRoute
	{
	public:
		bool circle;
		OctPoint pos1;
		OctPoint pos2;
		OctPoint center;
		float radius;

	public:
		void setLine(float _x1, float _y1, float _x2, float _y2) {
			pos1.x = _x1;
			pos1.y = _y1;
			pos2.x = _x2;
			pos2.y = _y2;
			circle = false;
			return;
		}

		void setCircle(float _x1, float _y1, float _radius) {
			center.x = _x1;
			center.y = _y1;
			radius = _radius;
			circle = true;
			return;
		}

		bool isCircle(void) {
			return circle;
		}

		OctPoint& getStartPoint(void) {
			return pos1;
		}

		OctPoint& getEndPoint(void) {
			return pos2;
		}

		OctPoint& getCenterPoint(void) {
			return center;
		}

		float getRadius(void) {
			return radius;
		}
		
		void addOffset(float _x, float _y) {
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


	struct OCTGLOBAL_DLL_API OctRouteN
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


	struct OCTGLOBAL_DLL_API OctRange : public OctRoute
	{
	public:
		bool plane = false;

	public:
		void setPlane(float _x1, float _y1, float _x2, float _y2) {
			setLine(_x1, _y1, _x2, _y2);
			plane = true;
			return;
		}

		float getWidth(void) {
			return (float)fabs(getEndPoint().x - getStartPoint().x);
		}

		float getHeight(void) {
			return (float)fabs(getEndPoint().y - getStartPoint().y);
		}
		
		bool isHorizontalLine(void) {
			return (getEndPoint().y == getStartPoint().y);
		}

		bool isVerticalLine(void) {
			return (getEndPoint().x == getStartPoint().x);
		}


		bool hitTest(float xPos, float yPos, float offset) 
		{
			float x1, x2, y1, y2;

			if (isHorizontalLine()) {
				y1 = getStartPoint().y - offset;
				y2 = getEndPoint().y + offset;
				if (getStartPoint().x < getEndPoint().x) {
					x1 = getStartPoint().x;
					x2 = getEndPoint().x;
				}
				else {
					x1 = getEndPoint().x;
					x2 = getStartPoint().x;
				}
				if (x1 <= xPos && xPos <= x2 && y1 <= yPos && yPos <= y2) {
					return true;
				}
			}
			else if (isVerticalLine()) {
				x1 = getStartPoint().x - offset;
				x2 = getEndPoint().x + offset;
				if (getStartPoint().y < getEndPoint().y) {
					y1 = getStartPoint().y;
					y2 = getEndPoint().y;
				}
				else {
					y1 = getEndPoint().y;
					y2 = getStartPoint().y;
				}
				if (x1 <= xPos && xPos <= x2 && y1 <= yPos && yPos <= y2) {
					return true;
				}
			}
			else if (isCircle()) {

			}
			else {
				x1 = getStartPoint().x;
				y1 = getStartPoint().y;
				x2 = getEndPoint().x;
				y2 = getEndPoint().y;

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
	};

}
