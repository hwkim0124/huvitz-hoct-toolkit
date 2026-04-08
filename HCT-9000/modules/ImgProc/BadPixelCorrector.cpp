
#include "stdafx.h"
#include "BadPixelCorrector.h"

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>

using Pixel = std::pair<unsigned int, unsigned int>;

struct BadPixelCorrector::impl_t {
    impl_t() = delete;
    impl_t(unsigned int w, unsigned int h);
    ~impl_t() {};
    std::set<Pixel> getNeighbors4(Pixel);
    std::set<Pixel> getNeighbors8(Pixel);
    std::set<Pixel> filterBoundary(std::set<std::pair<int,int>>&);

    unsigned int width;
    unsigned int height;
    std::map<Pixel, std::set<Pixel>> badPixelNeighbors;
};

BadPixelCorrector::impl_t::impl_t(unsigned int w, unsigned int h)
    :width(w), height(h)
{
}

std::set<Pixel> BadPixelCorrector::impl_t::getNeighbors4(Pixel pixel)
{
    std::set<std::pair<int, int>> tmp; // it's signed : to distinguish boundary

    int x = (int)pixel.first;
    int y = (int)pixel.second;
    tmp.insert(std::pair<int, int>{x - 1, y - 1});
    tmp.insert(std::pair<int, int>{x + 1, y - 1});
    tmp.insert(std::pair<int, int>{x - 1, y + 1});
    tmp.insert(std::pair<int, int>{x + 1, y + 1});

    return filterBoundary(tmp);
}

std::set<Pixel> BadPixelCorrector::impl_t::getNeighbors8(Pixel pixel)
{
    std::set<std::pair<int, int>> tmp;

    int x = (int)pixel.first;
    int y = (int)pixel.second;
    tmp.insert(std::pair<int, int>{x - 2, y - 2});
    tmp.insert(std::pair<int, int>{x - 0, y - 2});
    tmp.insert(std::pair<int, int>{x + 2, y - 2});
    tmp.insert(std::pair<int, int>{x - 2, y + 0});
    tmp.insert(std::pair<int, int>{x + 2, y + 0});
    tmp.insert(std::pair<int, int>{x - 2, y + 2});
    tmp.insert(std::pair<int, int>{x - 0, y + 2});
    tmp.insert(std::pair<int, int>{x + 2, y + 2});

    return filterBoundary(tmp);
}

std::set<Pixel> BadPixelCorrector::impl_t::filterBoundary(std::set<std::pair<int, int>>& pixels)
{
    std::set<Pixel> selected;
    for (auto p : pixels) {
        if ((p.first >= 0 && p.first < (int)width) &&
            (p.second >= 0 && p.second < (int)height)) {
            selected.insert(Pixel{ p.first, p.second });
        }
    }

    return selected;
}

BadPixelCorrector::BadPixelCorrector(unsigned int w, unsigned int h, BAYER_FORMAT f)
    :format(f)
{
    try {
        impl = std::unique_ptr<impl_t>{ new impl_t{ w, h } };
    }
    catch (...) {
        throw;
    }
}


BadPixelCorrector::~BadPixelCorrector()
{
}

void BadPixelCorrector::setBadPixels(const std::set<Pixel>& pixels)
{
    auto badPixels = pixels;

    for (auto &pixel : badPixels) {
        unsigned int sum = pixel.first + pixel.second;
        std::set<Pixel> temporary_neighbors;
        if (format == BAYER_FORMAT::GR || format == BAYER_FORMAT::GB) {
            if (sum % 2 == 0) {     // even : green
                temporary_neighbors = impl->getNeighbors4(pixel);
            }
            else {                  // odd : red or blue
                temporary_neighbors = impl->getNeighbors8(pixel);
            }
        }
        else {
            if (sum % 2 == 0) {     // even : red or blue
                temporary_neighbors = impl->getNeighbors8(pixel);
            }
            else {                  // odd : green
                temporary_neighbors = impl->getNeighbors4(pixel);
            }

        }

        std::set<Pixel> neighbors;
        for (auto &neighbor : temporary_neighbors) {
            if (pixels.find(neighbor) == pixels.end()) {
                neighbors.insert(neighbor);
            }
        }

        impl->badPixelNeighbors[pixel] = neighbors;
    }
}

void BadPixelCorrector::correct_median(unsigned short *buf)
{
    std::vector<unsigned short> neighborValues;

    for (auto &pixelSet : impl->badPixelNeighbors) {
        auto &p = pixelSet.first;
        auto x = p.first;
        auto &neighborPixels = pixelSet.second;
        for (auto &n : neighborPixels) {   // accumulate surrounding points
            auto idx = n.first + impl->width*n.second;
            neighborValues.push_back(buf[n.first + impl->width*n.second]);
        }
        std::sort(neighborValues.begin(), neighborValues.end());
        auto median = neighborValues.at(neighborValues.size() / 2);
        buf[p.first + impl->width*p.second] = median;

        neighborValues.clear();
    }
}

