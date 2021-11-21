#version 330 core

in VS_OUT {
    vec3 Texture;
    vec3 Color;
} fs_in;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normal;

uniform sampler2DArray textureAtlas;

void main() {
    vec4 col = texture(textureAtlas, fs_in.Texture);
    if (col.a < 0.3)
        discard;

    color = vec4(col.rgb * fs_in.Color, 1.0f);
    normal = vec4(1.0f - color.r, 1.0f - color.b, 1.0f, 1.0f);
}
