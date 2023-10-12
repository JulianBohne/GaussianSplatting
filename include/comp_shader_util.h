#ifndef COMP_SHADER_UTIL_H
#define COMP_SHADER_UTIL_H

#include <stdbool.h>

typedef  struct CompShader {
    unsigned int id;
    bool enabled;
} CompShader;

// This first
CompShader LoadCompShader(const char* filePath);
void UnloadCompShader(CompShader shader);

bool IsCompShaderValid(CompShader shader);

// This second
void EnableCompShader(CompShader* shader);
void DisableCompShader(CompShader* shader);

// Then you can do this
void DispatchCompShader(CompShader shader, unsigned int groupX, unsigned int groupY, unsigned int groupZ);

#endif // COMP_SHADER_UTIL_H
