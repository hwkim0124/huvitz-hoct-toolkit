#pragma once

#include "OctGlobalDef.h"


namespace OctGlobal
{
	// Internal Fixation
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int FIXATION_CENTER_ROW = 8;
	constexpr int FIXATION_CENTER_COL = 4;

	constexpr int FIXATION_ROW_OD_MACULAR = FIXATION_CENTER_ROW;
	constexpr int FIXATION_COL_OD_MACULAR = FIXATION_CENTER_COL;
	constexpr int FIXATION_ROW_OD_DISC = (FIXATION_CENTER_ROW-3);
	constexpr int FIXATION_COL_OD_DISC = FIXATION_CENTER_COL;
	constexpr int FIXATION_ROW_OD_FUNDUS = (FIXATION_CENTER_ROW - 1);
	constexpr int FIXATION_COL_OD_FUNDUS = FIXATION_CENTER_COL;

	constexpr int FIXATION_ROW_OS_MACULAR = FIXATION_CENTER_ROW;
	constexpr int FIXATION_COL_OS_MACULAR = FIXATION_CENTER_COL;
	constexpr int FIXATION_ROW_OS_DISC = (FIXATION_CENTER_ROW+3);
	constexpr int FIXATION_COL_OS_DISC = FIXATION_CENTER_COL;
	constexpr int FIXATION_ROW_OS_FUNDUS = (FIXATION_CENTER_ROW + 1);
	constexpr int FIXATION_COL_OS_FUNDUS = FIXATION_CENTER_COL;

	
	enum class EyeSide {
		Unknown = 0,
		OD,
		OS, 
		BOTH,
		CMP1,  // compare left
		CMP2   // compare right
	};

	enum class EyeRegion {
		Unknown = 0,
		Macular,
		OpticDisc,
		Cornea, 
		Fundus
	};

	enum class FixationTarget {
		Center,
		Fundus, 
		OpticDisc,
		LeftSide,
		LeftUp,
		LeftDown,
		RightSide,
		RightUp,
		RightDown
	};

	enum class CompensationLensMode {
		Zero = 0, 
		Minus, 
		Plus
	};

	enum class ScanSpeed
	{
		Unknown = 0,
		Normal, 
		Faster, 
		Fastest, 
		Custom
	};

}