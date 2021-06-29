#version 330 core

in VS_OUT {
    vec3 Texture;
    vec3 Color;
} fs_in;

out vec4 fragColor;

uniform sampler2DArray textureAtlas;

void main() {
    vec4 color = texture(textureAtlas,fs_in.Texture);
    if (color.a < 0.3)
        discard;

    fragColor = vec4(color.rgb * fs_in.Color, 1.0);
}
