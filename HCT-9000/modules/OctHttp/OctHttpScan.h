#pragma once

#include <string>
#include <ctime>
#include <vector>
#include <tuple>
#include <utility>

#include "OctHttpSetup.h"

#ifdef __OCTHTTP_Scan__
#define OctHttpScanExport   __declspec( dllexport )  
#else
#define OctHttpScanExport   __declspec( dllimport )  
#endif

namespace octhttp {

    class OctHttpScanExport Scan
    {
    public:
        Scan(MeasurementType pat, EyeSide es);
        ~Scan();

        void setPatientId(const std::wstring &pid);
        void setDeviceId(const std::wstring &devid);
		void setMeasureTime(const ATL::COleDateTime &mt);
		void setFixation(unsigned short fx);
        void setSsi(int si);
        void setSqi(int qi);
        void setAscans(unsigned short as);
        void setBscans(unsigned short bs);
        void setScanWidth(float w);
        void setScanHeight(float h);
        void setScanDirection(unsigned short sd);
        void setXoffset(float xoff);
        void setYoffset(float yoff);
        void setBaseline(int bs);
        void setCenter(std::pair<double, double> &c);
        void setDiscArea(std::vector<std::tuple<int, int, int>> &v);
        void setCupArea(std::vector<std::tuple<int, int, int>> &v);

        int post();
        int process(int id);
		int cancel(int id);
        void debug();

    private:
        MeasurementType pattern;
        EyeSide eyeSide;
        std::wstring patientId;
        std::wstring deviceId;
		ATL::COleDateTime measureTime;
        unsigned short fixation;
        int ssi;
        int sqi;
        unsigned short aScans;
        unsigned short bScans;
        float scanWidth;
        float scanHeight;
        unsigned short scanDirection;
        float xOffset;
        float yOffset;
        int baseline;
        std::pair<double , double> center; // <cx, cy> : fovea or optic disc center
        std::vector<std::tuple<int, int, int>> discBorders; // <y, x0, x1>
        std::vector<std::tuple<int, int, int>> cupBorders;  // <y, x0, x1>


    };

};  // namespace octhttp