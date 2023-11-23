#include "globals_util.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#include <assert.h>

struct Globals {
    float viewMat[4][4];
    float projMat[4][4];
    unsigned int width;
    unsigned int height; 
    unsigned int tileSize; // Remember that there might be padding after this (if I add another matrix)
} globals;

unsigned int globalsId;

float inverseViewRotation[3][3];

float fovy;
float near;
float far;

Vector3 camPos = { 2.500000, 1.000000, 3.000000 };
float camYaw = 2.909999;
float camPitch = -0.270796;

Matrix baseMatrix;
Matrix invBaseMatrix;

void printCurrentParams() {
    printf("Vector3 camPos = { %f, %f, %f }\n", camPos.x, camPos.y, camPos.z);
    printf("float camYaw = %f\n", camYaw);
    printf("float camPitch = %f\n", camPitch);
}

bool InitGlobals(float fieldOfViewY, float nearClip, float farClip) {

    // Aspect ratio
    float a = ((float)GetRenderWidth()) / GetRenderHeight();

    fovy = fieldOfViewY;
    near = nearClip;
    far = farClip;

    float f = 1.0 / tanf(fovy / 2.0);

    baseMatrix = MatrixIdentity();
    invBaseMatrix = MatrixIdentity();

    // Init matrices
    globals = (struct Globals) {
        .viewMat = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        },
        .projMat = {
            {f/a, 0,                         0,                         0},
            {0,   f,                         0,                         0},
            {0,   0, (near + far)/(far - near), (2*near*far)/(far - near)},
            {0,   0,                        -1,                         0}
        },
        .width = GetRenderWidth(),
        .height = GetRenderHeight(),
        .tileSize = 64
    };

    printf("viewMat[0][0] = %f\n", globals.viewMat[0][0]);

    printf("projMat:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
        globals.projMat[0][0],
        globals.projMat[0][1],
        globals.projMat[0][2],
        globals.projMat[0][3],
        globals.projMat[1][0],
        globals.projMat[1][1],
        globals.projMat[1][2],
        globals.projMat[1][3],
        globals.projMat[2][0],
        globals.projMat[2][1],
        globals.projMat[2][2],
        globals.projMat[2][3],
        globals.projMat[3][0],
        globals.projMat[3][1],
        globals.projMat[3][2],
        globals.projMat[3][3]
    );

    // Upload to GPU
    globalsId = rlLoadShaderBuffer(sizeof(globals), &globals, 0); // TODO: Change the 0
    if (!globalsId) {
        fprintf(stderr, "Could not load globals!");
        return false;
    }
    rlBindShaderBuffer(globalsId, 0);

    return true;
}

void UploadGlobalsChangesToGPU() {

    unsigned int width = GetRenderWidth();
    unsigned int height = GetRenderHeight();

    Matrix translation = MatrixTranslate(camPos.x, camPos.y, camPos.z);

    Matrix viewMat = MatrixMultiply(MatrixMultiply(translation, MatrixRotateY(camYaw)), MatrixRotateX(camPitch));

    // Aspect ratio
    float a = ((float)width) / height;

    float f = 1.0 / tanf(fovy / 2.0);

    // Init matrices
    globals = (struct Globals) {
        .viewMat = {
            {viewMat.m0, viewMat.m4, viewMat.m8, viewMat.m12},
            {viewMat.m1, viewMat.m5, viewMat.m9, viewMat.m13},
            {viewMat.m2, viewMat.m6, viewMat.m10, viewMat.m14},
            {0, 0, 0, 1}
        },
        .projMat = {
            {f/a, 0,                         0,                         0},
            {0,   f,                         0,                         0},
            {0,   0, (near + far)/(far - near), (2*near*far)/(far - near)},
            {0,   0,                        -1,                         0}
        },
        .width = width,
        .height = height,
        .tileSize = 64
    };

    rlUpdateShaderBuffer(globalsId, &globals, sizeof(globals), 0); // I'm just gonna upload everything
}

Vector3 GetCamForward() {
    assert(0 && "Not implemented");
    return (Vector3){0, 0, -1};
}

Vector3 GetCamUp() {
    assert(0 && "Not implemented");
    return (Vector3){0, 0, -1};
}

Vector3 GetCamRight() {
    assert(0 && "Not implemented");
    return (Vector3){0, 0, -1};
}
