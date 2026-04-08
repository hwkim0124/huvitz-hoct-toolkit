
#include "stdafx.h"
#include "BadPixelDetector.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <utility>
#include <memory>
#include <set>
#include <algorithm>

using ushort = unsigned short;
using uint = unsigned int;
using Pixel = std::pair<uint, uint>;

struct BadPixelDetector::impl_t {
    impl_t() = delete;
    impl_t(uint w, uint h, BAYER_FORMAT f);
    ~impl_t();
    void loadImage(char *buf, const std::string& filename);

    std::vector<ushort> getMedian(const ushort *buf, uint w, uint h);

    void detectDark();
    void detectBright();
    void detectDarkSubregion(const ushort *buf, std::vector<ushort> &meds, uint offsetX, uint offsetY, uint w, uint h);
    void detectBrightSubregion(const ushort *buf, std::vector<ushort> &meds, uint offsetX, uint offsetY, uint w, uint h);

    void setDarkImages(const std::vector<std::string>& filenames);
    void setBrightImages(const std::vector<std::string>& filenames);
    void setImages(std::unique_ptr<ushort[]> &uptr, const std::vector<std::string>& filenames);

    uint width;
    uint height;
    BAYER_FORMAT format;

    std::vector<Pixel> offsetR;
    std::vector<Pixel> offsetG;
    std::vector<Pixel> offsetB;

    std::unique_ptr<ushort[]> bufDark;
    std::unique_ptr<ushort[]> bufBright;

    std::set<Pixel> badPixels;
    const size_t MaxBadPixels = 4096;

    ushort thresDarkR;
    ushort thresDarkG;
    ushort thresDarkB;
    ushort thresBrightR;
    ushort thresBrightG;
    ushort thresBrightB;
};

BadPixelDetector::impl_t::impl_t(uint w, uint h, BAYER_FORMAT f)
    :width(w), height(h), format(f)
{
    thresDarkR = 250;
    thresDarkG = 250;
    thresDarkB = 250;
    thresBrightR = 12;
    thresBrightG = 12;
    thresBrightB = 12;

    if (format == BAYER_FORMAT::GB) {
        offsetR = std::vector<Pixel>{ Pixel{ 1, 0 } };
        offsetG = std::vector<Pixel>{ Pixel{ 0, 0 }, Pixel{ 1, 1 } };
        offsetB = std::vector<Pixel>{ Pixel{ 0, 1 } };
    }
    else if (format == BAYER_FORMAT::GR) {
        offsetR = std::vector<Pixel>{ Pixel{ 0, 1 } };
        offsetG = std::vector<Pixel>{ Pixel{ 0, 0 }, Pixel{ 1, 1 } };
        offsetB = std::vector<Pixel>{ Pixel{ 1, 0 } };
    }
    else if (format == BAYER_FORMAT::BG) {
        offsetR = std::vector<Pixel>{ Pixel{ 0, 0 } };
        offsetG = std::vector<Pixel>{ Pixel{ 0, 1 }, Pixel{ 1, 0 } };
        offsetB = std::vector<Pixel>{ Pixel{ 1, 1 } };
    }
    else {  // format == BAYER_FORMAT::RG
        offsetR = std::vector<Pixel>{ Pixel{ 1, 1 } };
        offsetG = std::vector<Pixel>{ Pixel{ 0, 1 }, Pixel{ 1, 0 } };
        offsetB = std::vector<Pixel>{ Pixel{ 0, 0 } };
    }

}

BadPixelDetector::impl_t::~impl_t()
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
        std::string errmsg = std::string("Cannot open file ") + filename;
        throw std::runtime_error(errmsg.c_str());
    }
}

