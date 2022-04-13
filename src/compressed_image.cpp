#include "compressed_image.h"
#include "image.h"
#include "line.h"

#include <cassert>
#include <fstream>

#include <glm/geometric.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>


static CompressedBlock compressBlock(const Block& blk);
static float optimizeEndpoints(const std::vector<vec3>& cblk, const std::vector<uint8_t>& mblk, Block& blk);
static void findColorInterval(const std::vector<vec3>& cblk, const Line3& line, vec3& c0, vec3& c1);
static unsigned char getQuantizationMask(const vec3& color, const vec3& c0, const vec3& c1);
static unsigned char swappedMask(unsigned char mask);
static Block computeBlock(const std::vector<vec3>& cblk, const std::vector<uint8_t>& mblk, uint8_t bitmask);
static vec3 getColor(const Block& blk, int i);

static uint16_t quantizeColor(const vec3& color);
static vec3 quantized2rgb(uint16_t c);


vec2 CompressedImage::getWeights(unsigned char bitmask)
{
    switch (bitmask) {
    case QMASK_C0:
        return vec2(1, 0);
    case QMASK_C0_23_C1_13:
        return vec2(2.0f / 3.0f, 1.0f / 3.0f);
    case QMASK_C0_13_C1_23:
        return vec2(1.0f / 3.0f, 2.0f / 3.0f);
    case QMASK_C1:
        return vec2(0, 1);
    default:
        assert(0 && "getWeights(): invalid bit mask");
    }
}

void CompressedImage::initialize(const Image& img, uint8_t bitmask)
{
    assert(img.resx % 4 == 0);
    assert(img.resy % 4 == 0);

    resx = img.resx;
    resy = img.resy;

    data.clear();
    data.reserve((resx * resy) / 16);

    for (int y = 0; y < resy / 4; ++y)
    for (int x = 0; x < resx / 4; ++x) {
        std::vector<vec3> cblk;
        std::vector<uint8_t> mblk;
        for (int h = 0; h < 4; ++h)
        for (int k = 0; k < 4; ++k) {
            cblk.push_back(img.pixel(4 * x + k, 4 * y + h));
            mblk.push_back(img.mask(4 * x + k, 4 * y + h));
        }
        Block blk = computeBlock(cblk, mblk, bitmask);
        data.push_back(blk);
    }
}

std::vector<BlockErrorData> CompressedImage::computePerBlockError(const Image& img) const
{
    assert(resx == img.resx);
    assert(resy == img.resy);

    std::vector<BlockErrorData> perBlockError;
    for (int by = 0; by < resy / 4; ++by)
    for (int bx = 0; bx < resx / 4; ++bx) {
        std::vector<vec3> cblk;
        std::vector<uint8_t> mblk;
        for (int h = 0; h < 4; ++h)
        for (int k = 0; k < 4; ++k) {
            cblk.push_back(img.pixel(4 * bx + k, 4 * by + h));
            mblk.push_back(img.mask(4 * bx + k, 4 * by + h));
        }

        int blkIndex = getBlockIndex(4 * bx, 4 * by);

        float minError = 1e10;
        float maxError = 0;
        float totalError = 0;
        int n = 0;
        for (unsigned i = 0; i < 16; ++i) {
            if (mblk[i] & (Image::MaskBit::Internal | Image::MaskBit::Seam)) {
                n++;
                vec3 c = getColor(data[blkIndex], i);
                vec3 src = cblk[i];
                float dist = glm::distance(c, src);
                minError = std::min(minError, dist);
                maxError = std::max(maxError, dist);
                totalError += dist;
            }
        }

        if (n > 0)
            perBlockError.push_back({blkIndex, minError, maxError, totalError / n});
        else
            perBlockError.push_back({blkIndex, 0, 0, 0});
    }
    return perBlockError;
}

DDS_PIXELFORMAT CompressedImage::generatePixelFormat() const
{
    DDS_PIXELFORMAT pf = {};
    pf.dwSize = 32;
    pf.dwFlags = 0x4; // TODO
    pf.dwFourCC = ((uint32_t)('D')) | ((uint32_t)('X') << 8) | ((uint32_t)('T') << 16) | ((uint32_t)('1') << 24);
    pf.dwRGBBitCount = 0;
    pf.dwRBitMask = 0;
    pf.dwGBitMask = 0;
    pf.dwBBitMask = 0;
    pf.dwABitMask = 0;
    return pf;
}

// https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide
DDS_HEADER CompressedImage::generateHeader() const
{
    DDS_HEADER header = {};
    // set header
    header.dwSize = 124;
    header.dwFlags = 0x1 | 0x2 | 0x4 | 0x1000; // TODO
    header.dwHeight = resy;
    header.dwWidth = resx;
    header.dwPitchOrLinearSize = 0;
    header.dwDepth = 0;
    header.dwMipMapCount = 1;
    //header.dwReserved1[11] = 0;
    header.dwCaps = 0x1000; // TODO
    header.dwCaps2 = 0;
    header.dwCaps3 = 0;
    header.dwCaps4 = 0;
    header.dwReserved2 = 0;
    // set pixelformat structure
    header.ddspf = generatePixelFormat();
    return header;
}

