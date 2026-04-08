#pragma once

#include <string>
#include <ctime>

#include "ATLComTime.h"

#ifdef __OCTHTTP_SETUP__
#define OctHttpSetupExport   __declspec( dllexport )  
#else
#define OctHttpSetupExport   __declspec( dllimport )  
#endif

namespace octhttp {

    enum class OctHttpSetupExport MeasurementType {
        Macular_Line,
        Macular_Cross,
        Macular_Radial,
        Macular_3D,
        Macular_Raster,
        Disc_Radial,
        Disc_Raster,
        Disc_3D,
        Disc_Circle,
        Anterior_Line,
        Anterior_Radial,
        Anterior_3D,
        Color_Fundus,
		Macular_Disc,
		Macular_Angio,
		Disc_Angio,
		Anterior_Full,
		Topography,
		Axial_Length,
		Lens_Thickness
    };

    enum class OctHttpSetupExport EyeSide {
		UNKNOWN,
		OD,  // right
		OS   // left
    };

    enum class OctHttpSetupExport Gender {
        Male,
        Female
    };

    enum class OctHttpSetupExport Race {
        Asian = 1,
        Caucasian,
        Latino,
        Blacks,
        Indian,
        Arab,
		Other
		//Mixed
    };

    class OctHttpSetupExport OctHttpSetup
    {
    public:
        OctHttpSetup() = delete;
        ~OctHttpSetup() = delete;

        static void setIpAddress(const std::wstring &ip);
        static void setPort(const unsigned short p);

        // utilities for inner use
        static std::wstring patternToString(MeasurementType pat);
        static std::wstring eyeSideToString(EyeSide es);
        static std::wstring genderToString(Gender g);
        static std::wstring raceToString(Race r);
		static std::wstring timeToString(COleDateTime t);
    };

};  // namespace octhttp