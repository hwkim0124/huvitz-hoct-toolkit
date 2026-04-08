#pragma once


#include "OctGlobalDef.h"
#include "GlobalFundusDef.h"
#include "GlobalMeasureDef.h"

#include <cstdint>

namespace OctGlobal
{
	struct OctFundusMeasure
	{
		EyeSide _side;

	public:
		OctFundusMeasure() {
			init();
		}

		void init(void) {
			_side = EyeSide::OD;
		}

		EyeSide& eyeSide(void) {
			return _side;
		}

		bool isOD(void) const {
			return (EyeSide::OD == _side);
		}

		std::string getLabel(void) {
			std::string label;
			label = (isOD() ? "OD" : "OS");
			// label += "_";
			// label += "Normal";
			return label;
		}
	};



}
