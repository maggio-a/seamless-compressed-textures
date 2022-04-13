#ifndef SOLVER_H
#define SOLVER_H

#include "mesh.h"
#include "lineareq.h"

#include "compressed_image.h"

#include <set>

class Solver
{
    friend void fixArtifacts(Mesh& m, Image& img);

    LinearEquationSet sys;

    std::vector<int> vi; // per pixel variable index
    std::vector<int> cover; // per pixel coverage buffer

    int resx;
    int resy;

public:
    Solver();

    void fixSeams(const Mesh& m, Image& img);

    void fixSeamsMIP(const Mesh& m, Image& img, const Image& img0, const std::vector<int>& cover);

    //Image generateNextMipLevel(const Mesh& m, Image& imgCurr);
    void generateNextMipLevel(const Mesh& m, const Image& imgCurr, Image& next);

    // multi channel
    LinearVec3 pixel(int x, int y);
    LinearVec3 pixel(vec2 p); // bilinear interpolation

#if 0
    // single channel
    LinearExp pixelExp(int x, int y);
    LinearExp pixelExp(vec2 p); // bilinear interpolation
#endif

    bool active(const ivec2& p) const; // returns true if the pixel is a system variable or covered
    int indexOf(int x, int y) const; // TODO copy to img

};

void fixArtifacts(Mesh &m, Image& img);

class SolverCompressedImage {
    LinearEquationSet sys; // same as solver
    std::vector<int> vi; // same as solver
    std::vector<int> cover; // same as solver
    int resx; // same as solver
    int resy; // same as solver

    CompressedImage *cptr;

public:
    SolverCompressedImage();

    void fixSeams(const Mesh& m, const Image& img, CompressedImage& cimg, const std::set<int>& fixedBlocks);

    int indexOf(int bx, int by, int ci) const;
    LinearVec3 pixel(int x, int y);
    LinearVec3 pixel(vec2 p); // same as Solver
    LinearVec3 blockVars(int bx, int by, int ci);


};

#endif // SOLVER_H
