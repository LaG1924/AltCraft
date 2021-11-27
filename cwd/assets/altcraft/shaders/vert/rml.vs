#version 330 core

in vec2 pos;
in uvec4 col;
in vec2 uvPos;

out vec4 color;
out vec2 uv;

uniform vec2 translation;

layout (std140) uniform Globals {
    mat4 projView;
    uvec2 viewportSize;
    float globalTime;
    float dayTime;
};

void main() {
    float x = ((pos.x + translation.x) / viewportSize.x) * 2.0f - 1.0f;
    float y = ((pos.y + translation.y) / viewportSize.y) * 2.0f - 1.0f;
    gl_Position = vec4(x, -y, -1.0f, 1.0f);
    color = vec4(float(col.x) / 255.0f, float(col.y) / 255.0f, float(col.z) / 255.0f, float(col.w) / 255.0f);
    uv = uvPos;
}
