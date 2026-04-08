#pragma once
#include <set>
#include <utility>
#include <memory>

class BadPixelCorrector
{
public :
    enum class BAYER_FORMAT {
        GR,
        RG,
        GB,
        BG
    };

public:
    BadPixelCorrector() = delete;
    BadPixelCorrector(unsigned int w, unsigned int h, BAYER_FORMAT f);
    virtual ~BadPixelCorrector();

    void setBadPixels(const std::set<std::pair<unsigned int, unsigned int>>& pixels);
    void correct_median(unsigned short *);

private:
    BAYER_FORMAT format;
    struct impl_t;
    std::unique_ptr<impl_t> impl;
};

