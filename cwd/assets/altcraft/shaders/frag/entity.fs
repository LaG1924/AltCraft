#version 330 core

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 light;

uniform vec3 entityColor;

void main() {
    color = vec4(entityColor, 1.0f);
    normal = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
