#ifndef GLOBALS_UTIL_H
#define GLOBALS_UTIL_H

#include <stdbool.h>
#include <raylib.h>

bool InitGlobals(float fieldOfViewY, float nearClip, float farClip);

void UploadGlobalsChangesToGPU();

extern Vector3 camPos;
extern float camYaw;
extern float camPitch;

Vector3 GetCamForward();
Vector3 GetCamUp();
Vector3 GetCamRight();

#endif // GLOBALS_UTIL_H
