// ScanReport.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "framework.h"
#include "ScanReport.h"

#include "parse_options.h"
#include "process_retina.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: code your application's behavior here.
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // Pass struct to function to parse options based on argc/argv
            ScanOptions opts = ParseScanOptions(__argc, __argv);

            // Print parsed options for confirmation
            std::cout << "Directory: " << opts.dir_path << std::endl;
            std::cout << "Number of B-scan: " << opts.num_bscan << std::endl;
            std::cout << "Number of A-scan points: " << opts.num_ascan << std::endl;
            std::cout << "B-scan height: " << opts.bscan_height << std::endl;
            std::cout << "Scan range: " << opts.scan_range << std::endl;
            std::cout << "Scan domain: " << opts.scan_domain << std::endl;
            std::cout << "Pattern type: " << opts.pattern_type << std::endl;

            segment_retina_scans(opts);
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
