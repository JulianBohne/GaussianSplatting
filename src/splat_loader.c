#include "splat_loader.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <raymath.h>

// https://stackoverflow.com/questions/2100331/macro-definition-to-determine-big-endian-or-little-endian-machine

const char expectedHeaderStart[] = "ply\nformat binary_little_endian 1.0\n";
// Then theres a number that we need
const char expectedHeaderRest[] = "property float x\nproperty float y\nproperty float z\nproperty float nx\nproperty float ny\nproperty float nz\nproperty float f_dc_0\nproperty float f_dc_1\nproperty float f_dc_2\nproperty float f_rest_0\nproperty float f_rest_1\nproperty float f_rest_2\nproperty float f_rest_3\nproperty float f_rest_4\nproperty float f_rest_5\nproperty float f_rest_6\nproperty float f_rest_7\nproperty float f_rest_8\nproperty float f_rest_9\nproperty float f_rest_10\nproperty float f_rest_11\nproperty float f_rest_12\nproperty float f_rest_13\nproperty float f_rest_14\nproperty float f_rest_15\nproperty float f_rest_16\nproperty float f_rest_17\nproperty float f_rest_18\nproperty float f_rest_19\nproperty float f_rest_20\nproperty float f_rest_21\nproperty float f_rest_22\nproperty float f_rest_23\nproperty float f_rest_24\nproperty float f_rest_25\nproperty float f_rest_26\nproperty float f_rest_27\nproperty float f_rest_28\nproperty float f_rest_29\nproperty float f_rest_30\nproperty float f_rest_31\nproperty float f_rest_32\nproperty float f_rest_33\nproperty float f_rest_34\nproperty float f_rest_35\nproperty float f_rest_36\nproperty float f_rest_37\nproperty float f_rest_38\nproperty float f_rest_39\nproperty float f_rest_40\nproperty float f_rest_41\nproperty float f_rest_42\nproperty float f_rest_43\nproperty float f_rest_44\nproperty float opacity\nproperty float scale_0\nproperty float scale_1\nproperty float scale_2\nproperty float rot_0\nproperty float rot_1\nproperty float rot_2\nproperty float rot_3\nend_header\n";

#define clamp(x, a, b) (x < a ? a : (x > b ? b : x))
// #define clamp(x, a, b) x

int readFloat(FILE* restrict file, float* restrict output) {
    if (fread((void*)output, sizeof(float), 1, file) != 1) {
        return 1;
    } else {
        return 0;
    }
}

