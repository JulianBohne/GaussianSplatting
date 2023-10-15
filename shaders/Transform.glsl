#version 430 core

layout(std430, row_major, binding = 0) buffer globals {
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

layout(std430, row_major, binding = 2) buffer TransformedGaussianBuffer {
    struct TransformedGaussian {
        vec4 position;
        mat2 invCovariance;
        unsigned int depthIndex; // These have to be set to 0-N in ascending order for this to work
    } t_gaussians[];
};

layout(std430, row_major, binding = 3) buffer DepthBuffer {
    struct Depth {
        unsigned int gaussianIndex;
        float depth;
    } depthBuf[];
};

mat2 invertMat2(mat2 mat) {
    float det = (mat[0][0]*mat[1][1])
              - (mat[0][1]*mat[1][0]);
    
    mat2 tmp = {
        { mat[1][1], -mat[0][1]},
        {-mat[1][0],  mat[0][0]}
    };

    return tmp/det;
}

mat3 createJacobian(vec4 at) {

    float a = projMat[0][0];
    float b = projMat[1][1];
    float d = projMat[2][3];

    return mat3(
        -a/at.z, 0, (a * at.x)/(at.z * at.z),
        0, -b/at.z, (b * at.y)/(at.z * at.z),
        0, 0, d/(at.z*at.z)
    );
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    unsigned int i = gl_GlobalInvocationID.x;

    vec4 camSpacePos = viewMat*gaussians[i].position;

    vec4 projected = vec4(0.0, 0.0, camSpacePos.z + 10, 1.0);
    projected = projMat*camSpacePos;
    projected /= projected.w;

    mat3 cov3D = {
        {0.1,   0,   0},
        {  0, 0.1,   0},
        {  0,   0, 0.1}
    };

    mat3 W = mat3(viewMat);

    mat3 J = createJacobian(camSpacePos);

    mat3 t_cov = J * W * cov3D * transpose(W) * transpose(J);

    t_gaussians[i].position = projected;
    t_gaussians[i].invCovariance = invertMat2(mat2(t_cov));
    depthBuf[t_gaussians[i].depthIndex].gaussianIndex = i;
    depthBuf[t_gaussians[i].depthIndex].depth = -t_gaussians[i].position.z;
}
