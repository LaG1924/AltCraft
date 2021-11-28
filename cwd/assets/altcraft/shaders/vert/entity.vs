#version 330 core

in vec3 pos;

uniform mat4 model;

layout (std140) uniform Globals {
    mat4 projView;
    uvec2 viewportSize;
    float globalTime;
    float dayTime;
    float gamma;
};

void main() {
    gl_Position = projView * model * vec4(pos, 1);
}
