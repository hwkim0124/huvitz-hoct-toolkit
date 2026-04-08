#pragma once

#include <vector>
#include <string>
#include <functional>
#include <utility>
#include <memory>
#include <set>

class BadPixelDetector
{
public:
    enum class BAYER_FORMAT {
        GR,
        RG,
        GB,
        BG
    };

public:
    BadPixelDetector() = delete;
    BadPixelDetector( unsigned int w, unsigned int h, BAYER_FORMAT f);
    virtual ~BadPixelDetector();

    void setDarkImages(const std::vector<std::string>& filenames);
    void setBrightImages(const std::vector<std::string>& filenames);

    void setDarkThresholdR(unsigned short dn);
    void setDarkThresholdG(unsigned short dn);
    void setDarkThresholdB(unsigned short dn);

    void setBrightThresholdR(unsigned short percentage);
    void setBrightThresholdG(unsigned short percentage);
    void setBrightThresholdB(unsigned short percentage);

    void detect();

    size_t getBadPixelsSize();
    std::set<std::pair<unsigned int, unsigned int>>& getBadPixels();

private:
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

