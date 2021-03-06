# YCC Colorspace Conversion

Embedded systems course project. Tests various implementations of RGB to YcbCr 4:2:0 colorspace conversion on an ARM-based system.

## Contents 

### Colorspace conversion code

src/cc/*

### Colorspace conversion hardware description

hardware-desc/cc.sv

### Support code (main function, image file I/O wrapper)

src/main.cpp
src/image.hpp

### Third-party code (PNG image loader)

src/lodepng/lodepng.cpp

src/lodepng/lodepng.h

## Compilation

Written for Raspberry Pi 4. Use provided CMakeLists.txt to compile.

To run:
./cc [option]

Performs conversion on test.png

Generates YCbCr data as ycc-result.raw

Use script im-test.sh to convert ycc-result.raw back to RGB image (back-conv.png)

#### Valid options:

* _float_ : Floating-point conversion
* _naive_ : Naive sequential conversion
* _fixed_ : Optimized fixed-point sequential conversion
* _vector_ : ARM NEON vectorized conversion
* _vector2_ : ARM NEON vectorized conversion (combined algorithm)
* _hardware_ : Memory-mapped hardware conversion
