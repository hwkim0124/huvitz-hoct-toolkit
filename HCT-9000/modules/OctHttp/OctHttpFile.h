#pragma once

#include <string>
#include <ctime>

#include "OctHttpSetup.h"

#ifdef __OCTHTTP_FILE__
#define OctHttpFileExport   __declspec( dllexport )  
#else
#define OctHttpFileExport   __declspec( dllimport )  
#endif

namespace octhttp {

    class OctHttpFileExport File
    {
    public:
        File(int id);
        ~File();

        int post(const std::wstring& filePath);
		int post2(const std::wstring& filePath, const std::wstring& fileName);
        void debug();

    private:
        int examId;
    };

};  // namespace octhttp