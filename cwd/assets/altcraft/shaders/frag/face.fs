#version 330 core

in VS_OUT {
    vec3 Texture;
    vec3 Color;
    vec3 faceNormal;
} fs_in;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normal;

uniform sampler2DArray textureAtlas;

void main() {
    vec4 col = texture(textureAtlas, fs_in.Texture);
    if (col.a < 0.3)
        discard;

    color = vec4(col.rgb * fs_in.Color, 1.0f);
    normal = vec4(fs_in.faceNormal, 1.0f);
}
