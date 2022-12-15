#version 330 core

in vec3 faceTextureUv;
in vec3 faceAddColor;
in vec3 faceNormal;
in vec2 faceLight;
in float faceAmbientOcclusion;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 light;

uniform sampler2DArray textureAtlas;

void main() {
    vec4 col = texture(textureAtlas, faceTextureUv);

    color = vec4(col.rgb * faceAddColor.rgb, 1.0f);
    normal = vec4(faceNormal, 1.0f);
    light = vec4(faceLight / 15.0f, faceAmbientOcclusion, 1.0f);
    color.a = col.a;
}
