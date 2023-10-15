#ifndef GLOBALS_UTIL_H
#define GLOBALS_UTIL_H

#include <stdbool.h>

bool InitGlobals(float fieldOfViewY, float nearClip, float farClip);

void UploadGlobalsChangesToGPU();

void MoveCameraBy(float x, float y, float z);

void MoveCameraTo(float x, float y, float z);

#endif // GLOBALS_UTIL_H
