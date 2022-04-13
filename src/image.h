#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

using namespace glm;

class Mesh;
struct Pyramid;

class Image
{
    friend struct Pyramid;

    std::vector<vec3> data;
    //std::vector<float> mask;
    std::vector<uint8_t> mask_;

public:

    static constexpr double SEAM_SAMPLING_FACTOR = 2.0;

    enum MaskBit {
        Internal = 1,
        Seam = 2
    };

    enum ResampleMode {
        Linear,
        Nearest
    };

    int resx;
    int resy;

    Image() : resx(0), resy(0) {}

#ifdef __EMSCRIPTEN__
    void load(uint8_t *imgbuf, int w, int h);
    void store(uint8_t *imgbuf, int w, int h);
#else
    bool load(const char *path);
    bool save(const char *path) const;
    bool saveMask(const char *path, uint8_t bits) const;
#endif

    void resize(int rx, int ry);

    void drawLine(vec2 from, vec2 to, vec3 c);
    void drawPoint(vec2 p, vec3 c);

    uint indexOf(int x, int y) const;

    vec3& pixel(int x, int y) {
        return data[indexOf(x, y)];
    }

    vec3 pixel(int x, int y) const {
        return data[indexOf(x, y)];
    }

    vec3 pixel(vec2 p) const;

    uint8_t mask(int x, int y) const {
        return mask_[indexOf(x, y)];
    }

    uint8_t& mask(int x, int y) {
        return mask_[indexOf(x, y)];
    }

    /*
    float weight_(int x, int y) const {
        return mask[indexOf(x, y)];
    }

    float& weight_(int x, int y) {
        return mask[indexOf(x, y)];
    }
    */

    // fetches component of the texture lookup with their weights
    void fetch(vec2 p, vec3& t00, vec3& t10, vec3& t01, vec3& t11,
               double &w00, double &w10, double &w01, double& w11) const;

    // fetches the pixels contributing to the bilinear interpolation lookup of p
    void fetchIndex(vec2 p, vec3& p00, vec3& p10, vec3& p01, vec3& p11) const;

    void clearMask();
    unsigned setMaskInternal(const Mesh& m);
    unsigned setMaskSeam(const Mesh& m);

};

#endif // IMAGE_H
