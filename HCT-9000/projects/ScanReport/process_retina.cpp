#include "pch.h"
#include "process_retina.h"
#include "parse_options.h"
#include "ScanReport.h"

using namespace Gdiplus;

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>


void segment_retina_scans(ScanOptions opts)
{
    std::cout << "Segmenting retina scans..." << std::endl;
    std::vector<std::string> bscan_image_paths;

    for (int i = 0; i < opts.num_bscan; ++i) {
        std::ostringstream oss;
        oss << opts.dir_path;
        if (!opts.dir_path.empty() && opts.dir_path.back() != '\\') {
            oss << "\\";
        }
        oss << std::setw(3) << std::setfill('0') << i << ".jpg";
        bscan_image_paths.push_back(oss.str());
        std::cout << "B-scan image path: " << oss.str() << std::endl;
    }

    OctSystem::Analysis::clearAllScanData();
    OctSystem::Analysis::clearAllReports();

     std::string output_dir = opts.dir_path;
    if (!output_dir.empty() && output_dir.back() != '\\') {
        output_dir += "\\";
    }
    // output_dir += "segm_out_old";
    output_dir += "segm_new";

    CreateDirectoryA(output_dir.c_str(), NULL);
    std::cout << "Output directory created/checked: " << output_dir << std::endl;

    auto name = opts.getPatternName();
    // name = OctGlobal::PatternName::MacularAngio;
    // name = OctGlobal::PatternName::Disc3D;
    name = OctGlobal::PatternName::Macular3D;
    if (opts.isAngioPattern() || opts.isCubePattern()) {
        auto data = OctSystem::Analysis::obtainCubeScanData();
        data->getDescript().setup(name, opts.num_ascan, opts.num_bscan, opts.scan_range, opts.scan_range);

        std::cout << "Importing pattern images from " << opts.dir_path << std::endl;
        std::wstring wdir_path(opts.dir_path.begin(), opts.dir_path.end());
        data->importPatternImages(wdir_path);
        data->importPreviewImages(wdir_path);

        std::cout << "Doing segmentation..." << std::endl;
        OctSystem::Analysis::doSegmentation(data);

        for (int i = 0; i < data->countBscanData(); i++) {
            auto bscan = data->getBscanData(i);
            auto image = bscan->getBsegmSource();
            auto bdraw = image->getBitmap();
            auto width = image->getWidth();
            auto height = image->getHeight();

            auto inner = bscan->getLayerPoints(OcularLayerType::INNER, width, height);
            for (int j = 0; j < inner.size(); j++) {
                if (inner[j] >= 0) {
                    bdraw->SetPixel(j, inner[j], Gdiplus::Color(0, 0, 255));
                    bdraw->SetPixel(j, inner[j] + 1, Gdiplus::Color(0, 0, 255));
                }
            }
            auto outer = bscan->getLayerPoints(OcularLayerType::OUTER, width, height);
            for (int j = 0; j < outer.size(); j++) {
                if (outer[j] >= 0) {
                    bdraw->SetPixel(j, outer[j], Gdiplus::Color(255, 0, 0));
                    bdraw->SetPixel(j, outer[j] + 1, Gdiplus::Color(255, 0, 0));
                }
            } 

            auto ilms = bscan->getLayerPoints(OcularLayerType::ILM, width, height);
            for (int j = 0; j < ilms.size(); j++) {
                if (ilms[j] >= 0) {
                    bdraw->SetPixel(j, ilms[j], Gdiplus::Color(0, 255, 0));
                    bdraw->SetPixel(j, ilms[j] + 1, Gdiplus::Color(0, 255, 0));
                }
            }

            auto nfls = bscan->getLayerPoints(OcularLayerType::NFL, width, height);
            for (int j = 0; j < nfls.size(); j++) {
                if (nfls[j] >= 0) {
                    bdraw->SetPixel(j, nfls[j], Gdiplus::Color(255, 0, 255));
                    bdraw->SetPixel(j, nfls[j] + 1, Gdiplus::Color(255, 0, 255));
                }
            }

            auto ipls = bscan->getLayerPoints(OcularLayerType::IPL, width, height);
            for (int j = 0; j < ipls.size(); j++) {
                if (ipls[j] >= 0) {
                    bdraw->SetPixel(j, ipls[j], Gdiplus::Color(255, 255, 0));
                    bdraw->SetPixel(j, ipls[j] + 1, Gdiplus::Color(255, 255, 0));
                }
            }
            auto opls = bscan->getLayerPoints(OcularLayerType::OPL, width, height);
            for (int j = 0; j < opls.size(); j++) {
                if (opls[j] >= 0) {
                    bdraw->SetPixel(j, opls[j], Gdiplus::Color(0, 192, 255));
                    bdraw->SetPixel(j, opls[j] + 1, Gdiplus::Color(0, 192, 255));
                }
            }

            auto ioss = bscan->getLayerPoints(OcularLayerType::IOS, width, height);
            for (int j = 0; j < ioss.size(); j++) {
                if (ioss[j] >= 0) {
                    bdraw->SetPixel(j, ioss[j], Gdiplus::Color(0, 255, 160));
                    bdraw->SetPixel(j, ioss[j] + 1, Gdiplus::Color(0, 255, 160));
                }
            }

            auto rpes = bscan->getLayerPoints(OcularLayerType::RPE, width, height);
            for (int j = 0; j < rpes.size(); j++) {
                if (rpes[j] >= 0) {
                    bdraw->SetPixel(j, rpes[j], Gdiplus::Color(255, 160, 0));
                    bdraw->SetPixel(j, rpes[j] + 1, Gdiplus::Color(255, 160, 0));
                }
            }

            auto brm = bscan->getLayerPoints(OcularLayerType::BRM, width, height);
            for (int j = 0; j < brm.size(); j++) {
                if (brm[j] >= 0) {
                    bdraw->SetPixel(j, brm[j], Gdiplus::Color(160, 0, 255));
                    bdraw->SetPixel(j, brm[j] + 1, Gdiplus::Color(160, 0, 255));
                }
            }
            
             // Save the annotated image as a JPG file
             std::wstringstream save_path;
             save_path << std::wstring(output_dir.begin(), output_dir.end());
             save_path << L"\\bscan_";
             save_path << std::setfill(L'0') << std::setw(3) << i << L".jpg";
 
             CLSID jpgClsid;
             {
                 UINT num = 0, size = 0;
                 Gdiplus::GetImageEncodersSize(&num, &size);
                 if (size > 0) {
                     std::vector<BYTE> buffer(size);
                     Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(buffer.data());
                     if (Gdiplus::GetImageEncoders(num, size, pImageCodecInfo) == Ok) {
                         for (UINT j = 0; j < num; ++j) {
                             if (wcscmp(pImageCodecInfo[j].MimeType, L"image/jpeg") == 0) {
                                 jpgClsid = pImageCodecInfo[j].Clsid;
                                 break;
                             }
                         }
                     }
                 }
             }
             bdraw->Save(save_path.str().c_str(), &jpgClsid, NULL);
        }
    }
    return;
}