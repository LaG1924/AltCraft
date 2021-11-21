#version 330 core

in vec2 pos;
in vec2 uvPos;

out vec2 uv;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    uv = uvPos;
}
