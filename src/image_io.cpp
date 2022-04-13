#include <QImage>
#include <iostream>
#include <glm/common.hpp>

#include "image.h"
#include "compressed_image.h"

vec3 rgb2vec3(QColor rgb)
{
    return vec3(rgb.red(), rgb.green(), rgb.blue());
}

QRgb vec3toRgb(vec3 c)
{
    c = glm::clamp(c, vec3(0), vec3(255));
    int r = std::round(c.r);
    int g = std::round(c.g);
    int b = std::round(c.b);
    QColor qc(r, g, b);
    return qc.rgba();
}

bool Image::load(const char *path)
{
    QImage img(path);

    if (img.isNull())
        return false;

    img = img.scaledToWidth(img.width());

    resize(img.width(), img.height());

    for (int y = 0, i = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x, ++i) {
        data[i] = rgb2vec3(QColor(img.pixel(x, y)));
    }

    return true;
}

bool Image::save(const char *path) const
{
    QImage img(resx, resy, QImage::Format_RGBA8888);

    for (int y = 0, i = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x, ++i) {
        img.setPixel(x, y, vec3toRgb(data[i]));
    }

    return img.save(path, "png", 66);
}

bool Image::saveMask(const char *path, uint8_t bits) const
{
    QImage img(resx, resy, QImage::Format_RGBA8888);

    for (int y = 0, i = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x, ++i) {
        if (mask_[i] & bits)
            img.setPixel(x, y, QColor(255, 255, 255).rgba());
        else
            img.setPixel(x, y, QColor(0, 0, 0).rgba());
    }

    return img.save(path);
}

bool CompressedImage::saveUncompressed(const char *path) const
{
    QImage img(resx, resy, QImage::Format_RGBA8888);

    for (int y = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x) {
        img.setPixel(x, y, vec3toRgb(pixel(x, y)));
    }

    return img.save(path);
}