void CompressedImage::save(const char *filename) const
{
    uint32_t dwMagic = 0x20534444;
    DDS_HEADER dwHeader = generateHeader();

    std::ofstream dds(filename, std::ios::binary);
    dds.write(reinterpret_cast<char *>(&dwMagic), sizeof(uint32_t));
    dds.write(reinterpret_cast<char *>(&dwHeader), sizeof(DDS_HEADER));
    for (const Block& blk : data) {
        CompressedBlock qb = compressBlock(blk);
        dds.write(reinterpret_cast<char *>(&qb), sizeof(CompressedBlock));
    }
    dds.close();
}

unsigned CompressedImage::nblk() const
{
    return data.size();
}

int CompressedImage::getBlockIndex(int x, int y) const
{
    return (y/4) * (resx/4) + (x/4);
}

Block& CompressedImage::getBlock(int x, int y)
{
    return data[getBlockIndex(x, y)];
}

const Block& CompressedImage::getBlock(int x, int y) const
{
    return data[getBlockIndex(x, y)];
}

Block& CompressedImage::getBlock(int i)
{
    return data[i];
}

unsigned char CompressedImage::getMask(int x, int y) const
{
    x = (x + resx) % resx;
    y = (y + resy) % resy;
    const Block& blk = getBlock(x, y);
    return blk.bit[(y % 4) * 4 + (x % 4)];
}

void CompressedImage::setBlockColor(int bx, int by, int ci, vec3 c)
{
    int bi = by * (resx/4) + (bx);
    if (ci == 0)
        data[bi].c0 = c;
    else
        data[bi].c1 = c;
}

vec3 CompressedImage::pixel(int x, int y) const
{
    const Block& blk = getBlock(x, y);
    unsigned char bitmask = getMask(x, y);
    vec2 w = getWeights(bitmask);
    return glm::mix(blk.c0, blk.c1, w.y);
}

void CompressedImage::quantizeBlocks()
{
    for (Block& blk : data) {

        assert(blk.c0.r >= 0 && "pre");
        assert(blk.c0.g >= 0 && "pre");
        assert(blk.c0.b >= 0 && "pre");
        assert(blk.c1.r >= 0 && "pre");
        assert(blk.c1.g >= 0 && "pre");
        assert(blk.c1.b >= 0 && "pre");

        uint16_t c0 = quantizeColor(blk.c0);
        uint16_t c1 = quantizeColor(blk.c1);
        blk.c0 = quantized2rgb(c0);
        assert(blk.c0.r >= 0);
        assert(blk.c0.g >= 0);
        assert(blk.c0.b >= 0);
        blk.c1 = quantized2rgb(c1);

        assert(blk.c1.r >= 0);
        assert(blk.c1.g >= 0);
        assert(blk.c1.b >= 0);

    }
}


// -- static functions ---------------------------------------------------------


static CompressedBlock compressBlock(const Block& blk)
{
    CompressedBlock cb = {0, 0, 0};
    cb.c0 = quantizeColor(blk.c0);
    cb.c1 = quantizeColor(blk.c1);

    bool swapped = false;
    if (cb.c0 < cb.c1) {
        uint16_t c0 = cb.c0;
        cb.c0 = cb.c1;
        cb.c1 = c0;
        swapped = true;
    }

    for (unsigned i = 0; i < 16; ++i) {
        uint32_t mask = swapped ? swappedMask(blk.bit[i]) : blk.bit[i];
        mask = ((cb.c0 == cb.c1 ? 0 : mask) << (2 * i));
        cb.index |= mask;
    }
    return cb;
}

static float optimizeEndpoints(const std::vector<vec3>& cblk, const std::vector<uint8_t>& mblk, uint8_t bitmask, Block& blk)
{
    std::vector<int> ind;
    for (unsigned i = 0; i < cblk.size(); ++i)
        if ((!bitmask) || (mblk[i] & bitmask))
           ind.push_back(i);

    Eigen::MatrixXf A(ind.size(), 2);
    Eigen::MatrixXf B(ind.size(), 3);

    for (unsigned i = 0; i < ind.size(); ++i) {
        vec2 w = CompressedImage::getWeights(blk.bit[ind[i]]);
        A.row(i) = Eigen::Vector2f(w.x, w.y);
        for (int j = 0; j < 3; ++j)
            B(i, j) = cblk[ind[i]][j];
    }
    Eigen::MatrixXf AtA = A.transpose() * A;
    Eigen::MatrixXf AtB = A.transpose() * B;

    float r = 0;

    for (int j = 0; j < 3; ++j) {
        Eigen::Vector2f xj = AtA.ldlt().solve(AtB.col(j));
        blk.c0[j] = xj[0];
        blk.c1[j] = xj[1];

        Eigen::VectorXf rvec = A * xj - B.col(j);

        r += rvec.squaredNorm();
    }

    blk.c0 = glm::clamp(blk.c0, vec3(0, 0, 0), vec3(255, 255, 255));
    blk.c1 = glm::clamp(blk.c1, vec3(0, 0, 0), vec3(255, 255, 255));

    return r;
}

