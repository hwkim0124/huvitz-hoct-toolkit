#include "BadPixelDetector.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <utility>
#include <memory>
#include <set>

using Pixel = std::pair<unsigned int, unsigned int>;

struct BadPixelDetector::impl_t {
    impl_t() = delete;
    impl_t(unsigned int w, unsigned int h);
    void loadImage(char *buf, const std::string& filename);
    template<typename Comp>
    void detect(std::vector<std::string> &images, Comp cut);

    unsigned int width;
    unsigned int height;
    std::set<Pixel> badPixels;
};

BadPixelDetector::impl_t::impl_t(unsigned int w, unsigned int h)
    :width(w), height(h)
{
}

void BadPixelDetector::impl_t::loadImage(char *buf, const std::string& filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);
        file.read((char *)buf, size);
        file.close();
    }
    else {
        throw - 1;
    }
}

template<typename Comp>
void BadPixelDetector::impl_t::detect(std::vector<std::string> &images, Comp cut)
{
    auto rawbuf = new unsigned short[width*height];

    for (auto s : images) {
        loadImage((char *)rawbuf, s);
        unsigned int idx;
        for (unsigned int y = 0; y < height; y++) {
            for (unsigned int x = 0; x < width; x++) {
                idx = width*y + x;
                if (cut(rawbuf[idx])) {
                    badPixels.insert(Pixel{ x, y });
                }
            }
        }
    }

    delete[] rawbuf;
}

BadPixelDetector::BadPixelDetector(unsigned int w, unsigned int h)
{
    try {
        impl = std::unique_ptr<impl_t>{ new impl_t{w, h} };
    }
    catch (...) {
        throw;
    }
}

BadPixelDetector::~BadPixelDetector()
{
}

void BadPixelDetector::detect()
{
    auto isHot = [this](auto val) ->bool { return val > dark_cut; };
    impl->detect(darks, isHot);

    auto isCold = [this](auto val) ->bool { return val < bright_cut; };
    impl->detect(brights, isCold);
}

std::set<Pixel>& BadPixelDetector::getBadPixels()
{
    return impl->badPixels;
}

void BadPixelDetector::setDarkImages(const std::vector<std::string>& filenames)
{
    darks = filenames;
}

void BadPixelDetector::setBrightImages(const std::vector<std::string>& filenames)
{
    brights = filenames;
}

void BadPixelDetector::setBrightThreshold(unsigned short val)
{
    bright_cut = val;
}

void BadPixelDetector::setDarkThreshold(unsigned short val)
{
    dark_cut = val;
}

void BadPixelDetector::saveToFile(const std::string &filename)
{
    std::ofstream file(filename, std::ios::out);
    if (!file.is_open()) {
        throw - 1;
    }

    for (auto &pixel : impl->badPixels) {
        file << pixel.first << ' ' << pixel.second;
        file << std::endl;
    }
    file.close();
}
