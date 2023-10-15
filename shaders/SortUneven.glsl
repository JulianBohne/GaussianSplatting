#version 430 core

layout(std430, binding = 2) buffer TransformedGaussianBuffer {
    struct TransformedGaussian {
        vec4 position;
        mat2 invCovariance;
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
    unsigned int i = gl_GlobalInvocationID.x*2 + 1;
    
    if (depthBuf[i].depth < depthBuf[i+1].depth) {
        float tmpDepth = depthBuf[i].depth;
        unsigned int tmpGaussianIndex = depthBuf[i].gaussianIndex;

        depthBuf[i].depth = depthBuf[i+1].depth;
        depthBuf[i].gaussianIndex = depthBuf[i+1].gaussianIndex;

        depthBuf[i+1].depth = tmpDepth;
        depthBuf[i+1].gaussianIndex = tmpGaussianIndex;

        t_gaussians[depthBuf[i].gaussianIndex].depthIndex = i;
        t_gaussians[depthBuf[i+1].gaussianIndex].depthIndex = i+1;
    }

}
