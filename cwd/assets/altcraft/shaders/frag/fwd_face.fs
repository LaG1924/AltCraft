#version 330 core

in vec4 faceWorldPos;
in vec3 faceTextureUv;
in vec3 faceAddColor;
in vec3 faceNormal;
in vec2 faceLight;

out vec4 fragColor;

uniform sampler2DArray textureAtlas;

layout (std140) uniform Globals {
    mat4 projView;
    mat4 proj;
    mat4 invProj;
    mat4 view;
    uvec2 viewportSize;
    vec4 ssaoKernels[64];
    float globalTime;
    float dayTime;
    float gamma;
};

void main() {
    vec4 col = texture(textureAtlas, faceTextureUv);
    if (col.a < 0.3f)
        discard;

    float localLight = faceLight.r / 15.0f;
    float skyLight = faceLight.g / 15.0f;
    float lightLevel = clamp(localLight + skyLight * dayTime, 0.01f, 1.0f);
    lightLevel = pow(lightLevel, 3);
    lightLevel = clamp(lightLevel, 0.005f, 1.0f);

    fragColor = vec4(col.rgb * faceAddColor.rgb * lightLevel, 1.0f);

    fragColor.rgb = pow(fragColor.rgb, vec3(1.0f / gamma));
}
