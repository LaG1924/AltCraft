#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D depthStencil;

void main() {
    vec4 c = texture(color, uv);
    vec4 n = texture(normal, uv);
    float d = texture(depthStencil, uv).r;
    fragColor = n;
}
