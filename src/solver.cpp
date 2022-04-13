#include <cmath>

#include "solver.h"
#include "image.h"

#include <memory>

Solver::Solver()
{

}

void Solver::fixSeams(const Mesh& m, Image& img)
{
    resx = img.resx;
    resy = img.resy;

    vi.clear();
    vi.resize(resx * resy, -1);

    sys.clear();

    // be seamless
    for (const Seam& s : m.seam) {
        double d = m.maxLength(s, vec2(resx, resy));
        for (double t = 0; t <= 1; t += 1 / (2*d)) {
            sys.addEquation(
                pixel(m.uvpos(s.first, t) * vec2(resx, resy)) == pixel(m.uvpos(s.second, t) * vec2(resx, resy))
            );
        }
    }

    LinearEquationSet s1 = sys;
    sys.printShort();
    s1.printShort();

    //std::vector<scalar> init(vi.size() * 3, 0);

    // be yourself
    for (int y = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x) {
        if (vi[indexOf(x, y)] != -1) {
            double w = (img.mask(x, y) & Image::MaskBit::Internal) ? 1.0 : 0.1;
            //double w = 0.01;
            sys.addEquation(w * (
                pixel(x, y) == img.pixel(x, y)
            ));
            //init[vi[indexOf(x, y)] + 0] = img.pixel(x, y).r;
            //init[vi[indexOf(x, y)] + 1] = img.pixel(x, y).g;
            //init[vi[indexOf(x, y)] + 2] = img.pixel(x, y).b;
        }
    }

    sys.printShort();
    std::vector<scalar> vars;
    sys.initializeVars(vars);

    LinearEquationSet s2 = sys;
    s2.eq.erase(s2.eq.begin(), s2.eq.begin() + s1.eq.size());
    s2.solve(vars);

    double e1_tot = sys.squaredErrorFor(vars);
    double e1_seamless = s1.squaredErrorFor(vars);
    double e1_id = s2.squaredErrorFor(vars);
    sys.solve(vars);
    double e2_tot = sys.squaredErrorFor(vars);
    double e2_seamless = s1.squaredErrorFor(vars);
    double e2_id = s2.squaredErrorFor(vars);

    std::cout << "Error tot " << e1_tot << " -> " << e2_tot << std::endl;
    std::cout << "Error seamless " << e1_seamless << " -> " << e2_seamless << std::endl;
    std::cout << "Error identity " << e1_id << " -> " << e2_id << std::endl;

    assert(std::abs(e1_tot - (e1_seamless + e1_id)) < 0.001);

    for (int y = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x) {
        if (vi[indexOf(x, y)] != -1) {
            img.pixel(x, y) = glm::clamp(pixel(x, y).evaluateFor(vars), vec3(0), vec3(255));
        }
    }
}

