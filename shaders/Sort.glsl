#version 430 core

layout(location = 0, std430) uniform globals {
    mat4 viewMat;
    mat4 projMat;
}

layout(std430, binding = 0) buffer GaussianBuffer {
    vec4 position;
    vec4 rotation;
    vec3 scale;
    vec4 color;
} gaussians[];

layout(std430, binding = 1) buffer TransformedGaussianBuffer {
    vec4 position;
    mat3 covariance;
    unsigned int depthIndex; 
} t_gaussians[];

layout(std430, binding = 2) buffer DepthBuffer {
    unsigned int gaussianIndex;
    float depth;
} depth;

uniform bool sortEven = false;

void main() {

}
