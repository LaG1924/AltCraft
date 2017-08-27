#version 330 core

uniform vec4 transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UvCoordinates;

out vec2 uv;

vec2 TransfromWidgetCoord() {
    vec2 origin = vec2((transform.x * 2.0f) - 1.0f, (0.5 - transform.y) * 2.0f);

    float x = transform.x;
    float y = transform.y;
    float w = transform.z;
    float h = transform.w;
    vec2 A = vec2(x, 1 - y - h);
    vec2 B = vec2(x + w, 1 - y);
    vec2 ret = vec2(A + position.xy * (B - A));
    return vec2(ret.x-1.0f,ret.y);
}

void main(){
    uv = UvCoordinates;

    gl_Position = vec4(TransfromWidgetCoord(),0,1);
}