void Solver::fixSeamsMIP(const Mesh& m, Image& img, const Image& img0, const std::vector<int>& cover0)
{
    resx = img.resx;
    resy = img.resy;

    vi.clear();
    vi.resize(resx * resy, -1);

    sys.clear();

    //m.generateTextureCoverageBuffer(img, cover, false);

    // be seamless
    /*
    for (const Seam& s : m.seam) {
        vec2 img0sz(img0.resx, img0.resy);
        double d = m.maxLength(s, img0sz);
        for (double t = 0; t <= 1; t += 1 / (2*d)) {
            sys.addEquation(
                pixel(m.uvpos(s.first, t) * vec2(resx, resy)) == img0.pixel(m.uvpos(s.first, t) * img0sz)
            );

            sys.addEquation(
                pixel(m.uvpos(s.first, t) * vec2(resx, resy)) == img0.pixel(m.uvpos(s.second, t) * img0sz)
            );

            sys.addEquation(
                pixel(m.uvpos(s.second, t) * vec2(resx, resy)) == img0.pixel(m.uvpos(s.second, t) * img0sz)
            );

            sys.addEquation(
                pixel(m.uvpos(s.second, t) * vec2(resx, resy)) == img0.pixel(m.uvpos(s.first, t) * img0sz)
            );

        }
    }
    */

    /*
    for (unsigned i = 0; i < vi.size(); ++i) {
        if (vi[i] != -1) {
            cover[i] = 1;
        }
    }
    */


    // be yourself
    /*
    for (int y = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x) {
        if (vi[indexOf(x, y)] != -1) {
            double w = cover[img.indexOf(x, y)] ? 0.1 : 0.1;
            sys.addEquation(w * (
                pixel(x, y) == img.pixel(x, y)
            ));
        }
    }
    */

    // be what you used to be at the finest level
    for (int y = 0; y < img0.resy; ++y)
    for (int x = 0; x < img0.resx; ++x) {
        //if (vi[indexOf(x, y)] != -1) {
            double w = cover0[img0.indexOf(x, y)] ? 1 : 0.001;
            //double w = 0.01;
            sys.addEquation(w * (
                pixel(x / (img0.resx / resx), y / (img0.resy / resy)) == img0.pixel(x, y)
            ));

        //}

   }

    std::vector<scalar> vars;
    sys.initializeVars(vars);

    double e1 = sys.squaredErrorFor(vars);
    sys.solve(vars);
    double e2 = sys.squaredErrorFor(vars);

    std::cout << "error " << e1 << " -> " << e2 << std::endl;

    for (int y = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x) {
        if (vi[indexOf(x, y)] != -1) {
            img.pixel(x, y) = pixel(x, y).evaluateFor(vars);
        }
    }
}

bool Solver::active(const ivec2& p) const
{
    int i = indexOf(p.x, p.y);
    return cover[i];
    //return cover[i] || (vi[i] != -1);
}

int Solver::indexOf(int x, int y) const
{
    x = (x + resx) % resx;
    y = (y + resy) % resy;
    return y * resx + x;
}

LinearVec3 Solver::pixel(int x, int y)
{
    int i = indexOf(x, y);
    if (vi[i] == -1) {
        vi[i] = sys.nvar;
        return sys.newLinearVec3();
    } else {
        return LinearVec3(variable(vi[i]), variable(vi[i] + 1), variable(vi[i] + 2));
    }
}

LinearVec3 Solver::pixel(vec2 p)
{
    p -= vec2(0.5);
    vec2 p0 = floor(p);
    vec2 p1 = floor(p + vec2(1));
    vec2 w = fract(p);
    return mix(
        mix(pixel(int(p0.x), int(p0.y)), pixel(int(p1.x), int(p0.y)), scalar(w.x)),
        mix(pixel(int(p0.x), int(p1.y)), pixel(int(p1.x), int(p1.y)), scalar(w.x)),
        scalar(w.y)
    );
}

SolverCompressedImage::SolverCompressedImage()
    : cptr{nullptr}
{

}

