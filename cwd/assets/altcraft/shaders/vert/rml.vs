#version 330 core

uniform uvec2 viewportSize;
uniform mat4 rotationMat;

layout (location = 0) in vec2 pos;
layout (location = 1) in uvec4 color;
layout (location = 2) in vec2 tex_coord;

out VS_OUT {
    vec4 color;
    vec2 tex_coord;
} vs_out;

void main() {
    float x = (pos.x - viewportSize.x) / viewportSize.x;
    float y = ((pos.y - viewportSize.y) / viewportSize.y) * -1;
    gl_Position = vec4(x, y, -1.0f, 1.0f);
    vs_out.color = vec4(float(color.x) / 255.0f, float(color.y) / 255.0f, float(color.z) / 255.0f, float(color.w) / 255.0f);
    vs_out.tex_coord = tex_coord;
}
