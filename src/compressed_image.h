#ifndef COMPRESSED_IMAGE_H
#define COMPRESSED_IMAGE_H

#include <glm/common.hpp>

#include <vector>

class Image;

using namespace glm;

typedef struct __attribute__ ((packed)) {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
} DDS_PIXELFORMAT ;

typedef struct __attribute__ ((packed)) {
    uint32_t        dwSize;
    uint32_t        dwFlags;
    uint32_t        dwHeight;
    uint32_t        dwWidth;
    uint32_t        dwPitchOrLinearSize;
    uint32_t        dwDepth;
    uint32_t        dwMipMapCount;
    uint32_t        dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t        dwCaps;
    uint32_t        dwCaps2;
    uint32_t        dwCaps3;
    uint32_t        dwCaps4;
    uint32_t        dwReserved2;
} DDS_HEADER;

struct Block {
    vec3 c0;
    vec3 c1;
    unsigned char bit[16];
};

typedef struct __attribute__ ((packed)) {
    uint16_t c0;
    uint16_t c1;
    uint32_t index;
} CompressedBlock;

const unsigned char QMASK_C0 = 0;
const unsigned char QMASK_C0_23_C1_13 = 2;
const unsigned char QMASK_C0_13_C1_23 = 3;
const unsigned char QMASK_C1 = 1;

struct BlockErrorData {
    int blkIndex;
    float minError;
    float maxError;
    float avgError;
};

class CompressedImage {

public:

    static vec2 getWeights(unsigned char bitmask);

private:

    DDS_HEADER generateHeader() const;
    DDS_PIXELFORMAT generatePixelFormat() const;

public:

    std::vector<Block> data;

    CompressedImage() {}

    int resx;
    int resy;

    void initialize(const Image& img, uint8_t bitmask);
    std::vector<BlockErrorData> computePerBlockError(const Image& img) const;

    /* (virtual) 16 bit quantization of block colors */
    void quantizeBlocks();

    void save(const char *filename) const;
    bool saveUncompressed(const char *path) const;

    unsigned nblk() const;
    int getBlockIndex(int x, int y) const;

    Block& getBlock(int x, int y);
    Block& getBlock(int i);

    const Block& getBlock(int x, int y) const;

    unsigned char getMask(int x, int y) const;

    void setBlockColor(int x, int y, int ci, vec3 c);
    vec3 pixel(int x, int y) const;


};

#endif // COMPRESSION_H
