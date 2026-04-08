#pragma once

#ifdef __OCTHTTP_LIB__
#define OctHttpExport   __declspec( dllexport )  
#else
#define OctHttpExport   __declspec( dllimport )  
#endif

#include <cpprest/json.h>

namespace octhttp {
    class OctHttpExport OctHttp {
    private:
        OctHttp() {};
        OctHttp(const OctHttp& other);
        ~OctHttp() {};

        static OctHttp *instance;
        static std::wstring ipaddr;
        static unsigned short port;
		static unsigned short timeout;

    public:
        static OctHttp& getInstance();
        static void setIpAddress(const std::wstring& ip);
        static void setPort(const unsigned short p);
		static void setTimeout(const unsigned short secs);
		static unsigned short getTimeout(void);

        int get(const std::wstring& path);
        int postJson(const std::wstring& path, web::json::value& j);
        int putJson(const std::wstring& path, web::json::value& j);
        int postJsonDump(web::json::value& j);
        int postFile(const std::wstring& path, const std::wstring& filename);
		int postFile2(const std::wstring& path, const std::wstring& filename, const std::wstring& customFileName);
    };

}  // namespace octhttp




