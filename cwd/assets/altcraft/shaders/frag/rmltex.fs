#version 330 core

in vec4 color;
in vec2 uv;

out vec4 fragColor;

uniform sampler2D fontTexture;

void main() {
    fragColor = color * texture(fontTexture, uv);
}
