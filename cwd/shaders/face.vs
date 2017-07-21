#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;
layout (location = 7) in vec4 Texture;
layout (location = 8) in mat4 model;
layout (location = 12) in vec3 color;

out VS_OUT {
    vec2 UvPosition;
    vec4 Texture;
    vec3 Color;
} vs_out;

uniform  mat4 view;
uniform  mat4 projection;
uniform  float time;

void main()
{
    vec4 sourcePosition = vec4(position,1.0f);
    gl_Position = projection * view * model * sourcePosition;

    vs_out.UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    vs_out.Texture = Texture;
    vs_out.Color = color;
}
