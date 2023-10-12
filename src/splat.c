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
        gaussians[i].position = (Vec4){i, i+1, i+2, 1.0}; // last 1.0 because of homogeneous coordinates
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

void DeinitBuffers() {
    rlUnloadShaderBuffer(gaussianBufId);
    rlUnloadShaderBuffer(t_gaussianBufId);
    rlUnloadShaderBuffer(depthBufId);
}

int main(int argc, char** argv) {

    InitWindow(400, 300, "Gaussian Splatting");
    
    SetTargetFPS(60);

    CompShader compTransform = LoadCompShader("shaders/Transform.glsl");

    if (!IsCompShaderValid(compTransform)) {
        return 1;
    }

    EnableCompShader(&compTransform);

    if (!InitGlobals()) {
        return 2;
    }

    if (!InitBuffers()) {
        return 3;
    }
    
    printf("Dispatching shader...\n");
    
    DispatchCompShader(compTransform, N, 1, 1);

    rlReadShaderBuffer(t_gaussianBufId, (void*)t_gaussians, sizeof(T_Gaussian)*2, 0);

    printf("Position: (%f, %f, %f, %f)\n", t_gaussians[1].position.x, t_gaussians[1].position.y, t_gaussians[1].position.z, t_gaussians[1].position.w);
    printf("Depth index: %d\n", t_gaussians[1].depthIndex);
    printf("\n");

    DeinitBuffers();

    DisableCompShader(&compTransform);
    UnloadCompShader(compTransform);

    Shader fragShader = LoadShader(NULL, "shaders/FragmentShader.glsl");

    // Set default internal texture (1px white) and rectangle to be used for shapes drawing
    Texture2D defaultTexture = {
        .format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .width = 1,
        .height = 1,
        .mipmaps = 0,
        .id = rlGetTextureIdDefault()
    };
    SetShapesTexture(defaultTexture, (Rectangle){ 0.0f, 0.0f, 1.0f, 1.0f });

    while (!WindowShouldClose()) {
        BeginDrawing();
            BeginShaderMode(fragShader);
                DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), WHITE);
            EndShaderMode();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
