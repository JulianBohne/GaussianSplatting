#include "globals_util.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>

struct Globals {
    float viewMat[4][4];
    float projMat[4][4];
} globals;

unsigned int globalsId;

float inverseViewRotation[3][3];

float fovy;
float near;
float far;

bool InitGlobals(float fieldOfViewY, float nearClip, float farClip) {

    // Aspect ratio
    float a = ((float)GetRenderWidth()) / GetRenderHeight();
    // printf("Aspect ratio: %f\n", a);

    fovy = fieldOfViewY;
    near = nearClip;
    far = farClip;

    float f = 1.0 / tanf(fovy / 2.0);

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
        }
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
    rlUpdateShaderBuffer(globalsId, &globals, sizeof(globals), 0); // I'm just gonna upload everything
}


void MoveCameraBy(float x, float y, float z) {
    globals.viewMat[0][3] += x;
    globals.viewMat[1][3] += y;
    globals.viewMat[2][3] += z;
}

void MoveCameraTo(float x, float y, float z) {
    globals.viewMat[0][3] = x;
    globals.viewMat[1][3] = y;
    globals.viewMat[2][3] = z;
}
