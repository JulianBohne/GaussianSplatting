#ifndef SPLAT_LOADER_H
#define SPLAT_LOADER_H
#include <stddef.h>

#include "gaussian.h"

typedef struct Splats {
    Gaussian* splats;
    size_t splatCount;
} Splats;

Splats loadSplatsFromFile(const char* filePath);

#endif // SPLAT_LOADER_H
