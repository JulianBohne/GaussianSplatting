#ifndef GAUSSIAN_H
#define GAUSSIAN_H

typedef struct Vec4 {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

typedef struct Gaussian {
    Vec4 position; // Vec4 because we'll use homogeneous coordinates for this
    Vec4 rotation;
    Vec4 scale; // Vec4 for alignment lol
    Vec4 color;
} Gaussian;

typedef struct T_Gaussian {
    Vec4 position;
    float invCovariance[4];
    unsigned int depthIndex;
    unsigned int _padding[3]; // Gotta pad it thanks shaders :POG:
} T_Gaussian;

typedef struct DepthStruct {
    unsigned int gaussianIndex;
    float depth;
} DepthStruct;

#endif // GAUSSIAN_H
