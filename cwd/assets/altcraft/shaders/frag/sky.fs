#version 330 core

in vec2 uvPos;
in vec3 pos;

out vec4 fragColor;

uniform sampler2DArray textureAtlas;
uniform float DayTime;
uniform vec4 sunTexture;
uniform float sunTextureLayer;
uniform vec4 moonTexture;
uniform float moonTextureLayer;

const vec4 DaySkyColor = vec4(0.49,0.66,1, 1);

const vec3 SunPos = vec3(0,0.1,0.5);

const vec3 MoonPos = vec3(0,0.1,-0.5);

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
    vec3 sunDelta = (pos - SunPos)*3.0f;
    float distanceToSun = length(sunDelta);
    vec4 sunColor = texture(textureAtlas,TransformTextureCoord(sunTexture,(vec2(sunDelta.xy)+0.5f),sunTextureLayer));
    vec4 sun = mix(vec4(0,0,0,1),sunColor,clamp(1-distanceToSun*2.0f,0,1));
    return sun;
}

vec4 Moon() {
    vec3 moonDelta = (pos - MoonPos)*4.5f;
    float distanceToMoon = length(moonDelta);
    vec4 moonColor = texture(textureAtlas,TransformTextureCoord(moonTexture,(vec2(moonDelta.xy)+0.5f),moonTextureLayer));
    vec4 moon = mix(vec4(0,0,0,1),moonColor,clamp(1-distanceToMoon*2.0f,0,1));
    return moon;
}

void main() {
    vec4 starColor = vec4(0.0f, 0.04f, 0.06f, 1.0f);
    fragColor = mix(starColor, DaySkyColor, DayTime);
    fragColor += Sun();
    fragColor += Moon();
}