#include "image.h"
#include "mesh.h"

#include <cmath>
#include <cassert>
#include <iostream>
#include <numeric>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

void Image::resize(int rx, int ry)
{
    resx = rx;
    resy = ry;
    data.resize(resx * resy, vec3(0));
    clearMask();
}

void Image::drawPoint(vec2 p, vec3 c)
{
    pixel(std::floor(p[0] - 0.5), std::floor(p[1] - 0.5)) = c;
    pixel(std::floor(p[0] - 0.5), std::floor(p[1] + 0.5)) = c;
    pixel(std::floor(p[0] + 0.5), std::floor(p[1] - 0.5)) = c;
    pixel(std::floor(p[0] + 0.5), std::floor(p[1] + 0.5)) = c;
}

void Image::drawLine(vec2 from, vec2 to, vec3 c)
{
    int d = std::ceil(glm::distance(from, to));

    for (double t = 0; t <= 1.0; t += 1.0 / d) {
        drawPoint(mix(from, to, t), c);
    }

}

uint Image::indexOf(int x, int y) const
{
    x = (x + resx) % resx;
    y = (y + resy) % resy;
    return y * resx + x;
}


vec3 Image::pixel(vec2 p) const
{
    p -= vec2(0.5);
    vec2 p0 = floor(p);
    vec2 p1 = floor(p + vec2(1));
    vec2 w = fract(p);
    return mix(
        mix(pixel(int(p0.x), int(p0.y)), pixel(int(p1.x), int(p0.y)), w.x),
        mix(pixel(int(p0.x), int(p1.y)), pixel(int(p1.x), int(p1.y)), w.x),
        w.y
    );
}

void Image::fetch(vec2 p, vec3& t00, vec3& t10, vec3& t01, vec3& t11,
                  double &w00, double &w10, double &w01, double& w11) const
{
    p -= vec2(0.5);
    vec2 p0 = floor(p);
    vec2 p1 = floor(p + vec2(1));
    vec2 w = fract(p);

    t00 = data[indexOf(int(p0.x), int(p0.y))];
    w00 = (1 - w.x) * (1 - w.y);
    t10 = data[indexOf(int(p1.x), int(p0.y))];
    w10 = (    w.x) * (1 - w.y);
    t01 = data[indexOf(int(p0.x), int(p1.y))];
    w01 = (1 - w.x) * (    w.y);
    t11 = data[indexOf(int(p1.x), int(p1.y))];
    w11 = (    w.x) * (    w.y);
}

void Image::fetchIndex(vec2 p, vec3& p00, vec3& p10, vec3& p01, vec3& p11) const
{
    p -= vec2(0.5);
    vec2 p0 = floor(p);
    vec2 p1 = floor(p + vec2(1));
    vec2 w = fract(p);

    p00 = vec3(p0.x, p0.y, (1 - w.x) * (1 - w.y));
    p10 = vec3(p1.x, p0.y, (    w.x) * (1 - w.y));
    p01 = vec3(p0.x, p1.y, (1 - w.x) * (    w.y));
    p11 = vec3(p1.x, p1.y, (    w.x) * (    w.y));
}

// returns true when p lies in the ``left'' half-plane
static inline bool isInside(vec2 l0, vec2 l1, vec2 p)
{
    vec2 l = l1 - l0;
    vec2 n(l.y, -l.x);
    return dot(p - l0, n) >= 0;
}

unsigned Image::setMaskInternal(const Mesh& m)
{
    unsigned n = 0;
    // FIXME does not work with polygonal faces
    vec2 imgsz(resx, resy);
    for (const Face& f : m.face) {
        int minx = std::numeric_limits<int>::max();
        int miny = std::numeric_limits<int>::max();
        int maxx = std::numeric_limits<int>::min();
        int maxy = std::numeric_limits<int>::min();
        for (unsigned t : f.ti) {
            vec2 tc = m.vtvec[t] * imgsz;
            minx = min(minx, int(tc.x));
            miny = min(miny, int(tc.y));
            maxx = max(maxx, int(tc.x));
            maxy = max(maxy, int(tc.y));
        }
        minx--;
        miny--;
        maxx++;
        maxy++;

        for (int y = miny; y <= maxy; ++y)
        for (int x = minx; x <= maxx; ++x) {
            Edge e0 = f.edge2(0);
            Edge e1 = f.edge2(1);
            Edge e2 = f.edge2(2);
            bool ins0 = isInside(m.vtvec[e0.first] * imgsz, m.vtvec[e0.second] * imgsz, vec2(x, y));
            bool ins1 = isInside(m.vtvec[e1.first] * imgsz, m.vtvec[e1.second] * imgsz, vec2(x, y));
            bool ins2 = isInside(m.vtvec[e2.first] * imgsz, m.vtvec[e2.second] * imgsz, vec2(x, y));
            if ((ins0 == ins1) && (ins1 == ins2)) {
                if (!(mask(x, y) & MaskBit::Internal)) {
                    mask(x, y) |= MaskBit::Internal;
                    n++;
                }
            }
        }
    }
    return n;
}

unsigned Image::setMaskSeam(const Mesh& m)
{
    using ::Seam;

    unsigned n = 0;
    vec2 imgsz(resx, resy);
    for (const Seam& s : m.seam) {
        double d = m.maxLength(s, imgsz);
        for (double t = 0; t <= 1; t += 1 / (SEAM_SAMPLING_FACTOR*d)) {
            vec3 p[8];
            fetchIndex(m.uvpos(s.first, t) * imgsz, p[0], p[1], p[2], p[3]);
            fetchIndex(m.uvpos(s.second, t) * imgsz, p[4], p[5], p[6], p[7]);
            for (int i = 0; i < 8; ++i) {
                //mask[indexOf(int(p[i].x), int(p[i].y))] = std::max(mask[indexOf(int(p[i].x), int(p[i].y))], p[i].z);
                int px = int(p[i].x);
                int py = int(p[i].y);
                if (!(mask(px, py) & MaskBit::Seam)) {
                    mask(px, py) |= MaskBit::Seam;
                    n++;
                }
            }
            /*
            fetchIndex(m.uvpos(s.second, t) * imgsz, p[0], p[1], p[2], p[3]);
            for (int i = 0; i < 4; ++i) {
                //mask[indexOf(int(p[i].x), int(p[i].y))] = std::max(mask[indexOf(int(p[i].x), int(p[i].y))], p[i].z);
                mask(int(p[i].x), int(p[i].y)) = MaskBit::Seam;
            }
            */
        }
    }
    return n;
}

void Image::clearMask()
{
    mask_.clear();
    mask_.resize(resx * resy, 0);
}