std::vector<ushort> BadPixelDetector::impl_t::getMedian(const ushort *buf, uint w, uint h)
{
    std::vector<ushort> valuesR;
    std::vector<ushort> valuesG;
    std::vector<ushort> valuesB;

    for (uint y = 0; y < h; y += 2) {
        for (uint x = 0; x < w; x += 2) {
            for (auto &p : offsetR) {
                valuesR.push_back(buf[(x + p.first) + width*(y + p.second)]);
            }
            for (auto &p : offsetG) {
                valuesG.push_back(buf[(x + p.first) + width*(y + p.second)]);
            }
            for (auto &p : offsetB) {
                valuesB.push_back(buf[(x + p.first) + width*(y + p.second)]);
            }
        }
    }

    std::sort(valuesR.begin(), valuesR.end());
    std::sort(valuesG.begin(), valuesG.end());
    std::sort(valuesB.begin(), valuesB.end());

    std::vector<ushort> retvec;
    retvec.push_back((valuesR[valuesR.size() / 2] + valuesR[valuesR.size() / 2 - 1]) / 2);
    retvec.push_back((valuesG[valuesG.size() / 2] + valuesG[valuesG.size() / 2 - 1]) / 2);
    retvec.push_back((valuesB[valuesB.size() / 2] + valuesB[valuesB.size() / 2 - 1]) / 2);

    return retvec;
}

void BadPixelDetector::impl_t::detectDark()
{
    const uint subRegionSizeX = 128;
    const uint subRegionSizeY = 128;
    const uint startX = (width % subRegionSizeX) / 2;
    const uint startY = (height % subRegionSizeY) / 2;
    for (uint y = startY; y <= height - subRegionSizeY; y += subRegionSizeY) {      //GR
        for (uint x = startX; x <= width - subRegionSizeX; x += subRegionSizeX) {   //BG
            auto medians = getMedian(&bufDark[x + width*y], subRegionSizeX, subRegionSizeY);
            try {
                detectDarkSubregion(&bufDark[x + width*y], medians, x, y, subRegionSizeX, subRegionSizeY);
            }
            catch (...) {
                throw;
            }
        }
    }
}

void BadPixelDetector::impl_t::detectBright()
{
    const uint subRegionSizeX = 128;
    const uint subRegionSizeY = 128;
    const uint startX = (width % subRegionSizeX) / 2;
    const uint startY = (height % subRegionSizeY) / 2;
    for (uint y = startY; y <= height - subRegionSizeY; y += subRegionSizeY) {
        for (uint x = startX; x <= width - subRegionSizeX; x += subRegionSizeX) {
            auto medians = getMedian(&bufBright[x + width*y], subRegionSizeX, subRegionSizeY);
            try {
                detectBrightSubregion(&bufBright[x + width*y], medians, x, y, subRegionSizeX, subRegionSizeY);
            }
            catch (...) {
                throw;
            }
        }
    }
}

void BadPixelDetector::impl_t::detectDarkSubregion(const ushort *buf, std::vector<ushort> &meds, uint offsetX, uint offsetY, uint w, uint h)
{
    for (uint y = 0; y < h; y += 2) {
        for (uint x = 0; x < w; x += 2) {
            for (auto &p : offsetR) {
                if (buf[(x + p.first) + width*(y + p.second)] >(meds[0] + thresDarkR)) {
                    badPixels.insert(Pixel{ offsetX + x + p.first, offsetY + y + p.second });
                }
            }
            for (auto &p : offsetG) {
                if (buf[(x + p.first) + width*(y + p.second)] >(meds[0] + thresDarkG)) {
                    badPixels.insert(Pixel{ offsetX + x + p.first, offsetY + y + p.second });
                }
            }
            for (auto &p : offsetB) {
                if (buf[(x + p.first) + width*(y + p.second)] >(meds[0] + thresDarkB)) {
                    badPixels.insert(Pixel{ offsetX + x + p.first, offsetY + y + p.second });
                }
            }

            if (badPixels.size() >= MaxBadPixels) {
                throw std::runtime_error("Too many bad pixels");
            }
        }
    }
}

