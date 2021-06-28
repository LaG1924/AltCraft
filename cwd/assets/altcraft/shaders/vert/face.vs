#version 330 core

layout (location = 0) in vec4 position[4];
layout (location = 4) in vec2 uv[4];
layout (location = 8) in float uvLayer;
layout (location = 9) in vec2 animation;
layout (location = 10) in vec3 color;
layout (location = 11) in vec2 light;

out VS_OUT {
    vec3 Texture;
    vec3 Color;
    vec2 Light;
} vs_out;

uniform float GlobalTime;
uniform mat4 projView;

void main() {
    gl_Position = projView * position[gl_VertexID];
    vs_out.Color = color;
    vs_out.Light = light;
    vs_out.Texture = vec3(uv[gl_VertexID], uvLayer);
    vs_out.Texture.y -= animation.x * trunc(mod(GlobalTime * 4.0f, animation.y));
}
