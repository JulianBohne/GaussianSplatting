#version 430 core

layout(location = 5) uniform struct block {
    float x;
} instance;

layout(std430, binding = 0) buffer bufA {
    float bufAValues[];
};

layout(std430, binding = 1) buffer bufB {
    float bufBValues[];
};

layout(std430, binding = 2) buffer bufC {
    float bufCValues[];
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    unsigned int index = gl_GlobalInvocationID.x;
    bufCValues[index] = bufAValues[index] + bufBValues[index] + instance.x;
}
