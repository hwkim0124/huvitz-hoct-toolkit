#pragma once

#include <string>

#include "OctHttpSetup.h"

#ifdef __OCTHTTP_PATIENT__
#define OctHttpPatientExport   __declspec( dllexport )  
#else
#define OctHttpPatientExport   __declspec( dllimport )  
#endif
namespace octhttp {

    class OctHttpPatientExport Patient
    {
    public:
        Patient(const std::wstring &id);
        ~Patient();

        void setFirstName(const std::wstring &fn);
        void setMiddleName(const std::wstring &mn);
        void setLastName(const std::wstring &ln);
        void setGender(Gender g);
        void setBirthDate(unsigned int yyyy, unsigned int mm, unsigned int dd);
        void setRefract(float od, float os);
        void setRace(Race r);
        void setPhysicianName(const std::wstring &pname);
        void setOperatorName(const std::wstring &oname);
        void setDescription(const std::wstring &desc);
		void setPatientId(const std::wstring &pid);
		void setModality(const std::wstring & value);
		void setAccessionNumber(const std::wstring & value);

        int postJson();
        int update();
        void debug();

    private:
		std::wstring key;
        std::wstring patientId;
        std::wstring firstName;
        std::wstring middleName;
        std::wstring lastName;
        Gender gender;
        std::wstring birthDate;
        float refractOs;
        float refractOd;
        Race race;
        std::wstring physicianName;
        std::wstring operatorName;
        std::wstring description;
		std::wstring modality;
		std::wstring accessionNumber;
    };

};  // namespace octhttp