#include "pyramid.h"
#include "mesh.h"

#include <glm/common.hpp>

Pyramid::Pyramid()
{

}

void Pyramid::clear()
{
    level.clear();
}

void Pyramid::init(const Image& img)
{
    clear();
    level.push_back(img);
}

void Pyramid::pullNearest()
{
    while (level.back().resx > 1 && level.back().resy > 1) {
        Image& imgprev = level.back();
        level.push_back(Image());
        assert(0);
        //imgprev.resampleHalf(level.back(), Image::ResampleMode::Nearest);
    }
}

void Pyramid::pullLinear()
{
    while (level.back().resx > 1 && level.back().resy > 1) {
        Image& imgprev = level.back();
        level.push_back(Image());
        assert(0);
        //imgprev.resampleHalf(level.back(), Image::ResampleMode::Linear);
    }
}

void Pyramid::push()
{
    assert(0);
}

void Pyramid::pullPixelHoppe(int x, int y, const Image& coarse, Image& fine)
{
    ivec2 p[4] = {
        ivec2(x, y) * 2,
        ivec2(x, y) * 2 + ivec2(1, 0),
        ivec2(x, y) * 2 + ivec2(0, 1),
        ivec2(x, y) * 2 + ivec2(1, 1),
    };

    for (int i = 0; i < 4; ++i) {
        assert(0 && "the following 3 lines are obsolete (mask is now uint8_t instead of float");
        //float wxy = fine.mask[fine.indexOf(p[i].x, p[i].y)];
        //float w = coarse.weight(x, y);
        //fine.pixel(p[i].x, p[i].y) = glm::mix(fine.pixel(p[i].x, p[i].y), coarse.pixel(x, y), (1 - wxy));

        /*
        if (w < 1.0f) {
            fine.pixel(p[i].x, p[i].y) = coarse.pixel(x, y);
        }
        */
    }
}

void Pyramid::pullHoppe()
{
    if (level.size() < 2)
        return;
    for (unsigned i = level.size() - 1; i > 0; --i) {
        Image& coarse = level[i];
        Image& fine = level[i-1];
        for (int y = 0; y < coarse.resy; ++y)
        for (int x = 0; x < coarse.resx; ++x) {
            pullPixelHoppe(x, y, coarse, fine);
        }
    }
}

void Pyramid::setMaskInternal(const Mesh& m)
{
    for (Image& l : level)
        l.setMaskInternal(m);
}

void Pyramid::setMaskSeam(const Mesh& m)
{
    for (Image& l : level)
        l.setMaskSeam(m);
}

void Pyramid::save(const char *name)
{
    for (unsigned i = 0; i < level.size(); ++i) {
        std::string lname = std::string(name) + "_" + std::to_string(i) + ".png";
        level[i].save(lname.c_str());
    }
}

