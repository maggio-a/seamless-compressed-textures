#ifndef METRIC_H
#define METRIC_H

#include "image.h"
#include <glm/geometric.hpp>

template <typename ImgCmpType2>
double mse(const Image& i1, const ImgCmpType2& i2, uint8_t bitmask = 0)
{
    assert(i1.resx == i2.resx);
    assert(i1.resy == i2.resy);

    double sum = 0;
    int count = 0;
    for (int y = 0; y < i1.resy; ++y) {
        for (int x = 0; x < i1.resx; ++x) {
            if ((!bitmask) || (i1.mask(x, y) & bitmask)) {
                vec3 d = i1.pixel(x, y) - i2.pixel(x,y);
                sum += glm::dot(d, d);
                count += 3;
            }
        }
    }

    return sum / (double) count;
}

#endif // METRIC_H
