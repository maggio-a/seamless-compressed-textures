#ifndef PYRAMID_H
#define PYRAMID_H

#include "image.h"

class Mesh;

struct Pyramid
{
    std::vector<Image> level;

    Pyramid();

    void clear();
    void init(const Image& img);

    void pullNearest();
    void pullLinear();

    void push();

    void pullPixelHoppe(int x, int y, const Image& coarse, Image& fine);
    void pullPixelUs();

    void pullHoppe();
    void pullUs();


    void setMaskInternal(const Mesh& m);
    void setMaskSeam(const Mesh& m);

    void save(const char *name);
};

#endif // PYRAMID_H
