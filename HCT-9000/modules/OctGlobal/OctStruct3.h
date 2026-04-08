#pragma once

#include "OctGlobalDef.h"

#include <cstdint>
#include <string>


namespace OctGlobal
{
	struct OctPatient
	{
	public:
		std::string id = "";
		std::string firstName = "";
		std::string middleName = "";
		std::string lastName = "";

		std::string birthDate = "";
		std::uint16_t age = 0;
		Gender gender = Gender::UNKNOWN;

		float diopterOd = 0.0f;
		float diopterOs = 0.0f;

	public:
		void set(std::string id_, std::string fName_, std::string mName_, std::string lName_,
				std::string birth_, std::uint16_t age_, Gender gender_, float dioptOd_, float dioptOs_) {
			id = id_;
			firstName = fName_;
			middleName = mName_;
			lastName = lName_;
			birthDate = birth_;
			age = age_;
			gender = gender_;
			diopterOd = dioptOd_;
			diopterOs = dioptOs_;
			return;
		}

	};
}