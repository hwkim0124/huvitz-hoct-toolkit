#pragma once

#include "OctGlobalDef.h"

namespace OctGlobal
{
	struct OctRect
	{
		int _sx;
		int _sy;
		int _ex;
		int _ey;

		OctRect() : _sx(0), _sy(0), _ex(0), _ey(0) {
		}

		OctRect(int sx, int sy, int ex, int ey) {
			_sx = sx; 
			_sy = sy;
			_ex = ex;
			_ey = ey;
		}

		bool enlargeToHorzLine(int sx, int ex, int y) {
			if (((_sy - 1) == y || (_ey + 1) == y) && 
				(_sx >= sx && _sx <= ex) && (_ex >= sx && _ex <= ex)) {
				_sy = (_sy > y ? y : _sy) ;
				_ey = (_ey < y ? y : _ey);
				_sx = (_sx > sx ? sx : _sx);
				_ex = (_ex < ex ? ex : _ex);
				return true;
			}
			return false;
		}
	};
}