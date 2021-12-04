#version 330 core

in vec4 faceWorldPos;
in vec3 faceTextureUv;
in vec3 faceAddColor;
in vec3 faceNormal;
in vec2 faceLight;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 worldPos;
layout (location = 3) out vec4 addColor;
layout (location = 4) out vec4 light;

uniform sampler2DArray textureAtlas;

void main() {
    vec4 col = texture(textureAtlas, faceTextureUv);
    if (col.a < 0.3)
        discard;

    color = vec4(col.rgb, 1.0f);
    normal = vec4(faceNormal, 1.0f);
    worldPos = faceWorldPos;
    addColor = vec4(faceAddColor, 1.0f);
    light = vec4(faceLight / 15.0f, 0.0f, 1.0f);
}
