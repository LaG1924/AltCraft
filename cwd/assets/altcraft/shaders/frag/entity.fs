#version 330 core

uniform vec3 color;
in vec2 uvPos;
out vec4 fragColor;

void main(){
    //if (uvPos.x < 0.9 && uvPos.x > 0.1 && uvPos.y < 0.9 && uvPos.y > 0.1) discard;
    fragColor = vec4(color,1);
}