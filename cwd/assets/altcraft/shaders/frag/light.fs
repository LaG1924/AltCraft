#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D depthStencil;
uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D addColor;
uniform sampler2D light;
uniform sampler2D ssao;

uniform int renderBuff;
uniform bool applySsao;

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

vec3 RecoverViewWorldPos(vec2 screenPos, float depth) {
    vec4 viewPos = invProj * vec4(screenPos * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return viewPos.xyz / viewPos.w;
}

void main() {
    vec4 c = texture(color, uv);
    vec4 n = texture(normal, uv);
    n += 1.0f;
    n /= 2.0f;

    vec4 ac = texture(addColor, uv);
    vec4 l = texture(light, uv);
    float depth = texture(depthStencil, uv).r;
    float d = (1.0f - depth) * 16.0f;
    vec4 s = texture(ssao, uv);

    float faceLight = l.r;
    float skyLight = l.g;
    float lightLevel = clamp(faceLight + skyLight * dayTime, 0.01f, 1.0f);
    lightLevel = pow(lightLevel, 3);
    if (applySsao) {
        lightLevel *= pow(s.r, 2);
    }
    lightLevel = clamp(lightLevel, 0.005f, 1.0f);

    vec4 finalColor = vec4(c.rgb * ac.rgb * lightLevel, 1.0f);

    finalColor.rgb = pow(finalColor.rgb, vec3(1.0f / gamma));

    switch(renderBuff) {
        case 0:
            fragColor = finalColor;
            break;
        case 1:
            fragColor = c;
            break;
        case 2:
            fragColor = n;
            break;
        case 3:
            fragColor = vec4(RecoverViewWorldPos(uv, depth), 1.0f);
            break;
        case 4:
            fragColor = ac;
            break;
        case 5:
            fragColor = l;
            break;
        case 6:
            fragColor = vec4(vec3(d), 1.0f);
            break;
        case 7:
            fragColor = vec4(pow(s.r, 2));
            break;
    }
}
