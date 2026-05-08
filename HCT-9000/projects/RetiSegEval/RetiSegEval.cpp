// RetiSegEval.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h" 
#include "RetiSegEval.h"
#include "FileUtil.h"

#include <iostream>
#include <boost/format.hpp>

int main()
{
    CString discPath = _T("D:/HOCT-Samples/in_house/260504_ia_fda_pretest/disc_proc");
    CString dataPath = _T("D:/HOCT-Samples/in_house/260504_ia_fda_pretest/hoct_data");

    std::vector<CString> resultFolders = FileUtil::FindResultFolders(dataPath);
    for (const auto& resultFolder : resultFolders) {
        std::wcout << L"Loaded path: " << (LPCTSTR)resultFolder << std::endl;

        CString examName = FileUtil::getExamFolderName(resultFolder);
        CString patiName = FileUtil::getPatientFolderName(resultFolder);

        auto pattern = PatternName::Macular3D;
        auto rangeX = 6.0f;
        auto rangeY = 6.0f;
        bool isWide = false;
        bool isAngio = false;
        bool isMacular = false;
        bool isDisc = false;
        auto numImages = 0;
        auto numPoints = 512;
        auto direction = 0;
        auto eyeSide = EyeSide::OD;

        if (resultFolder.Find(_T("OS")) >= 0) {
            eyeSide = EyeSide::OS;
        }

        numImages = FileUtil::GetSequentialOctImageCount(resultFolder);
        if (numImages >= 256) {
            isAngio = true;
            numPoints = 384;
        }

        CString fileToCheck = _T("macularcenter.json");
        bool exists = FileUtil::FileExistsInFolder(resultFolder, fileToCheck);
        if (exists) {
            isMacular = true;
            rangeX = 9.0f;
            rangeY = 9.0f;
        }
        
        fileToCheck = _T("disccenter.json");
        exists = FileUtil::FileExistsInFolder(resultFolder, fileToCheck);
        if (exists) {
            pattern = PatternName::Disc3D;
            isDisc = true;
            rangeX = 6.0f;
            rangeY = 6.0f;
        }

        if (isMacular && isDisc) {
            pattern = PatternName::Disc3D;
            isWide = true;
            rangeX = 12.0f;
            rangeY = 9.0f;
        }

        if (!isDisc) {
            continue;
        }

        OctSystem::Analysis::clearAllScanData();
        OctSystem::Analysis::clearAllReports();
        auto sdata = OctSystem::Analysis::obtainCubeScanData();
        sdata->getDescript().setup(pattern, numPoints, numImages, rangeX, rangeY, direction);

        sdata->importPatternImages((LPCTSTR)resultFolder);
        sdata->importPreviewImages((LPCTSTR)resultFolder);
        Analysis::doSegmentation(sdata);

        if (isWide) {
            OctSystem::Analysis::obtainMacularReport(0)->setProtocolData(sdata, eyeSide);
            OctSystem::Analysis::obtainDiscReport(0)->setProtocolData(sdata, eyeSide);
        }
        else if (isMacular) {
            OctSystem::Analysis::obtainMacularReport(0)->setProtocolData(sdata, eyeSide);
        }
        else if (isDisc) {
            OctSystem::Analysis::obtainDiscReport(0)->setProtocolData(sdata, eyeSide);
        }

        CString segmOutFolder = resultFolder + _T("\\segm_out");
        if (!CreateDirectory(segmOutFolder, NULL)) {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS) {
                std::wcerr << L"Failed to create segm_out folder: " << (LPCTSTR)segmOutFolder << L" (Error " << err << L")" << std::endl;
            }
        }

        auto pdata = Analysis::getProtocolData(0);
        /*
        for (int i = 0; i < pdata->countBscanData(); i++) {
            auto bscan = pdata->getBscanData(i);
            auto image = bscan->getBsegmSource();
            auto bdraw = image->getBitmap();
            auto width = image->getWidth();
            auto height = image->getHeight();

            auto ilms = bscan->getLayerPoints(OcularLayerType::ILM, numPoints, height);
            for (int j = 0; j < ilms.size(); j++) {
                if (ilms[j] >= 0) {
                    bdraw->SetPixel(j, ilms[j], Gdiplus::Color(0, 255, 255));
                    bdraw->SetPixel(j, ilms[j] + 1, Gdiplus::Color(0, 255, 255));
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
                    bdraw->SetPixel(j, ipls[j], Gdiplus::Color(0, 255, 128));
                    bdraw->SetPixel(j, ipls[j] + 1, Gdiplus::Color(0, 255, 128));
                }
            }
            auto opls = bscan->getLayerPoints(OcularLayerType::OPL, width, height);
            for (int j = 0; j < opls.size(); j++) {
                if (opls[j] >= 0) {
                    bdraw->SetPixel(j, opls[j], Gdiplus::Color(255, 255, 64));
                    bdraw->SetPixel(j, opls[j] + 1, Gdiplus::Color(255, 255, 64));
                }
            }

            auto ioss = bscan->getLayerPoints(OcularLayerType::IOS, width, height);
            for (int j = 0; j < ioss.size(); j++) {
                if (ioss[j] >= 0) {
                    bdraw->SetPixel(j, ioss[j], Gdiplus::Color(64, 192, 255));
                    bdraw->SetPixel(j, ioss[j] + 1, Gdiplus::Color(64, 192, 255));
                }
            }

            auto rpes = bscan->getLayerPoints(OcularLayerType::RPE, width, height);
            for (int j = 0; j < rpes.size(); j++) {
                if (rpes[j] >= 0) {
                    bdraw->SetPixel(j, rpes[j], Gdiplus::Color(255, 192, 64));
                    bdraw->SetPixel(j, rpes[j] + 1, Gdiplus::Color(255, 192, 64));
                }
            }

            auto brm = bscan->getLayerPoints(OcularLayerType::BRM, width, height);
            for (int j = 0; j < brm.size(); j++) {
                if (brm[j] >= 0) {
                    bdraw->SetPixel(j, brm[j], Gdiplus::Color(192, 64, 255));
                    bdraw->SetPixel(j, brm[j] + 1, Gdiplus::Color(192, 64, 255));
                }
            }

            // Compose output file path (assuming 'folder' is the base current folder, and 'i' the B-scan index)
            CString outFileName;
            outFileName.Format(_T("%03d.png"), i); // 'i' must be in scope as B-scan index

            CString outPath = segmOutFolder + _T("\\") + outFileName;

            // Save the Gdiplus bitmap as a PNG file using the utility function in FileUtil
            FileUtil::SaveBitmapToPng(bdraw, outPath);
        }
        */
        /*
        if (isMacular) {
            auto* report = OctSystem::Analysis::obtainMacularReport(0);
            auto enface = report->getMacularEshot(OcularLayerType::ILM, OcularLayerType::RPE);
            auto image = enface->makeImage(300, 300, false);
            auto bdraw = image.getBitmap();
            auto width = image.getWidth();
            auto height = image.getHeight();

            auto center_x = report->getFoveaCenterX();
            auto center_y = report->getFoveaCenterY();
            auto range_hw = report->getDescript()->getScanRangeX() * 0.5f;
            auto range_hh = report->getDescript()->getScanRangeY() * 0.5f;
            auto cx_pos = (int)((1.0f + (center_x / range_hw)) * width * 0.5f);
            auto cy_pos = (int)((1.0f + (center_y / range_hh)) * height * 0.5f);

            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width-1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height-1), Gdiplus::Color(255, 0, 0));
            }

            CString outPath = segmOutFolder + _T("\\") + _T("macular_enface.png");;
            FileUtil::SaveBitmapToPng(bdraw, outPath);

            auto tmap = report->getMacularEplot(OcularLayerType::ILM, OcularLayerType::IPL);
            image = tmap->makeImage(300, 300);
            bdraw = image.getBitmap();
            width = image.getWidth();
            height = image.getHeight();

            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width - 1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height - 1), Gdiplus::Color(255, 0, 0));
            }

            outPath = segmOutFolder + _T("\\") + _T("macular_ilm_ipl.png");;
            FileUtil::SaveBitmapToPng(bdraw, outPath);
        }
        */
        if (isDisc) {
            auto* report = OctSystem::Analysis::obtainDiscReport(0);
            auto enface = report->getDiscEshot(OcularLayerType::IOS, OcularLayerType::BRM);
            auto image = enface->makeImage(300, 300, false);
            auto bdraw = image.getBitmap();
            auto width = image.getWidth();
            auto height = image.getHeight();

            auto center_x = report->getNerveHeadCenterX();
            auto center_y = report->getNerveHeadCenterY();
            auto range_hw = report->getDescript()->getScanRangeX() * 0.5f;
            auto range_hh = report->getDescript()->getScanRangeY() * 0.5f;
            auto cx_pos = (int)((1.0f + (center_x / range_hw)) * width * 0.5f);
            auto cy_pos = (int)((1.0f + (center_y / range_hh)) * height * 0.5f);
            
            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width - 1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height - 1), Gdiplus::Color(255, 0, 0));
            }

            CString outPath = segmOutFolder + _T("\\") + _T("disc_enface.png");;
            // FileUtil::SaveBitmapToPng(bdraw, outPath);
      
            outPath = discPath + _T("\\") + patiName + _T("_") + examName + _T("_enface_fft.png");
            FileUtil::SaveBitmapToPng(bdraw, outPath);

            /*
            auto tmap = report->getDiscEplot(OcularLayerType::ILM, OcularLayerType::NFL);
            image = tmap->makeImage(300, 300);
            bdraw = image.getBitmap();
            width = image.getWidth();
            height = image.getHeight();

            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width - 1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height - 1), Gdiplus::Color(255, 0, 0));
            }
            
            outPath = segmOutFolder + _T("\\") + _T("disc_ilm_nfl.png");;
            FileUtil::SaveBitmapToPng(bdraw, outPath);
            */

            auto iosToBase = report->getDiscEshot(OcularLayerType::IOS, OcularLayerType::BASE);
            image = iosToBase->makeImage(300, 300, false);
            bdraw = image.getBitmap();
            width = image.getWidth();
            height = image.getHeight();

            for (int y = 0; y < pdata->countBscanData(); y++) {
                auto bscan = pdata->getBscanData(y);
                int x1, x2, cx, cy;
                if (bscan->getOpticNerveDiscRange(x1, x2)) {
                    cx = (int)(((float)x1 / (float)numPoints) * width + 0.5f);
                    cy = (int)(((float)y / (float)numImages) * height + 0.5f);
                    for (int k = -3; k <= +3; k++) {
                        bdraw->SetPixel(min(max(cx + k, 0), width - 1), cy, Gdiplus::Color(0, 0, 255));
                        bdraw->SetPixel(cx, min(max(cy + k, 0), height - 1), Gdiplus::Color(0, 0, 255));
                    }
                    cx = (int)(((float)x2 / (float)numPoints) * width + 0.5f);
                    cy = (int)(((float)y / (float)numImages) * height + 0.5f);
                    for (int k = -3; k <= +3; k++) {
                        bdraw->SetPixel(min(max(cx + k, 0), width - 1), cy, Gdiplus::Color(0, 0, 255));
                        bdraw->SetPixel(cx, min(max(cy + k, 0), height - 1), Gdiplus::Color(0, 0, 255));
                    }
                }
                if (bscan->getOpticNerveCupRange(x1, x2)) {
                    cx = (int)(((float)x1 / (float)numPoints) * width + 0.5f);
                    cy = (int)(((float)y / (float)numImages) * height + 0.5f);
                    for (int k = -3; k <= +3; k++) {
                        bdraw->SetPixel(min(max(cx + k, 0), width - 1), cy, Gdiplus::Color(0, 255, 0));
                        bdraw->SetPixel(cx, min(max(cy + k, 0), height - 1), Gdiplus::Color(0, 255, 0));
                    }
                    cx = (int)(((float)x2 / (float)numPoints) * width + 0.5f);
                    cy = (int)(((float)y / (float)numImages) * height + 0.5f);
                    for (int k = -3; k <= +3; k++) {
                        bdraw->SetPixel(min(max(cx + k, 0), width - 1), cy, Gdiplus::Color(0, 255, 0));
                        bdraw->SetPixel(cx, min(max(cy + k, 0), height - 1), Gdiplus::Color(0, 255, 0));
                    }
                }
            }
            for (int k = -5; k <= +5; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width - 1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height - 1), Gdiplus::Color(255, 0, 0));
            }
            
            outPath = segmOutFolder + _T("\\") + _T("disc_ios_base.png");;
            // FileUtil::SaveBitmapToPng(bdraw, outPath);

            outPath = discPath + _T("\\") + patiName + _T("_") + examName + _T("_fft.png");
            FileUtil::SaveBitmapToPng(bdraw, outPath);
        }
    }

    std::wcout << L"\nTotal result folders found: " << resultFolders.size() << std::endl;
}
