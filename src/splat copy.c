#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

// Compile raylib with OPENGL 4.3 otherwise we don't have compute shaders lol
// cmake -G "MinGW Makefiles" -D OPENGL_VERSION="4.3" .
// Not sure if I HAVE to specify the platform and graphics api here tbh, but I don't want to compile it again to try it out.
// make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_43
#include <raylib.h>
#include <rlgl.h>

#include "gaussian.h"
#include "comp_shader_util.h"
#include "globals_util.h"

#define N 10

float bufA[N];
float bufB[N];
float bufC[N];

Gaussian gaussians[N];
unsigned int gaussianBufId;

T_Gaussian t_gaussians[N];
unsigned int t_gaussianBufId;

DepthStruct depthBuffer[N];
unsigned int depthBufId;

bool InitBuffers() {
    for (unsigned int i = 0; i < N; i ++) {
        gaussians[i].position = (Vec4){0.0, 0.0, 0.0, 1.0}; // last 1.0 because of homogeneous coordinates
        gaussians[i].rotation = (Vec4){1.0, 0.0, 0.0, 0.0}; // Identity quaternion
        gaussians[i].scale = (Vec4){0.1, 0.1, 0.1, 1.0};
        gaussians[i].color = (Vec4){1.0, 1.0, 1.0, 1.0};

        // The depth index has to be initialized properly, but the rest will be overwritten anyway
        t_gaussians[i].depthIndex = i;

        // Depth buffer should be fine as is
    }

    gaussianBufId = rlLoadShaderBuffer(N*sizeof(Gaussian), gaussians, 0); // TODO: Change the 0
    if(!gaussianBufId) {
        fprintf(stderr, "Could not upload gaussians to GPU");
        return false;
    }

    t_gaussianBufId = rlLoadShaderBuffer(N*sizeof(T_Gaussian), t_gaussians, 0); // TODO: Change the 0
    if(!t_gaussianBufId) {
        fprintf(stderr, "Could not upload t_gaussians to GPU");
        return false;
    }

    depthBufId = rlLoadShaderBuffer(N*sizeof(DepthStruct), depthBuffer, 0); // TODO: Change the 0
    if(!depthBufId) {
        fprintf(stderr, "Could not upload depthBuffer to GPU");
        return false;
    }

    rlBindShaderBuffer(gaussianBufId, 1);
    rlBindShaderBuffer(t_gaussianBufId, 2);
    rlBindShaderBuffer(depthBufId, 3);

    return true;
}

void deinitBuffers() {
    rlUnloadShaderBuffer(gaussianBufId);
    rlUnloadShaderBuffer(t_gaussianBufId);
    rlUnloadShaderBuffer(depthBufId);
}

int main(int argc, char** argv) {

    InitWindow(400, 300, "Gaussian Splatting");
    
    SetTargetFPS(60);

    CompShader compShader = LoadCompShader("shaders/Test.glsl");
    EnableCompShader(&compShader);

    for (int i = 0; i < N; i ++) {
        bufA[i] = (float)i;
        bufB[i] = N - (float)i;
        bufC[i] = -1.0;
    }

    
    
    //rlEnableShader(compShaderProgramId);

    unsigned int bufAId = rlLoadShaderBuffer(N * sizeof(float), (void*)bufA, 0); // TODO: Change the 0
    unsigned int bufBId = rlLoadShaderBuffer(N * sizeof(float), (void*)bufB, 0); // TODO: Change the 0
    unsigned int bufCId = rlLoadShaderBuffer(N * sizeof(float), (void*)bufC, 0); // TODO: Change the 0

    printf("bufAId: %u, bufBId: %u, bufCId: %u\n", bufAId, bufBId, bufCId);

    rlBindShaderBuffer(bufAId, 0);
    rlBindShaderBuffer(bufBId, 1);
    rlBindShaderBuffer(bufCId, 2);

    printf("Location of x is: %d\n", rlGetLocationUniform(compShader.id, "x"));
    printf("Location of block is: %d\n", rlGetLocationUniform(compShader.id, "block"));
    printf("Location of block.x is: %d\n", rlGetLocationUniform(compShader.id, "block.x"));
    printf("Location of instance is: %d\n", rlGetLocationUniform(compShader.id, "instance"));
    printf("Location of instance.x is: %d\n", rlGetLocationUniform(compShader.id, "instance.x"));
    rlSetUniform(rlGetLocationUniform(compShader.id, "instance.x"), &(float){3.1415}, RL_SHADER_UNIFORM_FLOAT, 1);

    printf("Dispatching shader...\n");
    
    DispatchCompShader(compShader, N, 1, 1);

    rlReadShaderBuffer(bufCId, (void*)bufC, N * sizeof(float), 0);

    for (int i = 0; i < N; i ++) {
        printf("%f, ", bufC[i]);
    }
    printf("\n");

    DisableCompShader(&compShader);

    rlUnloadShaderBuffer(bufAId);
    rlUnloadShaderBuffer(bufBId);
    rlUnloadShaderBuffer(bufCId);

    UnloadCompShader(compShader);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
