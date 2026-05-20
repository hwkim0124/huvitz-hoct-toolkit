// RetiSegEval.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h" 
#include "RetiSegEval.h"
#include "FileUtil.h"

#include <iostream>
#include <boost/format.hpp>
#include <fstream>
#include <iomanip>

int main()
{

    CString dataPath = _T("D:/HOCT-Samples/in_house/260504_ia_fda_pretest/hoct_data2/hoct_data_b");
    CString macuPath = _T("D:/HOCT-Samples/in_house/260504_ia_fda_pretest/segm_eval2/macu_proc_b");
    CString discPath = _T("D:/HOCT-Samples/in_house/260504_ia_fda_pretest/segm_eval2/disc_proc_b");
    CString sectPath = _T("D:/HOCT-Samples/in_house/260504_ia_fda_pretest/segm_eval2/sect_data_b");

    bool isSavingCSV = true;
    CStdioFile csvEtdrsFile;
    CStdioFile csvGccFile;
    CStdioFile csvRnflFile;
    CStdioFile csvRnflFile1;
	CStdioFile csvRnflFile2;
	CStdioFile csvEtdrsWideFile;
	CStdioFile csvGccWideFile;
	CStdioFile csvRnflWideFile;
	CStdioFile csvRnflWideFile1;
	CStdioFile csvRnflWideFile2;

    if (isSavingCSV) {
		csvEtdrsFile.Open(sectPath + _T("\\etdrs_b.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		csvGccFile.Open(sectPath + _T("\\gcc_b.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		csvRnflFile.Open(sectPath + _T("\\rnfl_b.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		csvRnflFile1.Open(sectPath + _T("\\rnfl_b_1.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		csvRnflFile2.Open(sectPath + _T("\\rnfl_b_2.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);

        csvEtdrsWideFile.Open(sectPath + _T("\\etdrs_wide_b.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
        csvGccWideFile.Open(sectPath + _T("\\gcc_wide_b.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
        csvRnflWideFile.Open(sectPath + _T("\\rnfl_wide_b.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		csvRnflWideFile1.Open(sectPath + _T("\\rnfl_wide_b_1.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		csvRnflWideFile2.Open(sectPath + _T("\\rnfl_wide_b_2.csv"), CFile::modeCreate | CFile::modeWrite | CFile::typeText);

		csvEtdrsFile.WriteString(_T("PID,EXAM,01_CEN,02_INN,03_INN,04_INN,05_INN,06_OUT,07_OUT,08_OUT,09_OUT\n"));
		csvGccFile.WriteString(_T("PID,EXAM,G_SUP,G_S_N,G_N_I,G_INP,G_I_T,G_TEM\n"));
		csvRnflFile.WriteString(_T("PID,EXAM,01_SUP,02_S_N,03_N_S,04_NAS,05_N_I,06_I_N,07_INP,08_I_T,09_T_I,10_TEM,11_T_S,12_S_T,CupDiscVolume,CupDiscArea,RimArea,DiscArea,CupVolume\n"));
		csvRnflFile1.WriteString(_T("PID,EXAM,01_SUP,02_S_N,03_N_S,04_NAS,05_N_I,06_I_N,07_INP,08_I_T,09_T_I,10_TEM,11_T_S,12_S_T,CupDiscVolume,CupDiscArea,RimArea,DiscArea,CupVolume\n"));
		csvRnflFile2.WriteString(_T("PID,EXAM,01_SUP,02_S_N,03_N_S,04_NAS,05_N_I,06_I_N,07_INP,08_I_T,09_T_I,10_TEM,11_T_S,12_S_T,CupDiscVolume,CupDiscArea,RimArea,DiscArea,CupVolume\n"));

        csvEtdrsWideFile.WriteString(_T("PID,EXAM,01_CEN,02_INN,03_INN,04_INN,05_INN,06_OUT,07_OUT,08_OUT,09_OUT\n"));
		csvGccWideFile.WriteString(_T("PID,EXAM,G_SUP,G_S_N,G_N_I,G_INP,G_I_T,G_TEM\n"));
		csvRnflWideFile.WriteString(_T("PID,EXAM,01_SUP,02_S_N,03_N_S,04_NAS,05_N_I,06_I_N,07_INP,08_I_T,09_T_I,10_TEM,11_T_S,12_S_T,CupDiscVolume,CupDiscArea,RimArea,DiscArea,CupVolume\n"));
		csvRnflWideFile1.WriteString(_T("PID,EXAM,01_SUP,02_S_N,03_N_S,04_NAS,05_N_I,06_I_N,07_INP,08_I_T,09_T_I,10_TEM,11_T_S,12_S_T,CupDiscVolume,CupDiscArea,RimArea,DiscArea,CupVolume\n"));
		csvRnflWideFile2.WriteString(_T("PID,EXAM,01_SUP,02_S_N,03_N_S,04_NAS,05_N_I,06_I_N,07_INP,08_I_T,09_T_I,10_TEM,11_T_S,12_S_T,CupDiscVolume,CupDiscArea,RimArea,DiscArea,CupVolume\n"));
    }

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
        /*
        if (!isDisc) {
            continue;
        }
        */

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
                    bdraw->SetPixel(j, opls[j], Gdiplus::Color(255, 255, 0));
                    bdraw->SetPixel(j, opls[j] + 1, Gdiplus::Color(255, 255, 0));
                }
            }

            auto ioss = bscan->getLayerPoints(OcularLayerType::IOS, width, height);
            for (int j = 0; j < ioss.size(); j++) {
                if (ioss[j] >= 0) {
                    bdraw->SetPixel(j, ioss[j], Gdiplus::Color(0, 192, 255));
                    bdraw->SetPixel(j, ioss[j] + 1, Gdiplus::Color(0, 192, 255));
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

            /*
            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width-1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height-1), Gdiplus::Color(255, 0, 0));
            }

            CString outPath = segmOutFolder + _T("\\") + _T("macular_enface.png");;

            outPath = macuPath + _T("\\") + patiName + _T("_") + examName + _T("_enface.png");
            FileUtil::SaveBitmapToPng(bdraw, outPath);
            */

            auto tmap = report->getMacularEplot(OcularLayerType::ILM, OcularLayerType::IPL);
            image = tmap->makeImage(300, 300);
            bdraw = image.getBitmap();
            width = image.getWidth();
            height = image.getHeight();

            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width - 1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height - 1), Gdiplus::Color(255, 0, 0));
            }

            CString outPath = segmOutFolder + _T("\\") + _T("macular_ilm_ipl.png");;
            outPath = macuPath + _T("\\") + patiName + _T("_") + examName + _T("_tmap.png");
            FileUtil::SaveBitmapToPng(bdraw, outPath);

            float foveaX = report->getFoveaCenterX();
            float foveaY = report->getFoveaCenterY();
            auto chartThick = report->makeETDRSChart(OcularLayerType::ILM, OcularLayerType::RPE, 0.0f, 0.0f, foveaX, foveaY);
			auto chartGcc = report->makeGCCThicknessChart(OcularLayerType::ILM, OcularLayerType::IPL, 0.0f, 0.0f, foveaX, foveaY);

            if (isSavingCSV) {
                CString line1, line2;
                line1.Format(_T("%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n"), patiName, examName,
                    chartThick.centerThickness(),
                    chartThick.innerThickness(0), chartThick.innerThickness(1), chartThick.innerThickness(2), chartThick.innerThickness(3),
                    chartThick.outerThickness(0), chartThick.outerThickness(1), chartThick.outerThickness(2), chartThick.outerThickness(3));
				line2.Format(_T("%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n"), patiName, examName,
					chartGcc.sectionThickness(0), chartGcc.sectionThickness(1), chartGcc.sectionThickness(2), chartGcc.sectionThickness(3), chartGcc.sectionThickness(4), chartGcc.sectionThickness(5));

                if (isWide) {
					csvEtdrsWideFile.WriteString(line1);
					csvGccWideFile.WriteString(line2);
                }
                else {
					csvEtdrsFile.WriteString(line1);
					csvGccFile.WriteString(line2);
                }
            }
        }

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
            
            /*
            for (int k = -3; k <= +3; k++) {
                bdraw->SetPixel(min(max(cx_pos + k, 0), width - 1), cy_pos, Gdiplus::Color(255, 0, 0));
                bdraw->SetPixel(cx_pos, min(max(cy_pos + k, 0), height - 1), Gdiplus::Color(255, 0, 0));
            }

            CString outPath = segmOutFolder + _T("\\") + _T("disc_enface.png");;
            // FileUtil::SaveBitmapToPng(bdraw, outPath);
      
            outPath = discPath + _T("\\") + patiName + _T("_") + examName + _T("_enface_fft.png");
            FileUtil::SaveBitmapToPng(bdraw, outPath);
            */

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
            
            CString outPath = segmOutFolder + _T("\\") + _T("disc_ios_base.png");;
            // FileUtil::SaveBitmapToPng(bdraw, outPath);

            outPath = discPath + _T("\\") + patiName + _T("_") + examName + _T("_enface.png");
            FileUtil::SaveBitmapToPng(bdraw, outPath);

			float discCenterX = report->getNerveHeadCenterX();
			float discCenterY = report->getNerveHeadCenterY();
			auto chartRnfl = report->makeDiscChart(OcularLayerType::ILM, OcularLayerType::NFL, discCenterX, discCenterY, 3.45f);
            auto chartRnfl1 = report->makeDiscChart(OcularLayerType::ILM, OcularLayerType::NFL, discCenterX, discCenterY, 3.45f * 1.05f);
            auto chartRnfl2 = report->makeDiscChart(OcularLayerType::ILM, OcularLayerType::NFL, discCenterX, discCenterY, 3.45f * 1.1f);

            if (isSavingCSV) {
                CString line, line1, line2;
                line.Format(_T("%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.5f,%.5f,%.5f,%.5f,%.5f\n"), patiName, examName,
                    chartRnfl.clockThickness(0), chartRnfl.clockThickness(1), chartRnfl.clockThickness(2), chartRnfl.clockThickness(3), chartRnfl.clockThickness(4), chartRnfl.clockThickness(5),
                    chartRnfl.clockThickness(6), chartRnfl.clockThickness(7), chartRnfl.clockThickness(8), chartRnfl.clockThickness(9), chartRnfl.clockThickness(10), chartRnfl.clockThickness(11), 
					report->getCupDiscVertRatio(), report->getCupDiscAreaRatio(), report->getRimArea(), report->getDiscArea(), report->getCupVolume()
                    );
                line1.Format(_T("%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.5f,%.5f,%.5f,%.5f,%.5f\n"), patiName, examName,
                    chartRnfl1.clockThickness(0), chartRnfl1.clockThickness(1), chartRnfl1.clockThickness(2), chartRnfl1.clockThickness(3), chartRnfl1.clockThickness(4), chartRnfl1.clockThickness(5),
                    chartRnfl1.clockThickness(6), chartRnfl1.clockThickness(7), chartRnfl1.clockThickness(8), chartRnfl1.clockThickness(9), chartRnfl1.clockThickness(10), chartRnfl1.clockThickness(11),
                    report->getCupDiscVertRatio(), report->getCupDiscAreaRatio(), report->getRimArea(), report->getDiscArea(), report->getCupVolume()
                );
                line2.Format(_T("%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.5f,%.5f,%.5f,%.5f,%.5f\n"), patiName, examName,
                    chartRnfl2.clockThickness(0), chartRnfl2.clockThickness(1), chartRnfl2.clockThickness(2), chartRnfl2.clockThickness(3), chartRnfl2.clockThickness(4), chartRnfl2.clockThickness(5),
                    chartRnfl2.clockThickness(6), chartRnfl2.clockThickness(7), chartRnfl2.clockThickness(8), chartRnfl2.clockThickness(9), chartRnfl2.clockThickness(10), chartRnfl2.clockThickness(11),
                    report->getCupDiscVertRatio(), report->getCupDiscAreaRatio(), report->getRimArea(), report->getDiscArea(), report->getCupVolume()
                );

                if (isWide) {
					csvRnflWideFile.WriteString(line);
					csvRnflWideFile1.WriteString(line1);
					csvRnflWideFile2.WriteString(line2);
                }
                else {
					csvRnflFile.WriteString(line);
					csvRnflFile1.WriteString(line1);
					csvRnflFile2.WriteString(line2);
                }
            }
        }
    }

    if (isSavingCSV) {
        csvEtdrsFile.Close();
		csvGccFile.Close();
		csvRnflFile.Close();
        csvRnflFile1.Close();
        csvRnflFile2.Close();

        csvEtdrsWideFile.Close();
		csvGccWideFile.Close();
		csvRnflWideFile.Close();
		csvRnflWideFile1.Close();
		csvRnflWideFile2.Close();
    }

    std::wcout << L"\nTotal result folders found: " << resultFolders.size() << std::endl;
}
