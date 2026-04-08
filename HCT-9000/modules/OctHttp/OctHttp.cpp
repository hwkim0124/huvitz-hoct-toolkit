// OctHttp.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <utility>
#include <chrono>
#include <thread>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/containerstream.h>

#define __OCTHTTP_LIB__
#include "octhttp.h"

using namespace octhttp;

OctHttp* OctHttp::instance = nullptr;
std::wstring OctHttp::ipaddr = L"";
unsigned short OctHttp::port = 8080;
unsigned short OctHttp::timeout = 5;


OctHttp::OctHttp(const OctHttp& other)
{
}


OctHttp& OctHttp::getInstance()
{
    if (instance == nullptr) {
        instance = new OctHttp();
    }

    return *instance;
}

void OctHttp::setIpAddress(const std::wstring& ip)
{
    if (ip.compare(0, 7, L"http://") == 0) {
        ipaddr = ip;
    }
    else {
        ipaddr = L"http://" + ip;
    }
}

void OctHttp::setPort(const unsigned short p)
{
    port = p;
}


void octhttp::OctHttp::setTimeout(const unsigned short secs)
{
	OctHttp::timeout = secs;
	return;
}


unsigned short octhttp::OctHttp::getTimeout(void)
{
	return OctHttp::timeout;
}


using namespace ::pplx;
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

static http_client_config _clientConfig;


static const http_client_config& getRequestConfig(void) 
{
	_clientConfig.set_timeout(std::chrono::seconds(OctHttp::getTimeout()));
	return _clientConfig;
}


int OctHttp::get(const std::wstring& path)
{
    std::wstring url(ipaddr + L":" + std::to_wstring(port));

    http_client client(url, getRequestConfig());
    auto contentType = utility::string_t(L"application/json");

    http_response response;
    try {
        response = client.request(methods::GET, path).get();
    }
    catch (http_exception const& e) {
        return -1;
    }

    return 0;
}

int OctHttp::postJsonDump(web::json::value& j)
{
    for (auto iter = j.as_object().cbegin(); iter != j.as_object().cend(); ++iter) {
        const utility::string_t &str = iter->first;
        const web::json::value &v = iter->second;

        std::wcout << L"[JSON] " << str << L" : " << v.serialize() << std::endl;
    }
    return 0;
}

pplx::task<int> postJsonAsync(const std::wstring& url, const std::wstring& path, web::json::value& j)
{
    http_client client(url, getRequestConfig());
    auto contentType = utility::string_t(L"application/json");

    utility::stringstream_t stream;
    j.serialize(stream);

    return client.request(methods::POST, path, stream.str(), contentType)
        .then([](http_response response) -> pplx::task<web::json::value>
    {
        std::wcout << L"postJsonAsync() : response.status_code = " << response.status_code() << std::endl;
        if (response.status_code() == status_codes::OK) {
            return response.extract_json();
        }

        std::wcout << L"postJsonAsync() : status_code NOT OK" << std::endl;
        return pplx::task_from_result(web::json::value());
    })
        .then([](pplx::task<web::json::value> prevTask) -> pplx::task<int>
    {
        try {
            web::json::value const & v = prevTask.get();
            int count;
            if (!v.is_null()) {
                count = _wtoi(v.at(L"SerialId").as_string().c_str());
            }
            else {
                count = -1;
            }
            return pplx::task<int>([count]() -> int {
                return count;
            });
        }
        catch (http_exception const & e) {
            std::wcout << e.what() << std::endl;
        }
    });
}

