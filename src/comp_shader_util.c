#include "comp_shader_util.h"
#include <stdio.h>
#include <assert.h>
#include <raylib.h>
#include <rlgl.h>

CompShader LoadCompShader(const char* filePath) {
    const char* compShaderCode = LoadFileText(filePath);
    if (!compShaderCode) {
        fprintf(stderr, "Could not load file text: %s", filePath);
        return (CompShader){0};
    }
    unsigned int compShaderId = rlCompileShader(compShaderCode, RL_COMPUTE_SHADER);
    if (!compShaderId) {
        fprintf(stderr, "Could not create/compile shader: %s", filePath);
        return (CompShader){0};
    }
    unsigned int compProgId = rlLoadComputeShaderProgram(compShaderId);
    if (!compProgId) {
        fprintf(stderr, "Could not load shader: %s", filePath);
        return (CompShader){0};
    }
    return (CompShader) {
        .id = compProgId,
        .enabled = false
    };
}

void UnloadCompShader(CompShader shader) {
    rlUnloadShaderProgram(shader.id);
}

bool IsCompShaderValid(CompShader shader) {
    return shader.id != 0;
}

void EnableCompShader(CompShader* shader) {
    rlEnableShader(shader->id);
    shader->enabled = true;
}

void DisableCompShader(CompShader* shader) {
    rlDisableShader();
    shader->enabled = false;
}

void DispatchCompShader(CompShader shader, unsigned int groupX, unsigned int groupY, unsigned int groupZ) {
    assert(shader.enabled && "The shader has to be enabled first");
    rlComputeShaderDispatch(groupX, groupY, groupZ);
}
