#version 330 core

in vec3 position;

out vec3 pos;

uniform mat4 projView;
uniform mat4 model;

void main() {
    pos = position;
    gl_Position = projView * model * vec4(position, 1);
}
