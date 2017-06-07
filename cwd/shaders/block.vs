#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UvCoordinates;

out vec2 UvPosition;
out vec3 FragmentPosition;

uniform  mat4 view;
uniform  mat4 projection;
uniform  float time;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    UvPosition = vec2(UvCoordinates.x,UvCoordinates.y);
    FragmentPosition = position;
}
