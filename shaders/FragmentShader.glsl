
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

// This should probably be done in the transform shader tbh
mat2 invertMat2(mat2 mat) {
    float det = (mat[0][0]*mat[1][1])
              - (mat[0][1]*mat[1][0]);
    
    mat2 tmp = {
        { mat[1][1], -mat[0][1]},
        {-mat[1][0],  mat[0][0]}
    };

    return tmp/det;
}

float gaussianPDF(vec2 coord, vec2 mew, mat2 invCov) {
    return exp(-0.5 * dot((coord - mew), invCov*(coord - mew)));
}

uniform mat2 testCov = {
    {0.25, 0.10},
    {0.10, 0.10}
};

void main() {
    vec2 coord = (fragTexCoord*2) - 1;

    mat2 inverseCov = invertMat2(testCov);

    float pdfVal = gaussianPDF(coord, vec2(0, 0), inverseCov);

    outputColor = vec4(vec3(pdfVal), 1.0);
}
