#version 330 core

uniform  mat4 view;
uniform  mat4 projection;
uniform  mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uvPosition;

out vec2 uvPos;
out vec3 pos;

void main(){
    uvPos = uvPosition;
    pos = position;
    gl_Position = projection*view*model*vec4(position,1);
}