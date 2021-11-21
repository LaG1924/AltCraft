#version 330 core

in vec3 position[4];
in vec3 normal;
in vec2 uv[4];
in float uvLayer;
in float animation;
in vec3 color;
in vec2 light;

out VS_OUT {
    vec3 Texture;
    vec3 Color;
    vec3 faceNormal;
} vs_out;

uniform float GlobalTime;
uniform mat4 projView;
uniform float DayTime;
uniform float MinLightLevel;

void main() {
    gl_Position = projView * vec4(position[gl_VertexID], 1.0f);
    vs_out.Texture = vec3(uv[gl_VertexID], uvLayer);
    vs_out.Texture.y -= (uv[2].y - uv[0].y) * trunc(mod(GlobalTime * 4.0f, animation));

    float faceLight = clamp(light.x / 15.0 + (light.y / 15.0) * DayTime, MinLightLevel, 1.0);
    vs_out.Color = mix(color.rgb * faceLight, vec3(1,1,1) * faceLight, float(color == vec3(0,0,0)));
    vs_out.faceNormal = normal;
}
