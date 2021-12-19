#version 330 core

in vec3 pos[4];
in vec2 uv[4];
in vec2 light[4];
in vec3 normal;
in vec3 color;
in vec3 layerAnimationAo;

out vec3 faceTextureUv;
out vec3 faceNormal;
out vec3 faceAddColor;
out vec2 faceLight;
out float faceAmbientOcclusion;

layout (std140) uniform Globals {
    mat4 projView;
    mat4 proj;
    mat4 invProj;
    mat4 view;
    uvec2 viewportSize;
    vec4 ssaoKernels[64];
    float globalTime;
    float dayTime;
    float gamma;
};

void main() {
    gl_Position = projView * vec4(pos[gl_VertexID], 1.0f);

    faceTextureUv = vec3(uv[gl_VertexID], layerAnimationAo.r);
    faceTextureUv.y -= (uv[2].y - uv[0].y) * trunc(mod(globalTime * 4.0f, layerAnimationAo.g));

    faceNormal = (view * vec4(normal, 0.0f)).xyz;
    faceAddColor = color;
    faceLight = light[gl_VertexID];
    faceAmbientOcclusion = layerAnimationAo.b;
}