// network error 만 exception 으로 처리한다.
int postJsonSync(const std::wstring& url, const std::wstring& path, web::json::value& j)
{
    http_client client(url, getRequestConfig());
    auto contentType = utility::string_t(L"application/json");

    utility::stringstream_t stream;
    j.serialize(stream);

    http_response response;
    try {
        response = client.request(methods::POST, path, stream.str(), contentType).get();
    }
    catch (http_exception const& e) {
        throw std::runtime_error(e.what());
    }

    auto stcode = response.status_code();
    if (stcode != status_codes::OK) {
		return -1; // handle with return value if not network error.
    }

    web::json::value const & retJson = response.extract_json().get();
    if (!retJson.is_null()) {
        int count = _wtoi(retJson.at(L"SerialId").as_string().c_str());
		// Added new exception, refer to https://msdn.microsoft.com/en-us/library/yd5xkb5c.aspx
		// - The return value is 0 for atoi and _wtoi, if the input cannot be converted to a value of that type.
		if (count == 0) {
			return -1; // handle with return value if not network error.
		}
		else {
			return count;
		}
    }
    else {
		return -1; // handle with return value if not network error.
    }
}

int OctHttp::postJson(const std::wstring& query, web::json::value& j)
{
    std::wstring url(ipaddr + L":" + std::to_wstring(port));
    try {
        auto ret = postJsonSync(url, query, j);
        return ret;
    }
    catch (std::exception const& e) {
        throw e;
    }

}

int putJsonSync(const std::wstring& url, const std::wstring& path, web::json::value& j)
{
    http_client client(url, getRequestConfig());
    auto contentType = utility::string_t(L"application/json");

    utility::stringstream_t stream;
    j.serialize(stream);

    http_response response;
    try {
        response = client.request(methods::PUT, path, stream.str(), contentType).get();
    }
    catch (http_exception const& e) {
        throw std::runtime_error(e.what());
    }

    auto stcode = response.status_code();
    if (stcode != status_codes::OK) {
        throw std::runtime_error("Http response not OK");
    }

    web::json::value const & retJson = response.extract_json().get();
    if (!retJson.is_null()) {
        int count = _wtoi(retJson.at(L"SerialId").as_string().c_str());
        return count;
    }
    else {
        throw std::runtime_error("Json value is null");
    }

}

int OctHttp::putJson(const std::wstring& query, web::json::value& j)
{
    std::wstring url(ipaddr + L":" + std::to_wstring(port));
    try {
        auto ret = putJsonSync(url, query, j);
        return ret;
    }
    catch (std::exception const& e) {
        throw e;
    }

}


task<void> uploadFile(const std::wstring& url, const std::wstring& query, const utility::string_t &filePath)
{
    using concurrency::streams::file_stream;
    using concurrency::streams::basic_istream;

    return file_stream<unsigned char>::open_istream(filePath)
        .then([url,query](pplx::task<basic_istream<unsigned char>> previousTask)
    {
        try
        {
            auto fileStream = previousTask.get();

            fileStream.seek(0, std::ios::end);
            auto length = static_cast<size_t>(fileStream.tell());
            fileStream.seek(0, 0);

            http_request req;
            req.set_body(fileStream, length);
            req.set_method(methods::POST);

            http_client client(url, getRequestConfig());
            return client.request(methods::POST, query, fileStream.streambuf())
                .then([fileStream](pplx::task<http_response> previousTask)
            {
                fileStream.close();
                std::wostringstream ss;
                try {
                    auto response = previousTask.get();
                    auto body = response.body();
                }
                catch (const http_exception& e) {
                    ss << e.what() << std::endl;
                    std::wcout << ss.str();
                }
            });
        }
        catch (const std::system_error& e)
        {
            std::cout << L"2" << e.what() << std::endl;
            // Return an empty task
            return pplx::task_from_result();
        }
    });
}

int OctHttp::postFile(const std::wstring& path, const std::wstring& filePath)
{
    auto fileName = filePath.substr(filePath.find_last_of(L"\\") + 1);

    std::wstring url(ipaddr + L":" + std::to_wstring(port));
    std::wstring query(path + L"/" + fileName);

    auto ret = uploadFile(url, query, filePath);
    ret.wait();

    return 0;
}
int OctHttp::postFile2(const std::wstring& path, const std::wstring& filePath, const std::wstring& customFileName)
{
	std::wstring url(ipaddr + L":" + std::to_wstring(port));
	std::wstring query(path + L"/" + customFileName);

	auto ret = uploadFile(url, query, filePath);
	ret.wait();

	return 0;
}

