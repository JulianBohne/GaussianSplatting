#version 430 core

layout(std430, binding = 0) buffer globals {
    mat4 viewMat;
    mat4 projMat;
};

layout(std430, binding = 1) buffer GaussianBuffer {
    struct Gaussian {
        vec4 position;
        vec4 rotation;
        vec3 scale;
        vec4 color;
    } gaussians[];
};

layout(std430, binding = 2) buffer TransformedGaussianBuffer {
    struct TransformedGaussian {
        vec4 position;
        mat4 covariance;
        unsigned int depthIndex; // These have to be set to 0-N in ascending order for this to work
    } t_gaussians[];
};

layout(std430, binding = 3) buffer DepthBuffer {
    struct Depth {
        unsigned int gaussianIndex;
        float depth;
    } depthBuf[];
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    unsigned int i = gl_GlobalInvocationID.x;
    t_gaussians[i].position = projMat*viewMat*gaussians[i].position;
    t_gaussians[i].position /= t_gaussians[i].position.w;
}
