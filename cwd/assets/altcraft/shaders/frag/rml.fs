#version 330 core

in VS_OUT {
    vec4 color;
    vec2 tex_coord;
} fs_in;

out vec4 fragColor;

void main() {
    fragColor = fs_in.color;
}
