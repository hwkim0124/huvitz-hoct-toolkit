#pragma once

#include <string>
#include <ctime>

#include "OctHttpSetup.h"

#ifdef __OCTHTTP_FUNDUS__
#define OctHttpFundusExport   __declspec( dllexport )  
#else
#define OctHttpFundusExport   __declspec( dllimport )  
#endif

namespace octhttp {

    class OctHttpFundusExport Fundus
    {
    public:
        Fundus(MeasurementType pat, EyeSide es);
        ~Fundus();

        void setPatientId(const std::wstring &pid);
		void setDeviceId(const std::wstring &devid);
		void setMeasureTime(const ATL::COleDateTime &mt);
		void setFixation(unsigned short fx);
		void setFlashLevel(unsigned short v);
		void setShutterSpeed(unsigned short v);
		void setIso(unsigned short v);
		void setPanorama(unsigned short v);
        void setStereo(unsigned short v);

        int post();
        int process(int id);
        void debug();

    private:
        MeasurementType pattern;
        EyeSide eyeSide;
        std::wstring patientId;
		std::wstring deviceId;
		ATL::COleDateTime measureTime;
        unsigned short fixation;
        unsigned short flashLevel;
        unsigned short shutterSpeed;
        unsigned short iso;
        unsigned short panorama;
        unsigned short stereo;

    };

};  // namespace octhttp