void SolverCompressedImage::fixSeams(const Mesh& m, const Image& img, CompressedImage& cimg, const std::set<int>& fixedBlocks)
{
    resx = img.resx;
    resy = img.resy;

    //m.generateTextureCoverageBuffer(img, cover, false);

    vi.clear();
    vi.resize(cimg.nblk() * 2, -1);

    sys.clear();

    cptr = &cimg;

    // be seamless
    for (const Seam& s : m.seam) {
        double d = m.maxLength(s, vec2(resx, resy));
        for (double t = 0; t <= 1; t += 1 / (2*d)) {
            sys.addEquation(
                pixel(m.uvpos(s.first, t) * vec2(resx, resy)) == pixel(m.uvpos(s.second, t) * vec2(resx, resy))
            );
        }
    }
    sys.printShort();

    LinearEquationSet s1 = sys;

    // be yourself
    for (int y = 0; y < resy; ++y)
    for (int x = 0; x < resx; ++x) {
        int bx = x / 4;
        int by = y / 4;
        if ((vi[indexOf(bx, by, 0)] != -1) || (vi[indexOf(bx, by, 1)] != -1)) {
            double w = (img.mask(x, y) & Image::MaskBit::Internal) ? 1 : 0.1;
            sys.addEquation(w * (
                pixel(x, y) == img.pixel(x, y)
            ));
        }
    }
    sys.printShort();

    std::vector<scalar> vars(sys.nvar, 10);
    //sys.initializeVars(vars);

    LinearEquationSet s2 = sys;
    s2.eq.erase(s2.eq.begin(), s2.eq.begin() + s1.eq.size());
    s2.solve(vars); // first solve with only identity constraints to initialize value


    //std::cout << "there are " << fixedBlocks.size() << " fixed blocks" << std::endl;
    int k = 0;
    for (int i : fixedBlocks) {
        int v0 = vi[2 * i];
        int v1 = vi[2 * i + 1];

        if (v0 != -1) {
            LinearVec3 c0 = LinearVec3(v0, v0 + 1, v0 + 2);
            sys.addEquation(10000 * (c0 == cimg.getBlock(i).c0));
            k++;
        }

        if (v1 != -1) {
            LinearVec3 c1 = LinearVec3(v1, v1 + 1, v1 + 2);
            sys.addEquation(10000 * (c1 == cimg.getBlock(i).c1));
            k++;
        }
    }
    //std::cout << " Added " << k << " equations" << std::endl;
    sys.printShort();

    double e1_tot = sys.squaredErrorFor(vars);
    double e1_seamless = s1.squaredErrorFor(vars);
    double e1_id = s2.squaredErrorFor(vars);
    sys.solve(vars);
    double e2_tot = sys.squaredErrorFor(vars);
    double e2_seamless = s1.squaredErrorFor(vars);
    double e2_id = s2.squaredErrorFor(vars);

    std::cout << "Error tot " << e1_tot << " -> " << e2_tot << std::endl;
    std::cout << "Error seamless " << e1_seamless << " -> " << e2_seamless << std::endl;
    std::cout << "Error identity " << e1_id << " -> " << e2_id << std::endl;

    for (int by = 0; by < resy/4; ++by)
    for (int bx = 0; bx < resx/4; ++bx)
    for (int ci = 0; ci < 2; ++ci) {
        int i = vi[indexOf(bx, by, ci)];
        if (i != -1) {
            LinearVec3 v(i, i + 1, i + 2);
            vec3 cval = glm::clamp(v.evaluateFor(vars), vec3(0), vec3(255));
            cimg.setBlockColor(bx, by, ci, cval);
        }
    }

    cptr = nullptr;
}

int SolverCompressedImage::indexOf(int bx, int by, int ci) const
{
    return (by * (resx / 4) + bx) * 2 + ci;
}

LinearVec3 SolverCompressedImage::pixel(vec2 p)
{
    p -= vec2(0.5);
    vec2 p0 = floor(p);
    vec2 p1 = floor(p + vec2(1));
    vec2 w = fract(p);
    return mix(
        mix(pixel(int(p0.x), int(p0.y)), pixel(int(p1.x), int(p0.y)), scalar(w.x)),
        mix(pixel(int(p0.x), int(p1.y)), pixel(int(p1.x), int(p1.y)), scalar(w.x)),
        scalar(w.y)
    );
}

LinearVec3 SolverCompressedImage::blockVars(int bx, int by, int ci)
{
    int i = indexOf(bx, by, ci);
    if (vi[i] == -1) {
        vi[i] = sys.nvar;
        return sys.newLinearVec3();
    } else {
        return LinearVec3(variable(vi[i] + 0), variable(vi[i] + 1), variable(vi[i] + 2));
    }
}

LinearVec3 SolverCompressedImage::pixel(int x, int y)
{
    unsigned char bitmask = cptr->getMask(x, y);

    x = x / 4;
    y = y / 4;

    switch (bitmask) {
    case QMASK_C0:
        return blockVars(x, y, 0);
    case QMASK_C0_23_C1_13:
        return blockVars(x, y, 0) * (2.0 / 3.0)
             + blockVars(x, y, 1) * (1.0 / 3.0);
    case QMASK_C0_13_C1_23:
        return blockVars(x, y, 1) * (2.0 / 3.0)
             + blockVars(x, y, 0) * (1.0 / 3.0);
    case QMASK_C1:
        return blockVars(x, y, 1);
    default:
        assert(0 && "Solver: invalid bit mask");
    }
}
