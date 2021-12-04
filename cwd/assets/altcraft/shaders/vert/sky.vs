#version 330 core

in vec3 pos;

out vec3 facePos;

uniform mat4 model;

layout (std140) uniform Globals {
    mat4 projView;
    mat4 proj;
    mat4 view;
    uvec2 viewportSize;
    vec4 ssaoKernels[64];
    float globalTime;
    float dayTime;
    float gamma;
};

void main() {
    facePos = pos;
    gl_Position = projView * model * vec4(pos, 1);
}
