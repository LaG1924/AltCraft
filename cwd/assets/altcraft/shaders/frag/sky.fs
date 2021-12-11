#version 330 core

in vec3 facePos;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 addColor;
layout (location = 3) out vec4 light;

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

uniform sampler2DArray textureAtlas;
uniform vec4 sunTexture;
uniform float sunTextureLayer;
uniform vec4 moonTexture;
uniform float moonTextureLayer;

const vec4 DaySkyColor = vec4(0.21, 0.4, 1, 1);

const vec3 SunPos = vec3(0, 0.1, 0.5);

const vec3 MoonPos = vec3(0, 0.1, -0.5);

const vec4 NightSkyColor = vec4(0.0, 0.0008, 0.002, 1.0);

vec3 TransformTextureCoord(vec4 TextureAtlasCoords, vec2 UvCoords, float Layer) {
    float x = TextureAtlasCoords.x;
    float y = TextureAtlasCoords.y;
    float w = TextureAtlasCoords.z;
    float h = TextureAtlasCoords.w;
    vec2 A = vec2(x, 1 - y - h);
    vec2 B = vec2(x + w, 1 - y);
    vec2 transformed = A + UvCoords * (B - A);
    return vec3(transformed.x, transformed.y, Layer);
}

vec4 Sun() {
    vec3 sunDelta = (facePos - SunPos) * 3.0f;
    float distanceToSun = length(sunDelta);
    vec4 sunColor = texture(textureAtlas, TransformTextureCoord(sunTexture, (vec2(sunDelta.xy) + 0.5f), sunTextureLayer));
    vec4 sun = mix(vec4(0, 0, 0, 1), sunColor, clamp(1 - distanceToSun * 2.0f, 0, 1));
    return sun;
}

vec4 Moon() {
    vec3 moonDelta = (facePos - MoonPos) * 4.5f;
    float distanceToMoon = length(moonDelta);
    vec4 moonColor = texture(textureAtlas, TransformTextureCoord(moonTexture, (vec2(moonDelta.xy) + 0.5f), moonTextureLayer));
    vec4 moon = mix(vec4(0, 0, 0, 1),moonColor, clamp(1 - distanceToMoon * 2.0f, 0, 1));
    return moon;
}

void main() {
    color = vec4(mix(NightSkyColor, DaySkyColor, dayTime).rgb, 1.0f);
    color += vec4(Sun().rgb, 1.0f);
    color += vec4(Moon().rgb, 1.0f);
    normal = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    addColor = vec4(1.0f);
    light = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    gl_FragDepth = 1.0f;
}
