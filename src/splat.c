#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

// Compile raylib with OPENGL 4.3 otherwise we don't have compute shaders lol
// cmake -G "MinGW Makefiles" -D OPENGL_VERSION="4.3" .
// Not sure if I HAVE to specify the platform and graphics api here tbh, but I don't want to compile it again to try it out.
// make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_43
#include <raylib.h>
#include <rlgl.h>

#include "gaussian.h"
#include "comp_shader_util.h"
#include "globals_util.h"

#include "splat_loader.h"

#define N 300

float bufA[N];
float bufB[N];
float bufC[N];

Gaussian gaussians[N];
unsigned int gaussianBufId;

T_Gaussian t_gaussians[N];
unsigned int t_gaussianBufId;

DepthStruct depthBuffer[N];
unsigned int depthBufId;

float map(float x, float fromA, float fromB, float toA, float toB) {
    return toA + (toB - toA) * (x - fromA) / (fromB - fromA);
}

float randf(float min, float max) {
    return map((float)rand(), 0, (float)RAND_MAX, min, max);
}

bool InitBuffers() {
    srand(42);

    Splats splats = loadSplatsFromFile("resources/point_cloud.ply");
    
    if (splats.splatCount == 0) {
        return false;
    }

    Gaussian g = splats.splats[0];

    printf("First gaussian: {\n\tposition: [%f, %f, %f],\n\trotation: [%f, %f, %f, %f],\n\tscale: [%f, %f, %f],\n\tcolor: [%f, %f, %f, %f]\n}\n",
        g.position.x, g.position.y, g.position.z,
        g.rotation.x, g.rotation.y, g.rotation.z, g.rotation.w,
        g.scale.x, g.scale.y, g.scale.z,
        g.color.x, g.color.y, g.color.z, g.color.w 
    );

    // free(splats.splats); // FIXME: Move this!

    for (unsigned int i = 0; i < N; i ++) {
        gaussians[i].position = (Vec4){randf(-10, 10), randf(-10, 10), randf(-1, -10), 1.0}; // last 1.0 because of homogeneous coordinates
        gaussians[i].rotation = (Vec4){1.0, 0.0, 0.0, 0.0}; // Identity quaternion
        gaussians[i].scale = (Vec4){0.1, 0.1, 0.1, 1.0};
        gaussians[i].color = (Vec4){randf(0, 1), randf(0, 1), randf(0, 1), 1.0};

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

    InitWindow(800, 600, "Gaussian Splatting");
    
    SetTargetFPS(60);

    CompShader compTransform = LoadCompShader("shaders/Transform.glsl");
    if (!IsCompShaderValid(compTransform)) {
        return 1;
    }

    CompShader compSortEven = LoadCompShader("shaders/SortEven.glsl");
    if (!IsCompShaderValid(compSortEven)) {
        return 1;
    }

    CompShader compSortUneven = LoadCompShader("shaders/SortUneven.glsl");
    if (!IsCompShaderValid(compSortUneven)) {
        return 1;
    }


    if (!InitGlobals(PI*0.5, 0.01, 10000.0)) {
        return 2;
    }

    if (!InitBuffers()) {
        return 3;
    }
    
    printf("Dispatching shader...\n");
    
    EnableCompShader(&compTransform);
    DispatchCompShader(compTransform, N, 1, 1);

    rlReadShaderBuffer(t_gaussianBufId, (void*)t_gaussians, sizeof(T_Gaussian), 0);

    printf("Position: (%f, %f, %f, %f)\n", t_gaussians[0].position.x, t_gaussians[0].position.y, t_gaussians[0].position.z, t_gaussians[0].position.w);
    printf("Depth index: %d\n", t_gaussians[0].depthIndex);
    printf("\n");

    DisableCompShader(&compTransform);

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
            
            if (IsKeyDown(KEY_W)) camPos.y += 0.5;
            if (IsKeyDown(KEY_S)) camPos.y -= 0.5;
            if (IsKeyDown(KEY_A)) camPos.x += 0.5;
            if (IsKeyDown(KEY_D)) camPos.x -= 0.5;
            if (IsKeyDown(KEY_Q)) camPos.z += 0.5;
            if (IsKeyDown(KEY_E)) camPos.z -= 0.5;

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 mouseDelta = GetMouseDelta();
                camYaw += mouseDelta.x * 0.01;
                camPitch += -mouseDelta.y * 0.01;
                camPitch = camPitch > PI/2 ? PI/2 : camPitch;
                camPitch = camPitch < -PI/2 ? -PI/2 : camPitch;

            }

            UploadGlobalsChangesToGPU();

            EnableCompShader(&compTransform);
            DispatchCompShader(compTransform, N, 1, 1);
            DisableCompShader(&compTransform);

            EnableCompShader(&compSortEven);
            DispatchCompShader(compSortEven, N/2, 1, 1);
            DisableCompShader(&compSortEven);
            
            EnableCompShader(&compSortUneven);
            DispatchCompShader(compSortUneven, (N-1)/2, 1, 1);
            DisableCompShader(&compSortUneven);

            ClearBackground(BLACK);
            BeginShaderMode(fragShader);
                DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), WHITE);
            EndShaderMode();

            DrawFPS(10, 10);
        EndDrawing();
    }
    
    DeinitBuffers();

    UnloadCompShader(compTransform);

    CloseWindow();

    return 0;
}
