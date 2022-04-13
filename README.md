# Seamless Compressed Textures

## Build instructions

Building requires QT5 and CMake:

```
  mkdir build && cd build
  cmake ..
  make
```

## Running

The executable takes as input a UV-mapped OBJ file and a texture image,
and produces the following set of files (obj and textures):

```
<FILENAME>_s.*    Seamless uncompressed
<FILENAME>_sc.*   Seamless, then compressed
<FILENAME>_sc_s.* Seamless, then compressed with integrated seam-masking
```

Compressed textures are saved both as PNG for convenience and compressed .dds files

