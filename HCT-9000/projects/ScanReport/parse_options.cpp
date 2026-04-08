#include "pch.h"
#include "parse_options.h"

#include <string>
#include <cstring>

// Helper function to parse double from string, returns default_val if unable.
static double ParseDouble(const char* s, double default_val = 0.0) {
    char* endptr = nullptr;
    double val = std::strtod(s, &endptr);
    if (endptr == s || *endptr != '\0')
        return default_val;
    return val;
}

// Helper function to parse int from string, returns default_val if unable.
static int ParseInt(const char* s, int default_val = 0) {
    char* endptr = nullptr;
    int val = static_cast<int>(std::strtol(s, &endptr, 10));
    if (endptr == s || *endptr != '\0')
        return default_val;
    return val;
}

ScanOptions ParseScanOptions(int argc, char* argv[]) {
    ScanOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "--directory" || arg == "-d") && i + 1 < argc) {
            opts.dir_path = argv[++i];
        }
        else if ((arg == "--num-bscan" || arg == "-b") && i + 1 < argc) {
            opts.num_bscan = ParseInt(argv[++i]);
        }
        else if ((arg == "--num-ascan" || arg == "-a") && i + 1 < argc) {
            opts.num_ascan = ParseInt(argv[++i]);
        }
        else if ((arg == "--bscan-height" || arg == "-h") && i + 1 < argc) {
            opts.bscan_height = ParseInt(argv[++i]);
        }
        else if ((arg == "--scan-range" || arg == "-r") && i + 1 < argc) {
            opts.scan_range = (float)ParseDouble(argv[++i]);
        }
        else if ((arg == "--scan-domain" || arg == "-s") && i + 1 < argc) {
            opts.scan_domain = argv[++i];
        }
        else if ((arg == "--pattern-type" || arg == "-p") && i + 1 < argc) {
            opts.pattern_type = argv[++i];
        }
    }
    return opts;
}