int readGaussian(FILE* restrict file, Gaussian* gaussian) {
    *gaussian = (Gaussian){0};
    float ignore;
    if (readFloat(file, &gaussian->position.x)) {
        fprintf(stderr, "ERROR: Could not read x\n");
        return 1;
    }
    if (readFloat(file, &gaussian->position.y)) {
        fprintf(stderr, "ERROR: Could not read y\n");
        return 1;
    }
    if (readFloat(file, &gaussian->position.z)) {
        fprintf(stderr, "ERROR: Could not read z\n");
        return 1;
    }
    gaussian->position.w = 1.0; // homogenous coordinates

    for (size_t i = 0; i < 3; ++i) {
        readFloat(file, &ignore); // nx, ny, nz
    }
    if (readFloat(file, &gaussian->color.x)) {
        fprintf(stderr, "ERROR: Could not read f_dc_0\n");
        return 1;
    }
    if (readFloat(file, &gaussian->color.y)) {
        fprintf(stderr, "ERROR: Could not read f_dc_1\n");
        return 1;
    }
    if (readFloat(file, &gaussian->color.z)) {
        fprintf(stderr, "ERROR: Could not read f_dc_2\n");
        return 1;
    }
    float zeroth_harmonic;
    if (readFloat(file, &zeroth_harmonic)) {
        fprintf(stderr, "ERROR: Could not read f_rest_0\n");
        return 1;
    }

    gaussian->color.x = clamp(gaussian->color.x, 0.0, 1.0);
    gaussian->color.y = clamp(gaussian->color.y, 0.0, 1.0);
    gaussian->color.z = clamp(gaussian->color.z, 0.0, 1.0);

    // gaussian->color.x = zeroth_harmonic;
    // gaussian->color.x = gaussian->color.x*zeroth_harmonic + 0.5;
    // gaussian->color.y = gaussian->color.y*zeroth_harmonic + 0.5;
    // gaussian->color.z = gaussian->color.z*zeroth_harmonic + 0.5;

    for (size_t i = 1; i <= 44; ++i) {
        readFloat(file, &ignore); // harmonics
    }
    if (readFloat(file, &gaussian->color.w)) {
        fprintf(stderr, "ERROR: Could not read opacity\n");
        return 1;
    }
    // gaussian->color.w = clamp(gaussian->color.w, 0.0, 1.0);
    gaussian->color.w = 1.0;

    if (readFloat(file, &gaussian->scale.x)) {
        fprintf(stderr, "ERROR: Could not read scale_0\n");
        return 1;
    }
    if (readFloat(file, &gaussian->scale.y)) {
        fprintf(stderr, "ERROR: Could not read scale_1\n");
        return 1;
    }
    if (readFloat(file, &gaussian->scale.z)) {
        fprintf(stderr, "ERROR: Could not read scale_2\n");
        return 1;
    }
    gaussian->scale.x = expf(gaussian->scale.x);
    gaussian->scale.y = expf(gaussian->scale.y);
    gaussian->scale.z = expf(gaussian->scale.z);
    gaussian->scale.w = 1.0;

    if (readFloat(file, &gaussian->rotation.x)) {
        fprintf(stderr, "ERROR: Could not read rot_0\n");
        return 1;
    }
    if (readFloat(file, &gaussian->rotation.y)) {
        fprintf(stderr, "ERROR: Could not read rot_1\n");
        return 1;
    }
    if (readFloat(file, &gaussian->rotation.z)) {
        fprintf(stderr, "ERROR: Could not read rot_2\n");
        return 1;
    }
    if (readFloat(file, &gaussian->rotation.w)) {
        fprintf(stderr, "ERROR: Could not read rot_3\n");
        return 1;
    }
    return 0;
}

Splats loadSplatsFromFile(const char* filePath) {

    printf("-------------------------------------------\n");

    FILE* file = fopen(filePath, "rb");
    Splats splats = {0};
    
    const size_t headerStartLen = sizeof(expectedHeaderStart) - 1;
    char actualHeaderStart[headerStartLen];
    
    const size_t headerRestLen = sizeof(expectedHeaderRest) - 1;
    char actualHeaderRest[headerRestLen];

    size_t amountRead;

    size_t splatCount = 0;

    if (!file) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", filePath, strerror(errno));
        goto defer_return;
    }

    amountRead = fread((void*)actualHeaderStart, 1, headerStartLen, file);
    if (amountRead != headerStartLen) {
        fprintf(stderr, "ERROR: Could not read start of header\n");
        goto defer_return;
    }

    if (strncmp(expectedHeaderStart, actualHeaderStart, headerStartLen) != 0) {
        fprintf(stderr, "ERROR: Start of header did not match the expected format\n");
        goto defer_return;
    }

    if (fscanf(file, "element vertex %llu\n", &splatCount) != 1) {
        fprintf(stderr, "ERROR: Could not read vertex count from file\n");
        goto defer_return;
    }

    amountRead = fread((void*)actualHeaderRest, 1, headerRestLen, file);
    if (amountRead != headerRestLen) {
        fprintf(stderr, "ERROR: Could not read rest of header\n");
        goto defer_return;
    }

    if (strncmp(expectedHeaderRest, actualHeaderRest, headerRestLen) != 0) {
        fprintf(stderr, "ERROR: Rest of header did not match the expected format\n");
        goto defer_return;
    }

    printf("Header checked successfully! Splat count: %llu\n", splatCount);

    splats.splats = malloc(splatCount * sizeof(Gaussian));
    if (splats.splats == NULL) {
        fprintf(stderr, "ERROR: Could not malloc gaussians\n");
        goto defer_return;
    }

    splats.splatCount = splatCount;

    for (size_t i = 0; i < splatCount; ++i) {
        if(readGaussian(file, &splats.splats[i])) {
            fprintf(stderr, "ERROR: Could not read gaussian %llu\n", i);
            free((void*)splats.splats);
            splats = (Splats){0};
            goto defer_return;
        }
    }

    defer_return:
    printf("-------------------------------------------\n");
    fclose(file); // If this fails, the universe will collapse (so no need to check it)
    return splats;
}
