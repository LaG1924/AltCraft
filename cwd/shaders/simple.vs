#version 330 core

uniform  mat4 view;
uniform  mat4 projection;
layout (location = 0) in vec3 position;

void main(){
    gl_Position = vec4(position,1);//projection*view*vec4(position,1);
}