
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 outputColor;

layout(std430, binding = 0) buffer globals {
    mat4 viewMat;
    mat4 projMat;
};

layout(std430, row_major, binding = 1) buffer GaussianBuffer {
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

float gaussianPDF(vec2 coord, vec2 mew, mat2 invCov) {
    return exp(-0.5 * dot((coord - mew), invCov*(coord - mew)));
}

void main() {
    vec2 coord = ((fragTexCoord*2) - 1);

    float alpha = 1.0;

    for (int i = 0; i < gaussians.length(); i ++) {
        unsigned int gI = depthBuf[i].gaussianIndex;
        if (t_gaussians[gI].position.z >= 1) break;

        vec2 gaussianPos = t_gaussians[gI].position.xy;
        mat2 gaussianInvCov = t_gaussians[gI].invCovariance;

        float pdfVal = gaussianPDF(coord, gaussianPos, gaussianInvCov);

        outputColor += alpha*pdfVal*gaussians[gI].color;
        alpha -= alpha*pdfVal*gaussians[gI].color.w;
    }
}
