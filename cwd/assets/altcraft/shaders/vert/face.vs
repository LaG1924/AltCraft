#version 330 core

in vec3 position[4];
in vec3 normal;
in vec2 uv[4];
in float uvLayer;
in float animation;
in vec3 color;
in vec2 light;

out vec3 faceTexture;
out vec3 faceNormal;
out vec3 faceAddColor;
out vec2 faceLight;

uniform mat4 projView;
uniform float GlobalTime;

void main() {
    gl_Position = projView * vec4(position[gl_VertexID], 1.0f);

    faceTexture = vec3(uv[gl_VertexID], uvLayer);
    faceTexture.y -= (uv[2].y - uv[0].y) * trunc(mod(GlobalTime * 4.0f, animation));

    faceNormal = normal;
    faceAddColor = color;
    faceLight = light;
}
