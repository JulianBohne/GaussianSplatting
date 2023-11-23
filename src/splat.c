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

#define N 2000

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

void printGaussian(const char* preamble, Gaussian* g) {
    printf("%s: {\n\tposition: [%f, %f, %f],\n\trotation: [%f, %f, %f, %f],\n\tscale: [%f, %f, %f],\n\tcolor: [%f, %f, %f, %f]\n}\n",
        preamble,
        g->position.x, g->position.y, g->position.z,
        g->rotation.x, g->rotation.y, g->rotation.z, g->rotation.w,
        g->scale.x, g->scale.y, g->scale.z,
        g->color.x, g->color.y, g->color.z, g->color.w 
    );
}

#define min(A, B) (A < B ? A : B)
#define max(A, B) (A > B ? A : B)

#define prop_min(A, BP, PROP) A.PROP = min(A.PROP, BP->PROP)
#define prop_max(A, BP, PROP) A.PROP = max(A.PROP, BP->PROP)
#define prop_sum(A, BP, PROP) A.PROP += BP->PROP

bool InitBuffers() {
    srand(42);

    Splats splats = loadSplatsFromFile("resources/point_cloud.ply");
    
    if (splats.splatCount == 0) {
        return false;
    }

    Gaussian minG = splats.splats[0];
    Gaussian maxG = splats.splats[0];
    Gaussian avgG = splats.splats[0];

    for (int i = 1; i < splats.splatCount; ++i) {
        Gaussian* g = &splats.splats[i];
        // minG.position.x = min(minG.position.x, g->position.x);
        prop_min(minG, g, position.x);
        prop_min(minG, g, position.y);
        prop_min(minG, g, position.z);
        prop_min(minG, g, rotation.x);
        prop_min(minG, g, rotation.y);
        prop_min(minG, g, rotation.z);
        prop_min(minG, g, rotation.w);
        prop_min(minG, g, scale.x);
        prop_min(minG, g, scale.y);
        prop_min(minG, g, scale.z);
        prop_min(minG, g, color.x);
        prop_min(minG, g, color.y);
        prop_min(minG, g, color.z);
        prop_min(minG, g, color.w);
        
        prop_max(maxG, g, position.x);
        prop_max(maxG, g, position.y);
        prop_max(maxG, g, position.z);
        prop_max(maxG, g, rotation.x);
        prop_max(maxG, g, rotation.y);
        prop_max(maxG, g, rotation.z);
        prop_max(maxG, g, rotation.w);
        prop_max(maxG, g, scale.x);
        prop_max(maxG, g, scale.y);
        prop_max(maxG, g, scale.z);
        prop_max(maxG, g, color.x);
        prop_max(maxG, g, color.y);
        prop_max(maxG, g, color.z);
        prop_max(maxG, g, color.w);
        
        prop_sum(avgG, g, position.x);
        prop_sum(avgG, g, position.y);
        prop_sum(avgG, g, position.z);
        prop_sum(avgG, g, rotation.x);
        prop_sum(avgG, g, rotation.y);
        prop_sum(avgG, g, rotation.z);
        prop_sum(avgG, g, rotation.w);
        prop_sum(avgG, g, scale.x);
        prop_sum(avgG, g, scale.y);
        prop_sum(avgG, g, scale.z);
        prop_sum(avgG, g, color.x);
        prop_sum(avgG, g, color.y);
        prop_sum(avgG, g, color.z);
        prop_sum(avgG, g, color.w);
    }

    avgG.position.x /= splats.splatCount;
    avgG.position.y /= splats.splatCount;
    avgG.position.z /= splats.splatCount;
    
    avgG.rotation.x /= splats.splatCount;
    avgG.rotation.y /= splats.splatCount;
    avgG.rotation.z /= splats.splatCount;
    avgG.rotation.w /= splats.splatCount;

    avgG.scale.x /= splats.splatCount;
    avgG.scale.y /= splats.splatCount;
    avgG.scale.z /= splats.splatCount;
    
    avgG.color.x /= splats.splatCount;
    avgG.color.y /= splats.splatCount;
    avgG.color.z /= splats.splatCount;
    avgG.color.w /= splats.splatCount;

    printGaussian("First Gaussian", &splats.splats[0]);
    printGaussian("Min Gaussian", &minG);
    printGaussian("Max Gaussian", &maxG);
    printGaussian("Avg Gaussian", &avgG);

    // free(splats.splats); // FIXME: Move this!

    for (unsigned int i = 0; i < N; i ++) {
        // // gaussians[i].position = (Vec4){randf(-10, 10), randf(-10, 10), randf(-1, -10), 1.0}; // last 1.0 because of homogeneous coordinates
        // gaussians[i].position = splats.splats[i].position;
        // gaussians[i].rotation = (Vec4){1.0, 0.0, 0.0, 0.0}; // Identity quaternion
        // gaussians[i].scale = (Vec4){0.01, 0.01, 0.01, 1.0};
        // // gaussians[i].color = (Vec4){randf(0, 1), randf(0, 1), randf(0, 1), 1.0};
        // gaussians[i].color = splats.splats[i].color;
        gaussians[i] = splats.splats[i];

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

            if (IsKeyPressed(KEY_SPACE)) printCurrentParams();

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