static void findColorInterval(const std::vector<vec3>& cblk, const Line3& line, vec3& c0, vec3& c1)
{
    //float tmin = std::numeric_limits<float>::max();
    //float tmax = std::numeric_limits<float>::lowest();

    assert(std::abs(glm::length(line.d) - 1.0f) < 1e-6);

    float tmin = 0;
    float tmax = 0;

    for (unsigned i = 0; i < cblk.size(); ++i) {
        vec3 cvec = cblk[i] - line.o;
        float t = glm::dot(cvec, line.d);
        tmin = std::min(t, tmin);
        tmax = std::max(t, tmax);
    }
    c0 = glm::clamp(line(tmin), vec3(0), vec3(255));
    c1 = glm::clamp(line(tmax), vec3(0), vec3(255));
}

static unsigned char getQuantizationMask(const vec3& color, const vec3& c0, const vec3& c1)
{
    const float a = 2.0f/3.0f;
    const float b = 1.0f/3.0f;
    vec3 c2 = a * c0 + b * c1;
    vec3 c3 = b * c0 + a * c1;

    float dmin = std::numeric_limits<float>::max();
    float d;
    unsigned char mask = 0xff;

    if ((d = distance(color, c0)) < dmin) {
        dmin = d;
        mask = QMASK_C0;
    }

    if ((d = distance(color, c2)) < dmin) {
        dmin = d;
        mask = QMASK_C0_23_C1_13;
    }

    if ((d = distance(color, c3)) < dmin) {
        dmin = d;
        mask = QMASK_C0_13_C1_23;
    }

    if ((d = distance(color, c1)) < dmin) {
        dmin = d;
        mask = QMASK_C1;
    }
    assert(mask != 0xff);

    return mask;
}

static unsigned char swappedMask(unsigned char mask)
{
    switch (mask) {
    case QMASK_C0:
        return QMASK_C1;
    case QMASK_C0_23_C1_13:
        return QMASK_C0_13_C1_23;
    case QMASK_C0_13_C1_23:
        return QMASK_C0_23_C1_13;
    case QMASK_C1:
        return QMASK_C0;
    default:
        assert(0 && "Invalid mask");
    }
}

#include <iostream>
// cblk is a 4x4 block of pixels stored by row
static Block computeBlock(const std::vector<vec3>& cblk, const std::vector<uint8_t> &mblk, uint8_t bitmask)
{
    Block blk;

    std::vector<vec3> cblkPosWeight;
    assert(mblk.size() == 16);
    for (unsigned i = 0; i < mblk.size(); ++i)
        if ((!bitmask) || (mblk[i] & bitmask))
            cblkPosWeight.push_back(cblk[i]);

    if (cblkPosWeight.size() == 0)
        //cblkPosWeight.push_back(cblk.front());
        cblkPosWeight.push_back(vec3(0, 0, 0));

    Line3 line = fitLine(cblkPosWeight);
    findColorInterval(cblkPosWeight, line, blk.c0, blk.c1);

    for (unsigned i = 0; i < 16; ++i) {
        blk.bit[i] = getQuantizationMask(cblk[i], blk.c0, blk.c1);
    }

    if (cblkPosWeight.size() > 2)
        optimizeEndpoints(cblk, mblk, bitmask, blk);

#if 0
    // iterative version
    if (cblkPosWeight.size() > 2) {
        float rmin = std::numeric_limits<float>::max();
        while (true) {
            k++;
            float r = optimizeEndpoints(cblk, mblk, bitmask, blk);
            if (r >= rmin) {
                break;
            } else {
                rmin = r;
                for (unsigned i = 0; i < 16; ++i) {
                    blk.bit[i] = getQuantizationMask(cblk[i], blk.c0, blk.c1);
                }
            }
        }
    }
#endif

    return blk;
}

static vec3 getColor(const Block& blk, int i)
{
    assert(i >= 0);
    assert(i < 16);
    vec2 w = CompressedImage::getWeights(blk.bit[i]);
    return glm::mix(blk.c0, blk.c1, w.y);
}

static uint16_t quantizeColor_(const vec3& color)
{
    vec3 qc = (color / vec3(256.0f)) * vec3(32.0f, 64.0f, 32.0f);
    qc = glm::clamp(qc, vec3(0), vec3(31.0f, 63.0f, 31.0f));

    uint16_t c = 0;
    c |= (uint16_t(qc.x) << 11);
    c |= (uint16_t(qc.y) << 5);
    c |= uint16_t(qc.z);
    return c;
}

static uint16_t quantizeColor(const vec3& color)
{
    uint16_t r16 = std::round(color.r);
    uint16_t g16 = std::round(color.g);
    uint16_t b16 = std::round(color.b);
    uint16_t p = ((r16 >> 3) << 11) | ((g16 >> 2) << 5) | (b16 >> 3);
    return p;
}

static inline vec3 quantized2rgb(uint16_t c)
{
    return vec3((c >> 11)           * (255.0f / 31.0f),
               ((c >>  5) & 0x003f) * (255.0f / 63.0f),
               ((c        & 0x001f) * (255.0f / 31.0f)));
}
