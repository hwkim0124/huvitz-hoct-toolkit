// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// PatientDB.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"



#ifdef _DEBUG
#pragma comment(lib, "./sqlite3/sqlite3d.lib")
#else
#pragma comment(lib, "./sqlite3/sqlite3.lib")
#endif