void BadPixelDetector::impl_t::detectBrightSubregion(const ushort *buf, std::vector<ushort> &meds, uint offsetX, uint offsetY, uint w, uint h)
{
    uint thres;
    int thresUpperR, thresLowerR;
    int thresUpperG, thresLowerG;
    int thresUpperB, thresLowerB;

    thres = (uint)meds[0] * thresBrightR / 100;
    thresUpperR = meds[0] + thres;
    thresLowerR = (meds[0] - thres) > 0 ? (meds[0] - thres) : 0;
    thresUpperG = meds[1] + thres;
    thresLowerG = (meds[1] - thres) > 0 ? (meds[1] - thres) : 0;
    thresUpperB = meds[2] + thres;
    thresLowerB = (meds[2] - thres) > 0 ? (meds[2] - thres) : 0;

    for (uint y = 0; y < h; y += 2) {
        for (uint x = 0; x < w; x += 2) {
            for (auto &p : offsetR) {
                auto val = buf[(x + p.first) + width*(y + p.second)];
                if (val > thresUpperR || val < thresLowerR) {
                    badPixels.insert(Pixel{ offsetX + x + p.first, offsetY + y + p.second });
                }
            }
            for (auto &p : offsetG) {
                auto val = buf[(x + p.first) + width*(y + p.second)];
                if (val > thresUpperG || val < thresLowerG) {
                    badPixels.insert(Pixel{ offsetX + x + p.first, offsetY + y + p.second });
                }
            }
            for (auto &p : offsetB) {
                auto val = buf[(x + p.first) + width*(y + p.second)];
                if (val > thresUpperB || val < thresLowerB) {
                    badPixels.insert(Pixel{ offsetX + x + p.first, offsetY + y + p.second });
                }
            }

            if (badPixels.size() >= MaxBadPixels) {
                throw std::runtime_error("Too many bad pixels");
            }
        }
    }
}

void BadPixelDetector::impl_t::setDarkImages(const std::vector<std::string>& filenames)
{
    bufDark = std::make_unique<ushort[]>(width*height);
    std::memset(bufDark.get(), 0x00, width*height*sizeof(ushort));

    setImages(bufDark, filenames);
}

void BadPixelDetector::impl_t::setBrightImages(const std::vector<std::string>& filenames)
{
    bufBright = std::make_unique<ushort[]>(width*height);
    std::memset(bufBright.get(), 0x00, width*height*sizeof(ushort));

    setImages(bufBright, filenames);
}

void BadPixelDetector::impl_t::setImages(std::unique_ptr<ushort[]> &uptr, const std::vector<std::string>& filenames)
{
    const size_t wh = width * height;
    auto imgbuf = std::make_unique<ushort[]>(wh);

    ushort count = 0;
    for (auto &filename : filenames) {
        try {
            loadImage((char *)imgbuf.get(), filename);
        }
        catch (...) {
            throw;
        }

        for (size_t i = 0; i < wh; i++) {
            uptr[i] += imgbuf[i];
        }

        if (++count == 16) {
            break;
        }
    }

    if (count == 0) {
        return;
    }

    for (size_t i = 0; i < wh; i++) {
        uptr[i] /= count;
    }
}

BadPixelDetector::BadPixelDetector(uint w, uint h, BAYER_FORMAT f)
{
    try {
        impl = std::unique_ptr<impl_t>{ new impl_t{w, h, f} };
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
    try {
        if (impl->bufDark) {
            impl->detectDark();
        }
        if (impl->bufBright) {
            impl->detectBright();
        }
    }
    catch (...) {
        throw;
    }
}

std::set<Pixel>& BadPixelDetector::getBadPixels()
{
    return impl->badPixels;
}

size_t BadPixelDetector::getBadPixelsSize()
{
    return impl->badPixels.size();
}

void BadPixelDetector::setDarkImages(const std::vector<std::string>& filenames)
{
    impl->setDarkImages(filenames);
}

void BadPixelDetector::setBrightImages(const std::vector<std::string>& filenames)
{
    impl->setBrightImages(filenames);
}

void BadPixelDetector::setDarkThresholdR(ushort dn)
{
    impl->thresDarkR = dn;
}

void BadPixelDetector::setDarkThresholdG(ushort dn)
{
    impl->thresDarkG = dn;
}

void BadPixelDetector::setDarkThresholdB(ushort dn)
{
    impl->thresDarkB = dn;
}

void BadPixelDetector::setBrightThresholdR(ushort percentage)
{
    impl->thresBrightR = percentage;
}

void BadPixelDetector::setBrightThresholdG(ushort percentage)
{
    impl->thresBrightG = percentage;
}

void BadPixelDetector::setBrightThresholdB(ushort percentage)
{
    impl->thresBrightB = percentage;
}
