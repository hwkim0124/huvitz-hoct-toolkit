#pragma once

#include "OctGlobalDef.h"

#include <cstdint>
#include <string>


namespace OctGlobal
{
	struct OctPatient
	{
	public:
		std::string _id = "";
		std::string _firstName = "";
		std::string _middleName = "";
		std::string _lastName = "";

		std::string _birthDate = "";
		std::uint16_t _age = 0;
		Gender _gender = Gender::UNKNOWN;

		float _diopterOd = 0.0f;
		float _diopterOs = 0.0f;

	public:
		void setup(std::string id, std::string fName, std::string mName, std::string lName,
				std::string birth, std::uint16_t age, Gender gender, float dioptOd = 0.0f, float dioptOs = 0.0f) {
			_id = id;
			_firstName = fName;
			_middleName = mName;
			_lastName = lName;
			_birthDate = birth;
			_age = age;
			_gender = gender;
			_diopterOd = dioptOd;
			_diopterOs = dioptOs;
			return;
		}

		void init(void) {
			setup("", "", "", "", "", 0, Gender::UNKNOWN, 0.0f, 0.0f);
		}

		float diopterOD(void) {
			return _diopterOd;
		}

		float diopterOS(void) {
			return _diopterOs;
		}

		void setDiopter(float dioptOd, float dioptOs) {
			_diopterOd = dioptOd;
			_diopterOs = dioptOs;
			return;
		}
	};
}