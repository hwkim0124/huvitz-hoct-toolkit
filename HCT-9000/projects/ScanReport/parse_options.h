#pragma once

#include "ScanReport.h"

#include <string>

struct ScanOptions {
    std::string dir_path = "D:\\HOCT-Samples\\clinics\\251202_chungju_hanyang\\OCT_250721_104008_OD";
    int num_bscan = 96; // 384;
    int num_ascan = 512; // 384;
    int bscan_height = 768;
    float scan_range = 4.5f;
    std::string scan_domain = "macular";
    std::string pattern_type = "cube"; // "angio";

    ScanOptions() {
        dir_path = "D://HOCT-Samples//clinics//251202_chungju_gunkuk//_disc//OCT_251201_155544_OD";
        scan_domain = "macular";
        // scan_range = 11.0f;
    }

    bool isMacularDomain() const { return scan_domain == "macular"; }
    bool isDiscDomain() const { return scan_domain == "disc"; }
    bool isWideDomain() const { return scan_domain == "cornea"; }
    bool isCubePattern() const { return pattern_type == "cube"; }
    bool isAngioPattern() const { return pattern_type == "angio"; }
    bool isRasterPattern() const { return pattern_type == "raster"; }
    bool isRadialPattern() const { return pattern_type == "radial"; }
    bool isLinePattern() const { return pattern_type == "line"; }

    OctGlobal::PatternName getPatternName() const {
        if (isCubePattern()) {
            if (isDiscDomain()) {
                return OctGlobal::PatternName::Disc3D;
            }
            else {
                return OctGlobal::PatternName::Macular3D;
            }
        }
        else if (isAngioPattern()) {
            if (isDiscDomain()) {
                return OctGlobal::PatternName::DiscAngio;
            }
            else {
                return OctGlobal::PatternName::MacularAngio;
            }
        }
        return OctGlobal::PatternName::MacularLine;
    }
};


ScanOptions ParseScanOptions(int argc, char* argv[]);
