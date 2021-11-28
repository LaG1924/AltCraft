#version 330 core

in vec3 pos[4];
in vec3 normal;
in vec2 uv[4];
in float uvLayer;
in float animation;
in vec3 color;
in vec2 light;

out vec3 faceTextureUv;
out vec3 faceNormal;
out vec3 faceAddColor;
out vec2 faceLight;

layout (std140) uniform Globals {
    mat4 projView;
    uvec2 viewportSize;
    float globalTime;
    float dayTime;
    float gamma;
};

void main() {
    gl_Position = projView * vec4(pos[gl_VertexID], 1.0f);

    faceTextureUv = vec3(uv[gl_VertexID], uvLayer);
    faceTextureUv.y -= (uv[2].y - uv[0].y) * trunc(mod(globalTime * 4.0f, animation));

    faceNormal = normal;
    faceAddColor = color;
    faceLight = light;
}
