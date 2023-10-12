#include "globals_util.h"
#include <rlgl.h>
#include <stdio.h>

struct Globals {
    float viewMat[4][4];
    float projMat[4][4];
} globals;

unsigned int globalsId;

bool InitGlobals() {
    // Init matrices
    float* v = (float*)globals.viewMat;
    v[ 0] = 1; v[ 1] = 0; v[ 2] = 0; v[ 3] = 0;
    v[ 4] = 0; v[ 5] = 1; v[ 6] = 0; v[ 7] = 0;
    v[ 8] = 0; v[ 9] = 0; v[10] = 1; v[11] = 0;
    v[12] = 0; v[13] = 0; v[14] = 0; v[15] = 1;

    float* p = (float*)globals.projMat;
    p[ 0] = 1; p[ 1] = 0; p[ 2] = 0; p[ 3] = 0;
    p[ 4] = 0; p[ 5] = 1; p[ 6] = 0; p[ 7] = 0;
    p[ 8] = 0; p[ 9] = 0; p[10] = 1; p[11] = 0;
    p[12] = 0; p[13] = 0; p[14] = 0; p[15] = 1;

    printf("viewMat[0][0] = %f\n", globals.viewMat[0][0]);

    // Upload to GPU
    globalsId = rlLoadShaderBuffer(sizeof(globals), &globals, 0); // TODO: Change the 0
    if (!globalsId) {
        fprintf(stderr, "Could not load globals!");
        return false;
    }
    rlBindShaderBuffer(globalsId, 0);

    return true;
}
