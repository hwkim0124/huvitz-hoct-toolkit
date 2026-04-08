#pragma once

#ifdef __PATIENTDB_DLL
#define PATIENTDB_DLL_API		__declspec(dllexport)
#else
#define PATIENTDB_DLL_API		__declspec(dllimport)
#endif


namespace PatientDB
{
	enum
	{
		INVALID_INDEX = 0,
	};

	enum class Status
	{
		UNKNOWN = -1,
		NOUSE = 0,
		USE = 1,
	};

	enum class Direction
	{
		UNKNOWN = -1,
		HORZ = 0,
		VERT = 1,
	};

//	enum class EyeSide
//	{
//		UNKNOWN = 0,
//		OD = 1,
//		OS = 2,
//	};

	enum class Gender
	{
		UNKNOWN = 0,
		MALE = 1,
		FEMALE = 2,
	};

	enum class ScanType
	{
		INVALID = -1,
		MARCULAR = 0,
		OPTICDISC = 1,
		ANTERIOR = 2,
	};

	enum class Permission
	{
		NOUSE = 0,
		SEARCH = 1,
		INSERT = 2,
		ALL = 3,
		ADMIN = 4,
	};
}