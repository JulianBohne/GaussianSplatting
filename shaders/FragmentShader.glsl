
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 outputColor;

layout(std430, binding = 0) buffer globals {
    mat4 viewMat;
    mat4 projMat;
    unsigned int width;
    unsigned int height;
    unsigned int tileSize;
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
    vec2 coord = (fragTexCoord*2) - 1;
    vec2 pixelCoord = fragTexCoord*vec2(width, height);
    vec2 tileCount = floor(vec2(width-1, height-1)/tileSize) + 1;
    vec2 tileIndex = floor(pixelCoord/tileSize);
    vec2 tileCenter = (((tileIndex*tileSize + vec2(tileSize)/2.0) / vec2(width, height))*2) - 1;

    vec3 color = vec3(0.0);
    float alpha = 1.0;

    for (int i = 0; i < gaussians.length(); i ++) {
        unsigned int gI = depthBuf[i].gaussianIndex;
        if (abs(t_gaussians[gI].position.x) > 1 || abs(t_gaussians[gI].position.y) > 1 || abs(t_gaussians[gI].position.z) > 1) continue;

        vec2 gaussianPos = t_gaussians[gI].position.xy;
        mat2 gaussianInvCov = t_gaussians[gI].invCovariance;

        // Testing "tiling" method
        // if (gaussianPDF(tileCenter, gaussianPos, gaussianInvCov) < 0.01) continue;

        float pdfVal = gaussianPDF(coord, gaussianPos, gaussianInvCov);

        // color = color + alpha*pdfVal*gaussians[gI].color.xyz;
        color = pow(sqrt(color) + alpha*pdfVal*sqrt(gaussians[gI].color.xyz), vec3(2)); // Maybe a slightly better color mixing? 
        alpha -= alpha*pdfVal*gaussians[gI].color.w;
    }

    outputColor = vec4(color, 1.0-alpha);
    // outputColor = vec4(vec3((tileIndex.x + tileIndex.y * tileCount.x)/(tileCount.x * tileCount.y)), 1.0);
}
