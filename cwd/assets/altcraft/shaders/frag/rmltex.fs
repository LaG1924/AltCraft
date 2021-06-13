#version 330 core

uniform sampler2D fontTexture;

in VS_OUT {
    vec4 color;
    vec2 tex_coord;
} fs_in;

out vec4 fragColor;

void main() {
    fragColor = fs_in.color * texture(fontTexture, fs_in.tex_coord);
}
