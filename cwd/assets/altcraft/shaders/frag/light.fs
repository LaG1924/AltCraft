#version 330 core

out vec4 fragColor;

in vec2 uv;

uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D addColor;
uniform sampler2D light;
uniform sampler2D depthStencil;

uniform float dayTime;
uniform int renderBuff;

void main() {
    vec4 c = texture(color, uv);
    vec4 n = texture(normal, uv);
    vec4 ac = texture(addColor, uv);
    vec4 l = texture(light, uv);
    float d = 1.0f - texture(depthStencil, uv).r;

    float faceLight = l.r;
    float skyLight = l.g;
    float lightLevel = clamp(faceLight + skyLight * dayTime, 0.2f, 1.0f);
    vec3 faceColor = mix(ac.rgb * lightLevel, vec3(1,1,1) * lightLevel, float(ac.rgb == vec3(0,0,0)));

    vec4 finalColor = vec4(c.rgb * faceColor, 1.0f);

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
            fragColor = ac;
            break;
        case 4:
            fragColor = l;
            break;
        case 5:
            fragColor = vec4(d, d, d, 1.0f);
            break;
    }
}
