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
    BadPixelDetector() = delete;
    BadPixelDetector( unsigned int w, unsigned int h);
    virtual ~BadPixelDetector();

    void setDarkImages(const std::vector<std::string>& filenames);
    void setBrightImages(const std::vector<std::string>& filenames);
    void setDarkThreshold(unsigned short val);
    void setBrightThreshold(unsigned short val);
    void detect();

    std::set<std::pair<unsigned int, unsigned int>>& getBadPixels();
    void saveToFile(const std::string &filename);

private:
    unsigned short dark_cut = 100;
    unsigned short bright_cut = 3900;

    struct impl_t;
    std::unique_ptr<impl_t> impl;

    std::vector<std::string> darks;
    std::vector<std::string> brights;
};

